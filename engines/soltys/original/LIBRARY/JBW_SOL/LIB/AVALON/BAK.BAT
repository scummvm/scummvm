@echo off
verify on
ifold bak.ok *.* arj u %BAK%\avalon -jt
rem >bak.ok
verify off
