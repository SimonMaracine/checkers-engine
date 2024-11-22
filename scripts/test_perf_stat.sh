#! /bin/bash

perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-load-misses $1
