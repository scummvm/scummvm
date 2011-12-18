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

void DreamGenContext::reminders() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 24);
	if (!flags.z())
		return /* (notinedenslift) */;
	_cmp(data.byte(kMapx), 44);
	if (!flags.z())
		return /* (notinedenslift) */;
	_cmp(data.byte(kProgresspoints), 0);
	if (!flags.z())
		return /* (notfirst) */;
	al = 'D';
	ah = 'K';
	cl = 'E';
	ch = 'Y';
	isRyanHolding();
	if (flags.z())
		goto forgotone;
	al = 'C';
	ah = 'S';
	cl = 'H';
	ch = 'R';
	findExObject();
	_cmp(al, (114));
	if (flags.z())
		goto forgotone;
	ax = es.word(bx+2);
	_cmp(al, 4);
	if (!flags.z())
		goto forgotone;
	_cmp(ah, 255);
	if (flags.z())
		goto havegotcard;
	cl = 'P';
	ch = 'U';
	dl = 'R';
	dh = 'S';
	_xchg(al, ah);
	compare();
	if (!flags.z())
		goto forgotone;
havegotcard:
	_inc(data.byte(kProgresspoints));
	return;
forgotone:
	al = 50;
	bl = 54;
	bh = 70;
	cx = 48;
	dx = 8;
	setupTimedUse();
}

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

void DreamGenContext::fadeDownMon() {
	STACK_CHECK;
	palToStartPal();
	palToEndPal();
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768)+(231*3);
	cx = 3*8;
	ax = 0;
	_stosb(cx, true);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768)+(246*3);
	_stosb();
	_stosw();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	cx = 64;
	hangOn();
}

void DreamGenContext::fadeUpMon() {
	STACK_CHECK;
	palToStartPal();
	palToEndPal();
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3))+(231*3);
	cx = 3*8;
	ax = 0;
	_stosb(cx, true);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3))+(246*3);
	_stosb();
	_stosw();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	cx = 128;
	hangOn();
}

void DreamGenContext::initialMonCols() {
	STACK_CHECK;
	palToStartPal();
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3))+(230*3);
	cx = 3*9;
	ax = 0;
	_stosb(cx, true);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3))+(246*3);
	_stosb();
	_stosw();
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3))+(230*3);
	al = 230;
	cx = 18;
	showGroup();
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

void DreamGenContext::incRyanPage() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 222);
	if (flags.z())
		goto alreadyincryan;
	data.byte(kCommandtype) = 222;
	al = 31;
	commandOnly();
alreadyincryan:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (noincryan) */;
	_and(ax, 1);
	if (!flags.z())
		goto doincryan;
	return;
doincryan:
	ax = data.word(kMousex);
	_sub(ax, (80)+167);
	data.byte(kRyanpage) = -1;
findnewpage:
	_inc(data.byte(kRyanpage));
	_sub(ax, 18);
	if (!flags.c())
		goto findnewpage;
	delPointer();
	fillRyan();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::getObTextStart() {
	STACK_CHECK;
	es = data.word(kFreedesc);
	si = (0);
	cx = (0+(82*2));
	_cmp(data.byte(kObjecttype), 2);
	if (flags.z())
		goto describe;
	es = data.word(kSetdesc);
	si = (0);
	cx = (0+(130*2));
	_cmp(data.byte(kObjecttype), 1);
	if (flags.z())
		goto describe;
	es = data.word(kExtras);
	si = (0+2080+30000+(16*114));
	cx = (0+2080+30000+(16*114)+((114+2)*2));
describe:
	al = data.byte(kCommand);
	ah = 0;
	_add(ax, ax);
	_add(si, ax);
	ax = es.word(si);
	_add(ax, cx);
	si = ax;
	bx = ax;
tryagain:
	push(si);
	findNextColon();
	al = es.byte(si);
	cx = si;
	si = pop();
	_cmp(data.byte(kObjecttype), 1);
	if (!flags.z())
		return /* (cantmakeoneup) */;
	_cmp(al, 0);
	if (flags.z())
		goto findsometext;
	_cmp(al, ':');
	if (flags.z())
		goto findsometext;
	return;
findsometext:
	searchForSame();
	goto tryagain;
}

void DreamGenContext::searchForSame() {
	STACK_CHECK;
	si = cx;
searchagain:
	_inc(si);
	al = es.byte(bx);
search:
	_cmp(es.byte(si), al);
	if (flags.z())
		goto gotstartletter;
	_inc(cx);
	_inc(si);
	_cmp(si, 8000);
	if (flags.c())
		goto search;
	si = bx;
	ax = pop();
	return;
gotstartletter:
	push(bx);
	push(si);
keepchecking:
	_inc(si);
	_inc(bx);
	al = es.byte(bx);
	ah = es.byte(si);
	_cmp(al, ':');
	if (flags.z())
		goto foundmatch;
	_cmp(al, 0);
	if (flags.z())
		goto foundmatch;
	_cmp(al, ah);
	if (flags.z())
		goto keepchecking;
	si = pop();
	bx = pop();
	goto searchagain;
foundmatch:
	si = pop();
	bx = pop();
}

void DreamGenContext::reExFromInv() {
	STACK_CHECK;
	findInvPos();
	ax = es.word(bx);
	data.byte(kCommandtype) = ah;
	data.byte(kCommand) = al;
	data.byte(kExamagain) = 1;
	data.byte(kPointermode) = 0;
}

void DreamGenContext::swapWithInv() {
	STACK_CHECK;
	al = data.byte(kItemframe);
	ah = data.byte(kObjecttype);
	_cmp(ax, data.word(kOldsubject));
	if (!flags.z())
		goto difsub7;
	_cmp(data.byte(kCommandtype), 243);
	if (flags.z())
		goto alreadyswap1;
	data.byte(kCommandtype) = 243;
difsub7:
	data.word(kOldsubject) = ax;
	bx = ax;
	al = 34;
	commandWithOb();
alreadyswap1:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (cantswap1) */;
	_and(ax, 1);
	if (!flags.z())
		goto doswap1;
	return;
doswap1:
	ah = data.byte(kObjecttype);
	al = data.byte(kItemframe);
	push(ax);
	findInvPos();
	ax = es.word(bx);
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = ah;
	getEitherAd();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
	bl = data.byte(kItemframe);
	bh = data.byte(kObjecttype);
	ax = pop();
	data.byte(kObjecttype) = ah;
	data.byte(kItemframe) = al;
	push(bx);
	findInvPos();
	delPointer();
	al = data.byte(kItemframe);
	getEitherAd();
	es.byte(bx+2) = 4;
	es.byte(bx+3) = 255;
	al = data.byte(kLastinvpos);
	es.byte(bx+4) = al;
	ax = pop();
	data.byte(kObjecttype) = ah;
	data.byte(kItemframe) = al;
	fillRyan();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::swapWithOpen() {
	STACK_CHECK;
	al = data.byte(kItemframe);
	ah = data.byte(kObjecttype);
	_cmp(ax, data.word(kOldsubject));
	if (!flags.z())
		goto difsub8;
	_cmp(data.byte(kCommandtype), 242);
	if (flags.z())
		goto alreadyswap2;
	data.byte(kCommandtype) = 242;
difsub8:
	data.word(kOldsubject) = ax;
	bx = ax;
	al = 34;
	commandWithOb();
alreadyswap2:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (cantswap2) */;
	_and(ax, 1);
	if (!flags.z())
		goto doswap2;
	return;
doswap2:
	getEitherAd();
	isItWorn();
	if (!flags.z())
		goto notwornswap;
	wornError();
	return;
notwornswap:
	delPointer();
	al = data.byte(kItemframe);
	_cmp(al, data.byte(kOpenedob));
	if (!flags.z())
		goto isntsame2;
	al = data.byte(kObjecttype);
	_cmp(al, data.byte(kOpenedtype));
	if (!flags.z())
		goto isntsame2;
	errorMessage1();
	return;
isntsame2:
	checkObjectSize();
	_cmp(al, 0);
	if (flags.z())
		goto sizeok2;
	return;
sizeok2:
	ah = data.byte(kObjecttype);
	al = data.byte(kItemframe);
	push(ax);
	findOpenPos();
	ax = es.word(bx);
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = ah;
	_cmp(ah, 4);
	if (!flags.z())
		goto makeswapex;
	getEitherAd();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
	goto actuallyswap;
makeswapex:
	transferToEx();
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = 4;
	getEitherAd();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
actuallyswap:
	bl = data.byte(kItemframe);
	bh = data.byte(kObjecttype);
	ax = pop();
	data.byte(kObjecttype) = ah;
	data.byte(kItemframe) = al;
	push(bx);
	findOpenPos();
	getEitherAd();
	al = data.byte(kOpenedtype);
	es.byte(bx+2) = al;
	al = data.byte(kOpenedob);
	es.byte(bx+3) = al;
	al = data.byte(kLastinvpos);
	es.byte(bx+4) = al;
	al = data.byte(kReallocation);
	es.byte(bx+5) = al;
	ax = pop();
	data.byte(kObjecttype) = ah;
	data.byte(kItemframe) = al;
	fillOpen();
	fillRyan();
	underTextLine();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::inToInv() {
	STACK_CHECK;
	_cmp(data.byte(kPickup), 0);
	if (!flags.z())
		goto notout;
	outOfInv();
	return;
notout:
	findInvPos();
	ax = es.word(bx);
	_cmp(al, 255);
	if (flags.z())
		goto canplace1;
	swapWithInv();
	return;
canplace1:
	al = data.byte(kItemframe);
	ah = data.byte(kObjecttype);
	_cmp(ax, data.word(kOldsubject));
	if (!flags.z())
		goto difsub1;
	_cmp(data.byte(kCommandtype), 220);
	if (flags.z())
		goto alreadyplce;
	data.byte(kCommandtype) = 220;
difsub1:
	data.word(kOldsubject) = ax;
	bx = ax;
	al = 35;
	commandWithOb();
alreadyplce:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notletgo2) */;
	_and(ax, 1);
	if (!flags.z())
		goto doplace;
	return;
doplace:
	delPointer();
	al = data.byte(kItemframe);
	getExAd();
	es.byte(bx+2) = 4;
	es.byte(bx+3) = 255;
	al = data.byte(kLastinvpos);
	es.byte(bx+4) = al;
	data.byte(kPickup) = 0;
	fillRyan();
	readMouse();
	showPointer();
	outOfInv();
	workToScreen();
	delPointer();
}

void DreamGenContext::outOfInv() {
	STACK_CHECK;
	findInvPos();
	ax = es.word(bx);
	_cmp(al, 255);
	if (!flags.z())
		goto canpick2;
	blank();
	return;
canpick2:
	bx = data.word(kMousebutton);
	_cmp(bx, 2);
	if (!flags.z())
		goto canpick2a;
	reExFromInv();
	return;
canpick2a:
	_cmp(ax, data.word(kOldsubject));
	if (!flags.z())
		goto difsub3;
	_cmp(data.byte(kCommandtype), 221);
	if (flags.z())
		goto alreadygrab;
	data.byte(kCommandtype) = 221;
difsub3:
	data.word(kOldsubject) = ax;
	bx = ax;
	al = 36;
	commandWithOb();
alreadygrab:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notletgo) */;
	_and(ax, 1);
	if (!flags.z())
		goto dograb;
	return;
dograb:
	delPointer();
	data.byte(kPickup) = 1;
	findInvPos();
	ax = es.word(bx);
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = ah;
	getExAd();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
	fillRyan();
	readMouse();
	showPointer();
	inToInv();
	workToScreen();
	delPointer();
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

void DreamGenContext::dropObject() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 223);
	if (flags.z())
		goto alreadydrop;
	data.byte(kCommandtype) = 223;
	_cmp(data.byte(kPickup), 0);
	if (flags.z())
		{ blank(); return; };
	bl = data.byte(kItemframe);
	bh = data.byte(kObjecttype);
	al = 37;
	commandWithOb();
alreadydrop:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (nodrop) */;
	_and(ax, 1);
	if (!flags.z())
		goto dodrop;
	return;
dodrop:
	getEitherAd();
	isItWorn();
	if (!flags.z())
		goto nowornerror;
	wornError();
	return;
nowornerror:
	_cmp(data.byte(kReallocation), 47);
	if (flags.z())
		goto nodrop2;
	cl = data.byte(kRyanx);
	_add(cl, 12);
	ch = data.byte(kRyany);
	_add(ch, 12);
	checkOne();
	_cmp(cl, 2);
	if (flags.c())
		goto nodroperror;
nodrop2:
	dropError();
	return;
nodroperror:
	_cmp(data.byte(kMapxsize), 64);
	if (!flags.z())
		goto notinlift;
	_cmp(data.byte(kMapysize), 64);
	if (!flags.z())
		goto notinlift;
	dropError();
	return;
notinlift:
	al = data.byte(kItemframe);
	ah = 4;
	cl = 'G';
	ch = 'U';
	dl = 'N';
	dh = 'A';
	compare();
	if (flags.z())
		{ cantDrop(); return; };
	al = data.byte(kItemframe);
	ah = 4;
	cl = 'S';
	ch = 'H';
	dl = 'L';
	dh = 'D';
	compare();
	if (flags.z())
		{ cantDrop(); return; };
	data.byte(kObjecttype) = 4;
	al = data.byte(kItemframe);
	getExAd();
	es.byte(bx+2) = 0;
	al = data.byte(kRyanx);
	_add(al, 4);
	cl = 4;
	_shr(al, cl);
	_add(al, data.byte(kMapx));
	ah = data.byte(kRyany);
	_add(ah, 8);
	cl = 4;
	_shr(ah, cl);
	_add(ah, data.byte(kMapy));
	es.byte(bx+3) = al;
	es.byte(bx+5) = ah;
	al = data.byte(kRyanx);
	_add(al, 4);
	_and(al, 15);
	ah = data.byte(kRyany);
	_add(ah, 8);
	_and(ah, 15);
	es.byte(bx+4) = al;
	es.byte(bx+6) = ah;
	data.byte(kPickup) = 0;
	al = data.byte(kReallocation);
	es.byte(bx) = al;
}

void DreamGenContext::removeObFromInv() {
	STACK_CHECK;
	_cmp(data.byte(kCommand), 100);
	if (flags.z())
		return /* (obnotexist) */;
	getAnyAd();
	di = bx;
	cl = data.byte(kCommand);
	ch = 0;
	deleteExObject();
}

void DreamGenContext::selectOpenOb() {
	STACK_CHECK;
	al = data.byte(kCommand);
	getAnyAd();
	_cmp(al, 255);
	if (!flags.z())
		goto canopenit1;
	blank();
	return;
canopenit1:
	_cmp(data.byte(kCommandtype), 224);
	if (flags.z())
		goto alreadyopob;
	data.byte(kCommandtype) = 224;
	bl = data.byte(kCommand);
	bh = data.byte(kObjecttype);
	al = 38;
	commandWithOb();
alreadyopob:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (noopenob) */;
	_and(ax, 1);
	if (!flags.z())
		goto doopenob;
	return;
doopenob:
	al = data.byte(kCommand);
	data.byte(kOpenedob) = al;
	al = data.byte(kObjecttype);
	data.byte(kOpenedtype) = al;
	createPanel();
	showPanel();
	showMan();
	examIcon();
	showExit();
	openInv();
	openOb();
	underTextLine();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::useOpened() {
	STACK_CHECK;
	_cmp(data.byte(kOpenedob), 255);
	if (flags.z())
		return /* (cannotuseopen) */;
	_cmp(data.byte(kPickup), 0);
	if (!flags.z())
		goto notout2;
	outOfOpen();
	return;
notout2:
	findOpenPos();
	ax = es.word(bx);
	_cmp(al, 255);
	if (flags.z())
		goto canplace3;
	swapWithOpen();
	return;
canplace3:
	_cmp(data.byte(kPickup), 1);
	if (flags.z())
		goto intoopen;
	blank();
	return;
intoopen:
	al = data.byte(kItemframe);
	ah = data.byte(kObjecttype);
	_cmp(ax, data.word(kOldsubject));
	if (!flags.z())
		goto difsub2;
	_cmp(data.byte(kCommandtype), 227);
	if (flags.z())
		goto alreadyplc2;
	data.byte(kCommandtype) = 227;
difsub2:
	data.word(kOldsubject) = ax;
	bx = ax;
	al = 35;
	commandWithOb();
alreadyplc2:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notletgo3) */;
	_cmp(ax, 1);
	if (flags.z())
		goto doplace2;
	return;
doplace2:
	getEitherAd();
	isItWorn();
	if (!flags.z())
		goto notworntoopen;
	wornError();
	return;
notworntoopen:
	delPointer();
	al = data.byte(kItemframe);
	_cmp(al, data.byte(kOpenedob));
	if (!flags.z())
		goto isntsame;
	al = data.byte(kObjecttype);
	_cmp(al, data.byte(kOpenedtype));
	if (!flags.z())
		goto isntsame;
	errorMessage1();
	return;
isntsame:
	checkObjectSize();
	_cmp(al, 0);
	if (flags.z())
		goto sizeok1;
	return;
sizeok1:
	data.byte(kPickup) = 0;
	al = data.byte(kItemframe);
	getEitherAd();
	al = data.byte(kOpenedtype);
	es.byte(bx+2) = al;
	al = data.byte(kOpenedob);
	es.byte(bx+3) = al;
	al = data.byte(kLastinvpos);
	es.byte(bx+4) = al;
	al = data.byte(kReallocation);
	es.byte(bx+5) = al;
	fillOpen();
	underTextLine();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::checkObjectSize() {
	STACK_CHECK;
	getOpenedSize();
	push(ax);
	al = data.byte(kItemframe);
	getEitherAd();
	al = es.byte(bx+9);
	cx = pop();
	_cmp(al, 255);
	if (!flags.z())
		goto notunsized;
	al = 6;
notunsized:
	_cmp(al, 100);
	if (!flags.c())
		goto specialcase;
	_cmp(cl, 100);
	if (flags.c())
		goto isntspecial;
	errorMessage3();
	goto sizewrong;
isntspecial:
	_cmp(cl, al);
	if (!flags.c())
		goto sizeok;
specialcase:
	_sub(al, 100);
	_cmp(cl, 100);
	if (!flags.c())
		goto bothspecial;
	_cmp(cl, al);
	if (!flags.c())
		goto sizeok;
	errorMessage2();
	goto sizewrong;
bothspecial:
	_sub(cl, 100);
	_cmp(al, cl);
	if (flags.z())
		goto sizeok;
	errorMessage3();
sizewrong:
	al = 1;
	return;
sizeok:
	al = 0;
}

void DreamGenContext::outOfOpen() {
	STACK_CHECK;
	_cmp(data.byte(kOpenedob), 255);
	if (flags.z())
		goto cantuseopen;
	findOpenPos();
	ax = es.word(bx);
	_cmp(al, 255);
	if (!flags.z())
		goto canpick4;
cantuseopen:
	blank();
	return;
canpick4:
	_cmp(ax, data.word(kOldsubject));
	if (!flags.z())
		goto difsub4;
	_cmp(data.byte(kCommandtype), 228);
	if (flags.z())
		goto alreadygrb;
	data.byte(kCommandtype) = 228;
difsub4:
	data.word(kOldsubject) = ax;
	bx = ax;
	al = 36;
	commandWithOb();
alreadygrb:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notletgo4) */;
	_cmp(ax, 1);
	if (flags.z())
		goto dogrb;
	_cmp(ax, 2);
	if (!flags.z())
		return /* (notletgo4) */;
	reExFromOpen();
	return;
dogrb:
	delPointer();
	data.byte(kPickup) = 1;
	findOpenPos();
	ax = es.word(bx);
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = ah;
	_cmp(ah, 4);
	if (!flags.z())
		goto makeintoex;
	getEitherAd();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
	goto actuallyout;
makeintoex:
	transferToEx();
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = 4;
	getEitherAd();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
actuallyout:
	fillOpen();
	underTextLine();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
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

void DreamGenContext::purgeALocation() {
	STACK_CHECK;
	push(ax);
	es = data.word(kExtras);
	di = (0+2080+30000);
	bx = pop();
	cx = 0;
purgeloc:
	_cmp(bl, es.byte(di+0));
	if (!flags.z())
		goto dontpurge;
	_cmp(es.byte(di+2), 0);
	if (!flags.z())
		goto dontpurge;
	push(di);
	push(es);
	push(bx);
	push(cx);
	deleteExObject();
	cx = pop();
	bx = pop();
	es = pop();
	di = pop();
dontpurge:
	_add(di, 16);
	_inc(cx);
	_cmp(cx, (114));
	if (!flags.z())
		goto purgeloc;
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

void DreamGenContext::deleteExObject() {
	STACK_CHECK;
	push(cx);
	push(cx);
	push(cx);
	push(cx);
	al = 255;
	cx = 16;
	_stosb(cx, true);
	ax = pop();
	cl = al;
	_add(al, al);
	_add(al, cl);
	deleteExFrame();
	ax = pop();
	cl = al;
	_add(al, al);
	_add(al, cl);
	_inc(al);
	deleteExFrame();
	ax = pop();
	deleteExText();
	bx = pop();
	bh = bl;
	bl = 4;
	di = (0+2080+30000);
	cx = 0;
deleteconts:
	_cmp(es.word(di+2), bx);
	if (!flags.z())
		goto notinsideex;
	push(bx);
	push(cx);
	push(di);
	deleteExObject();
	di = pop();
	cx = pop();
	bx = pop();
notinsideex:
	_add(di, 16);
	_inc(cx);
	_cmp(cx, (114));
	if (!flags.z())
		goto deleteconts;
}

void DreamGenContext::deleteExFrame() {
	STACK_CHECK;
	di = (0);
	ah = 0;
	_add(ax, ax);
	_add(di, ax);
	_add(ax, ax);
	_add(di, ax);
	al = es.byte(di);
	ah = 0;
	cl = es.byte(di+1);
	ch = 0;
	_mul(cx);
	si = es.word(di+2);
	push(si);
	_add(si, (0+2080));
	cx = (30000);
	_sub(cx, es.word(di+2));
	di = si;
	_add(si, ax);
	push(ax);
	ds = es;
	_movsb(cx, true);
	bx = pop();
	_sub(data.word(kExframepos), bx);
	si = pop();
	cx = (114)*3;
	di = (0);
shuffleadsdown:
	ax = es.word(di+2);
	_cmp(ax, si);
	if (flags.c())
		goto beforethisone;
	_sub(ax, bx);
beforethisone:
	es.word(di+2) = ax;
	_add(di, 6);
	if (--cx)
		goto shuffleadsdown;
}

void DreamGenContext::deleteExText() {
	STACK_CHECK;
	di = (0+2080+30000+(16*114));
	ah = 0;
	_add(ax, ax);
	_add(di, ax);
	ax = es.word(di);
	si = ax;
	di = ax;
	_add(si, (0+2080+30000+(16*114)+((114+2)*2)));
	_add(di, (0+2080+30000+(16*114)+((114+2)*2)));
	ax = 0;
findlenextext:
	cl = es.byte(si);
	_inc(ax);
	_inc(si);
	_cmp(cl, 0);
	if (!flags.z())
		goto findlenextext;
	cx = (18000);
	bx = si;
	_sub(bx, (0+2080+30000+(16*114)+((114+2)*2)));
	push(bx);
	push(ax);
	_sub(cx, bx);
	_movsb(cx, true);
	bx = pop();
	_sub(data.word(kExtextpos), bx);
	si = pop();
	cx = (114);
	di = (0+2080+30000+(16*114));
shuffletextads:
	ax = es.word(di);
	_cmp(ax, si);
	if (flags.c())
		goto beforethistext;
	_sub(ax, bx);
beforethistext:
	es.word(di) = ax;
	_add(di, 2);
	if (--cx)
		goto shuffletextads;
}

void DreamGenContext::startTalk() {
	STACK_CHECK;
	data.byte(kTalkmode) = 0;
	al = data.byte(kCharacter);
	_and(al, 127);
	getPersonText();
	data.word(kCharshift) = 91+91;
	di = 66;
	bx = 64;
	dl = 241;
	al = 0;
	ah = 79;
	printDirect();
	data.word(kCharshift) = 0;
	di = 66;
	bx = 80;
	dl = 241;
	al = 0;
	ah = 0;
	printDirect();
	data.byte(kSpeechloaded) = 0;
	al = data.byte(kCharacter);
	_and(al, 127);
	ah = 0;
	cx = 64;
	_mul(cx);
	cl = 'C';
	dl = 'R';
	dh = data.byte(kReallocation);
	loadSpeech();
	_cmp(data.byte(kSpeechloaded), 1);
	if (!flags.z())
		return /* (nospeech1) */;
	data.byte(kVolumedirection) = 1;
	data.byte(kVolumeto) = 6;
	al = 50+12;
	playChannel1();
}

void DreamGenContext::getPersonText() {
	STACK_CHECK;
	ah = 0;
	cx = 64*2;
	_mul(cx);
	si = ax;
	es = data.word(kPeople);
	_add(si, (0+24));
	cx = (0+24+(1026*2));
	ax = es.word(si);
	_add(ax, cx);
	si = ax;
}

void DreamGenContext::doSomeTalk() {
	STACK_CHECK;
dospeech:
	al = data.byte(kTalkpos);
	al = data.byte(kCharacter);
	_and(al, 127);
	ah = 0;
	cx = 64;
	_mul(cx);
	cx = ax;
	al = data.byte(kTalkpos);
	ah = 0;
	_add(ax, cx);
	_add(ax, ax);
	si = ax;
	es = data.word(kPeople);
	_add(si, (0+24));
	cx = (0+24+(1026*2));
	ax = es.word(si);
	_add(ax, cx);
	si = ax;
	_cmp(es.byte(si), 0);
	if (flags.z())
		goto endheartalk;
	push(es);
	push(si);
	createPanel();
	showPanel();
	showMan();
	showExit();
	convIcons();
	si = pop();
	es = pop();
	di = 164;
	bx = 64;
	dl = 144;
	al = 0;
	ah = 0;
	printDirect();
	al = data.byte(kCharacter);
	_and(al, 127);
	ah = 0;
	cx = 64;
	_mul(cx);
	cl = data.byte(kTalkpos);
	ch = 0;
	_add(ax, cx);
	cl = 'C';
	dl = 'R';
	dh = data.byte(kReallocation);
	loadSpeech();
	_cmp(data.byte(kSpeechloaded), 0);
	if (flags.z())
		goto noplay1;
	al = 62;
	playChannel1();
noplay1:
	data.byte(kPointermode) = 3;
	workToScreenM();
	cx = 180;
	hangOnPQ();
	if (!flags.c())
		goto _tmp1;
	return;
_tmp1:
	_inc(data.byte(kTalkpos));
	al = data.byte(kTalkpos);
	al = data.byte(kCharacter);
	_and(al, 127);
	ah = 0;
	cx = 64;
	_mul(cx);
	cx = ax;
	al = data.byte(kTalkpos);
	ah = 0;
	_add(ax, cx);
	_add(ax, ax);
	si = ax;
	es = data.word(kPeople);
	_add(si, (0+24));
	cx = (0+24+(1026*2));
	ax = es.word(si);
	_add(ax, cx);
	si = ax;
	_cmp(es.byte(si), 0);
	if (flags.z())
		goto endheartalk;
	_cmp(es.byte(si), ':');
	if (flags.z())
		goto skiptalk2;
	_cmp(es.byte(si), 32);
	if (flags.z())
		goto skiptalk2;
	push(es);
	push(si);
	createPanel();
	showPanel();
	showMan();
	showExit();
	convIcons();
	si = pop();
	es = pop();
	di = 48;
	bx = 128;
	dl = 144;
	al = 0;
	ah = 0;
	printDirect();
	al = data.byte(kCharacter);
	_and(al, 127);
	ah = 0;
	cx = 64;
	_mul(cx);
	cl = data.byte(kTalkpos);
	ch = 0;
	_add(ax, cx);
	cl = 'C';
	dl = 'R';
	dh = data.byte(kReallocation);
	loadSpeech();
	_cmp(data.byte(kSpeechloaded), 0);
	if (flags.z())
		goto noplay2;
	al = 62;
	playChannel1();
noplay2:
	data.byte(kPointermode) = 3;
	workToScreenM();
	cx = 180;
	hangOnPQ();
	if (!flags.c())
		goto skiptalk2;
	return;
skiptalk2:
	_inc(data.byte(kTalkpos));
	goto dospeech;
endheartalk:
	data.byte(kPointermode) = 0;
}

void DreamGenContext::lookAtPlace() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 224);
	if (flags.z())
		goto alreadyinfo;
	data.byte(kCommandtype) = 224;
	al = 27;
	commandOnly();
alreadyinfo:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (flags.z())
		return /* (noinfo) */;
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (noinfo) */;
	bl = data.byte(kDestpos);
	_cmp(bl, 15);
	if (!flags.c())
		return /* (noinfo) */;
	push(bx);
	delPointer();
	delTextLine();
	getUnderCentre();
	ds = data.word(kTempgraphics3);
	al = 0;
	ah = 0;
	di = 60;
	bx = 72;
	showFrame();
	al = 4;
	ah = 0;
	di = 60;
	bx = 72+55;
	showFrame();
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto _tmp1;
	al = 4;
	ah = 0;
	di = 60;
	bx = 72+55+21;
	showFrame();
_tmp1:
	bx = pop();
	bh = 0;
	_add(bx, bx);
	es = data.word(kTraveltext);
	si = es.word(bx);
	_add(si, (66*2));
	findNextColon();
	di = 63;
	bx = 84;
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto _tmp2;
	bx = 84+4;
_tmp2:
	dl = 191;
	al = 0;
	ah = 0;
	printDirect();
	workToScreenM();
	cx = 500;
	hangOnP();
	data.byte(kPointermode) = 0;
	data.byte(kPointerframe) = 0;
	putUnderCentre();
	workToScreenM();
}

void DreamGenContext::locationPic() {
	STACK_CHECK;
	getDestInfo();
	al = es.byte(si);
	push(es);
	push(si);
	di = 0;
	_cmp(al, 6);
	if (!flags.c())
		goto secondlot;
	ds = data.word(kTempgraphics);
	_add(al, 4);
	goto gotgraphic;
secondlot:
	_sub(al, 6);
	ds = data.word(kTempgraphics2);
gotgraphic:
	_add(di, 104);
	bx = 138+14;
	ah = 0;
	showFrame();
	si = pop();
	es = pop();
	al = data.byte(kDestpos);
	_cmp(al, data.byte(kReallocation));
	if (!flags.z())
		goto notinthisone;
	al = 3;
	di = 104;
	bx = 140+14;
	ds = data.word(kTempgraphics);
	ah = 0;
	showFrame();
notinthisone:
	bl = data.byte(kDestpos);
	bh = 0;
	_add(bx, bx);
	es = data.word(kTraveltext);
	si = es.word(bx);
	_add(si, (66*2));
	di = 50;
	bx = 20;
	dl = 241;
	al = 0;
	ah = 0;
	printDirect();
}

void DreamGenContext::getDestInfo() {
	STACK_CHECK;
	al = data.byte(kDestpos);
	ah = 0;
	push(ax);
	dx = data;
	es = dx;
	si = 1120;
	_add(si, ax);
	cl = es.byte(si);
	ax = pop();
	push(cx);
	dx = data;
	es = dx;
	si = 1136;
	_add(si, ax);
	ax = pop();
}

void DreamGenContext::resetLocation() {
	STACK_CHECK;
	push(ax);
	_cmp(al, 5);
	if (!flags.z())
		goto notdelhotel;
	purgeALocation();
	al = 21;
	purgeALocation();
	al = 22;
	purgeALocation();
	al = 27;
	purgeALocation();
	goto clearedlocations;
notdelhotel:
	_cmp(al, 8);
	if (!flags.z())
		goto notdeltvstud;
	purgeALocation();
	al = 28;
	purgeALocation();
	goto clearedlocations;
notdeltvstud:
	_cmp(al, 6);
	if (!flags.z())
		goto notdelsarters;
	purgeALocation();
	al = 20;
	purgeALocation();
	al = 25;
	purgeALocation();
	goto clearedlocations;
notdelsarters:
	_cmp(al, 13);
	if (!flags.z())
		goto notdelboathouse;
	purgeALocation();
	al = 29;
	purgeALocation();
	goto clearedlocations;
notdelboathouse:
clearedlocations:
	ax = pop();
	ah = 0;
	bx = ax;
	dx = data;
	es = dx;
	_add(bx, 1120);
	es.byte(bx) = 0;
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
	di = 1045;
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

void DreamGenContext::signOn() {
	STACK_CHECK;
	parser();
	_inc(di);
	ds = cs;
	si = offset_keys;
	cx = 4;
signonloop:
	push(cx);
	push(si);
	push(di);
	_add(si, 14);
	cx = 11;
signonloop2:
	_lodsb();
	_cmp(al, 32);
	if (flags.z())
		goto foundsign;
	makeCaps();
	ah = es.byte(di);
	_inc(di);
	_cmp(al, ah);
	if (!flags.z())
		goto nomatch;
	if (--cx)
		goto signonloop2;
nomatch:
	di = pop();
	si = pop();
	cx = pop();
	_add(si, 26);
	if (--cx)
		goto signonloop;
	al = 13;
	monMessage();
	return;
foundsign:
	di = pop();
	si = pop();
	cx = pop();
	bx = si;
	es = ds;
	_cmp(es.byte(bx), 0);
	if (flags.z())
		goto notyetassigned;
	al = 17;
	monMessage();
	return;
notyetassigned:
	push(es);
	push(bx);
	scrollMonitor();
	al = 15;
	monMessage();
	di = data.word(kMonadx);
	bx = data.word(kMonady);
	push(di);
	push(bx);
	input();
	bx = pop();
	di = pop();
	data.word(kMonadx) = di;
	data.word(kMonady) = bx;
	bx = pop();
	es = pop();
	push(es);
	push(bx);
	_add(bx, 2);
	ds = cs;
	si = 1154;
checkpass:
	_lodsw();
	ah = es.byte(bx);
	_inc(bx);
	_cmp(ah, 32);
	if (flags.z())
		goto passpassed;
	_cmp(al, ah);
	if (flags.z())
		goto checkpass;
	bx = pop();
	es = pop();
	scrollMonitor();
	al = 16;
	monMessage();
	return;
passpassed:
	al = 14;
	monMessage();
	bx = pop();
	es = pop();
	push(es);
	push(bx);
	_add(bx, 14);
	monPrint();
	scrollMonitor();
	bx = pop();
	es = pop();
	es.byte(bx) = 1;
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
	di = 1045;
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
	di = 1045+1;
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

void DreamGenContext::getKeyAndLogo() {
	STACK_CHECK;
	_inc(bx);
	al = es.byte(bx);
	_sub(al, 48);
	data.byte(kNewlogonum) = al;
	_add(bx, 2);
	al = es.byte(bx);
	_sub(al, 48);
	data.byte(kKeynum) = al;
	_inc(bx);
	push(es);
	push(bx);
	al = data.byte(kKeynum);
	ah = 0;
	cx = 26;
	_mul(cx);
	es = cs;
	bx = offset_keys;
	_add(bx, ax);
	al = es.byte(bx);
	_cmp(al, 1);
	if (flags.z())
		goto keyok;
	push(bx);
	push(es);
	al = 12;
	monMessage();
	es = pop();
	bx = pop();
	_add(bx, 14);
	monPrint();
	scrollMonitor();
	bx = pop();
	es = pop();
	al = 1;
	return;
keyok:
	bx = pop();
	es = pop();
	al = data.byte(kNewlogonum);
	data.byte(kLogonum) = al;
	al = 0;
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
	si = 1154;
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

void DreamGenContext::nextColon() {
	STACK_CHECK;
lookcolon:
	al = es.byte(si);
	_inc(si);
	_cmp(al, ':');
	if (!flags.z())
		goto lookcolon;
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
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00a0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00b0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00c0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x00, 
		//0x00d0: .... .... .... .  .
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
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0150: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0160: .... .... .... ....
		0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		//0x0170: .... .... .... ....
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		//0x0180: .... .... .... ....
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0x00, 0x00, 0x00, 0x00, 
		//0x0190: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x01a0: .... .... .... ....
		0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x01, 0x2c, 0x00, 0x14, 0x00, 0x02, 0x00, 
		//0x01b0: .... .... ..,. ....
		0x01, 0x01, 0x37, 0x00, 0x00, 0x00, 0x32, 0x14, 0x00, 0x18, 0x16, 0x00, 0x4a, 0x00, 0x01, 0x00, 
		//0x01c0: ..7. ..2. .... J...
		0x00, 0x18, 0x21, 0x0a, 0x4b, 0x00, 0x01, 0x00, 0x01, 0x01, 0x2c, 0x00, 0x1b, 0x00, 0x02, 0x00, 
		//0x01d0: ..!. K... ..,. ....
		0x02, 0x01, 0x2c, 0x00, 0x60, 0x00, 0x03, 0x00, 0x04, 0x01, 0x2c, 0x00, 0x76, 0x00, 0x02, 0x00, 
		//0x01e0: ..,. `... ..,. v...
		0x05, 0x01, 0x2c, 0x0a, 0x00, 0x00, 0x02, 0x00, 0x00, 0x05, 0x16, 0x14, 0x35, 0x00, 0x03, 0x00, 
		//0x01f0: ..,. .... .... 5...
		0x00, 0x05, 0x16, 0x14, 0x28, 0x00, 0x01, 0x00, 0x02, 0x05, 0x16, 0x14, 0x32, 0x00, 0x01, 0x00, 
		//0x0200: .... (... .... 2...
		0x03, 0x02, 0x0b, 0x0a, 0xc0, 0x00, 0x01, 0x00, 0x00, 0x02, 0x0b, 0x0a, 0xb6, 0x00, 0x02, 0x00, 
		//0x0210: .... .... .... ....
		0x01, 0x08, 0x0b, 0x0a, 0x00, 0x00, 0x02, 0x00, 0x01, 0x17, 0x00, 0x32, 0x00, 0x00, 0x03, 0x00, 
		//0x0220: .... .... ...2 ....
		0x00, 0x1c, 0x0b, 0x14, 0xfa, 0x00, 0x04, 0x00, 0x00, 0x17, 0x00, 0x32, 0x2b, 0x00, 0x02, 0x00, 
		//0x0230: .... .... ...2 +...
		0x08, 0x17, 0x0b, 0x28, 0x82, 0x00, 0x02, 0x00, 0x01, 0x17, 0x16, 0x28, 0x7a, 0x00, 0x02, 0x00, 
		//0x0240: ...( .... ...( z...
		0x02, 0x17, 0x16, 0x28, 0x69, 0x00, 0x02, 0x00, 0x03, 0x17, 0x16, 0x28, 0x51, 0x00, 0x02, 0x00, 
		//0x0250: ...( i... ...( Q...
		0x04, 0x17, 0x0b, 0x28, 0x87, 0x00, 0x02, 0x00, 0x05, 0x17, 0x16, 0x28, 0x91, 0x00, 0x02, 0x00, 
		//0x0260: ...( .... ...( ....
		0x06, 0x04, 0x16, 0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x2d, 0x16, 0x1e, 0xc8, 0x00, 0x00, 0x00, 
		//0x0270: .... .... .-.. ....
		0x14, 0x2d, 0x16, 0x1e, 0x27, 0x00, 0x02, 0x00, 0x00, 0x2d, 0x16, 0x1e, 0x19, 0x00, 0x02, 0x00, 
		//0x0280: .-.. '... .-.. ....
		0x00, 0x08, 0x16, 0x28, 0x20, 0x00, 0x02, 0x00, 0x00, 0x07, 0x0b, 0x14, 0x40, 0x00, 0x02, 0x00, 
		//0x0290: ...(  ... .... @...
		0x00, 0x16, 0x16, 0x14, 0x52, 0x00, 0x02, 0x00, 0x00, 0x1b, 0x0b, 0x1e, 0x00, 0x00, 0x02, 0x00, 
		//0x02a0: .... R... .... ....
		0x00, 0x14, 0x00, 0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x0e, 0x21, 0x28, 0x15, 0x00, 0x01, 0x00, 
		//0x02b0: .... .... ..!( ....
		0x00, 0x1d, 0x0b, 0x0a, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x02, 0x00, 0x02, 0x00, 
		//0x02c0: .... .... .... ....
		0x00, 0x19, 0x00, 0x32, 0x04, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 0x1e, 0x79, 0x00, 0x02, 0x00, 
		//0x02d0: ...2 .... .2.. y...
		0x00, 0x32, 0x16, 0x1e, 0x00, 0x00, 0x14, 0x00, 0x00, 0x34, 0x16, 0x1e, 0xc0, 0x00, 0x02, 0x00, 
		//0x02e0: .2.. .... .4.. ....
		0x00, 0x34, 0x16, 0x1e, 0xe9, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 0x28, 0x68, 0x00, 0x37, 0x00, 
		//0x02f0: .4.. .... .2.( h.7.
		0x00, 0x35, 0x21, 0x00, 0x63, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 0x28, 0x00, 0x00, 0x03, 0x00, 
		//0x0300: .5!. c... .2.( ....
		0x00, 0x32, 0x16, 0x1e, 0xa2, 0x00, 0x02, 0x00, 0x00, 0x34, 0x16, 0x1e, 0x39, 0x00, 0x02, 0x00, 
		//0x0310: .2.. .... .4.. 9...
		0x00, 0x34, 0x16, 0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x36, 0x00, 0x00, 0x48, 0x00, 0x03, 0x00, 
		//0x0320: .4.. .... .6.. H...
		0x00, 0x37, 0x2c, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 
		//0x0330: .7,. .... .... ....
		0x00, 0x0e, 0x16, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x0e, 0x16, 0x00, 0x2c, 0x01, 0x01, 0x00, 
		//0x0340: .... .... .... ,...
		0x00, 0x0a, 0x16, 0x1e, 0xae, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x16, 0x14, 0x00, 0x00, 0x01, 0x00, 
		//0x0350: .... .... .... ....
		0x00, 0x0b, 0x0b, 0x14, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0b, 0x0b, 0x1e, 0x00, 0x00, 0x32, 0x14, 
		//0x0360: .... ..2. .... ..2.
		0x00, 0x0b, 0x16, 0x14, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0e, 0x21, 0x28, 0x00, 0x00, 0x32, 0x14, 
		//0x0370: .... ..2. ..!( ..2.
		0x00, 0xff, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x56, 0x39, 0x39, 0x00, 0x01, 
		//0x0380: ..DR EAMW EB.V 99..
		0x00, 0x50, 0x55, 0x42, 0x4c, 0x49, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x50, 0x55, 0x42, 
		//0x0390: .PUB LIC        PUB
		0x4c, 0x49, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x42, 0x4c, 0x41, 0x43, 0x4b, 
		//0x03a0: LIC       ...B LACK
		0x44, 0x52, 0x41, 0x47, 0x4f, 0x4e, 0x20, 0x52, 0x59, 0x41, 0x4e, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x03b0: DRAG ON R YAN      
		0x20, 0x20, 0x00, 0x00, 0x00, 0x48, 0x45, 0x4e, 0x44, 0x52, 0x49, 0x58, 0x20, 0x20, 0x20, 0x20, 
		//0x03c0:   .. .HEN DRIX     
		0x20, 0x4c, 0x4f, 0x55, 0x49, 0x53, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x53, 
		//0x03d0:  LOU IS        ...S
		0x45, 0x50, 0x54, 0x49, 0x4d, 0x55, 0x53, 0x20, 0x20, 0x20, 0x20, 0x42, 0x45, 0x43, 0x4b, 0x45, 
		//0x03e0: EPTI MUS     B ECKE
		0x54, 0x54, 0x20, 0x20, 0x20, 0x20, 0x00, 0xff, 0xff, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x03f0: TT     .. .        
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x22, 0x52, 0x4f, 0x4f, 0x54, 0x20, 0x20, 0x20, 0x20, 
		//0x0400:        ." ROOT     
		0x20, 0x20, 0x20, 0x20, 0x00, 0x22, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x0410:      ."            
		0x20, 0x20, 0x00, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x10, 0x12, 0x12, 0x11, 0x10, 0x10, 0x10, 0x01, 
		//0x0420:   .. ...$ .... ....
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
		//0x0430: .... .... .... ....
		0x44, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0440: D:.. .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0450: .... .... .... ....
		0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0460: .... .... .... ....
		0x05, 0x00, 0x03, 0x02, 0x04, 0x01, 0x0a, 0x09, 0x08, 0x06, 0x0b, 0x04, 0x07, 0x07, 0x00, 0x00, 
		//0x0470: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0480: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0490: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x04a0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x04b0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x04c0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x04d0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x04e0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x04f0: .... .... .... ....
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, };
	ds.assign(src, src + sizeof(src));
	dreamweb(); 
}

} // End of namespace DreamGen
