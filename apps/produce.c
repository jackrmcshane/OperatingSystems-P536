#include <xinu.h>
#include <prodcons.h>

void producer(int count) {
	for (int i = 0; i <= count; i++){
		wait(can_write);
		n = i;
		printf("produced : %d\n", n);
		signal(can_read);
	}
}
