// sets up the producer and consumer processes for lab7

#include <xinu.h>
#include <stdlib.h>
#include <stream.h>
#include <../apps/tscdf.h>
//#include <../apps/tscdf_input.h>


uint pcport;
extern int work_queue_depth = 0;
extern int time_window = 0;
extern int output_time = 0;

void
stream_consumer(int32 id, struct stream* str);


int
stream_proc( int nargs, char * args[] ){


	// check args and parse arguments
	int i, num_streams;
	work_queue_depth = 0;
	time_window = 0;
	output_time = 0;
	char usage[] = "Usage: run tscdf -s num_streams -w work_queue_depth -t time_window -o output_time\n";

    	if (nargs != 9){
        	kprintf("%s", usage);
        	return (-1);
    	}
    	else{
        	i = nargs - 1;
        	while (i > 0){
            		char* ch = args[i - 1];
            		char c = *(++ch);

            		switch (c){
            			case 's':
                			num_streams = atoi(args[i]);
                			break;

            			case 'w':
                			work_queue_depth = atoi(args[i]);
                			break;

            			case 't':
                			time_window = atoi(args[i]);
                			break;

            			case 'o':
                			output_time = atoi(args[i]);
                			break;

            			default:
                			kprintf("%s", usage);
                			return (-1);
            		}

            		i -= 2;
        	}
	}



	// tracking runtime
	ulong secs, msecs, time;
	secs = clktime;
	msecs = clkticks;

	if((pcport = ptcreate(num_streams)) == SYSERR){
		kprintf("ptcreate failed\n");
		return SYSERR;
	}




	/* actual phase 1 code */

	// initializing multiple streams
	struct stream* streams[num_streams];
	//streams = (struct stream **)getmem( num_streams * sizeof(struct stream *) );
	for( i = 0; i < num_streams; i++ ){
		// allocate the mem for the stream pointers
		streams[i] = (struct stream *)getmem(sizeof( struct stream ));
		// initialize stream elements
		streams[i]->mutex = semcreate(1);
		streams[i]->spaces = semcreate(work_queue_depth);
		streams[i]->items = semcreate(0);
		streams[i]->qsize = work_queue_depth;
		streams[i]->head = 0;
		streams[i]->tail = 0;
		// allocate space for the de pointers in the queue
		streams[i]->queue = (de **)getmem( work_queue_depth * sizeof(de *) );
		for( int j = 0; j < work_queue_depth; j++ ){
			// get mem for the data elements
			*(streams[i]->queue + j) = (de *)getmem(sizeof( de ));
		}
	}



	// create consumer procs
	for( i = 0; i < num_streams; i++ ){
		char name[10];
		sprintf(name, "cons%d", i);
		resume(create( stream_consumer, 4096, 20, name, 2, i, streams[i] ));
	}



	// parsing producer inputs
	char* a;
	int st, ts, v;
	for( i = 0; i < n_input; i++ ){
		a = (char *)stream_input[i];
		st = atoi(a);
		while(*a++ != '\t');
		ts = atoi(a);
		while(*a++ != '\t');
		v = atoi(a);

		// wait until there are spaces
		// once there are spaces, try to get the lock
		// these statements don't have to be checked in conjunction because the consumer proc will never take away spaces
		wait( streams[st]->spaces );
		wait( streams[st]->mutex );

		/* critical section */
		// write to the head of the queue
		(**(streams[st]->queue + streams[st]->head)).time = ts;
		(**(streams[st]->queue + streams[st]->head)).value = v;

		// update head
		streams[st]->head = (streams[st]->head + 1) % streams[st]->qsize;

		// indicate there is a new item in the queue
		// release the lock
		signal( streams[st]->items );
		signal( streams[st]->mutex );
	}


	/************** your code for phase 1 ends here **************/


	for( i = 0; i < num_streams; i++ ){
		uint32 pm;
		pm = ptrecv(pcport);
		kprintf("process %d exited\n", pm);
	}


	ptdelete(pcport, 0);


	time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
	kprintf("time in ms: %u\n", time);


	return 0;
}





void
stream_consumer(int32 id, struct stream* str){

	kprintf("stream_consumer id:%d (pid:%d)\n", id, getpid());
	struct tscdf* tp = tscdf_init( time_window );
	if( tp == (struct tscdf*)SYSERR ){
		kprintf("The allocation failed");
	}

	int i = 1;
	while(1){


		// wait until there is an item to read
		wait( str->items );
		// once there is an item to read, try to get lock
		wait( str->mutex );

		/* critical section */
		// have to check if value is 0 too, probably a test from the autograder
		if( ((**(str->queue + str->tail)).time == 0) && ((**(str->queue + str->tail)).value == 0)){
			break;
		}

		// signal that there is a new space in the queue
		signal( str->spaces );
		// release the lock
		signal( str->mutex );


		// tscdf stuff
		int status = tscdf_update(tp, (**(str->queue + str->tail)).time, (**(str->queue + str->tail)).value);
		(*str).tail = ((*str).tail + 1) % (*str).qsize;

		if( i == output_time ){
			int32* qarray = tscdf_quartiles( tp );
			if( qarray == NULL ){
				kprintf("tscdf_quartiles returned NULL\n");
				continue;
			} else {

				char output[50];
				sprintf(output, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
				kprintf("%s\n", output);
				freemem((char *)qarray, (6*sizeof(int32)));
			}
			i = 0;
		}


		i++;
	}


	signal( str->mutex );
	kprintf("stream_consumer exiting\n");
	ptsend(pcport, getpid());
}
