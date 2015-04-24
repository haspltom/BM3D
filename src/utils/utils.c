#include <stdio.h>
#include "utils.h"

// limits a given value to 255
double limit (double const x) {
	return (x < 0.0) ? .00 : (x > 255.0) ? 255.0 : x;
}

// produces an output filename from given string literals
int get_output_filename (char* outfile, char* const path, char* const prefix, char* const ext, int const attr) {
	int count;

	if (attr != 0) {
		if (attr < 100) {
			count = sprintf (outfile, "%s%s[0%d].%s", path, prefix, attr, ext); 
		}
		else {
			count = sprintf (outfile, "%s%s[%d].%s", path, prefix, attr, ext);
		}
	}
	else {
		count = sprintf (outfile, "%s%s.%s", path, prefix, ext); 
	}

	return (count == 0) ? 1 : 0;
}
