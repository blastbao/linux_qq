#include	"util.h"

int Util::create_socket(char* ip, int port_no) {
	int sock_fd;
	struct sockaddr_in serv_addr;

	if ( (sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_port        = htons(port_no);
	if (!ip)
		serv_addr.sin_addr.s_addr = INADDR_ANY;
	else 
		inet_pton(AF_INET, ip,  &serv_addr.sin_addr);

	if (!ip)
	{
		if ( bind(sock_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
			return -1;
	}
	else
	{
		if ( connect(sock_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
			return -1;
	}

	return sock_fd;
}

int Util::make_socket_unblocking(int sockfd) {
	int flags;

	if ( (flags = fcntl(sockfd, F_GETFL, 0)) < 0)
		return -1;

	flags |= O_NONBLOCK;
	if ( fcntl(sockfd, F_SETFL, flags) < 0)
		return -1;
	
	return 0;
}

void Util::create_event(epoll_event* pev, int fd, uint32_t events) {
	(pev->data).fd = fd;
	pev->events = events;
}

int Util::update_event(int epoll_fd, int sock_fd, uint32_t events)
{
	epoll_event ev;
	create_event(&ev, sock_fd, events);

	if ( epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &ev) < 0)
		return -1;
	return 0;
}

void Util::str_glue(const char* s1, const char* s2, const char delimiter, char* new_str)
{
	strcpy(new_str, s1);
	strcpy(new_str+strlen(new_str)+1, s2);
	new_str[strlen(s1)] = delimiter;
}

void Util::str_split(const char* old_str, const char delimiter, char* s1, char* s2)
{
	int i;
	for (i = 0; old_str[i] != delimiter; ++ i)
		s1[i] = old_str[i];
	s1[i++] = '\0';

	int j = 0;
	for (; old_str[i] != '\0'; ++ i)
		s2[j++] = old_str[i];
	s2[j] = '\0';
}

void Util::err_quit(const char* msg) {
	printf("%s, error code = %d\n", msg, errno);
	_exit(1);
}

void Util::err_sys(const char* msg) {
	printf("%s, error code = %d\n", msg, errno);
}
