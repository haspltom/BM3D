CC      = clang
SRC     = main.c error.c png_io.c param_pars.c utils.c bm3d.c
LIBS    = -lm -lpng12
CFLAGS  = -c -g3 -Wall -I/usr/include/libpng12
EXT     = c
SRCDIR  = src
OBJDIR  = obj
IMGDIR  = ../img
OBJ     := $(subst .$(EXT),.o,$(SRC))
OBJ     := $(addprefix $(OBJDIR)/, $(OBJ))
 
bm3d: $(OBJ)
	@echo "Link $< ..."
	@$(CC) -o $@ $(OBJ) $(LIBS)
 
$(OBJDIR)/error.o: ../error/error.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/png_io.o: ../png_io/png_io.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/param_pars.o: ../param_pars/param_pars.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/utils.o: ../utils/utils.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/bm3d.o: $(SRCDIR)/bm3d.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/main.o: $(SRCDIR)/main.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
 
clean:
	rm bm3d $(OBJ) $(IMGDIR)/rgb/denoised*.png $(IMGDIR)/yuv/noisy*.png 
