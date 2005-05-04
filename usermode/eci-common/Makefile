all: eoc.o
eoc.o: eoc.c eoc.h
CLEANFILES = eoc.o
install :

clean: clean-generic

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)