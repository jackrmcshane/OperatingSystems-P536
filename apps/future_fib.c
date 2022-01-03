#include <xinu.h>
#include <stdlib.h>
#include <future.h>



int zero = 0;
int one = 1;
int two = 2;
future_t **fibfut;


int ffib( int n ){
	int minus1 = 0, minus2 = 0, this = 0;
	// declare zero and one

	// if want F0
	if( n == 0 ){
		future_set( fibfut[0], (char *)&zero );
		return OK;
	}


	// if want F1
	if( n == 1 ){
		future_set( fibfut[1], (char *)&one );
		return OK;
	}


	// get val for Fn-2
	int status = ( int )future_get( fibfut[n - 2], (char *)&minus2 );
	if( status < 1 ){
		printf("future_get failed\n");
		return -1;
	}

	// get val for Fn-1
	status = ( int )future_get( fibfut[n - 1], (char *)&minus1 );
	if( status < 1 ){
		printf("future_get failed\n");
		return -1;
	}

	printf( "the value of sum: %d\n", (minus2 + minus1) );
	this = minus1 + minus2;
	printf( "written to this: %d\n", this );
	future_set( fibfut[n], (char *)&this );

	return 0;
}/* args */


int future_fib( int nargs, char *args[] ){
	int fib = -1, i;

	// fibonacci num to calc
	fib = atoi( args[2] );
	if( fib > -1 ){
		int final_fib = 0;
		int future_flags = FUTURE_SHARED;

		//creating array of future pointers
		if((fibfut = (future_t **)getmem( sizeof(future_t *) * (fib + 1) )) == (future_t **)SYSERR)
		{
			printf( "getmem failed\n" );
			return SYSERR;
		}



		// get futures for the future array
		for( i = 0; i <= fib; i++ ){
			if((fibfut[i] = future_alloc(future_flags, sizeof(int), 1)) == (future_t *)SYSERR){
				printf( "future_alloc failed\n" );
				return SYSERR;
			}
		}


		// spawn the fibonacci threads and get the final value
		for( i = 0; i <= fib; i++ ){
			char id[10];
			sprintf(id, "ffib%d", i);
			resume( create( ffib, 2048, 20, id, 1, i ) );
		}


		//for( i = 0; i <fib; i++ ){
			//future_get( fibfut[i], (char *)&final_fib );
			//printf("Nth Fibonacci value for N=%d is %d\n", i, final_fib);
		//}

		// get the final fib number from the array of future pointers
		future_get( fibfut[fib], (char *)&final_fib );

		// free up the mem held by each of the elems of the array
		// and the array itself
		for( i = 0; i <= fib; i++ ){ future_free( fibfut[i] ); }
		freemem( (char *)fibfut, sizeof(future_t *) * (fib + 1) );

		printf("Nth Fibonacci value for N=%d is %d\n", fib, final_fib);
		return OK;
	}

	// otherwise
	return SYSERR;
}




int future_free_test( int nargs, char *args[] ){

	future_t *f_exclusive;
	f_exclusive = future_alloc( FUTURE_EXCLUSIVE, sizeof( int ), 1 );
	printf("future exclusive created\n");
	future_free( f_exclusive );
	printf("future exclusive freed\n");

	future_t *f_shared;
	f_shared = future_alloc(FUTURE_SHARED, sizeof( int ), 1);
	printf("future shared created\n");
	future_free( f_shared );
	printf("future shared freed\n");

	return OK;
}
