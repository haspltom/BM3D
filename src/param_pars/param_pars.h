#ifndef PARAM_PARS_H
#define PARAM_PARS_H

typedef struct params {
	char* kind;
	unsigned int block_size;
	unsigned int block_step;
	unsigned int sigma;
	unsigned int max_blocks;
	unsigned int h_search;
	unsigned int v_search;
	double th_2d;
	double tau_2d;
	double th_3d;
} params_t;

typedef enum states {
	IDLE,
	ACTIVE
} state;

extern int get_params (char* const filename, params_t* params);

#endif // PARAM_PARS_H
