#! /bin/bash

./build.sh player

if [ "$?" -ne 0 ]; then
    exit 1
fi

cd ../build/gui/player
./player
