/* xsh_run.c - xsh_run */

#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <shprototypes.h>
#include <future.h>
#include <future_prodcons.h>
#include <future_fib.h>
#include <future_fq.h>
#include <stream.h>


void test();
void future_prodcons( int nargs, char *args[] );
void future_fibonacci( int nargs, char *args[] );
void future_queue( int nargs, char* args[] );


char * val;


shellcmd xsh_run(int nargs, char *args[]){

	if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0)){
		// print supported function calls *in alphabetical order*
		// could create an array of strings for the function calls
		printf ("futest\n");
		printf ("hello\n");
		printf ("list\n");
		printf ("prodcons\n");
		printf ("prodcons_bb\n");
		printf ("tscdf\n");
		printf ("tscdf_fq\n");
		return 0;
	}


	// iterating past the run command argument for simplicity's sake
	// incrementing pointer to arg array
	args++;
	nargs--;


	/* create the proper if statements for the supported arguments here */
	// i.e.
	if (strncmp(args[0], "hello", 6) == 0){
		if (hello_validate_args(nargs, args) != 0){
			printf("Syntax: run hello \[name\]\n");
			return 1;
		}

		resume(create((void *)xsh_hello, 4096, 20, "hello", 2, nargs, args));

	} else if (strncmp(args[0], "prodcons", 9) == 0){
		if (prodcons_validate_args(nargs, args) != 0){
			printf("Syntax: run prodcons \[counter\]\n");
			return 1;
		}

		resume(create((void *)xsh_prodcons, 4096, 20, "prodcons", 2, nargs, args));

	} else if(strcmp(args[0], "futest") == 0){

		char usage[] = "Syntax: run futest [-pc [g ...] [s VALUE ...]] | [-pcq LENGTH [g ...] [s VALUE ...]] | [-f NUMBER] | [--free]";

		if( strcmp(args[1], "-pc") == 0 ){
			future_prodcons(nargs, args);
		} else if( strcmp(args[1], "--free") == 0 ){
			if( nargs != 2 ){
				printf("%s\n", usage);
				return SYSERR;
			}
			future_free_test( nargs, args );
		} else if( strcmp(args[1], "-f") == 0 ){
			if( nargs != 3 ){
				printf("%s\n", usage);
				return SYSERR;
			}

			future_fib( nargs, args );

		} else if( strcmp(args[1], "-pcq") == 0 ) {
			// do something
			resume(create( future_queue, 4096, 20, "future_queue", 2, nargs, args ));

		} else {
			printf("%s\n", usage);
		}

	} else if( strcmp(args[0], "test") == 0 ) {
		test();
	} else if( strcmp(args[0], "tscdf") == 0 ){

		resume(create( stream_proc, 4096, 20, "stream_proc", 2, nargs, args ));

	} else if( strcmp(args[0], "tscdf_fq") == 0 ) {

		resume(create( stream_proc_futures, 4096, 20, "stream_proc_futures", 2, nargs, args ));

	} else {
		printf ("futest\n");
		printf ("hello\n");
		printf ("list\n");
		printf ("prodcons\n");
		printf ("prodcons_bb\n");
		printf ("tscdf\n");
		printf ("tscdf_fq\n");
	}

	return 0;
}




void future_prodcons( int nargs, char *args[] ){

	// do argument handling here
	if( nargs < 3 ){
		printf("Syntax: run futest [-pc [g ...] [s VALUE ...]|-f NUMBER][--free]\n");
		return SYSERR;
	}



	print_sem = semcreate( 1 );
	future_t *f_exclusive;
	f_exclusive = future_alloc( FUTURE_EXCLUSIVE, sizeof( int ), 1 );


	// try to iterate through the arguments and make sure they are all valid based on the requirements
	// you may assume that after the argument 's' there is always a number arg
	int i = 2;
	int convert;
	while( i < nargs ){
		//write your code here
		if( ( strcmp(args[i], "g") != 0 )  && ( strcmp(args[i], "s") != 0 ) && (convert = atoi(args[i]) == 0)/* args[i] is num */ ){
			printf("Syntax: run futest [-pc [g ...] [s VALUE ...]|-f NUMBER][--free]\n");
			return SYSERR;
		}
		i++;
	}


	int num_args = i; //keeping number of args for later creation of an array
	i = 2; // reset index
	val = (char *)getmem( num_args ); // initializing the array to keep the 's' numbers


	// Iterate again through the arguments and create the following processes based on the passed argument ('g' or 's VALUE')

	while( i < nargs ){
		if( strcmp(args[i], "g") == 0 ){
			char id[10];
			sprintf(id, "fcons%d", i);
			resume( create(future_cons, 2048, 20, id, 1, f_exclusive) );
		}

		if( strcmp(args[i], "s") == 0 ){
			i++;
			uint8 number = atoi( args[i] );
			val[i] = number;
			resume( create(future_prod, 2048, 20, "fprod1", 2, f_exclusive, &val[i]) );
			sleepms( 5 );
		}

		i++;
	}

	sleepms( 100 );
	future_free( f_exclusive );
}




void future_queue( int nargs, char* args[] ){

	// test args
	char usage[] = "Syntax: run futest [-pc [g ...] [s VALUE ...]] | [-pcq LENGTH [g ...] [s VALUE ...]] | [-f NUMBER] | [--free]";
	if( nargs < 6 ){
		printf("%s\n", usage);
		return SYSERR;
	}


	// try to iterate through the arguments and make sure they are all valid based on the requirements
	// you may assume that after the argument 's' there is always a number arg
	int i = 2;
	int convert;
	while( i < nargs ){
		//write your code here
		if( ( strcmp(args[i], "g") != 0 )  && ( strcmp(args[i], "s") != 0 ) && (convert = atoi(args[i]) == 0)/* args[i] is num */ ){
			printf("%s\n", usage);
			return SYSERR;
		}
		i++;
	}


	/* args validated */

	// create future in FUTURE_QUEUE mode
	future_t* f;
	print_sem = semcreate( 1 );
	int nelems = atoi( args[2] );
	int future_flag = FUTURE_QUEUE;
	if( (f = future_alloc(future_flag, sizeof(char*), nelems)) == (future_t *)SYSERR ){
		printf("future_alloc failed.\n");
		return SYSERR;
	}



	// read passed args and create consumers/producers
	// Iterate again through the arguments and create the following processes based on the passed argument ('g' or 's VALUE')
	int num_args = i; //keeping number of args for later creation of an array
	i = 3; // reset index
	val = (char *)getmem( num_args ); // initializing the array to keep the 's' numbers

	while( i < nargs ){
		if( strcmp(args[i], "g") == 0 ){
			//printf("g\n");
			char id[10];
			sprintf(id, "fcons%d", i);
			resume( create(future_cons, 2048, 20, id, 1, f) );
		}

		if( strcmp(args[i], "s") == 0 ){
			//printf("s ");
			i++;
			uint8 number = atoi( args[i] );
			val[i] = number;
			//printf("%d\n", number);
			resume( create(future_prod, 2048, 20, "fprod1", 2, f, &val[i]) );
			sleepms( 5 );
		}

		i++;
	}



	// free the future
	sleepms( 100 );
	future_free( f );

}



void test(){

	qid16 myq = newqueue();
	enqueue(getpid(), myq);
	dequeue( myq );

}
