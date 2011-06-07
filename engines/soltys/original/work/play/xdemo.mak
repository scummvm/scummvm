.AUTODEPEND

#		*Translator Definitions*
CC = bcc +XDEMO.CFG
TASM = TASM
TLIB = tlib
TLINK = tlink
LIBPATH = D:\BORLANDC\LIB;C:\JBW\LIB
INCLUDEPATH = D:\BORLANDC\INCLUDE;C:\JBW\INC


#		*Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#		*List Macros*


EXE_dependencies =  \
 xdemo.obj \
 ..\..\..\jbw\lib\avalon\_snddrv.obj \
 ..\..\..\jbw\lib\general.lib

#		*Explicit Rules*
xdemo.exe: xdemo.cfg $(EXE_dependencies)
  $(TLINK) /s/n/c/P-/L$(LIBPATH) @&&|
c0s.obj+
xdemo.obj+
..\..\..\jbw\lib\avalon\_snddrv.obj
xdemo,xdemo
..\..\..\jbw\lib\general.lib+
cs.lib
|


#		*Individual File Dependencies*
xdemo.obj: xdemo.cfg xdemo.cpp 

#		*Compiler Configuration File*
xdemo.cfg: xdemo.mak
  copy &&|
-2
-f-
-ff-
-K
-w+
-v
-y
-d
-vi-
-H=XDEMO.SYM
-I$(INCLUDEPATH)
-L$(LIBPATH)
| xdemo.cfg


