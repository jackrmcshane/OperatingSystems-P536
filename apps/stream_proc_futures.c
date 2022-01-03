#include <xinu.h>
#include <future.h>
#include <stream.h>
#include <../apps/tscdf.h>


uint pcport;
extern int wqd = 0;
extern int tw = 0;
extern int ot = 0;
extern int ns = 0;

void stream_consumer_future(int32 id, future_t *f);


int stream_proc_futures(int nargs, char* args[]) {

  	char usage[] = "Usage: run tscdf_fq -s num_streams -w work_queue_depth -t time_window -o output_time\n";

  	// Timing: Tick
	ulong secs, msecs, time;
	secs = clktime;
	msecs = clkticks;

  	// Parse arguments
	int i;
  	wqd = 0;
	tw = 0;
	ot = 0;
	ns = 0;

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
                			ns = atoi(args[i]);
                			break;

            			case 'w':
                			wqd = atoi(args[i]);
                			break;

            			case 't':
                			tw = atoi(args[i]);
                			break;

            			case 'o':
                			ot = atoi(args[i]);
                			break;

            			default:
                			kprintf("%s", usage);
                			return (-1);
            		}

            		i -= 2;
        	}
	}

	//kprintf("ns: %d\n", ns);
	//kprintf("wqd: %d\n", wqd);
	//kprintf("tw: %d\n", tw);
	//kprintf("ot: %d\n", ot);

  	// Create port that allows `num_streams` outstanding messages
	if((pcport = ptcreate(ns)) == SYSERR){
		kprintf("ptcreate failed\n");
		return SYSERR;
	}

  	// Create array to hold `num_streams` futures
  	future_t **futures;
	//futures = (future_t **)getmem(ns * sizeof(future_t *));
	if((futures = (future_t **)getmem(ns *sizeof(future_t *))) == (future_t **)SYSERR){
		printf("getmem failed for future pointer array\n");
	}


  	/* Allocate futures and create consumer processes
   	* Use `i` as the stream id.
   	* Future mode        = FUTURE_QUEUE
   	* Size of element    = sizeof(struct data_element)
   	* Number of elements = work_queue_depth
   	*/
	int future_mode = FUTURE_QUEUE;
  	for (i = 0; i < ns; i++) {
		futures[i] = future_alloc(future_mode, sizeof(struct data_element), wqd);

		char id[10];
		sprintf(id, "cons%d", i);
		resume(create( stream_consumer_future, 4096, 20, id, 2, i, futures[i] ));
  	}

  	// Parse input header file data and set future values
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

		// new rendition
		struct data_element* de;
		de = (struct data_element *)getmem(sizeof(struct data_element));
		if(de == (struct data_element *)SYSERR){
			kprintf("The allocation failed\n");
		}
		de->time = ts;
		de->value = v;
		future_set( futures[st], (char *)de );
	}


  	// Wait for all consumers to exit
	for( i = 0; i < ns; i++ ){
		uint32 pm;
		pm = ptrecv(pcport);
		kprintf("process %d exited\n", pm);
	}

	ptdelete(pcport, 0);


  	// Free all futures
	for( i = 0; i < ns; i++ ){
		future_free( futures[i] );
	}


  	// Timing: Tock+Report
	time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
	kprintf("time in ms: %u\n", time);

  	return 0;
}



void stream_consumer_future(int32 id, future_t *f) {

	kprintf("stream_consumer_future id:%d (pid:%d)\n", id, getpid());
  	// Initialize tscdf time window
	struct tscdf* tp = tscdf_init( tw );
	if( tp == (struct tscdf*)SYSERR ){ kprintf("The allocation failed\n"); }

  	// Loop until exit condition reached
	int i = 1;
	while( 1 ){

  		//   Get future values
		char* out;
		struct data_element* de;
		future_get( f, out );
		de = (struct data_element*)out;
		//printf("time %d : value %d\n", de->time, de->value);
		if((de->time == 0) && (de->value == 0)){
			break;
		}

  		//   Update tscdf time window
		tscdf_update(tp, de->time, de->value);
  		//   Calculate and print tscdf quartiles
		if( i == ot ){
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



  	// Free tscdf time window
	tscdf_free( tp );

  	// Signal producer and exit
	kprintf("stream_consumer_future exiting\n");
	ptsend(pcport, getpid());

  	return;
}
