@echo off
rem This batch file is used to convert MSVC8 (Visual Studio 2005) project files to MSVC9 (Visual Studio 2008) ones
rem You need the Windows version of GNU rpl
rem Get it here:
rem http://gnuwin32.sourceforge.net/packages/rpl.htm
rem Place rpl.exe from the bin folder inside the archive in the folder where
rem this batch file resides

if not exist rpl.exe goto no_rpl

echo Creating MSVC9 project files from the MSVC8 ones
copy /y msvc8\*.vcproj msvc9\
copy /y msvc8\*.sln msvc9\
rpl -e -q "Version=\"8.00\"" "Version=\"9.00\"" msvc9\*.vcproj
rpl -e -q "Version=\"8,00\"" "Version=\"9,00\"" msvc9\*.vcproj
rpl -e -q "Keyword=\"Win32Proj\"" "Keyword=\"Win32Proj\"\n\tTargetFrameworkVersion=\"131072\"" msvc9\*.vcproj
rpl -e -q "EntryPointSymbol=\"WinMainCRTStartup\"" "EntryPointSymbol=\"WinMainCRTStartup\"\n\t\t\t\tRandomizedBaseAddress=\"1\"\n\t\t\t\tDataExecutionPrevention=\"0\"" msvc9\*.vcproj
rpl -e -q "Format Version 9.00" "Format Version 10.00" msvc9\*.sln
rpl -e -q "Format Version 9,00" "Format Version 10,00" msvc9\*.sln
rpl -e -q "# Visual C++ Express 2005" "# Visual C++ Express 2008" msvc9\*.sln
rpl -e -q "# Visual Studio 2005" "# Visual Studio 2008" msvc9\*.sln
goto the_end

:no_rpl
echo You need the Windows version of GNU rpl
echo Get it here:
echo http://gnuwin32.sourceforge.net/packages/rpl.htm
echo Place rpl.exe from the bin folder inside the archive in the folder where
echo this batch file resides

:the_end
pause
