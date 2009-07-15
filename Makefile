# Micro Snake Makefile
# Remember to update the VERSION before a new release.
# -- " --  to set the DESTDIR env. variable when installing.
#
# Set CC and CFGLAGS in your local environment for a suitable
# compiler (tcc?) and CFLAGS (-Os -W -Wall -Werror).

VERSION   = 1.0.0
CC       ?= gcc
CFLAGS   += -W -Wall -Werror

all: snake

snake.o: Makefile snake.c snake.h

clean:
	-@$(RM) snake snake.o

distclean: clean

install: all
	install snake $(DESTDIR)

dist:
	@git archive --format=tar --prefix=snake-$(VERSION)/ $(VERSION) | bzip2 >../snake-$(VERSION).tar.bz2

