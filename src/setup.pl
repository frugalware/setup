# (c) 2003-2004 Vajna Miklos <vmiklos@frugalware.org>
# (c)           Patrick J. Volkerding, <volkerdi@slackware.com>

setup="Setup"

# greeting section
welcome="Witaj w instalatorze Frugalware $osver"
greetstring="Witaj w¶ród u¿ytkowników Frugalware!\n\n \
Tworz±c Frugalware stawiali¶my na prostotê i ³atw± obs³ugê. Mamy nadziejê ¿e nasz produkt spodoba ci siê.\n\n \
Twórcy Frugalware"

# keyboard section
kbdconf="Konfiguracja Klawiatury"
selmapt="Wybierz zestaw znaków klawiatury"
selmapd="Mo¿esz wybraæ mapê znaków dla swojej klawiatury. Je¿eli chcesz mieæ polsk± klawiaturê wybierz pl2 lub pl"

# hotplug section
hotplug="Wykrywanie sprzêtu"
hotplugt="Proszê czekaæ"
hotplugd="Szukam kart SCSI i PCI"

# selecting media section
searchmedia="Wybieram ¼ród³o instalacji"
scanmediat="Przeszukujê"
scanmediad="Szukam CD/DVD instalacyjnego Frugalware..."
mediafoundt="No¶nik CD/DVD znaleziony"
mediafoundd="No¶nik instalacyjny znaleziony w "

# partitioning section
partitioning="Tworzê partycjê"
createraid="Tworzê RAID"
selppt="Wybierz program do partycjonowania"
selppd="Z jakiego programu chcesz skorzystaæ:"
selhdt="Wybierz dysk do partycjonowania"
selhdd="Wybierz dysk do partycjonowania, dostêpne dyski to:"
fdiskd="Tradycyjny program do partycjonowania"
cfdiskd="£atwiejsza w obs³udzê quasi-graficzna wersja fdisk"

# swap section
setswap="Wybór partycji SWAP"
noswaptitle="Nie znaleziono partycji swap"
noswap="Nie stworzy³e¶ partycji SWAP. Czy chcesz kontynuowaæ?"
detectedswapt="Znaleziono partycje SWAP"
detectedswapd="Wybierz partycjê SWAP dla Frugalware:"
formatpartst="Formatowanie partycji SWAP"
formatpartsd1="Formatujê"
formatpartsd2="jako SWAP i sprwadzam na obecno¶æ uszkodzonych bloków"

# rootdev section
rootdevstring="Wybór partycji g³ównej"
miscdevstring="Wybór partycji"
selrootdevt="Wybierz partycjê g³ówn± /"
select="Wybierz"
continue="Kontuuj"
selrootdevd="Wybierz partycjê g³ówn± (/) z poni¿szej listy."
formatpart="Formatowanie Partycji"
wantformatq="Je¿eli partycja nie by³a formatowana to powiniene¶ zrobiæ to teraz\nFormatowanie usunie z niej wszystkie dane. Kontynuowaæ?"
formatt="Formatuj"
fromatd="Szybkie formatowanie"
checkt="Sprawd¼"
checkd="Wolne formatowanie z poszukiwaniem uszkodzonych bloków"
nofromatt="Nie"
noformatd="Nie formatuj partycji"
ext2predesc="Ext2 to podstawowy system plików linuxa.\n"
reiserpredesc="ReiserFS to nowoczesny system plików z ksiêgowaniem.\n"
ext3predesc="Ext3 to system ext2 z ksiêgowaniem.\n"
jfspredesc="JFS to system plików z ksiêgowaniem IBMa u¿ywany na jego maszynach.\n"
xfspredesc="XFS to system plików z ksiêgowaniem SGI.\n"
selectfst="Wybór system plików dla"
selectfsd="Wybierz system plików:\n"
ext2shortdesc="Standardowy system ext2"
reisershortdesc="System z ksiêgowaniem Reisera"
ext3shortdesc="Pospolity system plików z ksiêgowaniem"
jfsshortdesc="system IBMa"
xfsshortdesc="system SGI"
duringformatt="Formatujê"
formatdevicestring="Formatujê partycjê:"
formatfsstring="System plików:"

# linux partitions section

sellinuxpartst="Wybierz inne partycje dla /etc/fstab"
sellinuxpartsd="Chyba masz wiêcej dostêpnych partycji. Mo¿esz montowaæ je np. jako /home czy te¿ wybraæ zwyk³e montowanie w /mnt. Je¿eli nie chcesz nic ustawiaæ albo ju¿ to zrobi³e¶ przejd¼ dalej poprzez: <$continue>"
inuse="w u¿yciu"
askwherelt="Wybierz punkt montowania dla"
askwhereld="Podaj punkt montowania, np. /usr/local lub /mnt/hdaX"

# dos partitions section

seldospartst="Dodanie partycji do /etc/fstab"
seldospartsd="By partycje te by³y widoczne w systemie musz± byæ dodane do /etc/fstab. Wybierz interesuj±ce ciê partycje i ustaw punkty montowania.\n\nJe¿eli nie chcesz niæ zmieniaæ, lub ju¿ to zrobi³e¶ przejd¼ dalej poprzez: <$continue>"

# packages section
mirrorconfstring="Wybierz mirror"
mirrorconft="Wybierz mirror pakietów"
mirrorconfd="Mo¿esz wybraæ najbli¿szy tobie serwer, co przy¶pieszy instalacjê."
categorychk="Szukam kategorii, proszê czekaæ..."
pkgchk="Szukam pakietów. Proszê czekaæ..."
categorystring="Wybieram kategoriê"
pkgstring="Wybieram pakiety"
instpkgstring="Instalowanie pakietów"
sect="grupa" #will be displaied like this: Installing packages (base section)
pleaseselectcategories="Wybierz kategorie pakietów do instalacji"
pleaseselectpkgs="Wybierz pakiety do instalacji"

neednextt="Umie¶æ nastêpny no¶nik instalacyjny"
neednextd="Umie¶æ nastêpny no¶nik i wci¶nij Enter."
continued="Instaluj pakiety z kolejnego no¶nika"
quit="Wyjd¼"
quitd="Przerwij instalacjê pakietów"

instpkg="Instalujê wybrane pakiety"
doneinstpkg="Zainstalowano wszystkie pakiety"
errinstpkg="B³êdy przy instalacji pakietów"
pressenter="Wci¶nij Enter by kontynuowaæ..."

# configure section
confstring="Konfiguracja zainstalowanego systemu"
confkmodt="Konfiguracja modu³ów kernela"
confkmodd="Aktualizacja zale¿no¶ci modu³ów..."

nopasswdt="Brak has³a roota"
nopasswdd="Czy ustawiæ has³o roota (ZALECANE)?"

nonormalusert="Brak zwyk³ego u¿ytkownika"
nonormaluserd="Obecnie istnieje tylko root. Zaleca siê utworzenie i korzystanie z konta zwyk³ego u¿ytkownika. Utworzyæ konto zwyk³ego u¿ytkownika ?"

endsetupt="Instalacja zakoñczona"
erebootd="Wybierz Nie - dostaniesz konsolê, wybierz Tak - restart systemu"
endsetupd="Instalacja Frugalware $osver zakoñczona. $erebootd"
ferrort="B³±d instalacji"
ferrord="Wyst±pi³ b³±d krytyczny w czasie instalacji. $erebootd"

# vim: ft=sh
