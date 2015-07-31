#include	"../util.h"
#include	"../const.h"
#include	"../msg_util.h"
#include	"../msg_buffer.h"
#include	"../ring_queue.h"

#include	"./user_input_util.h"
#include	"./fsocket.h"

#include	<sys/select.h>
#include	<pthread.h>

/*
*[Client] is a [state machine],
*[the state of client] and [the data received from user or server] determine 
	[the next client state] and [what should client do].
*/

/*====================*/
/*       global       */
int conn_fd; 				/* socket with server */
Msg_buffer* p_msg_buffer;	/* message buffer with conn_fd socket */

char user_name[CONST::USER_NAME_SIZE+1];

Fsocket_s 	fsock_s;		/* sending socket for file transfering */
Fsocket_r	fsock_r;		/* receiving socket for file transfering */
Ring_queue*	p_ring_queue;	/* ring queue for receiving file thread */
/*====================*/


/*
*all state transition function by user inputing follow the standard below:
*-----four params are basic:-----
*@param msg, msg_len in
	message from user inputing
*@param state in, out
	the current state of client
*@param name in, out
	user name
*-----if user will send message, another param is needed:-----
*@param send_msg_fields out
	the message fields user will send
*-----@return-----
	true if client succeed to go to next state,
	false if user inputing is illegal, and the state of client unchanged.
*/
bool go_to_wait_input_user_name_state(const char* msg, const int msg_len, int & state, char* name)
{
	if (msg_len == 1)
	{
		if (msg[0] == '0') 
		{
			state = CONST::LOGIN_WAIT_USERNAME;
			return true;
		}
		else if (msg[0] == '1')
		{
			state = CONST::REGIST_WAIT_USERNAME;
			return true;
		}
		else
			return false;
	}
	else 
		return false;
}
bool go_to_wait_input_user_psw_state(const char* msg, const int msg_len, int & state, char* name)
{
	if (msg_len > 0 && msg_len <= CONST::USER_NAME_SIZE)
	{
		memmove(name, msg, msg_len);
		name[msg_len] = '\0';
		if (state == CONST::LOGIN_WAIT_USERNAME)
			state = CONST::LOGIN_WAIT_USERPSW;
		else
			state = CONST::REGIST_WAIT_USERPSW;
		return true;
	}
	else
		return false;
}
bool go_to_wait_server_ret_state(const char* msg, const int msg_len, int & state, char* name, Msg_field & send_msg_fields)
{
	if (msg_len > 0 && msg_len <= CONST::USER_PSW_SIZE)
	{
		/* get password */
		char psw[CONST::USER_PSW_SIZE+1];
		memmove(psw, msg, msg_len);
		psw[msg_len] = '\0';
		
		/* glue name and password */
		char msg_data[CONST::MSG_DATA_SIZE];
		Util::str_glue(name, psw, CONST::CHAR_GLUE_NAME_PSW, msg_data);
		
		/* packing to message content */
		char msg_type[CONST::MSG_TYPE_SIZE];
		if (state == CONST::LOGIN_WAIT_USERPSW)
			strcpy(msg_type, CONST::MSG_TYPE_LOGIN);
		else
			strcpy(msg_type, CONST::MSG_TYPE_REGIST);
		send_msg_fields.set_fields(msg_type, (char*)"", (char*)"", msg_data);
		
		/* go to next state */
		state = CONST::WAIT_SERVER_RET;
		return true;
	}
	else
		return false;
}
void sending_msg(const char* msg_type, const char* from_name, const char* to_name, const char* data, 
				Msg_buffer* p_msg_buffer, const int conn_fd, fd_set & wfds)
{	
	Msg_field send_msg_fields;
	send_msg_fields.set_fields(msg_type, from_name, to_name, data);

	char send_msg[CONST::MSG_SIZE];
	int  send_msg_len;
	send_msg_len = Msg_util::packing(&send_msg_fields, send_msg);
	p_msg_buffer->push_a_msg(send_msg, send_msg_len);
	FD_SET(conn_fd, &wfds);
}

/*
*@param msg, msg_len
	message from server inputing
*@param state in, out
	the current state of client
*@param receive_msg_fields out
	the message fields received from server
*-----@return-----
	true if message formation is correct, client succeed to go to next state,
	false if message formation is wrong, and the state of client unchanged.
*/
bool go_to_communication_state(const char* msg, const int msg_len, int & state, Msg_field & receive_msg_fields)
{
	if ( Msg_util::unpacking(msg, msg_len, &receive_msg_fields))
	{
		if ( strcmp(receive_msg_fields.msg_type, CONST::MSG_TYPE_RET_SUCC) == 0)
			state = CONST::COMMUNICATE;
		else
			state = CONST::QUIT;
		return true;
	}
	else
		return false;
}
bool receiving_msg(const char* msg, const int msg_len, Msg_field & receive_msg_fields)
{
	if ( Msg_util::unpacking(msg, msg_len, &receive_msg_fields))
		return true;
	else
		return false;
}

/*
*Threads for sending and receiving file
*Sending Thread will read data from files and write data into socket,
*Receiving Thread will read data from ring queue and write data into files.
*/
void *sending_files(void* argv)
{
	int ret, sock_ret;
	Msg_field msg_fields;
	char msg[CONST::MSG_SIZE];
	int  msg_len;
	
	char data[CONST::MSG_DATA_SIZE+1];
	while ( (ret = fsock_s.fsock_read(data)) > 0)	
	{
		msg_fields.set_fields(CONST::MSG_TYPE_FI, user_name, fsock_s.get_othside_name(), data);
		msg_len = Msg_util::packing(& msg_fields, msg);

		p_msg_buffer->push_a_msg(msg, msg_len);
		p_msg_buffer->write_all();
	}

	if (ret == 0)
	{
		msg_fields.set_fields(CONST::MSG_TYPE_FE, user_name, fsock_s.get_othside_name(), data);
		msg_len = Msg_util::packing(& msg_fields, msg);

		p_msg_buffer->push_a_msg(msg, msg_len);
		if ( p_msg_buffer->write_all() == 0)
			puts("socket写缓冲已满，还需要把剩下的消息全部写入socket");
	}

	else
		puts("read file error !!!!");

	while( p_msg_buffer->write_all() == 0);
	
	fsock_s.fsock_close();
	puts("发送文件结束");
}

void *receiving_files(void* argv)
{
	int  ret;
	char data[CONST::MSG_DATA_SIZE+1];

	while (1)
	{
		p_ring_queue->pop(data);

		if ( (ret = fsock_r.fsock_write(data)) < 0)
		{
			puts("write file error !!!!");
			break;
		}
		if (ret == 0)
		{
			puts("文件接收成功");
			break;
		}
	}

	fsock_r.fsock_close();
}

void run_sending_thread()
{
	pthread_t tid;
	pthread_create(&tid, NULL, sending_files, NULL);
}

void run_receiving_thread()
{
	pthread_t tid;
	pthread_create(&tid, NULL, receiving_files, NULL);
}

int main(int argc, char** argv) {
	
	int client_state;

	if (argc < 2)
		Util::err_quit("please input server ip !");

	/* create socket and message buffer */
	if ( (conn_fd = Util::create_socket(argv[1], CONST::PORT_NO)) < 0)
		Util::err_quit("create connection with server failed");

	p_msg_buffer = new Msg_buffer(conn_fd, 
								  CONST::MSG_QUEUE_SIZE, 
								  CONST::MSG_N_BYTE_OF_LENGTH);

	/* make socket unblocking */
	if ( Util::make_socket_unblocking(conn_fd) < 0)
		Util::err_quit("make socket unblocking failed");

	/* create ring queue for receiving file */
	p_ring_queue = new Ring_queue(CONST::RING_QUEUE_SIZE, CONST::MSG_DATA_SIZE+1);

	/* init client state */
	client_state = CONST::NON_LOGIN_REGIST;
	puts("请输入 0:登录 1:注册");

	/* monitor user input and server input */
	fd_set rfds, wfds;
	int n_ready, maxplus1;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	while (1)
	{

		if (client_state == CONST::QUIT)
			break;

		FD_SET(fileno(stdin), &rfds);
		FD_SET(conn_fd, &rfds);
		maxplus1 = conn_fd + 1;

		if ( (n_ready = select(maxplus1, &rfds, &wfds, NULL, NULL)) < 0)
			Util::err_quit("select failed");

		if ( FD_ISSET(fileno(stdin), &rfds))
		{
			char user_msg[CONST::MSG_SIZE];
			int  n_read;
			if ( (n_read = read(fileno(stdin), user_msg, CONST::MSG_SIZE)) >= 0)
			{
				-- n_read; /* remove '\n' from user message */
				
				User_input_field input_fields;
				Msg_field		 send_msg_fields;
				char			 send_msg[CONST::MSG_SIZE];
				int				 send_msg_len;
				switch (client_state)
				{
				case CONST::NON_LOGIN_REGIST:
					if ( go_to_wait_input_user_name_state(user_msg, n_read, client_state, user_name))
						puts("用户名:");
					else
						puts("非法输入\n请输入 0.登录 1.注册");
					break;
				case CONST::REGIST_WAIT_USERNAME:
				case CONST::LOGIN_WAIT_USERNAME:
					if ( go_to_wait_input_user_psw_state(user_msg, n_read, client_state, user_name))
						puts("密码:");
					else
						puts("用户名不得为空,也不得超过30字节(一个汉子占3字节,其它符号占1字节)\n用户名:");
					break;
				case CONST::REGIST_WAIT_USERPSW:
				case CONST::LOGIN_WAIT_USERPSW:
					if ( go_to_wait_server_ret_state(user_msg, n_read, client_state, user_name, send_msg_fields))
					{
						send_msg_len = Msg_util::packing(&send_msg_fields, send_msg);
						p_msg_buffer->push_a_msg(send_msg, send_msg_len);
						FD_SET(conn_fd, &wfds);
					}
					else
						puts("密码不得为空,不得超过10位\n密码:");
					break;
				case CONST::COMMUNICATE:
					/*
					* there are two types of communication
					* first type is sending message,
					* second type is sending file 
					*/
					if ( User_input_util::extract(user_msg, n_read, input_fields))
					{
						if ( strcmp(input_fields.request_type, CONST::USER_REQUEST_TYPE_MSG) == 0)
						{
							/* User would like to send message */
							sending_msg(CONST::MSG_TYPE_MSG, user_name, input_fields.to_name, input_fields.data, 
										p_msg_buffer, conn_fd, wfds);
						}
						else if ( strcmp(input_fields.request_type, CONST::USER_REQUEST_TYPE_FTO) == 0)
						{
							/* User would like to send file */
							if ( !fsock_s.get_trans())
								if ( fsock_s.fsock_connect(input_fields.to_name, input_fields.data))
									sending_msg(CONST::MSG_TYPE_FB, user_name, input_fields.to_name, fsock_s.get_file_name(), 
												p_msg_buffer, conn_fd, wfds);
								else
									puts("路径错误 或 路径太长(路径不得超过256个字符)");
							else
								puts("您正在向其它用户发送文件，暂不支持同时发送多个文件");
						}
						else if ( strcmp(input_fields.request_type, CONST::USER_REQUEST_TYPE_FOK) == 0)
						{
							/* User agree with receiving file */
							if ( !fsock_r.get_trans())
								if ( fsock_r.fsock_is_exist(input_fields.to_name, input_fields.data))
								{
									sending_msg(CONST::MSG_TYPE_FOK, user_name, input_fields.to_name, input_fields.data,
												p_msg_buffer, conn_fd, wfds);
									fsock_r.set_trans(); /* start transfering state */
									p_ring_queue->make_empty(); /* make ring queue empty */
									run_receiving_thread(); /* start receiving files thread */
								}
								else
									puts("接收列表中无此文件");
							else
								puts("您正在接受其它用户发送的文件，暂不支持同时接受多个文件");
						}
						else if ( strcmp(input_fields.request_type, CONST::USER_REQUEST_TYPE_FNO) == 0)
						{
							/* User refuse to receive file */
							if ( !fsock_r.get_trans())
								if ( fsock_r.fsock_is_exist(input_fields.to_name, input_fields.data))
								{
									fsock_r.fsock_close();
									sending_msg(CONST::MSG_TYPE_FNO, user_name, input_fields.to_name, input_fields.data,
												p_msg_buffer, conn_fd, wfds);
								}
								else
									puts("接收列表中无此文件");
							else
								puts("您正在接受其它用户发送的文件，暂不支持同时接受多个文件");
						}
					}
					else
						puts("输入格式错误");
					break;
				default:
					puts("输入无效");
				}
			}
			else
			{
				puts("stdin error");
				break;
			}
		}

		if ( FD_ISSET(conn_fd, &rfds))
		{
			int ret;
			if ( (ret = p_msg_buffer->read_all()) > 0)
			{
				char serv_msg[CONST::MSG_SIZE];
				int  serv_msg_len;
				while ( (serv_msg_len = p_msg_buffer->pop_a_msg(serv_msg)) >= 0)
				{
					Msg_field serv_msg_fields; 
					switch (client_state)
					{
					case CONST::WAIT_SERVER_RET:
						if ( go_to_communication_state(serv_msg, serv_msg_len, client_state, serv_msg_fields))
							puts(serv_msg_fields.msg_data);
						else
							puts("来自服务器的消息格式错误");
						break;
					case CONST::COMMUNICATE:
						if ( receiving_msg(serv_msg, serv_msg_len, serv_msg_fields))
						{
							if ( strcmp(serv_msg_fields.msg_type, CONST::MSG_TYPE_MSG) == 0)
							{
								printf("from [%s]: %s\n", serv_msg_fields.msg_from, serv_msg_fields.msg_data);
							}
							else if ( strcmp(serv_msg_fields.msg_type, CONST::MSG_TYPE_FB) == 0)
							{
								fsock_r.fsock_accept(serv_msg_fields.msg_from, serv_msg_fields.msg_data);
							}
							else if ( strcmp(serv_msg_fields.msg_type, CONST::MSG_TYPE_FOK) == 0)
							{
								fsock_s.set_trans(); /* start transfering state */
								run_sending_thread(); /* start thread to send files */
							}
							else if ( strcmp(serv_msg_fields.msg_type, CONST::MSG_TYPE_FNO) == 0)
							{
								fsock_s.fsock_close();
								printf("[%s] 拒绝了你的发送文件请求\n", serv_msg_fields.msg_from);
							}
							else if ( strcmp(serv_msg_fields.msg_type, CONST::MSG_TYPE_FI) == 0)
							{
								p_ring_queue->put(serv_msg_fields.msg_data); /* put data into ring queue */
							}
							else if ( strcmp(serv_msg_fields.msg_type, CONST::MSG_TYPE_FE) == 0)
							{
								p_ring_queue->put(CONST::MSG_TYPE_FE); /* put the eof of the file transfering into ring queue */
							}
							else
								printf("%s\n", serv_msg_fields.msg_data);
						}
						else
							puts("来自服务器的消息格式错误");
						break;
					default:
						puts("异常的服务器数据");
					}
				}
			}
			else if (ret == 0)
			{
				puts("服务器关闭");
				break;
			}
			else
			{
				puts("读取socket数据错误");		
				break;
			}
		}

		if ( FD_ISSET(conn_fd, &wfds))
		{
			int ret;
			if ( (ret = p_msg_buffer->write_all()) > 0)
			{
				FD_CLR(conn_fd, &wfds);
			}
			else if (ret < 0)
			{
				puts("向socket写入数据出错");
				break;
			}
			else
				; /* writing socket block */
		}
	}

	delete p_ring_queue;
	delete p_msg_buffer;
	close(conn_fd);
	return 0;
}
