/**
 * File              : main.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 08.02.2023
 * Last Modified Date: 08.02.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include <stdio.h>
#include <string.h>
#include "utf8rtf.h"

void print_usage(int argc, char *argv[]){
	fprintf(stderr, 
			"usage: %s [params]\n"
			"\n"
			"params:\n"
			"--decode\t\t convert from rtf unicode to utf8\n"
			"--encode\t\t convert from utf8 to rtf unicode\n"
			, argv[0]
	);
}

int main(int argc, char *argv[])
{
	if (argc > 1){
		if (strncmp(argv[1], "--decode", 9) == 0 )
			return utf8rtf_decode();
		else if (strncmp(argv[1], "--encode", 9) == 0 )
			return utf8rtf_encode();
	}
	
	print_usage(argc, argv);
	
	return 0;
}
