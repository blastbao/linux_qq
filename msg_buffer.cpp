#include	"msg_buffer.h"

Msg_buffer::Msg_buffer(int sock_fd, int queue_size, int n_byte_of_msg_len)
{
	_sock_fd 			= sock_fd;
	_queue_size 		= queue_size;
	_n_byte_of_msg_len 	= n_byte_of_msg_len;

	_read_queue = (char*) malloc(queue_size);
	_r_len 	= queue_size;
	_r_head 	= 0;
	_r_tail 	= 0;
	
	_write_queue= (char*) malloc(queue_size);
	_w_len 	= queue_size;
	_w_head 	= 0;
	_w_tail 	= 0;
}

int Msg_buffer::read_all()
{
	int n_read;
	bool read_flag = false;

	while (1)
	{
		/*if (_r_tail >= _r_len)
		{
			_read_queue = realloc(_read_queue, _r_len+_queue_size);
			_r_len += _queue_size;
		}*/

		if ( (n_read = read(_sock_fd, _read_queue+_r_tail, _r_len-_r_tail)) < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (n_read == 0)
		{
			if (!read_flag)
				return 0;
			else
				break;
		}
		else
		{
			_r_tail += n_read;
			read_flag = true;
		}
	}

	return 1;
}

int Msg_buffer::pop_a_msg(char* msg_buff)
{
	/* get message length */
	int msg_len;
	if ( (msg_len = get_msg_len()) < 0)
		return -1;
	
	/* copy the message to msg_buff */
	int n_byte_rest = _r_tail - (_r_head+_n_byte_of_msg_len);

	if (msg_len > n_byte_rest)
		return -1;

	memmove(msg_buff, _read_queue+_r_head+_n_byte_of_msg_len, msg_len);
	_r_head += _n_byte_of_msg_len + msg_len;

	reset_queue(_read_queue, _r_head, _r_tail);

	return msg_len;
}

int Msg_buffer::push_a_msg(char* msg_buff, int msg_len)
{
	if (msg_len < 0)
		return -1;

	/*if (_w_tail+_n_byte_of_msg_len+msg_len > _w_len)
	{
		_write_queue = (char*) realloc(_write_queue, _w_len+_queue_size);
		_w_len += _queue_size;
	}*/

	/* set message length */
	set_msg_len(msg_len);

	/* copy message to write queue */
	memmove(_write_queue+_w_tail+_n_byte_of_msg_len, msg_buff, msg_len);
	_w_tail += _n_byte_of_msg_len + msg_len;
	return 0;
}

int Msg_buffer::write_all()
{
	int n_write;

	while (_w_tail-_w_head > 0)
	{
		n_write = write(_sock_fd, _write_queue+_w_head, _w_tail-_w_head);
		if (n_write < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				reset_queue(_write_queue, _w_head, _w_tail);
				return 0;
			}
			if (errno == EINTR)
				continue;
			return -1;
		}
		else
		{
			_w_head += n_write;
		}
	}

	_w_head = _w_tail = 0;

	return 1;
}

/*
* private
*/

int Msg_buffer::get_msg_len()
{
	/* assume n_byte is equal or less than 4 */
	if (_n_byte_of_msg_len > _r_tail-_r_head)
		return -1;
	
	int msg_len = 0;
	char* p = (char*) & msg_len;

	for (int i = 0; i < _n_byte_of_msg_len; ++ i)
		p[i] = *(_read_queue+_r_head+i);

	return msg_len;
}

void Msg_buffer::set_msg_len(int msg_len)
{
	char* p = (char*) & msg_len;

	for (int i = 0; i < _n_byte_of_msg_len; ++ i)
		*(_write_queue+_w_tail+i) = p[i];
}

void Msg_buffer::reset_queue(char* q, int & head, int & tail)
{
	memmove(q, q+head, tail-head);
	tail = tail - head;
	head = 0;
}
