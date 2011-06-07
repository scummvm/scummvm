.AUTODEPEND

#		*Translator Definitions*
CC = bcc +CGE.CFG
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
 btfile.obj \
 vol.obj \
 text.obj \
 startup.obj \
 engine.obj \
 ..\..\jbw\lib\sound\_digi.obj \
 ..\..\jbw\lib\sound\_midi.obj \
 wav.obj \
 sound.obj \
 config.obj \
 gettext.obj \
 bitmap.obj \
 vga13h.obj \
 snail.obj \
 bitmaps.obj \
 mouse.obj \
 mousefun.obj \
 keybd.obj \
 talk.obj \
 vmenu.obj \
 game.obj \
 findway.obj \
 stdpal.obj \
 cge.obj \
 {$(LIBPATH)}general.lib

#		*Explicit Rules*
cge.exe: cge.cfg $(EXE_dependencies)
  $(TLINK) /v/s/n/c/P-/L$(LIBPATH) @&&|
c0s.obj+
cfile.obj+
btfile.obj+
vol.obj+
text.obj+
startup.obj+
engine.obj+
..\..\jbw\lib\sound\_digi.obj+
..\..\jbw\lib\sound\_midi.obj+
wav.obj+
sound.obj+
config.obj+
gettext.obj+
bitmap.obj+
vga13h.obj+
snail.obj+
bitmaps.obj+
mouse.obj+
mousefun.obj+
keybd.obj+
talk.obj+
vmenu.obj+
game.obj+
findway.obj+
stdpal.obj+
cge.obj
cge,cge
general.lib+
cs.lib
|


#		*Individual File Dependencies*
cfile.obj: cge.cfg ..\..\jbw\lib\cfile\cfile.cpp 
	$(CC) -1- -c ..\..\jbw\lib\cfile\cfile.cpp

btfile.obj: cge.cfg btfile.cpp 
	$(CC) -1- -c btfile.cpp

vol.obj: cge.cfg vol.cpp 
	$(CC) -1- -c vol.cpp

text.obj: cge.cfg text.cpp 
	$(CC) -1- -c text.cpp

startup.obj: cge.cfg startup.cpp 
	$(CC) -1- -c startup.cpp

engine.obj: cge.cfg ..\..\jbw\lib\engine\engine.cpp 
	$(CC) -c ..\..\jbw\lib\engine\engine.cpp

wav.obj: cge.cfg ..\..\jbw\lib\wav\wav.cpp 
	$(CC) -c ..\..\jbw\lib\wav\wav.cpp

sound.obj: cge.cfg sound.cpp 

config.obj: cge.cfg config.cpp 

gettext.obj: cge.cfg gettext.cpp 

bitmap.obj: cge.cfg bitmap.cpp 

vga13h.obj: cge.cfg vga13h.cpp 

snail.obj: cge.cfg snail.cpp 

bitmaps.obj: cge.cfg bitmaps.cpp 

mouse.obj: cge.cfg mouse.cpp 

mousefun.obj: cge.cfg mousefun.asm 
	$(TASM) /ML /ZI /O MOUSEFUN.ASM,MOUSEFUN.OBJ

keybd.obj: cge.cfg keybd.cpp 

talk.obj: cge.cfg talk.cpp 

vmenu.obj: cge.cfg vmenu.cpp 

game.obj: cge.cfg game.cpp 

findway.obj: cge.cfg findway.asm 
	$(TASM) /ML /ZI /O FINDWAY.ASM,FINDWAY.OBJ

stdpal.obj: cge.cfg stdpal.cpp 

cge.obj: cge.cfg cge.cpp 

#		*Compiler Configuration File*
cge.cfg: cge.mak
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
-H=CGE.SYM
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DDEBUG;VOL;INI_FILE=CFILE;PIC_FILE=VFILE;BMP_MODE=0;DROP=VGA::Exit;DROP_H
| cge.cfg


