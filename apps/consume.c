#include <xinu.h>
#include <prodcons.h>

void consumer(int count) {
	for( int i = 0; i <= count; i++){
		wait(can_read);
		printf("consumed : %d\n", n);
		signal(can_write);
	}

}
