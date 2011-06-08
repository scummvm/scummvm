@echo off
smartdrv c+>nul
rem>vol.cat
rem>vol.dat
if exist cge.svg erase cge.svg
if exist files\cge.svg erase files\cge.svg >nul
ds NE files
pack files\*.*
cge
copy cge.svg files
erase cge.svg >nul
rem>vol.cat
rem>vol.dat
ds NE files
pack files\*.*
smartdrv c>nul
