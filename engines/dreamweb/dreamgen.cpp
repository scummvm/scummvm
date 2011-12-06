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

void DreamGenContext::alleyBarkSound() {
	STACK_CHECK;
	ax = es.word(bx+3);
	_dec(ax);
	_cmp(ax, 0);
	if (!flags.z())
		goto nobark;
	push(bx);
	push(es);
	al = 14;
	playChannel1();
	es = pop();
	bx = pop();
	ax = 1000;
nobark:
	es.word(bx+3) = ax;
}

void DreamGenContext::receptionist() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto gotrecep;
	_cmp(data.byte(kCardpassflag), 1);
	if (!flags.z())
		goto notsetcard;
	_inc(data.byte(kCardpassflag));
	es.byte(bx+7) = 1;
	es.word(bx+3) = 64;
notsetcard:
	_cmp(es.word(bx+3), 58);
	if (!flags.z())
		goto notdes1;
	randomNumber();
	_cmp(al, 30);
	if (flags.c())
		goto notdes2;
	es.word(bx+3) = 55;
	goto gotrecep;
notdes1:
	_cmp(es.word(bx+3), 60);
	if (!flags.z())
		goto notdes2;
	randomNumber();
	_cmp(al, 240);
	if (flags.c())
		goto gotrecep;
	es.word(bx+3) = 53;
	goto gotrecep;
notdes2:
	_cmp(es.word(bx+3), 88);
	if (!flags.z())
		goto notendcard;
	es.word(bx+3) = 53;
	goto gotrecep;
notendcard:
	_inc(es.word(bx+3));
gotrecep:
	showGameReel();
	addToPeopleList();
	al = es.byte(bx+7);
	_and(al, 128);
	if (flags.z())
		return /* (nottalkedrecep) */;
	data.byte(kTalkedtorecep) = 1;
}

void DreamGenContext::attendant() {
	STACK_CHECK;
	showGameReel();
	addToPeopleList();
	al = es.byte(bx+7);
	_and(al, 128);
	if (flags.z())
		return /* (nottalked) */;
	data.byte(kTalkedtoattendant) = 1;
}

void DreamGenContext::louisChair() {
	STACK_CHECK;
	_cmp(data.byte(kRockstardead), 0);
	if (flags.z())
		return /* (notlouis2) */;
	checkSpeed();
	if (!flags.z())
		goto notlouisanim;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 191);
	if (flags.z())
		goto restartlouis;
	_cmp(ax, 185);
	if (flags.z())
		goto randomlouis;
	es.word(bx+3) = ax;
	goto notlouisanim;
randomlouis:
	es.word(bx+3) = ax;
	randomNumber();
	_cmp(al, 245);
	if (!flags.c())
		goto notlouisanim;
restartlouis:
	ax = 182;
	es.word(bx+3) = ax;
notlouisanim:
	showGameReel();
	addToPeopleList();
}

void DreamGenContext::manAsleep2() {
	STACK_CHECK;
	al = es.byte(bx+7);
	_and(al, 127);
	es.byte(bx+7) = al;
	showGameReel();
	addToPeopleList();
}

void DreamGenContext::drinker() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto gotdrinker;
	_inc(es.word(bx+3));
	_cmp(es.word(bx+3), 115);
	if (!flags.z())
		goto notdrinker1;
	es.word(bx+3) = 105;
	goto gotdrinker;
notdrinker1:
	_cmp(es.word(bx+3), 106);
	if (!flags.z())
		goto gotdrinker;
	randomNumber();
	_cmp(al, 3);
	if (flags.c())
		goto gotdrinker;
	es.word(bx+3) = 105;
gotdrinker:
	showGameReel();
	addToPeopleList();
}

void DreamGenContext::bartender() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto gotsmoket;
	_cmp(es.word(bx+3), 86);
	if (!flags.z())
		goto notsmoket1;
	randomNumber();
	_cmp(al, 18);
	if (flags.c())
		goto notsmoket2;
	es.word(bx+3) = 81;
	goto gotsmoket;
notsmoket1:
	_cmp(es.word(bx+3), 103);
	if (!flags.z())
		goto notsmoket2;
	es.word(bx+3) = 81;
	goto gotsmoket;
notsmoket2:
	_inc(es.word(bx+3));
gotsmoket:
	showGameReel();
	_cmp(data.byte(kGunpassflag), 1);
	if (!flags.z())
		goto notgotgun;
	es.byte(bx+7) = 9;
notgotgun:
	addToPeopleList();
}

void DreamGenContext::interviewer() {
	STACK_CHECK;
	_cmp(data.word(kReeltowatch), 68);
	if (!flags.z())
		goto notgeneralstart;
	_inc(es.word(bx+3));
notgeneralstart:
	_cmp(es.word(bx+3), 250);
	if (flags.z())
		goto talking;
	checkSpeed();
	if (!flags.z())
		goto talking;
	_cmp(es.word(bx+3), 259);
	if (flags.z())
		goto talking;
	_inc(es.word(bx+3));
talking:
	showGameReel();
}

void DreamGenContext::soldier1() {
	STACK_CHECK;
	_cmp(es.word(bx+3), 0);
	if (flags.z())
		goto soldierwait;
	data.word(kWatchingtime) = 10;
	_cmp(es.word(bx+3), 30);
	if (!flags.z())
		goto notaftersshot;
	_inc(data.byte(kCombatcount));
	_cmp(data.byte(kCombatcount), 40);
	if (!flags.z())
		goto gotsoldframe;
	data.byte(kMandead) = 2;
	goto gotsoldframe;
notaftersshot:
	checkSpeed();
	if (!flags.z())
		goto gotsoldframe;
	_inc(es.word(bx+3));
	goto gotsoldframe;
soldierwait:
	_cmp(data.byte(kLastweapon), 1);
	if (!flags.z())
		goto gotsoldframe;
	data.word(kWatchingtime) = 10;
	_cmp(data.byte(kManspath), 2);
	if (!flags.z())
		goto gotsoldframe;
	_cmp(data.byte(kFacing), 4);
	if (!flags.z())
		goto gotsoldframe;
	_inc(es.word(bx+3));
	data.byte(kLastweapon) = -1;
	data.byte(kCombatcount) = 0;
gotsoldframe:
	showGameReel();
	addToPeopleList();
}

void DreamGenContext::helicopter() {
	STACK_CHECK;
	ax = es.word(bx+3);
	_cmp(ax, 203);
	if (flags.z())
		goto heliwon;
	checkSpeed();
	if (!flags.z())
		goto helispeed;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 53);
	if (!flags.z())
		goto notbeforehdead;
	_inc(data.byte(kCombatcount));
	_cmp(data.byte(kCombatcount), 8);
	if (flags.c())
		goto waitabit;
	data.byte(kMandead) = 2;
waitabit:
	ax = 49;
	goto gotheliframe;
notbeforehdead:
	_cmp(ax, 9);
	if (!flags.z())
		goto gotheliframe;
	_dec(ax);
	_cmp(data.byte(kLastweapon), 1);
	if (!flags.z())
		goto notgunonheli;
	data.byte(kLastweapon) = -1;
	ax = 55;
	goto gotheliframe;
notgunonheli:
	ax = 5;
	_inc(data.byte(kCombatcount));
	_cmp(data.byte(kCombatcount), 20);
	if (!flags.z())
		goto gotheliframe;
	data.byte(kCombatcount) = 0;
	ax = 9;
gotheliframe:
	es.word(bx+3) = ax;
helispeed:
	showGameReel();
	al = data.byte(kMapx);
	es.byte(bx+1) = al;
	ax = es.word(bx+3);
	_cmp(ax, 9);
	if (!flags.c())
		goto notwaitingheli;
	_cmp(data.byte(kCombatcount), 7);
	if (flags.c())
		goto notwaitingheli;
	data.byte(kPointermode) = 2;
	data.word(kWatchingtime) = 0;
	return;
notwaitingheli:
	data.byte(kPointermode) = 0;
	data.word(kWatchingtime) = 2;
	return;
heliwon:
	data.byte(kPointermode) = 0;
}

void DreamGenContext::mugger() {
	STACK_CHECK;
	ax = es.word(bx+3);
	_cmp(ax, 138);
	if (flags.z())
		goto endmugger1;
	_cmp(ax, 176);
	if (flags.z())
		return /* (endmugger2) */;
	_cmp(ax, 2);
	if (!flags.z())
		goto havesetwatch;
	data.word(kWatchingtime) = 175*2;
havesetwatch:
	checkSpeed();
	if (!flags.z())
		goto notmugger;
	_inc(es.word(bx+3));
notmugger:
	showGameReel();
	al = data.byte(kMapx);
	es.byte(bx+1) = al;
	return;
endmugger1:
	push(es);
	push(bx);
	createPanel2();
	showIcon();
	al = 41;
	findPuzText();
	di = 33+20;
	bx = 104;
	dl = 241;
	ah = 0;
	printDirect();
	workToScreen();
	cx = 300;
	hangOn();
	bx = pop();
	es = pop();
	push(es);
	push(bx);
	es.word(bx+3) = 140;
	data.byte(kManspath) = 2;
	data.byte(kFinaldest) = 2;
	findXYFromPath();
	data.byte(kResetmanxy) = 1;
	al = 'W';
	ah = 'E';
	cl = 'T';
	ch = 'A';
	findExObject();
	data.byte(kCommand) = al;
	data.byte(kObjecttype) = 4;
	removeObFromInv();
	al = 'W';
	ah = 'E';
	cl = 'T';
	ch = 'B';
	findExObject();
	data.byte(kCommand) = al;
	data.byte(kObjecttype) = 4;
	removeObFromInv();
	makeMainScreen();
	al = 48;
	bl = 68-32;
	bh = 54+64;
	cx = 70;
	dx = 10;
	setupTimedUse();
	data.byte(kBeenmugged) = 1;
	bx = pop();
	es = pop();
}

void DreamGenContext::businessMan() {
	STACK_CHECK;
	data.byte(kPointermode) = 0;
	data.word(kWatchingtime) = 2;
	ax = es.word(bx+3);
	_cmp(ax, 2);
	if (!flags.z())
		goto notfirstbiz;
	push(ax);
	push(bx);
	push(es);
	al = 49;
	cx = 30;
	dx = 1;
	bl = 68;
	bh = 174;
	setupTimedUse();
	es = pop();
	bx = pop();
	ax = pop();
notfirstbiz:
	_cmp(ax, 95);
	if (flags.z())
		goto buscombatwonend;
	_cmp(ax, 49);
	if (flags.z())
		return /* (buscombatend) */;
	checkSpeed();
	if (!flags.z())
		goto busspeed;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 48);
	if (!flags.z())
		goto notbeforedeadb;
	data.byte(kMandead) = 2;
	goto gotbusframe;
notbeforedeadb:
	_cmp(ax, 15);
	if (!flags.z())
		goto buscombatwon;
	_dec(ax);
	_cmp(data.byte(kLastweapon), 3);
	if (!flags.z())
		goto notshieldonbus;
	data.byte(kLastweapon) = -1;
	data.byte(kCombatcount) = 0;
	ax = 51;
	goto gotbusframe;
notshieldonbus:
	_inc(data.byte(kCombatcount));
	_cmp(data.byte(kCombatcount), 20);
	if (!flags.z())
		goto gotbusframe;
	data.byte(kCombatcount) = 0;
	ax = 15;
	goto gotbusframe;
buscombatwon:
	_cmp(ax, 91);
	if (!flags.z())
		goto gotbusframe;
	push(bx);
	push(es);
	al = 0;
	turnPathOn();
	al = 1;
	turnPathOn();
	al = 2;
	turnPathOn();
	al = 3;
	turnPathOff();
	data.byte(kManspath) = 5;
	data.byte(kFinaldest) = 5;
	findXYFromPath();
	data.byte(kResetmanxy) = 1;
	es = pop();
	bx = pop();
	ax = 92;
	goto gotbusframe;
gotbusframe:
	es.word(bx+3) = ax;
busspeed:
	showGameReel();
	al = data.byte(kMapy);
	es.byte(bx+2) = al;
	ax = es.word(bx+3);
	_cmp(ax, 14);
	if (!flags.z())
		return /* (buscombatend) */;
	data.word(kWatchingtime) = 0;
	data.byte(kPointermode) = 2;
	return;
buscombatwonend:
	data.byte(kPointermode) = 0;
	data.word(kWatchingtime) = 0;
}

void DreamGenContext::poolGuard() {
	STACK_CHECK;
	ax = es.word(bx+3);
	_cmp(ax, 214);
	if (flags.z())
		goto combatover2;
	_cmp(ax, 258);
	if (flags.z())
		goto combatover2;
	_cmp(ax, 185);
	if (flags.z())
		goto combatover1;
	_cmp(ax, 0);
	if (!flags.z())
		goto notfirstpool;
	al = 0;
	turnPathOn();
notfirstpool:
	checkSpeed();
	if (!flags.z())
		goto guardspeed;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 122);
	if (!flags.z())
		goto notendguard1;
	_dec(ax);
	_cmp(data.byte(kLastweapon), 2);
	if (!flags.z())
		goto notaxeonpool;
	data.byte(kLastweapon) = -1;
	ax = 122;
	goto gotguardframe;
notaxeonpool:
	_inc(data.byte(kCombatcount));
	_cmp(data.byte(kCombatcount), 40);
	if (!flags.z())
		goto gotguardframe;
	data.byte(kCombatcount) = 0;
	ax = 195;
	goto gotguardframe;
notendguard1:
	_cmp(ax, 147);
	if (!flags.z())
		goto gotguardframe;
	_dec(ax);
	_cmp(data.byte(kLastweapon), 1);
	if (!flags.z())
		goto notgunonpool;
	data.byte(kLastweapon) = -1;
	ax = 147;
	goto gotguardframe;
notgunonpool:
	_inc(data.byte(kCombatcount));
	_cmp(data.byte(kCombatcount), 40);
	if (!flags.z())
		goto gotguardframe;
	data.byte(kCombatcount) = 0;
	ax = 220;
gotguardframe:
	es.word(bx+3) = ax;
guardspeed:
	showGameReel();
	ax = es.word(bx+3);
	_cmp(ax, 121);
	if (flags.z())
		goto iswaitingpool;
	_cmp(ax, 146);
	if (flags.z())
		goto iswaitingpool;
	data.byte(kPointermode) = 0;
	data.word(kWatchingtime) = 2;
	return;
iswaitingpool:
	data.byte(kPointermode) = 2;
	data.word(kWatchingtime) = 0;
	return;
combatover1:
	data.word(kWatchingtime) = 0;
	data.byte(kPointermode) = 0;
	al = 0;
	turnPathOn();
	al = 1;
	turnPathOff();
	return;
combatover2:
	showGameReel();
	data.word(kWatchingtime) = 2;
	data.byte(kPointermode) = 0;
	_inc(data.byte(kCombatcount));
	_cmp(data.byte(kCombatcount), 100);
	if (flags.c())
		return /* (doneover2) */;
	data.word(kWatchingtime) = 0;
	data.byte(kMandead) = 2;
}

void DreamGenContext::heavy() {
	STACK_CHECK;
	al = es.byte(bx+7);
	_and(al, 127);
	es.byte(bx+7) = al;
	_cmp(es.word(bx+3), 43);
	if (flags.z())
		goto heavywait;
	data.word(kWatchingtime) = 10;
	_cmp(es.word(bx+3), 70);
	if (!flags.z())
		goto notafterhshot;
	_inc(data.byte(kCombatcount));
	_cmp(data.byte(kCombatcount), 80);
	if (!flags.z())
		goto gotheavyframe;
	data.byte(kMandead) = 2;
	goto gotheavyframe;
notafterhshot:
	checkSpeed();
	if (!flags.z())
		goto gotheavyframe;
	_inc(es.word(bx+3));
	goto gotheavyframe;
heavywait:
	_cmp(data.byte(kLastweapon), 1);
	if (!flags.z())
		goto gotheavyframe;
	_cmp(data.byte(kManspath), 5);
	if (!flags.z())
		goto gotheavyframe;
	_cmp(data.byte(kFacing), 4);
	if (!flags.z())
		goto gotheavyframe;
	data.byte(kLastweapon) = -1;
	_inc(es.word(bx+3));
	data.byte(kCombatcount) = 0;
gotheavyframe:
	showGameReel();
	addToPeopleList();
}

void DreamGenContext::bossMan() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto notboss;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 4);
	if (flags.z())
		goto firstdes;
	_cmp(ax, 20);
	if (flags.z())
		goto secdes;
	_cmp(ax, 41);
	if (!flags.z())
		goto gotallboss;
	ax = 0;
	_inc(data.byte(kGunpassflag));
	es.byte(bx+7) = 10;
	goto gotallboss;
firstdes:
	_cmp(data.byte(kGunpassflag), 1);
	if (flags.z())
		goto gotallboss;
	push(ax);
	randomNumber();
	cl = al;
	ax = pop();
	_cmp(cl, 10);
	if (flags.c())
		goto gotallboss;
	ax = 0;
	goto gotallboss;
secdes:
	_cmp(data.byte(kGunpassflag), 1);
	if (flags.z())
		goto gotallboss;
	ax = 0;
gotallboss:
	es.word(bx+3) = ax;
notboss:
	showGameReel();
	addToPeopleList();
	al = es.byte(bx+7);
	_and(al, 128);
	if (flags.z())
		return /* (nottalkedboss) */;
	data.byte(kTalkedtoboss) = 1;
}

void DreamGenContext::keeper() {
	STACK_CHECK;
	_cmp(data.byte(kKeeperflag), 0);
	if (!flags.z())
		goto notwaiting;
	_cmp(data.word(kReeltowatch), 190);
	if (flags.c())
		return /* (waiting) */;
	_inc(data.byte(kKeeperflag));
	ah = es.byte(bx+7);
	_and(ah, 127);
	_cmp(ah, data.byte(kDreamnumber));
	if (flags.z())
		return /* (notdiff) */;
	al = data.byte(kDreamnumber);
	es.byte(bx+7) = al;
	return;
notwaiting:
	addToPeopleList();
	showGameReel();
}

void DreamGenContext::candles1() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto candle1;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 44);
	if (!flags.z())
		goto notendcandle1;
	ax = 39;
notendcandle1:
	es.word(bx+3) = ax;
candle1:
	showGameReel();
}

void DreamGenContext::smallCandle() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto smallcandlef;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 37);
	if (!flags.z())
		goto notendsmallcandle;
	ax = 25;
notendsmallcandle:
	es.word(bx+3) = ax;
smallcandlef:
	showGameReel();
}

void DreamGenContext::candles2() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto candles2fin;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 238);
	if (!flags.z())
		goto gotcandles2;
	ax = 233;
gotcandles2:
	es.word(bx+3) = ax;
candles2fin:
	showGameReel();
}

void DreamGenContext::introMagic2() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto introm2fin;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 216);
	if (!flags.z())
		goto gotintrom2;
	ax = 192;
gotintrom2:
	es.word(bx+3) = ax;
introm2fin:
	showGameReel();
}

void DreamGenContext::introMagic3() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto introm3fin;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 218);
	if (!flags.z())
		goto gotintrom3;
	data.byte(kGetback) = 1;
gotintrom3:
	es.word(bx+3) = ax;
introm3fin:
	showGameReel();
	al = data.byte(kMapx);
	es.byte(bx+1) = al;
}

void DreamGenContext::introMonks1() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto intromonk1fin;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 80);
	if (!flags.z())
		goto notendmonk1;
	_add(data.byte(kMapy), 10);
	data.byte(kNowinnewroom) = 1;
	showGameReel();
	return;
notendmonk1:
	_cmp(ax, 30);
	if (!flags.z())
		goto gotintromonk1;
	_sub(data.byte(kMapy), 10);
	data.byte(kNowinnewroom) = 1;
	ax = 51;
gotintromonk1:
	es.word(bx+3) = ax;
	_cmp(ax, 5);
	if (flags.z())
		goto waitstep;
	_cmp(ax, 15);
	if (flags.z())
		goto waitstep;
	_cmp(ax, 25);
	if (flags.z())
		goto waitstep;
	_cmp(ax, 61);
	if (flags.z())
		goto waitstep;
	_cmp(ax, 71);
	if (flags.z())
		goto waitstep;
	goto intromonk1fin;
waitstep:
	push(es);
	push(bx);
	intro2Text();
	bx = pop();
	es = pop();
	es.byte(bx+6) = -20;
intromonk1fin:
	showGameReel();
	al = data.byte(kMapy);
	es.byte(bx+2) = al;
}

void DreamGenContext::introMonks2() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto intromonk2fin;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 87);
	if (!flags.z())
		goto nottalk1;
	_inc(data.byte(kIntrocount));
	push(es);
	push(bx);
	monks2text();
	bx = pop();
	es = pop();
	_cmp(data.byte(kIntrocount), 19);
	if (!flags.z())
		goto notlasttalk1;
	ax = 87;
	goto gotintromonk2;
notlasttalk1:
	ax = 74;
	goto gotintromonk2;
nottalk1:
	_cmp(ax, 110);
	if (!flags.z())
		goto notraisearm;
	_inc(data.byte(kIntrocount));
	push(es);
	push(bx);
	monks2text();
	bx = pop();
	es = pop();
	_cmp(data.byte(kIntrocount), 35);
	if (!flags.z())
		goto notlastraise;
	ax = 111;
	goto gotintromonk2;
notlastraise:
	ax = 98;
	goto gotintromonk2;
notraisearm:
	_cmp(ax, 176);
	if (!flags.z())
		goto notendmonk2;
	data.byte(kGetback) = 1;
	goto gotintromonk2;
notendmonk2:
	_cmp(ax, 125);
	if (!flags.z())
		goto gotintromonk2;
	ax = 140;
gotintromonk2:
	es.word(bx+3) = ax;
intromonk2fin:
	showGameReel();
}

void DreamGenContext::monkAndRyan() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto notmonkryan;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 83);
	if (!flags.z())
		goto gotmonkryan;
	_inc(data.byte(kIntrocount));
	push(es);
	push(bx);
	textForMonk();
	bx = pop();
	es = pop();
	ax = 77;
	_cmp(data.byte(kIntrocount), 57);
	if (!flags.z())
		goto gotmonkryan;
	data.byte(kGetback) = 1;
	return;
gotmonkryan:
	es.word(bx+3) = ax;
notmonkryan:
	showGameReel();
}

void DreamGenContext::endGameSeq() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto notendseq;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 51);
	if (!flags.z())
		goto gotendseq;
	_cmp(data.byte(kIntrocount), 140);
	if (flags.z())
		goto gotendseq;
	_inc(data.byte(kIntrocount));
	push(es);
	push(bx);
	textForEnd();
	bx = pop();
	es = pop();
	ax = 50;
gotendseq:
	es.word(bx+3) = ax;
	_cmp(ax, 134);
	if (!flags.z())
		goto notfadedown;
	push(es);
	push(bx);
	push(ax);
	fadeScreenDownHalf();
	ax = pop();
	bx = pop();
	es = pop();
	goto notendseq;
notfadedown:
	_cmp(ax, 324);
	if (!flags.z())
		goto notfadeend;
	push(es);
	push(bx);
	push(ax);
	fadeScreenDowns();
	data.byte(kVolumeto) = 7;
	data.byte(kVolumedirection) = 1;
	ax = pop();
	bx = pop();
	es = pop();
notfadeend:
	_cmp(ax, 340);
	if (!flags.z())
		goto notendseq;
	data.byte(kGetback) = 1;
notendseq:
	showGameReel();
	al = data.byte(kMapy);
	es.byte(bx+2) = al;
	ax = es.word(bx+3);
	_cmp(ax, 145);
	if (!flags.z())
		return /* (notendcreds) */;
	es.word(bx+3) = 146;
	rollEndCredits();
}

void DreamGenContext::rollEndCredits() {
	STACK_CHECK;
	al = 16;
	ah = 255;
	playChannel0();
	data.byte(kVolume) = 7;
	data.byte(kVolumeto) = 0;
	data.byte(kVolumedirection) = -1;
	cl = 160;
	ch = 160;
	di = 75;
	bx = 20;
	ds = data.word(kMapstore);
	si = 0;
	multiGet();
	es = data.word(kTextfile1);
	si = 3*2;
	ax = es.word(si);
	si = ax;
	_add(si, (66*2));
	cx = 254;
endcredits1:
	push(cx);
	bx = 10;
	cx = data.word(kLinespacing);
endcredits2:
	push(cx);
	push(si);
	push(di);
	push(es);
	push(bx);
	vSync();
	cl = 160;
	ch = 160;
	di = 75;
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
onelot:
	push(cx);
	di = 75;
	dx = 161;
	ax = 0;
	printDirect();
	_add(bx, data.word(kLinespacing));
	cx = pop();
	if (--cx)
		goto onelot;
	vSync();
	cl = 160;
	ch = 160;
	di = 75;
	bx = 20;
	multiDump();
	bx = pop();
	es = pop();
	di = pop();
	si = pop();
	cx = pop();
	_dec(bx);
	if (--cx)
		goto endcredits2;
	cx = pop();
looknext:
	al = es.byte(si);
	_inc(si);
	_cmp(al, ':');
	if (flags.z())
		goto gotnext;
	_cmp(al, 0);
	if (flags.z())
		goto gotnext;
	goto looknext;
gotnext:
	if (--cx)
		goto endcredits1;
	cx = 100;
	hangOn();
	panelToMap();
	fadeScreenUpHalf();
}

void DreamGenContext::priest() {
	STACK_CHECK;
	_cmp(es.word(bx+3), 8);
	if (flags.z())
		return /* (priestspoken) */;
	data.byte(kPointermode) = 0;
	data.word(kWatchingtime) = 2;
	checkSpeed();
	if (!flags.z())
		return /* (priestwait) */;
	_inc(es.word(bx+3));
	push(es);
	push(bx);
	priestText();
	bx = pop();
	es = pop();
}

void DreamGenContext::priestText() {
	STACK_CHECK;
	_cmp(es.word(bx+3), 2);
	if (flags.c())
		return /* (nopriesttext) */;
	_cmp(es.word(bx+3), 7);
	if (!flags.c())
		return /* (nopriesttext) */;
	al = es.byte(bx+3);
	_and(al, 1);
	if (!flags.z())
		return /* (nopriesttext) */;
	al = es.byte(bx+3);
	_shr(al, 1);
	_add(al, 50);
	bl = 72;
	bh = 80;
	cx = 54;
	dx = 1;
	setupTimedUse();
}

void DreamGenContext::advisor() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto noadvisor;
	goto noadvisor;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 123);
	if (!flags.z())
		goto notendadvis;
	ax = 106;
	goto gotadvframe;
notendadvis:
	_cmp(ax, 108);
	if (!flags.z())
		goto gotadvframe;
	push(ax);
	randomNumber();
	cl = al;
	ax = pop();
	_cmp(cl, 3);
	if (flags.c())
		goto gotadvframe;
	ax = 106;
gotadvframe:
	es.word(bx+3) = ax;
noadvisor:
	showGameReel();
	addToPeopleList();
}

void DreamGenContext::copper() {
	STACK_CHECK;
	checkSpeed();
	if (!flags.z())
		goto nocopper;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 94);
	if (!flags.z())
		goto notendcopper;
	ax = 64;
	goto gotcopframe;
notendcopper:
	_cmp(ax, 81);
	if (flags.z())
		goto mightwait;
	_cmp(ax, 66);
	if (!flags.z())
		goto gotcopframe;
mightwait:
	push(ax);
	randomNumber();
	cl = al;
	ax = pop();
	_cmp(cl, 7);
	if (flags.c())
		goto gotcopframe;
	_dec(ax);
gotcopframe:
	es.word(bx+3) = ax;
nocopper:
	showGameReel();
	addToPeopleList();
}

void DreamGenContext::train() {
	STACK_CHECK;
	return;
	ax = es.word(bx+3);
	_cmp(ax, 21);
	if (!flags.c())
		goto notrainyet;
	_inc(ax);
	goto gottrainframe;
notrainyet:
	randomNumber();
	_cmp(al, 253);
	if (flags.c())
		return /* (notrainatall) */;
	_cmp(data.byte(kManspath), 5);
	if (!flags.z())
		return /* (notrainatall) */;
	_cmp(data.byte(kFinaldest), 5);
	if (!flags.z())
		return /* (notrainatall) */;
	ax = 5;
gottrainframe:
	es.word(bx+3) = ax;
	showGameReel();
}

void DreamGenContext::checkForExit() {
	STACK_CHECK;
	cl = data.byte(kRyanx);
	_add(cl, 12);
	ch = data.byte(kRyany);
	_add(ch, 12);
	checkOne();
	data.byte(kLastflag) = cl;
	data.byte(kLastflagex) = ch;
	data.byte(kFlagx) = dl;
	data.byte(kFlagy) = dh;
	al = data.byte(kLastflag);
	_test(al, 64);
	if (flags.z())
		goto notnewdirect;
	al = data.byte(kLastflagex);
	data.byte(kAutolocation) = al;
	return;
notnewdirect:
	_test(al, 32);
	if (flags.z())
		goto notleave;
	push(es);
	push(bx);
	_cmp(data.byte(kReallocation), 2);
	if (!flags.z())
		goto notlouis;
	bl = 0;
	push(bx);
	al = 'W';
	ah = 'E';
	cl = 'T';
	ch = 'A';
	isRyanHolding();
	bx = pop();
	if (flags.z())
		goto noshoe1;
	_inc(bl);
noshoe1:
	push(bx);
	al = 'W';
	ah = 'E';
	cl = 'T';
	ch = 'B';
	isRyanHolding();
	bx = pop();
	if (flags.z())
		goto noshoe2;
	_inc(bl);
noshoe2:
	_cmp(bl, 2);
	if (flags.z())
		goto notlouis;
	al = 42;
	_cmp(bl, 0);
	if (flags.z())
		goto notravmessage;
	_inc(al);
notravmessage:
	cx = 80;
	dx = 10;
	bl = 68;
	bh = 64;
	setupTimedUse();
	al = data.byte(kFacing);
	_add(al, 4);
	_and(al, 7);
	data.byte(kTurntoface) = al;
	bx = pop();
	es = pop();
	return;
notlouis:
	bx = pop();
	es = pop();
	data.byte(kNeedtotravel) = 1;
	return;
notleave:
	_test(al, 4);
	if (flags.z())
		goto notaleft;
	adjustLeft();
	return;
notaleft:
	_test(al, 2);
	if (flags.z())
		goto notaright;
	adjustRight();
	return;
notaright:
	_test(al, 8);
	if (flags.z())
		goto notadown;
	adjustDown();
	return;
notadown:
	_test(al, 16);
	if (flags.z())
		return /* (notanup) */;
	adjustUp();
}

void DreamGenContext::adjustDown() {
	STACK_CHECK;
	push(es);
	push(bx);
	_add(data.byte(kMapy), 10);
	al = data.byte(kLastflagex);
	cl = 16;
	_mul(cl);
	es.byte(bx+11) = al;
	data.byte(kNowinnewroom) = 1;
	bx = pop();
	es = pop();
}

void DreamGenContext::adjustUp() {
	STACK_CHECK;
	push(es);
	push(bx);
	_sub(data.byte(kMapy), 10);
	al = data.byte(kLastflagex);
	cl = 16;
	_mul(cl);
	es.byte(bx+11) = al;
	data.byte(kNowinnewroom) = 1;
	bx = pop();
	es = pop();
}

void DreamGenContext::adjustLeft() {
	STACK_CHECK;
	push(es);
	push(bx);
	data.byte(kLastflag) = 0;
	_sub(data.byte(kMapx), 11);
	al = data.byte(kLastflagex);
	cl = 16;
	_mul(cl);
	es.byte(bx+10) = al;
	data.byte(kNowinnewroom) = 1;
	bx = pop();
	es = pop();
}

void DreamGenContext::adjustRight() {
	STACK_CHECK;
	push(es);
	push(bx);
	_add(data.byte(kMapx), 11);
	al = data.byte(kLastflagex);
	cl = 16;
	_mul(cl);
	_sub(al, 2);
	es.byte(bx+10) = al;
	data.byte(kNowinnewroom) = 1;
	bx = pop();
	es = pop();
}

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

void DreamGenContext::delEverything() {
	STACK_CHECK;
	al = data.byte(kMapysize);
	ah = 0;
	_add(ax, data.word(kMapoffsety));
	_cmp(ax, 182);
	if (!flags.c())
		goto bigroom;
	mapToPanel();
	return;
bigroom:
	_sub(data.byte(kMapysize), 8);
	mapToPanel();
	_add(data.byte(kMapysize), 8);
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

void DreamGenContext::doFade() {
	STACK_CHECK;
	_cmp(data.byte(kFadedirection), 0);
	if (flags.z())
		return /* (finishfade) */;
	cl = data.byte(kNumtofade);
	ch = 0;
	al = data.byte(kColourpos);
	ah = 0;
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3));
	_add(si, ax);
	_add(si, ax);
	_add(si, ax);
	showGroup();
	al = data.byte(kNumtofade);
	_add(al, data.byte(kColourpos));
	data.byte(kColourpos) = al;
	_cmp(al, 0);
	if (!flags.z())
		return /* (finishfade) */;
	fadeCalculation();
}

void DreamGenContext::clearPalette() {
	STACK_CHECK;
	data.byte(kFadedirection) = 0;
	clearStartPal();
	dumpCurrent();
}

void DreamGenContext::fadeToWhite() {
	STACK_CHECK;
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
	cx = 768;
	al = 63;
	_stosb(cx, true);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
	al = 0;
	_stosb(3);
	palToStartPal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
}

void DreamGenContext::fadeFromWhite() {
	STACK_CHECK;
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3));
	cx = 768;
	al = 63;
	_stosb(cx, true);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3));
	al = 0;
	_stosb(3);
	palToEndPal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
}

void DreamGenContext::fadeScreenDownHalf() {
	STACK_CHECK;
	palToStartPal();
	palToEndPal();
	cx = 768;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
halfend:
	al = es.byte(bx);
	_shr(al, 1);
	es.byte(bx) = al;
	_inc(bx);
	if (--cx)
		goto halfend;
	ds = data.word(kBuffers);
	es = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3))+(56*3);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768)+(56*3);
	cx = 3*5;
	_movsb(cx, true);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3))+(77*3);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768)+(77*3);
	cx = 3*2;
	_movsb(cx, true);
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 31;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 32;
}

void DreamGenContext::showGun() {
	STACK_CHECK;
	data.byte(kAddtored) = 0;
	data.byte(kAddtogreen) = 0;
	data.byte(kAddtoblue) = 0;
	palToStartPal();
	palToEndPal();
	greyscaleSum();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	cx = 130;
	hangOn();
	endPalToStart();
	clearEndPal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	cx = 200;
	hangOn();
	data.byte(kRoomssample) = 34;
	loadRoomsSample();
	data.byte(kVolume) = 0;
	dx = 1061;
	loadIntoTemp();
	createPanel2();
	ds = data.word(kTempgraphics);
	al = 0;
	ah = 0;
	di = 100;
	bx = 4;
	showFrame();
	ds = data.word(kTempgraphics);
	al = 1;
	ah = 0;
	di = 158;
	bx = 106;
	showFrame();
	workToScreen();
	getRidOfTemp();
	fadeScreenUp();
	cx = 160;
	hangOn();
	al = 12;
	ah = 0;
	playChannel0();
	dx = 1035;
	loadTempText();
	rollEndCredits2();
	getRidOfTempText();
}

void DreamGenContext::rollEndCredits2() {
	STACK_CHECK;
	rollEm();
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

void DreamGenContext::greyscaleSum() {
	STACK_CHECK;
	es = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
	cx = 256;
greysumloop1:
	push(cx);
	bx = 0;
	al = es.byte(si);
	ah = 0;
	cx = 20;
	_mul(cx);
	_add(bx, ax);
	al = es.byte(si+1);
	ah = 0;
	cx = 59;
	_mul(cx);
	_add(bx, ax);
	al = es.byte(si+2);
	ah = 0;
	cx = 11;
	_mul(cx);
	_add(bx, ax);
	al = -1;
greysumloop2:
	_inc(al);
	_sub(bx, 100);
	if (!flags.c())
		goto greysumloop2;
	bl = al;
	al = bl;
	ah = data.byte(kAddtored);
	_cmp(al, 0);
	_add(al, ah);
	_stosb();
	ah = data.byte(kAddtogreen);
	al = bl;
	_cmp(al, 0);
	if (flags.z())
		goto noaddg;
	_add(al, ah);
noaddg:
	_stosb();
	ah = data.byte(kAddtoblue);
	al = bl;
	_cmp(al, 0);
	if (flags.z())
		goto noaddb;
	_add(al, ah);
noaddb:
	_stosb();
	_add(si, 3);
	cx = pop();
	if (--cx)
		goto greysumloop1;
}

void DreamGenContext::allPalette() {
	STACK_CHECK;
	es = data.word(kBuffers);
	ds = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3));
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768);
	cx = 768/2;
	_movsw(cx, true);
	dumpCurrent();
}

void DreamGenContext::dumpCurrent() {
	STACK_CHECK;
	si = (0+(228*13)+32+60+(32*32)+(11*10*3));
	ds = data.word(kBuffers);
	vSync();
	al = 0;
	cx = 128;
	showGroup();
	vSync();
	al = 128;
	cx = 128;
	showGroup();
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

void DreamGenContext::endGame() {
	STACK_CHECK;
	dx = 1035;
	loadTempText();
	monkSpeaking();
	gettingShot();
	getRidOfTempText();
	data.byte(kVolumeto) = 7;
	data.byte(kVolumedirection) = 1;
	cx = 200;
	hangOn();
}

void DreamGenContext::monkSpeaking() {
	STACK_CHECK;
	data.byte(kRoomssample) = 35;
	loadRoomsSample();
	dx = 1074;
	loadIntoTemp();
	clearWork();
	showMonk();
	workToScreen();
	data.byte(kVolume) = 7;
	data.byte(kVolumedirection) = -1;
	data.byte(kVolumeto) = 5;
	al = 12;
	ah = 255;
	playChannel0();
	fadeScreenUps();
	cx = 300;
	hangOn();
	al = 40;
loadspeech2:
	push(ax);
	dl = 'T';
	dh = 83;
	cl = 'T';
	ah = 0;
	loadSpeech();
	al = 50+12;
	playChannel1();
notloadspeech2:
	vSync();
	_cmp(data.byte(kCh1playing), 255);
	if (!flags.z())
		goto notloadspeech2;
	ax = pop();
	_inc(al);
	_cmp(al, 48);
	if (!flags.z())
		goto loadspeech2;
	data.byte(kVolumedirection) = 1;
	data.byte(kVolumeto) = 7;
	fadeScreenDowns();
	cx = 300;
	hangOn();
	getRidOfTemp();
}

void DreamGenContext::showMonk() {
	STACK_CHECK;
	al = 0;
	ah = 128;
	di = 160;
	bx = 72;
	ds = data.word(kTempgraphics);
	showFrame();
}

void DreamGenContext::runIntroSeq() {
	STACK_CHECK;
	data.byte(kGetback) = 0;
moreintroseq:
	vSync();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto earlyendrun;
	spriteUpdate();
	vSync();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto earlyendrun;
	delEverything();
	printSprites();
	reelsOnScreen();
	afterIntroRoom();
	useTimedText();
	vSync();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto earlyendrun;
	dumpMap();
	dumpTimedText();
	vSync();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto earlyendrun;
	_cmp(data.byte(kGetback), 1);
	if (!flags.z())
		goto moreintroseq;
	return;
earlyendrun:
	getRidOfTempText();
	clearBeforeLoad();
}

void DreamGenContext::runEndSeq() {
	STACK_CHECK;
	atmospheres();
	data.byte(kGetback) = 0;
moreendseq:
	vSync();
	spriteUpdate();
	vSync();
	delEverything();
	printSprites();
	reelsOnScreen();
	afterIntroRoom();
	useTimedText();
	vSync();
	dumpMap();
	dumpTimedText();
	vSync();
	_cmp(data.byte(kGetback), 1);
	if (!flags.z())
		goto moreendseq;
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

void DreamGenContext::makeMainScreen() {
	STACK_CHECK;
	createPanel();
	data.byte(kNewobs) = 1;
	drawFloor();
	spriteUpdate();
	printSprites();
	reelsOnScreen();
	showIcon();
	getUnderZoom();
	underTextLine();
	data.byte(kCommandtype) = 255;
	animPointer();
	workToScreenM();
	data.byte(kCommandtype) = 200;
	data.byte(kManisoffscreen) = 0;
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

void DreamGenContext::openInv() {
	STACK_CHECK;
	data.byte(kInvopen) = 1;
	al = 61;
	di = (80);
	bx = (58)-10;
	dl = 240;
	printMessage();
	fillRyan();
	data.byte(kCommandtype) = 255;
}

void DreamGenContext::openOb() {
	STACK_CHECK;
	al = data.byte(kOpenedob);
	ah = data.byte(kOpenedtype);
	di = offset_commandline;
	copyName();
	di = (80);
	bx = (58)+86;
	al = 62;
	dl = 240;
	printMessage();
	di = data.word(kLastxpos);
	_add(di, 5);
	bx = (58)+86;
	es = cs;
	si = offset_commandline;
	dl = 220;
	al = 0;
	ah = 0;
	printDirect();
	fillOpen();
	getOpenedSize();
	al = ah;
	ah = 0;
	cx = (44);
	_mul(cx);
	_add(ax, (80));
	bx = offset_openchangesize;
	cs.word(bx) = ax;
}

void DreamGenContext::describeOb() {
	STACK_CHECK;
	getObTextStart();
	di = 33;
	bx = 92;
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto notsetd;
	_cmp(data.byte(kObjecttype), 1);
	if (!flags.z())
		goto notsetd;
	bx = 82;
notsetd:
	dl = 241;
	ah = 16;
	data.word(kCharshift) = 91+91;
	printDirect();
	data.word(kCharshift) = 0;
	di = 36;
	bx = 104;
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto notsetd2;
	_cmp(data.byte(kObjecttype), 1);
	if (!flags.z())
		goto notsetd2;
	bx = 94;
notsetd2:
	dl = 241;
	ah = 0;
	printDirect();
	push(bx);
	obsThatDoThings();
	bx = pop();
	additionalText();
}

void DreamGenContext::additionalText() {
	STACK_CHECK;
	_add(bx, 10);
	push(bx);
	al = data.byte(kCommand);
	ah = data.byte(kObjecttype);
	cl = 'C';
	ch = 'U';
	dl = 'P';
	dh = 'E';
	compare();
	if (flags.z())
		goto emptycup;
	al = data.byte(kCommand);
	ah = data.byte(kObjecttype);
	cl = 'C';
	ch = 'U';
	dl = 'P';
	dh = 'F';
	compare();
	if (flags.z())
		goto fullcup;
	bx = pop();
	return;
emptycup:
	al = 40;
	findPuzText();
	bx = pop();
	di = 36;
	dl = 241;
	ah = 0;
	printDirect();
	return;
fullcup:
	al = 39;
	findPuzText();
	bx = pop();
	di = 36;
	dl = 241;
	ah = 0;
	printDirect();
}

void DreamGenContext::obsThatDoThings() {
	STACK_CHECK;
	al = data.byte(kCommand);
	ah = data.byte(kObjecttype);
	cl = 'M';
	ch = 'E';
	dl = 'M';
	dh = 'B';
	compare();
	if (!flags.z())
		return /* (notlouiscard) */;
	al = 4;
	getLocation();
	_cmp(al, 1);
	if (flags.z())
		return /* (seencard) */;
	al = 4;
	setLocation();
	lookAtCard();
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

void DreamGenContext::setPickup() {
	STACK_CHECK;
	_cmp(data.byte(kObjecttype), 1);
	if (flags.z())
		goto cantpick;
	_cmp(data.byte(kObjecttype), 3);
	if (flags.z())
		goto cantpick;
	getAnyAd();
	al = es.byte(bx+2);
	_cmp(al, 4);
	if (!flags.z())
		goto canpick;
cantpick:
	blank();
	return;
canpick:
	_cmp(data.byte(kCommandtype), 209);
	if (flags.z())
		goto alreadysp;
	data.byte(kCommandtype) = 209;
	bl = data.byte(kCommand);
	bh = data.byte(kObjecttype);
	al = 33;
	commandWithOb();
alreadysp:
	ax = data.word(kMousebutton);
	_cmp(ax, 1);
	if (!flags.z())
		return /* (nosetpick) */;
	_cmp(ax, data.word(kOldbutton));
	if (!flags.z())
		goto dosetpick;
	return;
dosetpick:
	createPanel();
	showPanel();
	showMan();
	showExit();
	examIcon();
	data.byte(kPickup) = 1;
	data.byte(kInvopen) = 2;
	_cmp(data.byte(kObjecttype), 4);
	if (flags.z())
		goto pickupexob;
	al = data.byte(kCommand);
	data.byte(kItemframe) = al;
	data.byte(kOpenedob) = 255;
	transferToEx();
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = 4;
	getEitherAd();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
	openInv();
	workToScreenM();
	return;
pickupexob:
	al = data.byte(kCommand);
	data.byte(kItemframe) = al;
	data.byte(kOpenedob) = 255;
	openInv();
	workToScreenM();
}

void DreamGenContext::examineInventory() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 249);
	if (flags.z())
		goto alreadyexinv;
	data.byte(kCommandtype) = 249;
	al = 32;
	commandOnly();
alreadyexinv:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (!flags.z())
		goto doexinv;
	return;
doexinv:
	createPanel();
	showPanel();
	showMan();
	showExit();
	examIcon();
	data.byte(kPickup) = 0;
	data.byte(kInvopen) = 2;
	openInv();
	workToScreenM();
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

void DreamGenContext::reExFromOpen() {
	STACK_CHECK;
	return;
	findOpenPos();
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

void DreamGenContext::getOpenedSize() {
	STACK_CHECK;
	_cmp(data.byte(kOpenedtype), 4);
	if (flags.z())
		goto isex2;
	_cmp(data.byte(kOpenedtype), 2);
	if (flags.z())
		goto isfree2;
	al = data.byte(kOpenedob);
	getSetAd();
	ax = es.word(bx+3);
	return;
isfree2:
	al = data.byte(kOpenedob);
	getFreeAd();
	ax = es.word(bx+7);
	return;
isex2:
	al = data.byte(kOpenedob);
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

void DreamGenContext::findInvPos() {
	STACK_CHECK;
	cx = data.word(kMousex);
	_sub(cx, (80));
	bx = -1;
findinv1:
	_inc(bx);
	_sub(cx, (44));
	if (!flags.c())
		goto findinv1;
	cx = data.word(kMousey);
	_sub(cx, (58));
	_sub(bx, 5);
findinv2:
	_add(bx, 5);
	_sub(cx, (44));
	if (!flags.c())
		goto findinv2;
	al = data.byte(kRyanpage);
	ah = 0;
	cx = 10;
	_mul(cx);
	_add(bx, ax);
	al = bl;
	data.byte(kLastinvpos) = al;
	_add(bx, bx);
	es = data.word(kBuffers);
	_add(bx, (0+(228*13)+32));
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

void DreamGenContext::dropError() {
	STACK_CHECK;
	data.byte(kCommandtype) = 255;
	delPointer();
	di = 76;
	bx = 21;
	al = 56;
	dl = 240;
	printMessage();
	workToScreenM();
	cx = 50;
	hangOnP();
	showPanel();
	showMan();
	examIcon();
	data.byte(kCommandtype) = 255;
	workToScreenM();
}

void DreamGenContext::cantDrop() {
	STACK_CHECK;
	data.byte(kCommandtype) = 255;
	delPointer();
	di = 76;
	bx = 21;
	al = 24;
	dl = 240;
	printMessage();
	workToScreenM();
	cx = 50;
	hangOnP();
	showPanel();
	showMan();
	examIcon();
	data.byte(kCommandtype) = 255;
	workToScreenM();
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

void DreamGenContext::errorMessage1() {
	STACK_CHECK;
	delPointer();
	di = 76;
	bx = 21;
	al = 58;
	dl = 240;
	printMessage();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	cx = 50;
	hangOnP();
	showPanel();
	showMan();
	examIcon();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::errorMessage2() {
	STACK_CHECK;
	data.byte(kCommandtype) = 255;
	delPointer();
	di = 76;
	bx = 21;
	al = 59;
	dl = 240;
	printMessage();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	cx = 50;
	hangOnP();
	showPanel();
	showMan();
	examIcon();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::errorMessage3() {
	STACK_CHECK;
	delPointer();
	di = 76;
	bx = 21;
	al = 60;
	dl = 240;
	printMessage();
	workToScreenM();
	cx = 50;
	hangOnP();
	showPanel();
	showMan();
	examIcon();
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

void DreamGenContext::getBack1() {
	STACK_CHECK;
	_cmp(data.byte(kPickup), 0);
	if (flags.z())
		goto notgotobject;
	blank();
	return;
notgotobject:
	_cmp(data.byte(kCommandtype), 202);
	if (flags.z())
		goto alreadyget;
	data.byte(kCommandtype) = 202;
	al = 26;
	commandOnly();
alreadyget:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (nogetback) */;
	_and(ax, 1);
	if (!flags.z())
		goto dogetback;
	return;
dogetback:
	data.byte(kGetback) = 1;
	data.byte(kPickup) = 0;
}

void DreamGenContext::talk() {
	STACK_CHECK;
	data.byte(kTalkpos) = 0;
	data.byte(kInmaparea) = 0;
	al = data.byte(kCommand);
	data.byte(kCharacter) = al;
	createPanel();
	showPanel();
	showMan();
	showExit();
	underTextLine();
	convIcons();
	startTalk();
	data.byte(kCommandtype) = 255;
	readMouse();
	showPointer();
	workToScreen();
waittalk:
	delPointer();
	readMouse();
	animPointer();
	showPointer();
	vSync();
	dumpPointer();
	dumpTextLine();
	data.byte(kGetback) = 0;
	bx = offset_talklist;
	checkCoords();
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		goto finishtalk;
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto waittalk;
finishtalk:
	bx = data.word(kPersondata);
	es = cs;
	_cmp(data.byte(kTalkpos), 4);
	if (flags.c())
		goto notnexttalk;
	al = es.byte(bx+7);
	_or(al, 128);
	es.byte(bx+7) = al;
notnexttalk:
	redrawMainScrn();
	workToScreenM();
	_cmp(data.byte(kSpeechloaded), 1);
	if (!flags.z())
		return /* (nospeech) */;
	cancelCh1();
	data.byte(kVolumedirection) = -1;
	data.byte(kVolumeto) = 0;
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

void DreamGenContext::moreTalk() {
	STACK_CHECK;
	_cmp(data.byte(kTalkmode), 0);
	if (flags.z())
		goto canmore;
	redes();
	return;
canmore:
	_cmp(data.byte(kCommandtype), 215);
	if (flags.z())
		goto alreadymore;
	data.byte(kCommandtype) = 215;
	al = 49;
	commandOnly();
alreadymore:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (nomore) */;
	_and(ax, 1);
	if (!flags.z())
		goto domoretalk;
	return;
domoretalk:
	data.byte(kTalkmode) = 2;
	data.byte(kTalkpos) = 4;
	_cmp(data.byte(kCharacter), 100);
	if (flags.c())
		goto notsecondpart;
	data.byte(kTalkpos) = 48;
notsecondpart:
	doSomeTalk();
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

void DreamGenContext::hangOnPQ() {
	STACK_CHECK;
	data.byte(kGetback) = 0;
	bx = 0;
hangloopq:
	push(cx);
	push(bx);
	delPointer();
	readMouse();
	animPointer();
	showPointer();
	vSync();
	dumpPointer();
	dumpTextLine();
	bx = offset_quitlist;
	checkCoords();
	bx = pop();
	cx = pop();
	_cmp(data.byte(kGetback), 1);
	if (flags.z())
		goto quitconv;
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		goto quitconv;
	_cmp(data.byte(kSpeechloaded), 1);
	if (!flags.z())
		goto notspeaking;
	_cmp(data.byte(kCh1playing), 255);
	if (!flags.z())
		goto notspeaking;
	_inc(bx);
	_cmp(bx, 40);
	if (flags.z())
		goto finishconv;
notspeaking:
	_cmp(data.word(kMousebutton), 0);
	if (flags.z())
		goto hangloopq;
	_cmp(data.word(kOldbutton), 0);
	if (!flags.z())
		goto hangloopq;
finishconv:
	delPointer();
	data.byte(kPointermode) = 0;
	flags._c = false;
 	return;
quitconv:
	delPointer();
	data.byte(kPointermode) = 0;
	cancelCh1();
	flags._c = true;
 }

void DreamGenContext::redes() {
	STACK_CHECK;
	_cmp(data.byte(kCh1playing), 255);
	if (!flags.z())
		goto cantredes;
	_cmp(data.byte(kTalkmode), 2);
	if (flags.z())
		goto canredes;
cantredes:
	blank();
	return;
canredes:
	_cmp(data.byte(kCommandtype), 217);
	if (flags.z())
		goto alreadyreds;
	data.byte(kCommandtype) = 217;
	al = 50;
	commandOnly();
alreadyreds:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (!flags.z())
		goto doredes;
	return;
doredes:
	delPointer();
	createPanel();
	showPanel();
	showMan();
	showExit();
	convIcons();
	startTalk();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::newPlace() {
	STACK_CHECK;
	_cmp(data.byte(kNeedtotravel), 1);
	if (flags.z())
		goto istravel;
	_cmp(data.byte(kAutolocation), -1);
	if (!flags.z())
		goto isautoloc;
	return;
isautoloc:
	al = data.byte(kAutolocation);
	data.byte(kNewlocation) = al;
	data.byte(kAutolocation) = -1;
	return;
istravel:
	data.byte(kNeedtotravel) = 0;
	selectLocation();
}

void DreamGenContext::selectLocation() {
	STACK_CHECK;
	data.byte(kInmaparea) = 0;
	clearBeforeLoad();
	data.byte(kGetback) = 0;
	data.byte(kPointerframe) = 22;
	readCityPic();
	showCity();
	getRidOfTemp();
	readDestIcon();
	loadTravelText();
	showPanel();
	showMan();
	showArrows();
	showExit();
	locationPic();
	underTextLine();
	data.byte(kCommandtype) = 255;
	readMouse();
	data.byte(kPointerframe) = 0;
	showPointer();
	workToScreen();
	al = 9;
	ah = 255;
	playChannel0();
	data.byte(kNewlocation) = 255;
select:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		goto quittravel;
	delPointer();
	readMouse();
	showPointer();
	vSync();
	dumpPointer();
	dumpTextLine();
	_cmp(data.byte(kGetback), 1);
	if (flags.z())
		goto quittravel;
	bx = offset_destlist;
	checkCoords();
	_cmp(data.byte(kNewlocation), 255);
	if (flags.z())
		goto select;
	al = data.byte(kNewlocation);
	_cmp(al, data.byte(kLocation));
	if (flags.z())
		goto quittravel;
	getRidOfTemp();
	getRidOfTemp2();
	getRidOfTemp3();
	es = data.word(kTraveltext);
	deallocateMem();
	return;
quittravel:
	al = data.byte(kReallocation);
	data.byte(kNewlocation) = al;
	data.byte(kGetback) = 0;
	getRidOfTemp();
	getRidOfTemp2();
	getRidOfTemp3();
	es = data.word(kTraveltext);
	deallocateMem();
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

void DreamGenContext::getUnderCentre() {
	STACK_CHECK;
	di = 58;
	bx = 72;
	ds = data.word(kMapstore);
	si = 0;
	cl = 254;
	ch = 110;
	multiGet();
}

void DreamGenContext::putUnderCentre() {
	STACK_CHECK;
	di = 58;
	bx = 72;
	ds = data.word(kMapstore);
	si = 0;
	cl = 254;
	ch = 110;
	multiPut();
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
	si = 1723;
	_add(si, ax);
	cl = es.byte(si);
	ax = pop();
	push(cx);
	dx = data;
	es = dx;
	si = 1739;
	_add(si, ax);
	ax = pop();
}

void DreamGenContext::showArrows() {
	STACK_CHECK;
	di = 116-12;
	bx = 16;
	ds = data.word(kTempgraphics);
	al = 0;
	ah = 0;
	showFrame();
	di = 226+12;
	bx = 16;
	ds = data.word(kTempgraphics);
	al = 1;
	ah = 0;
	showFrame();
	di = 280;
	bx = 14;
	ds = data.word(kTempgraphics);
	al = 2;
	ah = 0;
	showFrame();
}

void DreamGenContext::nextDest() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 218);
	if (flags.z())
		goto alreadydu;
	data.byte(kCommandtype) = 218;
	al = 28;
	commandOnly();
alreadydu:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (flags.z())
		return /* (nodu) */;
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (nodu) */;
searchdestup:
	_inc(data.byte(kDestpos));
	_cmp(data.byte(kDestpos), 15);
	if (!flags.z())
		goto notlastdest;
	data.byte(kDestpos) = 0;
notlastdest:
	getDestInfo();
	_cmp(al, 0);
	if (flags.z())
		goto searchdestup;
	data.byte(kNewtextline) = 1;
	delTextLine();
	delPointer();
	showPanel();
	showMan();
	showArrows();
	locationPic();
	underTextLine();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::lastDest() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 219);
	if (flags.z())
		goto alreadydd;
	data.byte(kCommandtype) = 219;
	al = 29;
	commandOnly();
alreadydd:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (flags.z())
		return /* (nodd) */;
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (nodd) */;
searchdestdown:
	_dec(data.byte(kDestpos));
	_cmp(data.byte(kDestpos), -1);
	if (!flags.z())
		goto notfirstdest;
	data.byte(kDestpos) = 15;
notfirstdest:
	getDestInfo();
	_cmp(al, 0);
	if (flags.z())
		goto searchdestdown;
	data.byte(kNewtextline) = 1;
	delTextLine();
	delPointer();
	showPanel();
	showMan();
	showArrows();
	locationPic();
	underTextLine();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::destSelect() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 222);
	if (flags.z())
		goto alreadytrav;
	data.byte(kCommandtype) = 222;
	al = 30;
	commandOnly();
alreadytrav:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (flags.z())
		return /* (notrav) */;
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notrav) */;
	getDestInfo();
	al = data.byte(kDestpos);
	data.byte(kNewlocation) = al;
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
	_add(bx, 1723);
	es.byte(bx) = 0;
}

void DreamGenContext::printOuterMon() {
	STACK_CHECK;
	di = 40;
	bx = 32;
	ds = data.word(kTempgraphics);
	al = 1;
	ah = 0;
	showFrame();
	di = 264;
	bx = 32;
	ds = data.word(kTempgraphics);
	al = 2;
	ah = 0;
	showFrame();
	di = 40;
	bx = 12;
	ds = data.word(kTempgraphics);
	al = 3;
	ah = 0;
	showFrame();
	di = 40;
	bx = 164;
	ds = data.word(kTempgraphics);
	al = 4;
	ah = 0;
	showFrame();
}

void DreamGenContext::lookInInterface() {
	STACK_CHECK;
	al = 'I';
	ah = 'N';
	cl = 'T';
	ch = 'F';
	findSetObject();
	ah = 1;
	checkInside();
	_cmp(cl, (114));
	if (flags.z())
		goto emptyinterface;
	al = es.byte(bx+15);
	_inc(al);
	return;
emptyinterface:
	al = 0;
}

void DreamGenContext::lockLightOn() {
	STACK_CHECK;
	di = 56;
	bx = 182;
	ds = data.word(kTempgraphics);
	al = 10;
	ah = 0;
	push(di);
	push(bx);
	showFrame();
	bx = pop();
	di = pop();
	cl = 12;
	ch = 8;
	multiDump();
}

void DreamGenContext::lockLightOff() {
	STACK_CHECK;
	di = 56;
	bx = 182;
	ds = data.word(kTempgraphics);
	al = 9;
	ah = 0;
	push(di);
	push(bx);
	showFrame();
	bx = pop();
	di = pop();
	cl = 12;
	ch = 8;
	multiDump();
}

void DreamGenContext::makeCaps() {
	STACK_CHECK;
	_cmp(al, 'a');
	if (flags.c())
		return /* (notupperc) */;
	_sub(al, 32);
}

void DreamGenContext::execCommand() {
	STACK_CHECK;
	es = cs;
	bx = offset_comlist;
	ds = cs;
	si = 1757;
	al = ds.byte(si);
	_cmp(al, 0);
	if (!flags.z())
		goto notblankinp;
	scrollMonitor();
	return;
notblankinp:
	cl = 0;
comloop:
	push(bx);
	push(si);
comloop2:
	al = ds.byte(si);
	_add(si, 2);
	ah = es.byte(bx);
	_inc(bx);
	_cmp(ah, 32);
	if (flags.z())
		goto foundcom;
	_cmp(al, ah);
	if (flags.z())
		goto comloop2;
	si = pop();
	bx = pop();
	_add(bx, 10);
	_inc(cl);
	_cmp(cl, 6);
	if (!flags.z())
		goto comloop;
	netError();
	al = 0;
	return;
foundcom:
	si = pop();
	bx = pop();
	_cmp(cl, 1);
	if (flags.z())
		goto testcom;
	_cmp(cl, 2);
	if (flags.z())
		goto directory;
	_cmp(cl, 3);
	if (flags.z())
		goto accesscom;
	_cmp(cl, 4);
	if (flags.z())
		goto signoncom;
	_cmp(cl, 5);
	if (flags.z())
		goto keyscom;
	goto quitcom;
directory:
	dirCom();
	al = 0;
	return;
signoncom:
	signOn();
	al = 0;
	return;
accesscom:
	read();
	al = 0;
	return;
keyscom:
	showKeys();
	al = 0;
	return;
testcom:
	al = 6;
	monMessage();
	al = 0;
	return;
quitcom:
	al = 1;
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
	di = 1412;
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
	si = 1757;
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

void DreamGenContext::showKeys() {
	STACK_CHECK;
	cx = 10;
	randomAccess();
	scrollMonitor();
	al = 18;
	monMessage();
	es = cs;
	bx = offset_keys;
	cx = 4;
keysloop:
	push(cx);
	push(bx);
	_cmp(es.byte(bx), 0);
	if (flags.z())
		goto notheld;
	_add(bx, 14);
	monPrint();
notheld:
	bx = pop();
	cx = pop();
	_add(bx, 26);
	if (--cx)
		goto keysloop;
	scrollMonitor();
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
	di = 1412;
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
	di = 1412+1;
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
	si = 1757;
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

void DreamGenContext::monitorLogo() {
	STACK_CHECK;
	al = data.byte(kLogonum);
	_cmp(al, data.byte(kOldlogonum));
	if (flags.z())
		goto notnewlogo;
	data.byte(kOldlogonum) = al;
	printLogo();
	printUnderMon();
	workToScreen();
	printLogo();
	printLogo();
	al = 26;
	playChannel1();
	cx = 20;
	randomAccess();
	return;
notnewlogo:
	printLogo();
}

void DreamGenContext::processTrigger() {
	STACK_CHECK;
	_cmp(data.byte(kLasttrigger), '1');
	if (!flags.z())
		goto notfirsttrigger;
	al = 8;
	setLocation();
	al = 45;
	triggerMessage();
	return;
notfirsttrigger:
	_cmp(data.byte(kLasttrigger), '2');
	if (!flags.z())
		goto notsecondtrigger;
	al = 9;
	setLocation();
	al = 55;
	triggerMessage();
	return;
notsecondtrigger:
	_cmp(data.byte(kLasttrigger), '3');
	if (!flags.z())
		return /* (notthirdtrigger) */;
	al = 2;
	setLocation();
	al = 59;
	triggerMessage();
}

void DreamGenContext::triggerMessage() {
	STACK_CHECK;
	push(ax);
	di = 174;
	bx = 153;
	cl = 200;
	ch = 63;
	ds = data.word(kMapstore);
	si = 0;
	multiGet();
	ax = pop();
	findPuzText();
	di = 174;
	bx = 156;
	dl = 141;
	ah = 16;
	printDirect();
	cx = 140;
	hangOn();
	workToScreen();
	cx = 340;
	hangOn();
	di = 174;
	bx = 153;
	cl = 200;
	ch = 63;
	ds = data.word(kMapstore);
	si = 0;
	multiPut();
	workToScreen();
	data.byte(kLasttrigger) = 0;
}

void DreamGenContext::useObject() {
	STACK_CHECK;
	data.byte(kWithobject) = 255;
	_cmp(data.byte(kCommandtype), 229);
	if (flags.z())
		goto alreadyuse;
	data.byte(kCommandtype) = 229;
	bl = data.byte(kCommand);
	bh = data.byte(kObjecttype);
	al = 51;
	commandWithOb();
alreadyuse:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (nouse) */;
	_and(ax, 1);
	if (!flags.z())
		goto douse;
	return;
douse:
	useRoutine();
}

void DreamGenContext::runTap() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto tapwith;
	withWhat();
	return;
tapwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'C';
	ch = 'U';
	dl = 'P';
	dh = 'E';
	compare();
	if (flags.z())
		goto fillcupfromtap;
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'C';
	ch = 'U';
	dl = 'P';
	dh = 'F';
	compare();
	if (flags.z())
		goto cupfromtapfull;
	cx = 300;
	al = 56;
	showPuzText();
	putBackObStuff();
	return;
fillcupfromtap:
	al = data.byte(kWithobject);
	getExAd();
	es.byte(bx+15) = 'F'-'A';
	al = 8;
	playChannel1();
	cx = 300;
	al = 57;
	showPuzText();
	putBackObStuff();
	return;
cupfromtapfull:
	cx = 300;
	al = 58;
	showPuzText();
	putBackObStuff();
}

void DreamGenContext::hotelControl() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 21);
	if (!flags.z())
		goto notrightcont;
	_cmp(data.byte(kMapx), 33);
	if (!flags.z())
		goto notrightcont;
	showFirstUse();
	putBackObStuff();
	return;
notrightcont:
	showSecondUse();
	putBackObStuff();
}

void DreamGenContext::openTomb() {
	STACK_CHECK;
	_inc(data.byte(kProgresspoints));
	showFirstUse();
	data.word(kWatchingtime) = 35*2;
	data.word(kReeltowatch) = 1;
	data.word(kEndwatchreel) = 33;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useTrainer() {
	STACK_CHECK;
	getAnyAd();
	_cmp(es.byte(bx+2), 4);
	if (!flags.z())
		goto notheldtrainer;
	_inc(data.byte(kProgresspoints));
	makeWorn();
	showSecondUse();
	putBackObStuff();
	return;
notheldtrainer:
	notHeldError();
}

void DreamGenContext::notHeldError() {
	STACK_CHECK;
	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	di = 64;
	bx = 100;
	al = 63;
	ah = 1;
	dl = 201;
	printmessage2();
	workToScreenM();
	cx = 50;
	hangOnP();
	putBackObStuff();
}

void DreamGenContext::usePipe() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto pipewith;
	withWhat();
	return;
pipewith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'C';
	ch = 'U';
	dl = 'P';
	dh = 'E';
	compare();
	if (flags.z())
		goto fillcup;
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'C';
	ch = 'U';
	dl = 'P';
	dh = 'F';
	compare();
	if (flags.z())
		goto alreadyfull;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
fillcup:
	cx = 300;
	al = 36;
	showPuzText();
	putBackObStuff();
	al = data.byte(kWithobject);
	getExAd();
	es.byte(bx+15) = 'F'-'A';
	return;
alreadyfull:
	cx = 300;
	al = 35;
	showPuzText();
	putBackObStuff();
}

void DreamGenContext::useFullCart() {
	STACK_CHECK;
	_inc(data.byte(kProgresspoints));
	al = 2;
	ah = data.byte(kRoomnum);
	_add(ah, 6);
	turnAnyPathOn();
	data.byte(kManspath) = 4;
	data.byte(kFacing) = 4;
	data.byte(kTurntoface) = 4;
	data.byte(kFinaldest) = 4;
	findXYFromPath();
	data.byte(kResetmanxy) = 1;
	showFirstUse();
	data.word(kWatchingtime) = 72*2;
	data.word(kReeltowatch) = 58;
	data.word(kEndwatchreel) = 142;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::usePlinth() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto plinthwith;
	withWhat();
	return;
plinthwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'D';
	ch = 'K';
	dl = 'E';
	dh = 'Y';
	compare();
	if (flags.z())
		goto isrightkey;
	showFirstUse();
	putBackObStuff();
	return;
isrightkey:
	_inc(data.byte(kProgresspoints));
	showSecondUse();
	data.word(kWatchingtime) = 220;
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 104;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	al = data.byte(kRoomafterdream);
	data.byte(kNewlocation) = al;
}

void DreamGenContext::chewy() {
	STACK_CHECK;
	showFirstUse();
	getAnyAd();
	es.byte(bx+2) = 255;
	data.byte(kGetback) = 1;
}

void DreamGenContext::sLabDoorB() {
	STACK_CHECK;
	_cmp(data.byte(kDreamnumber), 1);
	if (!flags.z())
		goto slabbwrong;
	al = 'S';
	ah = 'H';
	cl = 'L';
	ch = 'D';
	isRyanHolding();
	if (!flags.z())
		goto gotcrystal;
	al = 44;
	cx = 200;
	showPuzText();
	putBackObStuff();
	return;
gotcrystal:
	showFirstUse();
	_inc(data.byte(kProgresspoints));
	data.byte(kGetback) = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 44;
	data.word(kWatchingtime) = 60;
	data.word(kEndwatchreel) = 71;
	data.byte(kNewlocation) = 47;
	return;
slabbwrong:
	showFirstUse();
	data.byte(kGetback) = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 44;
	data.word(kWatchingtime) = 40;
	data.word(kEndwatchreel) = 63;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
}

void DreamGenContext::useSLab() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto slabwith;
	withWhat();
	return;
slabwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'J';
	ch = 'E';
	dl = 'W';
	dh = 'L';
	compare();
	if (flags.z())
		goto nextslab;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
nextslab:
	al = data.byte(kWithobject);
	getExAd();
	es.byte(bx+2) = 0;
	al = data.byte(kCommand);
	push(ax);
	removeSetObject();
	ax = pop();
	_inc(al);
	push(ax);
	placeSetObject();
	ax = pop();
	_cmp(al, 54);
	if (!flags.z())
		goto notlastslab;
	al = 0;
	turnPathOn();
	data.word(kWatchingtime) = 22;
	data.word(kReeltowatch) = 35;
	data.word(kEndwatchreel) = 48;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
notlastslab:
	_inc(data.byte(kProgresspoints));
	showFirstUse();
	data.byte(kGetback) = 1;
}

void DreamGenContext::useCart() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto cartwith;
	withWhat();
	return;
cartwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'R';
	ch = 'O';
	dl = 'C';
	dh = 'K';
	compare();
	if (flags.z())
		goto nextcart;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
nextcart:
	al = data.byte(kWithobject);
	getExAd();
	es.byte(bx+2) = 0;
	al = data.byte(kCommand);
	push(ax);
	removeSetObject();
	ax = pop();
	_inc(al);
	placeSetObject();
	_inc(data.byte(kProgresspoints));
	al = 17;
	playChannel1();
	showFirstUse();
	data.byte(kGetback) = 1;
}

void DreamGenContext::useClearBox() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto clearboxwith;
	withWhat();
	return;
clearboxwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'R';
	ch = 'A';
	dl = 'I';
	dh = 'L';
	compare();
	if (flags.z())
		goto openbox;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
openbox:
	_inc(data.byte(kProgresspoints));
	showFirstUse();
	data.word(kWatchingtime) = 80;
	data.word(kReeltowatch) = 67;
	data.word(kEndwatchreel) = 105;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useOpenBox() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto openboxwith;
	withWhat();
	return;
openboxwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'C';
	ch = 'U';
	dl = 'P';
	dh = 'F';
	compare();
	if (flags.z())
		goto destoryopenbox;
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'C';
	ch = 'U';
	dl = 'P';
	dh = 'E';
	compare();
	if (flags.z())
		goto openboxwrong;
	showFirstUse();
	return;
destoryopenbox:
	_inc(data.byte(kProgresspoints));
	cx = 300;
	al = 37;
	showPuzText();
	al = data.byte(kWithobject);
	getExAd();
	es.byte(bx+15) = 'E'-'A';
	data.word(kWatchingtime) = 140;
	data.word(kReeltowatch) = 105;
	data.word(kEndwatchreel) = 181;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	al = 4;
	turnPathOn();
	data.byte(kGetback) = 1;
	return;
openboxwrong:
	cx = 300;
	al = 38;
	showPuzText();
	putBackObStuff();
}

void DreamGenContext::useHole() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto holewith;
	withWhat();
	return;
holewith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'H';
	ch = 'N';
	dl = 'D';
	dh = 'A';
	compare();
	if (flags.z())
		goto righthand;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
righthand:
	showFirstUse();
	al = 86;
	removeSetObject();
	al = data.byte(kWithobject);
	getExAd();
	es.byte(bx+2) = 255;
	data.byte(kCanmovealtar) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useAltar() {
	STACK_CHECK;
	al = 'C';
	ah = 'N';
	cl = 'D';
	ch = 'A';
	findExObject();
	_cmp(al, (114));
	if (flags.z())
		goto thingsonaltar;
	al = 'C';
	ah = 'N';
	cl = 'D';
	ch = 'B';
	findExObject();
	_cmp(al, (114));
	if (flags.z())
		goto thingsonaltar;
	_cmp(data.byte(kCanmovealtar), 1);
	if (flags.z())
		goto movealtar;
	cx = 300;
	al = 23;
	showPuzText();
	data.byte(kGetback) = 1;
	return;
movealtar:
	_inc(data.byte(kProgresspoints));
	showSecondUse();
	data.word(kWatchingtime) = 160;
	data.word(kReeltowatch) = 81;
	data.word(kEndwatchreel) = 174;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	al = 47;
	bl = 52;
	bh = 76;
	cx = 32;
	dx = 98;
	setupTimedUse();
	data.byte(kGetback) = 1;
	return;
thingsonaltar:
	showFirstUse();
	data.byte(kGetback) = 1;
}

void DreamGenContext::openTVDoor() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto tvdoorwith;
	withWhat();
	return;
tvdoorwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'U';
	ch = 'L';
	dl = 'O';
	dh = 'K';
	compare();
	if (flags.z())
		goto keyontv;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
keyontv:
	showFirstUse();
	data.byte(kLockstatus) = 0;
	data.byte(kGetback) = 1;
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

void DreamGenContext::drawItAll() {
	STACK_CHECK;
	createPanel();
	drawFloor();
	printSprites();
	showIcon();
}

void DreamGenContext::openHotelDoor() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto hoteldoorwith;
	withWhat();
	return;
hoteldoorwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'K';
	ch = 'E';
	dl = 'Y';
	dh = 'A';
	compare();
	if (flags.z())
		goto keyonhotel1;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
keyonhotel1:
	al = 16;
	playChannel1();
	showFirstUse();
	data.byte(kLockstatus) = 0;
	data.byte(kGetback) = 1;
}

void DreamGenContext::openHotelDoor2() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto hoteldoorwith2;
	withWhat();
	return;
hoteldoorwith2:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'K';
	ch = 'E';
	dl = 'Y';
	dh = 'A';
	compare();
	if (flags.z())
		goto keyonhotel2;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
keyonhotel2:
	al = 16;
	playChannel1();
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::grafittiDoor() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto grafwith;
	withWhat();
	return;
grafwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'A';
	ch = 'P';
	dl = 'E';
	dh = 'N';
	compare();
	if (flags.z())
		goto dograf;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
dograf:
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::usePoolReader() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto poolwith;
	withWhat();
	return;
poolwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'M';
	ch = 'E';
	dl = 'M';
	dh = 'B';
	compare();
	if (flags.z())
		goto openpool;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
openpool:
	_cmp(data.byte(kTalkedtoattendant), 1);
	if (flags.z())
		goto canopenpool;
	showSecondUse();
	putBackObStuff();
	return;
canopenpool:
	al = 17;
	playChannel1();
	showFirstUse();
	data.byte(kCounttoopen) = 6;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useLighter() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotlighterwith;
	withWhat();
	return;
gotlighterwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'S';
	ch = 'M';
	dl = 'K';
	dh = 'E';
	compare();
	if (flags.z())
		goto cigarette;
	showFirstUse();
	putBackObStuff();
	return;
cigarette:
	cx = 300;
	al = 9;
	showPuzText();
	al = data.byte(kWithobject);
	getExAd();
	es.byte(bx+2) = 255;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useCardReader1() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotreader1with;
	withWhat();
	return;
gotreader1with:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'C';
	ch = 'S';
	dl = 'H';
	dh = 'R';
	compare();
	if (flags.z())
		goto correctcard;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
correctcard:
	_cmp(data.byte(kTalkedtosparky), 0);
	if (flags.z())
		goto notyet;
	_cmp(data.word(kCard1money), 0);
	if (flags.z())
		goto getscash;
	cx = 300;
	al = 17;
	showPuzText();
	putBackObStuff();
	return;
getscash:
	al = 16;
	playChannel1();
	cx = 300;
	al = 18;
	showPuzText();
	_inc(data.byte(kProgresspoints));
	data.word(kCard1money) = 12432;
	data.byte(kGetback) = 1;
	return;
notyet:
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::useCardReader2() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotreader2with;
	withWhat();
	return;
gotreader2with:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'C';
	ch = 'S';
	dl = 'H';
	dh = 'R';
	compare();
	if (flags.z())
		goto correctcard2;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
correctcard2:
	_cmp(data.byte(kTalkedtoboss), 0);
	if (flags.z())
		goto notyetboss;
	_cmp(data.word(kCard1money), 0);
	if (flags.z())
		goto nocash;
	_cmp(data.byte(kGunpassflag), 2);
	if (flags.z())
		goto alreadygotnew;
	al = 18;
	playChannel1();
	cx = 300;
	al = 19;
	showPuzText();
	al = 94;
	placeSetObject();
	data.byte(kGunpassflag) = 1;
	_sub(data.word(kCard1money), 2000);
	_inc(data.byte(kProgresspoints));
	data.byte(kGetback) = 1;
	return;
nocash:
	cx = 300;
	al = 20;
	showPuzText();
	putBackObStuff();
	return;
alreadygotnew:
	cx = 300;
	al = 22;
	showPuzText();
	putBackObStuff();
	return;
notyetboss:
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::useCardReader3() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotreader3with;
	withWhat();
	return;
gotreader3with:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'C';
	ch = 'S';
	dl = 'H';
	dh = 'R';
	compare();
	if (flags.z())
		goto rightcard;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
rightcard:
	_cmp(data.byte(kTalkedtorecep), 0);
	if (flags.z())
		goto notyetrecep;
	_cmp(data.byte(kCardpassflag), 0);
	if (!flags.z())
		goto alreadyusedit;
	al = 16;
	playChannel1();
	cx = 300;
	al = 25;
	showPuzText();
	_inc(data.byte(kProgresspoints));
	_sub(data.word(kCard1money), 8300);
	data.byte(kCardpassflag) = 1;
	data.byte(kGetback) = 1;
	return;
alreadyusedit:
	cx = 300;
	al = 26;
	showPuzText();
	putBackObStuff();
	return;
notyetrecep:
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::useCashCard() {
	STACK_CHECK;
	getRidOfReels();
	loadKeypad();
	createPanel();
	showPanel();
	showExit();
	showMan();
	di = 114;
	bx = 120;
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto _tmp1;
	bx = 120-3;
_tmp1:
	ds = data.word(kTempgraphics);
	al = 39;
	ah = 0;
	showFrame();
	ax = data.word(kCard1money);
	moneyPoke();
	getObTextStart();
	nextColon();
	nextColon();
	di = 36;
	bx = 98;
	dl = 241;
	al = 0;
	ah = 0;
	printDirect();
	di = 160;
	bx = 155;
	es = cs;
	si = offset_money1poke;
	data.word(kCharshift) = 91*2+75;
	al = 0;
	ah = 0;
	dl = 240;
	printDirect();
	di = 187;
	bx = 155;
	es = cs;
	si = offset_money2poke;
	data.word(kCharshift) = 91*2+85;
	al = 0;
	ah = 0;
	dl = 240;
	printDirect();
	data.word(kCharshift) = 0;
	workToScreenM();
	cx = 400;
	hangOnP();
	getRidOfTemp();
	restoreReels();
	putBackObStuff();
}

void DreamGenContext::lookAtCard() {
	STACK_CHECK;
	data.byte(kManisoffscreen) = 1;
	getRidOfReels();
	loadKeypad();
	createPanel2();
	di = 160;
	bx = 80;
	ds = data.word(kTempgraphics);
	al = 42;
	ah = 128;
	showFrame();
	getObTextStart();
	findNextColon();
	findNextColon();
	findNextColon();
	di = 36;
	bx = 124;
	dl = 241;
	al = 0;
	ah = 0;
	printDirect();
	push(es);
	push(si);
	workToScreenM();
	cx = 280;
	hangOnW();
	createPanel2();
	di = 160;
	bx = 80;
	ds = data.word(kTempgraphics);
	al = 42;
	ah = 128;
	showFrame();
	si = pop();
	es = pop();
	di = 36;
	bx = 130;
	dl = 241;
	al = 0;
	ah = 0;
	printDirect();
	workToScreenM();
	cx = 200;
	hangOnW();
	data.byte(kManisoffscreen) = 0;
	getRidOfTemp();
	restoreReels();
	putBackObStuff();
}

void DreamGenContext::moneyPoke() {
	STACK_CHECK;
	bx = offset_money1poke;
	cl = 48-1;
numberpoke0:
	_inc(cl);
	_sub(ax, 10000);
	if (!flags.c())
		goto numberpoke0;
	_add(ax, 10000);
	cs.byte(bx) = cl;
	_inc(bx);
	cl = 48-1;
numberpoke1:
	_inc(cl);
	_sub(ax, 1000);
	if (!flags.c())
		goto numberpoke1;
	_add(ax, 1000);
	cs.byte(bx) = cl;
	_inc(bx);
	cl = 48-1;
numberpoke2:
	_inc(cl);
	_sub(ax, 100);
	if (!flags.c())
		goto numberpoke2;
	_add(ax, 100);
	cs.byte(bx) = cl;
	_inc(bx);
	cl = 48-1;
numberpoke3:
	_inc(cl);
	_sub(ax, 10);
	if (!flags.c())
		goto numberpoke3;
	_add(ax, 10);
	cs.byte(bx) = cl;
	bx = offset_money2poke;
	_add(al, 48);
	cs.byte(bx) = al;
}

void DreamGenContext::useControl() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotcontrolwith;
	withWhat();
	return;
gotcontrolwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'K';
	ch = 'E';
	dl = 'Y';
	dh = 'A';
	compare();
	if (flags.z())
		goto rightkey;
	_cmp(data.byte(kReallocation), 21);
	if (!flags.z())
		goto balls;
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'K';
	ch = 'N';
	dl = 'F';
	dh = 'E';
	compare();
	if (flags.z())
		goto jimmycontrols;
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'A';
	ch = 'X';
	dl = 'E';
	dh = 'D';
	compare();
	if (flags.z())
		goto axeoncontrols;
balls:
	showFirstUse();
	putBackObStuff();
	return;
rightkey:
	al = 16;
	playChannel1();
	_cmp(data.byte(kLocation), 21);
	if (flags.z())
		goto goingdown;
	cx = 300;
	al = 0;
	showPuzText();
	data.byte(kNewlocation) = 21;
	data.byte(kCounttoclose) = 8;
	data.byte(kCounttoopen) = 0;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
	return;
goingdown:
	cx = 300;
	al = 3;
	showPuzText();
	data.byte(kNewlocation) = 30;
	data.byte(kCounttoclose) = 8;
	data.byte(kCounttoopen) = 0;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
	return;
jimmycontrols:
	al = 50;
	placeSetObject();
	al = 51;
	placeSetObject();
	al = 26;
	placeSetObject();
	al = 30;
	placeSetObject();
	al = 16;
	removeSetObject();
	al = 17;
	removeSetObject();
	al = 14;
	playChannel1();
	cx = 300;
	al = 10;
	showPuzText();
	_inc(data.byte(kProgresspoints));
	data.byte(kGetback) = 1;
	return;
axeoncontrols:
	cx = 300;
	al = 16;
	showPuzText();
	_inc(data.byte(kProgresspoints));
	putBackObStuff();
}

void DreamGenContext::useWire() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotwirewith;
	withWhat();
	return;
gotwirewith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'K';
	ch = 'N';
	dl = 'F';
	dh = 'E';
	compare();
	if (flags.z())
		goto wireknife;
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'A';
	ch = 'X';
	dl = 'E';
	dh = 'D';
	compare();
	if (flags.z())
		goto wireaxe;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
wireaxe:
	cx = 300;
	al = 16;
	showPuzText();
	putBackObStuff();
	return;
wireknife:
	al = 51;
	removeSetObject();
	al = 52;
	placeSetObject();
	cx = 300;
	al = 11;
	showPuzText();
	_inc(data.byte(kProgresspoints));
	data.byte(kGetback) = 1;
}

void DreamGenContext::useHandle() {
	STACK_CHECK;
	al = 'C';
	ah = 'U';
	cl = 'T';
	ch = 'W';
	findSetObject();
	al = es.byte(bx+58);
	_cmp(al, 255);
	if (!flags.z())
		goto havecutwire;
	cx = 300;
	al = 12;
	showPuzText();
	data.byte(kGetback) = 1;
	return;
havecutwire:
	cx = 300;
	al = 13;
	showPuzText();
	data.byte(kNewlocation) = 22;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useKey() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 5);
	if (flags.z())
		goto usekey1;
	_cmp(data.byte(kLocation), 30);
	if (flags.z())
		goto usekey1;
	_cmp(data.byte(kLocation), 21);
	if (flags.z())
		goto usekey2;
	cx = 200;
	al = 1;
	showPuzText();
	putBackObStuff();
	return;
usekey1:
	_cmp(data.byte(kMapx), 22);
	if (!flags.z())
		goto wrongroom1;
	_cmp(data.byte(kMapy), 10);
	if (!flags.z())
		goto wrongroom1;
	cx = 300;
	al = 0;
	showPuzText();
	data.byte(kCounttoclose) = 100;
	data.byte(kGetback) = 1;
	return;
usekey2:
	_cmp(data.byte(kMapx), 11);
	if (!flags.z())
		goto wrongroom1;
	_cmp(data.byte(kMapy), 10);
	if (!flags.z())
		goto wrongroom1;
	cx = 300;
	al = 3;
	showPuzText();
	data.byte(kNewlocation) = 30;
	al = 2;
	fadeScreenDown();
	showFirstUse();
	putBackObStuff();
	return;
wrongroom1:
	cx = 200;
	al = 2;
	showPuzText();
	putBackObStuff();
}

void DreamGenContext::useStereo() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 0);
	if (flags.z())
		goto stereook;
	cx = 400;
	al = 4;
	showPuzText();
	putBackObStuff();
	return;
stereook:
	_cmp(data.byte(kMapx), 11);
	if (!flags.z())
		goto stereonotok;
	_cmp(data.byte(kMapy), 0);
	if (flags.z())
		goto stereook2;
stereonotok:
	cx = 400;
	al = 5;
	showPuzText();
	putBackObStuff();
	return;
stereook2:
	al = 'C';
	ah = 'D';
	cl = 'P';
	ch = 'L';
	findSetObject();
	ah = 1;
	checkInside();
	_cmp(cl, (114));
	if (!flags.z())
		goto cdinside;
	al = 6;
	cx = 400;
	showPuzText();
	putBackObStuff();
	getAnyAd();
	al = 255;
	es.byte(bx+10) = al;
	return;
cdinside:
	getAnyAd();
	al = es.byte(bx+10);
	_xor(al, 1);
	es.byte(bx+10) = al;
	_cmp(al, 255);
	if (flags.z())
		goto stereoon;
	al = 7;
	cx = 400;
	showPuzText();
	putBackObStuff();
	return;
stereoon:
	al = 8;
	cx = 400;
	showPuzText();
	putBackObStuff();
}

void DreamGenContext::useCooker() {
	STACK_CHECK;
	al = data.byte(kCommand);
	ah = data.byte(kObjecttype);
	checkInside();
	_cmp(cl, (114));
	if (!flags.z())
		goto foodinside;
	showFirstUse();
	putBackObStuff();
	return;
foodinside:
	showSecondUse();
	putBackObStuff();
}

void DreamGenContext::useAxe() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 22);
	if (!flags.z())
		goto notinpool;
	_cmp(data.byte(kMapy), 10);
	if (flags.z())
		goto axeondoor;
	showSecondUse();
	_inc(data.byte(kProgresspoints));
	data.byte(kLastweapon) = 2;
	data.byte(kGetback) = 1;
	removeObFromInv();
	return;
notinpool:
	showFirstUse();
	return;
/*continuing to unbounded code: axeondoor from useelvdoor:19-30*/
axeondoor:
	al = 15;
	cx = 300;
	showPuzText();
	_inc(data.byte(kProgresspoints));
	data.word(kWatchingtime) = 46*2;
	data.word(kReeltowatch) = 31;
	data.word(kEndwatchreel) = 77;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useElvDoor() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotdoorwith;
	withWhat();
	return;
gotdoorwith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'A';
	ch = 'X';
	dl = 'E';
	dh = 'D';
	compare();
	if (flags.z())
		goto axeondoor;
	al = 14;
	cx = 300;
	showPuzText();
	putBackObStuff();
	return;
axeondoor:
	al = 15;
	cx = 300;
	showPuzText();
	_inc(data.byte(kProgresspoints));
	data.word(kWatchingtime) = 46*2;
	data.word(kReeltowatch) = 31;
	data.word(kEndwatchreel) = 77;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::withWhat() {
	STACK_CHECK;
	createPanel();
	showPanel();
	showMan();
	showExit();
	al = data.byte(kCommand);
	ah = data.byte(kObjecttype);
	es = cs;
	di = offset_commandline;
	copyName();
	di = 100;
	bx = 21;
	dl = 200;
	al = 63;
	ah = 2;
	printmessage2();
	di = data.word(kLastxpos);
	_add(di, 5);
	bx = 21;
	es = cs;
	si = offset_commandline;
	dl = 220;
	al = 0;
	ah = 0;
	printDirect();
	di = data.word(kLastxpos);
	_add(di, 5);
	bx = 21;
	dl = 200;
	al = 63;
	ah = 3;
	printmessage2();
	fillRyan();
	data.byte(kCommandtype) = 255;
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	data.byte(kInvopen) = 2;
}

void DreamGenContext::selectOb() {
	STACK_CHECK;
	findInvPos();
	ax = es.word(bx);
	_cmp(al, 255);
	if (!flags.z())
		goto canselectob;
	blank();
	return;
canselectob:
	data.byte(kWithobject) = al;
	data.byte(kWithtype) = ah;
	_cmp(ax, data.word(kOldsubject));
	if (!flags.z())
		goto diffsub3;
	_cmp(data.byte(kCommandtype), 221);
	if (flags.z())
		goto alreadyselob;
	data.byte(kCommandtype) = 221;
diffsub3:
	data.word(kOldsubject) = ax;
	bx = ax;
	al = 0;
	commandWithOb();
alreadyselob:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notselob) */;
	_and(ax, 1);
	if (!flags.z())
		goto doselob;
	return;
doselob:
	delPointer();
	data.byte(kInvopen) = 0;
	useRoutine();
}

void DreamGenContext::findSetObject() {
	STACK_CHECK;
	_sub(al, 'A');
	_sub(ah, 'A');
	_sub(cl, 'A');
	_sub(ch, 'A');
	es = data.word(kSetdat);
	bx = 0;
	dl = 0;
findsetloop:
	_cmp(al, es.byte(bx+12));
	if (!flags.z())
		goto nofind;
	_cmp(ah, es.byte(bx+13));
	if (!flags.z())
		goto nofind;
	_cmp(cl, es.byte(bx+14));
	if (!flags.z())
		goto nofind;
	_cmp(ch, es.byte(bx+15));
	if (!flags.z())
		goto nofind;
	al = dl;
	return;
nofind:
	_add(bx, 64);
	_inc(dl);
	_cmp(dl, 128);
	if (!flags.z())
		goto findsetloop;
	al = dl;
}

void DreamGenContext::findExObject() {
	STACK_CHECK;
	_sub(al, 'A');
	_sub(ah, 'A');
	_sub(cl, 'A');
	_sub(ch, 'A');
	es = data.word(kExtras);
	bx = (0+2080+30000);
	dl = 0;
findexloop:
	_cmp(al, es.byte(bx+12));
	if (!flags.z())
		goto nofindex;
	_cmp(ah, es.byte(bx+13));
	if (!flags.z())
		goto nofindex;
	_cmp(cl, es.byte(bx+14));
	if (!flags.z())
		goto nofindex;
	_cmp(ch, es.byte(bx+15));
	if (!flags.z())
		goto nofindex;
	al = dl;
	return;
nofindex:
	_add(bx, 16);
	_inc(dl);
	_cmp(dl, (114));
	if (!flags.z())
		goto findexloop;
	al = dl;
}

void DreamGenContext::isRyanHolding() {
	STACK_CHECK;
	_sub(al, 'A');
	_sub(ah, 'A');
	_sub(cl, 'A');
	_sub(ch, 'A');
	es = data.word(kExtras);
	bx = (0+2080+30000);
	dl = 0;
searchinv:
	_cmp(es.byte(bx+2), 4);
	if (!flags.z())
		goto nofindininv;
	_cmp(al, es.byte(bx+12));
	if (!flags.z())
		goto nofindininv;
	_cmp(ah, es.byte(bx+13));
	if (!flags.z())
		goto nofindininv;
	_cmp(cl, es.byte(bx+14));
	if (!flags.z())
		goto nofindininv;
	_cmp(ch, es.byte(bx+15));
	if (!flags.z())
		goto nofindininv;
	al = dl;
	_cmp(al, (114));
	return;
nofindininv:
	_add(bx, 16);
	_inc(dl);
	_cmp(dl, (114));
	if (!flags.z())
		goto searchinv;
	al = dl;
	_cmp(al, (114));
}

void DreamGenContext::checkInside() {
	STACK_CHECK;
	es = data.word(kExtras);
	bx = (0+2080+30000);
	cl = 0;
insideloop:
	_cmp(al, es.byte(bx+3));
	if (!flags.z())
		goto notfoundinside;
	_cmp(ah, es.byte(bx+2));
	if (!flags.z())
		goto notfoundinside;
	return;
notfoundinside:
	_add(bx, 16);
	_inc(cl);
	_cmp(cl, (114));
	if (!flags.z())
		goto insideloop;
}

void DreamGenContext::putBackObStuff() {
	STACK_CHECK;
	createPanel();
	showPanel();
	showMan();
	obIcons();
	showExit();
	obPicture();
	describeOb();
	underTextLine();
	data.byte(kCommandtype) = 255;
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::showPuzText() {
	STACK_CHECK;
	push(cx);
	findPuzText();
	push(es);
	push(si);
	createPanel();
	showPanel();
	showMan();
	showExit();
	obIcons();
	si = pop();
	es = pop();
	di = 36;
	bx = 104;
	dl = 241;
	ah = 0;
	printDirect();
	workToScreenM();
	cx = pop();
	hangOnP();
}

void DreamGenContext::findPuzText() {
	STACK_CHECK;
	ah = 0;
	si = ax;
	_add(si, si);
	es = data.word(kPuzzletext);
	ax = es.word(si);
	_add(ax, (66*2));
	si = ax;
}

void DreamGenContext::isSetObOnMap() {
	STACK_CHECK;
	push(es);
	push(bx);
	getSetAd();
	al = es.byte(bx+58);
	bx = pop();
	es = pop();
	_cmp(al, 0);
}

void DreamGenContext::placeFreeObject() {
	STACK_CHECK;
	push(es);
	push(bx);
	cl = 0;
	ch = 1;
	findOrMake();
	getFreeAd();
	es.byte(bx+2) = 0;
	bx = pop();
	es = pop();
}

void DreamGenContext::removeFreeObject() {
	STACK_CHECK;
	push(es);
	push(bx);
	getFreeAd();
	es.byte(bx+2) = 255;
	bx = pop();
	es = pop();
}

void DreamGenContext::autoAppear() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 32);
	if (!flags.z())
		goto notinalley;
	al = 5;
	resetLocation();
	al = 10;
	setLocation();
	data.byte(kDestpos) = 10;
	return;
notinalley:
	_cmp(data.byte(kReallocation), 24);
	if (!flags.z())
		goto notinedens;
	_cmp(data.byte(kGeneraldead), 1);
	if (!flags.z())
		goto edenspart2;
	_inc(data.byte(kGeneraldead));
	al = 44;
	placeSetObject();
	al = 18;
	placeSetObject();
	al = 93;
	placeSetObject();
	al = 92;
	removeSetObject();
	al = 55;
	removeSetObject();
	al = 75;
	removeSetObject();
	al = 84;
	removeSetObject();
	al = 85;
	removeSetObject();
	return;
edenspart2:
	_cmp(data.byte(kSartaindead), 1);
	if (!flags.z())
		return /* (notedens2) */;
	al = 44;
	removeSetObject();
	al = 93;
	removeSetObject();
	al = 55;
	placeSetObject();
	_inc(data.byte(kSartaindead));
	return;
notinedens:
	_cmp(data.byte(kReallocation), 25);
	if (!flags.z())
		goto notonsartroof;
	data.byte(kNewsitem) = 3;
	al = 6;
	resetLocation();
	al = 11;
	setLocation();
	data.byte(kDestpos) = 11;
	return;
notonsartroof:
	_cmp(data.byte(kReallocation), 2);
	if (!flags.z())
		return /* (notinlouiss) */;
	_cmp(data.byte(kRockstardead), 0);
	if (flags.z())
		return /* (notinlouiss) */;
	al = 23;
	placeSetObject();
}

void DreamGenContext::setupTimedUse() {
	STACK_CHECK;
	_cmp(data.word(kTimecount), 0);
	if (!flags.z())
		return /* (cantsetup) */;
	data.byte(kTimedy) = bh;
	data.byte(kTimedx) = bl;
	data.word(kCounttotimed) = cx;
	_add(dx, cx);
	data.word(kTimecount) = dx;
	bl = al;
	bh = 0;
	_add(bx, bx);
	es = data.word(kPuzzletext);
	cx = (66*2);
	ax = es.word(bx);
	_add(ax, cx);
	bx = ax;
	data.word(kTimedseg) = es;
	data.word(kTimedoffset) = bx;
}

void DreamGenContext::useChurchGate() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gatewith;
	withWhat();
	return;
gatewith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'C';
	ch = 'U';
	dl = 'T';
	dh = 'T';
	compare();
	if (flags.z())
		goto cutgate;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
cutgate:
	showFirstUse();
	data.word(kWatchingtime) = 64*2;
	data.word(kReeltowatch) = 4;
	data.word(kEndwatchreel) = 70;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	_inc(data.byte(kProgresspoints));
	al = 3;
	turnPathOn();
	_cmp(data.byte(kAidedead), 0);
	if (flags.z())
		return /* (notopenchurch) */;
	al = 2;
	turnPathOn();
}

void DreamGenContext::useGun() {
	STACK_CHECK;
	_cmp(data.byte(kObjecttype), 4);
	if (flags.z())
		goto istakengun;
	showSecondUse();
	putBackObStuff();
	return;
istakengun:
	_cmp(data.byte(kReallocation), 22);
	if (!flags.z())
		goto notinpoolroom;
	cx = 300;
	al = 34;
	showPuzText();
	data.byte(kLastweapon) = 1;
	data.byte(kCombatcount) = 39;
	data.byte(kGetback) = 1;
	_inc(data.byte(kProgresspoints));
	return;
notinpoolroom:
	_cmp(data.byte(kReallocation), 25);
	if (!flags.z())
		goto nothelicopter;
	cx = 300;
	al = 34;
	showPuzText();
	data.byte(kLastweapon) = 1;
	data.byte(kCombatcount) = 19;
	data.byte(kGetback) = 1;
	data.byte(kDreamnumber) = 2;
	data.byte(kRoomafterdream) = 38;
	data.byte(kSartaindead) = 1;
	_inc(data.byte(kProgresspoints));
	return;
nothelicopter:
	_cmp(data.byte(kReallocation), 27);
	if (!flags.z())
		goto notinrockroom;
	cx = 300;
	al = 46;
	showPuzText();
	data.byte(kPointermode) = 2;
	data.byte(kRockstardead) = 1;
	data.byte(kLastweapon) = 1;
	data.byte(kNewsitem) = 1;
	data.byte(kGetback) = 1;
	data.byte(kRoomafterdream) = 32;
	data.byte(kDreamnumber) = 0;
	_inc(data.byte(kProgresspoints));
	return;
notinrockroom:
	_cmp(data.byte(kReallocation), 8);
	if (!flags.z())
		goto notbystudio;
	_cmp(data.byte(kMapx), 22);
	if (!flags.z())
		goto notbystudio;
	_cmp(data.byte(kMapy), 40);
	if (!flags.z())
		goto notbystudio;
	al = 92;
	isSetObOnMap();
	if (flags.z())
		goto notbystudio;
	_cmp(data.byte(kManspath), 9);
	if (flags.z())
		goto notbystudio;
	data.byte(kDestination) = 9;
	data.byte(kFinaldest) = 9;
	autoSetWalk();
	data.byte(kLastweapon) = 1;
	data.byte(kGetback) = 1;
	_inc(data.byte(kProgresspoints));
	return;
notbystudio:
	_cmp(data.byte(kReallocation), 6);
	if (!flags.z())
		goto notsarters;
	_cmp(data.byte(kMapx), 11);
	if (!flags.z())
		goto notsarters;
	_cmp(data.byte(kMapy), 20);
	if (!flags.z())
		goto notsarters;
	al = 5;
	isSetObOnMap();
	if (!flags.z())
		goto notsarters;
	data.byte(kDestination) = 1;
	data.byte(kFinaldest) = 1;
	autoSetWalk();
	al = 5;
	removeSetObject();
	al = 6;
	placeSetObject();
	al = 1;
	ah = data.byte(kRoomnum);
	_dec(ah);
	turnAnyPathOn();
	data.byte(kLiftflag) = 1;
	data.word(kWatchingtime) = 40*2;
	data.word(kReeltowatch) = 4;
	data.word(kEndwatchreel) = 43;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	_inc(data.byte(kProgresspoints));
	return;
notsarters:
	_cmp(data.byte(kReallocation), 29);
	if (!flags.z())
		goto notaide;
	data.byte(kGetback) = 1;
	al = 13;
	resetLocation();
	al = 12;
	setLocation();
	data.byte(kDestpos) = 12;
	data.byte(kDestination) = 2;
	data.byte(kFinaldest) = 2;
	autoSetWalk();
	data.word(kWatchingtime) = 164*2;
	data.word(kReeltowatch) = 3;
	data.word(kEndwatchreel) = 164;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kAidedead) = 1;
	data.byte(kDreamnumber) = 3;
	data.byte(kRoomafterdream) = 33;
	_inc(data.byte(kProgresspoints));
	return;
notaide:
	_cmp(data.byte(kReallocation), 23);
	if (!flags.z())
		goto notwithboss;
	_cmp(data.byte(kMapx), 0);
	if (!flags.z())
		goto notwithboss;
	_cmp(data.byte(kMapy), 50);
	if (!flags.z())
		goto notwithboss;
	_cmp(data.byte(kManspath), 5);
	if (flags.z())
		goto pathokboss;
	data.byte(kDestination) = 5;
	data.byte(kFinaldest) = 5;
	autoSetWalk();
pathokboss:
	data.byte(kLastweapon) = 1;
	data.byte(kGetback) = 1;
	return;
notwithboss:
	_cmp(data.byte(kReallocation), 8);
	if (!flags.z())
		goto nottvsoldier;
	_cmp(data.byte(kMapx), 11);
	if (!flags.z())
		goto nottvsoldier;
	_cmp(data.byte(kMapy), 10);
	if (!flags.z())
		goto nottvsoldier;
	_cmp(data.byte(kManspath), 2);
	if (flags.z())
		goto pathoktv;
	data.byte(kDestination) = 2;
	data.byte(kFinaldest) = 2;
	autoSetWalk();
pathoktv:
	data.byte(kLastweapon) = 1;
	data.byte(kGetback) = 1;
	return;
nottvsoldier:
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::useButtonA() {
	STACK_CHECK;
	al = 95;
	isSetObOnMap();
	if (flags.z())
		goto donethisbit;
	showFirstUse();
	al = 0;
	ah = data.byte(kRoomnum);
	_dec(ah);
	turnAnyPathOn();
	al = 9;
	removeSetObject();
	al = 95;
	placeSetObject();
	data.word(kWatchingtime) = 15*2;
	data.word(kReeltowatch) = 71;
	data.word(kEndwatchreel) = 85;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	_inc(data.byte(kProgresspoints));
	return;
donethisbit:
	showSecondUse();
	putBackObStuff();
}

void DreamGenContext::usePlate() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto platewith;
	withWhat();
	return;
platewith:
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'S';
	ch = 'C';
	dl = 'R';
	dh = 'W';
	compare();
	if (flags.z())
		goto unscrewplate;
	al = data.byte(kWithobject);
	ah = data.byte(kWithtype);
	cl = 'K';
	ch = 'N';
	dl = 'F';
	dh = 'E';
	compare();
	if (flags.z())
		goto triedknife;
	cx = 300;
	al = 14;
	showPuzText();
	putBackObStuff();
	return;
unscrewplate:
	al = 20;
	playChannel1();
	showFirstUse();
	al = 28;
	placeSetObject();
	al = 24;
	placeSetObject();
	al = 25;
	removeSetObject();
	al = 0;
	placeFreeObject();
	_inc(data.byte(kProgresspoints));
	data.byte(kGetback) = 1;
	return;
triedknife:
	cx = 300;
	al = 54;
	showPuzText();
	putBackObStuff();
}

void DreamGenContext::useWinch() {
	STACK_CHECK;
	al = 40;
	ah = 1;
	checkInside();
	_cmp(cl, (114));
	if (flags.z())
		goto nowinch;
	al = cl;
	ah = 4;
	cl = 'F';
	ch = 'U';
	dl = 'S';
	dh = 'E';
	compare();
	if (!flags.z())
		goto nowinch;
	data.word(kWatchingtime) = 217*2;
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 217;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kDestpos) = 1;
	data.byte(kNewlocation) = 45;
	data.byte(kDreamnumber) = 1;
	data.byte(kRoomafterdream) = 44;
	data.byte(kGeneraldead) = 1;
	data.byte(kNewsitem) = 2;
	data.byte(kGetback) = 1;
	_inc(data.byte(kProgresspoints));
	return;
nowinch:
	showFirstUse();
	putBackObStuff();
}

void DreamGenContext::quitKey() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 222);
	if (flags.z())
		goto alreadyqk;
	data.byte(kCommandtype) = 222;
	al = 4;
	commandOnly();
alreadyqk:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notqk) */;
	_and(ax, 1);
	if (!flags.z())
		goto doqk;
	return;
doqk:
	data.byte(kGetback) = 1;
}

void DreamGenContext::dumpKeypad() {
	STACK_CHECK;
	di = (36+112)-3;
	bx = (72)-4;
	cl = 120;
	ch = 90;
	multiDump();
}

void DreamGenContext::quitSymbol() {
	STACK_CHECK;
	_cmp(data.byte(kSymboltopx), 24);
	if (!flags.z())
		{ blank(); return; };
	_cmp(data.byte(kSymbolbotx), 24);
	if (!flags.z())
		{ blank(); return; };
	_cmp(data.byte(kCommandtype), 222);
	if (flags.z())
		goto alreadyqs;
	data.byte(kCommandtype) = 222;
	al = 18;
	commandOnly();
alreadyqs:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notqs) */;
	_and(ax, 1);
	if (!flags.z())
		goto doqs;
	return;
doqs:
	data.byte(kGetback) = 1;
}

void DreamGenContext::dumpSymbol() {
	STACK_CHECK;
	data.byte(kNewtextline) = 0;
	di = (64);
	bx = (56)+20;
	cl = 104;
	ch = 60;
	multiDump();
}

void DreamGenContext::updateSymbolTop() {
	STACK_CHECK;
	_cmp(data.byte(kSymboltopdir), 0);
	if (flags.z())
		return /* (topfinished) */;
	_cmp(data.byte(kSymboltopdir), -1);
	if (flags.z())
		goto backwards;
	_inc(data.byte(kSymboltopx));
	_cmp(data.byte(kSymboltopx), 49);
	if (!flags.z())
		goto notwrapfor;
	data.byte(kSymboltopx) = 0;
	_dec(data.byte(kSymboltopnum));
	_cmp(data.byte(kSymboltopnum), -1);
	if (!flags.z())
		return /* (topfinished) */;
	data.byte(kSymboltopnum) = 5;
	return;
notwrapfor:
	_cmp(data.byte(kSymboltopx), 24);
	if (!flags.z())
		return /* (topfinished) */;
	data.byte(kSymboltopdir) = 0;
	return;
backwards:
	_dec(data.byte(kSymboltopx));
	_cmp(data.byte(kSymboltopx), -1);
	if (!flags.z())
		goto notwrapback;
	data.byte(kSymboltopx) = 48;
	_inc(data.byte(kSymboltopnum));
	_cmp(data.byte(kSymboltopnum), 6);
	if (!flags.z())
		return /* (topfinished) */;
	data.byte(kSymboltopnum) = 0;
	return;
notwrapback:
	_cmp(data.byte(kSymboltopx), 24);
	if (!flags.z())
		return /* (topfinished) */;
	data.byte(kSymboltopdir) = 0;
}

void DreamGenContext::updateSymbolBot() {
	STACK_CHECK;
	_cmp(data.byte(kSymbolbotdir), 0);
	if (flags.z())
		return /* (botfinished) */;
	_cmp(data.byte(kSymbolbotdir), -1);
	if (flags.z())
		goto backwardsbot;
	_inc(data.byte(kSymbolbotx));
	_cmp(data.byte(kSymbolbotx), 49);
	if (!flags.z())
		goto notwrapforb;
	data.byte(kSymbolbotx) = 0;
	_dec(data.byte(kSymbolbotnum));
	_cmp(data.byte(kSymbolbotnum), -1);
	if (!flags.z())
		return /* (botfinished) */;
	data.byte(kSymbolbotnum) = 5;
	return;
notwrapforb:
	_cmp(data.byte(kSymbolbotx), 24);
	if (!flags.z())
		return /* (botfinished) */;
	data.byte(kSymbolbotdir) = 0;
	return;
backwardsbot:
	_dec(data.byte(kSymbolbotx));
	_cmp(data.byte(kSymbolbotx), -1);
	if (!flags.z())
		goto notwrapbackb;
	data.byte(kSymbolbotx) = 48;
	_inc(data.byte(kSymbolbotnum));
	_cmp(data.byte(kSymbolbotnum), 6);
	if (!flags.z())
		return /* (botfinished) */;
	data.byte(kSymbolbotnum) = 0;
	return;
notwrapbackb:
	_cmp(data.byte(kSymbolbotx), 24);
	if (!flags.z())
		return /* (botfinished) */;
	data.byte(kSymbolbotdir) = 0;
}

void DreamGenContext::dumpSymBox() {
	STACK_CHECK;
	_cmp(data.word(kDumpx), -1);
	if (flags.z())
		return /* (nodumpsym) */;
	di = data.word(kDumpx);
	bx = data.word(kDumpy);
	cl = 30;
	ch = 77;
	multiDump();
	data.word(kDumpx) = -1;
}

void DreamGenContext::useDiary() {
	STACK_CHECK;
	getRidOfReels();
	dx = 1009;
	loadIntoTemp();
	dx = 1022;
	loadTempText();
	dx = 983;
	loadTempCharset();
	createPanel();
	showIcon();
	showDiary();
	underTextLine();
	showDiaryPage();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	data.byte(kGetback) = 0;
diaryloop:
	delPointer();
	readMouse();
	showDiaryKeys();
	showPointer();
	vSync();
	dumpPointer();
	dumpDiaryKeys();
	dumpTextLine();
	bx = offset_diarylist;
	checkCoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto diaryloop;
	getRidOfTemp();
	getRidOfTempText();
	getRidOfTempCharset();
	restoreReels();
	data.byte(kManisoffscreen) = 0;
	redrawMainScrn();
	workToScreenM();
}

void DreamGenContext::showDiary() {
	STACK_CHECK;
	al = 1;
	ah = 0;
	di = (68+24);
	bx = (48+12)+37;
	ds = data.word(kTempgraphics);
	showFrame();
	al = 2;
	ah = 0;
	di = (68+24)+176;
	bx = (48+12)+108;
	ds = data.word(kTempgraphics);
	showFrame();
}

void DreamGenContext::showDiaryKeys() {
	STACK_CHECK;
	_cmp(data.byte(kPresscount), 0);
	if (flags.z())
		return /* (nokeyatall) */;
	_dec(data.byte(kPresscount));
	_cmp(data.byte(kPresscount), 0);
	if (flags.z())
		return /* (nokeyatall) */;
	_cmp(data.byte(kPressed), 'N');
	if (!flags.z())
		goto nokeyn;
	al = 3;
	_cmp(data.byte(kPresscount), 1);
	if (flags.z())
		goto gotkeyn;
	al = 4;
gotkeyn:
	ah = 0;
	di = (68+24)+94;
	bx = (48+12)+97;
	ds = data.word(kTempgraphics);
	showFrame();
	_cmp(data.byte(kPresscount), 1);
	if (!flags.z())
		return /* (notshown) */;
	showDiaryPage();
	return;
nokeyn:
	al = 5;
	_cmp(data.byte(kPresscount), 1);
	if (flags.z())
		goto gotkeyp;
	al = 6;
gotkeyp:
	ah = 0;
	di = (68+24)+151;
	bx = (48+12)+71;
	ds = data.word(kTempgraphics);
	showFrame();
	_cmp(data.byte(kPresscount), 1);
	if (!flags.z())
		return /* (notshowp) */;
	showDiaryPage();
}

void DreamGenContext::dumpDiaryKeys() {
	STACK_CHECK;
	_cmp(data.byte(kPresscount), 1);
	if (!flags.z())
		goto notdumpdiary;
	_cmp(data.byte(kSartaindead), 1);
	if (flags.z())
		goto notsartadd;
	_cmp(data.byte(kDiarypage), 5);
	if (!flags.z())
		goto notsartadd;
	_cmp(data.byte(kDiarypage), 5);
	if (!flags.z())
		goto notsartadd;
	al = 6;
	getLocation();
	_cmp(al, 1);
	if (flags.z())
		goto notsartadd;
	al = 6;
	setLocation();
	delPointer();
	al = 12;
	findText1();
	di = 70;
	bx = 106;
	dl = 241;
	ah = 16;
	printDirect();
	workToScreenM();
	cx = 200;
	hangOnP();
	createPanel();
	showIcon();
	showDiary();
	showDiaryPage();
	workToScreenM();
	showPointer();
	return;
notsartadd:
	di = (68+24)+48;
	bx = (48+12)+15;
	cl = 200;
	ch = 16;
	multiDump();
notdumpdiary:
	di = (68+24)+94;
	bx = (48+12)+97;
	cl = 16;
	ch = 16;
	multiDump();
	di = (68+24)+151;
	bx = (48+12)+71;
	cl = 16;
	ch = 16;
	multiDump();
}

void DreamGenContext::diaryKeyP() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 214);
	if (flags.z())
		goto alreadykeyp;
	data.byte(kCommandtype) = 214;
	al = 23;
	commandOnly();
alreadykeyp:
	_cmp(data.word(kMousebutton), 0);
	if (flags.z())
		return /* (notkeyp) */;
	ax = data.word(kOldbutton);
	_cmp(ax, data.word(kMousebutton));
	if (flags.z())
		return /* (notkeyp) */;
	_cmp(data.byte(kPresscount), 0);
	if (!flags.z())
		return /* (notkeyp) */;
	al = 16;
	playChannel1();
	data.byte(kPresscount) = 12;
	data.byte(kPressed) = 'P';
	_dec(data.byte(kDiarypage));
	_cmp(data.byte(kDiarypage), -1);
	if (!flags.z())
		return /* (notkeyp) */;
	data.byte(kDiarypage) = 11;
}

void DreamGenContext::diaryKeyN() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 213);
	if (flags.z())
		goto alreadykeyn;
	data.byte(kCommandtype) = 213;
	al = 23;
	commandOnly();
alreadykeyn:
	_cmp(data.word(kMousebutton), 0);
	if (flags.z())
		return /* (notkeyn) */;
	ax = data.word(kOldbutton);
	_cmp(ax, data.word(kMousebutton));
	if (flags.z())
		return /* (notkeyn) */;
	_cmp(data.byte(kPresscount), 0);
	if (!flags.z())
		return /* (notkeyn) */;
	al = 16;
	playChannel1();
	data.byte(kPresscount) = 12;
	data.byte(kPressed) = 'N';
	_inc(data.byte(kDiarypage));
	_cmp(data.byte(kDiarypage), 12);
	if (!flags.z())
		return /* (notkeyn) */;
	data.byte(kDiarypage) = 0;
}

void DreamGenContext::showDiaryPage() {
	STACK_CHECK;
	al = 0;
	ah = 0;
	di = (68+24);
	bx = (48+12);
	ds = data.word(kTempgraphics);
	showFrame();
	al = data.byte(kDiarypage);
	findText1();
	data.byte(kKerning) = 1;
	useTempCharset();
	di = (68+24)+48;
	bx = (48+12)+16;
	dl = 240;
	ah = 16;
	data.word(kCharshift) = 91+91;
	printDirect();
	di = (68+24)+129;
	bx = (48+12)+16;
	dl = 240;
	ah = 16;
	printDirect();
	di = (68+24)+48;
	bx = (48+12)+23;
	dl = 240;
	ah = 16;
	printDirect();
	data.byte(kKerning) = 0;
	data.word(kCharshift) = 0;
	useCharset1();
}

void DreamGenContext::findText1() {
	STACK_CHECK;
	ah = 0;
	si = ax;
	_add(si, si);
	es = data.word(kTextfile1);
	ax = es.word(si);
	_add(ax, (66*2));
	si = ax;
}

void DreamGenContext::doSaveLoad() {
	STACK_CHECK;
	data.byte(kPointerframe) = 0;
	data.word(kTextaddressx) = 70;
	data.word(kTextaddressy) = 182-8;
	data.byte(kTextlen) = 181;
	data.byte(kManisoffscreen) = 1;
	clearWork();
	createPanel2();
	underTextLine();
	getRidOfAll();
	loadSaveBox();
	showOpBox();
	showMainOps();
	workToScreen();
	goto donefirstops;
restartops:
	showOpBox();
	showMainOps();
	workToScreenM();
donefirstops:
	data.byte(kGetback) = 0;
waitops:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		goto justret;
	readMouse();
	showPointer();
	vSync();
	dumpPointer();
	dumpTextLine();
	delPointer();
	bx = offset_opslist;
	checkCoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto waitops;
	_cmp(data.byte(kGetback), 2);
	if (flags.z())
		goto restartops;
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	_cmp(data.byte(kGetback), 4);
	if (flags.z())
		goto justret;
	getRidOfTemp();
	restoreAll();
	redrawMainScrn();
	workToScreenM();
	data.byte(kCommandtype) = 200;
justret:
	data.byte(kManisoffscreen) = 0;
}

void DreamGenContext::discOps() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 249);
	if (flags.z())
		goto alreadydiscops;
	data.byte(kCommandtype) = 249;
	al = 43;
	commandOnly();
alreadydiscops:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (nodiscops) */;
	_and(ax, 1);
	if (!flags.z())
		goto dodiscops;
	return;
dodiscops:
	scanForNames();
	data.byte(kLoadingorsave) = 2;
	showOpBox();
	showDiscOps();
	data.byte(kCurrentslot) = 0;
	workToScreenM();
	data.byte(kGetback) = 0;
discopsloop:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (quitdiscops) */;
	delPointer();
	readMouse();
	showPointer();
	vSync();
	dumpPointer();
	dumpTextLine();
	bx = offset_discopslist;
	checkCoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto discopsloop;
}

void DreamGenContext::checkInput() {
	STACK_CHECK;
	_cmp(data.byte(kLoadingorsave), 3);
	if (flags.z())
		return /* (nokeypress) */;
	readKey();
	al = data.byte(kCurrentkey);
	_cmp(al, 0);
	if (flags.z())
		return /* (nokeypress) */;
	_cmp(al, 13);
	if (!flags.z())
		goto notret;
	data.byte(kLoadingorsave) = 3;
	goto afterkey;
notret:
	_cmp(al, 8);
	if (!flags.z())
		goto nodel2;
	_cmp(data.byte(kCursorpos), 0);
	if (flags.z())
		return /* (nokeypress) */;
	getNamePos();
	_dec(data.byte(kCursorpos));
	es.byte(bx) = 0;
	es.byte(bx+1) = 1;
	goto afterkey;
nodel2:
	_cmp(data.byte(kCursorpos), 14);
	if (flags.z())
		return /* (nokeypress) */;
	getNamePos();
	_inc(data.byte(kCursorpos));
	al = data.byte(kCurrentkey);
	es.byte(bx+1) = al;
	es.byte(bx+2) = 0;
	es.byte(bx+3) = 1;
	goto afterkey;
	return;
afterkey:
	showOpBox();
	showNames();
	showSlots();
	showSaveOps();
	workToScreenM();
}

void DreamGenContext::getNamePos() {
	STACK_CHECK;
	al = data.byte(kCurrentslot);
	ah = 0;
	cx = 17;
	_mul(cx);
	dx = data;
	es = dx;
	bx = 1891;
	_add(bx, ax);
	al = data.byte(kCursorpos);
	ah = 0;
	_add(bx, ax);
}

void DreamGenContext::showOpBox() {
	STACK_CHECK;
	ds = data.word(kTempgraphics);
	di = (60);
	bx = (52);
	al = 0;
	ah = 0;
	showFrame();
	ds = data.word(kTempgraphics);
	di = (60);
	bx = (52)+55;
	al = 4;
	ah = 0;
	showFrame();
}

void DreamGenContext::showLoadOps() {
	STACK_CHECK;
	ds = data.word(kTempgraphics);
	di = (60)+128+4;
	bx = (52)+12;
	al = 1;
	ah = 0;
	showFrame();
	ds = data.word(kTempgraphics);
	di = (60)+176+2;
	bx = (52)+60-4;
	al = 5;
	ah = 0;
	showFrame();
	di = (60)+104;
	bx = (52)+14;
	al = 55;
	dl = 101;
	printMessage();
}

void DreamGenContext::showSaveOps() {
	STACK_CHECK;
	ds = data.word(kTempgraphics);
	di = (60)+128+4;
	bx = (52)+12;
	al = 1;
	ah = 0;
	showFrame();
	ds = data.word(kTempgraphics);
	di = (60)+176+2;
	bx = (52)+60-4;
	al = 5;
	ah = 0;
	showFrame();
	di = (60)+104;
	bx = (52)+14;
	al = 54;
	dl = 101;
	printMessage();
}

void DreamGenContext::selectSlot() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 244);
	if (flags.z())
		goto alreadysel;
	data.byte(kCommandtype) = 244;
	al = 45;
	commandOnly();
alreadysel:
	ax = data.word(kMousebutton);
	_cmp(ax, 1);
	if (!flags.z())
		return /* (noselslot) */;
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (noselslot) */;
	_cmp(data.byte(kLoadingorsave), 3);
	if (!flags.z())
		goto notnocurs;
	_dec(data.byte(kLoadingorsave));
notnocurs:
	oldToNames();
	ax = data.word(kMousey);
	_sub(ax, (52)+4);
	cl = -1;
getslotnum:
	_inc(cl);
	_sub(ax, 11);
	if (!flags.c())
		goto getslotnum;
	data.byte(kCurrentslot) = cl;
	delPointer();
	showOpBox();
	showSlots();
	showNames();
	_cmp(data.byte(kLoadingorsave), 1);
	if (flags.z())
		goto isloadmode;
	showSaveOps();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	return;
isloadmode:
	showLoadOps();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::showSlots() {
	STACK_CHECK;
	di = (60)+7;
	bx = (52)+8;
	al = 2;
	ds = data.word(kTempgraphics);
	ah = 0;
	showFrame();
	di = (60)+10;
	bx = (52)+11;
	cl = 0;
slotloop:
	push(cx);
	push(di);
	push(bx);
	_cmp(cl, data.byte(kCurrentslot));
	if (!flags.z())
		goto nomatchslot;
	al = 3;
	ds = data.word(kTempgraphics);
	ah = 0;
	showFrame();
nomatchslot:
	bx = pop();
	di = pop();
	cx = pop();
	_add(bx, 10);
	_inc(cl);
	_cmp(cl, 7);
	if (!flags.z())
		goto slotloop;
}

void DreamGenContext::showNames() {
	STACK_CHECK;
	dx = data;
	es = dx;
	si = 1891+1;
	di = (60)+21;
	bx = (52)+10;
	cl = 0;
shownameloop:
	push(cx);
	push(di);
	push(es);
	push(bx);
	push(si);
	al = 4;
	_cmp(cl, data.byte(kCurrentslot));
	if (!flags.z())
		goto nomatchslot2;
	_cmp(data.byte(kLoadingorsave), 2);
	if (!flags.z())
		goto loadmode;
	dx = si;
	cx = 15;
	_add(si, 15);
zerostill:
	_dec(si);
	_dec(cl);
	_cmp(es.byte(si), 1);
	if (!flags.z())
		goto foundcharacter;
	goto zerostill;
foundcharacter:
	data.byte(kCursorpos) = cl;
	es.byte(si) = '/';
	es.byte(si+1) = 0;
	push(si);
	si = dx;
	dl = 200;
	ah = 0;
	printDirect();
	si = pop();
	es.byte(si) = 0;
	es.byte(si+1) = 1;
	goto afterprintname;
loadmode:
	al = 0;
	dl = 200;
	ah = 0;
	data.word(kCharshift) = 91;
	printDirect();
	data.word(kCharshift) = 0;
	goto afterprintname;
nomatchslot2:
	dl = 200;
	ah = 0;
	printDirect();
afterprintname:
	si = pop();
	bx = pop();
	es = pop();
	di = pop();
	cx = pop();
	_add(si, 17);
	_add(bx, 10);
	_inc(cl);
	_cmp(cl, 7);
	if (!flags.z())
		goto shownameloop;
}

void DreamGenContext::decide() {
	STACK_CHECK;
	setMode();
	loadPalFromIFF();
	clearPalette();
	data.byte(kPointermode) = 0;
	data.word(kWatchingtime) = 0;
	data.byte(kPointerframe) = 0;
	data.word(kTextaddressx) = 70;
	data.word(kTextaddressy) = 182-8;
	data.byte(kTextlen) = 181;
	data.byte(kManisoffscreen) = 1;
	loadSaveBox();
	showDecisions();
	workToScreen();
	fadeScreenUp();
	data.byte(kGetback) = 0;
waitdecide:
	_cmp(data.byte(kQuitrequested),  0);
	if (flags.z())
		goto _tmp1;
	return;
_tmp1:
	readMouse();
	showPointer();
	vSync();
	dumpPointer();
	dumpTextLine();
	delPointer();
	bx = offset_decidelist;
	checkCoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto waitdecide;
	_cmp(data.byte(kGetback), 4);
	if (flags.z())
		goto hasloadedroom;
	getRidOfTemp();
hasloadedroom:
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
}

void DreamGenContext::showDecisions() {
	STACK_CHECK;
	createPanel2();
	showOpBox();
	ds = data.word(kTempgraphics);
	di = (60)+17;
	bx = (52)+13;
	al = 6;
	ah = 0;
	showFrame();
	underTextLine();
}

void DreamGenContext::createName() {
	STACK_CHECK;
	push(ax);
	di = offset_speechfile;
	cs.byte(di+0) = dl;
	cs.byte(di+3) = cl;
	al = dh;
	ah = '0'-1;
findten:
	_inc(ah);
	_sub(al, 10);
	if (!flags.c())
		goto findten;
	cs.byte(di+1) = ah;
	_add(al, 10+'0');
	cs.byte(di+2) = al;
	ax = pop();
	cl = '0'-1;
thousandsc:
	_inc(cl);
	_sub(ax, 1000);
	if (!flags.c())
		goto thousandsc;
	_add(ax, 1000);
	cs.byte(di+4) = cl;
	cl = '0'-1;
hundredsc:
	_inc(cl);
	_sub(ax, 100);
	if (!flags.c())
		goto hundredsc;
	_add(ax, 100);
	cs.byte(di+5) = cl;
	cl = '0'-1;
tensc:
	_inc(cl);
	_sub(ax, 10);
	if (!flags.c())
		goto tensc;
	_add(ax, 10);
	cs.byte(di+6) = cl;
	_add(al, '0');
	cs.byte(di+7) = al;
}

void DreamGenContext::volumeAdjust() {
	STACK_CHECK;
	al = data.byte(kVolumedirection);
	_cmp(al, 0);
	if (flags.z())
		return /* (volok) */;
	al = data.byte(kVolume);
	_cmp(al, data.byte(kVolumeto));
	if (flags.z())
		goto volfinish;
	_add(data.byte(kVolumecount), 64);
	if (!flags.z())
		return /* (volok) */;
	al = data.byte(kVolume);
	_add(al, data.byte(kVolumedirection));
	data.byte(kVolume) = al;
	return;
volfinish:
	data.byte(kVolumedirection) = 0;
}

void DreamGenContext::entryTexts() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 21);
	if (!flags.z())
		goto notloc15;
	al = 28;
	cx = 60;
	dx = 11;
	bl = 68;
	bh = 64;
	setupTimedUse();
	return;
notloc15:
	_cmp(data.byte(kLocation), 30);
	if (!flags.z())
		goto notloc43;
	al = 27;
	cx = 60;
	dx = 11;
	bl = 68;
	bh = 64;
	setupTimedUse();
	return;
notloc43:
	_cmp(data.byte(kLocation), 23);
	if (!flags.z())
		goto notloc23;
	al = 29;
	cx = 60;
	dx = 11;
	bl = 68;
	bh = 64;
	setupTimedUse();
	return;
notloc23:
	_cmp(data.byte(kLocation), 31);
	if (!flags.z())
		goto notloc44;
	al = 30;
	cx = 60;
	dx = 11;
	bl = 68;
	bh = 64;
	setupTimedUse();
	return;
notloc44:
	_cmp(data.byte(kLocation), 20);
	if (!flags.z())
		goto notsarters2;
	al = 31;
	cx = 60;
	dx = 11;
	bl = 68;
	bh = 64;
	setupTimedUse();
	return;
notsarters2:
	_cmp(data.byte(kLocation), 24);
	if (!flags.z())
		goto notedenlob;
	al = 32;
	cx = 60;
	dx = 3;
	bl = 68;
	bh = 64;
	setupTimedUse();
	return;
notedenlob:
	_cmp(data.byte(kLocation), 34);
	if (!flags.z())
		return /* (noteden2) */;
	al = 33;
	cx = 60;
	dx = 3;
	bl = 68;
	bh = 64;
	setupTimedUse();
}

void DreamGenContext::entryAnims() {
	STACK_CHECK;
	data.word(kReeltowatch) = -1;
	data.byte(kWatchmode) = -1;
	_cmp(data.byte(kLocation), 33);
	if (!flags.z())
		goto notinthebeach;
	switchRyanOff();
	data.word(kWatchingtime) = 76*2;
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 76;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	return;
notinthebeach:
	_cmp(data.byte(kLocation), 44);
	if (!flags.z())
		goto notsparkys;
	al = 8;
	resetLocation();
	data.word(kWatchingtime) = 50*2;
	data.word(kReeltowatch) = 247;
	data.word(kEndwatchreel) = 297;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	switchRyanOff();
	return;
notsparkys:
	_cmp(data.byte(kLocation), 22);
	if (!flags.z())
		goto notinthelift;
	data.word(kWatchingtime) = 31*2;
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 30;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	switchRyanOff();
	return;
notinthelift:
	_cmp(data.byte(kLocation), 26);
	if (!flags.z())
		goto notunderchurch;
	data.byte(kSymboltopnum) = 2;
	data.byte(kSymbolbotnum) = 1;
	return;
notunderchurch:
	_cmp(data.byte(kLocation), 45);
	if (!flags.z())
		goto notenterdream;
	data.byte(kKeeperflag) = 0;
	data.word(kWatchingtime) = 296;
	data.word(kReeltowatch) = 45;
	data.word(kEndwatchreel) = 198;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	switchRyanOff();
	return;
notenterdream:
	_cmp(data.byte(kReallocation), 46);
	if (!flags.z())
		goto notcrystal;
	_cmp(data.byte(kSartaindead), 1);
	if (!flags.z())
		goto notcrystal;
	al = 0;
	removeFreeObject();
	return;
notcrystal:
	_cmp(data.byte(kLocation), 9);
	if (!flags.z())
		goto nottopchurch;
	al = 2;
	checkIfPathIsOn();
	if (flags.z())
		goto nottopchurch;
	_cmp(data.byte(kAidedead), 0);
	if (flags.z())
		goto nottopchurch;
	al = 3;
	checkIfPathIsOn();
	if (!flags.z())
		goto makedoorsopen;
	al = 2;
	turnPathOn();
makedoorsopen:
	al = 4;
	removeSetObject();
	al = 5;
	placeSetObject();
	return;
nottopchurch:
	_cmp(data.byte(kLocation), 47);
	if (!flags.z())
		goto notdreamcentre;
	al = 4;
	placeSetObject();
	al = 5;
	placeSetObject();
	return;
notdreamcentre:
	_cmp(data.byte(kLocation), 38);
	if (!flags.z())
		goto notcarpark;
	data.word(kWatchingtime) = 57*2;
	data.word(kReeltowatch) = 4;
	data.word(kEndwatchreel) = 57;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	switchRyanOff();
	return;
notcarpark:
	_cmp(data.byte(kLocation), 32);
	if (!flags.z())
		goto notalley;
	data.word(kWatchingtime) = 66*2;
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 66;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	switchRyanOff();
	return;
notalley:
	_cmp(data.byte(kLocation), 24);
	if (!flags.z())
		return /* (notedensagain) */;
	al = 2;
	ah = data.byte(kRoomnum);
	_dec(ah);
	turnAnyPathOn();
}

void DreamGenContext::clearBuffers() {
	STACK_CHECK;
	es = data.word(kBuffers);
	cx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+983-526+68-0)/2;
	ax = 0;
	di = 0;
	_stosw(cx, true);
	es = data.word(kExtras);
	cx = (0+2080+30000+(16*114)+((114+2)*2)+18000)/2;
	ax = 0x0ffff;
	di = 0;
	_stosw(cx, true);
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64));
	ds = cs;
	si = 526;
	cx = (983-526);
	_movsb(cx, true);
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+983-526);
	ds = cs;
	si = 0;
	cx = (68-0);
	_movsb(cx, true);
	clearChanges();
}

void DreamGenContext::clearChanges() {
	STACK_CHECK;
	es = data.word(kBuffers);
	cx = (250)*2;
	ax = 0x0ffff;
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
	_stosw(cx, true);
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64));
	es = cs;
	di = 526;
	cx = (983-526);
	_movsb(cx, true);
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+983-526);
	es = cs;
	di = 0;
	cx = (68-0);
	_movsb(cx, true);
	data.byte(kExpos) = 0;
	data.word(kExframepos) = 0;
	data.word(kExtextpos) = 0;
	es = data.word(kExtras);
	cx = (0+2080+30000+(16*114)+((114+2)*2)+18000)/2;
	ax = 0x0ffff;
	di = 0;
	_stosw(cx, true);
	es = cs;
	di = 1723;
	al = 1;
	_stosb(2);
	al = 0;
	_stosb();
	al = 1;
	_stosb();
	ax = 0;
	cx = 6;
	_stosw(cx, true);
}

void DreamGenContext::clearRest() {
	STACK_CHECK;
	es = data.word(kMapdata);
	cx = (66*60)/2;
	ax = 0;
	di = (0);
	_stosw(cx, true);
	es = data.word(kBackdrop);
	deallocateMem();
	es = data.word(kSetframes);
	deallocateMem();
	es = data.word(kReels);
	deallocateMem();
	es = data.word(kPeople);
	deallocateMem();
	es = data.word(kSetdesc);
	deallocateMem();
	es = data.word(kBlockdesc);
	deallocateMem();
	es = data.word(kRoomdesc);
	deallocateMem();
	es = data.word(kFreeframes);
	deallocateMem();
	es = data.word(kFreedesc);
	deallocateMem();
}

void DreamGenContext::watchReel() {
	STACK_CHECK;
	_cmp(data.word(kReeltowatch), -1);
	if (flags.z())
		goto notplayingreel;
	al = data.byte(kManspath);
	_cmp(al, data.byte(kFinaldest));
	if (!flags.z())
		return /* (waitstopwalk) */;
	al = data.byte(kTurntoface);
	_cmp(al, data.byte(kFacing));
	if (flags.z())
		goto notwatchpath;
	return;
notwatchpath:
	_dec(data.byte(kSpeedcount));
	_cmp(data.byte(kSpeedcount), -1);
	if (!flags.z())
		goto showwatchreel;
	al = data.byte(kWatchspeed);
	data.byte(kSpeedcount) = al;
	ax = data.word(kReeltowatch);
	_cmp(ax, data.word(kEndwatchreel));
	if (!flags.z())
		goto ismorereel;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		goto showwatchreel;
	data.word(kReeltowatch) = -1;
	data.byte(kWatchmode) = -1;
	_cmp(data.word(kReeltohold), -1);
	if (flags.z())
		return /* (nomorereel) */;
	data.byte(kWatchmode) = 1;
	goto notplayingreel;
ismorereel:
	_inc(data.word(kReeltowatch));
showwatchreel:
	ax = data.word(kReeltowatch);
	data.word(kReelpointer) = ax;
	plotReel();
	ax = data.word(kReelpointer);
	data.word(kReeltowatch) = ax;
	checkForShake();
	return;
notplayingreel:
	_cmp(data.byte(kWatchmode), 1);
	if (!flags.z())
		goto notholdingreel;
	ax = data.word(kReeltohold);
	data.word(kReelpointer) = ax;
	plotReel();
	return;
notholdingreel:
	_cmp(data.byte(kWatchmode), 2);
	if (!flags.z())
		return /* (notreleasehold) */;
	_dec(data.byte(kSpeedcount));
	_cmp(data.byte(kSpeedcount), -1);
	if (!flags.z())
		goto notlastspeed2;
	al = data.byte(kWatchspeed);
	data.byte(kSpeedcount) = al;
	_inc(data.word(kReeltohold));
notlastspeed2:
	ax = data.word(kReeltohold);
	_cmp(ax, data.word(kEndofholdreel));
	if (!flags.z())
		goto ismorereel2;
	data.word(kReeltohold) = -1;
	data.byte(kWatchmode) = -1;
	al = data.byte(kDestafterhold);
	data.byte(kDestination) = al;
	data.byte(kFinaldest) = al;
	autoSetWalk();
	return;
ismorereel2:
	ax = data.word(kReeltohold);
	data.word(kReelpointer) = ax;
	plotReel();
}

void DreamGenContext::checkForShake() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 26);
	if (!flags.z())
		return /* (notstartshake) */;
	_cmp(ax, 104);
	if (!flags.z())
		return /* (notstartshake) */;
	data.byte(kShakecounter) = -1;
}

void DreamGenContext::madmanRun() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 14);
	if (!flags.z())
		{ identifyOb(); return; };
	_cmp(data.byte(kMapx), 22);
	if (!flags.z())
		{ identifyOb(); return; };
	_cmp(data.byte(kPointermode), 2);
	if (!flags.z())
		{ identifyOb(); return; };
	_cmp(data.byte(kMadmanflag), 0);
	if (!flags.z())
		{ identifyOb(); return; };
	_cmp(data.byte(kCommandtype), 211);
	if (flags.z())
		goto alreadyrun;
	data.byte(kCommandtype) = 211;
	al = 52;
	commandOnly();
alreadyrun:
	_cmp(data.word(kMousebutton), 1);
	if (!flags.z())
		return /* (norun) */;
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (norun) */;
	data.byte(kLastweapon) = 8;
}

void DreamGenContext::identifyOb() {
	STACK_CHECK;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		{ blank(); return; };
	ax = data.word(kMousex);
	_sub(ax, data.word(kMapadx));
	_cmp(ax, 22*8);
	if (flags.c())
		goto notover1;
	blank();
	return;
notover1:
	bx = data.word(kMousey);
	_sub(bx, data.word(kMapady));
	_cmp(bx, 20*8);
	if (flags.c())
		goto notover2;
	blank();
	return;
notover2:
	data.byte(kInmaparea) = 1;
	ah = bl;
	push(ax);
	findPathOfPoint();
	data.byte(kPointerspath) = dl;
	ax = pop();
	push(ax);
	findFirstPath();
	data.byte(kPointerfirstpath) = al;
	ax = pop();
	checkIfEx();
	if (!flags.z())
		return /* (finishidentify) */;
	checkIfFree();
	if (!flags.z())
		return /* (finishidentify) */;
	checkIfPerson();
	if (!flags.z())
		return /* (finishidentify) */;
	checkIfSet();
	if (!flags.z())
		return /* (finishidentify) */;
	ax = data.word(kMousex);
	_sub(ax, data.word(kMapadx));
	cl = al;
	ax = data.word(kMousey);
	_sub(ax, data.word(kMapady));
	ch = al;
	checkOne();
	_cmp(al, 0);
	if (flags.z())
		goto nothingund;
	_cmp(data.byte(kMandead), 1);
	if (flags.z())
		goto nothingund;
	ah = 3;
	obName();
	return;
nothingund:
	blank();
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

void DreamGenContext::afterNewRoom() {
	STACK_CHECK;
	_cmp(data.byte(kNowinnewroom), 0);
	if (flags.z())
		return /* (notnew) */;
	data.word(kTimecount) = 0;
	createPanel();
	data.byte(kCommandtype) = 0;
	findRoomInLoc();
	_cmp(data.byte(kRyanon), 1);
	if (flags.z())
		goto ryansoff;
	al = data.byte(kRyanx);
	_add(al, 12);
	ah = data.byte(kRyany);
	_add(ah, 12);
	findPathOfPoint();
	data.byte(kManspath) = dl;
	findXYFromPath();
	data.byte(kResetmanxy) = 1;
ryansoff:
	data.byte(kNewobs) = 1;
	drawFloor();
	data.word(kLookcounter) = 160;
	data.byte(kNowinnewroom) = 0;
	showIcon();
	spriteUpdate();
	printSprites();
	underTextLine();
	reelsOnScreen();
	mainScreen();
	getUnderZoom();
	zoom();
	workToScreenM();
	walkIntoRoom();
	reminders();
	atmospheres();
}

void DreamGenContext::printmessage2() {
	STACK_CHECK;
	push(dx);
	push(bx);
	push(di);
	push(ax);
	ah = 0;
	_add(ax, ax);
	bx = ax;
	es = data.word(kCommandtext);
	ax = es.word(bx);
	_add(ax, (66*2));
	si = ax;
	ax = pop();
searchmess:
	push(ax);
	findNextColon();
	ax = pop();
	_dec(ah);
	if (!flags.z())
		goto searchmess;
	di = pop();
	bx = pop();
	dx = pop();
	al = 0;
	ah = 0;
	printDirect();
}

void DreamGenContext::workoutFrames() {
	STACK_CHECK;
	bx = data.word(kLinestartx);
	_add(bx, 32);
	ax = data.word(kLineendx);
	_add(ax, 32);
	_sub(bx, ax);
	if (!flags.c())
		goto notneg1;
	_neg(bx);
notneg1:
	cx = data.word(kLinestarty);
	_add(cx, 32);
	ax = data.word(kLineendy);
	_add(ax, 32);
	_sub(cx, ax);
	if (!flags.c())
		goto notneg2;
	_neg(cx);
notneg2:
	_cmp(bx, cx);
	if (!flags.c())
		goto tendstohoriz;
	dl = 2;
	ax = cx;
	_shr(ax, 1);
	_cmp(bx, ax);
	if (flags.c())
		goto gotquad;
	dl = 1;
	goto gotquad;
tendstohoriz:
	dl = 0;
	ax = bx;
	_shr(ax, 1);
	_cmp(cx, ax);
	if (flags.c())
		goto gotquad;
	dl = 1;
	goto gotquad;
gotquad:
	bx = data.word(kLinestartx);
	_add(bx, 32);
	ax = data.word(kLineendx);
	_add(ax, 32);
	_sub(bx, ax);
	if (flags.c())
		goto isinright;
	cx = data.word(kLinestarty);
	_add(cx, 32);
	ax = data.word(kLineendy);
	_add(ax, 32);
	_sub(cx, ax);
	if (!flags.c())
		goto topleft;
	_cmp(dl, 1);
	if (flags.z())
		goto noswap1;
	_xor(dl, 2);
noswap1:
	_add(dl, 4);
	goto success;
topleft:
	_add(dl, 6);
	goto success;
isinright:
	cx = data.word(kLinestarty);
	_add(cx, 32);
	ax = data.word(kLineendy);
	_add(ax, 32);
	_sub(cx, ax);
	if (!flags.c())
		goto botright;
	_add(dl, 2);
	goto success;
botright:
	_cmp(dl, 1);
	if (flags.z())
		goto noswap2;
	_xor(dl, 2);
noswap2:
success:
	_and(dl, 7);
	data.byte(kTurntoface) = dl;
	data.byte(kTurndirection) = 0;
}

void DreamGenContext::middlePanel() {
	STACK_CHECK;
	ds = data.word(kTempsprites);
	di = 72+47+20;
	bx = 0;
	al = 48;
	ah = 0;
	showFrame();
	ds = data.word(kTempsprites);
	di = 72+19;
	bx = 21;
	al = 47;
	ah = 0;
	showFrame();
	ds = data.word(kTempsprites);
	di = 160+23;
	bx = 0;
	al = 48;
	ah = 4;
	showFrame();
	ds = data.word(kTempsprites);
	di = 160+71;
	bx = 21;
	al = 47;
	ah = 4;
	showFrame();
}

void DreamGenContext::getUnderZoom() {
	STACK_CHECK;
	di = (8)+5;
	bx = (132)+4;
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
	cl = 46;
	ch = 40;
	multiGet();
}

void DreamGenContext::dumpZoom() {
	STACK_CHECK;
	_cmp(data.byte(kZoomon), 1);
	if (!flags.z())
		return /* (notzoomon) */;
	di = (8)+5;
	bx = (132)+4;
	cl = 46;
	ch = 40;
	multiDump();
}

void DreamGenContext::putUnderZoom() {
	STACK_CHECK;
	di = (8)+5;
	bx = (132)+4;
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
	cl = 46;
	ch = 40;
	multiPut();
}

void DreamGenContext::underTextLine() {
	STACK_CHECK;
	di = data.word(kTextaddressx);
	bx = data.word(kTextaddressy);
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto _tmp1;
	_sub(bx, 3);
_tmp1:
	ds = data.word(kBuffers);
	si = (0);
	cl = (228);
	ch = (13);
	multiGet();
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
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
		//0x00b0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00c0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00d0: .... .... .... ....
		0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x00e0: ..   .... .... ....
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
		0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
		//0x0160: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0170: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0180: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0190: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		//0x01a0: .... .... .... ....
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		//0x01b0: .... .... .... ....
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0x00, 
		//0x01c0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x01d0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x01e0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x01f0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x01, 0x2c, 
		//0x0200: .... .... .... ...,
		0x00, 0x14, 0x00, 0x02, 0x00, 0x01, 0x01, 0x37, 0x00, 0x00, 0x00, 0x32, 0x14, 0x00, 0x18, 0x16, 
		//0x0210: .... ...7 ...2 ....
		0x00, 0x4a, 0x00, 0x01, 0x00, 0x00, 0x18, 0x21, 0x0a, 0x4b, 0x00, 0x01, 0x00, 0x01, 0x01, 0x2c, 
		//0x0220: .J.. ...! .K.. ...,
		0x00, 0x1b, 0x00, 0x02, 0x00, 0x02, 0x01, 0x2c, 0x00, 0x60, 0x00, 0x03, 0x00, 0x04, 0x01, 0x2c, 
		//0x0230: .... ..., .`.. ...,
		0x00, 0x76, 0x00, 0x02, 0x00, 0x05, 0x01, 0x2c, 0x0a, 0x00, 0x00, 0x02, 0x00, 0x00, 0x05, 0x16, 
		//0x0240: .v.. ..., .... ....
		0x14, 0x35, 0x00, 0x03, 0x00, 0x00, 0x05, 0x16, 0x14, 0x28, 0x00, 0x01, 0x00, 0x02, 0x05, 0x16, 
		//0x0250: .5.. .... .(.. ....
		0x14, 0x32, 0x00, 0x01, 0x00, 0x03, 0x02, 0x0b, 0x0a, 0xc0, 0x00, 0x01, 0x00, 0x00, 0x02, 0x0b, 
		//0x0260: .2.. .... .... ....
		0x0a, 0xb6, 0x00, 0x02, 0x00, 0x01, 0x08, 0x0b, 0x0a, 0x00, 0x00, 0x02, 0x00, 0x01, 0x17, 0x00, 
		//0x0270: .... .... .... ....
		0x32, 0x00, 0x00, 0x03, 0x00, 0x00, 0x1c, 0x0b, 0x14, 0xfa, 0x00, 0x04, 0x00, 0x00, 0x17, 0x00, 
		//0x0280: 2... .... .... ....
		0x32, 0x2b, 0x00, 0x02, 0x00, 0x08, 0x17, 0x0b, 0x28, 0x82, 0x00, 0x02, 0x00, 0x01, 0x17, 0x16, 
		//0x0290: 2+.. .... (... ....
		0x28, 0x7a, 0x00, 0x02, 0x00, 0x02, 0x17, 0x16, 0x28, 0x69, 0x00, 0x02, 0x00, 0x03, 0x17, 0x16, 
		//0x02a0: (z.. .... (i.. ....
		0x28, 0x51, 0x00, 0x02, 0x00, 0x04, 0x17, 0x0b, 0x28, 0x87, 0x00, 0x02, 0x00, 0x05, 0x17, 0x16, 
		//0x02b0: (Q.. .... (... ....
		0x28, 0x91, 0x00, 0x02, 0x00, 0x06, 0x04, 0x16, 0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x2d, 0x16, 
		//0x02c0: (... .... .... ..-.
		0x1e, 0xc8, 0x00, 0x00, 0x00, 0x14, 0x2d, 0x16, 0x1e, 0x27, 0x00, 0x02, 0x00, 0x00, 0x2d, 0x16, 
		//0x02d0: .... ..-. .'.. ..-.
		0x1e, 0x19, 0x00, 0x02, 0x00, 0x00, 0x08, 0x16, 0x28, 0x20, 0x00, 0x02, 0x00, 0x00, 0x07, 0x0b, 
		//0x02e0: .... .... ( .. ....
		0x14, 0x40, 0x00, 0x02, 0x00, 0x00, 0x16, 0x16, 0x14, 0x52, 0x00, 0x02, 0x00, 0x00, 0x1b, 0x0b, 
		//0x02f0: .@.. .... .R.. ....
		0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x14, 0x00, 0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x0e, 0x21, 
		//0x0300: .... .... .... ...!
		0x28, 0x15, 0x00, 0x01, 0x00, 0x00, 0x1d, 0x0b, 0x0a, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 
		//0x0310: (... .... .... ....
		0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x19, 0x00, 0x32, 0x04, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 
		//0x0320: .... .... 2... ..2.
		0x1e, 0x79, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 0x1e, 0x00, 0x00, 0x14, 0x00, 0x00, 0x34, 0x16, 
		//0x0330: .y.. ..2. .... ..4.
		0x1e, 0xc0, 0x00, 0x02, 0x00, 0x00, 0x34, 0x16, 0x1e, 0xe9, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 
		//0x0340: .... ..4. .... ..2.
		0x28, 0x68, 0x00, 0x37, 0x00, 0x00, 0x35, 0x21, 0x00, 0x63, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 
		//0x0350: (h.7 ..5! .c.. ..2.
		0x28, 0x00, 0x00, 0x03, 0x00, 0x00, 0x32, 0x16, 0x1e, 0xa2, 0x00, 0x02, 0x00, 0x00, 0x34, 0x16, 
		//0x0360: (... ..2. .... ..4.
		0x1e, 0x39, 0x00, 0x02, 0x00, 0x00, 0x34, 0x16, 0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x36, 0x00, 
		//0x0370: .9.. ..4. .... ..6.
		0x00, 0x48, 0x00, 0x03, 0x00, 0x00, 0x37, 0x2c, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x13, 0x00, 
		//0x0380: .H.. ..7, .... ....
		0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x0e, 0x16, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x0e, 0x16, 
		//0x0390: .... .... .... ....
		0x00, 0x2c, 0x01, 0x01, 0x00, 0x00, 0x0a, 0x16, 0x1e, 0xae, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x16, 
		//0x03a0: .,.. .... .... ....
		0x14, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0b, 0x0b, 0x14, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0b, 0x0b, 
		//0x03b0: .... .... ...2 ....
		0x1e, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0b, 0x16, 0x14, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0e, 0x21, 
		//0x03c0: ...2 .... ...2 ...!
		0x28, 0x00, 0x00, 0x32, 0x14, 0x00, 0xff, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 
		//0x03d0: (..2 ...D REAM WEB.
		0x43, 0x30, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x56, 0x39, 0x39, 
		//0x03e0: C02. DREA MWEB .V99
		0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 0x34, 0x00, 0x44, 0x52, 
		//0x03f0: .DRE AMWE B.G1 4.DR
		0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x35, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		//0x0400: EAMW EB.T 51.D REAM
		0x57, 0x45, 0x42, 0x2e, 0x54, 0x38, 0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 
		//0x0410: WEB. T83. DREA MWEB
		0x2e, 0x56, 0x4f, 0x4c, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 
		//0x0420: .VOL .DRE AMWE B.G1
		0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 0x35, 0x00, 0x44, 
		//0x0430: 3.DR EAMW EB.G 15.D
		0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 0x30, 0x37, 0x00, 0x00, 0x01, 0x11, 0x01, 
		//0x0440: REAM WEB. I07. ....
		0x40, 0x01, 0x9d, 0x00, 0xc6, 0x00, 0x7c, 0xc4, 0xf0, 0x00, 0x22, 0x01, 0x02, 0x00, 0x2c, 0x00, 
		//0x0450: @... ..|. ..". ..,.
		0x94, 0xc4, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x11, 0x01, 
		//0x0460: .... @... .... ....
		0x40, 0x01, 0x9d, 0x00, 0xc6, 0x00, 0x7c, 0xc4, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 
		//0x0470: @... ..|. ..@. ....
		0xa0, 0xca, 0xff, 0xff, 0xee, 0x00, 0x02, 0x01, 0x04, 0x00, 0x2c, 0x00, 0xc8, 0xc4, 0x68, 0x00, 
		//0x0480: .... .... ..,. ..h.
		0x7c, 0x00, 0x04, 0x00, 0x2c, 0x00, 0xcc, 0xc4, 0x18, 0x01, 0x34, 0x01, 0x04, 0x00, 0x2c, 0x00, 
		//0x0490: |... ,... ..4. ..,.
		0xb0, 0xc4, 0x68, 0x00, 0xd8, 0x00, 0x8a, 0x00, 0xc0, 0x00, 0xd0, 0xc4, 0x11, 0x01, 0x40, 0x01, 
		//0x04a0: ..h. .... .... ..@.
		0x9d, 0x00, 0xc6, 0x00, 0x7c, 0xc4, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 
		//0x04b0: .... |... @... ....
		0xff, 0xff, 0x45, 0x58, 0x49, 0x54, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x48, 0x45, 0x4c, 0x50, 
		//0x04c0: ..EX IT        HELP
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4c, 0x49, 0x53, 0x54, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x04d0:        LI ST       
		0x52, 0x45, 0x41, 0x44, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4c, 0x4f, 0x47, 0x4f, 0x4e, 0x20, 
		//0x04e0: READ        LO GON 
		0x20, 0x20, 0x20, 0x20, 0x4b, 0x45, 0x59, 0x53, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 
		//0x04f0:      KEYS        ..
		0x50, 0x55, 0x42, 0x4c, 0x49, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x50, 0x55, 0x42, 0x4c, 
		//0x0500: PUBL IC        PUBL
		0x49, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x42, 0x4c, 0x41, 0x43, 0x4b, 0x44, 
		//0x0510: IC      . ..BL ACKD
		0x52, 0x41, 0x47, 0x4f, 0x4e, 0x20, 0x52, 0x59, 0x41, 0x4e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x0520: RAGO N RY AN       
		0x20, 0x00, 0x00, 0x00, 0x48, 0x45, 0x4e, 0x44, 0x52, 0x49, 0x58, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x0530:  ... HEND RIX      
		0x4c, 0x4f, 0x55, 0x49, 0x53, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x53, 0x45, 
		//0x0540: LOUI S       . ..SE
		0x50, 0x54, 0x49, 0x4d, 0x55, 0x53, 0x20, 0x20, 0x20, 0x20, 0x42, 0x45, 0x43, 0x4b, 0x45, 0x54, 
		//0x0550: PTIM US     BE CKET
		0x54, 0x20, 0x20, 0x20, 0x20, 0x00, 0xff, 0xff, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x0560: T     ...          
		0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x22, 0x52, 0x4f, 0x4f, 0x54, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x0570:       ."R OOT      
		0x20, 0x20, 0x20, 0x00, 0x22, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x0580:    . "             
		0x20, 0x00, 0x30, 0x30, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 0xba, 0x00, 0xca, 0x00, 0x9d, 0x00, 
		//0x0590:  .00 00.0 0... ....
		0xad, 0x00, 0x1c, 0xc8, 0xf3, 0x00, 0x03, 0x01, 0x83, 0x00, 0x93, 0x00, 0x18, 0xc8, 0x0c, 0x01, 
		//0x05a0: .... .... .... ....
		0x1c, 0x01, 0xa8, 0x00, 0xb8, 0x00, 0x50, 0xc7, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 
		//0x05b0: .... ..P. ..@. ....
		0xa0, 0xca, 0xff, 0xff, 0x77, 0x00, 0xae, 0x00, 0x52, 0x00, 0x80, 0x00, 0x34, 0xc8, 0x46, 0x00, 
		//0x05c0: .... w... R... 4.F.
		0x89, 0x00, 0x3e, 0x00, 0x6f, 0x00, 0x80, 0xc8, 0xbc, 0x00, 0xfa, 0x00, 0x44, 0x00, 0x98, 0x00, 
		//0x05d0: ..>. o... .... D...
		0x4c, 0xc8, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x77, 0x00, 
		//0x05e0: L... @... .... ..w.
		0xae, 0x00, 0x52, 0x00, 0x80, 0x00, 0x44, 0xc8, 0x46, 0x00, 0x8b, 0x00, 0x3e, 0x00, 0x6f, 0x00, 
		//0x05f0: ..R. ..D. F... >.o.
		0x50, 0xc8, 0xec, 0x00, 0xfc, 0x00, 0x70, 0x00, 0x80, 0x00, 0x48, 0xc8, 0x00, 0x00, 0x40, 0x01, 
		//0x0600: P... ..p. ..H. ..@.
		0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x81, 0x00, 0xb8, 
		//0x0610: .... .... .... $...
		0x00, 0x52, 0x00, 0x80, 0x00, 0xc0, 0xc8, 0x50, 0x00, 0x93, 0x00, 0x3e, 0x00, 0x6f, 0x00, 0x80, 
		//0x0620: .R.. ...P ...> .o..
		0xc8, 0xb7, 0x00, 0xfa, 0x00, 0x3e, 0x00, 0x6f, 0x00, 0xc4, 0xc8, 0x00, 0x00, 0x40, 0x01, 0x00, 
		//0x0630: .... .>.o .... .@..
		0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x53, 0x50, 0x45, 0x45, 0x43, 0x48, 0x52, 0x32, 0x34, 
		//0x0640: .... ...S PEEC HR24
		0x43, 0x30, 0x30, 0x30, 0x35, 0x2e, 0x52, 0x41, 0x57, 0x00, 0x4f, 0x42, 0x4a, 0x45, 0x43, 0x54, 
		//0x0650: C000 5.RA W.OB JECT
		0x20, 0x4e, 0x41, 0x4d, 0x45, 0x20, 0x4f, 0x4e, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x0660:  NAM E ON E        
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		//0x0670:                    
		0x20, 0x20, 0x00, 0x10, 0x12, 0x12, 0x11, 0x10, 0x10, 0x10, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		//0x0680:   .. .... .... ....
		0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x44, 0x3a, 0x00, 0x00, 0x00, 
		//0x0690: .... .... ...D :...
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x06a0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 
		//0x06b0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x02, 0x04, 
		//0x06c0: .... .... .... ....
		0x01, 0x0a, 0x09, 0x08, 0x06, 0x0b, 0x04, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x06d0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x06e0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x06f0: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0700: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0710: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0720: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0730: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		//0x0740: .... .... .... ....
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		//0x0750: .... .... .... ....
		0xff, 0xff, 0xff, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		//0x0760: .... .... .... ....
		0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		//0x0770: .... .... .... ....
		0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		//0x0780: .... .... .... ....
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		//0x0790: .... .... .... ....
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		//0x07a0: .... .... .... ....
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		//0x07b0: .... .... .... ....
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 
		//0x07c0: .... .... .... ....
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, };
	ds.assign(src, src + sizeof(src));
	dreamweb(); 
}

} /*namespace*/
