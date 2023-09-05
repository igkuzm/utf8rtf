/**
 * File              : image2rtf.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 05.09.2023
 * Last Modified Date: 05.09.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include <stdio.h>
#include <string.h>
#include "image2rtf.h"

void print_usage(int argc, char *argv[]){
	fprintf(stderr, 
			"usage: %s filename\n"
			"\n"
			, argv[0]
	);
}

int main(int argc, char *argv[])
{
	if (argc > 1){
		unsigned char *str = image2hex(argv[1]);
		if (!str)
			return -1;
		puts("{\\pict\\picw0\\pich0\\picwgoal10254\\pichgoal6000\\jpegblip\n");
		puts((char *)str);
		puts("}\n");
		free(str);
		return 0;
	}
	
	print_usage(argc, argv);
	
	return 0;
}
