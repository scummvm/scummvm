@echo off
smartdrv c+
for %%f in (*.spr) do find /i "NAME=" %%f>>nazwy.sou
smartdrv c
