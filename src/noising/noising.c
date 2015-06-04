#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "../error/error.h"
#include "../png_io/png_io.h"
#include "../utils/utils.h"
#include "noising.h"

int image_noise (char* const infile, char* const output_path, int const sigma) {
	png_img img;
	png_img org;
	FILE* log;
	int i, j;
	png_byte* row;
	png_byte* tmp;
	char pure_name[30];
	char prefix[40];
	char logfile[30];
	
	// ----------------------------------------------------------------------
	// OPEN LOG-FILE FOR WRITING
	// ----------------------------------------------------------------------

	// obtain filename without path and extension
	if (exclude_extension(infile, pure_name) != 0) {
		return 1;
	}

	sprintf (logfile, "log/log_%s[%d].txt", pure_name, sigma);

	log = fopen (logfile, "a");

	if (log == NULL) {
		generate_error ("Unable to open log-file for writing ...");
		return 1;
	}

	// read input image
	if (png_read(&img, infile) != 0) {
		return 1;
	}

	if (png_read(&org, infile) != 0) {
		return 1;
	}

	// control color type
	if (img.color != PNG_COLOR_TYPE_RGB) {
		generate_error ("Wrong color type...");
		return 1;
	}

	// control number of channels
	if (img.channels != 3) {
		generate_error ("Wrong number of channels...");
		return 1;
	}

	// check standard deviation
	if ((sigma<0) || (sigma>255)) {
		generate_error ("Standard Deviation must be between 0 and 255...");
		return 1;
	}
	
	// seed random number generator
	srand (time(NULL));

	for (j=0; j<img.height; ++j) {
		row = img.data[j];

		for (i=0; i<img.width; ++i) {
			tmp = &(row[i*3]);

			// r = limit (y + 1.402*v);
			// g = limit (y - 0.3441*u - 0.7141*v);
			// b = limit (y + 1.772*u);

			// add noise
			if (sigma != 0) {
				tmp[0] = limit (tmp[0] + (rand() % (2*sigma)) - sigma);
				tmp[1] = limit (tmp[1] + (rand() % (2*sigma)) - sigma);
				tmp[2] = limit (tmp[2] + (rand() % (2*sigma)) - sigma);
			}
		}
	}

	sprintf (prefix, "noisy_rgb_%s", pure_name);

	// write output image
	if (png_write(&img, output_path, prefix, sigma) != 0) {
		return 1;
	}

	fprintf (log, "[INFO] ... PSNR before denoising: %fdB\n\n\n", get_snr(&org, &img));

	// ----------------------------------------------------------------------
	// FREEING DYNAMICALY ALLOCATED MEMORY
	// ----------------------------------------------------------------------
	png_free_mem (&img);
	png_free_mem (&org);
	fclose (log);

	return 0;
}
