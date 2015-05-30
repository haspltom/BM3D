#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error/error.h"
#include "utils/utils.h"
#include "param_pars/param_pars.h"
#include "noising/noising.h"
#include "bm3d/bm3d.h"

int main (int argc, char **argv) {
	char* err_prefix = "[ERROR] ... ";
	params_t params;
	char pure_name[20];
	char prefix[30];
	char filename[50];

	// check arguments
	if (argc != 4) {
		printf ("%s%s\n", err_prefix, "Wrong number of arguments...");
		printf ("Usage: bm3d <image-filename> <output path> <config-filename>\n");
		return 1;
	}

	// read parameters from config-file
	if (get_params(argv[3], &params) != 0) {
		printf ("%s%s\n", err_prefix, ptr);
		return 1;
	}

	// add noise to the image
	printf ("[INFO] ... adding noise to image...\n");

	if (image_noise(argv[1], argv[2], params.sigma) != 0) {
		printf ("%s%s\n", err_prefix, ptr);
		// free (ptr); //TODO
		return 1;
	}

	printf ("[INFO] ... finished adding noise...\n\n");

	// perform denoising
	printf ("[INFO] ... launch BM3D...\n");

	exclude_extension (argv[1], pure_name);
	sprintf (prefix, "noisy_rgb_%s", pure_name);
	get_output_filename (filename, argv[2], prefix, "png", params.sigma);

	if (bm3d(filename,
				params.kind,
				params.block_size, 
				params.block_step, 
				params.sigma, 
				params.max_blocks, 
				params.h_search, 
				params.v_search,
				params.th_2d,
				params.tau_2d,
				params.th_3d) != 0) {
		printf ("%s%s\n", err_prefix, ptr);
		// free (ptr); //TODO
		return 1;
	}

	printf ("[INFO] ... finished BM3D...\n\n");

	return 0;
}
