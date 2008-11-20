#
# qqshka: this file is work around about difference in BSD and GNU make...
#

default: qwfwd

.PHONY: fixmake

fixmake:
	@./fixmake.sh

qwfwd qwfwd.exe qwfwd-dl clean:	fixmake
	@make -f Makefile.tmp $@
	@rm -rf Makefile.tmp
