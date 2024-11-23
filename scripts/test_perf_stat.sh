#! /bin/bash

perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses $1
