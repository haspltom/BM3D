# BM3D
## C-programm for a BM3D denoising algorithm

**Execution example:**
The following described execution cycle is also done by the given bash script *example_cycle* and
can directly be executed in order not to type all the steps separately.

* execute Makefile
* generate parameter-file with by calling "param_gen"
	* example: `./param_gen ht 11 9 50 25 50 50 0.321 0.101 0.3105`
* generate noisy image by calling "noising"
	* example: `./noising 50 path/to/source.png target/path/`
