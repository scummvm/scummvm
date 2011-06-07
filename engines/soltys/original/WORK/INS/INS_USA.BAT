@echo off
smartdrv b+>nul
rem ------------------------------------
echo preparing SOLTYS disk #1...
format a: /u/f:1440/v:"SOLTYS #1"
putboot a:>nul
echo copying files...
copy usa\install.exe a:>nul
copy install.inf a:>nul
copy install.chn a:>nul
copy readme.1st a:>nul
copy usa\cge.exe a:soltys.001
lzw vol.cat a:soltys.002
lzw vol.#01 a:soltys.003
rem ------------------------------------
echo preparing SOLTYS disk #2...
format a: /u/f:1440/v:"SOLTYS #2"
putboot a:>nul
echo copying files...
lzw vol.#02 a:soltys.004
rem ------------------------------------
echo preparing SOLTYS disk #3...
format a: /u/f:1440/v:"SOLTYS #3"
putboot a:>nul
echo copying files...
lzw vol.#03 a:soltys.005
rem ------------------------------------
echo preparing SOLTYS disk #4...
format a: /u/f:1440/v:"SOLTYS #4"
putboot a:>nul
echo copying files...
lzw vol.#04 a:soltys.006
rem ------------------------------------
echo preparing SOLTYS disk #5...
format a: /u/f:1440/v:"SOLTYS #5"
putboot a:>nul
echo copying files...
lzw vol.#05 a:soltys.007
rem ------------------------------------
smartdrv b>nul
echo Finished.
