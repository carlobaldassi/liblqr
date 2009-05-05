#
# Makefile example for liquidrescale (lqr) library
# 
# This is thought for exemplification purposes.
# It will only work if you have already installed
# the lqr library, the pngwriter and all their 
# required dependencies (glib, z, png, freetype).
#
# It does not perform any kind of check on the system,
# and won't report any sensible error message.
#


# set the sources file name
tt1 = liquidrescale
tt2 = liquidrescale-basic
# objects to build
tt1obj = $(tt1).o
tt2obj = $(tt2).o
# executable names
tt1out = liquidrescale
tt2out = liquidrescale-basic


# collect flags for the compiler for lqr library
LQR_CFLAGS = `pkg-config --cflags lqr-1`

LQR_LIBS = `pkg-config --libs lqr-1`

LQR_LIBDIR = `pkg-config --variable=libdir lqr-1`

# collect flags for pngwriter library
PNGWRITER_CFLAGS = `freetype-config --cflags` 

PNGWRITER_LIBS = -lpng -lpngwriter -lz -lfreetype 

# collect flags for the compiler for glib thread support
GTHREAD_LIBS = `pkg-config --libs gthread-2.0`

# join the flags collected above
INCLUDES = \
	$(LQR_CFLAGS) \
	$(PNGWRITER_CFLAGS)

LIBDIRS = \
	  -L$(LQR_LIBDIR)

LIBS = \
       $(LQR_LIBS) \
       $(PNGWRITER_LIBS) \
       $(GTHREAD_LIBS)

# define the flags to use in the various compilation steps
BASE_FLAGS = -Winline -O2 -Wall
OBJ_BUILD_FLAGS = $(BASE_FLAGS) $(INCLUDES) $(LIBDIRS) 
LINKING_FLAGS = $(OBJ_BUILD_FLAGS) $(LIBS) 

# compilation rules

.PHONY : all clean

all: $(tt1out) $(tt2out)

# linking
$(tt1out) : $(tt1obj)
	g++ $(LINKING_FLAGS) -o $(tt1out) $(tt1obj)

$(tt2out) : $(tt2obj)
	g++ $(LINKING_FLAGS) -o $(tt2out) $(tt2obj)

# object building
$(tt1).o : $(tt1).cpp $(tt1).h
	g++ -c $(OBJ_BUILD_FLAGS) $(tt1).cpp

$(tt2).o : $(tt2).cpp
	g++ -c $(OBJ_BUILD_FLAGS) $(tt2).cpp

#cleanup
clean:
	rm -f *~ ctags $(tt1obj) $(tt2obj) $(tt1out) $(tt2out)

