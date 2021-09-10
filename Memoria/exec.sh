#!/bin/bash
FILE=Memoria
make $FILE
if test -f "./$FILE"; then
    ./$FILE
fi