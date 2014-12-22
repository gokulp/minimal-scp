#
# This is an example Makefile for a countwords program.  This
# program uses both the scanner module and a counter module.
# Typing 'make' or 'make count' will create the executable file.
#

# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
CC = gcc
CFLAGS  = -g
LDLIBS = -lgcrypt
INCLUDES = -I./

# typing 'make' will invoke the first target entry in the file 
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#
default: gatordec gatorcrypt

# To create the executable
#
gatordec:  server.o decrypt.o encDecGenFunc.o fileTrnsfr.o
	$(CC) $(CFLAGS) -o gatordec server.o decrypt.o encDecGenFunc.o fileTrnsfr.o $(LDLIBS)

server.o:   server.c public.h
	$(CC) $(CFLAGS) -c server.c $(INCLUDES)

# To create the object file encDecGenFunc.o
#
encDecGenFunc.o:  encDecGenFunc.c public.h encDecGenFunc.h
	$(CC) $(CFLAGS) -c encDecGenFunc.c $(INCLUDES)

# To create the object file decrypt.o
#
decrypt.o:  decrypt.c public.h 
	$(CC) $(CFLAGS) -c decrypt.c $(INCLUDES)

# To create the object file fileTrnsfr.o
#
fileTrnsfr.o:  fileTrnsfr.c public.h fileTrnsfr.h
	$(CC) $(CFLAGS) -c fileTrnsfr.c $(INCLUDES)

# To create the executable
#
gatorcrypt:  encrypt.o encDecGenFunc.o fileTrnsfr.o
	$(CC) $(CFLAGS) -o gatorcrypt encrypt.o encDecGenFunc.o fileTrnsfr.o $(LDLIBS)

# To create the object file decrypt.o
#
encrypt.o:  decrypt.c public.h
	$(CC) $(CFLAGS) -c encrypt.c $(INCLUDES)

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean: 
	$(RM) gator* *.o *~

