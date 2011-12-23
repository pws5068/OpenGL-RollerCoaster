# test Makefile 15-462
# Shayan Sarkar
# Jan 16, 2002

GRAPHICS = /afs/cs/academic/class/15462


INCLUDE = -I$(GRAPHICS)/include
LIBRARYPATH = -L$(GRAPHICS)/lib -L/usr/X11R6/lib/
LIBRARIES = $(LIBRARYPATH) -lGL -lGLU -lglut -lpicio -ltiff -ljpeg -lm -lXi -lXmu


COMPILER = gcc
COMPILERFLAGS = -O2 $(INCLUDE)

# ---------- BEGIN SECTION TO CHANGE AS NEEDED ----------

PROGRAM =	rollercoaster

SOURCE =	rc_main.cpp rc_spline.cpp

OBJECT =	rc_main.o rc_spline.o


# ---------- END SECTION TO CHANGE AS NEEDED ----------

.c.o: 
	$(COMPILER) -c $(COMPILERFLAGS) $<

.cpp.o: 
	$(COMPILER) -c $(COMPILERFLAGS) $<
    
all: $(PROGRAM)

$(PROGRAM): $(OBJECT)
	$(COMPILER) $(COMPILERFLAGS) -o $(PROGRAM) $(OBJECT) $(LIBRARIES)

clean:
	-rm -rf core *.o *~ "#"*"#" test


# DO NOT DELETE

