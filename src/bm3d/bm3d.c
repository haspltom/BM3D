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

// --------------------------------------------------------------------------
// METHODS FOR GENERAL PURPOSES
// --------------------------------------------------------------------------
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
	block_node_t* tmp = *group;

	while (tmp != NULL) {
		++len;
		tmp = tmp->next;
	}

	return len;
}

void free_group (group_t* group) {
	block_node_t* tmp = *group;

	while (*group != NULL) {
		tmp = *group;
		*group = tmp->next;
		free (tmp);
	}
}

int img2array (png_img* img, int const channel) {
	FILE* fd;
	char name[10];
	char outfile[30];
	int array[img->width][img->height];
	int i, j;
	png_byte* row;
	png_byte* tmp;

	// select filename
	switch (channel) {
		case 0:
			sprintf (name, "y_array");
			break;
		case 1:
			sprintf (name, "u_array");
			break;
		case 2:
			sprintf (name, "v_array");
			break;
		default:
			generate_error ("Invalid channel for array building...");
			return 1;
			break;
	}

	// obtain entire output filename
	if (get_output_filename (outfile, "dns/", name, "txt", 0) != 0) {
		generate_error ("Unable to process output filename for group...");
		return 1;
	}

	fd = fopen (outfile, "w");
	
	if (fd == NULL) {
		generate_error ("Unable to open file for printing group...");
		return 1;
	}

	// extract pixel values
	for (j=0; j<img->height; ++j) {
		row = img->data[j];

		for (i=0; i<img->width; ++i) {
			tmp = &(row[i*3]);

			array[j][i] = tmp[channel];
		}
	}

	// write array to file
	for (j=0; j<img->height; ++j) {
		for (i=0; i<img->width; ++i) {
			fprintf (fd, "%d ", array[j][i]);
		}
		fprintf (fd, "\n");
	}

	fclose (fd);

	return 0;
}

// --------------------------------------------------------------------------
// METHODS FOR FILE-WRITING
// --------------------------------------------------------------------------
void print_block (FILE* fd, block_t const block) {
	int i, j;

	for (j=0; j<block.block_size; ++j) {
		for (i=0; i<block.block_size; ++i) {
			fprintf (fd, "%f ", block.data[i][j]);
		}
		fprintf (fd, "\n");
	}
}

int print_list (list_t const list, char* const path, char* const prefix) {
	FILE* fd = 0;
	group_node_t* tmp = list;
	block_node_t* tmp_block;
	char groupname[40];
	int count = 0;

	printf ("[INFO] ... printing groups to file...\n");

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
		fprintf (fd, "[INFO] ... nr of blocks in group: %d\n", group_length(&tmp_block));

		if (strstr(path, "est") != NULL) {
			fprintf (fd, "[INFO] ... weight of group: %f\n", tmp->weight);
		}

		fprintf (fd, "\n");
		fprintf (fd, "[INFO] ... reference block...\n");

		while (tmp_block != NULL) {
			print_block (fd, tmp_block->block);
			fprintf (fd, "[INFO] ... block position: (%d/%d)\n", tmp_block->block.x, tmp_block->block.y);
			fprintf (fd, "[INFO] ... distance to reference block: %f\n\n", tmp_block->distance);
			tmp_block = tmp_block->next;
		}

		tmp = tmp->next;

		fclose (fd);
	}

	return 0;
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

int d_buf2file (unsigned int const width, unsigned int const height, double buf[width][height], char* const path, char* const prefix) {
	FILE* fd = 0;
	char outfile[40];
	int i, j;

	//obtain output filename
	if (get_output_filename (outfile, path, prefix, "txt", 0) != 0) {
		generate_error ("Unable to process output filename for buffer...");
		return 1;
	}

	fd = fopen (outfile, "w");
	
	if (fd == NULL) {
		generate_error ("Unable to open file for printing buffer...");
		return 1;
	}

	fprintf (fd, "[INFO] ... .............................................\n");
	fprintf (fd, "[INFO] ... %s\n", prefix);
	fprintf (fd, "[INFO] ... .............................................\n\n");

	for (j=0; j<height; ++j) {
		for (i=0; i<width; ++i) {
			fprintf (fd, "%f ", buf[j][i]);
		}
		fprintf (fd, "\n");
	}

	fclose (fd);

	return 0;
}

int i_buf2file (unsigned int const width, unsigned int const height, int const buf[width][height], char* const path, char* const prefix) {
	FILE* fd = 0;
	char outfile[40];
	int i, j;

	//obtain output filename
	if (get_output_filename (outfile, path, prefix, "txt", 0) != 0) {
		generate_error ("Unable to process output filename for buffer...");
		return 1;
	}

	fd = fopen (outfile, "w");
	
	if (fd == NULL) {
		generate_error ("Unable to open file for printing buffer...");
		return 1;
	}

	fprintf (fd, "[INFO] ... .............................................\n");
	fprintf (fd, "[INFO] ... %s\n", prefix);
	fprintf (fd, "[INFO] ... .............................................\n\n");

	for (j=0; j<height; ++j) {
		for (i=0; i<width; ++i) {
			fprintf (fd, "%d ", buf[j][i]);
		}
		fprintf (fd, "\n");
	}

	fclose (fd);

	return 0;
}


// --------------------------------------------------------------------------
// METHODS FOR BLOCK-MATCHING
// --------------------------------------------------------------------------
int exceeds_image_dimensions (int const width, int const height, int const bs, int const x, int const y) {
	if (((x+(bs/2) < width) && (x-(bs/2) >= 0)) && ((y+(bs/2) < height) && (y-(bs/2) >= 0))) { 
		return 0;
	}

	return 1;
}

int exceeds_search_window (int const width, int const height, int const bs, int const h_search, int const v_search, int const i, int const j, int const k, int const l) {
	if ((((k+(bs/2) <= (i+(h_search/2))) && (k-(bs/2) > (i-(h_search/2)))) || 	// block must not exceed horizontal search window dimensions
		  ((i <= (h_search/2)) && (k+(bs/2) <= h_search)) || 							// case if horizontal dimension is at the left border
		  ((i > (width-(h_search/2)) && (k-(bs/2) > (width-h_search))))) &&		// case if horizontal dimension is at the right border
		 (((l+(bs/2) <= (j+(v_search/2))) && (l-(bs/2) > (j-(v_search/2)))) || 	// block must not exceed vertical search window dimensions
		  ((j <= (v_search/2)) && (l+(bs/2) <= v_search)) || // case if vertical dimension is at the upper border
		  (((j > (height-(v_search/2))) && (l-(bs/2) > (height-v_search)))))) {
			 return 0;
		 }

	return 1;
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

			block->data[j][i] = (double)tmp[channel];
		}
	}

	block->x = x + (block->block_size/2);
	block->y = y + (block->block_size/2);
	
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
	block_node_t* tmp = *group;
	block_node_t* new_node;
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
	
	new_node = (block_node_t*)malloc(sizeof(block_node_t)); //MISTAKE: allocated block_node_t* instead of block_node_t
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

void mark_search_window (png_img* img, block_t* block, int const h_search, int const v_search) {
	png_byte* row;
	png_byte* tmp;
	int i, j;
	int h_start, v_start;

	if ((block->x - (h_search/2)) < 0) {
		h_start = 0;
	}
	else if ((block->x + (h_search/2)) > img->width-1) {
		h_start = img->width - h_search;
	}
	else {
		h_start = block->x - (h_search/2) + 1;
	}

	if ((block->y - (v_search/2)) < 0) {
		v_start = 0;
	}
	else if ((block->y + (v_search/2)) > img->height-1) {
		v_start = img->height - v_search;
	}
	else {
		v_start = block->y - (v_search/2) + 1;
	}

	for (j=v_start; j<(v_start+v_search); ++j) {
		row = img->data[j];

		for (i=h_start; i<(h_start+h_search); ++i) {
			tmp = &(row[i*3]);

			if (i==h_start || i==(h_start+h_search-1) || j==v_start || j==(v_start+v_search-1)) {
				tmp[0] = 0;
				tmp[1] = 0;
				tmp[2] = 0;
			}
		}
	}
}

void mark_ref_block (png_img* img, block_t* block) {
	png_byte* row;
	png_byte* tmp;
	int i, j;
	int x = block->x - (block->block_size/2);
	int y = block->y - (block->block_size/2);

	for (j=0; j<block->block_size; ++j) {
		row = img->data[j+y];

		for (i=0; i<block->block_size; ++i) {
			tmp = &(row[(i+x)*3]);

			tmp[0] = 0;
			tmp[1] = 0;
			tmp[2] = 0;
		}
	}
}

void mark_cmp_block (png_img* img, block_t* block) {
	png_byte* row;
	png_byte* tmp;
	int i, j;
	int bs = block->block_size;
	int x = block->x - (bs/2);
	int y = block->y - (bs/2);

	for (j=0; j<block->block_size; ++j) {
		row = img->data[j+y];

		for (i=0; i<block->block_size; ++i) {
			tmp = &(row[(i+x)*3]);

			if (i==0 || i==bs-1 || j==0 || j==bs-1) {
				tmp[0] = 0;
				tmp[1] = 0;
				tmp[2] = 0;
			}
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

void hard_threshold_2d (int const bs, double mat[bs][bs], double const th_2d, int const sigma) {
	int i, j;
	double threshold = th_2d * (double)sigma * sqrt(2.0*log(bs*bs));

	// printf ("%f\n", threshold);
	
	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			mat[j][i] = (abs(mat[j][i])>threshold) ? mat[j][i] : 0.0;
		}
	}
}

void subtract_blocks (int const bs, double const mat1[bs][bs], double const mat2[bs][bs], double res[bs][bs]) {
	int i, j;

	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			// res[i][j] = limit (mat1[i][j] - mat2[i][j]);
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
double get_block_distance (block_t* ref_block, block_t* cmp_block, int const sigma, double const th_2d) {
	int const bs = ref_block->block_size;
	double ref_mat[bs][bs];
	double cmp_mat[bs][bs];
	double sub_mat[bs][bs];
	double distance = 0.0;

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

// method, which performs the block-matching for a given channel
int block_matching (char* const kind,
						  png_img* img,
						  png_img* tmp,
						  unsigned int const b_size,
						  unsigned int const b_step,
						  unsigned int const sigma,
						  unsigned int const h_search,
						  unsigned int const v_search,
						  double const th_2d,
						  double const tau_match,
						  unsigned int const channel,
						  unsigned int block_marking,
						  list_t* list) {
	int i, j, k, l;
	block_t ref_block;
	block_t cmp_block;
	double d;	// block distance
	group_t group = 0;						// group, which holds a set of similar blocks
	int count = 0;
	char path[20];								// path is set according to 'ht' or 'wnr'
	char outfile[40];							// universally used output-filename

	// allocate block memory
	if (new_block_struct(b_size, &ref_block) != 0) {
		return 1;
	}
	
	if (new_block_struct(b_size, &cmp_block) != 0) {
		return 1;
	}

				double dmin = 100.0;
				double dmax = 0.0;
				double dsum = 0.0;
				int dnum = 0;


	// compare blocks according to the sliding-window manner
	for (j=0; j<img->height; j=j+b_step) {
		for (i=0; i<img->width; i=i+b_step) {

			// obtain refernce block
			if (!exceeds_image_dimensions(img->width, img->height, b_size, i, j)) { 

				if (get_block (img, channel, &ref_block, i-(b_size/2), j-(b_size/2)) != 0) {
					return 1;
				}
				
				// printf ("\n");
				// printf ("-----------------------------------------------------\n");
				// printf ("new reference block with indices (%d/%d) obtained...\n", i, j);
				// printf ("-----------------------------------------------------\n");
				if (append_block (&group, &ref_block, 0.0) != 0) {
					return 1;
				}
				
				if (block_marking) {
					png_copy_values (tmp, img);
					mark_search_window (tmp, &ref_block, h_search, v_search);
					mark_ref_block (tmp, &ref_block);
				}

				for (l=0; l<img->height; l=l+b_step) {
					for (k=0; k<img->width; k=k+b_step) {

						// obtain block to compare to reference block
						if (!(exceeds_image_dimensions(img->width, img->height, b_size, k, l)) && 								
							 !(exceeds_search_window(img->width, img->height, b_size, h_search, v_search, i, j, k, l)) &&
							 !((i==k) && (j==l))) {			// must be different from reference block

							if (get_block (img, channel, &cmp_block, k-(b_size/2), l-(b_size/2)) != 0) {
								return 1;
							}
							// printf ("(%d/%d)\n", k, l);

							// compare blocks for similarity
							d = get_block_distance (&ref_block, &cmp_block, sigma, th_2d);
							dmin = (d < dmin) ? d : dmin;
							dmax = (d > dmax) ? d : dmax;
							dsum += d;
							++dnum;
							// if (!strcmp(kind, "ht")) printf ("distance: %f\ntaumatch: %f\n", d, tau_match*255);
							
							// decide whether block similarity is sufficient
							if (d < tau_match*255) {
								if (append_block (&group, &cmp_block, d) != 0) {
									return 1;
								}
								// if (!strcmp(kind, "wnr")) printf ("block appended...\n");

								if (block_marking) {
									mark_cmp_block (tmp, &cmp_block);
								}
							}
						}
					}
				}


				// add group of similar blocks to list
				if (append_group (list, &group) != 0) {
					return 1;
				}

				if (block_marking) {
					// write image with marked group in it
					// sprintf (path, "img/yuv/grp/%s/", kind);
					sprintf (path, "img/yuv/grp/");

					// set filename for noisy yuv output image
					if (get_output_filename (outfile, path, "group", "png", ++count) != 0) {
						generate_error ("Unable to process output filename...");
						return 1;
					}

					// write output image
					yuv2rgb(tmp);
					if (png_write(tmp, outfile) != 0) {
						return 1;
					}
				}

				group = 0; //EVIL, cause same pointer
			}
		}
	}
				printf ("dmin: %f\ndmax: %f\ndavg: %f\n", dmin, dmax, dsum/((double)dnum));

	return 0;
}

// --------------------------------------------------------------------------
// METHODS FOR INTERMEDIATE STEPS
// --------------------------------------------------------------------------
int get_chrom (png_img* img, list_t* ylist, list_t* ulist, list_t* vlist) {
	group_node_t* tmp = *ylist;
	block_node_t* group;
	block_t* block;
	double w;							// weight of actual processed group
	int x, y, bs;
	double d;
	group_t u_group = 0;				// group, which holds a set of similar blocks
	group_t v_group = 0;				// group, which holds a set of similar blocks
	block_t tmp_block;

	// validate input list
	if (tmp == NULL) {
		generate_error ("Invalid reference list for extracting chrominance channels...");
		return 1;
	}

	// allocate block memory
	if (new_block_struct(tmp->group->block.block_size, &tmp_block) != 0) {
		return 1;
	}

	// go through all groups
	while (tmp->next != NULL) {
		group = tmp->group;
		w = tmp->weight;

		// iterate over all blocks within the actual group
		while (group != NULL) {
			block = &group->block;
			x = block->x;
			y = block->y;
			bs = block->block_size;
			d = group->distance;

			// obtain block data for the u-channel
			if (get_block (img, 1, &tmp_block, x-(bs/2), y-(bs/2)) != 0) {
				return 1;
			}
				
			// append block data to the regarding group
			if (append_block (&u_group, &tmp_block, d) != 0) {
				return 1;
			}

			// obtain block data for the v-channel
			if (get_block (img, 2, &tmp_block, x-(bs/2), y-(bs/2)) != 0) {
				return 1;
			}
				
			// append block data to the regarding group
			if (append_block (&v_group, &tmp_block, d) != 0) {
				return 1;
			}

			group = group->next;
		}

		// add extracted groups to regarding lists
		if (append_group (ulist, &u_group) != 0) {
			return 1;
		}

		if (append_group (vlist, &v_group) != 0) {
			return 1;
		}

		u_group = 0; //EVIL, cause same pointer
		v_group = 0; //EVIL, cause same pointer
		tmp = tmp->next;
	}
	// TODO delete dynamic memory

	return 0;
}

block_node_t* get_previous_block (group_t group, block_node_t* block) {
	block_node_t* prev = group;

	while ((prev != NULL) && (prev->next != block)) {
		prev = prev->next;
	}

	return prev;
}

int trim_group (group_t* group, unsigned int max_blocks) {
	block_node_t* tmp_block = *group;
	block_node_t* prev;
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

	// validate input list
	if (tmp == NULL) {
		generate_error ("Invalid reference list for trimming list...");
		return 1;
	}

	// go through all groups
	while (tmp->next != NULL) {
		if (trim_group(&tmp->group, max_blocks) != 0) {
			return 1;
		}

		tmp = tmp->next;
	}

	return 0;
}


// --------------------------------------------------------------------------
// METHODS FOR DENOISING
// --------------------------------------------------------------------------
void group2array (group_t* group, unsigned int len, unsigned const z, double arr[z][len][len]) {
	block_node_t* tmp = *group;
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
	block_node_t* tmp = *group;
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

// shrinkage function for averaging TODO
void average_3d (int const bs, int const z, double mat[z][bs][bs]) {
	int i, j, k;
	double avg[bs][bs];
	double sum = 0.0;
	
	// calculate average values
	for (k=0; k<bs; ++k) {
		for (j=0; j<bs; ++j) {
			for (i=0; i<z; ++i) {
				sum += mat[i][j][k];
			}
			avg[j][k] = sum / (double)z;
			sum = 0.0;
		}
	}

	// write average values to group
	for (k=0; k<z; ++k) {
		for (j=0; j<bs; ++j) {
			for (i=0; i<bs; ++i) {
				mat[k][j][i] = avg[j][i];
			}
		}
	}
}

// shrinkage function for hard-thresholding
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

// shrinkage function for averaging TODO
void wiener_3d (int const bs, int const z, double mat[z][bs][bs], double const th_3d, int const sigma) {
}

// determines the weight for a given group after shrinkage with averaging
double get_weight_avg (group_t* group) {
	int i, j, k;
	int count = 0;

	return (count >= 1) ? 1.0/(double)count : 1.0;
}

// determines the weight for a given group after shrinkage with hard-thresholding
double get_weight_ht (int const bs, int const z, double mat[z][bs][bs]) {
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

int shrinkage (char* const kind, list_t* list, int const sigma, int const channel) {
	FILE* fd = 0;
	char outfile[40];
	char path[30];
	char ch;
	group_node_t* tmp = *list;
	block_node_t* group;
	unsigned int z;
	double th_3d = 0.15;
	int count = 0;

	// obtain letter for channel
	ch = (channel==0) ? 'y' : (channel==1) ? 'u' : (channel==2) ? 'v' : 0;

	// determine path for file-writing
	sprintf (path, "dns/%s/%c/grp/", kind, ch);

	while (tmp != NULL) {
		group = tmp->group;
		z = group_length (&group);
		unsigned int len = group->block.block_size;
		double arr[z][len][len];

		//obtain output filename
		if (get_output_filename (outfile, path, "group", "txt", ++count) != 0) {
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

		// perform 3D-hard-thresholding
		if (!strcmp(kind, "avg")) {
			average_3d (len, z, arr);
		}
		else if (!strcmp(kind, "ht")) {
			// perform 3D-DCT
			dct_3d (len, z, arr);

			// append transformed group to log-file
			array2file (fd, len, z, arr, "group after 3D-DCT transformation");

			hard_threshold_3d (len, z, arr, th_3d, sigma);
		}
		else if (!strcmp(kind, "wnr")) {
			wiener_3d (len, z, arr, th_3d, sigma);
		}
		else if (!strcmp(kind, "none")) {
			// do nothing
		}
		else {
			generate_error ("Invalid kind of shrinkage...");
			return 1;
		}

		//append thresholded group to log-file
		array2file (fd, len, z, arr, "group after 3D-shrinkage-operation");

		// calculate the weight for the actual block
		if (!strcmp(kind, "none")) {
			tmp->weight = 1.0;
		}
		if (!strcmp(kind, "avg")) {
			tmp->weight = get_weight_avg (&group);
		}
		else {
			tmp->weight = get_weight_ht (len, z, arr);	
		}

		if (!strcmp(kind, "ht")) {
			// perform 3D-IDCT
			idct_3d (len, z, arr);

			// append inverse-transformed group to log-file
			array2file (fd, len, z, arr, "group after 3D-IDCT transformation");
		}

		// write array data back to a list node
		array2group (&group, len, z, arr);

		tmp = tmp->next;

		fclose (fd);
	}

	// probably need more variables in interface
	return 0;
}

// --------------------------------------------------------------------------
// METHOD FOR AGGREGATING
// --------------------------------------------------------------------------
int aggregate(char* const kind, png_img* img, list_t* list, unsigned int channel) {
	group_node_t* tmp = *list;
	block_node_t* group;
	block_t* block;
	double w;							// weight of actual processed group
	double ebuf[img->height][img->width];
	double wbuf[img->height][img->width];
	int estbuf[img->height][img->width];
	int i, j;
	int x, y, bs;
	png_byte* row;
	png_byte* pix;
	char path[20];
	int yindex, xindex = 0;


	while (tmp != NULL) {
		group = tmp->group;
		w = tmp->weight;
		// if (block->x==15 && block->y==15) printf ("x,y: %d %d\n", block->x, block->y);
		
		while (group != NULL) {
			block = &group->block;
			bs = block->block_size;
			x = block->x - (bs/2);
			y = block->y - (bs/2);
			// if (x==8 && y==8) printf ("x,y: %d %d\nw: %f\n", x, y, tmp->weight);

			// iterate over current block and extract values
			for (j=0; j<bs; ++j) {
				for (i=0; i<bs; ++i) {
					// if (block->x==14 && block->y==14) {
						// printf ("block->data: %f\n", block->data[j][i]);
						yindex = j+y;
						xindex = i+x;
						ebuf[yindex][xindex] += block->data[j][i] * tmp->weight;
						wbuf[yindex][xindex] += tmp->weight;
						if (xindex==174 && yindex==98) {
							printf ("block_data: %f\n", block->data[j][i]);
							printf ("weight: %f\n", tmp->weight);
						}
						// printf ("(%d,%d) ", xindex, yindex);
						// ebuf[j][i] += block->data[j][i];
						// printf ("ebuf: %f\n", ebuf[j][i]);
					// }
				}
				// if (block->x==15 && block->y==15) printf ("\n");
			}
			// if (block->x==15 && block->y==15) printf ("\n");
			// printf ("\n");

			group = group->next;
		}

		tmp = tmp->next;
	}

	switch (channel) {
		case 0:
			sprintf (path, "dns/%s/y/", kind);
			break;
		case 1:
			sprintf (path, "dns/%s/u/", kind);
			break;
		case 2:
			sprintf (path, "dns/%s/v/", kind);
			break;
		default:
			break;
	}

	if (d_buf2file(img->width, img->height, ebuf, path, "ebuf") != 0) {
		return 1;
	}	
	
	if (d_buf2file(img->width, img->height, wbuf, path, "wbuf") != 0) {
		return 1;
	}	

	int maxest = 0;
	int minest = 255;

	// determine estimates by dividing ebuf with wbuf
	for (j=0; j<img->height; ++j) {
		for (i=0; i<img->width; ++i) {
			if ((ebuf[j][i] != 0.0) && (wbuf[j][i] != 0.0)) {
				estbuf[j][i] = ebuf[j][i] / wbuf[j][i];
				if (estbuf[j][i] < 0) {
					printf ("faulty value %d at position (%d/%d)\n", estbuf[j][i], i, j);
					printf ("ebuf: %f wbuf: %f\n\n", ebuf[j][i], wbuf[j][i]);
				}
				// printf ("estbuf[%d][%d]: %d\n", j, i, estbuf[j][i]);
				maxest = (estbuf[j][i] > maxest) ? estbuf[j][i] : maxest;
				minest = (estbuf[j][i] < minest) ? estbuf[j][i] : minest;
			}
		}
	}

	printf ("max est: %d\n", maxest);
	printf ("min est: %d\n", minest);

	// write buffer with local estimates to file
	if (i_buf2file(img->width, img->height, estbuf, path, "estimates") != 0) {
		return 1;
	}	

	// write local estimates back to image
	for (j=0; j<img->height; ++j) {
		row = img->data[j];

		for (i=0; i<img->width; ++i) {
			pix = &(row[i*3]);
			if (estbuf[j][i] != 0) {
				// printf ("org: %d   est: %d\n", pix[channel], estbuf[j][i]);
				// printf ("pixel position: %d/%d\n", i, j);
				pix[channel] = estbuf[j][i];
				// pix[channel] = pix[channel];
			}
			// pix[channel] = (estbuf[j][i] != 0) ? estbuf[j][i] : pix[channel];
		}
	}
	
	return 0;
}

//					// Wiener Stuff -----------------------------------------------
//					int mean_2d(unsigned int const bs, block_t* block, double res[bs], unsigned int const dim) {
//						double sum = 0.0;
//						int i, j;
//					
//						// check dimension parameter
//						if (!((dim==1) || (dim==2))) {
//							generate_error ("Wrong dimension for mean calculation...");
//							return 1;
//						}
//					
//						for (j=0; j<bs; ++j) {
//							for (i=0; i<bs; ++i) {
//								sum += (dim==1) ? block->data[i][j] : block->data[j][i];
//							}
//							res[j] = sum / (double)bs;
//							sum = 0.0;
//						}
//					
//						return 0;
//					}
//					
//					int subtract_mean (unsigned int const bs, block_t* block, double res[bs][bs], unsigned int const dim) {
//						int i, j;
//						double mean[bs];
//					
//						// obtain mean values
//						if (mean_2d(bs, block, mean, dim) != 0) {
//							return 1;
//						}
//					
//						for (j=0; j<bs; ++j) {
//							for (i=0; i<bs; ++i) {
//								if (dim == 1) {
//									res[i][j] = limit (block->data[i][j] - mean[j]);
//								}
//								else if (dim == 2) {
//									res[j][i] = limit (block->data[j][i] - mean[j]);
//								}
//								else {
//									generate_error ("Wrong dimension for mean calculation...");
//									return 1;
//								}
//							}
//						}
//					
//						return 0;
//					}

int bm3d (char* const infile, 			// name of input file
			 char* const kind, 				// kind of shrinkage (ht, wnr, avg)
			 int const block_size, 			// size of internal processed blocks
			 int const block_step, 			// step size between blocks
			 int const sigma, 				// standard deviation of noise
			 int const max_blocks,			// maximum number of block in one 3D array
			 int const h_search,				// horizontal width of search window
			 int const v_search, 			// vertical width of search window
			 double const th_2d,				// threshold for the 2D transformation
			 double const tau_match, 			// match value for block-matching
			 double const th_3d) {			// threshold for the 3D transformtaion
	png_img img;								// noisy input image
	png_img tmp;								// temporary image for marking the blocks
	// png_img est;								// estimate-image after hard-thresholding
	char outfile[40];							// universally used output-filename
	char path[30];
	char prefix[20];
	list_t y_list = 0;					// list of groups	of the y-channel
	list_t u_list = 0;					// list of groups of the u-channel
	list_t v_list = 0;					// list of groups of the v-channel
	clock_t bm_start, bm_end;				// time variables for block matching start and end
	double time;

	// ----------------------------------------------------------------------
	// INPUT READING AND VALIDATION
	// ----------------------------------------------------------------------
	// read input image
	if (png_read(&img, infile) != 0) {
		return 1;
	}

	// read temporary image
	if (png_read(&tmp, infile) != 0) {
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

	// control search window dimensions
	if ((h_search > img.width) || (v_search > img.height)) {
		generate_error ("Invalid dimensions for search window...");
		return 1;
	}

	// ----------------------------------------------------------------------
	// PRINTING OF STATUS INFORMATION
	// ----------------------------------------------------------------------
	printf ("[INFO] ... .............................................\n");
	printf ("[INFO] ... image dimensions: %dx%d\n", img.width, img.height);
	printf ("[INFO] ... kind of shrinkage: %s\n", kind);
	printf ("[INFO] ... block size: %d\n", block_size);
	printf ("[INFO] ... block step: %d\n", block_step);
	printf ("[INFO] ... sigma: %d\n", sigma);
	printf ("[INFO] ... maximum number of blocks: %d\n", max_blocks);
	printf ("[INFO] ... horizontal search window size: %d\n", h_search);
	printf ("[INFO] ... vertical search window size: %d\n", v_search);
	printf ("[INFO] ... threshold 2D: %f\n", th_2d);
	printf ("[INFO] ... tau-match 2D: %f\n", tau_match);
	printf ("[INFO] ... threshold 3D: %f\n", th_3d);
	printf ("[INFO] ... .............................................\n\n");


	// ----------------------------------------------------------------------
	// COLORSPACE CONVERSION & WRITEBACK
	// ----------------------------------------------------------------------
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

	// ----------------------------------------------------------------------
	// IMAGE-TO-ARRAY CONVERSION
	// ----------------------------------------------------------------------
	printf ("[INFO] ... launch of conversion from image to array...\n");

	printf ("[INFO] ...    luminance channel...\n");
	img2array (&img, 0);

	printf ("[INFO] ...    chrominance channel 1...\n");
	img2array (&img, 1);

	printf ("[INFO] ...    chrominance channel 2...\n");
	img2array (&img, 2);

	printf ("[INFO] ... end of conversion from image to array...\n\n");


	// ----------------------------------------------------------------------
	// BLOCK-MATCHING
	// ----------------------------------------------------------------------
	printf ("[INFO] ... launch of block-matching...\n");
	printf ("[INFO] ...    luminance channel...\n");
	bm_start = clock();

	if (block_matching(kind, &img, &tmp, block_size, block_step, sigma, h_search, v_search, th_2d, tau_match, 0, 1, &y_list) != 0) {
		return 1;
	}

	bm_end = clock();
	time = (bm_end - bm_start) / (double)CLOCKS_PER_SEC;
	printf ("[INFO] ...    elapsed time: %f\n", time);
	printf ("[INFO] ...    number of groups in list: %d\n\n", list_length(&y_list));

	// printf ("[INFO] ... chrominance channel 1...\n");

	// bm_start = clock();

	// if (block_matching("ht", &img, 0, block_size, block_step, sigma, h_search, v_search, th_2d, tau_match, 1, 0, &u_list) != 0) {
	// 	return 1;
	// }

	// bm_end = clock();
	// time = (bm_end - bm_start) / (double)CLOCKS_PER_SEC;
	// printf ("[INFO] ... elapsed time: %f\n", time);
	// printf ("[INFO] ... number of groups in list: %d\n\n", list_length(&u_list));

	// printf ("[INFO] ... chrominance channel 2...\n");

	// bm_start = clock();

	// if (block_matching("ht", &img, 0, block_size, block_step, sigma, h_search, v_search, th_2d, tau_match, 2, 0, &v_list) != 0) {
	// 	return 1;
	// }

	// bm_end = clock();
	// time = (bm_end - bm_start) / (double)CLOCKS_PER_SEC;
	// printf ("[INFO] ... elapsed time: %f\n", time);
	// printf ("[INFO] ... number of groups in list: %d\n\n", list_length(&v_list));
	printf ("[INFO] ... end of block-matching...\n\n");

	sprintf (path, "grp/org/%s/y/", kind);

	if (print_list(y_list, path, "group") != 0) {
		return 1;
	}

	// set filename for txt-file of groups
	if (get_output_filename (outfile, "bms/", "block_matching_statistics", "csv", block_size) != 0) {
		generate_error ("Unable to process output filename...");
		return 1;
	}

	// add number of groups and computation time to txt-file for statistical evaluation
	if (add_csv_line(outfile, block_step, list_length(&y_list), time) != 0) {
		generate_error ("Unable to add values to csv-file...");
		return 1;
	}

	// obtain the pixel values from the u- and v-channel of the image
	printf ("[INFO] ... extracting blocks from chrominance channels...\n\n");
	if (get_chrom(&img, &y_list, &u_list, &v_list)) {
		return 1;
	}

	// trim groups to maximal number of blocks
	printf ("[INFO] ... trimming groups to maximum size...\n");
	printf ("[INFO] ...    luminance channel...\n");
	if (trim_list(&y_list, max_blocks) != 0) {
		return 1;
	}

	printf ("[INFO] ...    chrominance channel 1...\n");
	if (trim_list(&u_list, max_blocks) != 0) {
		return 1;
	}

	printf ("[INFO] ...    chrominance channel 2...\n\n");
	if (trim_list(&v_list, max_blocks) != 0) {
		return 1;
	}

	sprintf (path, "grp/trm/%s/y/", kind);

	if (print_list(y_list, path, "group") != 0) {
		return 1;
	}

	sprintf (path, "grp/trm/%s/u/", kind);

	if (print_list(u_list, path, "group") != 0) {
		return 1;
	}

	sprintf (path, "grp/trm/%s/v/", kind);

	if (print_list(v_list, path, "group") != 0) {
		return 1;
	}


	
	// ----------------------------------------------------------------------
	// IMAGE-DENOISING
	// ----------------------------------------------------------------------
	printf ("[INFO] ... launch of denoising...\n");
	printf ("[INFO] ...    determining estimates...\n");

	printf ("[INFO] ...       luminance channel...\n");
	if (shrinkage(kind, &y_list, sigma, 0) != 0) {
		return 1;
	}

	printf ("[INFO] ...       chrominance channel 1...\n");
	if (shrinkage(kind, &u_list, sigma, 1) != 0) {
		return 1;
	}

	printf ("[INFO] ...       chrominance channel 2...\n");
	if (shrinkage(kind, &v_list, sigma, 1) != 0) {
		return 1;
	}

	sprintf (path, "grp/est/%s/y/", kind);

	if (print_list(y_list, path, "group") != 0) {
		return 1;
	}

	sprintf (path, "grp/est/%s/u/", kind);

	if (print_list(u_list, path, "group") != 0) {
		return 1;
	}

	sprintf (path, "grp/est/%s/v/", kind);

	if (print_list(v_list, path, "group") != 0) {
		return 1;
	}

	// ----------------------------------------------------------------------
	// AGGREGATION
	// ----------------------------------------------------------------------
	printf ("[INFO] ...    aggregating local estimates...\n");

	printf ("[INFO] ...       luminance channel...\n");
	if (aggregate(kind, &img, &y_list, 0) != 0) {
		return 1;
	}

	printf ("[INFO] ...       chrominance channel 1...\n");
	if (aggregate(kind, &img, &u_list, 1) != 0) {
		return 1;
	}

	printf ("[INFO] ...       chrominance channel 2...\n");
	if (aggregate(kind, &img, &v_list, 2) != 0) {
		return 1;
	}

	printf ("[INFO] ... end of denoising...\n\n");
	
	// ----------------------------------------------------------------------
	// COLORSPACE CONVERSION & WRITEBACK
	// ----------------------------------------------------------------------
	printf ("[INFO] ... launch of color conversion...\n");
	yuv2rgb (&img);


	sprintf (prefix, "denoised_rgb_%s", kind);

	// set filename for denoised rgb output image
	if (get_output_filename (outfile, "img/rgb/", prefix, "png", sigma) != 0) {
		generate_error ("Unable to process output filename...");
		return 1;
	}

	// write output image
	if (png_write(&img, outfile) != 0) {
		return 1;
	}

	printf ("[INFO] ... end of color conversion...\n\n");

	// ----------------------------------------------------------------------
	// FREEING DYNAMICALY ALLOCATED MEMORY
	// ----------------------------------------------------------------------
	png_free_mem (&img);

	return 0;
}
