#
# Makefile for srogue
#

HDRS= rogue.h mach_dep.h ../tunable.h curses.h 

OBJS=	vers.o armor.o artifact.o chase.o command.o daemon.o daemons.o fight.o \
	encumb.o get_play.o init.o io.o list.o main.o maze.o misc.o \
	monsdata.o monsters.o move.o new_level.o options.o pack.o passages.o \
	player.o potions.o rings.o rip.o rooms.o save.o scrolls.o \
	sticks.o things.o trader.o usgv4.o weapons.o wizard.o rogue.o 

CFILES= vers.c armor.c artifact.c chase.c command.c daemon.c daemons.c fight.c \
	encumb.c get_play.c init.c io.c list.c main.c maze.c misc.c \
	monsdata.c monsters.c move.c new_level.c options.c pack.c passages.c \
	player.c potions.c rings.c rip.c rooms.c save.c scrolls.c \
	sticks.c things.c trader.c usgv4.c weapons.c wizard.c rogue.c 

SOURCES= $(CFILES) rogue.h mach_dep.h Makefile

TERMLIB= -ltermlib
CRLIB= -lcurses

# Installation variables.
# install srogue here

BIN=/a/staff/peterv/bin
LIB=/a/staff/peterv/urogue/ur.lib
UMASK=4711
CHMOD=chmod
CP=cp
RM=rm -f
LD=ld
CTAGS=ctags
TOUCH=touch
PROGRAM=tidy
TEST=ntidy
TUNABLE=../tunable.o

LDFLAGS=
CFLAGS= -O

$(PROGRAM):	$(HDRS) $(OBJS)	$(TUNABLE)
		$(CC) $(LDFLAGS) $(OBJS) $(TUNABLE) $(CRLIB) $(TERMLIB) -o $@
		@size $@
		@echo $@ made.

test:
		$(PROGRAM)
		$(CP) $(PROGRAM) $(BIN)/$(TEST)
		$(RM) $(PROGRAM)
		$(CHMOD) $(UMASK) $(BIN)/$(TEST)

install:
		$(CP) $(BIN)/$(TEST) $(BIN)/$(PROGRAM)
		$(RM) $(BIN)/$(TEST)
		$(CHMOD) $(UMASK) $(BIN)/$(PROGRAM)

tags:
		$(CTAGS) $(HDRS) $(CFILES)

curses.h:	/usr/include/curses.h
		ln -s /usr/include/curses.h

distrib:	$(OBJS)
		$(RM) $(LIB)
		$(LD) -r -o $(LIB) $(OBJS) 

lint:
		lint -hxbc $(CFILES) -lcurses > linterrs

clobber:	clean
		rm -f $(PROGRAM) a.out

clean:
		rm -f $(OBJS) core $(LIB)

count:
		wc -l $(HDRS) $(CFILES)

realcount:
		cc -E $(CFILES) | wc -l

depend:
	grep '^#[ 	]*include[ 	]*"' ${CFILES} | \
	sed -e 's/:[^"]*"\([^"]*\)"/:	\1/' -e 's/\.c/.o/' | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '' >> Makefile
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it


armor.o:	curses.h rogue.h
chase.o:	curses.h rogue.h
command.o:	curses.h rogue.h
daemon.o:	curses.h rogue.h
daemons.o:	curses.h rogue.h
fight.o:	curses.h rogue.h mach_dep.h
encumb.o:	curses.h rogue.h
get_play.o:	curses.h rogue.h
init.o:	curses.h rogue.h
io.o:	curses.h rogue.h
list.o:	curses.h rogue.h
main.o:	curses.h mach_dep.h rogue.h ../tunable.h
maze.o:	curses.h rogue.h
misc.o:	curses.h rogue.h
monsters.o:	curses.h rogue.h mach_dep.h
move.o:	curses.h rogue.h
new_level.o:	curses.h rogue.h
options.o:	mach_dep.h curses.h rogue.h
pack.o:	curses.h rogue.h
passages.o:	curses.h rogue.h
player.o:	curses.h rogue.h
potions.o:	curses.h rogue.h
rings.o:	curses.h rogue.h
rip.o:	curses.h mach_dep.h rogue.h
rooms.o:	curses.h rogue.h
save.o:	curses.h rogue.h
scrolls.o:	curses.h rogue.h
sticks.o:	curses.h rogue.h
things.o:	curses.h rogue.h
trader.o:	curses.h rogue.h
usgv4.o:	curses.h rogue.h mach_dep.h
weapons.o:	curses.h rogue.h
wizard.o:	curses.h rogue.h
rogue.o:	curses.h rogue.h
