CC      = clang
SRC     = main.c error.c png_io.c utils.c noising.c
LIBS    = -lm -lpng12
CFLAGS  = -c -g3 -Wall -I/usr/include/libpng12
EXT     = c
SRCDIR  = src
OBJDIR  = obj
IMGDIR  = ../img
OBJ     := $(subst .$(EXT),.o,$(SRC))
OBJ     := $(addprefix $(OBJDIR)/, $(OBJ))
 
noising: $(OBJ)
	@echo "Link $< ..."
	@$(CC) -o $@ $(OBJ) $(LIBS)
 
$(OBJDIR)/error.o: ../error/error.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/png_io.o: ../png_io/png_io.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/utils.o: ../utils/utils.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/noising.o: $(SRCDIR)/noising.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/main.o: $(SRCDIR)/main.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
 
clean:
	rm noising $(OBJ) $(IMGDIR)/rgb/noisy*.png
