#!/bin/sh

if [ ! -d bin/module ]
then
    mkdir bin/module
else
    unlink bin/chatserver
    unlink bin/module/libchatserver.so
fi

cp hight-performance-server/bin/tao bin/chatserver
cp lib/libchatserver.so bin/module/
