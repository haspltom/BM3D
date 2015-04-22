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

		if (!strcmp(path, "grp/est/y/") || !strcmp(path, "grp/est/u/") || !strcmp(path, "grp/est/v/")) {
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

void idct_3d (int const len, int const z, double arr[z][len][len]) {
	int i, j, k, l, m, n;
	double sum, al, am, an = 0.0;
	double tmp[z][len][len];		// dct result
	double fac;

	for (k=0; k<z; ++k) {
		for (j=0; j<len; ++j) {
			for (i=0; i<len; ++i) {
				for (n=0; n<z; ++n) {
					for (m=0; m<len; ++m) {
						for (l=0; l<len; ++l) {
							al = (l == 0) ? 1.0/sqrt((double)len) : sqrt(2.0/(double)len);
							am = (m == 0) ? 1.0/sqrt((double)len) : sqrt(2.0/(double)len);
							an = (n == 0) ? 1.0/sqrt((double)z) : sqrt(2.0/(double)z);

							fac = cos((PI/(double)len)*((double)j+0.5)*(double)m) * cos((PI/(double)len)*((double)i+0.5)*(double)l) * cos((PI/(double)z)*((double)k+0.5)*(double)n);
							sum += arr[n][m][l] * al * am * an * fac;
						}
					}
				}
				tmp[k][j][i] = sum;
				// if (k==0 && j==0 && i==0) printf ("sum: %f\n", sum);
				// if (k==0 && j==0 && i==0) printf ("ai: %f\naj: %f\nak: %f\n", ai, aj, ak);
				// if (k==0 && j==0 && i==0) printf ("product: %f\n", tmp[k][j][i]);
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

	// printf ("%f\n", threshold);
	
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

int get_chrom (png_img* img, list_t* ylist, list_t* ulist, list_t* vlist) {
	group_node_t* tmp = *ylist;
	node_t* group;
	block_t* block;
	double w;							// weight of actual processed group
	int x, y, bs;
	double d;
	group_t u_group = 0;				// group, which holds a set of similar blocks
	group_t v_group = 0;				// group, which holds a set of similar blocks
	block_t tmp_block;

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

int determine_estimates_ht (list_t const list, int const sigma, char* const path) {
	FILE* fd = 0;
	char outfile[40];
	group_node_t* tmp = list;
	node_t* group;
	unsigned int z;
	double th_3d = 0.15;
	int count = 0;

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

		// perform 3D-IDCT
		idct_3d (len, z, arr);

		// append inverse-transformed group to log-file
		array2file (fd, len, z, arr, "group after 3D-IDCT transformation");

		// write array data back to a list node
		array2group (&group, len, z, arr);

		tmp = tmp->next;

		fclose (fd);
	}

	// probably need more variables in interface
	return 0;
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

int i_buf2file (unsigned int const width, unsigned int const height, unsigned int const buf[width][height], char* const path, char* const prefix) {
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

int aggregate(png_img* img, list_t* list, unsigned int channel) {
	group_node_t* tmp = *list;
	node_t* group;
	block_t* block;
	double w;							// weight of actual processed group
	double ebuf[img->width][img->height];
	double wbuf[img->width][img->height];
	unsigned int estbuf[img->width][img->height];
	int i, j;
	int x, y, bs;
	png_byte* row;
	png_byte* pix;
	char path[20];


	while (tmp != NULL) {
		group = tmp->group;
		w = tmp->weight;
		// if (block->x==15 && block->y==15) printf ("x,y: %d %d\n", block->x, block->y);
		
		while (group != NULL) {
			block = &group->block;
			x = block->x;
			y = block->y;
			bs = block->block_size;

			// iterate over current block and extract values
			for (j=0; j<bs; ++j) {
				for (i=0; i<bs; ++i) {
					// if (block->x==14 && block->y==14) {
						// printf ("x,y: %d %d\n", block->x, block->y);
						// printf ("block->data: %f\n", block->data[j][i]);
						ebuf[j+y-(bs/2)][i+x-(bs/2)] += block->data[j][i] * tmp->weight;
						wbuf[j+y-(bs/2)][i+x-(bs/2)] += tmp->weight;
						// ebuf[j][i] += block->data[j][i];
						// printf ("ebuf: %f\n", ebuf[j][i]);
					// }
				}
				// if (block->x==15 && block->y==15) printf ("\n");
			}
			// if (block->x==15 && block->y==15) printf ("\n");

			group = group->next;
		}

		tmp = tmp->next;
	}

	switch (channel) {
		case 0:
			sprintf (path, "dns/y/");
			break;
		case 1:
			sprintf (path, "dns/u/");
			break;
		case 2:
			sprintf (path, "dns/v/");
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
				estbuf[j][i] = limit((int)(ebuf[j][i] / wbuf[j][i]));
				// printf ("estbuf[%d][%d]: %d\n", j, i, estbuf[j][i]);
				maxest = (estbuf[j][i] >= maxest) ? estbuf[j][i] : maxest;
				minest = (estbuf[j][i] <= minest) ? estbuf[j][i] : minest;
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
			pix[channel] = (estbuf[j][i] != 0) ? estbuf[j][i] : pix[channel];
		}
	}
	
	return 0;
}

// Wiener Stuff -----------------------------------------------
double mean_2d(block_t* block) {
	int bs = block->block_size;
	double sum = 0.0;
	int i, j;

	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			sum += block->data[j][i];
		}
	}

	return sum / (bs*bs);
}

void subtract_mean (int const bs, block_t* block, double mat[bs][bs]) {
	int i, j;

	for (j=0; j<bs; ++j) {
		for (i=0; i<bs; ++i) {
			mat[i][j] = block->data[i][j] - mean_2d(block);
		}
	}
}

// performs a check, whether two given blocks are similar
double get_block_distance (char* const kind, block_t* ref_block, block_t* cmp_block, int const sigma, double const th_2d) {
	int const bs = ref_block->block_size;
	double ref_mat[bs][bs];
	double cmp_mat[bs][bs];
	double sub_mat[bs][bs];
	double distance = 0.0;

	if (!strcmp(kind, "ht")) {
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
	}

	else if (!strcmp(kind, "wnr")) {
		// subtract the mean values from the regarding blocks
		subtract_mean (bs, ref_block, ref_mat);
		subtract_mean (bs, cmp_block, cmp_mat);
	}

	else {
		generate_error ("Wrong description of block-matching kind...");
		return -1.0;
	}

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
							d = get_block_distance (kind, &ref_block, &cmp_block, sigma, th_2d);
							if (!strcmp(kind, "wnr")) printf ("distance: %f\ntaumatch: %f\n", d, tau_match*255);
							
							// decide whether block similarity is sufficient
							if (d < tau_match*255) {
								if (append_block (&group, &cmp_block, d) != 0) {
									return 1;
								}
								if (!strcmp(kind, "wnr")) printf ("block appended...\n");

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
					sprintf (path, "img/yuv/grp/%s/", kind);

					// set filename for noisy yuv output image
					if (get_output_filename (outfile, path, "group", "png", ++count) != 0) {
						generate_error ("Unable to process output filename...");
						return 1;
					}

					// write output image
					if (png_write(tmp, outfile) != 0) {
						return 1;
					}
				}

				group = 0; //EVIL, cause same pointer
			}
		}
	}

	return 0;
}

int bm3d (char* const infile, 			// name of input file
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
	list_t y_list_ht = 0;					// list of groups	of the y-channel for the HT part
	list_t u_list_ht = 0;					// list of groups of the u-channel for the HT part
	list_t v_list_ht = 0;					// list of groups of the v-channel for the HT part
	list_t y_list_wnr = 0;					// list of groups	of the y-channel for the WNR part
	list_t u_list_wnr = 0;					// list of groups of the u-channel for the WNR part
	list_t v_list_wnr = 0;					// list of groups of the v-channel for the WNR part
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

	// ----------------------------------------------------------------------
	// PRINTING OF STATUS INFORMATION
	// ----------------------------------------------------------------------
	printf ("[INFO] ... .............................................\n");
	printf ("[INFO] ... image dimensions: %dx%d\n", img.width, img.height);
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
	// BLOCK-MATCHING STEP 1: HARD-THRESHOLDING
	// ----------------------------------------------------------------------
	printf ("[INFO] ... launch of block-matching step 1...\n");
	printf ("[INFO] ... luminance channel...\n");
	bm_start = clock();

	if (block_matching("ht", &img, &tmp, block_size, block_step, sigma, h_search, v_search, th_2d, tau_match, 0, 1, &y_list_ht) != 0) {
		return 1;
	}

	bm_end = clock();
	time = (bm_end - bm_start) / (double)CLOCKS_PER_SEC;
	printf ("[INFO] ... elapsed time: %f\n", time);
	printf ("[INFO] ... number of groups in list: %d\n\n", list_length(&y_list_ht));

	// printf ("[INFO] ... chrominance channel 1...\n");

	// bm_start = clock();

	// if (block_matching("ht", &img, 0, block_size, block_step, sigma, h_search, v_search, th_2d, tau_match, 1, 0, &u_list_ht) != 0) {
	// 	return 1;
	// }

	// bm_end = clock();
	// time = (bm_end - bm_start) / (double)CLOCKS_PER_SEC;
	// printf ("[INFO] ... elapsed time: %f\n", time);
	// printf ("[INFO] ... number of groups in list: %d\n\n", list_length(&u_list_ht));

	// printf ("[INFO] ... chrominance channel 2...\n");

	// bm_start = clock();

	// if (block_matching("ht", &img, 0, block_size, block_step, sigma, h_search, v_search, th_2d, tau_match, 2, 0, &v_list_ht) != 0) {
	// 	return 1;
	// }

	// bm_end = clock();
	// time = (bm_end - bm_start) / (double)CLOCKS_PER_SEC;
	// printf ("[INFO] ... elapsed time: %f\n", time);
	// printf ("[INFO] ... number of groups in list: %d\n\n", list_length(&v_list_ht));

	if (print_list(y_list_ht, "grp/org/", "group") != 0) {
		return 1;
	}

	// set filename for txt-file of groups
	if (get_output_filename (outfile, "bms/", "block_matching_statistics", "csv", block_size) != 0) {
		generate_error ("Unable to process output filename...");
		return 1;
	}

	// add number of groups and computation time to txt-file for statistical evaluation
	if (add_csv_line(outfile, block_step, list_length(&y_list_ht), time) != 0) {
		generate_error ("Unable to add values to csv-file...");
		return 1;
	}

	// trim groups to maximal number of blocks
	printf ("[INFO] ... trimming blocks to maximum size...\n");
	printf ("[INFO] ... luminance channel...\n");
	if (trim_list(&y_list_ht, max_blocks) != 0) {
		return 1;
	}

	// printf ("[INFO] ... chrominance channel 1...\n");
	// if (trim_list(&u_list_ht, max_blocks) != 0) {
	// 	return 1;
	// }

	// printf ("[INFO] ... chrominance channel 2...\n");
	// if (trim_list(&v_list_ht, max_blocks) != 0) {
	// 	return 1;
	// }

	// obtain the pixel values from the u- and v-channel of the image
	// printf ("[INFO] ... extracting blocks from chrominance channels...\n");
	// if (get_chrom(&img, &y_list_ht, &u_list_ht, &v_list_ht)) {
	// 	return 1;
	// }

	if (print_list(y_list_ht, "grp/trm/y/", "group") != 0) {
		return 1;
	}

	if (print_list(u_list_ht, "grp/trm/u/", "group") != 0) {
		return 1;
	}

	if (print_list(v_list_ht, "grp/trm/v/", "group") != 0) {
		return 1;
	}

	printf ("[INFO] ... end of block-matching step 1...\n\n");

	
	// ----------------------------------------------------------------------
	// IMAGE-DENOISING STEP 1: HARD-THRESHOLDING
	// ----------------------------------------------------------------------
	printf ("[INFO] ... launch of denoising for hard-thresholding...\n");
	printf ("[INFO] ... determining local estimates...\n");
	printf ("[INFO] ... luminance channel...\n");

	if (determine_estimates_ht(y_list_ht, sigma, "dns/y/grp/") != 0) {
		return 1;
	}

	// printf ("[INFO] ... chrominance channel 1...\n");
	// if (determine_estimates_ht(u_list_ht, sigma, "dns/u/grp/") != 0) {
	// 	return 1;
	// }

	// printf ("[INFO] ... chrominance channel 2...\n");
	// if (determine_estimates_ht(v_list_ht, sigma, "dns/v/grp/") != 0) {
	// 	return 1;
	// }

	if (print_list(y_list_ht, "grp/est/y/", "group") != 0) {
		return 1;
	}

	if (print_list(u_list_ht, "grp/est/u/", "group") != 0) {
		return 1;
	}

	if (print_list(v_list_ht, "grp/est/v/", "group") != 0) {
		return 1;
	}

	// ----------------------------------------------------------------------
	// AGGREGATION STEP 1: HARD-THRESHOLDING
	// ----------------------------------------------------------------------
	printf ("[INFO] ... aggregating local estimates...\n");
	printf ("[INFO] ... luminance channel...\n");

	if (aggregate(&img, &y_list_ht, 0) != 0) {
		return 1;
	}

	// printf ("[INFO] ... chrominance channel 1...\n");
	// if (aggregate(&img, &u_list_ht, 1) != 0) {
	// 	return 1;
	// }

	// printf ("[INFO] ... chrominance channel 2...\n");
	// if (aggregate(&img, &v_list_ht, 2) != 0) {
	// 	return 1;
	// }

	// ----------------------------------------------------------------------
	// BLOCK-MATCHING STEP 2: WIENER-FILTERING
	// ----------------------------------------------------------------------
	printf ("[INFO] ... launch of block-matching step 2...\n");
	printf ("[INFO] ... luminance channel...\n");

	bm_start = clock();

	if (block_matching("wnr", &img, &tmp, block_size, block_step, sigma, h_search, v_search, th_2d, tau_match, 0, 1, &y_list_wnr) != 0) {
		return 1;
	}

	bm_end = clock();
	time = (bm_end - bm_start) / (double)CLOCKS_PER_SEC;
	printf ("[INFO] ... elapsed time: %f\n", time);
	printf ("[INFO] ... number of groups in list: %d\n\n", list_length(&y_list_wnr));

	// if (print_list(y_list_wnr, "grp/org/", "group") != 0) {
	// 	return 1;
	// }

	// trim groups to maximal number of blocks
	printf ("[INFO] ... trimming blocks to maximum size...\n");
	printf ("[INFO] ... luminance channel...\n");
	// if (trim_list(&y_list_wnr, max_blocks) != 0) {
	// 	return 1;
	// }

	printf ("[INFO] ... end of block-matching step 2...\n\n");


	// ----------------------------------------------------------------------
	// IMAGE-DENOISING STEP 2: WIENER-FILTERING
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// AGGREGATION STEP 2: WIENER-FILTERING
	// ----------------------------------------------------------------------

	printf ("[INFO] ... end of denoising...\n\n");
	
	// ----------------------------------------------------------------------
	// COLORSPACE CONVERSION & WRITEBACK
	// ----------------------------------------------------------------------
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

	// ----------------------------------------------------------------------
	// FREEING DYNAMICALY ALLOCATED MEMORY
	// ----------------------------------------------------------------------
	png_free_mem (&img);

	return 0;
}
