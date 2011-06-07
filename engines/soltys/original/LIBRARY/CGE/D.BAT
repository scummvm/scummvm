@echo off
echo *** Directory: %1 ***
for %%f in (%1\*.*) do call p %%f
