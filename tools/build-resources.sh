#!/bin/sh
if [ -z $? ] ; then
  CMAKE_CURRENT_SOURCE_DIR="."
else
  CMAKE_CURRENT_SOURCE_DIR=$1
  cd $CMAKE_CURRENT_SOURCE_DIR
fi

glib-compile-resources $CMAKE_CURRENT_SOURCE_DIR/rcr-gnome.resource.xml --target=$CMAKE_CURRENT_SOURCE_DIR/resources.c --generate-source 

