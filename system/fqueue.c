#include <xinu.h>
#include <future.h>



bool isEmpty( fqueue* qptr ){ return (*qptr).pcount == 0; }
bool isFull( fqueue* qptr ){ return ((*qptr).tail + 1) == MAXE; }
pid32 peek( fqueue* qptr ){ return (*qptr).queue[(*qptr).head]; }
int size( fqueue* qptr ){ return (*qptr).pcount; }

void fqenqueue( fqueue* qptr, pid32 pid ){

	if( isFull(qptr) ){
		printf( "Queue is full!\n" );
		return;
	}

	(*qptr).queue[(*qptr).tail] = pid;
	(*qptr).pcount++;
	(*qptr).tail++;
}


pid32 fqdequeue( fqueue* qptr ){

	pid32 pid = (*qptr).queue[(*qptr).head];
	(*qptr).pcount--;
	(*qptr).head++;

	return pid;
}
