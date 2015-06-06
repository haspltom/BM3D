#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include "../error/error.h"
#include "param_pars.h"

double get_val (char* const str) {
	regex_t regex;
	regmatch_t regmatch;
	char* sub;
	unsigned int len = 0;
	double val = 0.0;

	// compile regular expression
	if (regcomp(&regex, "[[:blank:]][[:digit:]]+(.[[:digit:]]+)?", REG_EXTENDED) != 0) {
		generate_error ("Unable to compile regular expression");
		return 1;
	}

	// find first occurance of 'regex' within 'tmp'
	if (regexec(&regex, str, 1, &regmatch, 0) == REG_NOMATCH) {
		generate_error ("Unable to determine value");
		return 1;
	}

	len = regmatch.rm_eo - regmatch.rm_so;		// determine length of substring
	sub = malloc (len+1);							// allocate memory for substring
	strncpy (sub, str+regmatch.rm_so, len);	// read substring from line
	sub[len] = '\0';									// add terminating zero to substring
	val = atof (sub);									// convert substring to number
	free (sub);											// free allocated memory again
	regfree (&regex);									// free compiled regular expression again

	return val;
}

char* get_kind (char* const str) {
	regex_t regex;
	regmatch_t regmatch;
	char* kind;
	unsigned int len = 0;

	// compile regular expression
	if (regcomp(&regex, "avg|ht|wnr|none", REG_EXTENDED) != 0) {
		generate_error ("Unable to compile regular expression");
		return 0;
	}

	// find first occurance of 'regex' within 'tmp'
	if (regexec(&regex, str, 1, &regmatch, 0) == REG_NOMATCH) {
		generate_error ("Unable to determine kind");
		return 0;
	}

	len = regmatch.rm_eo - regmatch.rm_so;		// determine length of substring
	kind = malloc (len+1);							// allocate memory for substring
	strncpy (kind, str+regmatch.rm_so, len);	// read substring from line
	kind[len] = '\0';									// add terminating zero to substring
	// free (kind);											// free allocated memory again
	regfree (&regex);									// free compiled regular expression again

	return kind;
}

int get_params (char* const filename, params_t* params) {
	FILE* fd;
	state st = IDLE;
	char tmp[512];

	// open file for reading
	fd = fopen (filename, "r");
	if (fd == NULL) {
		generate_error ("Unable to open file for parsing parameters");
		return 1;
	}

	// read config-file line per line
	while (fgets(tmp, 512, fd) != NULL) {
		if (strstr(tmp, "BEGIN_PARAMS") != NULL) {						// begin of parameter section
			st = ACTIVE;
		}
		else if (strstr(tmp, "END_PARAMS") != NULL) {					// end of parameter section
			st = IDLE;
		}

		// write the extracted value to the regarding variable
		if (st == ACTIVE) {
			if (strstr(tmp, "SHRINKAGE_KIND") != NULL) {
				params->kind = get_kind(tmp);
				
				if (!params->kind) {
					generate_error ("Unable to determine shrinkage kind");
					return 1;
				}
			}
			else if (strstr(tmp, "BLOCK_SIZE") != NULL) {
				params->block_size = (int)get_val(tmp);
			}
			else if (strstr(tmp, "BLOCK_STEP") != NULL) {
				params->block_step = (int)get_val(tmp);
			}
			else if (strstr(tmp, "SIGMA") != NULL) {
				params->sigma = (int)get_val(tmp);
			}
			else if (strstr(tmp, "MAX_BLOCKS") != NULL) {
				params->max_blocks = (int)get_val(tmp);
			}
			else if (strstr(tmp, "H_SEARCH") != NULL) {
				params->h_search = (int)get_val(tmp);
			}
			else if (strstr(tmp, "V_SEARCH") != NULL) {
				params->v_search = (int)get_val(tmp);
			}
			else if (strstr(tmp, "THRESHOLD_2D") != NULL) {
				params->th_2d = get_val(tmp);
			}
			else if (strstr(tmp, "TAU_MATCH_2D") != NULL) {
				params->tau_2d = get_val(tmp);
			}
			else if (strstr(tmp, "THRESHOLD_3D") != NULL) {
				params->th_3d = get_val(tmp);
			}
			else if (strstr(tmp, "BLOCK_MARKING") != NULL) {
				params->block_marking = get_val(tmp);
			}
		}
	}

	// close file again
	if (fclose(fd) == EOF) {
		generate_error ("Unable to close config-file...");
		return 1;
	}

	return 0;
}
