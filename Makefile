#------------------------------------------------------------------------------

SOURCE=rib.cpp  
MYPROGRAM=rib

MYLIBRARIES=-llept -ltesseract
IDIR=../include
CFLAGS=-I$(IDIR)

CFLAGS2=`pkg-config --cflags opencv`
LIBS=`pkg-config --libs opencv`

INCLUDE  := -I./include/ -std=c++11
OBJ_DIR  := $(BUILD)/objects
CC=g++

#------------------------------------------------------------------------------



all: $(MYPROGRAM)






$(MYPROGRAM): $(SOURCE)

	$(CC) $(SOURCE) -o $(MYPROGRAM) $(MYLIBRARIES) $(INCLUDE) $(CFLAGS2) $(LIBS)

clean:

	rm -f $(MYPROGRAM)
