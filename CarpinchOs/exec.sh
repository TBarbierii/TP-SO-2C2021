#!/bin/bash
FILE=carpincho
make $FILE
if test -f "./$FILE"; then
    ./$FILE
fi