/* PLEASE DO NOT MODIFY THIS FILE. ALL CHANGES WILL BE LOST! LOOK FOR README FOR DETAILS */

/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "dreamgen.h"

namespace DreamGen {

void DreamGenContext::transferMap() {
	STACK_CHECK;
	di = data.word(kExframepos);
	push(di);
	al = data.byte(kExpos);
	ah = 0;
	bx = ax;
	_add(ax, ax);
	_add(ax, bx);
	cx = 6;
	_mul(cx);
	es = data.word(kExtras);
	bx = (0);
	_add(bx, ax);
	_add(di, (0+2080));
	push(bx);
	al = data.byte(kItemtotran);
	ah = 0;
	bx = ax;
	_add(ax, ax);
	_add(ax, bx);
	cx = 6;
	_mul(cx);
	ds = data.word(kFreeframes);
	bx = (0);
	_add(bx, ax);
	si = (0+2080);
	al = ds.byte(bx);
	ah = 0;
	cl = ds.byte(bx+1);
	ch = 0;
	_add(si, ds.word(bx+2));
	dx = ds.word(bx+4);
	bx = pop();
	es.byte(bx+0) = al;
	es.byte(bx+1) = cl;
	es.word(bx+4) = dx;
	_mul(cx);
	cx = ax;
	push(cx);
	_movsb(cx, true);
	cx = pop();
	ax = pop();
	es.word(bx+2) = ax;
	_add(data.word(kExframepos), cx);
}

void DreamGenContext::rollEm() {
	STACK_CHECK;
	cl = 160;
	ch = 160;
	di = 25;
	bx = 20;
	ds = data.word(kMapstore);
	si = 0;
	multiGet();
	es = data.word(kTextfile1);
	si = 49*2;
	ax = es.word(si);
	si = ax;
	_add(si, (66*2));
	cx = 80;
endcredits21:
	push(cx);
	bx = 10;
	cx = data.word(kLinespacing);
endcredits22:
	push(cx);
	push(si);
	push(di);
	push(es);
	push(bx);
	vSync();
	cl = 160;
	ch = 160;
	di = 25;
	bx = 20;
	ds = data.word(kMapstore);
	si = 0;
	multiPut();
	vSync();
	bx = pop();
	es = pop();
	di = pop();
	si = pop();
	push(si);
	push(di);
	push(es);
	push(bx);
	cx = 18;
onelot2:
	push(cx);
	di = 25;
	dx = 161;
	ax = 0;
	printDirect();
	_add(bx, data.word(kLinespacing));
	cx = pop();
	if (--cx)
		goto onelot2;
	vSync();
	cl = 160;
	ch = 160;
	di = 25;
	bx = 20;
	multiDump();
	bx = pop();
	es = pop();
	di = pop();
	si = pop();
	cx = pop();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto endearly2;
	_dec(bx);
	if (--cx)
		goto endcredits22;
	cx = pop();
looknext2:
	al = es.byte(si);
	_inc(si);
	_cmp(al, ':');
	if (flags.z())
		goto gotnext2;
	_cmp(al, 0);
	if (flags.z())
		goto gotnext2;
	goto looknext2;
gotnext2:
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		return /* (endearly) */;
	if (--cx)
		goto endcredits21;
	cx = 120;
	hangOne();
	return;
endearly2:
	cx = pop();
}

void DreamGenContext::fillOpen() {
	STACK_CHECK;
	delTextLine();
	getOpenedSize();
	_cmp(ah, 4);
	if (flags.c())
		goto lessthanapage;
	ah = 4;
lessthanapage:
	al = 1;
	push(ax);
	es = data.word(kBuffers);
	di = (0+(228*13));
	findAllOpen();
	si = (0+(228*13));
	di = (80);
	bx = (58)+96;
	cx = pop();
openloop1:
	push(cx);
	push(di);
	push(bx);
	ax = es.word(si);
	_add(si, 2);
	push(si);
	push(es);
	_cmp(ch, cl);
	if (flags.c())
		goto nextopenslot;
	obToInv();
nextopenslot:
	es = pop();
	si = pop();
	bx = pop();
	di = pop();
	cx = pop();
	_add(di, (44));
	_inc(cl);
	_cmp(cl, 5);
	if (!flags.z())
		goto openloop1;
	underTextLine();
}

void DreamGenContext::findAllOpen() {
	STACK_CHECK;
	push(di);
	cx = 16;
	ax = 0x0ffff;
	_stosw(cx, true);
	di = pop();
	cl = data.byte(kOpenedob);
	dl = data.byte(kOpenedtype);
	ds = data.word(kExtras);
	bx = (0+2080+30000);
	ch = 0;
findopen1:
	_cmp(ds.byte(bx+3), cl);
	if (!flags.z())
		goto findopen2;
	_cmp(ds.byte(bx+2), dl);
	if (!flags.z())
		goto findopen2;
	_cmp(data.byte(kOpenedtype), 4);
	if (flags.z())
		goto noloccheck;
	al = ds.byte(bx+5);
	_cmp(al, data.byte(kReallocation));
	if (!flags.z())
		goto findopen2;
noloccheck:
	al = ds.byte(bx+4);
	ah = 0;
	push(di);
	_add(di, ax);
	_add(di, ax);
	al = ch;
	ah = 4;
	_stosw();
	di = pop();
findopen2:
	_add(bx, 16);
	_inc(ch);
	_cmp(ch, (114));
	if (!flags.z())
		goto findopen1;
	cl = data.byte(kOpenedob);
	dl = data.byte(kOpenedtype);
	push(dx);
	ds = data.word(kFreedat);
	dx = pop();
	bx = 0;
	ch = 0;
findopen1a:
	_cmp(ds.byte(bx+3), cl);
	if (!flags.z())
		goto findopen2a;
	_cmp(ds.byte(bx+2), dl);
	if (!flags.z())
		goto findopen2a;
	al = ds.byte(bx+4);
	ah = 0;
	push(di);
	_add(di, ax);
	_add(di, ax);
	al = ch;
	ah = 2;
	_stosw();
	di = pop();
findopen2a:
	_add(bx, 16);
	_inc(ch);
	_cmp(ch, 80);
	if (!flags.z())
		goto findopen1a;
}

void DreamGenContext::getFreeAd() {
	STACK_CHECK;
	ah = 0;
	cl = 4;
	_shl(ax, cl);
	bx = ax;
	es = data.word(kFreedat);
}

void DreamGenContext::getExAd() {
	STACK_CHECK;
	ah = 0;
	bx = 16;
	_mul(bx);
	bx = ax;
	es = data.word(kExtras);
	_add(bx, (0+2080+30000));
}

void DreamGenContext::getEitherAd() {
	STACK_CHECK;
	_cmp(data.byte(kObjecttype), 4);
	if (flags.z())
		goto isinexlist;
	al = data.byte(kItemframe);
	getFreeAd();
	return;
isinexlist:
	al = data.byte(kItemframe);
	getExAd();
}

void DreamGenContext::getAnyAd() {
	STACK_CHECK;
	_cmp(data.byte(kObjecttype), 4);
	if (flags.z())
		goto isex;
	_cmp(data.byte(kObjecttype), 2);
	if (flags.z())
		goto isfree;
	al = data.byte(kCommand);
	getSetAd();
	ax = es.word(bx+4);
	return;
isfree:
	al = data.byte(kCommand);
	getFreeAd();
	ax = es.word(bx+7);
	return;
isex:
	al = data.byte(kCommand);
	getExAd();
	ax = es.word(bx+7);
}

void DreamGenContext::getSetAd() {
	STACK_CHECK;
	ah = 0;
	bx = 64;
	_mul(bx);
	bx = ax;
	es = data.word(kSetdat);
}

void DreamGenContext::findOpenPos() {
	STACK_CHECK;
	cx = data.word(kMousex);
	_sub(cx, (80));
	bx = -1;
findopenp1:
	_inc(bx);
	_sub(cx, (44));
	if (!flags.c())
		goto findopenp1;
	al = bl;
	data.byte(kLastinvpos) = al;
	_add(bx, bx);
	es = data.word(kBuffers);
	_add(bx, (0+(228*13)));
}

void DreamGenContext::transferToEx() {
	STACK_CHECK;
	emergencyPurge();
	getExPos();
	al = data.byte(kExpos);
	push(ax);
	push(di);
	al = data.byte(kItemframe);
	ah = 0;
	bx = 16;
	_mul(bx);
	ds = data.word(kFreedat);
	si = ax;
	cx = 8;
	_movsw(cx, true);
	di = pop();
	al = data.byte(kReallocation);
	es.byte(di) = al;
	es.byte(di+11) = al;
	al = data.byte(kItemframe);
	es.byte(di+1) = al;
	es.byte(di+2) = 4;
	es.byte(di+3) = 255;
	al = data.byte(kLastinvpos);
	es.byte(di+4) = al;
	al = data.byte(kItemframe);
	data.byte(kItemtotran) = al;
	transferMap();
	transferInv();
	transferText();
	al = data.byte(kItemframe);
	ah = 0;
	bx = 16;
	_mul(bx);
	ds = data.word(kFreedat);
	si = ax;
	ds.byte(si+2) = 254;
	pickupConts();
	ax = pop();
}

void DreamGenContext::pickupConts() {
	STACK_CHECK;
	al = ds.byte(si+7);
	_cmp(al, 255);
	if (flags.z())
		return /* (notopenable) */;
	al = data.byte(kItemframe);
	ah = data.byte(kObjecttype);
	dl = data.byte(kExpos);
	es = data.word(kFreedat);
	bx = 0;
	cx = 0;
pickupcontloop:
	push(cx);
	push(es);
	push(bx);
	push(dx);
	push(ax);
	_cmp(es.byte(bx+2), ah);
	if (!flags.z())
		goto notinsidethis;
	_cmp(es.byte(bx+3), al);
	if (!flags.z())
		goto notinsidethis;
	data.byte(kItemtotran) = cl;
	transferConToEx();
notinsidethis:
	ax = pop();
	dx = pop();
	bx = pop();
	es = pop();
	cx = pop();
	_add(bx, 16);
	_inc(cx);
	_cmp(cx, 80);
	if (!flags.z())
		goto pickupcontloop;
}

void DreamGenContext::transferConToEx() {
	STACK_CHECK;
	push(es);
	push(bx);
	push(dx);
	push(es);
	push(bx);
	getExPos();
	si = pop();
	ds = pop();
	push(di);
	cx = 8;
	_movsw(cx, true);
	di = pop();
	dx = pop();
	al = data.byte(kReallocation);
	es.byte(di) = al;
	es.byte(di+11) = al;
	al = data.byte(kItemtotran);
	es.byte(di+1) = al;
	es.byte(di+3) = dl;
	es.byte(di+2) = 4;
	transferMap();
	transferInv();
	transferText();
	si = pop();
	ds = pop();
	ds.byte(si+2) = 255;
}

void DreamGenContext::emergencyPurge() {
	STACK_CHECK;
checkpurgeagain:
	ax = data.word(kExframepos);
	_add(ax, 4000);
	_cmp(ax, (30000));
	if (flags.c())
		goto notnearframeend;
	purgeAnItem();
	goto checkpurgeagain;
notnearframeend:
	ax = data.word(kExtextpos);
	_add(ax, 400);
	_cmp(ax, (18000));
	if (flags.c())
		return /* (notneartextend) */;
	purgeAnItem();
	goto checkpurgeagain;
}

void DreamGenContext::purgeAnItem() {
	STACK_CHECK;
	es = data.word(kExtras);
	di = (0+2080+30000);
	bl = data.byte(kReallocation);
	cx = 0;
lookforpurge:
	al = es.byte(di+2);
	_cmp(al, 0);
	if (!flags.z())
		goto cantpurge;
	_cmp(es.byte(di+12), 2);
	if (flags.z())
		goto iscup;
	_cmp(es.byte(di+12), 255);
	if (!flags.z())
		goto cantpurge;
iscup:
	_cmp(es.byte(di+11), bl);
	if (flags.z())
		goto cantpurge;
	deleteExObject();
	return;
cantpurge:
	_add(di, 16);
	_inc(cx);
	_cmp(cx, (114));
	if (!flags.z())
		goto lookforpurge;
	di = (0+2080+30000);
	bl = data.byte(kReallocation);
	cx = 0;
lookforpurge2:
	al = es.byte(di+2);
	_cmp(al, 0);
	if (!flags.z())
		goto cantpurge2;
	_cmp(es.byte(di+12), 255);
	if (!flags.z())
		goto cantpurge2;
	deleteExObject();
	return;
cantpurge2:
	_add(di, 16);
	_inc(cx);
	_cmp(cx, (114));
	if (!flags.z())
		goto lookforpurge2;
}

void DreamGenContext::getDestInfo() {
	STACK_CHECK;
	al = data.byte(kDestpos);
	ah = 0;
	push(ax);
	dx = data;
	es = dx;
	si = 537;
	_add(si, ax);
	cl = es.byte(si);
	ax = pop();
	push(cx);
	dx = data;
	es = dx;
	si = 553;
	_add(si, ax);
	ax = pop();
}

void DreamGenContext::dirCom() {
	STACK_CHECK;
	cx = 30;
	randomAccess();
	parser();
	_cmp(es.byte(di+1), 0);
	if (flags.z())
		goto dirroot;
	dirFile();
	return;
dirroot:
	data.byte(kLogonum) = 0;
	ds = cs;
	si = offset_rootdir;
	_inc(si);
	es = cs;
	di = 462;
	_inc(di);
	cx = 12;
	_movsb(cx, true);
	monitorLogo();
	scrollMonitor();
	al = 9;
	monMessage();
	es = data.word(kTextfile1);
	searchForFiles();
	es = data.word(kTextfile2);
	searchForFiles();
	es = data.word(kTextfile3);
	searchForFiles();
	scrollMonitor();
}

void DreamGenContext::searchForFiles() {
	STACK_CHECK;
	bx = (66*2);
directloop1:
	al = es.byte(bx);
	_inc(bx);
	_cmp(al, '*');
	if (flags.z())
		return /* (endofdir) */;
	_cmp(al, 34);
	if (!flags.z())
		goto directloop1;
	monPrint();
	goto directloop1;
}

void DreamGenContext::read() {
	STACK_CHECK;
	cx = 40;
	randomAccess();
	parser();
	_cmp(es.byte(di+1), 0);
	if (!flags.z())
		goto okcom;
	netError();
	return;
okcom:
	es = cs;
	di = 462;
	ax = data.word(kTextfile1);
	data.word(kMonsource) = ax;
	ds = ax;
	si = (66*2);
	searchForString();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile2;
	ax = data.word(kTextfile2);
	data.word(kMonsource) = ax;
	ds = ax;
	si = (66*2);
	searchForString();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile2;
	ax = data.word(kTextfile3);
	data.word(kMonsource) = ax;
	ds = ax;
	si = (66*2);
	searchForString();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile2;
	al = 7;
	monMessage();
	return;
foundfile2:
	getKeyAndLogo();
	_cmp(al, 0);
	if (flags.z())
		goto keyok1;
	return;
keyok1:
	es = cs;
	di = offset_operand1;
	ds = data.word(kMonsource);
	searchForString();
	_cmp(al, 0);
	if (flags.z())
		goto findtopictext;
	al = data.byte(kOldlogonum);
	data.byte(kLogonum) = al;
	al = 11;
	monMessage();
	return;
findtopictext:
	_inc(bx);
	push(es);
	push(bx);
	monitorLogo();
	scrollMonitor();
	bx = pop();
	es = pop();
moretopic:
	monPrint();
	al = es.byte(bx);
	_cmp(al, 34);
	if (flags.z())
		goto endoftopic;
	_cmp(al, '=');
	if (flags.z())
		goto endoftopic;
	_cmp(al, '*');
	if (flags.z())
		goto endoftopic;
	push(es);
	push(bx);
	processTrigger();
	cx = 24;
	randomAccess();
	bx = pop();
	es = pop();
	goto moretopic;
endoftopic:
	scrollMonitor();
}

void DreamGenContext::dirFile() {
	STACK_CHECK;
	al = 34;
	es.byte(di) = al;
	push(es);
	push(di);
	ds = data.word(kTextfile1);
	si = (66*2);
	searchForString();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile;
	di = pop();
	es = pop();
	push(es);
	push(di);
	ds = data.word(kTextfile2);
	si = (66*2);
	searchForString();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile;
	di = pop();
	es = pop();
	push(es);
	push(di);
	ds = data.word(kTextfile3);
	si = (66*2);
	searchForString();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile;
	di = pop();
	es = pop();
	al = 7;
	monMessage();
	return;
foundfile:
	ax = pop();
	ax = pop();
	getKeyAndLogo();
	_cmp(al, 0);
	if (flags.z())
		goto keyok2;
	return;
keyok2:
	push(es);
	push(bx);
	ds = cs;
	si = offset_operand1+1;
	es = cs;
	di = 462+1;
	cx = 12;
	_movsb(cx, true);
	monitorLogo();
	scrollMonitor();
	al = 10;
	monMessage();
	bx = pop();
	es = pop();
directloop2:
	al = es.byte(bx);
	_inc(bx);
	_cmp(al, 34);
	if (flags.z())
		goto endofdir2;
	_cmp(al, '*');
	if (flags.z())
		goto endofdir2;
	_cmp(al, '=');
	if (!flags.z())
		goto directloop2;
	monPrint();
	goto directloop2;
endofdir2:
	scrollMonitor();
}

void DreamGenContext::searchForString() {
	STACK_CHECK;
	dl = es.byte(di);
	cx = di;
restartlook:
	di = cx;
	bx = si;
	dh = 0;
keeplooking:
	_lodsb();
	makeCaps();
	_cmp(al, '*');
	if (flags.z())
		goto notfound;
	_cmp(dl, '=');
	if (!flags.z())
		goto nofindingtopic;
	_cmp(al, 34);
	if (flags.z())
		goto notfound;
nofindingtopic:
	ah = es.byte(di);
	_cmp(al, dl);
	if (!flags.z())
		goto notbracket;
	_inc(dh);
	_cmp(dh, 2);
	if (flags.z())
		goto complete;
notbracket:
	_cmp(al, ah);
	if (!flags.z())
		goto restartlook;
	_inc(di);
	goto keeplooking;
complete:
	es = ds;
	al = 0;
	bx = si;
	return;
notfound:
	al = 1;
}

void DreamGenContext::parser() {
	STACK_CHECK;
	es = cs;
	di = offset_operand1;
	cx = 13;
	al = 0;
	_stosb(cx, true);
	di = offset_operand1;
	al = '=';
	_stosb();
	ds = cs;
	si = 571;
notspace1:
	_lodsw();
	_cmp(al, 32);
	if (flags.z())
		goto stillspace1;
	_cmp(al, 0);
	if (!flags.z())
		goto notspace1;
	goto finishpars;
stillspace1:
	_lodsw();
	_cmp(al, 32);
	if (flags.z())
		goto stillspace1;
copyin1:
	_stosb();
	_lodsw();
	_cmp(al, 0);
	if (flags.z())
		goto finishpars;
	_cmp(al, 32);
	if (!flags.z())
		goto copyin1;
finishpars:
	di = offset_operand1;
}

void DreamGenContext::findPathOfPoint() {
	STACK_CHECK;
	push(ax);
	bx = (0);
	es = data.word(kReels);
	al = data.byte(kRoomnum);
	ah = 0;
	cx = 144;
	_mul(cx);
	_add(bx, ax);
	cx = pop();
	dl = 0;
pathloop:
	al = es.byte(bx+6);
	_cmp(al, 255);
	if (!flags.z())
		goto flunkedit;
	ax = es.word(bx+2);
	_cmp(ax, 0x0ffff);
	if (flags.z())
		goto flunkedit;
	_cmp(cl, al);
	if (flags.c())
		goto flunkedit;
	_cmp(ch, ah);
	if (flags.c())
		goto flunkedit;
	ax = es.word(bx+4);
	_cmp(cl, al);
	if (!flags.c())
		goto flunkedit;
	_cmp(ch, ah);
	if (!flags.c())
		goto flunkedit;
	return /* (gotvalidpath) */;
flunkedit:
	_add(bx, 8);
	_inc(dl);
	_cmp(dl, 12);
	if (!flags.z())
		goto pathloop;
	dl = 255;
}

void DreamGenContext::findFirstPath() {
	STACK_CHECK;
	push(ax);
	bx = (0);
	es = data.word(kReels);
	al = data.byte(kRoomnum);
	ah = 0;
	cx = 144;
	_mul(cx);
	_add(bx, ax);
	cx = pop();
	dl = 0;
fpathloop:
	ax = es.word(bx+2);
	_cmp(ax, 0x0ffff);
	if (flags.z())
		goto nofirst;
	_cmp(cl, al);
	if (flags.c())
		goto nofirst;
	_cmp(ch, ah);
	if (flags.c())
		goto nofirst;
	ax = es.word(bx+4);
	_cmp(cl, al);
	if (!flags.c())
		goto nofirst;
	_cmp(ch, ah);
	if (!flags.c())
		goto nofirst;
	goto gotfirst;
nofirst:
	_add(bx, 8);
	_inc(dl);
	_cmp(dl, 12);
	if (!flags.z())
		goto fpathloop;
	al = 0;
	return;
gotfirst:
	al = es.byte(bx+6);
}

void DreamGenContext::__start() { 
	static const uint8 src[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x13, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0000: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
		//0x0010: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0020: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0xff, 0x00, 
		//0x0030: .... .... .... ....
		0xff, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x0d, 0x00, 0xb6, 
		//0x0040: ...0 .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0050: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0060: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0070: .... ...h .&.. ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0080: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0090: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00a0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00b0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 
		//0x00c0: .... .... ...   ...
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00d0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00e0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00f0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0100: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0110: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0120: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0130: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0140: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		//0x0150: .... .... .... ....
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		//0x0160: .... .... .... ....
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		//0x0170: .... .... .... ....
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0180: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 
		//0x0190: .... .... .... ....
		0x00, 0x00, 0x00, 0xff, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x56, 0x39, 
		//0x01a0: .... .DRE AMWE B.V9
		0x39, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 
		//0x01b0: 9.                .
		0x22, 0x52, 0x4f, 0x4f, 0x54, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x22, 0x20, 
		//0x01c0: "ROO T          ." 
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x0d, 0x0a, 0x0d, 0x0a, 
		//0x01d0:              . ....
		0x24, 0x10, 0x12, 0x12, 0x11, 0x10, 0x10, 0x10, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		//0x01e0: $... .... .... ....
		0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x44, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x01f0: .... .... .D:. ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0200: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
		//0x0210: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x02, 0x04, 0x01, 0x0a, 
		//0x0220: .... .... .... ....
		0x09, 0x08, 0x06, 0x0b, 0x04, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0230: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0240: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0250: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0260: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0270: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0280: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0290: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x02a0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
		//0x02b0: .... .... .... ....
		0xff, 0x00, 0x00, 0x00, };
	ds.assign(src, src + sizeof(src));
	dreamweb(); 
}

} // End of namespace DreamGen
