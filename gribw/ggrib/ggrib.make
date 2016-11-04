# Makefile for ggrib.c
#
# This makefile assumes that the library libgribw.a has been compiled and is 
# found in ../gribw together with the header files listed in $(INC).
# Also, a ./TMP directory is assumed for storing object files to keep your
# workspace neat & tidy.

GRIBW=..


.SUFFIXES:
.SUFFIXES: .o .c .h

CC = cc
LD = $(CC)
CCFLAGS= -O2

# must change here to reflect location of gribw library
LDPATH = ..
LDFLAGS = -I$(LDPATH) -L$(LDPATH)
LIBS = -lgribw -lm
TARGET = ggrib

# Suffix rule
.c.o:
	$(CC) -c $(CCFLAGS) -o $*.o $*.c $(LDFLAGS)

INC = $(GRIBW)/gribw.h $(GRIBW)/bds.h $(GRIBW)/gds.h angles.h
SRC = angles.c
OBJ = $(SRC:.c=.o)

$(TARGET): $(TARGET).o $(INC) $(OBJ)
	$(LD) -o $(TARGET) $(TARGET).o $(OBJ) $(LDFLAGS) $(LIBS)
