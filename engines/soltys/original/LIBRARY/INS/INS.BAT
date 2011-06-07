@echo off
smartdrv a+>nul
rem ------------------------------------
echo preparing SOLTYS disk #1...
format a: /u/f:1440/v:"SOLTYS #1"
putboot a:>nul
echo copying files...
copy pl\instaluj.exe a:>nul
copy instaluj.inf a:>nul
copy instaluj.chn a:>nul
copy czytaj.to a:>nul
copy pl\cge.exe a:soltys.001
pak vol.cat a:soltys.002
pak vol.#01 a:soltys.003
rem ------------------------------------
echo preparing SOLTYS disk #2...
format a: /u/f:1440/v:"SOLTYS #2"
putboot a:>nul
echo copying files...
pak vol.#02 a:soltys.004
rem ------------------------------------
echo preparing SOLTYS disk #3...
format a: /u/f:1440/v:"SOLTYS #3"
putboot a:>nul
echo copying files...
pak vol.#03 a:soltys.005
rem ------------------------------------
smartdrv b>nul
echo Finished.
