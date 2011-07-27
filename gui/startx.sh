#!/bin/sh

Xorg -nolisten tcp -modulepath /usr/lib/xorg/modules &
sleep 1
openbox --config-file /usr/share/openbox/obconfig.xml &
sleep 1
xloadimage -onroot -quiet -fullscreen /usr/share/themes/backgrounds/fw.jpg &
export GTK_PATH="/usr/lib/gtk-2.0/"
fwife && killall Xorg & exec /bin/sh