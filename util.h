#ifndef __UTIL_H__
#define __UTIL_H__

#include 	<stdio.h>
#include	<string.h>

#include	<sys/socket.h>
#include	<sys/types.h>
#include	<netinet/in.h>
#include	<sys/epoll.h>

#include	<unistd.h>
#include	<arpa/inet.h>
#include	<fcntl.h>
#include	<errno.h>


class Util
{

public:

	/*
	* @para port_num : the port number will be listened
	* @return : -1 if failed; otherwise return listening socket id
	*/
	static int create_socket(char* ip, int port_no);
	
	/*
	* @ param fd : the socket file descriptor will be made unblocking
	* @ return : -1 if failed; otherwise return 0
	*/
	static int make_socket_unblocking(int sockfd);
	
	/*
	 * @param pev
	 	a pointer to an epoll_event struct
	 * @param fd
	 	a file descriptor that will be listened
	 * @param events
	 	a bit set that represents event types
	 */
	static void create_event(epoll_event* pev, int fd, uint32_t events);
	
	/*
	 *@return 
	 	0 if success
		-1 if fail
	 * */
	static int update_event(int epoll_fd, int sock_fd, uint32_t events);
	
	static void err_quit(const char* msg);
	static void err_sys(const char* msg);

};

#endif
