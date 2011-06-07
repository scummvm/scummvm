.AUTODEPEND

#		*Translator Definitions*
CC = bcc +SPLIT.CFG
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
 split.obj \
 cfile.obj \
 {$(LIBPATH)}general.lib

#		*Explicit Rules*
split.exe: split.cfg $(EXE_dependencies)
  $(TLINK) /s/n/c/P-/L$(LIBPATH) @&&|
c0s.obj+
split.obj+
cfile.obj
split,split
general.lib+
cs.lib
|


#		*Individual File Dependencies*
split.obj: split.cfg split.cpp 

cfile.obj: split.cfg ..\..\jbw\lib\cfile\cfile.cpp 
	$(CC) -c ..\..\jbw\lib\cfile\cfile.cpp

#		*Compiler Configuration File*
split.cfg: split.mak
  copy &&|
-2
-f-
-ff-
-K
-w+
-v
-y
-O
-Oe
-Ob
-Z
-k-
-d
-vi-
-H=SPLIT.SYM
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DIOBUF_SIZ=K(8)
| split.cfg


