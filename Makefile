CFLAGS=-I/usr/X11R6/include -g
CC=c++

OBJECTS=GsWidget.o PSEditWidget.o PSEditor.o Postscript.o flpsed.o

%.o: %.cxx
	$(CC) -c $(CFLAGS) $*.cxx

flpsed: $(OBJECTS)
	$(CC) -g -o flpsed $(OBJECTS) -L/usr/X11R6/lib -lfltk -lX11 -lm

clean:
	rm -f flpsed $(OBJECTS)
