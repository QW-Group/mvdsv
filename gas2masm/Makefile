# Quick-and-dirty Makefile for gas2masm
#
#	$Id: Makefile,v 1.2 2006/06/05 12:36:06 vvd0 Exp $
#

all:	gas2masm
gas2masm:	gas2masm.c
	cc $(CFLAGS) -Dstrcmpi=strcasecmp -Dstrnicmp=strncasecmp -o gas2masm gas2masm.c
