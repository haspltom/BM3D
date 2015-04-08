#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "../png_io/png_io.h"
#include "../error/error.h"
#include "../csv_export/csv_export.h"
#include "../utils/utils.h"
#include "bm3d.h"

//------------ METHODS FOR COLORSPACE CONVERSION ------------
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


//------------ METHODS FOR GENERAL PURPOSE ------------
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


//------------ METHODS FOR BLOCK MATCHING ------------
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
	new_node->weight = 0.0;
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

int print_list (list_t const list, char* const path, char* const prefix) {
	FILE* fd = 0;
	group_node_t* tmp = list;
	node_t* tmp_block;
	char groupname[40];
	int count = 0;

	printf ("[INFO} ... printing groups to file...\n");

	while (tmp != NULL) {
		//obtain output filename
		if (get_output_filename (groupname, path, prefix, "txt", ++count) != 0) {
			generate_error ("Unable to process output filename for group...");
			return 1;
		}

		fd = fopen (groupname, "w");
		
		if (fd == NULL) {
			generate_error ("Unable to open file for printing group...");
			return 1;
		}

		tmp_block = tmp->group;
		fprintf (fd, "[INFO} ... nr of blocks in group: %d\n\n", group_length(&tmp_block));
		fprintf (fd, "[INFO} ... weight of group: %f\n", tmp->weight);
		fprintf (fd, "[INFO} ... reference block...\n");

		while (tmp_block != NULL) {
			print_block (fd, tmp_block->block);
			fprintf (fd, "[INFO} ... distance to reference block: %f\n\n", tmp_block->distance);
			tmp_block = tmp_block->next;
		}

		tmp = tmp->next;

		fclose (fd);
	}

	return 0;
}

void free_group (group_t* group) {
	node_t* tmp = *group;

	while (*group != NULL) {
		tmp = *group;
		*group = tmp->next;
		free (tmp);
	}
}

void dct_1d (int const len, double arr[len]) {
	int i, j;
	double sum = 0.0;
	double tmp[len];

	for (j=0; j<len; ++j) {
		for (i=0; i<len; ++i) {
			sum += arr[i] * (cos((PI/len)*(i+0.5)*j));
		}
		tmp[j] = sum;
		sum = 0.0;
	}
}

void dct_2d (int const len, double arr[len][len]) {
	int i, j, k, l;
	double sum, ai, aj;			// multiplicative constants
	double tmp[len][len];		// dct-value buffer
	double fac;						// result of the cosine multiplication

	for (j=0; j<len; ++j) {
		for (i=0; i<len; ++i) {
			ai = (i == 0) ? 1.0 / sqrt((double)len) : sqrt(2.0/(double)len);
			aj = (j == 0) ? 1.0 / sqrt((double)len) : sqrt(2.0/(double)len);

			for (l=0; l<len; ++l) {
				for (k=0; k<len; ++k) {
					fac = cos((PI/(double)len) * ((double)l + 0.5) * (double)j) * cos((PI/(double)len) * ((double)k + 0.5) * (double)i);
					sum += arr[l][k] * fac;
				}
			}
			tmp[j][i] = ai * aj * sum;
			sum = 0.0;
		}
	}

	// write back to original matrix
	for (j=0; j<len; ++j) {
		for (i=0; i<len; ++i) {
			arr[j][i] = tmp[j][i];
		}
	}
}

void dct_3d (int const len, int const z, double arr[z][len][len]) {
	int i, j, k, l, m, n;
	double sum, ai, aj, ak;
	double tmp[z][len][len];		// dct result
	double fac;

	for (k=0; k<z; ++k) {
		for (j=0; j<len; ++j) {
			for (i=0; i<len; ++i) {
				ai = (i == 0) ? 1.0/sqrt((double)len) : sqrt(2.0/(double)len);
				aj = (j == 0) ? 1.0/sqrt((double)len) : sqrt(2.0/(double)len);
				ak = (k == 0) ? 1.0/sqrt((double)z) : sqrt(2.0/(double)z);

				for (n=0; n<z; ++n) {
					for (m=0; m<len; ++m) {
						for (l=0; l<len; ++l) {
							fac = cos((PI/(double)z)*((double)n+0.5)*(double)k) * cos((PI/(double)len)*((double)m+0.5)*(double)j) * cos((PI/(double)len)*((double)l+0.5)*(double)i);
							sum += arr[n][m][l] * fac;
						}
					}
				}
				tmp[k][j][i] = ai * aj * ak * sum;
				sum = 0.0;
			}
		}
	}

	// write back to original matrix
	for (k=0; k<z; ++k) {
		for (j=0; j<len; ++j) {
			for (i=0; i<len; ++i) {
				arr[k][j][i] = tmp[k][j][i];
			}
		}
	}
}

void hard_threshold_2d (int const bs, double mat[bs][bs], double const th_2d, int const sigma) {
	int i, j;
	double threshold = th_2d * (double)sigma * sqrt(2.0*log(bs*bs));
	
	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			mat[j][i] = (abs(mat[j][i])>threshold) ? mat[j][i] : 0.0;
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
double get_block_distance (block_t* ref_block, block_t* cmp_block, int const sigma) {
	int const bs = ref_block->block_size;
	double ref_mat[bs][bs];
	double cmp_mat[bs][bs];
	double sub_mat[bs][bs];
	double distance = 0.0;
	double th_2d = 0.82;

	// subtract 128 for DCT transformation
	shift_values (bs, ref_block, ref_mat);
	shift_values (bs, cmp_block, cmp_mat);

	// perform DCT on reference block by two matrix multiplications
	dct_2d (bs, ref_mat);

	// perform DCT on compare block by two matrix multiplications
	dct_2d (bs, cmp_mat);

	// perform thresholding on reference block
	hard_threshold_2d (bs, ref_mat, th_2d, sigma);

	// perform thresholding on compare block
	hard_threshold_2d (bs, cmp_mat, th_2d, sigma);

	// subtract compare block from reference block
	subtract_blocks (bs, ref_mat, cmp_mat, sub_mat);

	// perform L2 norm on the difference of the two matrices
	distance = l2_norm(bs, sub_mat) / (double)bs;

	return distance;
}


//------------ METHODS FOR DENOISING ------------
node_t* get_previous_block (group_t group, node_t* block) {
	node_t* prev = group;

	while ((prev != NULL) && (prev->next != block)) {
		prev = prev->next;
	}

	return prev;
}

int trim_group (group_t* group, unsigned int max_blocks) {
	node_t* tmp_block = *group;
	node_t* prev;
	unsigned int diff = group_length(group) - max_blocks;;
	int i;

	if (group_length(group) <= max_blocks) {
		return 0;
	}

	// go to the end of the group
	while (tmp_block != NULL) {
		prev = tmp_block;
		tmp_block = tmp_block->next;
	}

	// delete obsolete blocks
	while (diff > 0) {
		tmp_block = prev;
		prev = get_previous_block (*group, tmp_block);

		for (i=0; i<tmp_block->block.block_size; ++i) {
			free (tmp_block->block.data[i]);
		}

		free (tmp_block->block.data);
		free (tmp_block);
		tmp_block = NULL;
		prev->next = NULL;

		--diff;
	}

	return 0;
}

int trim_list (list_t* list, unsigned int const max_blocks) {
	group_node_t* tmp = *list;

	// go through all groups
	while (tmp->next != NULL) {
		if (trim_group(&tmp->group, max_blocks) != 0) {
			return 1;
		}

		tmp = tmp->next;
	}

	return 0;
}

void group2array (group_t* group, unsigned int len, unsigned const z, double arr[z][len][len]) {
	node_t* tmp = *group;
	int i, j, k;

	while (tmp != NULL) {
		for (k=0; k<z; ++k) {
			for (j=0; j<len; ++j) {
				for (i=0; i<len; ++i) {
					arr[k][j][i] = tmp->block.data[j][i];
				}
			}
			tmp = tmp->next;
		}
	}
}

void array2group (group_t* group, unsigned int len, unsigned const z, double arr[z][len][len]) {
	node_t* tmp = *group;
	int i, j, k;

	while (tmp != NULL) {
		for (k=0; k<z; ++k) {
			for (j=0; j<len; ++j) {
				for (i=0; i<len; ++i) {
					tmp->block.data[j][i] = arr[k][j][i];
				}
			}
			tmp = tmp->next;
		}
	}
}

void hard_threshold_3d (int const bs, int const z, double mat[z][bs][bs], double const th_3d, int const sigma) {
	int i, j, k;
	double threshold = th_3d * (double)sigma * sqrt(2.0*log(bs*bs));
	
	for (k=0; k<z; ++k) {
		for (j=0; j<bs; ++j) {
			for (i=0; i<bs; ++i) {
				mat[k][j][i] = (abs(mat[k][j][i])>threshold) ? mat[k][j][i] : 0.0;
			}
		}
	}
}

double get_weight (int const bs, int const z, double mat[z][bs][bs]) {
	int i, j, k;
	int count = 0;

	for (k=0; k<z; ++k) {
		for (j=0; j<bs; ++j) {
			for (i=0; i<bs; ++i) {
				if (mat[k][j][i] != 0.0) {
					++count;
				}
			}
		}
	}

	return (count >= 1) ? 1.0/(double)count : 1.0;
}

void array2file (FILE* fd, int const len, int const z, double arr[z][len][len], char* const header) {
	int i, j, k;

	fprintf (fd, "[INFO] ... .............................................\n");
	fprintf (fd, "[INFO] ... %s\n", header);
	fprintf (fd, "[INFO] ... .............................................\n\n");

	for (k=0; k<z; ++k) {
		for (j=0; j<len; ++j) {
			for (i=0; i<len; ++i) {
				fprintf (fd, "%f ", arr[k][j][i]);
			}
			fprintf (fd, "\n");
		}
		fprintf (fd, "\n\n");
	}
	fprintf (fd, "\n\n\n");
}

int determine_estimates (list_t const list, int const sigma) {
	FILE* fd = 0;
	char outfile[40];
	group_node_t* tmp = list;
	node_t* group;
	unsigned int z;
	double th_3d = 0.75;
	int count = 0;

	while (tmp != NULL) {
		group = tmp->group;
		z = group_length (&group);
		unsigned int len = group->block.block_size;
		double arr[z][len][len];

		//obtain output filename
		if (get_output_filename (outfile, "dns/", "group", "txt", ++count) != 0) {
			generate_error ("Unable to process output filename for group...");
			return 1;
		}

		fd = fopen (outfile, "a");
		
		if (fd == NULL) {
			generate_error ("Unable to open file for printing group...");
			return 1;
		}

		// build a 3D-array from the actual group
		group2array (&group, len, z, arr);

		// append extracted group to log-file
		array2file (fd, len, z, arr, "extracted group");

		// perform 3D-DCT
		dct_3d (len, z, arr);

		// append transformed group to log-file
		array2file (fd, len, z, arr, "group after 3D-DCT transformation");

		// perform 3D-hard-thresholding
		hard_threshold_3d (len, z, arr, th_3d, sigma);

		// append thresholded group to log-file
		array2file (fd, len, z, arr, "group after 3D-hard-thresholding");

		// calculate the weight for the actual block
		tmp->weight = get_weight (len, z, arr);	

		// write array data back to a list node
		array2group (&group, len, z, arr);

		tmp = tmp->next;

		fclose (fd);
	}

	// probably need more variables in interface
	return 0;
}

int bm3d (char* const infile, 			// name of input file
			 int const block_size, 			// size of internal processed blocks
			 int const block_step, 			// step size between blocks
			 int const sigma, 				// standard deviation of noise
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
	clock_t bm_start, bm_end;				// time variables for block matching start and end
	double time;

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
	printf ("[INFO] ... .............................................\n");
	printf ("[INFO] ... image dimensions: %dx%d\n", img.width, img.height);
	printf ("[INFO] ... block size: %d\n", block_size);
	printf ("[INFO] ... block step: %d\n", block_step);
	printf ("[INFO] ... sigma: %d\n", sigma);
	printf ("[INFO] ... maximum number of blocks: %d\n", max_blocks);
	printf ("[INFO] ... horizontal search window size: %d\n", h_search);
	printf ("[INFO] ... vertical search window size: %d\n", v_search);
	printf ("[INFO] ... .............................................\n\n");

	// convert colorspace from RGB to YUV
	printf ("[INFO] ... launch of color conversion...\n");
	rgb2yuv (&img);

	// set filename for noisy yuv output image
	if (get_output_filename (outfile, "img/yuv/", "noisy_yuv", "png", sigma) != 0) {
		generate_error ("Unable to process output filename...");
		return 1;
	}

	// write output image
	if (png_write(&img, outfile) != 0) {
		return 1;
	}

	printf ("[INFO] ... end of color conversion...\n\n");

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

	printf ("[INFO] ... launch of block-matching...\n");
	bm_start = clock();

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
							d = get_block_distance (&ref_block, &cmp_block, sigma);
							
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

	if (print_list(list, "grp/org/", "group") != 0) {
		return 1;
	}

	bm_end = clock();
	time = (bm_end - bm_start) / (double)CLOCKS_PER_SEC;

	// set filename for txt-file of groups
	if (get_output_filename (outfile, "bms/", "block_matching_statistics", "csv", block_size) != 0) {
		generate_error ("Unable to process output filename...");
		return 1;
	}

	// add number of groups and computation time to txt-file for statistical evaluation
	if (add_csv_line(outfile, block_step, list_length(&list), time) != 0) {
		generate_error ("Unable to add values to csv-file...");
		return 1;
	}

	printf ("[INFO] ... end of block-matching...\n");
	printf ("[INFO] ... number of groups in list: %d\n", list_length(&list));
	printf ("[INFO] ... elapsed time: %f\n\n", time);

	// perform actual denoising of the actual block group (regarding to one ref_block)
	printf ("[INFO] ... launch of denoising...\n");

	// trim groups to maximal number of blocks
	printf ("[INFO] ... trimming blocks to maximum size...\n");
	if (trim_list(&list, max_blocks) != 0) {
		return 1;
	}

	if (print_list(list, "grp/trm/", "group") != 0) {
		return 1;
	}

	printf ("[INFO] ... determining local estimates...\n");
	// hard thresholding
	if (determine_estimates(list, sigma) != 0) {
		return 1;
	}

	if (print_list(list, "grp/est/", "group") != 0) {
		return 1;
	}

	// local estimates

	// Wiener filtering

	// final estimates
	printf ("[INFO] ... end of denoising...\n\n");

	// convert colorspace from YUV back to RGB
	printf ("[INFO] ... launch of color conversion...\n");
	yuv2rgb (&img);

	// set filename for denoised rgb output image
	if (get_output_filename (outfile, "img/rgb/", "denoised_rgb", "png", sigma) != 0) {
		generate_error ("Unable to process output filename...");
		return 1;
	}

	// write output image
	if (png_write(&img, outfile) != 0) {
		return 1;
	}

	printf ("[INFO] ... end of color conversion...\n\n");

	// free allocated memory
	png_free_mem (&img);

	return 0;
}
