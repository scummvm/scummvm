#!/usr/bin/env bash

printf "Creating MacGUI file...\n"

zip -r macgui.zip VERSION *.bmp qtvr
mv macgui.zip macgui.dat

echo done

ls -l macgui.dat
