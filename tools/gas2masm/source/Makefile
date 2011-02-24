# Quick-and-dirty Makefile for gas2masm
#
#	$Id$
#

all:	gas2masm
gas2masm:	gas2masm.c
	cc $(CFLAGS) -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -o gas2masm gas2masm.c

clean : 
	-rm -f *.o gas2masm

