#ifndef PARAM_GEN_H
#define PARAM_GEN_H

extern int generate_params_file (char* const kind,
										   unsigned int const block_size, 
											unsigned int const block_step, 
											unsigned int const sigma,
											unsigned int const max_blocks,
											unsigned int const h_search,
											unsigned int const v_search,
											double const th_2d,
											double const tau_2d,
											double const th_3d,
											unsigned int block_marking);

#endif // PARAM_GEN_H
