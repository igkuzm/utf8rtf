/**
 * File              : utf8rtf.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 06.09.2021
 * Last Modified Date: 14.01.2024
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

/* convert utf32 char to utf8 multybite char array and return number of bytes */ 
static int _utf8rtf_c32tomb(char s[6], const uint32_t c32){
	int i = 0;
	if (c32 <= 0x7F) {
		// Plain single-byte ASCII.
		s[i++] = (char) c32;
	}
	else if (c32 <= 0x7FF) {
		// Two bytes.
		s[i++] = 0xC0 |  (c32 >> 6);
		s[i++] = 0x80 | ((c32 >> 0) & 0x3F);
	}
	else if (c32 <= 0xFFFF) {
		// Three bytes.
		s[i++] = 0xE0 |  (c32 >> 12);
		s[i++] = 0x80 | ((c32 >> 6) & 0x3F);
		s[i++] = 0x80 | ((c32 >> 0) & 0x3F);
	}
	else if (c32 <= 0x1FFFFF) {
		// Four bytes.
		s[i++] = 0xF0 |  (c32 >> 18);
		s[i++] = 0x80 | ((c32 >> 12) & 0x3F);
		s[i++] = 0x80 | ((c32 >> 6)  & 0x3F);
		s[i++] = 0x80 | ((c32 >> 0)  & 0x3F);
	}
	else if (c32 <= 0x3FFFFFF) {
		// Five bytes.
		s[i++] = 0xF8 |  (c32 >> 24);
		s[i++] = 0x80 | ((c32 >> 18) & 0x3F);
		s[i++] = 0x80 | ((c32 >> 12) & 0x3F);
		s[i++] = 0x80 | ((c32 >> 6)  & 0x3F);
		s[i++] = 0x80 | ((c32 >> 0)  & 0x3F);
	}
	else if (c32 <= 0x7FFFFFFF) {
		// Six bytes.
		s[i++] = 0xFC |  (c32 >> 30);
		s[i++] = 0x80 | ((c32 >> 24) & 0x3F);
		s[i++] = 0x80 | ((c32 >> 18) & 0x3F);
		s[i++] = 0x80 | ((c32 >> 12) & 0x3F);
		s[i++] = 0x80 | ((c32 >> 6)  & 0x3F);
		s[i++] = 0x80 | ((c32 >> 0)  & 0x3F);
	}
	else{
		// Invalid char; don't encode anything.
	}	

	return i;
}

/* convert utf8 multybite null-terminated string to utf32 null-terminated string 
 * and return it's len */ 
static size_t _utf8rtf_mbtoc32(uint32_t *s32, const char *s){
	char *ptr = (char *)s;
	size_t i = 0;
	while (*ptr){
		uint8_t c = *ptr;
		if (c >= 252){/* 6-bytes */
			s32[i]  = (*ptr++ & 0x1)  << 30;  // 0b00000001
			s32[i] |= (*ptr++ & 0x3F) << 24;  // 0b00111111	
			s32[i] |= (*ptr++ & 0x3F) << 18;  // 0b00111111
			s32[i] |= (*ptr++ & 0x3F) << 12;  // 0b00111111
			s32[i] |= (*ptr++ & 0x3F) << 6;   // 0b00111111
			s32[i] |=  *ptr++ & 0x3F;         // 0b00111111
			i++;
		} 
		else if (c >= 248){/* 5-bytes */
			s32[i]  = (*ptr++ & 0x3)  << 24;  // 0b00000011
			s32[i] |= (*ptr++ & 0x3F) << 18;  // 0b00111111
			s32[i] |= (*ptr++ & 0x3F) << 12;  // 0b00111111
			s32[i] |= (*ptr++ & 0x3F) << 6;   // 0b00111111
			s32[i] |=  *ptr++ & 0x3F;         // 0b00111111
			i++;
		}
		else if (c >= 240){/* 4-bytes */
			s32[i]  = (*ptr++ & 0x7)  << 18;  // 0b00000111
			s32[i] |= (*ptr++ & 0x3F) << 12;  // 0b00111111
			s32[i] |= (*ptr++ & 0x3F) << 6;   // 0b00111111
			s32[i] |=  *ptr++ & 0x3F;         // 0b00111111
			i++;
		} 
		else if (c >= 224){/* 3-bytes */
			s32[i]  = (*ptr++ & 0xF)  << 12;  // 0b00001111
			s32[i] |= (*ptr++ & 0x3F) << 6;   // 0b00111111
			s32[i] |=  *ptr++ & 0x3F;         // 0b00111111
			i++;                
		}
		else if (c >= 192){/* 2-bytes */
			s32[i]  = (*ptr++ & 0x1F) << 6;   // 0b00011111
			s32[i] |=  *ptr++ & 0x3F;         // 0b00111111 
			i++; 
		} 
		else{/* 1-byte */
			s32[i++] = *ptr++;
		} 
	}

	// null-terminate string
	s32[i] = 0;
	return i;
}	

static int _utf8rtf_isinword(int ch)
{
	if ( 	
		ch == '\n' ||
		ch == '\r' ||
		ch == ' '  ||
		ch == '\t' ||
		ch == '}'  ||
		ch == '{'  ||
		ch == '\\'
		)
		return 0;
	return 1;
}

static int _utf8rtf_readword(FILE *fp, char *buf)
{
	int ch = 1;
	int blen = 0;
	while (1)
	{
		ch = fgetc(fp);
		if (ch == EOF)
			break;
		if (_utf8rtf_isinword(ch))
			buf[blen++] = ch;
		else
			break;;
	}
	buf[blen] = 0;
	return ch;
}

static int _utf8rtf_iscontrol(char *buf)
{
	if (
			(buf[0] >= 'a' && buf[0] <= 'z')
		 )
		return 1;
	return 0;
}

static int _utf8rtf_isutf(char *buf)
{
	if (buf[0] == 'u')
		if (buf[1] >= '0' && buf[1] <= '9') 
			return 1;
	return 0;
}

static int _utf8rtf_is8bit(char *buf)
{
	if (buf[0] == '\'')
		if (
			(buf[1] >= '0' && buf[1] <= '9') ||
			(buf[1] >= 'A' && buf[1] <= 'Z') ||
			(buf[1] >= 'a' && buf[1] <= 'z')
			)
		return 1;
	return 0;
}

static int _utf8rtf_parse_rtf(FILE *fp)
{
	char buf[BUFSIZ];
	int ch;

	while ((ch=fgetc(fp))!=EOF){

		parse:

		if (ch == '\\'){ // starts with dash
			char buf[BUFSIZ];	
			ch = _utf8rtf_readword(fp, buf);

			// check if not service word
			if (!_utf8rtf_iscontrol(buf)){
				if (_utf8rtf_is8bit(buf)){
					// handle with codepages
					/* TODO: CODEPAGES */
				} else{
					// print it
					fprintf(stdout, "\\%s", buf);
				}
				goto parse;
			}
			
			if (_utf8rtf_isutf(buf)){
				// handle utf
				char unicode[5] = 
				{
					buf[1], 
					buf[2], 
					buf[3], 
					buf[4], 
					0
				};
				
				uint32_t u;
				sscanf(unicode, "%u", &u);			
				char s[7];
				int l = _utf8rtf_c32tomb(s, u);
				s[l] = 0;
				fprintf(stdout, "%s", s);
				
				// if next char is space or ? - drop it
				if (ch == ' ' || ch == '?')
					continue;
			
				goto parse;
			}
			
			/* print service word */
			fprintf(stdout, "\\%s", buf);
			goto parse;
		}
		
		// by default print char
		fprintf(stdout, "%c", ch);
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

static int _utf8rtf_parse_utf8(FILE *fp)
{
	char buf[BUFSIZ];
	uint8_t ch;
	int i, l=0, c;

	while ((c=fgetc(fp))!=EOF){

		ch = c;

		if (c >= 252){/* 6-bytes */
			char buf[7];
			buf[0] = ch;
			buf[1] = fgetc(fp);
			buf[2] = fgetc(fp);
			buf[3] = fgetc(fp);
			buf[4] = fgetc(fp);
			buf[5] = fgetc(fp);
			buf[6] = 0;

			uint32_t s32;
			if (_utf8rtf_mbtoc32(&s32, buf))
				fprintf(stdout,
							"\\u%d ", s32);
		} 
		else if (c >= 248){/* 5-bytes */
			char buf[6];
			buf[0] = ch;
			buf[1] = fgetc(fp);
			buf[2] = fgetc(fp);
			buf[3] = fgetc(fp);
			buf[4] = fgetc(fp);
			buf[5] = 0;

			uint32_t s32;
			if (_utf8rtf_mbtoc32(&s32, buf))
				fprintf(stdout,
							"\\u%d ", s32);
		}
		else if (c >= 240){/* 4-bytes */
			char buf[5];
			buf[0] = ch;
			buf[1] = fgetc(fp);
			buf[2] = fgetc(fp);
			buf[3] = fgetc(fp);
			buf[4] = 0;

			uint32_t s32;
			if (_utf8rtf_mbtoc32(&s32, buf))
				fprintf(stdout,
							"\\u%d ", s32);
		} 
		else if (c >= 224){/* 3-bytes */
			char buf[4];
			buf[0] = ch;
			buf[1] = fgetc(fp);
			buf[2] = fgetc(fp);
			buf[3] = 0;

			uint32_t s32;
			if (_utf8rtf_mbtoc32(&s32, buf))
				fprintf(stdout,
							"\\u%d ", s32);
		}
		else if (c >= 192){/* 2-bytes */
			char buf[3];
			buf[0] = ch;
			buf[1] = fgetc(fp);
			buf[2] = 0;

			uint32_t s32;
			if (_utf8rtf_mbtoc32(&s32, buf))
				fprintf(stdout,
							"\\u%d ", s32);
		} 
		else{/* 1-byte */
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
