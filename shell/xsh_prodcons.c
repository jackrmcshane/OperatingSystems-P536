// author: jack mcshane
// CSCI-P556 Advanced Operating Systems


#include <xinu.h>
#include <prodcons.h>


int prodcons_validate_args(int nargs, char * args[]);


/* creates n as global var such that producer and consumer can see it on the Heap */
int n = 0;

shellcmd xsh_prodcons (int nargs, char *args[]){

	can_read = semcreate(0);
	can_write = semcreate(1);

	// check that the arguments passed are valid: not too many/too few, etc
	// check if args[1] is present and assign value to count
	if (nargs > 2){
		fprintf(stderr, "%s: too many arguments\n", args[0]);
		fprintf(stderr, "Try '%s --help', for more information\n", args[0]);
		return 1;
	}

	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0){
		printf("\nusage: %s [integer]\n\n", args[0]);
		printf("description:\n");
		printf("\tCreates producer and consumer processes which use [integer] as the initial counter.\n");
		printf("\tThe default value of the counter is set to 2000.\n");
		printf("options:\n");
		printf("\t--help\tdisplay this help and exit\n");
		return 0;
	}


	/* local declarations */
	int count = 200;

	if (nargs == 2){ count = atoi(args[1]); }


	// create producer and consumer processes and put into ready queue
	// defs of create() and resume() functions are in the system folder
	resume( create( producer, 1024, 20, "producer", 1, count ) );
	resume( create( consumer, 1024, 20, "consumer", 1, count ) );
	return 0;

}



int prodcons_validate_args(int nargs, char * args[]){

	if (nargs > 2){ return 1; }
	return 0;
}
