#ifndef __RING_QUEUE_H__
#define __RING_QUEUE_H__

/*
* ring queue
* for one producer and one consumer
*/

#include	<pthread.h>

#include	<string.h>

class Ring_queue
{

public:

	Ring_queue(const int queue_size, const int entry_size);
	~Ring_queue();

	void make_empty();

	/*
	*put a product into the queue
	*	the caller will be blocked if queue is full.
	*/
	void put(const char* product);

	/*
	*pop a product from the queue
	*	the caller will be blocked if queue is empty.
	*/
	void pop(char* product);

private:

	char** 	_queue;
	int 	_qsize;
	int   	_head;
	int   	_tail;

	/* condition variable,
		it represents the number of product in the queue */
	int 	_n_product;
	pthread_cond_t 	_cond_n_product;
	pthread_mutex_t	_mutex_n_product;

	/* condition variable,
		it represents the number of space in the queue */
	int 	_n_space;
	pthread_cond_t	_cond_n_space;
	pthread_mutex_t	_mutex_n_space;

};

#endif
