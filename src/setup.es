# (c) 2003-2004 Vajna Miklos <vmiklos@frugalware.org>
# (c)           Patrick J. Volkerding, <volkerdi@slackware.com>

setup="Setup"

# greeting section
welcome="Bienvenido a Frugalware $osver"
greetstring="Bienvenido a la comunidad de usuarios de Frugalware!\n\n \
Nuestro objetivo creando Frugalware, es ayudar a hacer su trabajo más simple y rápido. Esperamos que disfrute nuestro producto.\n\n \
Los Desarrolladores de Frugalware"

# keyboard section
kbdconf="Configuración del teclado"
selmapt="Selección del mapeo del teclado"
selmapd="Debe elegir uno de los siguientes mapeos de teclado.  Si no selecciona un mapeo de teclado, 'qwerty/us.map.gz' (the US keyboard map) se instala por defecto.  Use las teclas UP/DOWN y PageUp/PageDown para desplazarse en la lista de opciones."

# hotplug section
hotplug="Detectando hardware"
hotplugt="Por favor, aguarde"
hotplugd="Buscando dispositivos SCSI y tarjetas PCI"

# selecting media section
searchmedia="Elegir dispositivo de origen"
scanmediat="Buscando"
scanmediad="Buscando un CD/DVD que contenga el disco de instalación de Frugalware (install disc)"
mediafoundt="Se encontró el CD/DVD"
mediafoundd="El disco de instalación de Frugalware fue encontrado en el dispositivo"

# partitioning section
partitioning="Crear Partición"
createraid="Crear RAID"
selppt="Seleccionar programa de particionamiento"
selppd="Elija el programa que desee utilizar para particionar:"
selhdt="Seleccione el disco rígido a particionar"
selhdd="Por favor, seleccione el disco rígido a particionar. Se listan los disponibles:"
fdiskd="El programa para particionar tradicional de Linux"
cfdiskd="Versión amigable de fdisk (basada en curses)"

# swap section
setswap="Configurando el espacio de intercambio (swap)"
noswaptitle="No se detectó partición de intercambio (swap)"
noswap="Usted no ha creado una partición de intercambio (swap). ¿Desea continuar la instalación sin crearla?"
detectedswapt="Se detectó una partición de intercambio (swap)"
detectedswapd="Por favor, elija que partición de intercambio (swap) utilizará Frugalware:"
formatpartst="Formateando la partición de intercambio (swap)"
formatpartsd1="Formateando"
formatpartsd2="como partición de intercambio (swap) y buscando sectores defectuosos"

# rootdev section
rootdevstring="Configure la partición raíz (root)"
miscdevstring="Configurando particiones"
selrootdevt="Seleccione la partición de instalación de Linux"
select="Seleccione"
continue="Continúe"
selrootdevd="Por favor, seleccione una partición de la siguiente lista para ser usada como partición raíz (/)"
formatpart="Formateando partición"
wantformatq="Si esta partición no ha sido formateada, deberá formatearla. \n NOTA: Se borrará todos los datos en ella ¿Desea formatear la partición?"
formatt="Formateando"
fromatd="Formato rápido, sin comprobar sectores defectuosos"
checkt="comprobar"
checkd="Formato lento que comprueba sectores defectuosos"
nofromatt="No"
noformatd="No, no formatear la partición"
ext2predesc="Ext2 es el sistema de archivos tradicional de Linux. Es rápido y estable\n"
reiserpredesc="ReiserFS es un sistema de archivos de tipo \"journaling\" que almacena todos los archivos y sus nombres en una estructura de árbol balanceado\n"
ext3predesc="Ext3 es la versión \"journaling\" del sistema de archivos Ext2.\n"
jfspredesc="JFS es un sistema de archivos de tipo \"journaling\" de IBM, actualmente usado en los enterprise servers de IBM.\n"
xfspredesc="XFS es un sistema de archivos de tipo journaling de SGI originado en IRIX.\n"
selectfst="Elegir sistema de archivos para"
selectfsd="Por favor, elija el tipo de sistema de archivos para usar en el dispositivo especificado. Aquí hay una descripción de los sistemas de archivos disponibles:\n"
ext2shortdesc="Sistema de archivos Linux ext2fs, estandar de Linux"
reisershortdesc="Sistema de archivos (journaling)de Hans Reiser"
ext3shortdesc="Versión journaling del sistema de archivos ext2fs"
jfsshortdesc="Sistema de archivos(journaled) de IBM"
xfsshortdesc="Sistema de archivos (journaling) de SGI"
duringformatt="Formateando"
formatdevicestring="Formateando el dispositivo:"
formatfsstring="Sistema de archivos:"

# linux partitions section

sellinuxpartst="Seleccione otras particiones Linux para /etc/fstab"
sellinuxpartsd="Parece que tiene más de una partición de tipo Linux. Puede distribuir su sistema Linux en más de una partición. Actualmente usted ha montado solamente la partición /. Tal vez desee montar directorios como /home o /usr/local en particiones separadas. No debe intentar montar /etc, /sbin, o /bin en sus propias particiones porque contienen utilidades necesarias para el funcionamiento del sistema y para poder montar particiones. Tampoco use particiones que ya ha asignado antes.Por favor, seleccione una de las particiones Linux listadas abajo, o si desea continuar seleccione <$continue>" 
inuse="en uso"
askwherelt="Seleccione punto de montaje para"
askwhereld="Debe especificar el punto de montaje de la nueva partición. Por ejemplo, si desea montarla en /usr/local, entonces responda: /usr/local\n\n ¿Donde desea montar esta partición?"

# dos partitions section

seldospartst="Seleccionar partición para agregarla a /etc/fstab"
seldospartsd="Para hacer visibles otras particiones desde Linux, necesitamos agregarlas a /etc/fstab. Por favor elija particiones para ser agregadas a /etc/fstab, si desea continuar, seleccione <$continue>"

# packages section
mirrorconfstring="Elegir un servidor mirror"
mirrorconft="Por favor, elija un servidor mirror"
mirrorconfd="Puede especificar un servidor mirror más cercano a su ubicación para acelerar el proceso de descarga. En la mayoría de los casos la opción por defecto (default) es una buena opción."
categorychk="Buscando categorías. Por favor, espere..."
pkgchk="Buscando paquetes. Por favor, espere..."
categorystring="Seleccionando categorías"
pkgstring="Seleccionando paquetes"
instpkgstring="Instalando paquetes"
sect="(sección)" #will be displaied like this: Installing packages (base section)
pleaseselectcategories="Por favor, seleccione que categorías desea instalar"
pleaseselectpkgs="Por favor, seleccione que paquetes desea instalar"

neednextt="Inserte el disco siguiente"
neednextd="Por favor, inserte el siguiente disco de instalación de Frugalware y presione enter para continuar instalando los paquetes."
continued="Instalando paquetes del siguiente disco"
quit="Salir"
quitd="Salir de la instalación de paquetes y terminar"

instpkg="Instalar los paquetes seleccionados"
doneinstpkg="Se completó la instalación de los paquetes seleccionados"
errinstpkg="Ocurrieron errores durante la instalación de los paquetes"
pressenter="Presione ENTER para continuar..."

# configure section
confstring="Configurando el sistema instalado"
confkmodt="Configurando los módulos del kernel"
confkmodd="Actualizando las dependencias de módulos..."

nopasswdt="No se definió la contraseña (password) de root"
nopasswdd="No existe actualmente una contraseña para la cuenta del administrador del sistema (root). Se recomienda que la defina ahora así estará activa la primera vez que reinicie la computadora. Esto es de especial importancia si usted utiliza un kernel que habilite conexiones de red y si la computadora se conecta a internet ¿Desea definir una contraseña para root?"
nonormalusert="No se encontró una cuenta de usuario (user)"
nonormaluserd="No existe actualmente una cuenta no-root. Se recomienda crear una. ¿Desea crear una cuenta de usuario ahora?"

endsetupt="Instalación completada"
erebootd="Si desea realizar algo especial, pulse no, y saldrá a consola (shell). Desea reiniciar su computadora ahora?"
endsetupd="Se completó satisfactoriamente la instalación y configuración del sistema. Esperamos que Frugalware $osver sea de su agrado. $erebootd"
ferrort="Error en la instalación"
ferrord="Ocurrió un error fatal en la instalación. $erebootd"

# vim: ft=sh
