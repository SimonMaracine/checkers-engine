#! /bin/bash

clear
./build.sh test-engine
cd ../build/engine/tests
./test-engine
