#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "../error/error.h"
#include "../png_io/png_io.h"
#include "../utils/utils.h"
#include "noising.h"

int exclude_extension (char* const str, char* name) {
	int count = 0;
	char tmp[30];
	char* iter = strrchr (str, '/');

	if (!iter) {
		generate_error ("Invalid input filename...");
		return 1;
	}

	++iter; 		// in order to exclude the '/' itself as well

	while (iter && (*iter != '.')) {
		tmp[count] = *iter;
		++iter;
		++count;
	}

	tmp[count] = '\0';
	count = sprintf (name, "%s", tmp);

	return (count == 0) ? 1 : 0;
}

int image_noise (char* const infile, char* const output_path, int const std_dev) {
	png_img img;
	int i, j;
	png_byte* row;
	png_byte* tmp;
	char outfile[40];
	char pure_name[30];
	char prefix[40];
	
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

			// add noise
			if (std_dev != 0) {
				tmp[0] = limit (tmp[0] + (rand() % (2*std_dev)) - std_dev);
				tmp[1] = limit (tmp[1] + (rand() % (2*std_dev)) - std_dev);
				tmp[2] = limit (tmp[2] + (rand() % (2*std_dev)) - std_dev);
			}
		}
	}

	// obtain filename without path and extension
	if (exclude_extension(infile, pure_name) != 0) {
		return 1;
	}

	sprintf (prefix, "noisy_rgb_%s", pure_name);

	// set output filename
	if (get_output_filename (outfile, output_path, prefix, "png", std_dev) != 0) {
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
	if (argc < 4) {
		printf ("%s%s\n", err_prefix, "Wrong number of arguments...");
		printf ("Usage:\n");
		printf ("noising <standard deviation> <filename> <output path>\n");
		return 1;
	}

	if (image_noise(argv[2], argv[3], atoi(argv[1])) != 0) {
		printf ("%s%s\n", err_prefix, ptr);
		// free (ptr); //TODO
		return 1;
	}

	return 0;
}
