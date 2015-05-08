#ifndef PNG_IO_H
#define PNG_IO_H

#include "png.h"

#define PNG_SIG_SIZE 8			// size of the png-signature

// definition of a custom png structure
typedef struct png_img_struct {
	int width;
	int height;
	int channels;
	int passes;
	png_byte color;						// type of color
	png_byte depth;						// bit depth
	png_bytep* data;						// array of rows
} png_img;

extern void png_copy_values (png_img* target, png_img* sourc);
extern int png_read (png_img* img, char* const filename);
extern int png_write (png_img* const img, char* const path, char* const prefix, int const attr);
extern void png_free_mem (png_img* img);

#endif // PNG_IO_H
