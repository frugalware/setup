# (c) 2003-2004 Vajna Miklos <vmiklos@frugalware.org>
# (c)           Patrick J. Volkerding, <volkerdi@slackware.com>

setup="Setup"

# greeting section
welcome="Witaj w instalatorze Frugalware $osver"
greetstring="Witaj wśród użytkowników Frugalware!\n\n \
Tworząc Frugalware stawialiśmy na prostotę i łatwą obsługę. Mamy nadzieję że nasz produkt spodoba ci się.\n\n \
Twórcy Frugalware"

# keyboard section
kbdconf="Konfiguracja Klawiatury"
selmapt="Wybierz zestaw znaków klawiatury"
selmapd="Możesz wybrać mapę znaków dla swojej klawiatury. Jeżeli chcesz mieć polską klawiaturę wybierz pl2 lub pl"

# hotplug section
hotplug="Wykrywanie sprzętu"
hotplugt="Proszę czekać"
hotplugd="Szukam kart SCSI i PCI"

# selecting media section
searchmedia="Wybieram źródło instalacji"
scanmediat="Przeszukuję"
scanmediad="Szukam CD/DVD instalacyjnego Frugalware..."
mediafoundt="Nośnik CD/DVD znaleziony"
mediafoundd="Nośnik instalacyjny znaleziony w "

# partitioning section
partitioning="Tworzę partycję"
createraid="Tworzę RAID"
selppt="Wybierz program do partycjonowania"
selppd="Z jakiego programu chcesz skorzystać:"
selhdt="Wybierz dysk do partycjonowania"
selhdd="Wybierz dysk do partycjonowania, dostępne dyski to:"
fdiskd="Tradycyjny program do partycjonowania"
cfdiskd="Łatwiejsza w obsłudzę quasi-graficzna wersja fdisk"

# swap section
setswap="Wybór partycji SWAP"
noswaptitle="Nie znaleziono partycji swap"
noswap="Nie stworzyłeś partycji SWAP. Czy chcesz kontynuować?"
detectedswapt="Znaleziono partycje SWAP"
detectedswapd="Wybierz partycję SWAP dla Frugalware:"
formatpartst="Formatowanie partycji SWAP"
formatpartsd1="Formatuję"
formatpartsd2="jako SWAP i sprwadzam na obecność uszkodzonych bloków"

# rootdev section
rootdevstring="Wybór partycji głównej"
miscdevstring="Wybór partycji"
selrootdevt="Wybierz partycję główną /"
select="Wybierz"
continue="Kontuuj"
selrootdevd="Wybierz partycję główną (/) z poniższej listy."
formatpart="Formatowanie Partycji"
wantformatq="Jeżeli partycja nie była formatowana to powinieneś zrobić to teraz\nFormatowanie usunie z niej wszystkie dane. Kontynuować?"
formatt="Formatuj"
fromatd="Szybkie formatowanie"
checkt="Sprawdź"
checkd="Wolne formatowanie z poszukiwaniem uszkodzonych bloków"
nofromatt="Nie"
noformatd="Nie formatuj partycji"
ext2predesc="Ext2 to podstawowy system plików linuxa.\n"
reiserpredesc="ReiserFS to nowoczesny system plików z księgowaniem.\n"
ext3predesc="Ext3 to system ext2 z księgowaniem.\n"
jfspredesc="JFS to system plików z księgowaniem IBMa używany na jego maszynach.\n"
xfspredesc="XFS to system plików z księgowaniem SGI.\n"
selectfst="Wybór system plików dla"
selectfsd="Wybierz system plików:\n"
ext2shortdesc="Standardowy system ext2"
reisershortdesc="System z księgowaniem Reisera"
ext3shortdesc="Pospolity system plików z księgowaniem"
jfsshortdesc="system IBMa"
xfsshortdesc="system SGI"
duringformatt="Formatuję"
formatdevicestring="Formatuję partycję:"
formatfsstring="System plików:"

# linux partitions section

sellinuxpartst="Wybierz inne partycje dla /etc/fstab"
sellinuxpartsd="Chyba masz więcej dostępnych partycji. Możesz montować je np. jako /home czy też wybrać zwykłe montowanie w /mnt. Jeżeli nie chcesz nic ustawiać albo już to zrobiłeś przejdź dalej poprzez: <$continue>"
inuse="w użyciu"
askwherelt="Wybierz punkt montowania dla"
askwhereld="Podaj punkt montowania, np. /usr/local lub /mnt/hdaX"

# dos partitions section

seldospartst="Dodanie partycji do /etc/fstab"
seldospartsd="By partycje te były widoczne w systemie muszą być dodane do /etc/fstab. Wybierz interesujące cię partycje i ustaw punkty montowania.\n\nJeżeli nie chcesz nić zmieniać, lub już to zrobiłeś przejdź dalej poprzez: <$continue>"

# packages section
mirrorconfstring="Wybierz mirror"
mirrorconft="Wybierz mirror pakietów"
mirrorconfd="Możesz wybrać najbliższy tobie serwer, co przyśpieszy instalację."
categorychk="Szukam kategorii, proszę czekać..."
pkgchk="Szukam pakietów. Proszę czekać..."
categorystring="Wybieram kategorię"
pkgstring="Wybieram pakiety"
instpkgstring="Instalowanie pakietów"
sect="grupa" #will be displaied like this: Installing packages (base section)
pleaseselectcategories="Wybierz kategorie pakietów do instalacji"
pleaseselectpkgs="Wybierz pakiety do instalacji"

neednextt="Umieść następny nośnik instalacyjny"
neednextd="Umieść następny nośnik i wciśnij Enter."
continued="Instaluj pakiety z kolejnego nośnika"
quit="Wyjdź"
quitd="Przerwij instalację pakietów"

instpkg="Instaluję wybrane pakiety"
doneinstpkg="Zainstalowano wszystkie pakiety"
errinstpkg="Błędy przy instalacji pakietów"
pressenter="Wciśnij Enter by kontynuować..."

# configure section
confstring="Konfiguracja zainstalowanego systemu"
confkmodt="Konfiguracja modułów kernela"
confkmodd="Aktualizacja zależności modułów..."

nopasswdt="Brak hasła roota"
nopasswdd="Czy ustawić hasło roota (ZALECANE)?"

nonormalusert="Brak zwykłego użytkownika"
nonormaluserd="Obecnie istnieje tylko root. Zaleca się utworzenie i korzystanie z konta zwykłego użytkownika. Utworzyć konto zwykłego użytkownika ?"

endsetupt="Instalacja zakończona"
erebootd="Wybierz Nie - dostaniesz konsolę, wybierz Tak - restart systemu"
endsetupd="Instalacja Frugalware $osver zakończona. $erebootd"
ferrort="Błąd instalacji"
ferrord="Wystąpił błąd krytyczny w czasie instalacji. $erebootd"

# vim: ft=sh
