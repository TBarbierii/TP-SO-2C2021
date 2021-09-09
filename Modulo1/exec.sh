#!/bin/bash
FILE=Modulo1
make $FILE
if test -f "./$FILE"; then
    ./$FILE
fi