/* xsh_hello.c - xsh_hello */

/*------------------------------------------------------------------------
 * author: jack mcshane
 * A1: hello shell command
 * Due: january 28, 2021
 *------------------------------------------------------------------------
 */

#include <xinu.h>
#include <string.h>
#include <stdio.h>

int hello_validate_args(int nargs, char * args[]);


/*------------------------------------------------------------------------------
 * xsh_hello - takes an input string and prints a greeting utilizing the string
 *------------------------------------------------------------------------------
 */
shellcmd xsh_hello(int nargs, char * args[]){
	/* declarations */
	// not sure ill actually need any

	// info for --help argument
	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0){
		printf("\nusage: %s [string_argument]\n\n", args[0]);
		printf("description:\n");
		printf("\tprints a greeting to the given string\n");
		printf("options:\n");
		printf("\t--help\tdisplay this help and exit\n");
		return 0;
	}


	int valid_args = hello_validate_args(nargs, args);
	if (nargs > 2){
		fprintf(stderr, "%s: too many arguments\n", args[0]);
		fprintf(stderr, "Try '%s --help', for more information\n", args[0]);
		return 1;
	}


	if (nargs < 2){
		fprintf(stderr, "%s: too few arguments\n", args[0]);
		fprintf(stderr, "Try '%s --help', for more information\n", args[0]);
		return 1;
	}


	printf("Hello %s, Welcome to the world of Xinu!!\n", args[1]);
	return 0;
}



int hello_validate_args(int nargs, char * args[]){

	if (nargs < 2){ return 1; }
	if (nargs > 2){ return 2; }
	return 0;
}
