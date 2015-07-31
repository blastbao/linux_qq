#include	"ring_queue.h"

Ring_queue::Ring_queue(const int queue_size, const int entry_size)
{
	/* init _queue and _qsize */
	_qsize = queue_size;

	_queue = new char*[_qsize];
	for (int i = 0; i < _qsize; ++ i)
		_queue[i] = new char[entry_size];

	/* init _head and _tail */
	_head = _tail = 0;

	/* init condition variables */
	_n_product = 0;
	_n_space   = _qsize;

	pthread_cond_init(& _cond_n_product, NULL);
	pthread_cond_init(& _cond_n_space, NULL);

	pthread_mutex_init(& _mutex_n_product, NULL);
	pthread_mutex_init(& _mutex_n_space, NULL);
}

Ring_queue::~Ring_queue()
{
	/* destroy _queue */
	for (int i = 0; i < _qsize; ++ i)
		delete [] _queue[i];
	delete [] _queue;

	/* destroy condition variables */
	pthread_cond_destroy(& _cond_n_product);
	pthread_cond_destroy(& _cond_n_space);

	pthread_mutex_destroy(& _mutex_n_product);
	pthread_mutex_destroy(& _mutex_n_space);
}

void Ring_queue::make_empty()
{
	_head = _tail = 0;
}

void Ring_queue::put(const char* product)
{
	pthread_mutex_lock(& _mutex_n_space);
	while (_n_space == 0)
		pthread_cond_wait(& _cond_n_space, & _mutex_n_space);
	-- _n_space;
	pthread_mutex_unlock(& _mutex_n_space);

	strcpy(_queue[_tail], product);
	_tail = (_tail + 1) % _qsize;

	pthread_mutex_lock(& _mutex_n_product);
	if (_n_product == 0)
		pthread_cond_signal(& _cond_n_product);
	++ _n_product;
	pthread_mutex_unlock(& _mutex_n_product);
}

void Ring_queue::pop(char* product)
{
	pthread_mutex_lock(& _mutex_n_product);
	while (_n_product == 0)
		pthread_cond_wait(& _cond_n_product, & _mutex_n_product);
	-- _n_product;
	pthread_mutex_unlock(& _mutex_n_product);

	strcpy(product, _queue[_head]);
	_head = (_head + 1) % _qsize;

	pthread_mutex_lock(& _mutex_n_space);
	if (_n_space == 0)
		pthread_cond_signal(& _cond_n_space);
	++ _n_space;
	pthread_mutex_unlock(& _mutex_n_space);
}
