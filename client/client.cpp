#include	"../util.h"
#include	"../const.h"
#include	"../msg_util.h"
#include	"../msg_buffer.h"

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
void str_glue(char* s1, char* s2, char delimiter, char* new_str)
{
	strcpy(new_str, s1);
	strcpy(new_str+strlen(new_str)+1, s2);
	new_str[strlen(s1)] = delimiter;
}

/*
*state machine will run one step accroding to [msg, msg_len and state],
	other params are used to get or hold data.
*@param conn_fd in
*@param user_name in,out
*@param p_msg_buffer out
*@param wfds out
*/
void state_machine_run_a_step(char* msg, int msg_len, 
							int & state, 
							int conn_fd,
							char* user_name, 
							Msg_buffer* & p_msg_buffer,
							fd_set & wfds) 
{
	switch (state)
	{
	case CONST::NON_LOGIN_REGIST:
		if (msg_len == 2)
		{
			if (msg[0] == '0') 
			{
				state = CONST::LOGIN_WAIT_USERNAME;
				puts("用户名:");
			}
			else if (msg[0] == '1')
			{
				state = CONST::REGIST_WAIT_USERNAME;
				puts("用户名:");
			}
			else
			{
				puts("非法输入");
				puts("请输入 0:登录 1:注册");
			}
		}
		else 
		{
			puts("非法输入");
			puts("请输入 0:登录 1:注册");
		}
		break;
	case CONST::LOGIN_WAIT_USERNAME:
	case CONST::REGIST_WAIT_USERNAME:
		if (msg_len > 1 && msg_len <= CONST::USER_NAME_SIZE+1)
		{
			memmove(user_name, msg, msg_len);
			user_name[msg_len-1] = '\0';
			puts("密码:");
			if (state == CONST::LOGIN_WAIT_USERNAME)
				state = CONST::LOGIN_WAIT_USERPSW;
			else
				state = CONST::REGIST_WAIT_USERPSW;
		}
		else
		{
			puts("用户名不得为空,不得超过30字节(一个汉子符号占3字节,其它符号占1字节)");
			puts("用户名:");
		}
		break;
	case CONST::LOGIN_WAIT_USERPSW:
	case CONST::REGIST_WAIT_USERPSW:
		if (msg_len > 1 && msg_len <= CONST::USER_PSW_SIZE+1)
		{
			/* get password */
			char psw[CONST::USER_PSW_SIZE+1];
			memmove(psw, msg, msg_len);
			psw[msg_len-1] = '\0';
			/* glue user_name and user_password */
			char msg_data[CONST::MSG_DATA_SIZE];
			str_glue(user_name, psw, CONST::CHAR_GLUE_NAME_PSW, msg_data);
			puts("msg_data : ");
			puts(msg_data);
			/* packing to message content */
			char msg_content[CONST::MSG_SIZE];
			char msg_type[CONST::MSG_TYPE_SIZE];
			if (state == CONST::LOGIN_WAIT_USERPSW)
				strcpy(msg_type, CONST::MSG_TYPE_LOGIN);
			else
				strcpy(msg_type, CONST::MSG_TYPE_REGIST);
			Msg_field fields(msg_type, (char*)"", (char*)"", msg_data);
			int n_msg = Msg_util::packing(&fields, msg_content);
			printf("message content length=%d\n", n_msg);
			/* push message content into message buffer */
			if (p_msg_buffer->push_a_msg(msg_content, n_msg) == 0)
			{
				FD_SET(conn_fd, &wfds);	
				state = CONST::WAIT_SERVER_RET;
			}
			else
				puts("push message failed");
		}
		else
		{
			puts("密码不得为空,不得超过10位");
			puts("密码:");
		}
		break;
	case CONST::WAIT_SERVER_RET:
		break;
	case CONST::COMMUNICATE:
		break;
	}
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
	if ( (conn_fd = create_socket(argv[1], CONST::PORT_NO)) < 0)
		err_quit("create connection with server failed");

	p_msg_buffer = new Msg_buffer(conn_fd, 
								  CONST::MSG_QUEUE_SIZE, 
								  CONST::MSG_N_BYTE_OF_LENGTH);

	/* make socket unblocking */
	if ( make_socket_unblocking(conn_fd) < 0)
		err_quit("make socket unblocking failed");

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
		FD_SET(fileno(stdin), &rfds);
		FD_SET(conn_fd, &rfds);

		maxplus1 = conn_fd + 1;

		if ( (n_ready = select(maxplus1, &rfds, &wfds, NULL, NULL)) < 0)
			err_quit("select failed");

		if ( FD_ISSET(fileno(stdin), &rfds))
		{
			char user_msg[CONST::MSG_SIZE];
			int n_read = read(fileno(stdin), user_msg, sizeof(user_msg));
			if (n_read >= 0)
			{
				state_machine_run_a_step(user_msg, n_read,
										 client_state, 
										 conn_fd,
										 user_name, 
										 p_msg_buffer,
										 wfds);
			}
			else
				err_quit("stdin error");
		}

		if ( FD_ISSET(conn_fd, &rfds))
		{
		}

		if ( FD_ISSET(conn_fd, &wfds))
		{
			if ( p_msg_buffer->write_all() > 0)
				puts("全部消息写入socket");
			FD_CLR(conn_fd, &wfds);
		}
	}

	delete p_msg_buffer;
	return 0;
}
