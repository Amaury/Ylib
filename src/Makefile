# #####################################################################
#                              Makefile
# #####################################################################

# Library's static name (libNAME.a)
NAME =		liby.a
# Library's dynamic name (libNAME.so)
SONAME =	liby.so

# Name of source files (names.c)
SRC =		yarray.c	\
		ybase64.c	\
		ybin.c		\
		ycgi.c		\
		ydaemon.c	\
		ydom.c		\
		ydom_xpath.c	\
		yexception.c	\
		yhash.c		\
		yhashmap.c	\
		yhashtable.c	\
		yini.c		\
		yjson.c		\
		ylist.c		\
		ylock.c		\
		ylog.c		\
		ymemory.c	\
		yqprintable.c	\
		ysax.c		\
		ystr.c		\
		ytimer.c	\
		yurl.c		\
		yvalue.c	\
		yvar.c		\
		yvar_path.c


# Name of header files (names.h)
INCLUDES =	yarray.h	\
		ybase64.h	\
		ybin.h		\
		ycgi.h		\
		ydaemon.h	\
		ydom.h		\
		yexception.h	\
		y.h		\
		yhash.h		\
		yhashmap.h	\
		yhashtable.h	\
		yini.h		\
		yjson.h		\
		ylist.h		\
		ylock.h		\
		ylog.h		\
		ymemory.h	\
		yqprintable.h	\
		yresult.h	\
		ysax.h		\
		ystatus.h	\
		ystr.h		\
		ytimer.h	\
		yurl.h		\
		yvalue.h	\
		yvar.h


# #####################################################################

# Paths to header files
IPATH   =       -I.
# Path to libraries and lib's names
LDPATH  =       -L.
# Compiler options
EXEOPT  =       -O3 # -g for debug

# #####################################################################

CC      =	gcc
OBJS    =	$(SRC:.c=.o)

# Objects compilation options
CFLAGS	= -std=gnu11 -pedantic-errors -Wall -Wextra -Werror -Wmissing-prototypes \
	  -Wno-long-long -Wno-unused-parameter -Wno-unused-result -Wno-pointer-arith -D_GNU_SOURCE \
	  -D_LARGEFILE_SOURCE -D_THREAD_SAFE $(IPATH) $(EXEOPT) -fPIC
LDFLAGS	= -shared -Wl,-soname,$(SONAME)

# #####################################################################

.PHONY: lib copy clean all doc docclean

lib: $(SONAME) $(NAME) copy

$(SONAME): $(OBJS) $(SRC)
	$(CC) $(OBJS) $(LDFLAGS) -o $(SONAME)

$(NAME): $(OBJS) $(SRC)
	ar -r $(NAME) $(OBJS)
	ranlib $(NAME)

copy:
	if [ ! -d ../lib/ ]; then mkdir ../lib; fi
	cp $(SONAME) $(NAME) ../lib/
	if [ ! -d ../include ]; then mkdir ../include; fi
	cp $(INCLUDES) ../include/

clean:
	rm -f $(OBJS) $(NAME) $(SONAME) *~
	$(RM) ../lib/$(SONAME)
	if [ -d ../include ]; then cd ../include ; $(RM) $(INCLUDES); fi

all: clean lib

test: main.c
	$(CC) main.c -L. -ly -lm -Wl,-rpath -Wl,'$$ORIGIN/../lib' -o test

cleantest:
	rm -f test

alltest: cleantest test

doc:	# needs the HeaderBrowser program
	headerbrowser $(INCLUDES)

docclean:
	rm -rf hbresult-html

.c.o:
	$(CC) $(CFLAGS) -c $<

