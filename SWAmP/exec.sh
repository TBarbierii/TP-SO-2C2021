#!/bin/bash
FILE=SWAmP
make $FILE
if test -f "./$FILE"; then
    ./$FILE
fi