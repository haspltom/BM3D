#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error/error.h"
#include "param_pars/param_pars.h"
#include "bm3d/bm3d.h"
// #include "../cips2edsrc/CIPS.H" //EXPERIMENTAL

int main (int argc, char **argv) {
	char* err_prefix = "[ERROR] ... ";
	params_t params;

	// check arguments
	if (argc != 3) {
		printf ("%s%s\n", err_prefix, "Wrong number of arguments...");
		printf ("Usage: bm3d <image-filename> <config-filename>\n");
		return 1;
	}

	// read parameters from config-file
	if (get_params(argv[2], &params) != 0) {
		printf ("%s%s\n", err_prefix, ptr);
		return 1;
	}

	printf ("[INFO] ... launch BM3D...\n");

	if (bm3d(argv[1],
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
