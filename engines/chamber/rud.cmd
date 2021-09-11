@echo off
copy /y KULT.EXE ..\..\..\dbx\disk\kure
pushd ..\..\..\dbx
dosbox -c "cd kure" -c "td kult.exe"
popd
