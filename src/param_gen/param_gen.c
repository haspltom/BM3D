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
	char line[50];

	sprintf (prefix, "params_%d-%d-%d-%d-%d-%d", block_size, block_step, sigma, max_blocks, h_search, v_search);

	// set filename for parameter file
	if (get_output_filename (outfile, "res/", prefix, "txt", 0) != 0) {
		generate_error ("Unable to process output filename for parameter file...");
		return 1;
	}

	// open file for append
	fd = fopen (outfile, "w");
	if (fd == NULL) {
		generate_error ("Unable to open parameter-file for writing...");
		return 1;
	}

	sprintf (line, "# file:        %s", outfile);
	fprintf (fd, "%s\n", line);
	
	sprintf (line, "# author:      Thomas Haspl");
	fprintf (fd, "%s\n", line);

	sprintf (line, "# date:        2015-04-01");
	fprintf (fd, "%s\n", line);

	sprintf (line, "# description: Config-File for the BM3D denoising algorithm");
	fprintf (fd, "%s\n\n", line);

	sprintf (line, "BEGIN_PARAMS");
	fprintf (fd, "%s\n", line);

	sprintf (line, "	BLOCK_SIZE %d", block_size);
	fprintf (fd, "%s\n", line);

	sprintf (line, "	BLOCK_STEP %d", block_step);
	fprintf (fd, "%s\n", line);

	sprintf (line, "	SIGMA %d", sigma);
	fprintf (fd, "%s\n", line);

	sprintf (line, "	MAX_BLOCKS %d", max_blocks);
	fprintf (fd, "%s\n", line);

	sprintf (line, "	H_SEARCH %d", h_search);
	fprintf (fd, "%s\n", line);

	sprintf (line, "	V_SEARCH %d", v_search);
	fprintf (fd, "%s\n", line);

	sprintf (line, "END_PARAMS");
	fprintf (fd, "%s", line);

	// close file again
	if (fclose(fd) == EOF) {
		generate_error ("Unable to close parameter-file...");
		return 1;
	}

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
