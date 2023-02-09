/**
 * File              : utf8rtf.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 06.09.2021
 * Last Modified Date: 09.02.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef utf8rtf_h__
#define utf8rtf_h__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdint.h>

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

void 
_utf8_to_utf32(
		int count, 
		uint8_t *utf8_char, 
		void * user_data, 
		void (*callback)(void * user_data, uint32_t utf32_char))
{
	uint8_t *ptr = utf8_char; //pointer (do not change value)
	uint32_t utf32_char = 0;

	switch (count) {
		case 4: {
			//take last 3 bit from first char
			uint32_t byte0 = (*ptr++ & 0x7) << 18;   //0b00000111	
			
			//take last 6 bit from second char
			uint32_t byte1 = (*ptr++ & 0x3f) << 12;  //0b00111111	
			
			//take last 6 bit from third char
			uint32_t byte2 = (*ptr++ & 0x3f) << 6;   //0b00111111	
			
			//take last 6 bit from forth char
			uint32_t byte3 = *ptr++ & 0x3f;          //0b00111111	
			
			utf32_char = (byte0 | byte1 | byte2 | byte3);					

			break;
		} 

		case 3: {
			//take last 4 bit from first char
			uint32_t byte0 = (*ptr++ & 0xf) << 12;  //0b00001111	
			
			//take last 6 bit from second char
			uint32_t byte1 = (*ptr++ & 0x3f) << 6;  //0b00111111	
			
			//take last 6 bit from third char
			uint32_t byte2 = *ptr++ & 0x3f;         //0b00111111

			utf32_char = (byte0 | byte1 | byte2);

			break;
		} 

		case 2: {
			//take last 5 bit from first char
			uint32_t byte0 = (*ptr++ & 0x1f) << 6;  //0b00011111
			
			//take last 6 bit from second char
			uint32_t byte1 = *ptr++ & 0x3f;	        //0b00111111

			utf32_char = (byte0 | byte1);

			break;
		} 

		case 1: {
			utf32_char = *ptr++;

			break;
		} 				

		default:
			break;
				
	}
	if (callback)
		if(utf32_char)
			callback(user_data, utf32_char);
}

void
_utf32_to_utf8(
		uint32_t utf32_char,
		void * user_data,
		int (*callback)(void * user_data, uint8_t utf8_char)		
		)
{
	//more than 00000000 00000100 00000000 00000000
	if (utf32_char > 0x040000){ //4-byte
		uint8_t utf8_char;

		//get first byte - first 3 bit 00000000 00011100 00000000 00000000
		//and mask with 11110000 
		utf8_char = ((utf32_char & 0x1C0000) >> 18) | 0xF0;
		if (callback)
			if(callback(user_data, utf8_char))
				return;

		//get second - 00000000 00000011 11110000 00000000
		//and mask with 10000000 
		utf8_char = ((utf32_char & 0x03F000) >> 12) | 0x80;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
		
		//get third - 00000000 00000000 00001111 11000000
		//and mask with 10000000 
		utf8_char = ((utf32_char & 0x0FC0) >> 6 )   | 0x80;
		if (callback)
			if(callback(user_data, utf8_char))
				return;

		//get last - 00000000 00000000 00000000 00111111
		//and mask with 10000000 
		utf8_char = ( utf32_char & 0x3F)            | 0x80;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
	}
	//more than 00000000 00000000 00010000 00000000
	else if (utf32_char > 0x1000){ //3-byte
		uint8_t utf8_char;
		
		//get first byte - first 4 bit 00000000 00000000 11110000 00000000
		//and mask with 11100000 
		utf8_char = ((utf32_char & 0xF000) >> 12) | 0xE0;
		if (callback)
			if(callback(user_data, utf8_char))
				return;

		//get second - 00000000 00000000 00001111 11000000
		//and mask with 10000000 
		utf8_char = ((utf32_char & 0x0FC0) >> 6 ) | 0x80;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
		
		//get last - 00000000 00000000 00000000 00111111
		//and mask with 10000000 
		utf8_char = ( utf32_char & 0x3F)          | 0x80;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
	}
	//more than 000000000 00000000 00000000 1000000
	else if (utf32_char > 0x80){ //2-byte
		uint8_t utf8_char;
		//get first byte - first 5 bit 00000000 00000000 00000111 11000000
		//and mask with 11000000 
		utf8_char = ((utf32_char & 0x7C0)>> 6) | 0xC0;
		if (callback)
			if(callback(user_data, utf8_char))
				return;

		//get last - 00000000 00000000 00000000 00111111 
		//and mask with 10000000 
		utf8_char = ( utf32_char & 0x3F)       | 0x80;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
	}
	else { //ANSY
		uint8_t utf8_char = utf32_char;
		if (callback)
			if(callback(user_data, utf8_char))
				return;
	}
}

int _utf8rtf_parse_rtf_cb(void *user_data, uint8_t utf8_char){
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
			c=fgetc(fp); if (c==EOF) break;
			ch=(char)c;
			buf[l++] = ch; 
			if (ch == 'u'){
				/* this may be unicode, look next */	
				c=fgetc(fp); if (c==EOF) break;
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
					_utf32_to_utf8(u, NULL, _utf8rtf_parse_rtf_cb);					

					/* free buffer */
					for (i = 0; i < l; ++i)
						buf[i] = 0;	
					l = 0;					

					/* remove space after unicode */ 
					c=fgetc(fp); if (c==EOF) break;
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
	/* print \uXXXX code with space */
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

		ch = c;

		if (ch > 128 || ch < 0){ 
			/* this is utf8 */
			buf[l++] = ch; 
			/* get number of bytes in utf8 char */
			int count = 1;
			uint8_t u = ch;
			// 0b11110000
			if ((u & 0xf0) == 0xf0){ //4
				count = 4; 
				for (i = 0; i < 3; ++i) {
					c=fgetc(fp); if (c==EOF) break;
					ch=c;
					buf[l++] = ch; 
				}
			}
			// 0b11100000
			else if ((u & 0xe0) == 0xe0){ //3
				count = 3; 
				for (i = 0; i < 2; ++i) {
					c=fgetc(fp); if (c==EOF) break;
					ch=c;
					buf[l++] = ch; 
				}
			}
			// 0b11000000
			else if ((u & 0xc0) == 0xc0){ //2
				count = 2; 
				c=fgetc(fp); if (c==EOF) break;
				ch=c;
				buf[l++] = ch;
			}			
			else { //1
				/* just print char to stdout */	
				fprintf(stdout, "%c", ch);
				buf[0] = 0; l = 0;	
				continue;
			}

			/* parse buffer */
			_utf8_to_utf32(count, (uint8_t *)buf, NULL, _utf8rtf_parse_utf8_cb);
			
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
