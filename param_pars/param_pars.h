#ifndef PARAM_PARS_H
#define PARAM_PARS_H

typedef struct params {
	unsigned int block_size;
	unsigned int block_step;
	unsigned int sigma;
	unsigned int max_blocks;
	unsigned int h_search;
	unsigned int v_search;
} params_t;

typedef enum states {
	IDLE,
	ACTIVE,
	BLOCK_STEP,
	BLOCK_SIZE,
	SIGMA,
	MAX_BLOCKS,
	H_SEARCH,
	V_SEARCH
} state;

extern int get_params (char* const filename, params_t* params);

#endif // PARAM_PARS_H
