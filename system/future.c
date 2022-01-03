#include <xinu.h>
#include <stdlib.h>
#include <future.h>



future_t * future_alloc( future_mode_t mode, uint size, uint nelems ){

	intmask mask;
	mask = disable();

	future_t * fut;

	/* FUTURE_QUEUE */
	if( mode == FUTURE_QUEUE ){

		fut = (future_t *)getmem(sizeof( future_t ));
		if( fut == (future_t *)SYSERR ){ printf("future_alloc failed\n"); }

		fut->data = (char *)getmem( nelems * size );
		if( fut->data == (char *)SYSERR ){ printf("future_alloc failed\n"); }
		fut->size = size;
		fut->state = FUTURE_EMPTY;
		fut->mode = FUTURE_QUEUE;
		fut->get_queue = newqueue();
		fut->set_queue = newqueue();
		fut->max_elems = nelems;
		fut->count = 0;
		fut->head = 0;
		fut->tail = 0;

	/* FUTURE_SHARED */
	} else if( mode == FUTURE_SHARED ){

		fut = ( future_t * )getmem( sizeof( future_t ) );
		( *fut ).data = (char *)getmem( size );
		( *fut ).size = size;
		( *fut ).state = FUTURE_EMPTY;
		// get_queue already initialized with zeros
		( *fut ).get_queue = newqueue();
		( *fut ).mode = mode;
		// don't need nelems for now, just pass 1 as value and do nothing

	/* FUTURE_EXCLUSIVE */
	} else{
		// write your own code here
		fut = ( future_t * )getmem( sizeof( future_t ) );
		( *fut ).data = (char *)getmem( size );
		( *fut ).size = size;
		( *fut ).state = FUTURE_EMPTY;
		// get_queue already initialized with zeros
		( *fut ).get_queue = newqueue();
		( *fut ).mode = mode;
		// don't need nelems for now, just pass 1 as value and do nothing
	}


	restore( mask );
	return fut;
}


// might have to disable interrupts here as above
syscall future_free( future_t* f ){
	intmask mask;
	mask = disable();
	// kill any processes that are waiting on f
	kill( ( *f ).pid );


	/* FUTURE_SHARED */
	//if( ( *f ).mode == FUTURE_SHARED ){
		//// deallocate fqueue
		//int status = freemem( (char *)&(*f).get_queue, sizeof( fqueue ) );
		//if( status != OK ){
			//return SYSERR;
		//}
	//}
	/* FUTURE_SHARED */


	restore( mask );
	// free the memory held by f, this will return either SYSERR or OK
	return freemem((char*)f, sizeof(future_t));
}


syscall future_get( future_t* f, char* out ){
	intmask mask;
	mask = disable();


	//memcopy call for the copying of data
	/* FUTURE_QUEUE */
	if( ( *f ).mode == FUTURE_QUEUE ){

		if( f->count == 0 ){

			pid32 pid = getpid();
			enqueue( pid, (*f).get_queue );
			suspend( pid );
  			char* headelemptr = f->data + (f->head * f->size);
			memcpy( out, headelemptr, f->size );
			f->head = (f->head + 1) % f->max_elems;
			f->count--;

		} else { // there are elements in the data_queue

			// get val from data qeueue
  			char* headelemptr = f->data + (f->head * f->size);
			//memcpy( out, f->data[f->head], f->size );
			memcpy( out, headelemptr, f->size );
			f->head = (f->head + 1) % f->max_elems;
			f->count--;

			// resume proc from set queue
			if(nonempty( f->set_queue )){
				ready(dequeue( f->set_queue ));
			}
		}

	/* FUTURE_SHARED */
	} else if( ( *f ).mode == FUTURE_SHARED ){

		if( ( *f ).state == FUTURE_EMPTY ){

			( *f ).state = FUTURE_WAITING;
			pid32 pid = getpid();
			enqueue( pid, (*f).get_queue );
			suspend( pid );
			memcpy( out, (*f).data, sizeof(int) );
			//*out = *( ( *f ).data );

			//fqenqueue( &( *f ).get_queue, pid );
			//suspend(pid);
			//while( ( *f ).state != FUTURE_READY ){
				//sleepms( 5 );
			//}
			//sleep(10);

		} else if( ( *f ).state == FUTURE_WAITING ){

			pid32 pid = getpid();
			enqueue( pid, (*f).get_queue );
			suspend( pid );
			memcpy( out, (*f).data, sizeof(int) );
			//*out = *( ( *f ).data );

			//fqenqueue( &( *f ).get_queue, pid );
			//suspend(pid);
			//while( ( *f ).state != FUTURE_READY ){
				//sleepms( 5 );
			//}
			//sleep(10);

		} else {
			//*out = *( ( *f ).data );
			memcpy( out, (*f).data, sizeof(int) );
		}

	/* FUTURE_EXCLUSIVE */
	} else {

		// check state of f (whether it is ready or not)
			// if there is a process already waiting on it, return SYSERR
		if( ( *f ).state == FUTURE_WAITING ){
			restore( mask );
			return SYSERR;
		} else if( ( *f ).state == FUTURE_READY ){
			// get value of future f (if set), assign to var out
			//*out = *( ( *f ).data );
			memcpy( out, (*f).data, sizeof(int) );
			// change the state of f accordingly
			( *f ).state = FUTURE_EMPTY;
		} else {
			// set state to FUTURE_WAITING
			( *f ).state = FUTURE_WAITING;
			// set the pid
			( *f ).pid = getpid();

			// wait on future value
			while( ( *f ).state != FUTURE_READY ){
				sleepms( 5 );
			}

			//*out = *( ( *f ).data );
			memcpy( out, (*f).data, sizeof(int) );
			( *f ).state = FUTURE_EMPTY;
		}
	}

	restore( mask );
	return OK;
}



syscall future_set( future_t* f, char* in ){
	intmask mask;
	mask = disable();

	/* FUTURE_QUEUE */
	if( ( *f ).mode == FUTURE_QUEUE ){

		if( f->count == f->max_elems ){
			// queue the proc in set_queue
			pid32 pid = getpid();
			enqueue( pid, f->set_queue );
			suspend( pid );
			// do the thing
  			char* tailelemptr = f->data + (f->tail * f->size);
			f->count++;
			//memcpy( f->data[f->tail], in, f->size );
			memcpy( tailelemptr, in, f->size );
			f->tail = (f->tail + 1) % f->max_elems;

		} else {
			// set value (add to data queue)
  			char* tailelemptr = f->data + (f->tail * f->size);
			f->count++;
			//memcpy( f->data[f->tail], in, f->size );
			memcpy( tailelemptr, in, f->size );
			f->tail = (f->tail + 1) % f->max_elems;

			// dequeue proc from the get_queue
				// do i need to do a check on the queue
			if(nonempty( f->get_queue )){
				ready(dequeue( f->get_queue ));
			}
		}

	/* FUTURE_SHARED */
	} else if( ( *f ).mode == FUTURE_SHARED ){

		if( (*f).state == FUTURE_READY ){
			return SYSERR;
		} else if( ( *f ).state == FUTURE_EMPTY ){

			//*( ( *f ).data ) = *in;
			memcpy( (*f).data, in, sizeof(int) );
			( *f ).state = FUTURE_READY;

		} else { // FUTURE_WAITING

			//*( ( *f ).data ) = *in;
			memcpy( (*f).data, in, sizeof(int) );
			( *f ).state = FUTURE_READY;
			//resume all proc in the get_queue
			while( nonempty( (*f).get_queue ) ){
				ready( dequeue( (*f).get_queue ) );
			}
			//while( !isEmpty(( *f ).get_queue) ){
				// maybe use unsleep() call instead?
				//resume( fqdequeue(( *f ).get_queue) );
			//}
		}

	/* FUTURE_EXCLUSIVE */
	} else {
		// check the state of the future
			// return SYSERR if appropriate
		if( ( *f ).state == FUTURE_READY ){
			restore( mask );
			return SYSERR;
		} else if( ( *f ).state == FUTURE_EMPTY ){
			//*( ( *f ).data ) = *in;
			memcpy( (*f).data, in, sizeof(int) );
			( *f ).state = FUTURE_READY;
		} else { // proc is waiting on future
			//*( ( *f ).data ) = *in;
			memcpy( (*f).data, in, sizeof(int) );
			( *f ).state = FUTURE_READY;
		}
	}

	restore( mask );
	return OK;
}
