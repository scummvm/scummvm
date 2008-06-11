@echo off
rem This batch file is used to convert MSVC9 (Visual Studio 2008) project files to MSVC8 (Visual Studio 2005) ones
rem You need the Windows version of GNU rpl
rem Get it here:
rem http://gnuwin32.sourceforge.net/packages/rpl.htm
rem Place rpl.exe from the bin folder inside the archive in the folder where
rem this batch file resides

if not exist rpl.exe goto no_rpl

echo Creating MSVC8 project files from the MSVC9 ones
copy /y msvc9\*.vcproj msvc8\
copy /y msvc9\*.sln msvc8\
rpl -e -q "Version=\"9.00\"" "Version=\"8.00\"" msvc8\*.vcproj
rpl -e -q "Version=\"9,00\"" "Version=\"8,00\"" msvc8\*.vcproj
rpl -e -q "\tTargetFrameworkVersion=\"131072\"\n" "" msvc8\*.vcproj
rpl -e -q "\t\t\t\tRandomizedBaseAddress=\"1\"\n" "" msvc8\*.vcproj
rpl -e -q "\t\t\t\tDataExecutionPrevention=\"0\"\n" "" msvc8\*.vcproj
rpl -e -q "Format Version 10.00" "Format Version 9.00" msvc8\*.sln
rpl -e -q "Format Version 10,00" "Format Version 9,00" msvc8\*.sln
rpl -e -q "# Visual C++ Express 2008" "# Visual C++ Express 2005" msvc8\*.sln
rpl -e -q "# Visual Studio 2008" "# Visual Studio 2005" msvc8\*.sln
goto the_end

:no_rpl
echo You need the Windows version of GNU rpl
echo Get it here:
echo http://gnuwin32.sourceforge.net/packages/rpl.htm
echo Place rpl.exe from the bin folder inside the archive in the folder where
echo this batch file resides

:the_end
pause
