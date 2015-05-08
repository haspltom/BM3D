#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// #include <time.h>
#include "../error/error.h"
#include "../utils/utils.h"
#include "param_gen.h"

extern int generate_params_file (char* const kind,
											unsigned int const block_size, 
											unsigned int const block_step, 
											unsigned int const sigma,
											unsigned int const max_blocks,
											unsigned int const h_search,
											unsigned int const v_search,
											double const th_2d,
											double const tau_2d,
											double const th_3d) {
	FILE* fd;
	char outfile[40];
	char prefix[30];
	char line[50];
	time_t t = time (NULL); 
	struct tm tm = *localtime (&t);
	char month[3];
	char day[3];

	if ((block_size < 10) && (block_step >= 10)) {
		sprintf (prefix, "params_%s-0%d-%d-%d-%d-%d-%d", kind, block_size, block_step, sigma, max_blocks, h_search, v_search);
	}
	else if ((block_size >= 10) && (block_step < 10)) {
		sprintf (prefix, "params_%s-%d-0%d-%d-%d-%d-%d", kind, block_size, block_step, sigma, max_blocks, h_search, v_search);
	}
	else if ((block_size < 10) && (block_step < 10)) {
		sprintf (prefix, "params_%s-0%d-0%d-%d-%d-%d-%d", kind, block_size, block_step, sigma, max_blocks, h_search, v_search);
	}
	else {
		sprintf (prefix, "params_%s-%d-%d-%d-%d-%d-%d", kind, block_size, block_step, sigma, max_blocks, h_search, v_search);
	}

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

	// format month approriately
	if (tm.tm_mon+1 < 10) {
		sprintf (month, "0%d", tm.tm_mon+1);
	}
	else {
		sprintf (month, "%d", tm.tm_mon+1);
	}

	// format day approriately
	if (tm.tm_mday < 10) {
		sprintf (day, "0%d", tm.tm_mday);
	}
	else {
		sprintf (day, "%d", tm.tm_mday);
	}

	sprintf (line, "# date:        %d-%s-%s", tm.tm_year+1900, month, day);
	fprintf (fd, "%s\n", line);

	sprintf (line, "# description: Config-File for the BM3D denoising algorithm");
	fprintf (fd, "%s\n\n", line);

	sprintf (line, "BEGIN_PARAMS");
	fprintf (fd, "%s\n", line);

	sprintf (line, "	SHRINKAGE_KIND %s", kind);
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

	sprintf (line, "	THRESHOLD_2D %f", th_2d);
	fprintf (fd, "%s\n", line);

	sprintf (line, "	TAU_MATCH_2D %f", tau_2d);
	fprintf (fd, "%s\n", line);

	sprintf (line, "	THRESHOLD_3D %f", th_3d);
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
	if (argc != 11) {
		printf ("%s%s\n", err_prefix, "Wrong number of arguments...");
		printf ("Usage:\n");
		printf ("param_gen ");
		printf ("<shrinkage kind> ");
		printf ("<block size> ");
		printf ("<block step> ");
		printf ("<sigma> ");
		printf ("<maximum number of blocks> ");
		printf ("<horizontal search window size> ");
		printf ("<vertical search window size >");
		printf ("<2D threshold> ");
		printf ("<2D tau-match> ");
		printf ("<3D threshold>\n");
		return 1;
	}

	if (generate_params_file(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), atof(argv[8]), atof(argv[9]), atof(argv[10])) != 0) {
		printf ("%s%s\n", err_prefix, ptr);
		// free (ptr); //TODO
		return 1;
	}

	return 0;
}
