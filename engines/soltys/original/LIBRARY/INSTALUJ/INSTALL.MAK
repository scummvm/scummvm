.AUTODEPEND

#		*Translator Definitions*
CC = bcc +INSTALL.CFG
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
 hrderr.obj \
 instaluj.obj \
 {$(LIBPATH)}mouse.lib \
 {$(LIBPATH)}wind.lib \
 {$(LIBPATH)}boot.lib \
 ..\..\jbw\lib\general.lib

#		*Explicit Rules*
install.exe: install.cfg $(EXE_dependencies)
  $(TLINK) /s/c/P-/L$(LIBPATH) @&&|
c0s.obj+
insthook.obj+
cfile.obj+
lz.obj+
hrderr.obj+
instaluj.obj
install,install
mouse.lib+
wind.lib+
boot.lib+
..\..\jbw\lib\general.lib+
cs.lib
|


#		*Individual File Dependencies*
insthook.obj: install.cfg insthook.cpp 

cfile.obj: install.cfg ..\..\jbw\lib\cfile\cfile.cpp 
	$(CC) -c ..\..\jbw\lib\cfile\cfile.cpp

lz.obj: install.cfg ..\..\jbw\lib\lzw\lz.cpp 
	$(CC) -c ..\..\jbw\lib\lzw\lz.cpp

hrderr.obj: install.cfg ..\..\jbw\lib\wind\hrderr.c 
	$(CC) -c ..\..\jbw\lib\wind\hrderr.c

instaluj.obj: install.cfg instaluj.cpp 

#		*Compiler Configuration File*
install.cfg: install.mak
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
-H=INSTALL.SYM
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DxPL
| install.cfg


