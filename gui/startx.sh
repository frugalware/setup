#!/bin/sh

Xorg -nolisten tcp &
sleep 1
openbox --config-file /usr/share/openbox/obconfig.xml &
sleep 1
feh --bg-scale /usr/share/themes/backgrounds/fw.jpg &
fwife && killall Xorg & exec /bin/sh