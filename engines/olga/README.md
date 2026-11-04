# Olga engine

A ScummVM engine for *Olga's Arbejdsmiljø* (DOS, 1993, INFA, Danmarks
Lærerhøjskole), a Danish educational point-and-click game about the working
environment at school and at home. Work in progress: so far it detects the
game, shows the two title pictures and the start room.

This engine lives in a personal fork. It was written by an AI agent (Claude)
from reverse-engineering notes, not from decompiled code. Under ScummVM's
`AI-GUIDELINES.md` it can't be submitted to the ScummVM project.

## You need the original game

No game data is included. The engine reads the pictures and room files from
your copy at run time. Put the files of the DOS version together in one
directory: `OLGA.EXE` (127,488 bytes) and `OLGA.OVR` (27,052 bytes), both
detected by MD5, plus all the `*.LBM`, `*.SCN` and `*.PRS` files.

## Building on Windows (MSYS2)

In an **MSYS2 UCRT64** shell, in the ScummVM directory:
```
./configure --disable-all-engines --enable-engine=olga --disable-detection-full
make -j$(nproc)
strip scummvm.exe
./scummvm.exe --path=/c/path/to/game olga
```
Start it from the same UCRT64 shell so that it finds the SDL2 DLLs.

## Debugger

Ctrl+Alt+D opens the debugger. `room <name>` shows another room picture,
e.g. `room klassen`.
