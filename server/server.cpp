#include	"../util.h"
#include	"../const.h"
#include	"../msg_util.h"
#include	"../msg_buffer.h"

#include	<sys/epoll.h>

#include	<map>
#include	<string>
#include	<fstream>
#include	<iostream>
using namespace std;

/*
*load user name and password from registed file
*@return
	0 if success
	-1 if open file fail
*/
int load_registed_user(char* fname, map<string, string> & map_regist)
{
	ifstream fi(fname);
	if (fi.is_open())
	{
		string name, psw;
		fi >> name >> psw;
		
		while (!fi.eof())
		{
			map_regist[name] = psw;
			fi >> name >> psw;
		}

		fi.close();
		return 0;
	}
	else
		return -1;
}
/* 
*save one user to registed file 
*@return 
	0 if success
	-1 if open file fail
*/
int save_registed_user(char* fname, const string & name, const string & psw)
{
	ofstream fo(fname, ios_base::app);
	if (fo.is_open())
	{
		fo << name << "\t" << psw << "\n";	 
		fo.close();
		return 0;
	}
	else
		return -1;
}

/*
 *Accept a connection socket and add events to epoll
 *@return 
 	accepted socket id if success
	-1 if fail
 * */
int do_accept(int listen_fd, int epoll_fd)
{
	int conn_fd;

	/* accept a connection socket */
	if ( (conn_fd = accept(listen_fd, NULL, NULL)) < 0)
		return -1;

	if ( make_socket_unblocking(conn_fd) < 0) 
	{
		close(conn_fd);
		return -1;
	}

	/* add events to epoll */
	epoll_event ev;
	create_event(&ev, conn_fd, EPOLLIN);

	if ( epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) < 0)
	{
		close(conn_fd);
		return -1;
	}

	return conn_fd;
}

/*
*when there is message from client arrived at its socket, 
	this function will be used to provide service for this client.
*@param p_msg_buffer
	message buffer for receiving and sending message
*@param map_regist
	regist table
*@param map_login_name_sock,
		map_login_sock_name
	login table
*@param epoll_fd
	epoll fd used for opening writing event
*@return
	1 if success
	0 if client close
	-1 if read socket error
*/
int provide_service(Msg_buffer* p_msg_buffer, 
					map<string, string> & map_regist,
					map<string, int> & map_login_name_sock,
					map<int, string> & map_login_sock_name,
					int epoll_fd)
{
	int ret;
	if ( (ret = p_msg_buffer->read_all()) > 0)
	{
		char msg[CONST::MSG_SIZE];
		int  msg_len;
		while ( (msg_len = p_msg_buffer->pop_a_msg(msg)) >= 0)
		{
			/* we just deal with message with correcting formation,
			   throw away the message with bad formation */
			Msg_field fields;
			if ( Msg_util::unpacking(msg, msg_len, &fields))
			{
				if ( strcmp(fields.msg_type, CONST::MSG_TYPE_REGIST) == 0)
				{
					puts("regist!");	
				}
				else if ( strcmp(fields.msg_type, CONST::MSG_TYPE_LOGIN) == 0)
				{
					puts("login!");
				}
				else if ( strcmp(fields.msg_type, CONST::MSG_TYPE_MSG) == 0)
				{
				
				}
			}
		}
		return 1;
	}
	else if (ret == 0)
		return 0;
	else
		return -1;
}

void show_msg(char* msg, int msg_len)
{
	printf("message content :\n");
	for (int i = 0; i < msg_len; ++ i)
		printf("%d ", msg[i]);
	printf("\n");
}

void wait_event(int epoll_fd, int listen_fd)
{
	/* data structures will be used in some events */
	map<int, Msg_buffer*> map_sock_msgbuff; /* create a message buffer when accept a new connection,
											   destroy a message buffer when close a existed connection */
	
	map<string, int> map_login_name_sock; /* create a new <user_name, socket> pair when this user succeed to login 
								   			 destroy the <user_name, socket> pair when this user client has closed */
	map<int, string> map_login_sock_name; /* create a new <user_name, socket> pair when this user succeed to login 
								   			 destroy the <user_name, socket> pair when this user client has closed */
	
	map<string, string> map_regist; /* initialize with loading registed user from file,
									   create a new <user_name, user_password> pair when this user succeed to regist,
									   save this map into file when server close */
	
	if ( load_registed_user(CONST::REGISTED_FILE, map_regist) < 0)
	{
		printf("open registed file fail\n");
		return;
	}
	for (map<string, string>::iterator i=map_regist.begin(); i!=map_regist.end(); ++ i)
		cout << i->first << "\t" << i->second << "\n";	
	/* data structures finish */
	
	/* waiting for events */
	int n, i;
	epoll_event events[MAX_EVENTS];

	while (1)
	{
		if ( (n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1)) < 0)
			err_quit("epoll_wait call fail.");

		for (i = 0; i < n; ++ i)
		{
			int sock_fd = events[i].data.fd;

			if (sock_fd == listen_fd)
			{
				int conn_fd;
				if ( (conn_fd = do_accept(listen_fd, epoll_fd)) > 0)
				{
					map_sock_msgbuff[conn_fd] = new Msg_buffer(conn_fd, 
												CONST::MSG_QUEUE_SIZE, 
												CONST::MSG_N_BYTE_OF_LENGTH);
				}
			}
			else 
			{
				if (events[i].events & EPOLLIN)
				{
					int ret;
					if ( (ret = provide_service(map_sock_msgbuff[sock_fd], 
												map_regist,
												map_login_name_sock,
												map_login_sock_name,
												epoll_fd)) <= 0)
					{
						/* client has closed */
						// destroy map_sock_msgbuff
						// destroy map_login_name_sock
						// destroy map_login_sock_name
						printf("client close\n");
						
						close(sock_fd);
						
						delete map_sock_msgbuff[sock_fd];
						map_sock_msgbuff.erase(sock_fd);

						string name;
						name = map_login_sock_name[sock_fd];
						map_login_name_sock.erase(name);

						map_login_sock_name.erase(sock_fd);

						if (ret < 0)
							err_sys("read socket error");
					}
				}
			}
		}
	}
}



int main()
{
	int listen_fd;
	
	/* create listening socket */
	if ( (listen_fd = create_socket(NULL, PORT_NO)) < 0)
		err_quit("create listening socket fail.");

	if ( make_socket_unblocking(listen_fd) < 0)
		err_quit("make listening socket unblocking fail.");

	listen(listen_fd, 100);
	
	/* create epoll with lt */
	int epoll_fd;
	if ( (epoll_fd = epoll_create(1)) < 0)
		err_quit("create epoll fail.");

	/* add listening socket to epoll */
	epoll_event ev;
	create_event(&ev, listen_fd, EPOLLIN);

	if ( epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0)
		err_quit("add listening socket to epoll fail.");

	/* wait event */
	wait_event(epoll_fd, listen_fd);

	return 0;
}
