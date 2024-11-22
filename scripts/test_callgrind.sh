#! /bin/bash

# TODO see if this is useful
valgrind --tool=callgrind --simulate-cache=yes --dump-instr=yes $1
