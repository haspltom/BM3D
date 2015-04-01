#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "../error/error.h"
#include "../png_io/png_io.h"
#include "../utils/utils.h"
#include "noising.h"

int image_noise (char* const infile, int const std_dev) {
	png_img img;
	int i, j;
	png_byte* row;
	png_byte* tmp;
	char outfile[40];
	
	// read input image
	if (png_read(&img, infile) != 0) {
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
	if ((std_dev<0) || (std_dev>255)) {
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

			// add AWGN
			if (std_dev != 0) {
				tmp[0] = limit (tmp[0] + (rand() % (2*std_dev)) - std_dev);
				tmp[1] = limit (tmp[1] + (rand() % (2*std_dev)) - std_dev);
				tmp[2] = limit (tmp[2] + (rand() % (2*std_dev)) - std_dev);
			}
		}
	}

	// set output filename
	if (get_output_filename (outfile, "../img/rgb/", "noisy_rgb_house", std_dev) != 0) {
		generate_error ("Unable to process output filename...");
		return 1;
	}

	// write output image
	if (png_write(&img, outfile) != 0) {
		return 1;
	}

	return 0;
}

// main procedure, which calls the noising() function
int main (int argc, char **argv) {
	char* err_prefix = "[ERROR] ... ";

	// check arguments
	if (argc < 3) {
		printf ("%s%s\n", err_prefix, "Wrong number of arguments...");
		printf ("Usage:\n");
		printf ("noising <standard deviation> <filename>\n");
		return 1;
	}

	if (image_noise(argv[2], atoi(argv[1])) != 0) {
		printf ("%s%s\n", err_prefix, ptr);
		// free (ptr); //TODO
		return 1;
	}

	return 0;
}
