/**
 * File              : image2rtf.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 05.09.2023
 * Last Modified Date: 05.09.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef IMAGE_TO_RTF
#define IMAGE_TO_RTF

#define USE_LIBTIFF

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdint.h>
#ifdef USE_LIBTIFF
#include <tiffio.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct image_jpg_write_s {
	unsigned char *data;
	size_t len;
};

static void 
image_jpg_write_func(void *context, void *data, int size){
	struct image_jpg_write_s *s = 
		(struct image_jpg_write_s *)context; 
	
	// realloc data
	s->data = (unsigned char *)realloc(s->data, s->len + size);
	if (!s->data){
				perror("realloc");
		return;
	}
	
	// copy
	memcpy(&(s->data[s->len]), data, size);
	s->len += size;
}

static unsigned char * bin_to_strhex(
		const unsigned char *bin,
		unsigned int binsz,
		unsigned char **result)
{
	unsigned char hex_str[] = "0123456789abcdef";
	unsigned int  i;

	if (!binsz)
		return NULL;
	
	if (!(*result = (unsigned char *)malloc(binsz * 2 + 1)))
		return NULL;

	(*result)[binsz * 2] = 0;

	for (i = 0; i < binsz; ++i)
	{
		(*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
		(*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
	}

	return (*result);
}

static unsigned char *image2hex(const char *filename){
	// try to load file
	int w=0, h=0, c;
	stbi_uc *image = 
		stbi_load(filename, &w, &h, &c, 0);

#ifdef USE_LIBTIFF
	if (!image){
		// maybe TIFF?
		TIFF *tif =
			TIFFOpen(filename, "r");
		if (tif){
			c = 4;
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
			if (w > 1 && h > 1){
				image = (unsigned char *)_TIFFmalloc(w * h * w);
				if (image){
					if (!TIFFReadRGBAImage(
								tif, w, h, (uint32_t *)image, 0))
					{
						free(image);
						image = NULL;
					}
				}
			}
		}
	}
#endif

	if (!image){
				perror("can't open image");
		return NULL;
	}

	struct image_jpg_write_s s;
	s.data = (unsigned char *)malloc(1);
	if (!s.data){
		perror("allocate");
		exit(EXIT_FAILURE);
	}
	s.len = 0;

	if (stbi_write_jpg_to_func(
			image_jpg_write_func,
			&s, 
			w, h, c, 
			image, 90) == 0) 
	{
		perror("convert image to jpeg");
		stbi_image_free(image);
		return NULL;
	}
		
	stbi_image_free(image);

	unsigned char *str;
	bin_to_strhex(s.data, s.len, &str);
	free(s.data);

	return str;
}

#ifdef __cplusplus
}
#endif
#endif /* ifndef IMAGE_TO_RTF */
