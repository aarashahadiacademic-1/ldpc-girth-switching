#!/usr/bin/env bash
set -euo pipefail
CXX="${CXX:-g++}"
$CXX -O3 -std=c++17 -DNDEBUG ldpc_girth_switching.cpp -o ldpc_girth_switching
for args in \
  "--n 1944 --dv 3 --dc 6 --seed 1" \
  "--n 16200 --dv 3 --dc 6 --seed 1" \
  "--n 64800 --dv 3 --dc 6 --seed 1" \
  "--n 8448 --dv 4 --dc 8 --seed 1" \
  "--n 64800 --dv 4 --dc 8 --seed 1" \
  "--n 648 --dv 5 --dc 10 --seed 1"
do
  ./ldpc_girth_switching $args
done
