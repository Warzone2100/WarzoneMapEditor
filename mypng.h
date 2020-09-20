#ifndef MYPNG_H
#define MYPNG_H
#include <png.h>
struct mpng {
	int width, height;
	png_byte color_type;
	png_byte bit_depth;
	png_bytep *row_pointers = NULL;
	unsigned char *rgbpixels = NULL;
};
struct mpng read_png_file(char *filename);
#endif
