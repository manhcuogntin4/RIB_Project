
#------------------------------------------------------------------------------

SOURCE=remove_line.cc
MYPROGRAM=remove_line

MYLIBRARIES=-llept -ltesseract
IDIR=../include
CFLAGS=`pkg-config --cflags opencv`
LIBS=`pkg-config --libs opencv`
CC=g++

#------------------------------------------------------------------------------



all: $(MYPROGRAM)






$(MYPROGRAM): $(SOURCE)

	$(CC) $(CFLAGS) $(SOURCE) $(LIBS) -o $(MYPROGRAM) $(MYLIBRARIES) $(INCLUDE)

clean:

	rm -f $(MYPROGRAM)


