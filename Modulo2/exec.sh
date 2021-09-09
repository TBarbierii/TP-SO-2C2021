#!/bin/bash
FILE=Modulo2
make $FILE
if test -f "./$FILE"; then
    ./$FILE
fi