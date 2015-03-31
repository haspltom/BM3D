#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include "../error/error.h"
#include "param_pars.h"

unsigned int get_val (char* const str) {
	regex_t regex;
	regmatch_t regmatch;
	char* sub;
	unsigned int len = 0;
	unsigned int val = 0;

	// compile regular expression
	if (regcomp(&regex, "[[:digit:]]+", REG_EXTENDED) != 0) {
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
	val = atoi (sub);									// convert substring to number
	free (sub);											// free allocated memory again
	regfree (&regex);									// free compiled regular expression again

	return val;
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

		// set the appropriate status
		if (st == ACTIVE) {
			if (strstr(tmp, "BLOCK_SIZE") != NULL) st = BLOCK_SIZE;
			if (strstr(tmp, "BLOCK_STEP") != NULL) st = BLOCK_STEP;
			if (strstr(tmp, "SIGMA") != NULL) st = SIGMA;
			if (strstr(tmp, "MAX_BLOCKS") != NULL) st = MAX_BLOCKS;
			if (strstr(tmp, "H_SEARCH") != NULL) st = H_SEARCH;
			if (strstr(tmp, "V_SEARCH") != NULL) st = V_SEARCH;
		}

		// write the extracted value to the regarding variable
		params->block_size = (st == BLOCK_SIZE) ? get_val(tmp) : params->block_size;
		params->block_step = (st == BLOCK_STEP) ? get_val(tmp) : params->block_step;
		params->sigma = (st == SIGMA) ? get_val(tmp) : params->sigma;
		params->max_blocks = (st == MAX_BLOCKS) ? get_val(tmp) : params->max_blocks;
		params->h_search = (st == H_SEARCH) ? get_val(tmp) : params->h_search;
		params->v_search = (st == V_SEARCH) ? get_val(tmp) : params->v_search;

		st = ACTIVE; 																// reset status for the next value
	}

	// close file again
	if (fclose(fd) == EOF) {
		generate_error ("Unable to close config-file...");
		return 1;
	}

	return 0;
}
