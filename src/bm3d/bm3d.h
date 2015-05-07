#ifndef BM3D_H
#define BM3D_H

// structure of one block
typedef struct block {
	int block_size;									// block size
	double** data;										// pixel values
	int x;												// horizontal position of the block center
	int y;												// vertical position of the block center
} block_t;

// list node
typedef struct block_node {
	block_t block;										// block data
	double distance;									// block distance to the reference block
	struct block_node* next;
} block_node_t;

// type for a group
typedef block_node_t* group_t;

// node of a list of groups
typedef struct group_node {
	group_t group;										// list of blocks for one reference block
	double weight;										// weight for local estimates after shrinkage
	struct group_node* next;
} group_node_t;

// type for a list of groups
typedef group_node_t* list_t;

// extern int png_denoise ();
extern int bm3d (char* const infile, 			// name of input file
			 		  char* const kind, 				// kind of shrinkage (ht, wnr, avg)
			 		  int const block_size, 		// size of internal processed blocks
			 		  int const block_step, 		// step size between blocks
			 		  int const std_dev, 			// standard deviation of noise
			 		  int const max_blocks,			// maximum number of block in one 3D array
			 		  int const h_search,	// horizontal width of search window
			 		  int const v_search,  // vertical width of search window
					  double const th_2d,			// threshold for the 2D transformation
					  double const tau_match, 			// match value for block-matching
					  double const th_3d);			// threshold for the 3D transformtaion

#endif // BM3D_H
