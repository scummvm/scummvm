.AUTODEPEND

#		*Translator Definitions*
CC = bcc +cge_eva.cfg
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
cge.exe: cge_eva.cfg $(EXE_dependencies)
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
cfile.obj: cge_eva.cfg ..\..\jbw\lib\cfile\cfile.cpp
	$(CC) -1- -c ..\..\jbw\lib\cfile\cfile.cpp

btfile.obj: cge_eva.cfg btfile.cpp
	$(CC) -1- -c btfile.cpp

vol.obj: cge_eva.cfg vol.cpp
	$(CC) -1- -c vol.cpp

text.obj: cge_eva.cfg text.cpp
	$(CC) -1- -c text.cpp

startup.obj: cge_eva.cfg startup.cpp

wav.obj: cge_eva.cfg ..\..\jbw\lib\wav\wav.cpp
	$(CC) -c ..\..\jbw\lib\wav\wav.cpp

sound.obj: cge_eva.cfg sound.cpp

config.obj: cge_eva.cfg config.cpp

gettext.obj: cge_eva.cfg gettext.cpp

bitmap.obj: cge_eva.cfg bitmap.cpp

vga13h.obj: cge_eva.cfg vga13h.cpp

snail.obj: cge_eva.cfg snail.cpp

bitmaps.obj: cge_eva.cfg bitmaps.cpp

mouse.obj: cge_eva.cfg mouse.cpp

mousefun.obj: cge_eva.cfg mousefun.asm
	$(TASM) /ML /ZI /O MOUSEFUN.ASM,MOUSEFUN.OBJ

keybd.obj: cge_eva.cfg keybd.cpp

talk.obj: cge_eva.cfg talk.cpp

mixer.obj: cge_eva.cfg mixer.cpp

vmenu.obj: cge_eva.cfg vmenu.cpp

game.obj: cge_eva.cfg game.cpp

findway.obj: cge_eva.cfg findway.asm
	$(TASM) /ML /ZI /O FINDWAY.ASM,FINDWAY.OBJ

stdpal.obj: cge_eva.cfg stdpal.cpp

cge.obj: cge_eva.cfg cge.cpp

#		*Compiler Configuration File*
cge_eva.cfg: cge_eva.mak
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
-H=CGE.SYM
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DEVA;VOL;INI_FILE=VFILE;PIC_FILE=VFILE;BMP_MODE=0;DROP=VGA::Exit;DROP_H
| cge_eva.cfg


