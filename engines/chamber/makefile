CC = tcc -mh -c -v -w
LINK = tlink /c /x /v
LIB = tlib
OBJS = resdata.obj cga.obj anim.obj decompr.obj cursor.obj input.obj \
	 room.obj invent.obj print.obj dialog.obj portrait.obj menu.obj \
	 timer.obj sound.obj savegame.obj kult.obj script.obj

kult.exe: alldata.lib $(OBJS)
	$(LINK) $(CLIB)\c0h @kult.lnk,$*,,alldata $(CLIB)\ch

alldata.lib: r_puzzl.obj r_sprit.obj r_templ.obj r_texts.obj bkbuff.obj r_pers.obj
	$(LIB) alldata.lib /c +-r_puzzl +-r_sprit +-bkbuff +-r_texts +-r_pers +-r_templ

r_puzzl.obj: r_puzzl.c resdata.h
r_sprit.obj: r_sprit.c resdata.h
r_texts.obj: r_texts.c resdata.h
r_pers.obj: r_pers.c resdata.h
r_templ.obj: r_templ.c resdata.h
bkbuff.obj: bkbuff.c

kult.obj: kult.c common.h cga.h decompr.h script.h resdata.h
resdata.obj: resdata.c common.h resdata.h
cga.obj: cga.c common.h cga.h
anim.obj: anim.c common.h anim.h
decompr.obj: decompr.c common.h
cursor.obj: cursor.c common.h cursor.h cga.h resdata.h
input.obj: input.c common.h input.h
print.obj: print.c common.h print.h cga.h
dialog.obj: dialog.c common.h dialog.h cga.h
portrait.obj: portrait.c common.h portrait.h cga.h
room.obj: room.c common.h room.h cga.h
invent.obj: invent.c common.h invent.h cga.h script.h
menu.obj: menu.c common.h cga.h script.h input.h
sound.obj: sound.c common.h sound.h
timer.obj: timer.c common.h timer.h cga.h
script.obj: script.c common.h script.h cga.h resdata.h
savegame.obj: savegame.c common.h savegame.h script.h

.c.obj:
	$(CC) $<

clean:
	-del *.exe
	-del *.obj
	-del *.lib
