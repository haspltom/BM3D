#ifndef BM3D_H
#define BM3D_H

#define PI 3.14159265359

// structure of one block
typedef struct block {
	int block_size;
	double** data;
	int x;
	int y;
} block_t;

// list node
typedef struct block_node {
	block_t block;
	double distance;
	struct block_node* next;
} node_t;

// type for a group
typedef node_t* group_t;

// node of a list of groups
typedef struct group_node {
	group_t group;
	struct group_node* next;
} group_node_t;

// type for a list of groups
typedef group_node_t* list_t;

// extern int png_denoise ();
extern int bm3d (char* const infile, 			// name of input file
			 		  int const block_size, 		// size of internal processed blocks
			 		  int const block_step, 		// step size between blocks
			 		  int const std_dev, 			// standard deviation of noise
			 		  int const max_blocks,			// maximum number of block in one 3D array
			 		  int const search_window_x,	// horizontal width of search window
			 		  int const search_window_y); // vertical width of search window

#endif // BM3D_H
