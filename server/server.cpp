#include	"../util.h"
#include	"../const.h"
#include	"../msg_util.h"
#include	"../msg_buffer.h"

#include	"./db.h"
#include	"./service.h"

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

	if ( Util::make_socket_unblocking(conn_fd) < 0) 
	{
		close(conn_fd);
		return -1;
	}

	/* add events to epoll */
	epoll_event ev;
	Util::create_event(&ev, conn_fd, EPOLLIN);

	if ( epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) < 0)
	{
		close(conn_fd);
		return -1;
	}

	return conn_fd;
}

void close_sock(int sock_fd, map<int, Msg_buffer*> & map_sock_msgbuff, DB & db)
{
	/* destroy socket */
	close(sock_fd);
						
	delete map_sock_msgbuff[sock_fd];
	map_sock_msgbuff.erase(sock_fd);

	/* log out */
	string name;
	name = db.map_login_sock_name[sock_fd];
	db.map_login_name_sock.erase(name);

	db.map_login_sock_name.erase(sock_fd);
}

/*
*do service for the request message
*@param request_sockfd in
*@param	request_msg in
*@param	request_msg_len in
	request socket and message from client
*@param echo_msg out
*@param echo_msg_len out
	echo message generated by the service
*@param db in, out
	database, it will be used by service
*@return
	>=0 if success, it represents echo sockfd
	-1 if request message formation is wrong
*/
int do_service(const int request_sockfd, const char* request_msg, const int request_msg_len, 
			   char* echo_msg, int & echo_msg_len,
			   DB & db)
{
	int echo_sockfd = -1;
	Msg_field echo_fields;

	Msg_field request_fields;
	if ( Msg_util::unpacking(request_msg, request_msg_len, &request_fields))
	{
		if ( strcmp(request_fields.msg_type, CONST::MSG_TYPE_REGIST) == 0)
		{   
			/* client request regist service */
			if ( Service::regist(request_sockfd, request_fields, echo_sockfd, echo_fields, db) == 0)
				puts("注册成功");
			else
				puts("注册失败!");
		}
		else if ( strcmp(request_fields.msg_type, CONST::MSG_TYPE_LOGIN) == 0)
		{ 
			/* client request login service */
			if ( Service::login(request_sockfd, request_fields, echo_sockfd, echo_fields, db) == 0)
				puts("登录成功");
			else
				puts("登录失败!");
		}
		else if ( strcmp(request_fields.msg_type, CONST::MSG_TYPE_MSG) == 0 ||
				  strcmp(request_fields.msg_type, CONST::MSG_TYPE_FB) == 0  ||
				  strcmp(request_fields.msg_type, CONST::MSG_TYPE_FI) == 0 ||
				  strcmp(request_fields.msg_type, CONST::MSG_TYPE_FE) == 0 ||
				  strcmp(request_fields.msg_type, CONST::MSG_TYPE_FOK) == 0 ||
				  strcmp(request_fields.msg_type, CONST::MSG_TYPE_FNO) == 0 )
		{ 
			/* client request transponder message service */
			Service::transponder(request_sockfd, request_fields, echo_sockfd, echo_fields, db);
		}

		/* packing echo message */
		echo_msg_len = Msg_util::packing(&echo_fields, echo_msg);
	}
	return echo_sockfd;
}



void wait_event(int epoll_fd, int listen_fd)
{
	/* data structures will be used in some events */
	map<int, Msg_buffer*> map_sock_msgbuff; /* create a message buffer when accept a new connection,
											   destroy a message buffer when close a existed connection */
	
	DB db; /* database */
	
	if ( load_registed_user(CONST::REGISTED_FILE, db.map_regist) < 0)
	{
		printf("open registed file fail\n");
		return;
	}
	for (map<string, string>::iterator i=db.map_regist.begin(); i!=db.map_regist.end(); ++ i)
		cout << i->first << "\t" << i->second << "\n";	
	/* data structures finish */
	
	/* waiting for events */
	int n, i;
	epoll_event events[CONST::MAX_EVENTS];

	while (1)
	{
		if ( (n = epoll_wait(epoll_fd, events, CONST::MAX_EVENTS, -1)) < 0)
			Util::err_quit("epoll_wait call fail.");

		for (i = 0; i < n; ++ i)
		{
			int request_sockfd = events[i].data.fd;

			if (request_sockfd == listen_fd)
			{
				/* main function used to accepting client connection
				   and creating a messge buffer for new accepted client connection */
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
				/* main function is used to listening events,
				   reading and writing message from active socket.
			
				   work function is used to doing service for a message.
				   
				   1.this design may be not suit for multi-threads,
				   because params need to be shared between main and work threads. 
				   2.this design will block the listening of main function a little,
				   because main function need read and write socket.
				   3.but this design may be more clean than let work function not only do service,
				   and also read and write socket */
				if (events[i].events & EPOLLIN)
				{
					int ret;
					if ( (ret = map_sock_msgbuff[request_sockfd]->read_all()) > 0)
					{
						char request_msg[CONST::MSG_SIZE];
						int  request_msg_len;

						int  tmp = 0;
						while ( (request_msg_len = map_sock_msgbuff[request_sockfd]->pop_a_msg(request_msg)) > 0)
						{	
							char echo_msg[CONST::MSG_SIZE];
							int  echo_msg_len;
							int  echo_sockfd;
							if ( (echo_sockfd = do_service(request_sockfd, request_msg, request_msg_len, echo_msg, echo_msg_len, db)) < 0)
							{
								echo_msg_len = Msg_util::err_packing(echo_msg, (char*) "消息格式错误");
								echo_sockfd = request_sockfd;
							}

							if ( map_sock_msgbuff[echo_sockfd]->push_a_msg(echo_msg, echo_msg_len) == 0)
								Util::update_event(epoll_fd, echo_sockfd, EPOLLOUT);
							++ tmp;
						}
					}
					else if (ret == 0)
					{
						printf("client close\n");
						close_sock(request_sockfd, map_sock_msgbuff, db);
					}
					else
					{
						Util::err_sys("read socket error");
						close_sock(request_sockfd, map_sock_msgbuff, db);
					}
				}
				if (events[i].events & EPOLLOUT)
				{
					Msg_buffer* p_msg_buffer;
					p_msg_buffer = map_sock_msgbuff[request_sockfd];

					int ret;
					if ( (ret = p_msg_buffer->write_all()) > 0)
						Util::update_event(epoll_fd, request_sockfd, EPOLLIN);
					else if (ret < 0)
						close_sock(request_sockfd, map_sock_msgbuff, db);
					else
						; /* socket writing operation blocked,
						     do nothing and just wait this socket 
							 writing operation ready again,
							 then write rest bytes from message buffer to socket. */
				}
			}
		}
	}
}



int main()
{
	int listen_fd;
	
	/* create listening socket */
	if ( (listen_fd = Util::create_socket(NULL, CONST::PORT_NO)) < 0)
		Util::err_quit("create listening socket fail.");

	if ( Util::make_socket_unblocking(listen_fd) < 0)
		Util::err_quit("make listening socket unblocking fail.");

	listen(listen_fd, 100);
	
	/* create epoll with lt */
	int epoll_fd;
	if ( (epoll_fd = epoll_create(1)) < 0)
		Util::err_quit("create epoll fail.");

	/* add listening socket to epoll */
	epoll_event ev;
	Util::create_event(&ev, listen_fd, EPOLLIN);

	if ( epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0)
		Util::err_quit("add listening socket to epoll fail.");

	/* wait event */
	wait_event(epoll_fd, listen_fd);

	return 0;
}
