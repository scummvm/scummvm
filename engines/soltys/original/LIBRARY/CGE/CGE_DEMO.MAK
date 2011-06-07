.AUTODEPEND

#		*Translator Definitions*
CC = bcc +cge_usr.cfg
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
 ..\..\jbw\lib\avalon\_snddrv.obj \
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
 mixer.obj \
 vmenu.obj \
 game.obj \
 findway.obj \
 stdpal.obj \
 cge.obj \
 {$(LIBPATH)}general.lib \
 {$(LIBPATH)}boot.lib

#		*Explicit Rules*
cge.exe: cge_usr.cfg $(EXE_dependencies)
  $(TLINK) /s/n/c/P-/L$(LIBPATH) @&&|
c0s.obj+
cfile.obj+
btfile.obj+
vol.obj+
text.obj+
startup.obj+
..\..\jbw\lib\avalon\_snddrv.obj+
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
mixer.obj+
vmenu.obj+
game.obj+
findway.obj+
stdpal.obj+
cge.obj
cge,cge
general.lib+
boot.lib+
cs.lib
|


#		*Individual File Dependencies*
cfile.obj: cge_usr.cfg ..\..\jbw\lib\cfile\cfile.cpp
	$(CC) -1- -c ..\..\jbw\lib\cfile\cfile.cpp

btfile.obj: cge_usr.cfg btfile.cpp
	$(CC) -1- -c btfile.cpp

vol.obj: cge_usr.cfg vol.cpp
	$(CC) -1- -c vol.cpp

text.obj: cge_usr.cfg text.cpp
	$(CC) -1- -c text.cpp

startup.obj: cge_usr.cfg startup.cpp

wav.obj: cge_usr.cfg ..\..\jbw\lib\wav\wav.cpp
	$(CC) -c ..\..\jbw\lib\wav\wav.cpp

sound.obj: cge_usr.cfg sound.cpp

config.obj: cge_usr.cfg config.cpp

gettext.obj: cge_usr.cfg gettext.cpp

bitmap.obj: cge_usr.cfg bitmap.cpp

vga13h.obj: cge_usr.cfg vga13h.cpp

snail.obj: cge_usr.cfg snail.cpp

bitmaps.obj: cge_usr.cfg bitmaps.cpp

mouse.obj: cge_usr.cfg mouse.cpp

mousefun.obj: cge_usr.cfg mousefun.asm
	$(TASM) /ML /ZI /O MOUSEFUN.ASM,MOUSEFUN.OBJ

keybd.obj: cge_usr.cfg keybd.cpp

talk.obj: cge_usr.cfg talk.cpp

mixer.obj: cge_usr.cfg mixer.cpp

vmenu.obj: cge_usr.cfg vmenu.cpp

game.obj: cge_usr.cfg game.cpp

findway.obj: cge_usr.cfg findway.asm
	$(TASM) /ML /ZI /O FINDWAY.ASM,FINDWAY.OBJ

stdpal.obj: cge_usr.cfg stdpal.cpp

cge.obj: cge_usr.cfg cge.cpp

#		*Compiler Configuration File*
cge_usr.cfg: cge_usr.mak
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
-H=CGE.SYM
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DDEMO;VOL;INI_FILE=VFILE;PIC_FILE=VFILE;BMP_MODE=0;DROP=VGA::Exit;DROP_H
| cge_usr.cfg


