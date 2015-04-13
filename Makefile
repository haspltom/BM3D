CC      = clang
BIN     = noising param_gen bm3d
DIRS    = objdir rgbdir yuvdir grpdir orgdir trmdir estdir resdir bmsdir dnsdir dnsgrpdir
SRC     = main.c error.c png_io.c param_pars.c csv_export.c utils.c noising.c param_gen.c bm3d.c
LIBS    = -lm -lpng12
CFLAGS  = -g3 -Wall -I/usr/include/libpng12
MKDIR   = mkdir
EXT     = c
SRCDIR  = src
OBJDIR  = obj
IMGDIR  = img
RESDIR  = res
GRPDIR  = grp
BMSDIR  = bms
DNSDIR  = dns
OBJ     := $(subst .$(EXT),.o,$(SRC))
OBJ     := $(addprefix $(OBJDIR)/, $(OBJ))
 
all: $(DIRS) $(BIN)

objdir:
	$(MKDIR) -p $(OBJDIR)

rgbdir:
	$(MKDIR) -p $(IMGDIR)/rgb

yuvdir:
	$(MKDIR) -p $(IMGDIR)/yuv

grpdir:
	$(MKDIR) -p $(GRPDIR)

orgdir:
	$(MKDIR) -p $(GRPDIR)/org

trmdir:
	$(MKDIR) -p $(GRPDIR)/trm

estdir:
	$(MKDIR) -p $(GRPDIR)/est

resdir:
	$(MKDIR) -p $(RESDIR)

bmsdir:
	$(MKDIR) -p $(BMSDIR)

dnsdir:
	$(MKDIR) -p $(DNSDIR)

dnsgrpdir:
	$(MKDIR) -p $(DNSDIR)/grp

noising: $(OBJDIR)/noising.o $(OBJDIR)/error.o $(OBJDIR)/png_io.o $(OBJDIR)/utils.o
	@echo "Link $< ..."
	@$(CC) -o $@ $(OBJDIR)/error.o $(OBJDIR)/png_io.o $(OBJDIR)/utils.o $(OBJDIR)/noising.o $(LIBS)
 
param_gen: $(OBJDIR)/param_gen.o $(OBJDIR)/error.o $(OBJDIR)/utils.o
	@echo "Link $< ..."
	@$(CC) -o $@ $(OBJDIR)/error.o $(OBJDIR)/utils.o $(OBJDIR)/param_gen.o $(LIBS)
 
bm3d: $(OBJDIR)/main.o $(OBJDIR)/error.o $(OBJDIR)/png_io.o $(OBJDIR)/param_pars.o $(OBJDIR)/csv_export.o $(OBJDIR)/utils.o $(OBJDIR)/bm3d.o
	@echo "Link $< ..."
	@$(CC) -o $@ $(OBJDIR)/error.o $(OBJDIR)/png_io.o $(OBJDIR)/param_pars.o $(OBJDIR)/csv_export.o $(OBJDIR)/utils.o $(OBJDIR)/bm3d.o $(OBJDIR)/main.o $(LIBS)
 
$(OBJDIR)/error.o: $(SRCDIR)/error/error.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/png_io.o: $(SRCDIR)/png_io/png_io.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/param_pars.o: $(SRCDIR)/param_pars/param_pars.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/csv_export.o: $(SRCDIR)/csv_export/csv_export.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/utils.o: $(SRCDIR)/utils/utils.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/noising.o: $(SRCDIR)/noising/noising.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/param_gen.o: $(SRCDIR)/param_gen/param_gen.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/bm3d.o: $(SRCDIR)/bm3d/bm3d.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
       
$(OBJDIR)/main.o: $(SRCDIR)/main.c
	@echo "Compile $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<
 
clean:
	rm -r noising param_gen bm3d $(OBJDIR) $(IMGDIR)/rgb/ $(IMGDIR)/yuv/ $(GRPDIR) $(BMSDIR) $(DNSDIR)
