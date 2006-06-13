# Quick-and-dirty Makefile for gas2masm
#
#	$Id: Makefile,v 1.3 2006/06/13 18:22:18 vvd0 Exp $
#

all:	gas2masm
gas2masm:	gas2masm.c
	cc $(CFLAGS) -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -o gas2masm gas2masm.c
