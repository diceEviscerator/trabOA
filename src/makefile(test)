IDIR =../include
CC=g++
EXT = cpp
CFLAGS= -I $(IDIR) -Wall -lm

ODIR=obj
LDIR =../lib

GTESTLIBS = /usr/lib/libgtest.a -lgtest -lpthread
LIBS=$(GTESTLIBS) 

_DEPS = drive_lib.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = drive_test.o my_drive.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.$(EXT) $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

virtual_disk: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 