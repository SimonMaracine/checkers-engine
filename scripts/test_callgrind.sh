#! /bin/bash

valgrind --tool=callgrind --simulate-cache=yes --dump-instr=yes $1
