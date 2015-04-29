CC      = clang
BIN     = noising param_gen bm3d
DEFDIRS = objdir resdir bmsdir 
IMGDIRS = rgbdir yuvgrpdir
ORGDIRS = orghtydir orghtudir orghtvdir orgwnydir orgwnudir orgwnvdir 
TRMDIRS = trmhtydir trmhtudir trmhtvdir trmwnydir trmwnudir trmwnvdir
ESTDIRS = esthtydir esthtudir esthtvdir estwnydir estwnudir estwnvdir
DNSDIRS = dnshtydir dnshtudir dnshtvdir dnswnydir dnswnudir dnswnvdir
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
 
all: $(DEFDIRS) $(IMGDIRS) $(ORGDIRS) $(TRMDIRS) $(ESTDIRS) $(DNSDIRS) $(BIN)

objdir:
	$(MKDIR) -p $(OBJDIR)

rgbdir:
	$(MKDIR) -p $(IMGDIR)/rgb

yuvgrpdir:
	$(MKDIR) -p $(IMGDIR)/yuv/grp

orghtydir:
	$(MKDIR) -p $(GRPDIR)/org/ht/y

orghtudir:
	$(MKDIR) -p $(GRPDIR)/org/ht/u

orghtvdir:
	$(MKDIR) -p $(GRPDIR)/org/ht/v

orgwnydir:
	$(MKDIR) -p $(GRPDIR)/org/wnr/y

orgwnudir:
	$(MKDIR) -p $(GRPDIR)/org/wnr/u

orgwnvdir:
	$(MKDIR) -p $(GRPDIR)/org/wnr/v

trmhtydir:
	$(MKDIR) -p $(GRPDIR)/trm/ht/y

trmhtudir:
	$(MKDIR) -p $(GRPDIR)/trm/ht/u

trmhtvdir:
	$(MKDIR) -p $(GRPDIR)/trm/ht/v

trmwnydir:
	$(MKDIR) -p $(GRPDIR)/trm/wnr/y

trmwnudir:
	$(MKDIR) -p $(GRPDIR)/trm/wnr/u

trmwnvdir:
	$(MKDIR) -p $(GRPDIR)/trm/wnr/v

esthtydir:
	$(MKDIR) -p $(GRPDIR)/est/ht/y

esthtudir:
	$(MKDIR) -p $(GRPDIR)/est/ht/u

esthtvdir:
	$(MKDIR) -p $(GRPDIR)/est/ht/v

estwnydir:
	$(MKDIR) -p $(GRPDIR)/est/wnr/y

estwnudir:
	$(MKDIR) -p $(GRPDIR)/est/wnr/u

estwnvdir:
	$(MKDIR) -p $(GRPDIR)/est/wnr/v

resdir:
	$(MKDIR) -p $(RESDIR)

bmsdir:
	$(MKDIR) -p $(BMSDIR)

dnshtydir:
	$(MKDIR) -p $(DNSDIR)/ht/y/grp

dnshtudir:
	$(MKDIR) -p $(DNSDIR)/ht/u/grp

dnshtvdir:
	$(MKDIR) -p $(DNSDIR)/ht/v/grp

dnswnydir:
	$(MKDIR) -p $(DNSDIR)/wnr/y/grp

dnswnudir:
	$(MKDIR) -p $(DNSDIR)/wnr/u/grp

dnswnvdir:
	$(MKDIR) -p $(DNSDIR)/wnr/v/grp

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
