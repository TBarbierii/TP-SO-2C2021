#!/bin/bash
FILE=Modulo3
make $FILE
if test -f "./$FILE"; then
    ./$FILE
fi