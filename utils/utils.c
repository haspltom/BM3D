#include <stdio.h>
#include "utils.h"

// limits a given value to 255
int limit (int const x) {
	return (x < 0) ? 0 : (x > 255) ? 255 : x;
}

// produces an output filename from given string literals
int get_output_filename (char* outfile, char* const path, char* const prefix, int const std_dev) {
	int count;

	if (std_dev < 100) {
		count = sprintf (outfile, "%s%s[0%d].png", path, prefix, std_dev); 
	}
	else {
		count = sprintf (outfile, "%s%s[%d].png", path, prefix, std_dev);
	}

	return (count == 0) ? 1 : 0;
}
