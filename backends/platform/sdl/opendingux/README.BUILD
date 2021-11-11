Build instructions
==================
Running Linux on an x86/amd64 machine:

1. Download and install the desired toolchain (http://od.abstraction.se/opendingux/toolchain/) in /opt/

2. git clone the ScummVM repository

3. Run 'backends/platform/sdl/opendingux/build_odbeta.sh x'
   where x=gcw0|lepus|rg99 

   Or if you want a dual opk with one launcher capable of starting games directly 
   for e.g. simplemenu integration :
   'backends/platform/sdl/opendingux/build_odbeta.sh x dualopk'

4. Copy the resulting file scummvm_$(target).opk or scummvm_$(target)_dual.opk to your device

Game Auto-Detection (dualopk only)
==================================
1) add a blank text file 'detect.svm' alongside your individual game folders, for example:
     ---------------
    | - roms/scummvm/
    |    - detect.svm
    |    - Game Folder/
    |        - game files
     ------------------
2) load ScummVM, navigate to and select 'detect.svm'
    - the loading screen will show while the script runs
    - this may take longer if you have many games
    - .svm files will be generated for each of your games
3) load one of the .svm files to start your game directly

Troubleshooting
===============
In case you need to submit a bugreport, you may find the log file at the
following path:

     ~/.scummvm/scummvm.log

The log file is being overwritten at every ScummVM run.
