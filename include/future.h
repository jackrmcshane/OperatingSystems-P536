#define MAXE 100



typedef struct fqueue{
	pid32 queue[MAXE];
	int pcount;
	int head;
	int tail;
} fqueue;


#ifndef _FUTURE_H_
#define _FUTURE_H_

typedef enum {
	FUTURE_EMPTY,
	FUTURE_WAITING,
	FUTURE_READY
} future_state_t;


typedef enum {
	FUTURE_EXCLUSIVE,
	FUTURE_SHARED,
	FUTURE_QUEUE
} future_mode_t;


typedef struct future_t {
	char *data;
	uint size;
	future_state_t state;
	future_mode_t mode;
	pid32 pid;
	qid16 get_queue;
	//fqueue get_queue;
	// new for FUTURE_QUEUE
	qid16 set_queue;
	/* if count == max_elems, data queue is full */
	uint16 max_elems; // max size of data queue
	uint16 count; // track num elems in data queue
	/* elements are added at the tail, removed at the head */
	uint16 head; // for head of data queue
	uint16 tail; // for tail of data queue
} future_t;



/* Interface for the futures system calls */
future_t* future_alloc(future_mode_t mode, uint size, uint nelems);
syscall future_free(future_t*);
syscall future_get(future_t*, char*);
syscall future_set(future_t*, char*);

#endif /* _FUTURE_H_ */
