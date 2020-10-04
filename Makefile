
INSD=./games
LIB=$(INSD)/lib/ur
DISTRIB = Makefile.install namefinder.o control.o $(LIB)
SOURCES = Makefile Makefile.install namefinder.c control.c tunable.c tunable.h lav.c

TUNABLE = tunable.o

GET = co

all: install

allclean: rogueclean distribclean clean

$(SOURCES):
	@$(GET) $@

install: $(DISTRIB)
	@-mkdir $(INSD)
##	@chmod 755 $(INSD)
	@-rm -f $(INSD)/*
	cp Makefile.install $(INSD)/Makefile
	cp BUGS README lav.c tunable.c control.o namefinder.o $(INSD)
	@chmod 644 $(INSD)/*
	
control: control.o $(TUNABLE)
	cc -O control.o $(TUNABLE) -s -o control

clobber:
	rm -rf control namefinder *.o

tunable.o: tunable.h
control.o: tunable.h
namefinder.o: tunable.h

$(LIB): tunable.o
	@echo "making rogue distrib..."
	@cd rogue ; make distrib

rogue:
	@echo "making rogue..."
	@cd rogue ; make rogue

clean: clobber

rogueclean:
	@echo "making rogue clean"
	@cd rogue ; make clean
	
distribclean:
	@echo "making distrib clean"
	rm -rf $(INSD)
