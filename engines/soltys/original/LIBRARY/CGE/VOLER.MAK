.AUTODEPEND

#		*Translator Definitions*
CC = bcc +VOLER.CFG
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
 cfile.obj \
 bt_next.obj \
 btfile.obj \
 vol.obj \
 vol_upd.obj \
 vol_next.obj \
 voler.obj \
 {$(LIBPATH)}general.lib

#		*Explicit Rules*
voler.exe: voler.cfg $(EXE_dependencies)
  $(TLINK) /s/n/c/P-/L$(LIBPATH) @&&|
c0s.obj+
cfile.obj+
bt_next.obj+
btfile.obj+
vol.obj+
vol_upd.obj+
vol_next.obj+
voler.obj
voler,voler
general.lib+
cs.lib
|


#		*Individual File Dependencies*
cfile.obj: voler.cfg ..\..\jbw\lib\cfile\cfile.cpp 
	$(CC) -c ..\..\jbw\lib\cfile\cfile.cpp

bt_next.obj: voler.cfg bt_next.cpp 

btfile.obj: voler.cfg btfile.cpp 

vol.obj: voler.cfg vol.cpp 

vol_upd.obj: voler.cfg vol_upd.cpp 

vol_next.obj: voler.cfg vol_next.cpp 

voler.obj: voler.cfg voler.cpp 

#		*Compiler Configuration File*
voler.cfg: voler.mak
  copy &&|
-2
-f-
-ff-
-K
-w+
-v-
-y
-G
-O
-Og
-Oe
-Om
-Ov
-Ol
-Ob
-Op
-Oi
-Z
-k-
-d
-H=VOLER.SYM
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DDEBUG;VOL_UPD;CRP=XCrypt;IOBUF_SIZE=K(24)
| voler.cfg


