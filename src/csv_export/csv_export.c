#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../error/error.h"
#include "csv_export.h"

int add_csv_line (char* const filename, unsigned int const block_step, unsigned int const groups, double const time) {
	FILE* fd;

	// open file for append
	fd = fopen (filename, "a");
	if (fd == NULL) {
		generate_error ("Unable to open file for writing...");
		return 1;
	}

	fprintf (fd, "\"%d\",\"%d\",\"%f\"\n", block_step, groups, time);

	// close file again
	if (fclose(fd) == EOF) {
		generate_error ("Unable to close file...");
		return 1;
	}

	return 0;
}
