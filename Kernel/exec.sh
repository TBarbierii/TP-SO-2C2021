#!/bin/bash
FILE=Kernel
make $FILE
if test -f "./$FILE"; then
    ./$FILE
fi