#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bm3d.h"
#include "../png_io/png_io.h"
#include "../error/error.h"
#include "../utils/utils.h"

// function that make the coversion from RGB to YUV
void rgb2yuv (png_img* img) {
	int i, j, y, u, v, r, g, b;
	png_byte* row;
	png_byte* tmp;

	for (j=0; j<img->height; ++j) {
		row = img->data[j];

		for (i=0; i<img->width; ++i) {
			tmp = &(row[i*3]);

			// extract RGB values
			r = tmp[0];
			g = tmp[1];
			b = tmp[2];

			// convert pixel elements
			y = limit (0 + 0.299*r + 0.587*g + 0.114*b);
			u = limit (128 - 0.168736*r - 0.331264*g + 0.5*b);
			v = limit (128 + 0.5*r - 0.418688*g - 0.081312*b);

			// write back YUV values
			tmp[0] = y;
			tmp[1] = u;
			tmp[2] = v;
		}
	}
}

// delivers a block structure
int new_block_struct (int const bs, block_t* block) {
	int i;

	block->block_size = bs;
	block->x = 0;
	block->y = 0;
	block->data = calloc (bs, sizeof(double*));

	if (block->data == NULL) {
		generate_error ("Unable to allocate block structure...");
		return 1;
	}

	for (i=0; i<bs; ++i) {
		block->data[i] = calloc (bs, sizeof(double));

		if (block->data[i] == NULL) {
			generate_error ("Unable to allocate block structure...");
			return 1;
		}
	}

	return 0;
}

void print_single_block (block_t* block, char* const filename) {
	FILE* fd;
	int i, j;

	fd = fopen (filename, "w");

	for (j=0; j<block->block_size; ++j) {
		for (i=0; i<block->block_size; ++i) {
			fprintf (fd, "%f ", block->data[i][j]);
		}
		fprintf (fd, "\n");
	}

	fclose (fd);
}

void print_block (FILE* fd, block_t const block) {
	int i, j;

	for (j=0; j<block.block_size; ++j) {
		for (i=0; i<block.block_size; ++i) {
			fprintf (fd, "%f ", block.data[i][j]);
		}
		fprintf (fd, "\n");
	}
}

// picks a block out of a given image
int get_block (png_img* img,				// input image
					int const channel, 		// number of channels (for controlling reasons)
					block_t* block, 			// output block
					int const x, 				// x-index of desired block
					int const y) { 			// y-index of desired block
	int i, j;
	png_byte* row;
	png_byte* tmp;

	if ((channel < 0) || (channel > 2)) {
		generate_error ("Invalid channel for block building...");
		return 1;
	}

	for (j=0; j<block->block_size; ++j) {
		row = img->data[j+y];

		for (i=0; i<block->block_size; ++i) {
			tmp = &(row[(i+x)*3]);

			block->data[i][j] = (double)tmp[channel];
		}
	}

	block->x = x;
	block->y = y;
	
	return 0;
}

int append_group (list_t* list, group_t* group) {
	group_node_t* tmp = *list;
	group_node_t* new_node;
	
	new_node = (group_node_t*)malloc(sizeof(group_node_t));
	new_node->group = *group;
	new_node->next = 0;

	if (tmp != NULL) {
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		
		tmp->next = new_node;
	}
	else {
		*list = new_node;
	}
	return 0;
}

int append_block (group_t* group, block_t* block, double const distance) {
	node_t* tmp = *group;
	node_t* new_node;
	block_t tmp_block;
	int i, j;

	// allocate block memory
	if (new_block_struct(block->block_size, &tmp_block) != 0) {
		return 1;
	}

	for (j=0; j<tmp_block.block_size; ++j) {
		for (i=0; i<tmp_block.block_size; ++i) {
			tmp_block.data[i][j] = block->data[i][j];
		}
	}

	tmp_block.x = block->x;
	tmp_block.y = block->y;
	
	new_node = (node_t*)malloc(sizeof(node_t)); //MISTAKE: allocated node_t* instead of node_t
	new_node->block = tmp_block;
	new_node->distance = distance;
	new_node->next = 0;

	if (tmp != NULL) {
		while (tmp->next != NULL) {
			if ((new_node->distance >= tmp->distance) && (new_node->distance <= tmp->next->distance)) {
				new_node->next = tmp->next;
				tmp->next = new_node;
				return 0;
			}
			else {
				tmp = tmp->next;
			}
		}
		
		tmp->next = new_node;
	}
	else {
		*group = new_node;
	}

	return 0;
}

unsigned int list_length (list_t* list){
	unsigned int len = 0;
	group_node_t* tmp = *list;

	while (tmp != NULL) {
		++len;
		tmp = tmp->next;
	}

	return len;
}

unsigned int group_length (group_t* group){
	unsigned int len = 0;
	node_t* tmp = *group;

	while (tmp != NULL) {
		++len;
		tmp = tmp->next;
	}

	return len;
}

int print_list (list_t const list) {
	FILE* fd = 0;
	group_node_t* tmp = list;
	node_t* tmp_block;
	char groupname[50];
	int count = 0;

	printf ("printing groups...\n");

	while (tmp != NULL) {
		sprintf (groupname, "groups/group[%d].txt", ++count);
		fd = fopen (groupname, "w");
		
		if (fd == NULL) {
			generate_error ("Unable to open file for printing group...");
			return 1;
		}

		tmp_block = tmp->group;
		fprintf (fd, "nr of blocks in group: %d\n\n", group_length(&tmp_block));
		fprintf (fd, "reference block...\n");

		while (tmp_block != NULL) {
			print_block (fd, tmp_block->block);
			fprintf (fd, "distance to reference block: %f\n\n", tmp_block->distance);
			tmp_block = tmp_block->next;
		}

		tmp = tmp->next;

		fclose (fd);
	}

	return 0;
}

void free_group (group_t* group){
	node_t* tmp = *group;

	while (*group != NULL) {
		tmp = *group;
		*group = tmp->next;
		free (tmp);
	}
}

// produces DCT transformation and its transposed matrix regarding to the block size
void get_dct_matrixes (int const bs, double trans[bs][bs], double trans_t[bs][bs]) {
	int i, j;

	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			if (j == 0) {
				trans[i][j] = 1 / sqrt(bs);
			}
			else {
				trans[i][j] = sqrt(2/(double)bs) * cos(((2*i+1)*j*PI)/(2*(double)bs));
			}

			trans_t[j][i] = trans[i][j];
		}
	}
}

// function for multiplying two matrixes
void matrixmul (int const bs, double const m1[bs][bs], double const m2[bs][bs], double res[bs][bs]) {
	int i, j, k;
	double sum = 0.0;

	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			sum = 0.0;

			for (k=0; k<bs; ++k) {
				sum += m1[j][k] * m2[k][i];
			}

			res[i][j] = sum;
		}
	}
}

void hard_threshold (int const bs, double mat[bs][bs], double const lambda, int const std_dev) {
	int i, j;
	double threshold = lambda * (double)std_dev * sqrt(2*log(bs*bs));
	
	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			mat[i][j] = (abs(mat[i][j])>threshold) ? mat[i][j] : 0.0;
		}
	}
}

void shift_values (int const bs, block_t* block, double mat[bs][bs]) {
	int i, j;

	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			mat[i][j] = block->data[i][j] - 128.0;
		}
	}
}

void subtract_blocks (int const bs, double const mat1[bs][bs], double const mat2[bs][bs], double res[bs][bs]) {
	int i, j;

	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			res[i][j] = mat1[i][j] - mat2[i][j];
		}
	}
}

double l2_norm (int const bs, double const mat[bs][bs]) {
	int i, j;
	double sum = 0.0;

	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			sum += mat[i][j] * mat[i][j];
		}
	}

	return sqrt (sum);
}


// performs a check, whether two given blocks are similar
double get_block_distance (block_t* ref_block, block_t* cmp_block, int const std_dev) {
	int const bs = ref_block->block_size;
	double dct_trans[bs][bs];
	double dct_trans_t[bs][bs];
	double ref_mat[bs][bs];
	double cmp_mat[bs][bs];
	double dct_tmp[bs][bs];
	double ref_dct[bs][bs];
	double cmp_dct[bs][bs];
	double sub_mat[bs][bs];
	double distance = 0.0;
	double lambda = 0.82;

	// obtain regarding DCT transformation matrix
	get_dct_matrixes (bs, dct_trans, dct_trans_t);

	// subtract 128 for DCT transformation
	shift_values (bs, ref_block, ref_mat);
	shift_values (bs, cmp_block, cmp_mat);

	// perform DCT on reference block by two matrix multiplications
	matrixmul (bs, dct_trans, ref_mat, dct_tmp);
	matrixmul (bs, dct_tmp, dct_trans_t, ref_dct);

	// perform DCT on compare block by two matrix multiplications
	matrixmul (bs, dct_trans, cmp_mat, dct_tmp);
	matrixmul (bs, dct_tmp, dct_trans_t, cmp_dct);

	// perform thresholding on reference block
	hard_threshold (bs, ref_dct, lambda, std_dev);

	// perform thresholding on compare block
	hard_threshold (bs, cmp_dct, lambda, std_dev);

	// subtract compare block from reference block
	subtract_blocks (bs, ref_dct, cmp_dct, sub_mat);

	// perform L2 norm on the difference of the two matrices
	distance = l2_norm(bs, sub_mat) / (double)bs;

	return distance;
}

// function that make the coversion from YUV to RGB
void yuv2rgb (png_img* img) {
	int i, j, y, u, v, r, g, b;
	png_byte* row;
	png_byte* tmp;

	for (j=0; j<img->height; ++j) {
		row = img->data[j];

		for (i=0; i<img->width; ++i) {
			tmp = &(row[i*3]);

			// extract YUV values
			y = tmp[0];
			u = tmp[1] - 128;
			v = tmp[2] - 128;
			
			// convert pixel elements
			r = limit (y + 1.402*v);
			g = limit (y - 0.3441*u - 0.7141*v);
			b = limit (y + 1.772*u);
			
			// write back YUV values
			tmp[0] = r;
			tmp[1] = g;
			tmp[2] = b;
		}
	}
}

int bm3d (char* const infile, 			// name of input file
			 int const block_size, 			// size of internal processed blocks
			 int const block_step, 			// step size between blocks
			 int const std_dev, 				// standard deviation of noise
			 int const max_blocks,			// maximum number of block in one 3D array
			 int const h_search,				// horizontal width of search window
			 int const v_search) { 			// vertical width of search window
	png_img img;
	char outfile[40];
	// double ref_block[block_size][block_size];
	// double cmp_block[block_size][block_size];
	block_t ref_block;
	block_t cmp_block;
	double d;	// block distance
	// double tau_match = 0.233;
	double tau_match = 0.1; 				// factor to calculate the maximum deviation in %
	int i, j, k, l;
	group_t group = 0;						// group, which holds a set of similar blocks
	list_t list = 0;							// list of groups

	// read input image
	if (png_read(&img, infile) != 0) {
		return 1;
	}

	// control color type
	if (img.color != PNG_COLOR_TYPE_RGB) {
		generate_error ("Wrong color type...");
		return 1;
	}

	// control number of channels
	if (img.channels != 3) {
		generate_error ("Wrong number of channels...");
		return 1;
	}

	// print status information on the console
	printf ("[INFO] ... image dimensions: %dx%d\n", img.width, img.height);
	printf ("[INFO] ... block size: %d\n", block_size);
	printf ("[INFO] ... block step: %d\n", block_step);
	printf ("[INFO] ... sigma: %d\n", std_dev);
	printf ("[INFO] ... maximum number of blocks: %d\n", max_blocks);
	printf ("[INFO] ... horizontal search window size: %d\n", h_search);
	printf ("[INFO] ... vertical search window size: %d\n", v_search);

	// convert colorspace from RGB to YUV
	rgb2yuv (&img);

	// set output filename
	if (get_output_filename (outfile, "img/yuv/", "noisy_yuv", std_dev) != 0) {
		generate_error ("Unable to process output filename...");
		return 1;
	}

	// write output image
	if (png_write(&img, outfile) != 0) {
		return 1;
	}

	// allocate block memory
	if (new_block_struct(block_size, &ref_block) != 0) {
		return 1;
	}
	
	if (new_block_struct(block_size, &cmp_block) != 0) {
		return 1;
	}

	// apply sliding window processing
	/* - iterate over all pixels
		- produces blocks (define max. number of blocks)
		- apply denoising
		- next pixel
	*/

	// compare blocks according to the sliding-window manner
	for (j=0; j<img.height; j=j+block_step) {
		for (i=0; i<img.width; i=i+block_step) {

			// obtain refernce block
			if (((i+(block_size/2) < img.width) && (i-(block_size/2) >= 0)) &&  // block must not exceed horizontal image dimensions
				 ((j+(block_size/2) < img.height) && (j-(block_size/2) >= 0))) { // block must not exceed vertical image dimensions

				if (get_block (&img, 0, &ref_block, i-(block_size/2), j-(block_size/2)) != 0) {
					return 1;
				}
				
				// printf ("\n");
				// printf ("-----------------------------------------------------\n");
				// printf ("new reference block with indices (%d/%d) obtained...\n", i, j);
				// printf ("-----------------------------------------------------\n");
				if (append_block (&group, &ref_block, 0.0) != 0) {
					return 1;
				}

				for (l=0; l<img.height; l=l+block_step) {
					for (k=0; k<img.width; k=k+block_step) {

						// obtain block to compare to reference block
						if (((k+(block_size/2) < img.width) && (k-(block_size/2) >= 0)) &&								// block must not exceed horizontal image dimensions
							 ((l+(block_size/2) < img.height) && (l-(block_size/2) >= 0)) && 								// block must not exceed vertical image dimensions
							 ((k+(block_size/2) <= (i+(h_search/2))) && (k-(block_size/2) > (i-(h_search/2)))) &&	// block must not exceed horizontal search window dimensions
							 ((l+(block_size/2) <= (j+(v_search/2))) && (l-(block_size/2) > (j-(v_search/2)))) &&	// block must not exceed vertical search window dimensions
							 !((i==k) && (j==l))) {																						// must be different from reference block

							if (get_block (&img, 0, &cmp_block, k-(block_size/2), l-(block_size/2)) != 0) {
								return 1;
							}
							// printf ("(%d/%d)\n", k, l);

							// compare blocks for similarity
							d = get_block_distance (&ref_block, &cmp_block, std_dev);
							
							// decide whether block similarity is sufficient
							if (d < tau_match*255) {
								if (append_block (&group, &cmp_block, d) != 0) {
									return 1;
								}
							}
						}
					}
				}

				// add group of similar blocks to list
				if (append_group (&list, &group) != 0) {
					return 1;
				}

				group = 0; //EVIL, cause same pointer
			}
		}
	}

	// if (print_list(list) != 0) {
	// 	return 1;
	// }

	printf ("[INFO] ... nr of groups in list: %d\n", list_length(&list));

	// perform actual denoising of the actual block group (regarding to one ref_block)

	// hard thresholding

	// local estimates

	// Wiener filtering

	// final estimates

	// convert colorspace from YUV back to RGB
	yuv2rgb (&img);

	// set output filename
	if (get_output_filename (outfile, "img/rgb/", "denoised_rgb", std_dev) != 0) {
		generate_error ("Unable to process output filename...");
		return 1;
	}

	// write output image
	if (png_write(&img, outfile) != 0) {
		return 1;
	}

	// free allocated memory
	png_free_mem (&img);

	return 0;
}
