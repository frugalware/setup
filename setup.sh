#!/bin/bash

bindir=/root/programz/frugalware/frugalware-current/frugalware
core=(glibc ncurses bash coreutils popt chkconfig)

# do NOT modify anything above this line

. en

# strings 
selcat=
selpkg=
instlog=
yesdeps=
selectcategoriesbacktitle="$categorystring - FrugalWare `cat /etc/frugalware-release |cut -d ' ' -f 2` $setup"
selectpkgsbacktitle="$pkgstring - FrugalWare `cat /etc/frugalware-release |cut -d ' ' -f 2` $setup"
categorysearchbacktitle=$selectcategoriesbacktitle
pkgsearchbacktitle=$selectpkgsbacktitle
installpkgsbacktitle="$instpkgstring - FrugalWare `cat /etc/frugalware-release |cut -d ' ' -f 2` $setup"

# functions
info()
{
	echo -e "\033[1;32m==>\033[1;0m \033[1;1m$1\033[1;0m" >&2
}

categorysize ()
{
	categorytempfile=`mktemp`
	chmod +x $categorytempfile
	echo -n "( du -ch ">$categorytempfile
	for i in `cat $bindir/../Packages.lst |grep ^$1|sed 's|.*/\(.*\)|\1|'`
	do
		echo -n "$bindir/$i ">>$categorytempfile
	done
	echo ") |tail -1 |sed 's/\(.*\)\t.*/\1/'" >>$categorytempfile
	echo `$categorytempfile`
	rm $categorytempfile
}

pkgsize()
{
	du -h $1 |sed 's/\(.*\)\t.*/\1/'
}

category_search()
{
	dialog --backtitle "$categorysearchbacktitle" --infobox "$categorychk" 0 0
}
pkg_search()
{
	dialog --backtitle "$pkgsearchbacktitle" --infobox "$pkgchk" 0 0
}

category_select ()
{
	category_search
	selecttempfile=`mktemp`
	whichselectedtempfile=`mktemp`
	whichselectedtempfile2=`mktemp`
	chmod +x $selecttempfile
	echo -n "dialog --backtitle \"$selectcategoriesbacktitle\" --title \"$categorystring\" --checklist \"$pleaseselectcategories\" 0 0 0 " >$selecttempfile
	categories=`cat $bindir/../Packages.lst|sed 's|\(.*\)/.*|\1|'|sort|uniq`
	for i in $categories
	do
		echo -n "$i \"(`categorysize $i`)\" on ">>$selecttempfile
	done
	$selecttempfile 2>$whichselectedtempfile || exit 1
	rm $selecttempfile
	for i in `cat $whichselectedtempfile`
	do
		echo -n "$i "|sed 's/"//g' >>$whichselectedtempfile2
	done
	rm $whichselectedtempfile
	selcat=`cat $whichselectedtempfile2`
	rm $whichselectedtempfile2
}

package_select()
{
	pkg_search
	printpkgtemp=`mktemp`
	chmod +x $printpkgtemp
	prepkglst=`mktemp`
	echo -n "cat $bindir/../Packages.lst |egrep ' " >$printpkgtemp
	for i in $*
	do
		echo -n "|^$i" >>$printpkgtemp
	done
	echo "| '|sed 's|.*/\(.*\)|\1|'">>$printpkgtemp
	$printpkgtemp >$prepkglst
	rm $printpkgtemp
	pkglstselect=`mktemp`
	whichselected=`mktemp`
	whichselected2=`mktemp`
	whichselected3=`mktemp`
	chmod +x $pkglstselect
	echo -n "dialog --backtitle \"$selectpkgsbacktitle\" --title \"$pkgstring\" --checklist \"$pleaseselectpkgs\" 0 0 0 " >$pkglstselect
	for i in `cat $prepkglst`
	do
		echo -n "$i \"(`pkgsize $bindir/$i`)\" on ">>$pkglstselect
	done
	rm $prepkglst
	$pkglstselect 2>$whichselected || exit 1
	rm $pkglstselect
	for i in `cat $whichselected`
	do
		echo -n "$i "|sed 's/"//g' >>$whichselected2
	done
	rm $whichselected
	for i in `cat $whichselected2`
	do
		echo $i|sed 's/\(.*\)-[0-9].*-[0-9].*fpm/\1/' \
			>>$whichselected3
	done
	rm $whichselected2
	selpkg=`cat $whichselected3`
	rm $whichselected3
}

install_packages()
{
	clear
	if ! [ -d tmp ]; then
		info "$instcore"
		mkdir tmp
		( mkdir -p var/lib/pacman/current
		  cd var/lib/pacman/current
		  tar xzf $bindir/current.fdb )
		echo "[current]" >/etc/pacman.conf
		echo "Server = file://$bindir" >>/etc/pacman.conf
		( mkdir -p var/cache/pacman
		  cd var/cache/pacman 
		  ln -s $bindir pkg )
		info "$doneinstcore"
	fi
	info "$instpkg"
	pkginstall=`mktemp`
	chmod +x $pkginstall
	echo -n "pacman -S -r ./ --noconfirm ${core[@]} ">$pkginstall
	for i in $*
	do
		echo -n "$i ">>$pkginstall
	done
	$pkginstall && info "$doneinstpkg" || info "$errinstpkg"
	rm $pkginstall
}

# main
category_select
package_select $selcat
install_packages $selpkg
#clear
