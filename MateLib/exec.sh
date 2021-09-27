#!/bin/bash
FILE=Matelib
make $FILE
if test -f "./$FILE"; then
    ./$FILE
fi