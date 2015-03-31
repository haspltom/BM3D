#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../error/error.h"
#include "../../png_io/png_io.h"
#include "noising.h"

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
