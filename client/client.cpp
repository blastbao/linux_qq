#include	"../util.h"
#include	"../const.h"
#include	"../msg_util.h"
#include	"../msg_buffer.h"

#include	"./user_input_util.h"

#include	<sys/select.h>

/*
*[Client] is a [state machine],
*[the state of client] and [the data received from user or server] determine 
	[the next client state] and [what should client do].
*/

/*
*glue s1 and s2 into new_str
*@param s1 in
	this variable have to end with '\0'
*@param s2 in
	this variable have to end with '\0'
*@param delimiter in
	it is used as a delimiter between s1 and s2
*@param new_str out
	new_str must have enough space to hold s1+delimiter+s2
	this variable will be end with '\0'
*@return
	assume this function is always success
*/
void str_glue(const char* s1, const char* s2, const char delimiter, char* new_str)
{
	strcpy(new_str, s1);
	strcpy(new_str+strlen(new_str)+1, s2);
	new_str[strlen(s1)] = delimiter;
}

/*
*all state transition function by user inputing follow the standard below:
*-----four params are basic:-----
*@param msg, msg_len in
	message from user inputing
*@param state in, out
	the current state of client
*@param name in, out
	user name
*-----if user will send message, two another params are needed:-----
*@param send_msg, send_msg_len out
	the message user will send
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
bool go_to_wait_server_ret_state(const char* msg, const int msg_len, int & state, char* name, char* send_msg, int & send_msg_len)
{
	if (msg_len > 0 && msg_len <= CONST::USER_PSW_SIZE)
	{
		/* get password */
		char psw[CONST::USER_PSW_SIZE+1];
		memmove(psw, msg, msg_len);
		psw[msg_len] = '\0';
		
		/* glue name and password */
		char msg_data[CONST::MSG_DATA_SIZE];
		str_glue(name, psw, CONST::CHAR_GLUE_NAME_PSW, msg_data);
		
		/* packing to message content */
		char msg_type[CONST::MSG_TYPE_SIZE];
		if (state == CONST::LOGIN_WAIT_USERPSW)
			strcpy(msg_type, CONST::MSG_TYPE_LOGIN);
		else
			strcpy(msg_type, CONST::MSG_TYPE_REGIST);
		Msg_field msg_fields(msg_type, (char*)"", (char*)"", msg_data);

		send_msg_len = Msg_util::packing(&msg_fields, send_msg);
		
		/* go to next state */
		state = CONST::WAIT_SERVER_RET;
		return true;
	}
	else
		return false;
}
bool stay_at_communication_state(const char* msg, const int msg_len, int & state, char* name, char* send_msg, int & send_msg_len)
{
	User_input_field input_field;
	
	if ( User_input_util::extract(msg, msg_len, &input_field))
	{
		Msg_field msg_fields(CONST::MSG_TYPE_MSG, name, input_field.to_name, input_field.data);
		send_msg_len = Msg_util::packing(&msg_fields, send_msg);
		return true;
	}
	else
		return false;
}

/*
*all state transition function by server inputing follow the standard below:
*-----four params are basic:-----
*@param msg, msg_len in
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
bool stay_at_communication_state(const char* msg, const int msg_len, int & state, Msg_field & receive_msg_fields)
{
	if ( Msg_util::unpacking(msg, msg_len, &receive_msg_fields))
		return true;
	else
		return false;
}


int main(int argc, char** argv) {
	
	int client_state;

	int conn_fd; 				/* socket with server */
	Msg_buffer* p_msg_buffer;	/* message buffer with conn_fd socket */
	
	char user_name[CONST::USER_NAME_SIZE+1];

	if (argc < 2) {
		printf("please input server ip!\n");
		_exit(1);
	}

	/* create socket and message buffer */
	if ( (conn_fd = Util::create_socket(argv[1], CONST::PORT_NO)) < 0)
		Util::err_quit("create connection with server failed");

	p_msg_buffer = new Msg_buffer(conn_fd, 
								  CONST::MSG_QUEUE_SIZE, 
								  CONST::MSG_N_BYTE_OF_LENGTH);

	/* make socket unblocking */
	if ( Util::make_socket_unblocking(conn_fd) < 0)
		Util::err_quit("make socket unblocking failed");

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
				
				char send_msg[CONST::MSG_SIZE];
				int  send_msg_len;
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
					if ( go_to_wait_server_ret_state(user_msg, n_read, client_state, user_name, send_msg, send_msg_len))
					{
						p_msg_buffer->push_a_msg(send_msg, send_msg_len);
						FD_SET(conn_fd, &wfds);
					}
					else
						puts("密码不得为空,不得超过10位\n密码:");
					break;
				case CONST::COMMUNICATE:
					if ( stay_at_communication_state(user_msg, n_read, client_state, user_name, send_msg, send_msg_len))
					{
						p_msg_buffer->push_a_msg(send_msg, send_msg_len);
						FD_SET(conn_fd, &wfds);
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
						if ( stay_at_communication_state(serv_msg, serv_msg_len, client_state, serv_msg_fields))
							if ( strlen(serv_msg_fields.msg_from) > 0)
								printf("from [%s]: %s\n", serv_msg_fields.msg_from, serv_msg_fields.msg_data);
							else
								printf("%s\n", serv_msg_fields.msg_data);
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

	delete p_msg_buffer;
	close(conn_fd);
	return 0;
}
