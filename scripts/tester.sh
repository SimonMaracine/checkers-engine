#! /bin/bash

./build.sh tester

if [ "$?" -ne 0 ]; then
    exit 1
fi

cd ../build/gui/tester
./tester
