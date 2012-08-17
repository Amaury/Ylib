# #####################################################################
#                              Makefile
# #####################################################################

# Library's static name (libNAME.a)
NAME	=	liby.a

SRC_HASH =	yhashmap.c	\
		yhashtable.c	\
		ystr.c

# Name of source files (names.c)
SRC       =	ymalloc.c	\
		ystr.c		\
		yvect.c		\
		ylog.c		\
		ybase64.c	\
		ysax.c		\
		ydom.c		\
		ydom_xpath.c	\
		yurl.c		\
		yqprintable.c	\
		ycgi.c		\
		ychrono.c	\
		ycrc.c		\
		yvalue.c	\
		ylock.c		\
		ytcp_server.c	\
		ynetwork.c

# Name of header files (names.h)
HEADS     =	ybase64.h	\
		ycgi.h		\
		ychrono.h	\
		ycrc.h		\
		ydefs.h		\
		ydom.h		\
		yerror.h	\
		ylog.h		\
		yqprintable.h	\
		ysax.h		\
		ystr.h		\
		yurl.h		\
		yvalue.h	\
		yvect.h		\
		ylock.h		\
		ytcp_server.h	\
		ynetwork.h

# #####################################################################

# Paths to header files
IPATH   =       -I.
# Path to libraries and lib's names
LDPATH  =       -L.
# Compiler options
EXEOPT  =       -O2 # -g for debug

# #####################################################################

CC      =	gcc
RM      =	/bin/rm -f
OBJS    =	$(SRC:.c=.o)
OBJS_HASH =	$(SRC_HASH:.c=.o)

# Objects compilation options
CFLAGS  =	-ansi -std=c90 -pedantic-errors -Wall -Wextra -Wmissing-prototypes \
		  -Wno-long-long $(IPATH) -D_GNU_SOURCE \
		  -D_LARGEFILE_SOURCE -D_THREAD_SAFE -fPIC
CFLAGS_CYGWIN =	-Wall -Wmissing-prototypes -Wno-long-long $(IPATH) \
		  -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_THREAD_SAFE

# #####################################################################

.PHONY: lib cygwin clean all cygall doc docclean

hash: $(OBJS_HASH) $(SRC_HASH)
	ar -r $(NAME) $(OBJS_HASH)
	ranlib $(NAME)

$(NAME): $(OBJS) $(SRC)
	ar -r $(NAME) $(OBJS)
	ranlib $(NAME)
	cp $(NAME) ../
	cp $(HEADS) ../../include/

cygwin: $(SRC)
	$(CC) $(CFLAGS_CYGWIN) -c $(SRC)
	ar -r $(NAME) $(OBJS)
	ranlib $(NAME)
	cp $(NAME) ../

clean:
	$(RM) $(OBJS) $(NAME) *~

all: clean $(NAME)

cygall: clean cygwin

doc:	# needs the HeaderBrowser program
	headerbrowser $(HEADS)

docclean:
	$(RM) -r hbresult-html

.c.o:
	$(CC) $(CFLAGS) -c $<

