#!/usr/bin/env bash

printf "Creating border file...\n"

zip -r macgui.zip *.bmp
mv macgui.zip macgui.dat

echo done

ls -l macgui.dat
