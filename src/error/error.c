#include <stdlib.h>
#include <string.h>
#include "error.h"

void generate_error (char* msg) {
	int len = strlen(msg);
	err_msg tmp = malloc(sizeof(char)*len);

	tmp = (err_msg)msg;
	ptr = tmp;
}

void free_error_ptr () {
	free (ptr);
}
