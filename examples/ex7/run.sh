#!/bin/bash
ORIGDIR=$(pwd)
cd ../../build/examples
mpirun -np $1 ./ex7.exe
cd "${ORIGDIR}"
