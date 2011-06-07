.AUTODEPEND

#		*Translator Definitions*
CC = bcc +INSTALUJ.CFG
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
 insthook.obj \
 cfile.obj \
 lz.obj \
 instaluj.obj \
 hrderr_p.obj \
 {$(LIBPATH)}mouse.lib \
 {$(LIBPATH)}wind.lib \
 {$(LIBPATH)}boot.lib \
 ..\..\jbw\lib\general.lib

#		*Explicit Rules*
instaluj.exe: instaluj.cfg $(EXE_dependencies)
  $(TLINK) /s/c/P-/L$(LIBPATH) @&&|
c0s.obj+
insthook.obj+
cfile.obj+
lz.obj+
instaluj.obj+
hrderr_p.obj
instaluj,instaluj
mouse.lib+
wind.lib+
boot.lib+
..\..\jbw\lib\general.lib+
cs.lib
|


#		*Individual File Dependencies*
insthook.obj: instaluj.cfg insthook.cpp 

cfile.obj: instaluj.cfg ..\..\jbw\lib\cfile\cfile.cpp 
	$(CC) -c ..\..\jbw\lib\cfile\cfile.cpp

lz.obj: instaluj.cfg ..\..\jbw\lib\lzw\lz.cpp 
	$(CC) -c ..\..\jbw\lib\lzw\lz.cpp

instaluj.obj: instaluj.cfg instaluj.cpp 

hrderr_p.obj: instaluj.cfg ..\..\jbw\lib\wind\hrderr_p.c 
	$(CC) -c ..\..\jbw\lib\wind\hrderr_p.c

#		*Compiler Configuration File*
instaluj.cfg: instaluj.mak
  copy &&|
-f-
-ff-
-K
-w+
-v-
-y
-O
-Oe
-Ob
-Z
-k-
-d
-vi-
-H=INSTALUJ.SYM
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DPL
| instaluj.cfg


