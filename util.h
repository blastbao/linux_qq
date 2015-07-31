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
	static void str_glue(const char* s1, const char* s2, const char delimiter, char* new_str);

	/*
	*split old_str into s1 and s2
	*@param old_str in
		it must be end with '\0', and must contain one delimiter character
	*@param delimiter in
		used to spliting
	*@param s1 out
		it will be end with '\0', it must have enough space
	*@param s2 out
		it will be end with '\0', it must have enough space
	*@assume
		this function always success
	*/
	static void str_split(const char* old_str, const char delimiter, char* s1, char* s2);

	static void err_quit(const char* msg);
	static void err_sys(const char* msg);

};

#endif
