#!/bin/bash
FILE=MateLib
make $FILE
if test -f "./$FILE"; then
    ./$FILE
fi