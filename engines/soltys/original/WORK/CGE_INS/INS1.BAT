@echo off
smartdrv b+>nul
rem ------------------------------------
rem --- goto f1
echo insert SOLTYS disk #1 into drive B:
pause
echo preparing diskette...
goto b1
:f1
echo preparing SOLTYS disk #1...
format b: /q/u
:b1
putboot b:>nul
echo copying files...
copy c:\work\lm\instaluj.exe b:>nul
copy instaluj.inf b:>nul
copy instaluj.chn b:>nul
copy czytaj.to b:>nul
copy cge.exe b:soltys.001
lzw vol.cat b:soltys.002
lzw vol.#01 b:soltys.003
rem ------------------------------------
echo insert SOLTYS disk #2 into drive B:
pause
echo preparing diskette...
putboot b:>nul
echo y | erase b:*.* >nul
echo copying files...
lzw vol.#02 b:soltys.004
rem ------------------------------------
echo insert SOLTYS disk #3 into drive B:
pause
echo preparing diskette...
putboot b:>nul
echo y | erase b:*.* >nul
echo copying files...
lzw vol.#03 b:soltys.005
rem ------------------------------------
echo insert SOLTYS disk #4 into drive B:
pause
echo preparing diskette...
putboot b:>nul
echo y | erase b:*.* >nul
echo copying files...
lzw vol.#04 b:soltys.006
rem ------------------------------------
echo insert SOLTYS disk #5 into drive B:
pause
echo preparing diskette...
putboot b:>nul
echo y | erase b:*.* >nul
echo copying files...
lzw vol.#05 b:soltys.007
rem ------------------------------------
smartdrv b>nul
echo Finished.
