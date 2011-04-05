# $Id: Makefile,v 1.4 1999/01/25 22:20:01 brianp Exp $

# Makefile for TR library
# You'll probably have to tweak this for your platform.


CC = cc
CFLAGS = -c -g -ansi -I/usr/local/include
LIBS = -L/usr/X11R6/lib -lglut -lGLU -lGL -lX11 -lXmu -lXext -lm


DEMOS = trdemo1 trdemo2


default: $(DEMOS)


tr.o: tr.c tr.h
	$(CC) $(CFLAGS) tr.c

trdemo1: trdemo1.o tr.o
	$(CC) trdemo1.o tr.o $(LIBS) -o $@

trdemo1.o: trdemo1.c tr.h
	$(CC) $(CFLAGS) trdemo1.c

trdemo2: trdemo2.o tr.o
	$(CC) trdemo2.o tr.o $(LIBS) -o $@

trdemo2.o: trdemo2.c tr.h
	$(CC) $(CFLAGS) trdemo2.c



clean:
	-rm *.o
	-rm $(DEMOS)



TR_DIR = tr-1.3
TR_NAME = tr-1.3

FILES = \
	$(TR_DIR)/Makefile		\
	$(TR_DIR)/tr.htm		\
	$(TR_DIR)/README		\
	$(TR_DIR)/LICENSE		\
	$(TR_DIR)/tr.[ch]		\
	$(TR_DIR)/trdemo[12].c

tar:
	cd .. ; \
	tar -cvf $(TR_NAME).tar $(FILES) ; \
	gzip $(TR_NAME).tar ; \
	mv $(TR_NAME).tar.gz $(TR_DIR)

zip:
	-rm $(TR_NAME).zip
	cd .. ; \
	zip -r $(TR_NAME).zip $(FILES) ; \
	mv $(TR_NAME).zip $(TR_DIR)
