/*
*1.从socket中读取或写入固定格式消息的缓冲类.
*2.适用于: 通信双方采用的消息包含"消息长度"和"消息内容"两个域,
*3.使用此类可以带来的帮助:
	3.1 简化从socket中读取消息的工作.
	3.2 简化向socket中写入消息,并且在socket发送缓冲已满的情况下不会造成消息丢失.
	3.3 push_a_msg()和write_all()接口加入了互斥锁,写入socket操作是线程安全的
*4.需要注意:
	4.1 read_all()函数可能会从socket中读出多个消息,为避免其中部分消息处于饥饿状态,
		需要重复调用pop_a_msg()函数,确保所有消息都可以及时得到处理.
	4.2 当socket发送缓冲已满,write_all()函数不能将消息继续写入到socket发送缓冲;
		当socket发送缓冲再次就绪时,应该立即调用write_all()将消息的剩余部分写入socket.
*5.client和server代码包含了此类的使用样例
*/
#ifndef __APP_LAYER_H__
#define __APP_LAYER_H__

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	<unistd.h>
#include	<pthread.h>
#include	<errno.h>

class Msg_buffer
{

public:

	/*
	*@param sock_fd in
		the socket id that this app layer will read from and write to
	*@param queue_size in
		this parameter is firstly used to init the size of read queue and write queue,
		is secondly used to expand the size of read queue and write queue
	*@param n_byte_of_msg_len in
		the byte number of message length field, th value have to be in (0, 4]
	*/
	Msg_buffer(int sock_fd, int queue_size, int n_byte_of_msg_len);
	~Msg_buffer();

	/*
	*read all bytes from its socket
	*@return
		1 if succeed to read all bytes from its socket
		0 if encounter EOF
		-1 if any system error occur
	*/
	int read_all();

	/*
	*pop a message from read queue, if there is no an entire message in read queue, return fail
	*@param msg_buff out
		a buffer to hold the message content
	*@return
		>= 0 if success, the value equals message content length
		-1 if there is no message
	*/
	int pop_a_msg(char* msg_buff);

	/*
	*push a message into write queue, this function is always success
	*@param msg_buff in
		a buffer that hold the message content to pushing
	*@param msg_len in
		the length of the message content, the value have to be 
		in the range of int, witch is (0, 2147483647]
	*@return
		0 if success
		-1 if msg_len less than 0
	*/
	int push_a_msg(char* msg_buff, int msg_len);

	/*
	*write all bytes from write queue to its socket
	*@return 
		1 if all bytes in write queue are copyed to its socket
		0 if the socket is full, and there are some bytes still in write queue waiting for copy
		-1 if any system error occur
	*/
	int write_all();

	/*
	* attribute
	*/
	int get_sockfd() {return _sock_fd;}

private:

	int _sock_fd;

	char* _read_queue;
	int _r_len;
	int _r_head;
	int _r_tail;
	
	char* _write_queue;
	int _w_len;
	int _w_head;
	int _w_tail;
	pthread_mutex_t _w_mutex;

	int _queue_size;

	int _n_byte_of_msg_len;

	int  get_msg_len();
	void set_msg_len(int msg_len);
	void reset_queue(char* q, int & head, int & tail);
};

#endif
