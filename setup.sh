#!/bin/bash

bindir=/root/programz/frugalware/frugalware-current/frugalware

################
echo -n "installing core..."
mkdir tmp
for i in glibc ncurses bash coreutils popt chkconfig
do
	pacman -Ad $bindir/$i*fpm -r ./ >/dev/null
done
echo " done"
