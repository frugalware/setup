#!/bin/bash

bindir=/root/programz/frugalware/frugalware-current/frugalware
# core=(glibc ncurses bash coreutils popt chkconfig frugalware)
core=(bash coreutils chkconfig frugalware grep sed)
logdev=/dev/tty4
target=/mnt/target

### clear
### mkswap -c!!!

# do NOT modify anything above this line

. en

### strings 
# swap section
swapparts=
setswapbacktitle="$setswap - FrugalWare `cat /etc/frugalware-release |cut -d ' ' -f 2` $setup"

# rootdev section
prefstab=
rootdevbacktitle="$rootdevstring - FrugalWare `cat /etc/frugalware-release |cut -d ' ' -f 2` $setup"

# linux partitions section
towhere=
miscdevbacktitle="$miscdevstring - FrugalWare `cat /etc/frugalware-release |cut -d ' ' -f 2` $setup"

# packages section
selcat=
selpkg=
instlog=
yesdeps=
selectcategoriesbacktitle="$categorystring - FrugalWare `cat /etc/frugalware-release |cut -d ' ' -f 2` $setup"
selectpkgsbacktitle="$pkgstring - FrugalWare `cat /etc/frugalware-release |cut -d ' ' -f 2` $setup"
categorysearchbacktitle=$selectcategoriesbacktitle
pkgsearchbacktitle=$selectpkgsbacktitle
installpkgsbacktitle="$instpkgstring - FrugalWare `cat /etc/frugalware-release |cut -d ' ' -f 2` $setup"

### functions
# swap section

selswappart()
{
	fdisk -l|grep -q 'Linux swap$'
	if [ "$?" != 0 ]; then
		dialog --backtitle "$setswapbacktitle" --title "$noswaptitle" \
			--yesno "$noswap" 0 0 || exit 1
	else
		swapqfile=`mktemp`
		swapafile=`mktemp`
		chmod +x $swapqfile
		echo -n "dialog --backtitle \"$setswapbacktitle\" --title \"$detectedswapt\" --checklist \"$detectedswapd\" 0 0 0 ">$swapqfile
		fdisk -l|grep 'Linux swap$'|tr -s ' '|sed 's/\(.*\) [0-9]* [0-9]* [0-9+]* [0-9]* \(.*\)/\1 "\2" On \\/'>>$swapqfile
		echo >>$swapqfile
		$swapqfile 2>$swapafile
		rm $swapqfile
		swapparts=`cat $swapafile|sed 's/"//g'`
		rm $swapafile
	fi
}

doswap ()
{
	[ -z "$prefstab" ] && prefstab=`mktemp`
	printf "%-16s %-16s %-11s %-16s %-3s %s\n" "none" "/proc" "proc" "defaults" "0" "0" >>$prefstab
	printf "%-16s %-16s %-11s %-16s %-3s %s\n" "none" "/sys" "sysfs" "defaults" "0" "0" >>$prefstab
	printf "%-16s %-16s %-11s %-16s %-3s %s\n" "devpts" "/dev/pts" "devpts" "gid=5,mode=620" "0" "0" >>$prefstab
	for i in $*
	do
		dialog --backtitle "$setswapbacktitle" --title "$formatpartst" --infobox "$formatpartsd1 $i $formatpartsd2" 0 0
		#mkswap -c $i >$logdev
		mkswap $i >$logdev
		printf "%-16s %-16s %-11s %-16s %-3s %s\n" "$i" "swap" "swap" "defaults" "0" "0" >>$prefstab
	done
}

# rootdev dection

lstparts()
{
	for i in `fdisk -l|grep 'Linux$'|cut -d ' ' -f 1`
	do
		size=`fdisk -s $i`
		unset altname
		if grep -q $i $prefstab; then
			# added already
			on=`grep $i $prefstab |tr -s ' ' |cut -f 2 -d ' '`
			altname="$i $on Linux ${size}K"
		fi
		if [ -z "$altname" ]; then
			echo "\"$i\" \"Linux ${size}K\" \\" >>$1
		else
			echo "\"($inuse)\" \"$altname\" \\" >>$1
		fi
	done
}

selrootdev()
{
	[ -z "$prefstab" ] && prefstab=`mktemp`
	selrootpart=`mktemp`
	chmod +x $selrootpart
	rootpartf=`mktemp`
	echo "dialog --backtitle \"$rootdevbacktitle\" --title \"$selrootdevt\" \\" >$selrootpart
	echo "--ok-label \"$select\" --menu \"$selrootdevd\" 0 0 0 \\">>$selrootpart
	lstparts $selrootpart
	echo "2>$rootpartf">>$selrootpart
	$selrootpart || exit 1 # aborted when choosing root device
	rm $selrootpart
	rootdev=`cat $rootpartf`
	rm $rootpartf
}

mkfss() # $1 which device, $2 which fs, $3 how: $formatt or $checkt
{
	[ "$3" = "$checkt" ] && opts="-c"
	dialog --backtitle "$miscdevbacktitle" \
		--title "$duringformatt" \
		--infobox "$formatdevicestring $1 \n \
		$formatfsstring $2" 0 0
	mount|grep -q $1 && umount $1
	if [ "$2" = "ext2" ]; then
		mke2fs $opts $1 >$logdev 2>&1
	elif [ "$2" = "reiserfs" ]; then
		echo y | mkreiserfs $1 >$logdev 2>&1
	elif [ "$2" = "ext3" ]; then
		mke2fs -j $opts $1 >$logdev 2>&1
	elif [ "$2" = "jfs" ]; then
		mkfs.jfs -q $opts $1 >$logdev 2>&1
	elif [ "$2" = "xfs" ]; then
		mkfs.xfs -f $1 >$logdev 2>&1
	fi
}

formatdev() # $1: which device
{
	junk=`mktemp`
	dialog --backtitle "$miscdevbacktitle" --title "$formatpart $1" \
	--menu "$wantformatq" 0 0 0 \
	"$formatt" "$fromatd" "$checkt" "$checkd" "$nofromatt" "$noformatd" 2>$junk || exit 1 # aborted when choosing format mode
	howformat=`cat $junk`
	rm $junk
	if [ "$howformat" != "$nofromatt" ]; then
		unset ext2desc reiserdesc ex3desc jfsdesc xfsdesc
		grep ext2 -q /proc/filesystems && \
			ext2desc=$ext2predesc && default=ext2
		grep reiserfs -q /proc/filesystems && \
			reiserdesc=$reiserpredesc && default=reiserfs
		grep ext3 -q /proc/filesystems && \
			ext3desc=$ext3predesc && default=ext3
		grep jfs -q /proc/filesystems && \
			jfsdesc=$jfspredesc && default=jfs
		grep ' xfs' -q /proc/filesystems && \
			xfsdesc=$xfspredesc && default=xfs
		selfsf=`mktemp`
		chmod +x $selfsf
		echo "dialog --backtitle \"$miscdevbacktitle\" \
		--title \"$selectfst $1\" --default-item \"$default\" \
		--menu \"$selectfsd \\">$selfsf
		echo "$ext2desc $reiserdesc $ext3desc $jfsdesc $xfsdesc\" \
			0 0 0 \\">>$selfsf
		[ -z "$ext2desc" ] || echo "\"ext2\" \
			\"$ext2shortdesc\" \\" >>$selfsf
		[ -z "$reiserdesc" ] || echo "\"reiserfs\" \
			\"$reisershortdesc\" \\" >>$selfsf
		[ -z "$ext3desc" ] || echo "\"ext3\" \
			\"$ext3shortdesc\" \\" >>$selfsf
		[ -z "$jfsdesc" ] || echo "\"jfs\" \
			\"$jfsshortdesc\" \\" >>$selfsf
		[ -z "$xfsdesc" ] || echo "\"xfs\" \
			\"$xfsshortdesc\" \\" >>$selfsf
		junk=`mktemp`
		echo "2>$junk">>$selfsf
		$selfsf || exit 1 # aborted when choosing fs
		rm $selfsf
		mkfss $1 `cat $junk` $howformat
		rm $junk
	fi
}

mountdev() # $1: which device $2: mount point
{
	[ -d $target/$2 ] || mkdir -p $target/$2
	mount $1 $target/$2 >$logdev 2>&1
	sleep 1
	type=`mount | grep ^$1 | cut -f 5 -d ' '`
	printf "%-16s %-16s %-11s %-16s %-3s %s\n" "$1" "$2" "$type" "defaults" "1" "1" >> $prefstab
}

# linux partitions section

askwhere() # $1: which device
{
	askwheref=`mktemp`
	dialog --backtitle "$miscdevbacktitle" --title "$askwherelt $1" \
		--inputbox "$askwhereld" 0 0 2>$askwheref || return 1
	towhere=`cat $askwheref`
	rm $askwheref
	# cosmetics
	[ "$towhere" = "" ] && return 1
	[ "`echo "$towhere" | cut -b1`" = " " ] && return 1
	[ "`echo "$towhere" | cut -b1`" = "/" ] || towhere="/$towhere"
}

setup_linux() {
	[ "`fdisk -l|grep Linux$|wc -l`" != 1 ] || return 1
	while /bin/true
	do
		sellinuxparts=`mktemp`
		chmod +x $sellinuxparts
		linuxpartsf=`mktemp`
		echo "dialog --backtitle \"$miscdevbacktitle\" --title \"$sellinuxpartst\" \\" >$sellinuxparts
		echo "--ok-label \"$select\" --cancel-label "$continue" --menu \"$sellinuxpartsd\" 0 0 0 \\">>$sellinuxparts
		lstparts $sellinuxparts
		echo "2>$linuxpartsf">>$sellinuxparts
		$sellinuxparts || break; # aborted when choosing linux parts
		rm $sellinuxparts
		linuxpart=`cat $linuxpartsf`
		rm $linuxpartsf
		if [ "$linuxpart" != "($inuse)" ]; then
			formatdev $linuxpart
			askwhere $linuxpart || continue
			mountdev $linuxpart $towhere
		fi
	done
	rm $sellinuxparts
	rm $linuxpartsf
}

# dos partitions section

lstdosparts()
{
	for i in `fdisk -l|egrep 'Win95 F|Win98 F|HPFS|W95 F|FAT12|FAT16'|cut -d ' ' -f 1`
	do
		size=`fdisk -s $i`
		unset altname
		if grep -q $i $prefstab; then
			# added already
			on=`grep $i $prefstab |tr -s ' ' |cut -f 2 -d ' '`
			altname="$i $on Linux ${size}K"
		fi
		if [ -z "$altname" ]; then
			echo "\"$i\" \"Linux ${size}K\" \\" >>$1
		else
			echo "\"($inuse)\" \"$altname\" \\" >>$1
		fi
	done
}

setup_dos() {
	[ "`fdisk -l|egrep 'Win95 F|Win98 F|HPFS|W95 F|FAT12|FAT16'|wc -l`" != 0 ] || return 1
	while /bin/true
	do
		seldosparts=`mktemp`
		chmod +x $seldosparts
		dospartsf=`mktemp`
		echo "dialog --backtitle \"$miscdevbacktitle\" --title \"$seldospartst\" \\" >$seldosparts
		echo "--ok-label \"$select\" --cancel-label "$continue" --menu \"$seldospartsd\" 0 0 0 \\">>$seldosparts
		lstdosparts $seldosparts
		echo "2>$dospartsf">>$seldosparts
		$seldosparts || break; # aborted when choosing dos parts
		rm $seldosparts
		dospart=`cat $dospartsf`
		rm $dospartsf
		if [ "$dospart" != "($inuse)" ]; then
			askwhere $dospart || continue
			mountdev $dospart $towhere
		fi
	done
	rm $seldosparts
	rm $dospartsf
}

# packages section

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
	# preparing pkgdb
	info "$instpkg"
	if ! [ -d tmp ]; then
		mkdir tmp
		( mkdir -p var/lib/pacman/current
		  cd var/lib/pacman/current
		  tar xzf $bindir/current.fdb )
		echo "[current]" >/etc/pacman.conf
		echo "Server = file://$bindir" >>/etc/pacman.conf
		( mkdir -p var/cache/pacman
		  cd var/cache/pacman 
		  ln -s $bindir pkg )
	fi
	pkginstall=`mktemp`
	chmod +x $pkginstall
	echo -n "pacman -S -r ./ --noconfirm ${core[@]} ">$pkginstall
	for i in $*
	do
		echo -n "$i ">>$pkginstall
	done
	# we will leave setup if errors occured: no way to configure a
	# not-installed system ;-)
	$pkginstall && info "$doneinstpkg" || (info "$errinstpkg" && exit 1)
	rm $pkginstall
}

### main
# swap section
selswappart # selected swap partitions now in $swapparts
doswap $swapparts # format selected partitions

# rootdev section
selrootdev #root device now in $rootdev
formatdev $rootdev
mountdev $rootdev /
[ -d $target/etc ] || mkdir $target/etc
mv -f $prefstab $target/etc/fstab
prefstab=$target/etc/fstab

# linux partitions section
setup_linux

# dos partitions section
setup_dos

# packages section
cd $target
category_select # selected categories now in $selcat
package_select $selcat # select packages
install_packages $selpkg # install packages

# configure section
# these two have to move to a function, or better to move to a setup script
echo kernel
chroot ./ /sbin/depmod -a
echo cups
killall cupsd
chroot ./ /usr/sbin/cupsd
killall cupsd
echo begin network, press enter
read junk;
chroot ./ /sbin/netconfig
#clear
