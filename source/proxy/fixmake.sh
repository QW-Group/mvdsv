#!/bin/sh


# OS determination
if [ x$1 != x ]; then
UNAME=$1
else
UNAME=`uname`
fi

case ${UNAME} in
FreeBSD | OpenBSD | NetBSD | DragonFly | BSD)
#	echo "${UNAME} system - use BSD Makefile."
	cp Makefile.BSD Makefile.tmp
	;;
Darwin | MacOSX | Linux | SunOS | GNU)
#	echo "${UNAME} system - use GNU Makefile."
	cp Makefile.GNU Makefile.tmp
	;;
*)
	echo "ERROR: Unknown system: ${UNAME}."
	exit 1
	;;
esac

