/**
 * File              : utf8rtf.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 06.09.2021
 * Last Modified Date: 08.02.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef utf8rtf_h__
#define utf8rtf_h__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdint.h>

#include "utf.h"

/*
 * convert rtf unicode to utf8 from stdin to stdout
 */
static int utf8rtf_decode();

/*
 * convert utf8 to rtf unicode from stdin to stdout
 */
static int utf8rtf_encode();

/*
 * IMP
 */
int _utf8rtf_uncode(void *user_data, uint8_t utf8_char){
	fprintf(stdout, "%c", utf8_char);
	return 0;
}

int _utf8rtf_parse_rtf(FILE *fp)
{
	char buf[BUFSIZ], ch;
	int i, l=0, c;

	while ((c=fgetc(fp))!=EOF){

		parse:
	
		/* buffer overload */
		if (l>BUFSIZ-1){
			perror("buffer overload");
			break;
		}

		ch = (char)c;

		if (ch == 0x5c){ // dash '\'
			buf[l++] = ch; 
			/* look next */
			c=fgetc(fp);
			if (c==EOF)
				break;
			ch=(char)c;
			buf[l++] = ch; 
			if (ch == 'u'){
				/* this may be unicode, look next */	
				c=fgetc(fp);
				if (c==EOF)
					break;
				ch=(char)c;
				if (ch >= '0' && ch <= '9'){
					/* this is UNICODE! */
					char unicode[5];
					unicode[0] = ch;
					unicode[1] = fgetc(fp);
					unicode[2] = fgetc(fp);
					unicode[3] = fgetc(fp);
					unicode[4] = 0;

					uint32_t u;
					sscanf(unicode, "%u", &u);			
					utf32_to_utf8(u, NULL, _utf8rtf_uncode);					

					/* free buffer */
					for (i = 0; i < l; ++i)
						buf[i] = 0;	
					l = 0;					

					/* remove space after unicode */ 
					c=fgetc(fp);
					if (c==EOF)
						break;
					ch=(char)c;
					if (ch != ' ')
						goto parse;

				}
				else {
					buf[l++] = ch; 
					/* print and free buffer */
					for (i = 0; i < l; ++i){
						fprintf(stdout, "%c", buf[i]);
						buf[i] = 0;	
					}
					l = 0;
				}
			}
			else {
				/* print and free buffer */
				for (i = 0; i < l; ++i){
					fprintf(stdout, "%c", buf[i]);
					buf[i] = 0;	
				}
				l = 0;
			}
		}
		else {
			/* print char */
			fprintf(stdout, "%c", ch);
		}
	}


	/* error handling */
	if (ferror(fp) != 0){
		perror("error reading file");
		return 1;
	}

	return 0;
}

int utf8rtf_decode(){
	return _utf8rtf_parse_rtf(stdin);
}

void 
_utf8rtf_parse_utf8_cb(void * user_data, uint32_t utf32_char){
	fprintf(stdout, "\\u%d ", utf32_char);
}

int _utf8rtf_parse_utf8(FILE *fp)
{
	char buf[BUFSIZ], ch;
	int i, l=0, c;

	while ((c=fgetc(fp))!=EOF){

		parse:
	
		/* buffer overload */
		if (l>BUFSIZ-1){
			perror("buffer overload");
			break;
		}

		ch = (char)c;

		if (ch < 0){ 
			/* this is utf8 */
			buf[l++] = ch; 
			/* get number of bytes in utf8 char */
			int count = 1;
			uint8_t u = ch;
			if ((u & 0b11110000) == 240){ //4
				count = 4; 
				for (i = 0; i < 3; ++i) {
					c=fgetc(fp);
					if (c==EOF)
						break;
					ch=(char)c;
					buf[l++] = ch; 
				}
			}
			else if ((u & 0b11100000) == 224){ //3
				count = 3; 
				for (i = 0; i < 2; ++i) {
					c=fgetc(fp);
					if (c==EOF)
						break;
					ch=(char)c;
					buf[l++] = ch; 
				}
			}
			else if ((u & 0b11000000) == 192){ //2
				count = 2; 
				c=fgetc(fp);
				if (c==EOF)
					break;
				ch=(char)c;
				buf[l++] = ch;

			}			
			else { //1
				/* just print char to stdout */	
				fprintf(stdout, "%c", ch);
				buf[0] = 0; l = 0;	
				continue;
			}

			/* parse buffer */
			utf8_to_utf32(count, (uint8_t *)buf, NULL, _utf8rtf_parse_utf8_cb);
			
			/* free buffer */
			for (i = 0; i < l; ++i)
				buf[i] = 0;	
			l = 0;
		}
		else {
			/* print char */
			fprintf(stdout, "%c", ch);
		}
	}

	/* error handling */
	if (ferror(fp) != 0){
		perror("error reading file");
		return 1;
	}

	return 0;
}


static int utf8rtf_encode(){
	return _utf8rtf_parse_utf8(stdin);
}

#ifdef __cplusplus
}
#endif

#endif //utf8rtf_h__
