#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../error/error.h"
#include "../utils/utils.h"
#include "param_gen.h"

extern int generate_params_file (unsigned int const block_size, 
											unsigned int const block_step, 
											unsigned int const sigma,
											unsigned int const max_blocks,
											unsigned int const h_search,
											unsigned int const v_search) {
	FILE* fd;
	char outfile[40];
	char prefix[30];

	sprintf (prefix, "params%d-%d-%d-%d-%d-%d", block_size, block_step, sigma, max_blocks, h_search, v_search);

	// set filename for parameter file
	if (get_output_filename (outfile, "res/", prefix, "txt", 0) != 0) {
		generate_error ("Unable to process output filename for parameter file...");
		return 1;
	}

	printf ("%s\n", outfile);

	return 0;
}

// main procedure, which calls the param-generation function
int main (int argc, char **argv) {
	char* err_prefix = "[ERROR] ... ";

	// check arguments
	if (argc != 7) {
		printf ("%s%s\n", err_prefix, "Wrong number of arguments...");
		printf ("Usage:\n");
		printf ("param_gen ");
		printf ("<block size> ");
		printf ("<block step> ");
		printf ("<sigma> ");
		printf ("<maximum number of blocks> ");
		printf ("<horizontal search window size> ");
		printf ("<vertical search window size>\n");
		return 1;
	}

	if (generate_params_file(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6])) != 0) {
		printf ("%s%s\n", err_prefix, ptr);
		// free (ptr); //TODO
		return 1;
	}

	return 0;
}
