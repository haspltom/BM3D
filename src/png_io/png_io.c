#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../error/error.h"
#include "../utils/utils.h"
#include "png_io.h"

void png_copy_values (png_img* target, png_img* source) {
	png_byte* row_source;
	png_byte* row_target;
	png_byte* tmp_source;
	png_byte* tmp_target;
	int i, j;

	for (j=0; j<target->height; ++j) {
		row_target = target->data[j];
		row_source = source->data[j];

		for (i=0; i<target->width; ++i) {
			tmp_target = &(row_target[i*3]);
			tmp_source = &(row_source[i*3]);

			tmp_target[0] = tmp_source[0];
			tmp_target[1] = tmp_source[1];
			tmp_target[2] = tmp_source[2];
		}
	}
}

int png_read (png_img* img, char* const filename) {
	FILE* fd;
	png_structp png;						// pointer to png-file
	png_infop png_info;					// pointer to png-information
	unsigned char sig[PNG_SIG_SIZE]; // contains information of png-signature
	int i;

	// open file for reading
	fd = fopen (filename, "rb");
	if (fd == NULL) {
		generate_error ("Unable to open file for reading...");
		return 1;
	}

	// read first eight byte of file (signature) and check its correctness
	fread (sig, sizeof(unsigned char), PNG_SIG_SIZE, fd);
	if (png_sig_cmp(sig, 0, PNG_SIG_SIZE) != 0) {
		generate_error ("Input file is not recognized as png...");
		return 1;
	}

	// allocate and initialize structure for png-file
	png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); // use default error handling funtions
	if (png == NULL) {
		generate_error ("Unable to create structure for png-reading...");
		return 1;
	}

	// allocate and initialize structure for png-information
	png_info = png_create_info_struct (png);
	if (png_info == NULL) {
		png_destroy_read_struct(&png, NULL, NULL);
		generate_error ("Unable to create structure for png-information...");
		return 1;
	}

	// initialize IO for reading
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &png_info, NULL);
		generate_error ("Unable to initialize IO for reading...");
		return 1;
	}

	// initialize IO for reading png-file
	png_init_io (png, fd);
	
	// skip first eight bytes of the file, which represent the png-signature
	png_set_sig_bytes (png, PNG_SIG_SIZE);

	// read png-information
	png_read_info (png, png_info);
	img->width = png_get_image_width (png, png_info);
	img->height = png_get_image_height (png, png_info);
	img->color = png_get_color_type (png, png_info);
	img->depth = png_get_bit_depth (png, png_info);
	img->channels = png_get_channels (png, png_info);

	png_read_update_info (png, png_info);

	// allocate memory for image
	img->data = (png_bytep*)(malloc(sizeof(png_bytep)*img->height));
	for (i=0; i<img->height; ++i) {
		img->data[i] = (png_byte*)(malloc(png_get_rowbytes(png, png_info)));
	}

	// read image
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &png_info, NULL);
		generate_error ("Unable to read input file...");
		return 1;
	}

	png_read_image (png, img->data);

	// free allocated memory for png-structures
	png_destroy_read_struct(&png, &png_info, NULL);

	// close file again
	if (fclose(fd) == EOF) {
		generate_error ("Unable to close file...");
		return 1;
	}

	return 0;
}

int png_write (png_img* const img, char* const path, char* const prefix, int const attr) {
	FILE* fd;
	png_structp png;						// pointer to png-file
	png_infop png_info;					// pointer to png-information
	char outfile[40];						// output-filename

	// set filename according to path and prefix information
	if (get_output_filename (outfile, path, prefix, "png", attr) != 0) {
		generate_error ("Unable to process output filename...");
		return 1;
	}

	// open file for writing
	fd = fopen (outfile, "wb");
	if (fd == NULL) {
		generate_error ("Unable to open file for writing...");
		return 1;
	}

	// allocate and initialize structure for png-file
	png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); // use default error handling funtions
	if (png == NULL) {
		generate_error ("Unable to create structure for png-writing...");
		return 1;
	}

	// allocate and initialize structure for png-information
	png_info = png_create_info_struct (png);
	if (png_info == NULL) {
		png_destroy_read_struct(&png, NULL, NULL);
		generate_error ("Unable to create structure for png-information...");
		return 1;
	}

	// initialize IO for writing
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &png_info, NULL);
		generate_error ("Unable to initialize IO for writing...");
		return 1;
	}

	// initialize IO for reading png-file
	png_init_io (png, fd);

	// write png-signature
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &png_info, NULL);
		generate_error ("Unable to write png-signature...");
		return 1;
	}

	// set image attributes
	png_set_IHDR (png, 
					  png_info, 
					  img->width, 
					  img->height, 
					  img->depth, 
					  img->color, 
					  PNG_INTERLACE_NONE, 
					  PNG_COMPRESSION_TYPE_BASE, 
					  PNG_FILTER_TYPE_BASE);
	
	// write png-information into output file
	png_write_info (png, png_info);

	// write file
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &png_info, NULL);
		generate_error ("Unable to write file...");
		return 1;
	}

	png_write_image (png, img->data);

	// write end of file
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &png_info, NULL);
		generate_error ("Unable to write end of file...");
		return 1;
	}

	png_write_end (png, NULL);

	// free allocated memory for png-structures
	png_destroy_write_struct(&png, &png_info);

	// close file again
	if (fclose(fd) == EOF) {
		generate_error ("Unable to close file...");
		return 1;
	}

	// printf ("[INFO] ... File \"%s\" written successfully...\n", outfile);

	return 0;
}

void png_free_mem (png_img* img) {
	int i;

	if (img->data != NULL) {
		for (i=0; i<img->height; ++i) {
			free (img->data[i]);
		}

		free (img->data);
	}
}

