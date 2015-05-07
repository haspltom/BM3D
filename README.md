# BM3D
## C-programm for a BM3D denoising algorithm

### Execution example:

The following described execution cycle is also done by the given bash script *example_cycle* and
can directly be executed in order not to type all the steps separately.

* execute Makefile
* generate parameter-file with by calling "param_gen"
	* example: `./param_gen ht 11 9 50 25 50 50 0.321 0.101 0.3105`
* generate noisy image by calling "noising"
	* example: `./noising 50 path/to/source_image.png target/path/`
* execute the main program by calling "bm3d"
	* example: `./bm3d path/to/noisy/image.png path/to/params_file.txt`

### Remarks:

The execution time differs very strongly by varying the parameters. When you choose *avg* as kind of
shrinkage the cycle takes the shortest time, because then the calculation of the DCT will be
omitted. Also, when you increase the block step, the execution time will drop significantly. On the
other hand, the quality of the output image will suffer from these speedup actions.

The block size should also be higher than the block step, so that the recognized blocks overlap.
Otherwise, you will have areas of the image, which will not be processed at all.

The shrinkage kind *none* is only for testing all steps of processing without changing the pixel
values. So, when using *none*, the input should be equal to the output. Unfortunately, this is only
the case to a certain degree. See issue *changing of colors* in section *known bugs*.

An additional possible setting is the generation of a bunch of images with the identified groups of
blocks marked in them. However, this has to be done in the source code directly when the function
`block_matching()` is called. The reason for this is simply, that I haven't brought the effort so
far to include this setting in the parameter file. To enable this block marking, just change the
function call in line src/bm3d/bm3d.c:1261 from `block_matching(kind, &img, 0, block_size,
block_step, sigma, h_search, v_search, th_2d, tau_match, 0, 0, &y_list)` to `block_matching(kind,
&img, &tmp, block_size, block_step, sigma, h_search, v_search, th_2d, tau_match, 0, 1, &y_list)`.
In this case, the recognized blocks for the channel *y* will be marked in images and stored into
`img/rgb/grp/`.
	

### Known issues and bugs:

* Changing of colors
	* Currently, the shrinkage and the aggregation of the chrominance channels is commented out,
	  because then the colors of the output image change slightly and I simply don't know why.
	* The source of the problem seems to be in function `aggregate()`, because always the first call
	  of `aggregate()` is correct, no matter with which channel. So actually, it is not a problem
	  bound to the channels *u* and *v*, but rather to function `aggregate()`.
* Freeing all the dynamically allocated memory is still missing.
* Unprocessed borders
	* Due to the fact, that the recognition of the blocks is dependend from the block step, there
	  always remain a certain border, which is not executed at all.
* Pixel artefacts at the bottom
	* Mostly there are some strange artefacts in the last few lines of the output image, which
	  origins are not clarified yet.
* Nearly no validation of the input parameters so far
	* When there is a faulty parameter, which can not be handled the program probably crashes.

### Parameters:
* **Kinds of shrinkage:** none, avg, ht, wnr
* **Sigma:** from 0 to 255
* **Most demonstrative block sizes:** 7, 9, 11, 13
* **Most demonstrative block steps:** 5, 7, 9, 11
