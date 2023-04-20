#!/bin/bash
# po/rcr-gnome.ru_RU.UTF-8.po
TEMPLATE=po/rcr-gnome.pot
xgettext -k_ -o $TEMPLATE *.cpp rcr-gnome.glade
for f in $(ls po/rcr-gnome.*.po) ; do
  regex="\.([a-z][a-z])_..\.UTF-8\.po"
  if [[ $f =~ $regex ]]; then
    code="${BASH_REMATCH[1]}"
    echo -n Merge ${code} ..
    msgmerge -U $f $TEMPLATE
    echo Copying ${code} ..
    mkdir -p locale/${code}/LC_MESSAGES
    msgfmt -o locale/${code}/LC_MESSAGES/rcr-gnome.mo $f
    sudo cp locale/${code}/LC_MESSAGES/rcr-gnome.mo /usr/share/locale/${code}/LC_MESSAGES/
#   sudo cp locale/${code}/LC_MESSAGES/rcr-gnome.mo /usr/local/share/locale/ru/LC_MESSAGES
  fi
done
