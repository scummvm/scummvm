#!/usr/bin/env bash

printf "Creating border/sounds file...\n"

zip -r freescape.zip *.bmp *.wav
mv freescape.zip freescape.dat

echo done

ls -l freescape.dat
