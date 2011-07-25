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

void DreamGenContext::alleybarksound() {
	STACK_CHECK;
	ax = es.word(bx+3);
	_dec(ax);
	_cmp(ax, 0);
	if (!flags.z())
		goto nobark;
	push(bx);
	push(es);
	al = 14;
	playchannel1();
	es = pop();
	bx = pop();
	ax = 1000;
nobark:
	es.word(bx+3) = ax;
}

void DreamGenContext::intromusic() {
	STACK_CHECK;
}

void DreamGenContext::foghornsound() {
	STACK_CHECK;
	randomnumber();
	_cmp(al, 198);
	if (!flags.z())
		return /* (nofog) */;
	al = 13;
	playchannel1();
}

void DreamGenContext::receptionist() {
	STACK_CHECK;
	checkspeed();
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
	randomnumber();
	_cmp(al, 30);
	if (flags.c())
		goto notdes2;
	es.word(bx+3) = 55;
	goto gotrecep;
notdes1:
	_cmp(es.word(bx+3), 60);
	if (!flags.z())
		goto notdes2;
	randomnumber();
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
	showgamereel();
	addtopeoplelist();
	al = es.byte(bx+7);
	_and(al, 128);
	if (flags.z())
		return /* (nottalkedrecep) */;
	data.byte(kTalkedtorecep) = 1;
}

void DreamGenContext::smokebloke() {
	STACK_CHECK;
	_cmp(data.byte(kRockstardead), 0);
	if (!flags.z())
		goto notspokento;
	al = es.byte(bx+7);
	_and(al, 128);
	if (flags.z())
		goto notspokento;
	push(es);
	push(bx);
	al = 5;
	setlocation();
	bx = pop();
	es = pop();
notspokento:
	checkspeed();
	if (!flags.z())
		goto gotsmokeb;
	_cmp(es.word(bx+3), 100);
	if (!flags.z())
		goto notsmokeb1;
	randomnumber();
	_cmp(al, 30);
	if (flags.c())
		goto notsmokeb2;
	es.word(bx+3) = 96;
	goto gotsmokeb;
notsmokeb1:
	_cmp(es.word(bx+3), 117);
	if (!flags.z())
		goto notsmokeb2;
	es.word(bx+3) = 96;
	goto gotsmokeb;
notsmokeb2:
	_inc(es.word(bx+3));
gotsmokeb:
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::attendant() {
	STACK_CHECK;
	showgamereel();
	addtopeoplelist();
	al = es.byte(bx+7);
	_and(al, 128);
	if (flags.z())
		return /* (nottalked) */;
	data.byte(kTalkedtoattendant) = 1;
}

void DreamGenContext::manasleep() {
	STACK_CHECK;
	al = es.byte(bx+7);
	_and(al, 127);
	es.byte(bx+7) = al;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::eden() {
	STACK_CHECK;
	_cmp(data.byte(kGeneraldead), 0);
	if (!flags.z())
		return /* (notinbed) */;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::edeninbath() {
	STACK_CHECK;
	_cmp(data.byte(kGeneraldead), 0);
	if (flags.z())
		return /* (notinbath) */;
	_cmp(data.byte(kSartaindead), 0);
	if (!flags.z())
		return /* (notinbath) */;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::malefan() {
	STACK_CHECK;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::femalefan() {
	STACK_CHECK;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::louis() {
	STACK_CHECK;
	_cmp(data.byte(kRockstardead), 0);
	if (!flags.z())
		return /* (notlouis1) */;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::louischair() {
	STACK_CHECK;
	_cmp(data.byte(kRockstardead), 0);
	if (flags.z())
		return /* (notlouis2) */;
	checkspeed();
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
	randomnumber();
	_cmp(al, 245);
	if (!flags.c())
		goto notlouisanim;
restartlouis:
	ax = 182;
	es.word(bx+3) = ax;
notlouisanim:
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::manasleep2() {
	STACK_CHECK;
	al = es.byte(bx+7);
	_and(al, 127);
	es.byte(bx+7) = al;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::mansatstill() {
	STACK_CHECK;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::tattooman() {
	STACK_CHECK;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::drinker() {
	STACK_CHECK;
	checkspeed();
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
	randomnumber();
	_cmp(al, 3);
	if (flags.c())
		goto gotdrinker;
	es.word(bx+3) = 105;
gotdrinker:
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::bartender() {
	STACK_CHECK;
	checkspeed();
	if (!flags.z())
		goto gotsmoket;
	_cmp(es.word(bx+3), 86);
	if (!flags.z())
		goto notsmoket1;
	randomnumber();
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
	showgamereel();
	_cmp(data.byte(kGunpassflag), 1);
	if (!flags.z())
		goto notgotgun;
	es.byte(bx+7) = 9;
notgotgun:
	addtopeoplelist();
}

void DreamGenContext::othersmoker() {
	STACK_CHECK;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::barwoman() {
	STACK_CHECK;
	showgamereel();
	addtopeoplelist();
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
	checkspeed();
	if (!flags.z())
		goto talking;
	_cmp(es.word(bx+3), 259);
	if (flags.z())
		goto talking;
	_inc(es.word(bx+3));
talking:
	showgamereel();
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
	checkspeed();
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
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::rockstar() {
	STACK_CHECK;
	ax = es.word(bx+3);
	_cmp(ax, 303);
	if (flags.z())
		goto rockcombatend;
	_cmp(ax, 118);
	if (flags.z())
		goto rockcombatend;
	checkspeed();
	if (!flags.z())
		goto rockspeed;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 118);
	if (!flags.z())
		goto notbeforedead;
	data.byte(kMandead) = 2;
	goto gotrockframe;
notbeforedead:
	_cmp(ax, 79);
	if (!flags.z())
		goto gotrockframe;
	_dec(ax);
	_cmp(data.byte(kLastweapon), 1);
	if (!flags.z())
		goto notgunonrock;
	data.byte(kLastweapon) = -1;
	ax = 123;
	goto gotrockframe;
notgunonrock:
	_inc(data.byte(kCombatcount));
	_cmp(data.byte(kCombatcount), 40);
	if (!flags.z())
		goto gotrockframe;
	data.byte(kCombatcount) = 0;
	ax = 79;
gotrockframe:
	es.word(bx+3) = ax;
rockspeed:
	showgamereel();
	_cmp(es.word(bx+3), 78);
	if (!flags.z())
		goto notalkrock;
	addtopeoplelist();
	data.byte(kPointermode) = 2;
	data.word(kWatchingtime) = 0;
	return;
notalkrock:
	data.word(kWatchingtime) = 2;
	data.byte(kPointermode) = 0;
	al = data.byte(kMapy);
	es.byte(bx+2) = al;
	return;
rockcombatend:
	data.byte(kNewlocation) = 45;
	showgamereel();
}

void DreamGenContext::helicopter() {
	STACK_CHECK;
	ax = es.word(bx+3);
	_cmp(ax, 203);
	if (flags.z())
		goto heliwon;
	checkspeed();
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
	showgamereel();
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
	checkspeed();
	if (!flags.z())
		goto notmugger;
	_inc(es.word(bx+3));
notmugger:
	showgamereel();
	al = data.byte(kMapx);
	es.byte(bx+1) = al;
	return;
endmugger1:
	push(es);
	push(bx);
	createpanel2();
	showicon();
	al = 41;
	findpuztext();
	di = 33+20;
	bx = 104;
	dl = 241;
	ah = 0;
	printdirect();
	worktoscreen();
	cx = 300;
	hangon();
	bx = pop();
	es = pop();
	push(es);
	push(bx);
	es.word(bx+3) = 140;
	data.byte(kManspath) = 2;
	data.byte(kFinaldest) = 2;
	findxyfrompath();
	data.byte(kResetmanxy) = 1;
	al = 'W';
	ah = 'E';
	cl = 'T';
	ch = 'A';
	findexobject();
	data.byte(kCommand) = al;
	data.byte(kObjecttype) = 4;
	removeobfrominv();
	al = 'W';
	ah = 'E';
	cl = 'T';
	ch = 'B';
	findexobject();
	data.byte(kCommand) = al;
	data.byte(kObjecttype) = 4;
	removeobfrominv();
	makemainscreen();
	al = 48;
	bl = 68-32;
	bh = 54+64;
	cx = 70;
	dx = 10;
	setuptimeduse();
	data.byte(kBeenmugged) = 1;
	bx = pop();
	es = pop();
}

void DreamGenContext::aide() {
	STACK_CHECK;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::businessman() {
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
	setuptimeduse();
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
	checkspeed();
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
	turnpathon();
	al = 1;
	turnpathon();
	al = 2;
	turnpathon();
	al = 3;
	turnpathoff();
	data.byte(kManspath) = 5;
	data.byte(kFinaldest) = 5;
	findxyfrompath();
	data.byte(kResetmanxy) = 1;
	es = pop();
	bx = pop();
	ax = 92;
	goto gotbusframe;
gotbusframe:
	es.word(bx+3) = ax;
busspeed:
	showgamereel();
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

void DreamGenContext::poolguard() {
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
	turnpathon();
notfirstpool:
	checkspeed();
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
	showgamereel();
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
	turnpathon();
	al = 1;
	turnpathoff();
	return;
combatover2:
	showgamereel();
	data.word(kWatchingtime) = 2;
	data.byte(kPointermode) = 0;
	_inc(data.byte(kCombatcount));
	_cmp(data.byte(kCombatcount), 100);
	if (flags.c())
		return /* (doneover2) */;
	data.word(kWatchingtime) = 0;
	data.byte(kMandead) = 2;
}

void DreamGenContext::security() {
	STACK_CHECK;
	_cmp(es.word(bx+3), 32);
	if (flags.z())
		goto securwait;
	_cmp(es.word(bx+3), 69);
	if (!flags.z())
		goto notaftersec;
	return;
notaftersec:
	data.word(kWatchingtime) = 10;
	checkspeed();
	if (!flags.z())
		goto gotsecurframe;
	_inc(es.word(bx+3));
	goto gotsecurframe;
securwait:
	_cmp(data.byte(kLastweapon), 1);
	if (!flags.z())
		goto gotsecurframe;
	data.word(kWatchingtime) = 10;
	_cmp(data.byte(kManspath), 9);
	if (!flags.z())
		goto gotsecurframe;
	_cmp(data.byte(kFacing), 0);
	if (!flags.z())
		goto gotsecurframe;
	data.byte(kLastweapon) = -1;
	_inc(es.word(bx+3));
gotsecurframe:
	showgamereel();
	addtopeoplelist();
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
	checkspeed();
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
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::bossman() {
	STACK_CHECK;
	checkspeed();
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
	randomnumber();
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
	showgamereel();
	addtopeoplelist();
	al = es.byte(bx+7);
	_and(al, 128);
	if (flags.z())
		return /* (nottalkedboss) */;
	data.byte(kTalkedtoboss) = 1;
}

void DreamGenContext::gamer() {
	STACK_CHECK;
	checkspeed();
	if (!flags.z())
		goto gamerfin;
gameragain:
	randomnum1();
	_and(al, 7);
	_cmp(al, 5);
	if (!flags.c())
		goto gameragain;
	_add(al, 20);
	_cmp(al, es.byte(bx+3));
	if (flags.z())
		goto gameragain;
	ah = 0;
	es.word(bx+3) = ax;
gamerfin:
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::sparkydrip() {
	STACK_CHECK;
	checkspeed();
	if (!flags.z())
		return /* (cantdrip) */;
	al = 14;
	ah = 0;
	playchannel0();
}

void DreamGenContext::carparkdrip() {
	STACK_CHECK;
	checkspeed();
	if (!flags.z())
		return /* (cantdrip2) */;
	al = 14;
	playchannel1();
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
	addtopeoplelist();
	showgamereel();
}

void DreamGenContext::candles1() {
	STACK_CHECK;
	checkspeed();
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
	showgamereel();
}

void DreamGenContext::smallcandle() {
	STACK_CHECK;
	checkspeed();
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
	showgamereel();
}

void DreamGenContext::intromagic1() {
	STACK_CHECK;
	checkspeed();
	if (!flags.z())
		goto introm1fin;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 145);
	if (!flags.z())
		goto gotintrom1;
	ax = 121;
gotintrom1:
	es.word(bx+3) = ax;
	_cmp(ax, 121);
	if (!flags.z())
		goto introm1fin;
	_inc(data.byte(kIntrocount));
	push(es);
	push(bx);
	intro1text();
	bx = pop();
	es = pop();
	_cmp(data.byte(kIntrocount), 8);
	if (!flags.z())
		goto introm1fin;
	_add(data.byte(kMapy), 10);
	data.byte(kNowinnewroom) = 1;
introm1fin:
	showgamereel();
}

void DreamGenContext::candles() {
	STACK_CHECK;
	checkspeed();
	if (!flags.z())
		goto candlesfin;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 167);
	if (!flags.z())
		goto gotcandles;
	ax = 162;
gotcandles:
	es.word(bx+3) = ax;
candlesfin:
	showgamereel();
}

void DreamGenContext::candles2() {
	STACK_CHECK;
	checkspeed();
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
	showgamereel();
}

void DreamGenContext::gates() {
	STACK_CHECK;
	checkspeed();
	if (!flags.z())
		goto gatesfin;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 116);
	if (!flags.z())
		goto notbang;
	push(ax);
	push(bx);
	push(es);
	al = 17;
	playchannel1();
	es = pop();
	bx = pop();
	ax = pop();
notbang:
	_cmp(ax, 110);
	if (flags.c())
		goto slowgates;
	es.byte(bx+5) = 2;
slowgates:
	_cmp(ax, 120);
	if (!flags.z())
		goto gotgates;
	data.byte(kGetback) = 1;
	ax = 119;
gotgates:
	es.word(bx+3) = ax;
	push(es);
	push(bx);
	intro3text();
	bx = pop();
	es = pop();
gatesfin:
	showgamereel();
}

void DreamGenContext::intromagic2() {
	STACK_CHECK;
	checkspeed();
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
	showgamereel();
}

void DreamGenContext::intromagic3() {
	STACK_CHECK;
	checkspeed();
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
	showgamereel();
	al = data.byte(kMapx);
	es.byte(bx+1) = al;
}

void DreamGenContext::intromonks1() {
	STACK_CHECK;
	checkspeed();
	if (!flags.z())
		goto intromonk1fin;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 80);
	if (!flags.z())
		goto notendmonk1;
	_add(data.byte(kMapy), 10);
	data.byte(kNowinnewroom) = 1;
	showgamereel();
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
	intro2text();
	bx = pop();
	es = pop();
	es.byte(bx+6) = -20;
intromonk1fin:
	showgamereel();
	al = data.byte(kMapy);
	es.byte(bx+2) = al;
}

void DreamGenContext::intromonks2() {
	STACK_CHECK;
	checkspeed();
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
	showgamereel();
}

void DreamGenContext::handclap() {
	STACK_CHECK;
}

void DreamGenContext::monks2text() {
	STACK_CHECK;
	_cmp(data.byte(kIntrocount), 1);
	if (!flags.z())
		goto notmonk2text1;
	al = 8;
	bl = 36;
	bh = 160;
	cx = 100;
	goto gotmonks2text;
notmonk2text1:
	_cmp(data.byte(kIntrocount), 4);
	if (!flags.z())
		goto notmonk2text2;
	al = 9;
	bl = 36;
	bh = 160;
	cx = 100;
	goto gotmonks2text;
notmonk2text2:
	_cmp(data.byte(kIntrocount), 7);
	if (!flags.z())
		goto notmonk2text3;
	al = 10;
	bl = 36;
	bh = 160;
	cx = 100;
	goto gotmonks2text;
notmonk2text3:
	_cmp(data.byte(kIntrocount), 10);
	if (!flags.z())
		goto notmonk2text4;
	data.byte(kIntrocount) = 12;
	al = 11;
	bl = 0;
	bh = 105;
	cx = 100;
	goto gotmonks2text;
notmonk2text4:
	_cmp(data.byte(kIntrocount), 13);
	if (!flags.z())
		goto notmonk2text5;
	data.byte(kIntrocount) = 17;
	return;
	al = 12;
	bl = 0;
	bh = 120;
	cx = 100;
	goto gotmonks2text;
notmonk2text5:
	_cmp(data.byte(kIntrocount), 16);
	if (!flags.z())
		goto notmonk2text6;
	al = 13;
	bl = 0;
	bh = 135;
	cx = 100;
	goto gotmonks2text;
notmonk2text6:
	_cmp(data.byte(kIntrocount), 19);
	if (!flags.z())
		goto notmonk2text7;
	al = 14;
	bl = 36;
	bh = 160;
	cx = 100;
	dx = 1;
	ah = 82;
	{ setuptimedtemp(); return; };
notmonk2text7:
	_cmp(data.byte(kIntrocount), 22);
	if (!flags.z())
		goto notmonk2text8;
	al = 15;
	bl = 36;
	bh = 160;
	cx = 100;
	goto gotmonks2text;
notmonk2text8:
	_cmp(data.byte(kIntrocount), 25);
	if (!flags.z())
		goto notmonk2text9;
	al = 16;
	bl = 36;
	bh = 160;
	cx = 100;
	goto gotmonks2text;
notmonk2text9:
	_cmp(data.byte(kIntrocount), 27);
	if (!flags.z())
		goto notmonk2text10;
	al = 17;
	bl = 36;
	bh = 160;
	cx = 100;
	goto gotmonks2text;
notmonk2text10:
	_cmp(data.byte(kIntrocount), 31);
	if (!flags.z())
		return /* (notmonk2text11) */;
	al = 18;
	bl = 36;
	bh = 160;
	cx = 100;
	goto gotmonks2text;
	return;
gotmonks2text:
	dx = 1;
	cx = 120;
	ah = 82;
	setuptimedtemp();
}

void DreamGenContext::intro1text() {
	STACK_CHECK;
	_cmp(data.byte(kIntrocount), 2);
	if (!flags.z())
		goto notintro1text1;
	al = 40;
	bl = 34;
	bh = 130;
	cx = 90;
	goto gotintro1text;
notintro1text1:
	_cmp(data.byte(kIntrocount), 4);
	if (!flags.z())
		goto notintro1text2;
	al = 41;
	bl = 34;
	bh = 130;
	cx = 90;
	goto gotintro1text;
notintro1text2:
	_cmp(data.byte(kIntrocount), 6);
	if (!flags.z())
		return /* (notintro1text3) */;
	al = 42;
	bl = 34;
	bh = 130;
	cx = 90;
	goto gotintro1text;
	return;
gotintro1text:
	dx = 1;
	ah = 82;
	_cmp(data.byte(kCh1playing), 255);
	if (flags.z())
		goto oktalk2;
	_dec(data.byte(kIntrocount));
	return;
oktalk2:
	setuptimedtemp();
}

void DreamGenContext::intro2text() {
	STACK_CHECK;
	_cmp(ax, 5);
	if (!flags.z())
		goto notintro2text1;
	al = 43;
	bl = 34;
	bh = 40;
	cx = 90;
	goto gotintro2text;
notintro2text1:
	_cmp(ax, 15);
	if (!flags.z())
		return /* (notintro2text2) */;
	al = 44;
	bl = 34;
	bh = 40;
	cx = 90;
	goto gotintro2text;
	return;
gotintro2text:
	dx = 1;
	ah = 82;
	setuptimedtemp();
}

void DreamGenContext::intro3text() {
	STACK_CHECK;
	_cmp(ax, 107);
	if (!flags.z())
		goto notintro3text1;
	al = 45;
	bl = 36;
	bh = 56;
	cx = 100;
	goto gotintro3text;
notintro3text1:
	_cmp(ax, 108);
	if (!flags.z())
		return /* (notintro3text2) */;
	al = 46;
	bl = 36;
	bh = 56;
	cx = 100;
	goto gotintro3text;
	return;
gotintro3text:
	dx = 1;
	ah = 82;
	setuptimedtemp();
}

void DreamGenContext::monkandryan() {
	STACK_CHECK;
	checkspeed();
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
	textformonk();
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
	showgamereel();
}

void DreamGenContext::endgameseq() {
	STACK_CHECK;
	checkspeed();
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
	textforend();
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
	fadescreendownhalf();
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
	fadescreendowns();
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
	showgamereel();
	al = data.byte(kMapy);
	es.byte(bx+2) = al;
	ax = es.word(bx+3);
	_cmp(ax, 145);
	if (!flags.z())
		return /* (notendcreds) */;
	es.word(bx+3) = 146;
	rollendcredits();
}

void DreamGenContext::rollendcredits() {
	STACK_CHECK;
	al = 16;
	ah = 255;
	playchannel0();
	data.byte(kVolume) = 7;
	data.byte(kVolumeto) = 0;
	data.byte(kVolumedirection) = -1;
	cl = 160;
	ch = 160;
	di = 75;
	bx = 20;
	ds = data.word(kMapstore);
	si = 0;
	multiget();
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
	vsync();
	cl = 160;
	ch = 160;
	di = 75;
	bx = 20;
	ds = data.word(kMapstore);
	si = 0;
	multiput();
	vsync();
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
	printdirect();
	_add(bx, data.word(kLinespacing));
	cx = pop();
	if (--cx)
		goto onelot;
	vsync();
	cl = 160;
	ch = 160;
	di = 75;
	bx = 20;
	multidump();
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
	hangon();
	paneltomap();
	fadescreenuphalf();
}

void DreamGenContext::priest() {
	STACK_CHECK;
	_cmp(es.word(bx+3), 8);
	if (flags.z())
		return /* (priestspoken) */;
	data.byte(kPointermode) = 0;
	data.word(kWatchingtime) = 2;
	checkspeed();
	if (!flags.z())
		return /* (priestwait) */;
	_inc(es.word(bx+3));
	push(es);
	push(bx);
	priesttext();
	bx = pop();
	es = pop();
}

void DreamGenContext::madmanstelly() {
	STACK_CHECK;
	ax = es.word(bx+3);
	_inc(ax);
	_cmp(ax, 307);
	if (!flags.z())
		goto notendtelly;
	ax = 300;
notendtelly:
	es.word(bx+3) = ax;
	showgamereel();
}

void DreamGenContext::madman() {
	STACK_CHECK;
	data.word(kWatchingtime) = 2;
	checkspeed();
	if (!flags.z())
		goto nomadspeed;
	ax = es.word(bx+3);
	_cmp(ax, 364);
	if (!flags.c())
		goto ryansded;
	_cmp(ax, 10);
	if (!flags.z())
		goto notfirstmad;
	push(es);
	push(bx);
	push(ax);
	dx = 2247;
	loadtemptext();
	ax = pop();
	bx = pop();
	es = pop();
	data.byte(kCombatcount) = -1;
	data.byte(kSpeechcount) = 0;
notfirstmad:
	_inc(ax);
	_cmp(ax, 294);
	if (flags.z())
		goto madmanspoken;
	_cmp(ax, 66);
	if (!flags.z())
		goto nomadspeak;
	_inc(data.byte(kCombatcount));
	push(es);
	push(bx);
	madmantext();
	bx = pop();
	es = pop();
	ax = 53;
	_cmp(data.byte(kCombatcount), 64);
	if (flags.c())
		goto nomadspeak;
	_cmp(data.byte(kCombatcount), 70);
	if (flags.z())
		goto killryan;
	_cmp(data.byte(kLastweapon), 8);
	if (!flags.z())
		goto nomadspeak;
	data.byte(kCombatcount) = 72;
	data.byte(kLastweapon) = -1;
	data.byte(kMadmanflag) = 1;
	ax = 67;
	goto nomadspeak;
killryan:
	ax = 310;
nomadspeak:
	es.word(bx+3) = ax;
nomadspeed:
	showgamereel();
	al = data.byte(kMapx);
	es.byte(bx+1) = al;
	madmode();
	return;
madmanspoken:
	_cmp(data.byte(kWongame), 1);
	if (flags.z())
		return /* (alreadywon) */;
	data.byte(kWongame) = 1;
	push(es);
	push(bx);
	getridoftemptext();
	bx = pop();
	es = pop();
	return;
ryansded:
	data.byte(kMandead) = 2;
	showgamereel();
}

void DreamGenContext::madmantext() {
	STACK_CHECK;
	_cmp(data.byte(kSpeechcount), 63);
	if (!flags.c())
		return /* (nomadtext) */;
	_cmp(data.byte(kCh1playing), 255);
	if (!flags.z())
		return /* (nomadtext) */;
	al = data.byte(kSpeechcount);
	_inc(data.byte(kSpeechcount));
	_add(al, 47);
	bl = 72;
	bh = 80;
	cx = 90;
	dx = 1;
	ah = 82;
	setuptimedtemp();
}

void DreamGenContext::madmode() {
	STACK_CHECK;
	data.word(kWatchingtime) = 2;
	data.byte(kPointermode) = 0;
	_cmp(data.byte(kCombatcount), 65);
	if (flags.c())
		return /* (iswatchmad) */;
	_cmp(data.byte(kCombatcount), 70);
	if (!flags.c())
		return /* (iswatchmad) */;
	data.byte(kPointermode) = 2;
}

void DreamGenContext::priesttext() {
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
	setuptimeduse();
}

void DreamGenContext::textforend() {
	STACK_CHECK;
	_cmp(data.byte(kIntrocount), 20);
	if (!flags.z())
		goto notendtext1;
	al = 0;
	bl = 34;
	bh = 20;
	cx = 60;
	goto gotendtext;
notendtext1:
	_cmp(data.byte(kIntrocount), 50);
	if (!flags.z())
		goto notendtext2;
	al = 1;
	bl = 34;
	bh = 20;
	cx = 60;
	goto gotendtext;
notendtext2:
	_cmp(data.byte(kIntrocount), 85);
	if (!flags.z())
		return /* (notendtext3) */;
	al = 2;
	bl = 34;
	bh = 20;
	cx = 60;
	goto gotendtext;
	return;
gotendtext:
	dx = 1;
	ah = 83;
	setuptimedtemp();
}

void DreamGenContext::textformonk() {
	STACK_CHECK;
	_cmp(data.byte(kIntrocount), 1);
	if (!flags.z())
		goto notmonktext1;
	al = 19;
	bl = 68;
	bh = 154;
	cx = 120;
	goto gotmonktext;
notmonktext1:
	_cmp(data.byte(kIntrocount), 5);
	if (!flags.z())
		goto notmonktext2;
	al = 20;
	bl = 68;
	bh = 38;
	cx = 120;
	goto gotmonktext;
notmonktext2:
	_cmp(data.byte(kIntrocount), 9);
	if (!flags.z())
		goto notmonktext3;
	al = 21;
	bl = 48;
	bh = 154;
	cx = 120;
	goto gotmonktext;
notmonktext3:
	_cmp(data.byte(kIntrocount), 13);
	if (!flags.z())
		goto notmonktext4;
	al = 22;
	bl = 68;
	bh = 38;
	cx = 120;
	goto gotmonktext;
notmonktext4:
	_cmp(data.byte(kIntrocount), 15);
	if (!flags.z())
		goto notmonktext5;
	al = 23;
	bl = 68;
	bh = 154;
	cx = 120;
	goto gotmonktext;
notmonktext5:
	_cmp(data.byte(kIntrocount), 21);
	if (!flags.z())
		goto notmonktext6;
	al = 24;
	bl = 68;
	bh = 38;
	cx = 120;
	goto gotmonktext;
notmonktext6:
	_cmp(data.byte(kIntrocount), 25);
	if (!flags.z())
		goto notmonktext7;
	al = 25;
	bl = 68;
	bh = 154;
	cx = 120;
	goto gotmonktext;
notmonktext7:
	_cmp(data.byte(kIntrocount), 29);
	if (!flags.z())
		goto notmonktext8;
	al = 26;
	bl = 68;
	bh = 38;
	cx = 120;
	goto gotmonktext;
notmonktext8:
	_cmp(data.byte(kIntrocount), 33);
	if (!flags.z())
		goto notmonktext9;
	al = 27;
	bl = 68;
	bh = 154;
	cx = 120;
	goto gotmonktext;
notmonktext9:
	_cmp(data.byte(kIntrocount), 37);
	if (!flags.z())
		goto notmonktext10;
	al = 28;
	bl = 68;
	bh = 154;
	cx = 120;
	goto gotmonktext;
notmonktext10:
	_cmp(data.byte(kIntrocount), 41);
	if (!flags.z())
		goto notmonktext11;
	al = 29;
	bl = 68;
	bh = 38;
	cx = 120;
	goto gotmonktext;
notmonktext11:
	_cmp(data.byte(kIntrocount), 45);
	if (!flags.z())
		goto notmonktext12;
	al = 30;
	bl = 68;
	bh = 154;
	cx = 120;
	goto gotmonktext;
notmonktext12:
	_cmp(data.byte(kIntrocount), 52);
	if (!flags.z())
		goto notmonktext13;
	al = 31;
	bl = 68;
	bh = 154;
	cx = 220;
	goto gotmonktext;
notmonktext13:
	_cmp(data.byte(kIntrocount), 53);
	if (!flags.z())
		return /* (notendtitles) */;
	fadescreendowns();
	data.byte(kVolumeto) = 7;
	data.byte(kVolumedirection) = 1;
	return;
gotmonktext:
	dx = 1;
	ah = 82;
	_cmp(data.byte(kCh1playing), 255);
	if (flags.z())
		goto oktalk;
	_dec(data.byte(kIntrocount));
	return;
oktalk:
	setuptimedtemp();
}

void DreamGenContext::drunk() {
	STACK_CHECK;
	_cmp(data.byte(kGeneraldead), 0);
	if (!flags.z())
		return /* (trampgone) */;
	al = es.byte(bx+7);
	_and(al, 127);
	es.byte(bx+7) = al;
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::advisor() {
	STACK_CHECK;
	checkspeed();
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
	randomnumber();
	cl = al;
	ax = pop();
	_cmp(cl, 3);
	if (flags.c())
		goto gotadvframe;
	ax = 106;
gotadvframe:
	es.word(bx+3) = ax;
noadvisor:
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::copper() {
	STACK_CHECK;
	checkspeed();
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
	randomnumber();
	cl = al;
	ax = pop();
	_cmp(cl, 7);
	if (flags.c())
		goto gotcopframe;
	_dec(ax);
gotcopframe:
	es.word(bx+3) = ax;
nocopper:
	showgamereel();
	addtopeoplelist();
}

void DreamGenContext::sparky() {
	STACK_CHECK;
	_cmp(data.word(kCard1money), 0);
	if (flags.z())
		goto animsparky;
	es.byte(bx+7) = 3;
	goto animsparky;
animsparky:
	checkspeed();
	if (!flags.z())
		goto finishsparky;
	_cmp(es.word(bx+3), 34);
	if (!flags.z())
		goto notsparky1;
	randomnumber();
	_cmp(al, 30);
	if (flags.c())
		goto dosparky;
	es.word(bx+3) = 27;
	goto finishsparky;
notsparky1:
	_cmp(es.word(bx+3), 48);
	if (!flags.z())
		goto dosparky;
	es.word(bx+3) = 27;
	goto finishsparky;
dosparky:
	_inc(es.word(bx+3));
finishsparky:
	showgamereel();
	addtopeoplelist();
	al = es.byte(bx+7);
	_and(al, 128);
	if (flags.z())
		return /* (nottalkedsparky) */;
	data.byte(kTalkedtosparky) = 1;
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
	randomnumber();
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
	showgamereel();
}

void DreamGenContext::addtopeoplelist() {
	STACK_CHECK;
	push(es);
	push(bx);
	push(bx);
	cl = es.byte(bx+7);
	ax = es.word(bx+3);
	bx = data.word(kListpos);
	es = data.word(kBuffers);
	es.word(bx) = ax;
	ax = pop();
	es.word(bx+2) = ax;
	es.byte(bx+4) = cl;
	bx = pop();
	es = pop();
	_add(data.word(kListpos), 5);
}

void DreamGenContext::showgamereel() {
	STACK_CHECK;
	ax = es.word(bx+3);
	_cmp(ax, 512);
	if (!flags.c())
		return /* (noshow) */;
	data.word(kReelpointer) = ax;
	push(es);
	push(bx);
	plotreel();
	bx = pop();
	es = pop();
	ax = data.word(kReelpointer);
	es.word(bx+3) = ax;
}

void DreamGenContext::checkspeed() {
	STACK_CHECK;
	_cmp(data.byte(kLastweapon), -1);
	if (!flags.z())
		goto forcenext;
	_inc(es.byte(bx+6));
	al = es.byte(bx+6);
	_cmp(al, es.byte(bx+5));
	if (!flags.z())
		return /* (notspeed) */;
	al = 0;
	es.byte(bx+6) = al;
	_cmp(al, al);
	return;
forcenext:
	_cmp(al, al);
}

void DreamGenContext::delsprite() {
	STACK_CHECK;
	di = bx;
	cx = (32);
	al = 255;
	_stosb(cx, true);
}

void DreamGenContext::checkone() {
	STACK_CHECK;
	push(cx);
	al = ch;
	ah = 0;
	cl = 4;
	_shr(ax, cl);
	dl = al;
	cx = pop();
	al = cl;
	ah = 0;
	cl = 4;
	_shr(ax, cl);
	ah = dl;
	push(ax);
	ch = 0;
	cl = al;
	push(cx);
	al = ah;
	ah = 0;
	cx = 11;
	_mul(cx);
	cx = pop();
	_add(ax, cx);
	cx = 3;
	_mul(cx);
	si = ax;
	ds = data.word(kBuffers);
	_add(si, (0+(228*13)+32+60+(32*32)));
	_lodsw();
	cx = ax;
	_lodsb();
	dx = pop();
}

void DreamGenContext::findsource() {
	STACK_CHECK;
	ax = data.word(kCurrentframe);
	_cmp(ax, 160);
	if (!flags.c())
		goto over1000;
	ds = data.word(kReel1);
	data.word(kTakeoff) = 0;
	return;
over1000:
	_cmp(ax, 320);
	if (!flags.c())
		goto over1001;
	ds = data.word(kReel2);
	data.word(kTakeoff) = 160;
	return;
over1001:
	ds = data.word(kReel3);
	data.word(kTakeoff) = 320;
}

void DreamGenContext::mainman() {
	STACK_CHECK;
	_cmp(data.byte(kResetmanxy), 1);
	if (!flags.z())
		goto notinnewroom;
	data.byte(kResetmanxy) = 0;
	al = data.byte(kRyanx);
	ah = data.byte(kRyany);
	es.word(bx+10) = ax;
	es.byte(bx+29) = 0;
	goto executewalk;
notinnewroom:
	_dec(es.byte(bx+22));
	_cmp(es.byte(bx+22), -1);
	if (flags.z())
		goto executewalk;
	return;
executewalk:
	es.byte(bx+22) = 0;
	al = data.byte(kTurntoface);
	_cmp(al, data.byte(kFacing));
	if (flags.z())
		goto facingok;
	aboutturn();
	goto notwalk;
facingok:
	_cmp(data.byte(kTurndirection), 0);
	if (flags.z())
		goto alreadyturned;
	_cmp(data.byte(kLinepointer), 254);
	if (!flags.z())
		goto alreadyturned;
	data.byte(kReasseschanges) = 1;
	al = data.byte(kFacing);
	_cmp(al, data.byte(kLeavedirection));
	if (!flags.z())
		goto alreadyturned;
	checkforexit();
alreadyturned:
	data.byte(kTurndirection) = 0;
	_cmp(data.byte(kLinepointer), 254);
	if (!flags.z())
		goto walkman;
	es.byte(bx+29) = 0;
	goto notwalk;
walkman:
	al = es.byte(bx+29);
	_inc(al);
	_cmp(al, 11);
	if (!flags.z())
		goto notanimend1;
	al = 1;
notanimend1:
	es.byte(bx+29) = al;
	walking();
	_cmp(data.byte(kLinepointer), 254);
	if (flags.z())
		goto afterwalk;
	al = data.byte(kFacing);
	_and(al, 1);
	if (flags.z())
		goto isdouble;
	al = es.byte(bx+29);
	_cmp(al, 2);
	if (flags.z())
		goto afterwalk;
	_cmp(al, 7);
	if (flags.z())
		goto afterwalk;
isdouble:
	walking();
afterwalk:
	_cmp(data.byte(kLinepointer), 254);
	if (!flags.z())
		goto notwalk;
	al = data.byte(kTurntoface);
	_cmp(al, data.byte(kFacing));
	if (!flags.z())
		goto notwalk;
	data.byte(kReasseschanges) = 1;
	al = data.byte(kFacing);
	_cmp(al, data.byte(kLeavedirection));
	if (!flags.z())
		goto notwalk;
	checkforexit();
notwalk:
	al = data.byte(kFacing);
	ah = 0;
	di = 1105;
	_add(di, ax);
	al = cs.byte(di);
	_add(al, es.byte(bx+29));
	es.byte(bx+15) = al;
	ax = es.word(bx+10);
	data.byte(kRyanx) = al;
	data.byte(kRyany) = ah;
}

void DreamGenContext::aboutturn() {
	STACK_CHECK;
	_cmp(data.byte(kTurndirection), 1);
	if (flags.z())
		goto incdir;
	_cmp(data.byte(kTurndirection), -1);
	if (flags.z())
		goto decdir;
	al = data.byte(kFacing);
	_sub(al, data.byte(kTurntoface));
	if (!flags.c())
		goto higher;
	_neg(al);
	_cmp(al, 4);
	if (!flags.c())
		goto decdir;
	goto incdir;
higher:
	_cmp(al, 4);
	if (!flags.c())
		goto incdir;
	goto decdir;
incdir:
	data.byte(kTurndirection) = 1;
	al = data.byte(kFacing);
	_inc(al);
	_and(al, 7);
	data.byte(kFacing) = al;
	es.byte(bx+29) = 0;
	return;
decdir:
	data.byte(kTurndirection) = -1;
	al = data.byte(kFacing);
	_dec(al);
	_and(al, 7);
	data.byte(kFacing) = al;
	es.byte(bx+29) = 0;
}

void DreamGenContext::facerightway() {
	STACK_CHECK;
	push(es);
	push(bx);
	getroomspaths();
	al = data.byte(kManspath);
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	_add(bx, ax);
	al = es.byte(bx+7);
	data.byte(kTurntoface) = al;
	data.byte(kLeavedirection) = al;
	bx = pop();
	es = pop();
}

void DreamGenContext::checkforexit() {
	STACK_CHECK;
	cl = data.byte(kRyanx);
	_add(cl, 12);
	ch = data.byte(kRyany);
	_add(ch, 12);
	checkone();
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
	isryanholding();
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
	isryanholding();
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
	setuptimeduse();
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
	adjustleft();
	return;
notaleft:
	_test(al, 2);
	if (flags.z())
		goto notaright;
	adjustright();
	return;
notaright:
	_test(al, 8);
	if (flags.z())
		goto notadown;
	adjustdown();
	return;
notadown:
	_test(al, 16);
	if (flags.z())
		return /* (notanup) */;
	adjustup();
}

void DreamGenContext::adjustdown() {
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

void DreamGenContext::adjustup() {
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

void DreamGenContext::adjustleft() {
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

void DreamGenContext::adjustright() {
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
	isryanholding();
	if (flags.z())
		goto forgotone;
	al = 'C';
	ah = 'S';
	cl = 'H';
	ch = 'R';
	findexobject();
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
	setuptimeduse();
}

void DreamGenContext::initrain() {
	STACK_CHECK;
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30));
	bx = 1113;
checkmorerain:
	al = cs.byte(bx);
	_cmp(al, 255);
	if (flags.z())
		goto finishinitrain;
	_cmp(al, data.byte(kReallocation));
	if (!flags.z())
		goto checkrain;
	al = cs.byte(bx+1);
	_cmp(al, data.byte(kMapx));
	if (!flags.z())
		goto checkrain;
	al = cs.byte(bx+2);
	_cmp(al, data.byte(kMapy));
	if (!flags.z())
		goto checkrain;
	al = cs.byte(bx+3);
	data.byte(kRainspace) = al;
	goto dorain;
checkrain:
	_add(bx, 4);
	goto checkmorerain;
dorain:
	cx = 4;
initraintop:
	randomnumber();
	_and(al, 31);
	_add(al, 3);
	_cmp(al, data.byte(kRainspace));
	if (!flags.c())
		goto initraintop;
	_add(cl, al);
	_cmp(cl, data.byte(kMapxsize));
	if (!flags.c())
		goto initrainside;
	push(cx);
	splitintolines();
	cx = pop();
	goto initraintop;
initrainside:
	cl = data.byte(kMapxsize);
	_dec(cl);
initrainside2:
	randomnumber();
	_and(al, 31);
	_add(al, 3);
	_cmp(al, data.byte(kRainspace));
	if (!flags.c())
		goto initrainside2;
	_add(ch, al);
	_cmp(ch, data.byte(kMapysize));
	if (!flags.c())
		goto finishinitrain;
	push(cx);
	splitintolines();
	cx = pop();
	goto initrainside2;
finishinitrain:
	al = 255;
	_stosb();
}

void DreamGenContext::splitintolines() {
	STACK_CHECK;
lookforlinestart:
	getblockofpixel();
	_cmp(al, 0);
	if (!flags.z())
		goto foundlinestart;
	_dec(cl);
	_inc(ch);
	_cmp(cl, 0);
	if (flags.z())
		return /* (endofthisline) */;
	_cmp(ch, data.byte(kMapysize));
	if (!flags.c())
		return /* (endofthisline) */;
	goto lookforlinestart;
foundlinestart:
	es.word(di) = cx;
	bh = 1;
lookforlineend:
	getblockofpixel();
	_cmp(al, 0);
	if (flags.z())
		goto foundlineend;
	_dec(cl);
	_inc(ch);
	_cmp(cl, 0);
	if (flags.z())
		goto foundlineend;
	_cmp(ch, data.byte(kMapysize));
	if (!flags.c())
		goto foundlineend;
	_inc(bh);
	goto lookforlineend;
foundlineend:
	push(cx);
	es.byte(di+2) = bh;
	randomnumber();
	es.byte(di+3) = al;
	randomnumber();
	es.byte(di+4) = al;
	randomnumber();
	_and(al, 3);
	_add(al, 4);
	es.byte(di+5) = al;
	_add(di, 6);
	cx = pop();
	_cmp(cl, 0);
	if (flags.z())
		return /* (endofthisline) */;
	_cmp(ch, data.byte(kMapysize));
	if (!flags.c())
		return /* (endofthisline) */;
	goto lookforlinestart;
}

void DreamGenContext::getblockofpixel() {
	STACK_CHECK;
	push(cx);
	push(es);
	push(di);
	ax = data.word(kMapxstart);
	_add(cl, al);
	ax = data.word(kMapystart);
	_add(ch, al);
	checkone();
	_and(cl, 1);
	if (!flags.z())
		goto failrain;
	di = pop();
	es = pop();
	cx = pop();
	return;
failrain:
	di = pop();
	es = pop();
	cx = pop();
	al = 0;
}

void DreamGenContext::showrain() {
	STACK_CHECK;
	ds = data.word(kMainsprites);
	si = 6*58;
	ax = ds.word(si+2);
	si = ax;
	_add(si, 2080);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30));
	es = data.word(kBuffers);
	_cmp(es.byte(bx), 255);
	if (flags.z())
		return /* (nothunder) */;
morerain:
	es = data.word(kBuffers);
	_cmp(es.byte(bx), 255);
	if (flags.z())
		goto finishrain;
	al = es.byte(bx+1);
	ah = 0;
	_add(ax, data.word(kMapady));
	_add(ax, data.word(kMapystart));
	cx = 320;
	_mul(cx);
	cl = es.byte(bx);
	ch = 0;
	_add(ax, cx);
	_add(ax, data.word(kMapadx));
	_add(ax, data.word(kMapxstart));
	di = ax;
	cl = es.byte(bx+2);
	ch = 0;
	ax = es.word(bx+3);
	dl = es.byte(bx+5);
	dh = 0;
	_sub(ax, dx);
	_and(ax, 511);
	es.word(bx+3) = ax;
	_add(bx, 6);
	push(si);
	_add(si, ax);
	es = data.word(kWorkspace);
	ah = 0;
	dx = 320-2;
rainloop:
	_lodsb();
	_cmp(al, ah);
	if (flags.z())
		goto noplot;
	_stosb();
	_add(di, dx);
	if (--cx)
		goto rainloop;
	si = pop();
	goto morerain;
noplot:
	_add(di, 320-1);
	if (--cx)
		goto rainloop;
	si = pop();
	goto morerain;
finishrain:
	_cmp(data.word(kCh1blockstocopy), 0);
	if (!flags.z())
		return /* (nothunder) */;
	_cmp(data.byte(kReallocation), 2);
	if (!flags.z())
		goto notlouisthund;
	_cmp(data.byte(kBeenmugged), 1);
	if (!flags.z())
		return /* (nothunder) */;
notlouisthund:
	_cmp(data.byte(kReallocation), 55);
	if (flags.z())
		return /* (nothunder) */;
	randomnum1();
	_cmp(al, 1);
	if (!flags.c())
		return /* (nothunder) */;
	al = 7;
	_cmp(data.byte(kCh0playing), 6);
	if (flags.z())
		goto isthunder1;
	al = 4;
isthunder1:
	playchannel1();
}

void DreamGenContext::backobject() {
	STACK_CHECK;
	ds = data.word(kSetdat);
	di = es.word(bx+20);
	al = es.byte(bx+18);
	_cmp(al, 0);
	if (flags.z())
		goto _tmp48z;
	_dec(al);
	es.byte(bx+18) = al;
	return /* (finishback) */;
_tmp48z:
	al = ds.byte(di+7);
	es.byte(bx+18) = al;
	al = ds.byte(di+8);
	_cmp(al, 6);
	if (!flags.z())
		goto notwidedoor;
	widedoor();
	return /* (finishback) */;
notwidedoor:
	_cmp(al, 5);
	if (!flags.z())
		goto notrandom;
	random();
	return /* (finishback) */;
notrandom:
	_cmp(al, 4);
	if (!flags.z())
		goto notlockdoor;
	lockeddoorway();
	return /* (finishback) */;
notlockdoor:
	_cmp(al, 3);
	if (!flags.z())
		goto notlift;
	liftsprite();
	return /* (finishback) */;
notlift:
	_cmp(al, 2);
	if (!flags.z())
		goto notdoor;
	doorway();
	return /* (finishback) */;
notdoor:
	_cmp(al, 1);
	if (!flags.z())
		goto steadyob;
	constant();
	return /* (finishback) */;
steadyob:
	steady();
}

void DreamGenContext::liftnoise() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 5);
	if (flags.z())
		goto hissnoise;
	_cmp(data.byte(kReallocation), 21);
	if (flags.z())
		goto hissnoise;
	playchannel1();
	return;
hissnoise:
	al = 13;
	playchannel1();
}

void DreamGenContext::random() {
	STACK_CHECK;
	randomnum1();
	push(di);
	_and(ax, 7);
	_add(di, 18);
	_add(di, ax);
	al = ds.byte(di);
	di = pop();
	es.byte(bx+15) = al;
}

void DreamGenContext::steady() {
	STACK_CHECK;
	al = ds.byte(di+18);
	ds.byte(di+17) = al;
	es.byte(bx+15) = al;
}

void DreamGenContext::constant() {
	STACK_CHECK;
	_inc(es.byte(bx+19));
	cl = es.byte(bx+19);
	ch = 0;
	_add(di, cx);
	_cmp(ds.byte(di+18), 255);
	if (!flags.z())
		goto gotconst;
	_sub(di, cx);
	cx = 0;
	es.byte(bx+19) = cl;
gotconst:
	al = ds.byte(di+18);
	_sub(di, cx);
	es.byte(bx+15) = al;
	ds.byte(di+17) = al;
}

void DreamGenContext::doorway() {
	STACK_CHECK;
	data.byte(kDoorcheck1) = -24;
	data.byte(kDoorcheck2) = 10;
	data.byte(kDoorcheck3) = -30;
	data.byte(kDoorcheck4) = 10;
	dodoor();
}

void DreamGenContext::widedoor() {
	STACK_CHECK;
	data.byte(kDoorcheck1) = -24;
	data.byte(kDoorcheck2) = 24;
	data.byte(kDoorcheck3) = -30;
	data.byte(kDoorcheck4) = 24;
	dodoor();
}

void DreamGenContext::lockeddoorway() {
	STACK_CHECK;
	al = data.byte(kRyanx);
	ah = data.byte(kRyany);
	cl = es.byte(bx+10);
	ch = es.byte(bx+11);
	_cmp(al, cl);
	if (!flags.c())
		goto rtofdoor2;
	_sub(al, cl);
	_cmp(al, -24);
	if (!flags.c())
		goto upordown2;
	goto shutdoor2;
rtofdoor2:
	_sub(al, cl);
	_cmp(al, 10);
	if (!flags.c())
		goto shutdoor2;
upordown2:
	_cmp(ah, ch);
	if (!flags.c())
		goto botofdoor2;
	_sub(ah, ch);
	_cmp(ah, -30);
	if (flags.c())
		goto shutdoor2;
	goto opendoor2;
botofdoor2:
	_sub(ah, ch);
	_cmp(ah, 12);
	if (!flags.c())
		goto shutdoor2;
opendoor2:
	_cmp(data.byte(kThroughdoor), 1);
	if (flags.z())
		goto mustbeopen;
	_cmp(data.byte(kLockstatus), 1);
	if (flags.z())
		goto shutdoor;
mustbeopen:
	cl = es.byte(bx+19);
	_cmp(cl, 1);
	if (!flags.z())
		goto notdoorsound4;
	al = 0;
	playchannel1();
notdoorsound4:
	_cmp(cl, 6);
	if (!flags.z())
		goto noturnonyet;
	al = data.byte(kDoorpath);
	push(es);
	push(bx);
	turnpathon();
	bx = pop();
	es = pop();
noturnonyet:
	cl = es.byte(bx+19);
	_cmp(data.byte(kThroughdoor), 1);
	if (!flags.z())
		goto notthrough2;
	_cmp(cl, 0);
	if (!flags.z())
		goto notthrough2;
	cl = 6;
notthrough2:
	_inc(cl);
	ch = 0;
	push(di);
	_add(di, cx);
	al = ds.byte(di+18);
	_cmp(al, 255);
	if (!flags.z())
		goto atlast3;
	_dec(di);
	_dec(cl);
atlast3:
	es.byte(bx+19) = cl;
	al = ds.byte(di+18);
	di = pop();
	es.byte(bx+15) = al;
	ds.byte(di+17) = al;
	_cmp(cl, 5);
	if (!flags.z())
		return /* (justshutting) */;
	data.byte(kThroughdoor) = 1;
	return;
shutdoor2:
	cl = es.byte(bx+19);
	_cmp(cl, 5);
	if (!flags.z())
		goto notdoorsound3;
	al = 1;
	playchannel1();
notdoorsound3:
	_cmp(cl, 0);
	if (flags.z())
		goto atlast4;
	_dec(cl);
	es.byte(bx+19) = cl;
atlast4:
	ch = 0;
	data.byte(kThroughdoor) = 0;
	push(di);
	_add(di, cx);
	al = ds.byte(di+18);
	di = pop();
	es.byte(bx+15) = al;
	ds.byte(di+17) = al;
	_cmp(cl, 0);
	if (!flags.z())
		return /* (notlocky) */;
	al = data.byte(kDoorpath);
	push(es);
	push(bx);
	turnpathoff();
	bx = pop();
	es = pop();
	data.byte(kLockstatus) = 1;
	return;
/*continuing to unbounded code: shutdoor from dodoor:60-87*/
shutdoor:
	cl = es.byte(bx+19);
	_cmp(cl, 5);
	if (!flags.z())
		goto notdoorsound1;
	al = 1;
	_cmp(data.byte(kReallocation), 5);
	if (!flags.z())
		goto nothoteldoor1;
	al = 13;
nothoteldoor1:
	playchannel1();
notdoorsound1:
	_cmp(cl, 0);
	if (flags.z())
		goto atlast2;
	_dec(cl);
	es.byte(bx+19) = cl;
atlast2:
	ch = 0;
	push(di);
	_add(di, cx);
	al = ds.byte(di+18);
	di = pop();
	es.byte(bx+15) = al;
	ds.byte(di+17) = al;
	_cmp(cl, 5);
	if (!flags.z())
		return /* (notnearly) */;
	data.byte(kThroughdoor) = 0;
}

void DreamGenContext::updatepeople() {
	STACK_CHECK;
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5));
	data.word(kListpos) = di;
	cx = 12*5;
	al = 255;
	_stosb(cx, true);
	_inc(data.word(kMaintimer));
	es = cs;
	bx = 534;
	di = 991;
updateloop:
	al = es.byte(bx);
	_cmp(al, 255);
	if (flags.z())
		return /* (endupdate) */;
	_cmp(al, data.byte(kReallocation));
	if (!flags.z())
		goto notinthisroom;
	cx = es.word(bx+1);
	_cmp(cl, data.byte(kMapx));
	if (!flags.z())
		goto notinthisroom;
	_cmp(ch, data.byte(kMapy));
	if (!flags.z())
		goto notinthisroom;
	push(di);
	ax = cs.word(di);
	__dispatch_call(ax);
	di = pop();
notinthisroom:
	_add(bx, 8);
	_add(di, 2);
	goto updateloop;
}

void DreamGenContext::getreelframeax() {
	STACK_CHECK;
	push(ds);
	data.word(kCurrentframe) = ax;
	findsource();
	es = ds;
	ds = pop();
	ax = data.word(kCurrentframe);
	_sub(ax, data.word(kTakeoff));
	_add(ax, ax);
	cx = ax;
	_add(ax, ax);
	_add(ax, cx);
	bx = ax;
}

void DreamGenContext::reelsonscreen() {
	STACK_CHECK;
	reconstruct();
	updatepeople();
	watchreel();
	showrain();
	usetimedtext();
}

void DreamGenContext::plotreel() {
	STACK_CHECK;
	getreelstart();
retryreel:
	push(es);
	push(si);
	ax = es.word(si+2);
	_cmp(al, 220);
	if (flags.c())
		goto normalreel;
	_cmp(al, 255);
	if (flags.z())
		goto normalreel;
	dealwithspecial();
	_inc(data.word(kReelpointer));
	si = pop();
	es = pop();
	_add(si, 40);
	goto retryreel;
normalreel:
	cx = 8;
plotloop:
	push(cx);
	push(es);
	push(si);
	ax = es.word(si);
	_cmp(ax, 0x0ffff);
	if (flags.z())
		goto notplot;
	showreelframe();
notplot:
	si = pop();
	es = pop();
	cx = pop();
	_add(si, 5);
	if (--cx)
		goto plotloop;
	soundonreels();
	bx = pop();
	es = pop();
}

void DreamGenContext::soundonreels() {
	STACK_CHECK;
	bl = data.byte(kReallocation);
	_add(bl, bl);
	_xor(bh, bh);
	_add(bx, 1214);
	si = cs.word(bx);
reelsoundloop:
	al = cs.byte(si);
	_cmp(al, 255);
	if (flags.z())
		goto endreelsound;
	ax = cs.word(si+1);
	_cmp(ax, data.word(kReelpointer));
	if (!flags.z())
		goto skipreelsound;
	_cmp(ax, data.word(kLastsoundreel));
	if (flags.z())
		goto skipreelsound;
	data.word(kLastsoundreel) = ax;
	al = cs.byte(si);
	_cmp(al, 64);
	if (flags.c())
		{ playchannel1(); return; };
	_cmp(al, 128);
	if (flags.c())
		goto channel0once;
	_and(al, 63);
	ah = 255;
	{ playchannel0(); return; };
channel0once:
	_and(al, 63);
	ah = 0;
	{ playchannel0(); return; };
skipreelsound:
	_add(si, 3);
	goto reelsoundloop;
endreelsound:
	ax = data.word(kLastsoundreel);
	_cmp(ax, data.word(kReelpointer));
	if (flags.z())
		return /* (nochange2) */;
	data.word(kLastsoundreel) = -1;
}

void DreamGenContext::reconstruct() {
	STACK_CHECK;
	_cmp(data.byte(kHavedoneobs), 0);
	if (flags.z())
		return /* (noneedtorecon) */;
	data.byte(kNewobs) = 1;
	drawfloor();
	spriteupdate();
	printsprites();
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto notfudge;
	_cmp(data.byte(kReallocation), 20);
	if (!flags.z())
		goto notfudge;
	undertextline();
notfudge:
	data.byte(kHavedoneobs) = 0;
}

void DreamGenContext::dealwithspecial() {
	STACK_CHECK;
	_sub(al, 220);
	_cmp(al, 0);
	if (!flags.z())
		goto notplset;
	al = ah;
	placesetobject();
	data.byte(kHavedoneobs) = 1;
	return;
notplset:
	_cmp(al, 1);
	if (!flags.z())
		goto notremset;
	al = ah;
	removesetobject();
	data.byte(kHavedoneobs) = 1;
	return;
notremset:
	_cmp(al, 2);
	if (!flags.z())
		goto notplfree;
	al = ah;
	placefreeobject();
	data.byte(kHavedoneobs) = 1;
	return;
notplfree:
	_cmp(al, 3);
	if (!flags.z())
		goto notremfree;
	al = ah;
	removefreeobject();
	data.byte(kHavedoneobs) = 1;
	return;
notremfree:
	_cmp(al, 4);
	if (!flags.z())
		goto notryanoff;
	switchryanoff();
	return;
notryanoff:
	_cmp(al, 5);
	if (!flags.z())
		goto notryanon;
	data.byte(kTurntoface) = ah;
	data.byte(kFacing) = ah;
	switchryanon();
	return;
notryanon:
	_cmp(al, 6);
	if (!flags.z())
		goto notchangeloc;
	data.byte(kNewlocation) = ah;
	return;
notchangeloc:
	movemap();
}

void DreamGenContext::movemap() {
	STACK_CHECK;
	_cmp(ah, 32);
	if (!flags.z())
		goto notmapup2;
	_sub(data.byte(kMapy), 20);
	data.byte(kNowinnewroom) = 1;
	return;
notmapup2:
	_cmp(ah, 16);
	if (!flags.z())
		goto notmapupspec;
	_sub(data.byte(kMapy), 10);
	data.byte(kNowinnewroom) = 1;
	return;
notmapupspec:
	_cmp(ah, 8);
	if (!flags.z())
		goto notmapdownspec;
	_add(data.byte(kMapy), 10);
	data.byte(kNowinnewroom) = 1;
	return;
notmapdownspec:
	_cmp(ah, 2);
	if (!flags.z())
		goto notmaprightspec;
	_add(data.byte(kMapx), 11);
	data.byte(kNowinnewroom) = 1;
	return;
notmaprightspec:
	_sub(data.byte(kMapx), 11);
	data.byte(kNowinnewroom) = 1;
}

void DreamGenContext::getreelstart() {
	STACK_CHECK;
	ax = data.word(kReelpointer);
	cx = 40;
	_mul(cx);
	es = data.word(kReels);
	si = ax;
	_add(si, (0+(36*144)));
}

void DreamGenContext::showreelframe() {
	STACK_CHECK;
	al = es.byte(si+2);
	ah = 0;
	di = ax;
	_add(di, data.word(kMapadx));
	al = es.byte(si+3);
	bx = ax;
	_add(bx, data.word(kMapady));
	ax = es.word(si);
	data.word(kCurrentframe) = ax;
	findsource();
	ax = data.word(kCurrentframe);
	_sub(ax, data.word(kTakeoff));
	ah = 8;
	showframe();
}

void DreamGenContext::deleverything() {
	STACK_CHECK;
	al = data.byte(kMapysize);
	ah = 0;
	_add(ax, data.word(kMapoffsety));
	_cmp(ax, 182);
	if (!flags.c())
		goto bigroom;
	maptopanel();
	return;
bigroom:
	_sub(data.byte(kMapysize), 8);
	maptopanel();
	_add(data.byte(kMapysize), 8);
}

void DreamGenContext::dumpeverything() {
	STACK_CHECK;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40));
dumpevery1:
	ax = es.word(bx);
	cx = es.word(bx+2);
	_cmp(ax, 0x0ffff);
	if (flags.z())
		goto finishevery1;
	_cmp(ax, es.word(bx+(40*5)));
	if (!flags.z())
		goto notskip1;
	_cmp(cx, es.word(bx+(40*5)+2));
	if (flags.z())
		goto skip1;
notskip1:
	push(bx);
	push(es);
	push(ds);
	bl = ah;
	bh = 0;
	ah = 0;
	di = ax;
	_add(di, data.word(kMapadx));
	_add(bx, data.word(kMapady));
	multidump();
	ds = pop();
	es = pop();
	bx = pop();
skip1:
	_add(bx, 5);
	goto dumpevery1;
finishevery1:
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40))+(40*5);
dumpevery2:
	ax = es.word(bx);
	cx = es.word(bx+2);
	_cmp(ax, 0x0ffff);
	if (flags.z())
		return /* (finishevery2) */;
	push(bx);
	push(es);
	push(ds);
	bl = ah;
	bh = 0;
	ah = 0;
	di = ax;
	_add(di, data.word(kMapadx));
	_add(bx, data.word(kMapady));
	multidump();
	ds = pop();
	es = pop();
	bx = pop();
	_add(bx, 5);
	goto dumpevery2;
}

void DreamGenContext::allocatework() {
	STACK_CHECK;
	bx = 0x1000;
	allocatemem();
	data.word(kWorkspace) = ax;
}

void DreamGenContext::loadpalfromiff() {
	STACK_CHECK;
	dx = 2481;
	openfile();
	cx = 2000;
	ds = data.word(kMapstore);
	dx = 0;
	readfromfile();
	closefile();
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768);
	ds = data.word(kMapstore);
	si = 0x30;
	cx = 768;
palloop:
	_lodsb();
	_shr(al, 1);
	_shr(al, 1);
	_cmp(data.byte(kBrightness), 1);
	if (!flags.z())
		goto nought;
	_cmp(al, 0);
	if (flags.z())
		goto nought;
	ah = al;
	_shr(ah, 1);
	_add(al, ah);
	_shr(ah, 1);
	_add(al, ah);
	_cmp(al, 64);
	if (flags.c())
		goto nought;
	al = 63;
nought:
	_stosb();
	if (--cx)
		goto palloop;
}

void DreamGenContext::paneltomap() {
	STACK_CHECK;
	di = data.word(kMapxstart);
	_add(di, data.word(kMapadx));
	bx = data.word(kMapystart);
	_add(bx, data.word(kMapady));
	ds = data.word(kMapstore);
	si = 0;
	cl = data.byte(kMapxsize);
	ch = data.byte(kMapysize);
	multiget();
}

void DreamGenContext::maptopanel() {
	STACK_CHECK;
	di = data.word(kMapxstart);
	_add(di, data.word(kMapadx));
	bx = data.word(kMapystart);
	_add(bx, data.word(kMapady));
	ds = data.word(kMapstore);
	si = 0;
	cl = data.byte(kMapxsize);
	ch = data.byte(kMapysize);
	multiput();
}

void DreamGenContext::dumpmap() {
	STACK_CHECK;
	di = data.word(kMapxstart);
	_add(di, data.word(kMapadx));
	bx = data.word(kMapystart);
	_add(bx, data.word(kMapady));
	cl = data.byte(kMapxsize);
	ch = data.byte(kMapysize);
	multidump();
}

void DreamGenContext::pixelcheckset() {
	STACK_CHECK;
	push(ax);
	_sub(al, es.byte(bx));
	_sub(ah, es.byte(bx+1));
	push(es);
	push(bx);
	push(cx);
	push(ax);
	al = es.byte(bx+4);
	getsetad();
	al = es.byte(bx+17);
	es = data.word(kSetframes);
	bx = (0);
	ah = 0;
	cx = 6;
	_mul(cx);
	_add(bx, ax);
	ax = pop();
	push(ax);
	al = ah;
	ah = 0;
	cl = es.byte(bx);
	ch = 0;
	_mul(cx);
	cx = pop();
	ch = 0;
	_add(ax, cx);
	_add(ax, es.word(bx+2));
	bx = ax;
	_add(bx, (0+2080));
	al = es.byte(bx);
	dl = al;
	cx = pop();
	bx = pop();
	es = pop();
	ax = pop();
	_cmp(dl, 0);
}

void DreamGenContext::createpanel() {
	STACK_CHECK;
	di = 0;
	bx = 8;
	ds = data.word(kIcons2);
	al = 0;
	ah = 2;
	showframe();
	di = 160;
	bx = 8;
	ds = data.word(kIcons2);
	al = 0;
	ah = 2;
	showframe();
	di = 0;
	bx = 104;
	ds = data.word(kIcons2);
	al = 0;
	ah = 2;
	showframe();
	di = 160;
	bx = 104;
	ds = data.word(kIcons2);
	al = 0;
	ah = 2;
	showframe();
}

void DreamGenContext::createpanel2() {
	STACK_CHECK;
	createpanel();
	di = 0;
	bx = 0;
	ds = data.word(kIcons2);
	al = 5;
	ah = 2;
	showframe();
	di = 160;
	bx = 0;
	ds = data.word(kIcons2);
	al = 5;
	ah = 2;
	showframe();
}

void DreamGenContext::clearwork() {
	STACK_CHECK;
	ax = 0x0;
	es = data.word(kWorkspace);
	di = 0;
	cx = (200*320)/64;
clearloop:
	_stosw(32);
	if (--cx)
		goto clearloop;
}

void DreamGenContext::zoom() {
	STACK_CHECK;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		return /* (inwatching) */;
	_cmp(data.byte(kZoomon), 1);
	if (flags.z())
		goto zoomswitch;
	return;
zoomswitch:
	_cmp(data.byte(kCommandtype), 199);
	if (flags.c())
		goto zoomit;
	putunderzoom();
	return;
zoomit:
	ax = data.word(kOldpointery);
	_sub(ax, 9);
	cx = (320);
	_mul(cx);
	_add(ax, data.word(kOldpointerx));
	_sub(ax, 11);
	si = ax;
	ax = (132)+4;
	cx = (320);
	_mul(cx);
	_add(ax, (8)+5);
	di = ax;
	es = data.word(kWorkspace);
	ds = data.word(kWorkspace);
	cx = 20;
zoomloop:
	push(cx);
	cx = 23;
zoomloop2:
	_lodsb();
	ah = al;
	_stosw();
	es.word(di+(320)-2) = ax;
	if (--cx)
		goto zoomloop2;
	_add(si, (320)-23);
	_add(di, (320)-46+(320));
	cx = pop();
	if (--cx)
		goto zoomloop;
	crosshair();
	data.byte(kDidzoom) = 1;
}

void DreamGenContext::delthisone() {
	STACK_CHECK;
	push(ax);
	push(ax);
	al = ah;
	ah = 0;
	_add(ax, data.word(kMapady));
	bx = (320);
	_mul(bx);
	bx = pop();
	bh = 0;
	_add(bx, data.word(kMapadx));
	_add(ax, bx);
	di = ax;
	ax = pop();
	push(ax);
	al = ah;
	ah = 0;
	bx = 22*8;
	_mul(bx);
	bx = pop();
	bh = 0;
	_add(ax, bx);
	si = ax;
	es = data.word(kWorkspace);
	ds = data.word(kMapstore);
	dl = cl;
	dh = 0;
	ax = (320);
	_sub(ax, dx);
	_neg(dx);
	_add(dx, 22*8);
deloneloop:
	push(cx);
	ch = 0;
	_movsb(cx, true);
	cx = pop();
	_add(di, ax);
	_add(si, dx);
	_dec(ch);
	if (!flags.z())
		goto deloneloop;
}

void DreamGenContext::doblocks() {
	STACK_CHECK;
	es = data.word(kWorkspace);
	ax = data.word(kMapady);
	cx = (320);
	_mul(cx);
	di = data.word(kMapadx);
	_add(di, ax);
	al = data.byte(kMapy);
	ah = 0;
	bx = (66);
	_mul(bx);
	bl = data.byte(kMapx);
	bh = 0;
	_add(ax, bx);
	si = (0);
	_add(si, ax);
	cx = 10;
loop120:
	push(di);
	push(cx);
	cx = 11;
loop124:
	push(cx);
	push(di);
	ds = data.word(kMapdata);
	_lodsb();
	ds = data.word(kBackdrop);
	push(si);
	_cmp(al, 0);
	if (flags.z())
		goto zeroblock;
	ah = al;
	al = 0;
	si = (0+192);
	_add(si, ax);
	bh = 14;
	bh = 4;
firstbitofblock:
	_movsw(8);
	_add(di, (320)-16);
	_dec(bh);
	if (!flags.z())
		goto firstbitofblock;
	bh = 12;
loop125:
	_movsw(8);
	ax = 0x0dfdf;
	_stosw(2);
	_add(di, (320)-20);
	_dec(bh);
	if (!flags.z())
		goto loop125;
	_add(di, 4);
	ax = 0x0dfdf;
	_stosw(8);
	_add(di, (320)-16);
	_stosw(8);
	_add(di, (320)-16);
	_stosw(8);
	_add(di, (320)-16);
	_stosw(8);
zeroblock:
	si = pop();
	di = pop();
	cx = pop();
	_add(di, 16);
	if (--cx)
		goto loop124;
	_add(si, (66)-11);
	cx = pop();
	di = pop();
	_add(di, (320)*16);
	if (--cx)
		goto loop120;
}

void DreamGenContext::transferinv() {
	STACK_CHECK;
	di = data.word(kExframepos);
	push(di);
	al = data.byte(kExpos);
	ah = 0;
	bx = ax;
	_add(ax, ax);
	_add(ax, bx);
	_inc(ax);
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
	_inc(ax);
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

void DreamGenContext::transfermap() {
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

void DreamGenContext::dofade() {
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
	showgroup();
	al = data.byte(kNumtofade);
	_add(al, data.byte(kColourpos));
	data.byte(kColourpos) = al;
	_cmp(al, 0);
	if (!flags.z())
		return /* (finishfade) */;
	fadecalculation();
}

void DreamGenContext::clearendpal() {
	STACK_CHECK;
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
	cx = 768;
	al = 0;
	_stosb(cx, true);
}

void DreamGenContext::clearpalette() {
	STACK_CHECK;
	data.byte(kFadedirection) = 0;
	clearstartpal();
	dumpcurrent();
}

void DreamGenContext::fadescreenup() {
	STACK_CHECK;
	clearstartpal();
	paltoendpal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
}

void DreamGenContext::fadetowhite() {
	STACK_CHECK;
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
	cx = 768;
	al = 63;
	_stosb(cx, true);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
	al = 0;
	_stosb(3);
	paltostartpal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
}

void DreamGenContext::fadefromwhite() {
	STACK_CHECK;
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3));
	cx = 768;
	al = 63;
	_stosb(cx, true);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3));
	al = 0;
	_stosb(3);
	paltoendpal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
}

void DreamGenContext::fadescreenups() {
	STACK_CHECK;
	clearstartpal();
	paltoendpal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 64;
}

void DreamGenContext::fadescreendownhalf() {
	STACK_CHECK;
	paltostartpal();
	paltoendpal();
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

void DreamGenContext::fadescreenuphalf() {
	STACK_CHECK;
	endpaltostart();
	paltoendpal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 31;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 32;
}

void DreamGenContext::fadescreendown() {
	STACK_CHECK;
	paltostartpal();
	clearendpal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
}

void DreamGenContext::fadescreendowns() {
	STACK_CHECK;
	paltostartpal();
	clearendpal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 64;
}

void DreamGenContext::clearstartpal() {
	STACK_CHECK;
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3));
	cx = 256;
wholeloop1:
	ax = 0;
	_stosw();
	al = 0;
	_stosb();
	if (--cx)
		goto wholeloop1;
}

void DreamGenContext::showgun() {
	STACK_CHECK;
	data.byte(kAddtored) = 0;
	data.byte(kAddtogreen) = 0;
	data.byte(kAddtoblue) = 0;
	paltostartpal();
	paltoendpal();
	greyscalesum();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	cx = 130;
	hangon();
	endpaltostart();
	clearendpal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	cx = 200;
	hangon();
	data.byte(kRoomssample) = 34;
	loadroomssample();
	data.byte(kVolume) = 0;
	dx = 2351;
	loadintotemp();
	createpanel2();
	ds = data.word(kTempgraphics);
	al = 0;
	ah = 0;
	di = 100;
	bx = 4;
	showframe();
	ds = data.word(kTempgraphics);
	al = 1;
	ah = 0;
	di = 158;
	bx = 106;
	showframe();
	worktoscreen();
	getridoftemp();
	fadescreenup();
	cx = 160;
	hangon();
	al = 12;
	ah = 0;
	playchannel0();
	dx = 2260;
	loadtemptext();
	rollendcredits2();
	getridoftemptext();
}

void DreamGenContext::rollendcredits2() {
	STACK_CHECK;
	rollem();
}

void DreamGenContext::rollem() {
	STACK_CHECK;
	cl = 160;
	ch = 160;
	di = 25;
	bx = 20;
	ds = data.word(kMapstore);
	si = 0;
	multiget();
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
	vsync();
	cl = 160;
	ch = 160;
	di = 25;
	bx = 20;
	ds = data.word(kMapstore);
	si = 0;
	multiput();
	vsync();
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
	printdirect();
	_add(bx, data.word(kLinespacing));
	cx = pop();
	if (--cx)
		goto onelot2;
	vsync();
	cl = 160;
	ch = 160;
	di = 25;
	bx = 20;
	multidump();
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
	hangone();
	return;
endearly2:
	cx = pop();
}

void DreamGenContext::fadecalculation() {
	STACK_CHECK;
	_cmp(data.byte(kFadecount), 0);
	if (flags.z())
		goto nomorefading;
	bl = data.byte(kFadecount);
	es = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3));
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
	cx = 768;
fadecolloop:
	al = es.byte(si);
	ah = es.byte(di);
	_cmp(al, ah);
	if (flags.z())
		goto gotthere;
	if (flags.c())
		goto lesscolour;
	_dec(es.byte(si));
	goto gotthere;
lesscolour:
	_cmp(bl, ah);
	if (flags.z())
		goto withit;
	if (!flags.c())
		goto gotthere;
withit:
	_inc(es.byte(si));
gotthere:
	_inc(si);
	_inc(di);
	if (--cx)
		goto fadecolloop;
	_dec(data.byte(kFadecount));
	return;
nomorefading:
	data.byte(kFadedirection) = 0;
}

void DreamGenContext::greyscalesum() {
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

void DreamGenContext::paltostartpal() {
	STACK_CHECK;
	es = data.word(kBuffers);
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3));
	cx = 768/2;
	_movsw(cx, true);
}

void DreamGenContext::endpaltostart() {
	STACK_CHECK;
	es = data.word(kBuffers);
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3));
	cx = 768/2;
	_movsw(cx, true);
}

void DreamGenContext::startpaltoend() {
	STACK_CHECK;
	es = data.word(kBuffers);
	ds = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3));
	cx = 768/2;
	_movsw(cx, true);
}

void DreamGenContext::paltoendpal() {
	STACK_CHECK;
	es = data.word(kBuffers);
	ds = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768);
	cx = 768/2;
	_movsw(cx, true);
}

void DreamGenContext::allpalette() {
	STACK_CHECK;
	es = data.word(kBuffers);
	ds = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3));
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768);
	cx = 768/2;
	_movsw(cx, true);
	dumpcurrent();
}

void DreamGenContext::dumpcurrent() {
	STACK_CHECK;
	si = (0+(228*13)+32+60+(32*32)+(11*10*3));
	ds = data.word(kBuffers);
	vsync();
	al = 0;
	cx = 128;
	showgroup();
	vsync();
	al = 128;
	cx = 128;
	showgroup();
}

void DreamGenContext::fadedownmon() {
	STACK_CHECK;
	paltostartpal();
	paltoendpal();
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
	hangon();
}

void DreamGenContext::fadeupmon() {
	STACK_CHECK;
	paltostartpal();
	paltoendpal();
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
	hangon();
}

void DreamGenContext::fadeupmonfirst() {
	STACK_CHECK;
	paltostartpal();
	paltoendpal();
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
	cx = 64;
	hangon();
	al = 26;
	playchannel1();
	cx = 64;
	hangon();
}

void DreamGenContext::fadeupyellows() {
	STACK_CHECK;
	paltoendpal();
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
	cx = 128;
	hangon();
}

void DreamGenContext::initialmoncols() {
	STACK_CHECK;
	paltostartpal();
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
	showgroup();
}

void DreamGenContext::titles() {
	STACK_CHECK;
	clearpalette();
	biblequote();
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (titlesearly) */;
	intro();
}

void DreamGenContext::endgame() {
	STACK_CHECK;
	dx = 2260;
	loadtemptext();
	monkspeaking();
	gettingshot();
	getridoftemptext();
	data.byte(kVolumeto) = 7;
	data.byte(kVolumedirection) = 1;
	cx = 200;
	hangon();
}

void DreamGenContext::monkspeaking() {
	STACK_CHECK;
	data.byte(kRoomssample) = 35;
	loadroomssample();
	dx = 2364;
	loadintotemp();
	clearwork();
	showmonk();
	worktoscreen();
	data.byte(kVolume) = 7;
	data.byte(kVolumedirection) = -1;
	data.byte(kVolumeto) = 5;
	al = 12;
	ah = 255;
	playchannel0();
	fadescreenups();
	cx = 300;
	hangon();
	al = 40;
loadspeech2:
	push(ax);
	dl = 'T';
	dh = 83;
	cl = 'T';
	ah = 0;
	loadspeech();
	al = 50+12;
	playchannel1();
notloadspeech2:
	vsync();
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
	fadescreendowns();
	cx = 300;
	hangon();
	getridoftemp();
}

void DreamGenContext::showmonk() {
	STACK_CHECK;
	al = 0;
	ah = 128;
	di = 160;
	bx = 72;
	ds = data.word(kTempgraphics);
	showframe();
}

void DreamGenContext::gettingshot() {
	STACK_CHECK;
	data.byte(kNewlocation) = 55;
	clearpalette();
	loadintroroom();
	fadescreenups();
	data.byte(kVolumeto) = 0;
	data.byte(kVolumedirection) = -1;
	runendseq();
	clearbeforeload();
}

void DreamGenContext::credits() {
	STACK_CHECK;
	clearpalette();
	realcredits();
}

void DreamGenContext::biblequote() {
	STACK_CHECK;
	mode640x480();
	dx = 2377;
	showpcx();
	fadescreenups();
	cx = 80;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto biblequotearly;
	cx = 560;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto biblequotearly;
	fadescreendowns();
	cx = 200;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto biblequotearly;
	cancelch0();
biblequotearly:
	data.byte(kLasthardkey) = 0;
}

void DreamGenContext::hangone() {
	STACK_CHECK;
hangonloope:
	push(cx);
	vsync();
	cx = pop();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		return /* (hangonearly) */;
	if (--cx)
		goto hangonloope;
}

void DreamGenContext::intro() {
	STACK_CHECK;
	dx = 2247;
	loadtemptext();
	loadpalfromiff();
	setmode();
	data.byte(kNewlocation) = 50;
	clearpalette();
	loadintroroom();
	data.byte(kVolume) = 7;
	data.byte(kVolumedirection) = -1;
	data.byte(kVolumeto) = 4;
	al = 12;
	ah = 255;
	playchannel0();
	fadescreenups();
	runintroseq();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto introearly;
	clearbeforeload();
	data.byte(kNewlocation) = 52;
	loadintroroom();
	runintroseq();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto introearly;
	clearbeforeload();
	data.byte(kNewlocation) = 53;
	loadintroroom();
	runintroseq();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto introearly;
	clearbeforeload();
	allpalette();
	data.byte(kNewlocation) = 54;
	loadintroroom();
	runintroseq();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto introearly;
	getridoftemptext();
	clearbeforeload();
introearly:
	data.byte(kLasthardkey) =  0;
}

void DreamGenContext::runintroseq() {
	STACK_CHECK;
	data.byte(kGetback) = 0;
moreintroseq:
	vsync();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto earlyendrun;
	spriteupdate();
	vsync();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto earlyendrun;
	deleverything();
	printsprites();
	reelsonscreen();
	afterintroroom();
	usetimedtext();
	vsync();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto earlyendrun;
	dumpmap();
	dumptimedtext();
	vsync();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto earlyendrun;
	_cmp(data.byte(kGetback), 1);
	if (!flags.z())
		goto moreintroseq;
	return;
earlyendrun:
	getridoftemptext();
	clearbeforeload();
}

void DreamGenContext::runendseq() {
	STACK_CHECK;
	atmospheres();
	data.byte(kGetback) = 0;
moreendseq:
	vsync();
	spriteupdate();
	vsync();
	deleverything();
	printsprites();
	reelsonscreen();
	afterintroroom();
	usetimedtext();
	vsync();
	dumpmap();
	dumptimedtext();
	vsync();
	_cmp(data.byte(kGetback), 1);
	if (!flags.z())
		goto moreendseq;
}

void DreamGenContext::loadintroroom() {
	STACK_CHECK;
	data.byte(kIntrocount) = 0;
	data.byte(kLocation) = 255;
	loadroom();
	data.word(kMapoffsetx) = 72;
	data.word(kMapoffsety) = 16;
	clearsprites();
	data.byte(kThroughdoor) = 0;
	data.byte(kCurrentkey) = '0';
	data.byte(kMainmode) = 0;
	clearwork();
	data.byte(kNewobs) = 1;
	drawfloor();
	reelsonscreen();
	spriteupdate();
	printsprites();
	worktoscreen();
}

void DreamGenContext::realcredits() {
	STACK_CHECK;
	data.byte(kRoomssample) = 33;
	loadroomssample();
	data.byte(kVolume) = 0;
	mode640x480();
	cx = 35;
	hangon();
	dx = 2390;
	showpcx();
	al = 12;
	ah = 0;
	playchannel0();
	cx = 2;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	allpalette();
	cx = 80;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	fadescreendowns();
	cx = 256;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	dx = 2403;
	showpcx();
	al = 12;
	ah = 0;
	playchannel0();
	cx = 2;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	allpalette();
	cx = 80;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	fadescreendowns();
	cx = 256;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	dx = 2416;
	showpcx();
	al = 12;
	ah = 0;
	playchannel0();
	cx = 2;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	allpalette();
	cx = 80;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	fadescreendowns();
	cx = 256;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	dx = 2429;
	showpcx();
	al = 12;
	ah = 0;
	playchannel0();
	cx = 2;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	allpalette();
	cx = 80;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	fadescreendowns();
	cx = 256;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	dx = 2442;
	showpcx();
	al = 12;
	ah = 0;
	playchannel0();
	cx = 2;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	allpalette();
	cx = 80;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	fadescreendowns();
	cx = 256;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	dx = 2455;
	showpcx();
	fadescreenups();
	cx = 60;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	al = 13;
	ah = 0;
	playchannel0();
	cx = 350;
	hangone();
	_cmp(data.byte(kLasthardkey), 1);
	if (flags.z())
		goto realcreditsearly;
	fadescreendowns();
	cx = 256;
	hangone();
realcreditsearly:
	data.byte(kLasthardkey) =  0;
}

void DreamGenContext::waitframes() {
	STACK_CHECK;
	push(di);
	push(bx);
	push(es);
	push(si);
	push(ds);
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	delpointer();
	ax = data.word(kMousebutton);
	ds = pop();
	si = pop();
	es = pop();
	bx = pop();
	di = pop();
}

void DreamGenContext::monprint() {
	STACK_CHECK;
	data.byte(kKerning) = 1;
	si = bx;
	dl = 166;
	di = data.word(kMonadx);
	bx = data.word(kMonady);
	ds = data.word(kTempcharset);
printloop8:
	push(bx);
	push(di);
	push(dx);
	getnumber();
	ch = 0;
printloop7:
	al = es.byte(si);
	_inc(si);
	_cmp(al, ':');
	if (flags.z())
		goto finishmon2;
	_cmp(al, 0);
	if (flags.z())
		goto finishmon;
	_cmp(al, 34);
	if (flags.z())
		goto finishmon;
	_cmp(al, '=');
	if (flags.z())
		goto finishmon;
	_cmp(al, '%');
	if (!flags.z())
		goto nottrigger;
	ah = es.byte(si);
	_inc(si);
	_inc(si);
	goto finishmon;
nottrigger:
	push(cx);
	push(es);
	modifychar();
	printchar();
	data.word(kCurslocx) = di;
	data.word(kCurslocy) = bx;
	data.word(kMaintimer) = 1;
	printcurs();
	vsync();
	push(si);
	push(dx);
	push(ds);
	push(es);
	push(bx);
	push(di);
	lockmon();
	di = pop();
	bx = pop();
	es = pop();
	ds = pop();
	dx = pop();
	si = pop();
	delcurs();
	es = pop();
	cx = pop();
	if (--cx)
		goto printloop7;
finishmon2:
	dx = pop();
	di = pop();
	bx = pop();
	scrollmonitor();
	data.word(kCurslocx) = di;
	goto printloop8;
finishmon:
	dx = pop();
	di = pop();
	bx = pop();
	_cmp(al, '%');
	if (!flags.z())
		goto nottrigger2;
	data.byte(kLasttrigger) = ah;
nottrigger2:
	data.word(kCurslocx) = di;
	scrollmonitor();
	bx = si;
	data.byte(kKerning) = 0;
}

void DreamGenContext::fillryan() {
	STACK_CHECK;
	es = data.word(kBuffers);
	di = (0+(228*13)+32);
	findallryan();
	si = (0+(228*13)+32);
	al = data.byte(kRyanpage);
	ah = 0;
	cx = 20;
	_mul(cx);
	_add(si, ax);
	di = (80);
	bx = (58);
	cx = 2;
ryanloop2:
	push(cx);
	push(di);
	push(bx);
	cx = 5;
ryanloop1:
	push(cx);
	push(di);
	push(bx);
	ax = es.word(si);
	_add(si, 2);
	push(si);
	push(es);
	obtoinv();
	es = pop();
	si = pop();
	bx = pop();
	di = pop();
	cx = pop();
	_add(di, (44));
	if (--cx)
		goto ryanloop1;
	bx = pop();
	di = pop();
	cx = pop();
	_add(bx, (44));
	if (--cx)
		goto ryanloop2;
	showryanpage();
}

void DreamGenContext::fillopen() {
	STACK_CHECK;
	deltextline();
	getopenedsize();
	_cmp(ah, 4);
	if (flags.c())
		goto lessthanapage;
	ah = 4;
lessthanapage:
	al = 1;
	push(ax);
	es = data.word(kBuffers);
	di = (0+(228*13));
	findallopen();
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
	obtoinv();
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
	undertextline();
}

void DreamGenContext::findallryan() {
	STACK_CHECK;
	push(di);
	cx = 30;
	ax = 0x0ffff;
	_stosw(cx, true);
	di = pop();
	cl = 4;
	ds = data.word(kExtras);
	bx = (0+2080+30000);
	ch = 0;
findryanloop:
	_cmp(ds.byte(bx+2), cl);
	if (!flags.z())
		goto notinryaninv;
	_cmp(ds.byte(bx+3), 255);
	if (!flags.z())
		goto notinryaninv;
	al = ds.byte(bx+4);
	ah = 0;
	push(di);
	_add(di, ax);
	_add(di, ax);
	al = ch;
	ah = 4;
	_stosw();
	di = pop();
notinryaninv:
	_add(bx, 16);
	_inc(ch);
	_cmp(ch, (114));
	if (!flags.z())
		goto findryanloop;
}

void DreamGenContext::findallopen() {
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

void DreamGenContext::obtoinv() {
	STACK_CHECK;
	push(bx);
	push(es);
	push(si);
	push(ax);
	push(ax);
	push(di);
	push(bx);
	ds = data.word(kIcons1);
	_sub(di, 2);
	_sub(bx, 1);
	al = 10;
	ah = 0;
	showframe();
	bx = pop();
	di = pop();
	ax = pop();
	_cmp(al, 255);
	if (flags.z())
		goto finishfill;
	push(bx);
	push(di);
	push(ax);
	ds = data.word(kExtras);
	_cmp(ah, 4);
	if (flags.z())
		goto isanextra;
	ds = data.word(kFreeframes);
isanextra:
	cl = al;
	_add(al, al);
	_add(al, cl);
	_inc(al);
	ah = 128;
	_add(bx, 19);
	_add(di, 18);
	showframe();
	ax = pop();
	di = pop();
	bx = pop();
	push(bx);
	getanyaddir();
	isitworn();
	bx = pop();
	if (!flags.z())
		goto finishfill;
	ds = data.word(kIcons1);
	_sub(di, 3);
	_sub(bx, 2);
	al = 7;
	ah = 0;
	showframe();
finishfill:
	ax = pop();
	si = pop();
	es = pop();
	bx = pop();
}

void DreamGenContext::isitworn() {
	STACK_CHECK;
	al = es.byte(bx+12);
	_cmp(al, 'W'-'A');
	if (!flags.z())
		return /* (notworn) */;
	al = es.byte(bx+13);
	_cmp(al, 'E'-'A');
}

void DreamGenContext::makeworn() {
	STACK_CHECK;
	es.byte(bx+12) = 'W'-'A';
	es.byte(bx+13) = 'E'-'A';
}

void DreamGenContext::examineob() {
	STACK_CHECK;
	data.byte(kPointermode) = 0;
	data.word(kTimecount) = 0;
examineagain:
	data.byte(kInmaparea) = 0;
	data.byte(kExamagain) = 0;
	data.byte(kOpenedob) = 255;
	data.byte(kOpenedtype) = 255;
	data.byte(kInvopen) = 0;
	al = data.byte(kCommandtype);
	data.byte(kObjecttype) = al;
	data.byte(kItemframe) = 0;
	data.byte(kPointerframe) = 0;
	createpanel();
	showpanel();
	showman();
	showexit();
	obicons();
	obpicture();
	describeob();
	undertextline();
	data.byte(kCommandtype) = 255;
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
waitexam:
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	delpointer();
	data.byte(kGetback) = 0;
	bx = 2494;
	_cmp(data.byte(kInvopen), 0);
	if (flags.z())
		goto notuseinv;
	bx = 2556;
	_cmp(data.byte(kInvopen), 1);
	if (flags.z())
		goto notuseinv;
	bx = 2618;
notuseinv:
	checkcoords();
	_cmp(data.byte(kExamagain), 0);
	if (flags.z())
		goto norex;
	goto examineagain;
norex:
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto waitexam;
	data.byte(kPickup) = 0;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		goto iswatching;
	_cmp(data.byte(kNewlocation), 255);
	if (!flags.z())
		goto justgetback;
iswatching:
	makemainscreen();
	data.byte(kInvopen) = 0;
	data.byte(kOpenedob) = 255;
	return;
justgetback:
	data.byte(kInvopen) = 0;
	data.byte(kOpenedob) = 255;
}

void DreamGenContext::makemainscreen() {
	STACK_CHECK;
	createpanel();
	data.byte(kNewobs) = 1;
	drawfloor();
	spriteupdate();
	printsprites();
	reelsonscreen();
	showicon();
	getunderzoom();
	undertextline();
	data.byte(kCommandtype) = 255;
	animpointer();
	worktoscreenm();
	data.byte(kCommandtype) = 200;
	data.byte(kManisoffscreen) = 0;
}

void DreamGenContext::getbackfromob() {
	STACK_CHECK;
	_cmp(data.byte(kPickup), 1);
	if (!flags.z())
		goto notheldob;
	blank();
	return;
notheldob:
	getback1();
}

void DreamGenContext::incryanpage() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 222);
	if (flags.z())
		goto alreadyincryan;
	data.byte(kCommandtype) = 222;
	al = 31;
	commandonly();
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
	delpointer();
	fillryan();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::openinv() {
	STACK_CHECK;
	data.byte(kInvopen) = 1;
	al = 61;
	di = (80);
	bx = (58)-10;
	dl = 240;
	printmessage();
	fillryan();
	data.byte(kCommandtype) = 255;
}

void DreamGenContext::showryanpage() {
	STACK_CHECK;
	ds = data.word(kIcons1);
	di = (80)+167;
	bx = (58)-12;
	al = 12;
	ah = 0;
	showframe();
	al = 13;
	_add(al, data.byte(kRyanpage));
	push(ax);
	al = data.byte(kRyanpage);
	ah = 0;
	cx = 18;
	_mul(cx);
	ds = data.word(kIcons1);
	di = (80)+167;
	_add(di, ax);
	bx = (58)-12;
	ax = pop();
	ah = 0;
	showframe();
}

void DreamGenContext::openob() {
	STACK_CHECK;
	al = data.byte(kOpenedob);
	ah = data.byte(kOpenedtype);
	di = 5847;
	copyname();
	di = (80);
	bx = (58)+86;
	al = 62;
	dl = 240;
	printmessage();
	di = data.word(kLastxpos);
	_add(di, 5);
	bx = (58)+86;
	es = cs;
	si = 5847;
	dl = 220;
	al = 0;
	ah = 0;
	printdirect();
	fillopen();
	getopenedsize();
	al = ah;
	ah = 0;
	cx = (44);
	_mul(cx);
	_add(ax, (80));
	bx = 2588;
	cs.word(bx) = ax;
}

void DreamGenContext::obicons() {
	STACK_CHECK;
	al = data.byte(kCommand);
	getanyad();
	_cmp(al, 255);
	if (flags.z())
		goto cantopenit;
	ds = data.word(kIcons2);
	di = 210;
	bx = 1;
	al = 4;
	ah = 0;
	showframe();
cantopenit:
	ds = data.word(kIcons2);
	di = 260;
	bx = 1;
	al = 1;
	ah = 0;
	showframe();
}

void DreamGenContext::examicon() {
	STACK_CHECK;
	ds = data.word(kIcons2);
	di = 254;
	bx = 5;
	al = 3;
	ah = 0;
	showframe();
}

void DreamGenContext::obpicture() {
	STACK_CHECK;
	al = data.byte(kCommand);
	ah = data.byte(kObjecttype);
	_cmp(ah, 1);
	if (flags.z())
		return /* (setframe) */;
	_cmp(ah, 4);
	if (flags.z())
		goto exframe;
	ds = data.word(kFreeframes);
	di = 160;
	bx = 68;
	cl = al;
	_add(al, al);
	_add(al, cl);
	_inc(al);
	ah = 128;
	showframe();
	return;
exframe:
	ds = data.word(kExtras);
	di = 160;
	bx = 68;
	cl = al;
	_add(al, al);
	_add(al, cl);
	_inc(al);
	ah = 128;
	showframe();
}

void DreamGenContext::describeob() {
	STACK_CHECK;
	getobtextstart();
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
	printdirect();
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
	printdirect();
	push(bx);
	obsthatdothings();
	bx = pop();
	additionaltext();
}

void DreamGenContext::additionaltext() {
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
	findpuztext();
	bx = pop();
	di = 36;
	dl = 241;
	ah = 0;
	printdirect();
	return;
fullcup:
	al = 39;
	findpuztext();
	bx = pop();
	di = 36;
	dl = 241;
	ah = 0;
	printdirect();
}

void DreamGenContext::obsthatdothings() {
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
	getlocation();
	_cmp(al, 1);
	if (flags.z())
		return /* (seencard) */;
	al = 4;
	setlocation();
	lookatcard();
}

void DreamGenContext::getobtextstart() {
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
	findnextcolon();
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
	searchforsame();
	goto tryagain;
}

void DreamGenContext::searchforsame() {
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

void DreamGenContext::findnextcolon() {
	STACK_CHECK;
isntcolon:
	al = es.byte(si);
	_inc(si);
	_cmp(al, 0);
	if (flags.z())
		return /* (endofcolon) */;
	_cmp(al, ':');
	if (!flags.z())
		goto isntcolon;
}

void DreamGenContext::inventory() {
	STACK_CHECK;
	_cmp(data.byte(kMandead), 1);
	if (flags.z())
		goto iswatchinv;
	_cmp(data.word(kWatchingtime), 0);
	if (flags.z())
		goto notwatchinv;
iswatchinv:
	blank();
	return;
notwatchinv:
	_cmp(data.byte(kCommandtype), 239);
	if (flags.z())
		goto alreadyopinv;
	data.byte(kCommandtype) = 239;
	al = 32;
	commandonly();
alreadyopinv:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (cantopinv) */;
	_and(ax, 1);
	if (!flags.z())
		goto doopeninv;
	return;
doopeninv:
	data.word(kTimecount) = 0;
	data.byte(kPointermode) = 0;
	data.byte(kInmaparea) = 0;
	animpointer();
	createpanel();
	showpanel();
	examicon();
	showman();
	showexit();
	undertextline();
	data.byte(kPickup) = 0;
	data.byte(kInvopen) = 2;
	openinv();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
	data.byte(kOpenedob) = 255;
	goto waitexam;
	return;
/*continuing to unbounded code: examineagain from examineob:3-66*/
examineagain:
	data.byte(kInmaparea) = 0;
	data.byte(kExamagain) = 0;
	data.byte(kOpenedob) = 255;
	data.byte(kOpenedtype) = 255;
	data.byte(kInvopen) = 0;
	al = data.byte(kCommandtype);
	data.byte(kObjecttype) = al;
	data.byte(kItemframe) = 0;
	data.byte(kPointerframe) = 0;
	createpanel();
	showpanel();
	showman();
	showexit();
	obicons();
	obpicture();
	describeob();
	undertextline();
	data.byte(kCommandtype) = 255;
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
waitexam:
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	delpointer();
	data.byte(kGetback) = 0;
	bx = 2494;
	_cmp(data.byte(kInvopen), 0);
	if (flags.z())
		goto notuseinv;
	bx = 2556;
	_cmp(data.byte(kInvopen), 1);
	if (flags.z())
		goto notuseinv;
	bx = 2618;
notuseinv:
	checkcoords();
	_cmp(data.byte(kExamagain), 0);
	if (flags.z())
		goto norex;
	goto examineagain;
norex:
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto waitexam;
	data.byte(kPickup) = 0;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		goto iswatching;
	_cmp(data.byte(kNewlocation), 255);
	if (!flags.z())
		goto justgetback;
iswatching:
	makemainscreen();
	data.byte(kInvopen) = 0;
	data.byte(kOpenedob) = 255;
	return;
justgetback:
	data.byte(kInvopen) = 0;
	data.byte(kOpenedob) = 255;
}

void DreamGenContext::setpickup() {
	STACK_CHECK;
	_cmp(data.byte(kObjecttype), 1);
	if (flags.z())
		goto cantpick;
	_cmp(data.byte(kObjecttype), 3);
	if (flags.z())
		goto cantpick;
	getanyad();
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
	commandwithob();
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
	createpanel();
	showpanel();
	showman();
	showexit();
	examicon();
	data.byte(kPickup) = 1;
	data.byte(kInvopen) = 2;
	_cmp(data.byte(kObjecttype), 4);
	if (flags.z())
		goto pickupexob;
	al = data.byte(kCommand);
	data.byte(kItemframe) = al;
	data.byte(kOpenedob) = 255;
	transfertoex();
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = 4;
	geteitherad();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
	openinv();
	worktoscreenm();
	return;
pickupexob:
	al = data.byte(kCommand);
	data.byte(kItemframe) = al;
	data.byte(kOpenedob) = 255;
	openinv();
	worktoscreenm();
}

void DreamGenContext::examinventory() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 249);
	if (flags.z())
		goto alreadyexinv;
	data.byte(kCommandtype) = 249;
	al = 32;
	commandonly();
alreadyexinv:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (!flags.z())
		goto doexinv;
	return;
doexinv:
	createpanel();
	showpanel();
	showman();
	showexit();
	examicon();
	data.byte(kPickup) = 0;
	data.byte(kInvopen) = 2;
	openinv();
	worktoscreenm();
}

void DreamGenContext::reexfrominv() {
	STACK_CHECK;
	findinvpos();
	ax = es.word(bx);
	data.byte(kCommandtype) = ah;
	data.byte(kCommand) = al;
	data.byte(kExamagain) = 1;
	data.byte(kPointermode) = 0;
}

void DreamGenContext::reexfromopen() {
	STACK_CHECK;
	return;
	findopenpos();
	ax = es.word(bx);
	data.byte(kCommandtype) = ah;
	data.byte(kCommand) = al;
	data.byte(kExamagain) = 1;
	data.byte(kPointermode) = 0;
}

void DreamGenContext::swapwithinv() {
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
	commandwithob();
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
	findinvpos();
	ax = es.word(bx);
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = ah;
	geteitherad();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
	bl = data.byte(kItemframe);
	bh = data.byte(kObjecttype);
	ax = pop();
	data.byte(kObjecttype) = ah;
	data.byte(kItemframe) = al;
	push(bx);
	findinvpos();
	delpointer();
	al = data.byte(kItemframe);
	geteitherad();
	es.byte(bx+2) = 4;
	es.byte(bx+3) = 255;
	al = data.byte(kLastinvpos);
	es.byte(bx+4) = al;
	ax = pop();
	data.byte(kObjecttype) = ah;
	data.byte(kItemframe) = al;
	fillryan();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::swapwithopen() {
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
	commandwithob();
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
	geteitherad();
	isitworn();
	if (!flags.z())
		goto notwornswap;
	wornerror();
	return;
notwornswap:
	delpointer();
	al = data.byte(kItemframe);
	_cmp(al, data.byte(kOpenedob));
	if (!flags.z())
		goto isntsame2;
	al = data.byte(kObjecttype);
	_cmp(al, data.byte(kOpenedtype));
	if (!flags.z())
		goto isntsame2;
	errormessage1();
	return;
isntsame2:
	checkobjectsize();
	_cmp(al, 0);
	if (flags.z())
		goto sizeok2;
	return;
sizeok2:
	ah = data.byte(kObjecttype);
	al = data.byte(kItemframe);
	push(ax);
	findopenpos();
	ax = es.word(bx);
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = ah;
	_cmp(ah, 4);
	if (!flags.z())
		goto makeswapex;
	geteitherad();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
	goto actuallyswap;
makeswapex:
	transfertoex();
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = 4;
	geteitherad();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
actuallyswap:
	bl = data.byte(kItemframe);
	bh = data.byte(kObjecttype);
	ax = pop();
	data.byte(kObjecttype) = ah;
	data.byte(kItemframe) = al;
	push(bx);
	findopenpos();
	geteitherad();
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
	fillopen();
	fillryan();
	undertextline();
	readmouse();
	useopened();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::intoinv() {
	STACK_CHECK;
	_cmp(data.byte(kPickup), 0);
	if (!flags.z())
		goto notout;
	outofinv();
	return;
notout:
	findinvpos();
	ax = es.word(bx);
	_cmp(al, 255);
	if (flags.z())
		goto canplace1;
	swapwithinv();
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
	commandwithob();
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
	delpointer();
	al = data.byte(kItemframe);
	getexad();
	es.byte(bx+2) = 4;
	es.byte(bx+3) = 255;
	al = data.byte(kLastinvpos);
	es.byte(bx+4) = al;
	data.byte(kPickup) = 0;
	fillryan();
	readmouse();
	showpointer();
	outofinv();
	worktoscreen();
	delpointer();
}

void DreamGenContext::deletetaken() {
	STACK_CHECK;
	es = data.word(kFreedat);
	ah = data.byte(kReallocation);
	ds = data.word(kExtras);
	si = (0+2080+30000);
	cx = (114);
takenloop:
	al = ds.byte(si+11);
	_cmp(al, ah);
	if (!flags.z())
		goto notinhere;
	bl = ds.byte(si+1);
	bh = 0;
	_add(bx, bx);
	_add(bx, bx);
	_add(bx, bx);
	_add(bx, bx);
	es.byte(bx+2) = 254;
notinhere:
	_add(si, 16);
	if (--cx)
		goto takenloop;
}

void DreamGenContext::outofinv() {
	STACK_CHECK;
	findinvpos();
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
	reexfrominv();
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
	commandwithob();
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
	delpointer();
	data.byte(kPickup) = 1;
	findinvpos();
	ax = es.word(bx);
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = ah;
	getexad();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
	fillryan();
	readmouse();
	showpointer();
	intoinv();
	worktoscreen();
	delpointer();
}

void DreamGenContext::getfreead() {
	STACK_CHECK;
	ah = 0;
	cl = 4;
	_shl(ax, cl);
	bx = ax;
	es = data.word(kFreedat);
}

void DreamGenContext::getexad() {
	STACK_CHECK;
	ah = 0;
	bx = 16;
	_mul(bx);
	bx = ax;
	es = data.word(kExtras);
	_add(bx, (0+2080+30000));
}

void DreamGenContext::geteitherad() {
	STACK_CHECK;
	_cmp(data.byte(kObjecttype), 4);
	if (flags.z())
		goto isinexlist;
	al = data.byte(kItemframe);
	getfreead();
	return;
isinexlist:
	al = data.byte(kItemframe);
	getexad();
}

void DreamGenContext::getanyad() {
	STACK_CHECK;
	_cmp(data.byte(kObjecttype), 4);
	if (flags.z())
		goto isex;
	_cmp(data.byte(kObjecttype), 2);
	if (flags.z())
		goto isfree;
	al = data.byte(kCommand);
	getsetad();
	ax = es.word(bx+4);
	return;
isfree:
	al = data.byte(kCommand);
	getfreead();
	ax = es.word(bx+7);
	return;
isex:
	al = data.byte(kCommand);
	getexad();
	ax = es.word(bx+7);
}

void DreamGenContext::getanyaddir() {
	STACK_CHECK;
	_cmp(ah, 4);
	if (flags.z())
		goto isex3;
	_cmp(ah, 2);
	if (flags.z())
		goto isfree3;
	getsetad();
	return;
isfree3:
	getfreead();
	return;
isex3:
	getexad();
}

void DreamGenContext::getopenedsize() {
	STACK_CHECK;
	_cmp(data.byte(kOpenedtype), 4);
	if (flags.z())
		goto isex2;
	_cmp(data.byte(kOpenedtype), 2);
	if (flags.z())
		goto isfree2;
	al = data.byte(kOpenedob);
	getsetad();
	ax = es.word(bx+3);
	return;
isfree2:
	al = data.byte(kOpenedob);
	getfreead();
	ax = es.word(bx+7);
	return;
isex2:
	al = data.byte(kOpenedob);
	getexad();
	ax = es.word(bx+7);
}

void DreamGenContext::getsetad() {
	STACK_CHECK;
	ah = 0;
	bx = 64;
	_mul(bx);
	bx = ax;
	es = data.word(kSetdat);
}

void DreamGenContext::findinvpos() {
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

void DreamGenContext::findopenpos() {
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

void DreamGenContext::dropobject() {
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
	commandwithob();
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
	geteitherad();
	isitworn();
	if (!flags.z())
		goto nowornerror;
	wornerror();
	return;
nowornerror:
	_cmp(data.byte(kReallocation), 47);
	if (flags.z())
		goto nodrop2;
	cl = data.byte(kRyanx);
	_add(cl, 12);
	ch = data.byte(kRyany);
	_add(ch, 12);
	checkone();
	_cmp(cl, 2);
	if (flags.c())
		goto nodroperror;
nodrop2:
	droperror();
	return;
nodroperror:
	_cmp(data.byte(kMapxsize), 64);
	if (!flags.z())
		goto notinlift;
	_cmp(data.byte(kMapysize), 64);
	if (!flags.z())
		goto notinlift;
	droperror();
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
		{ cantdrop(); return; };
	al = data.byte(kItemframe);
	ah = 4;
	cl = 'S';
	ch = 'H';
	dl = 'L';
	dh = 'D';
	compare();
	if (flags.z())
		{ cantdrop(); return; };
	data.byte(kObjecttype) = 4;
	al = data.byte(kItemframe);
	getexad();
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

void DreamGenContext::droperror() {
	STACK_CHECK;
	data.byte(kCommandtype) = 255;
	delpointer();
	di = 76;
	bx = 21;
	al = 56;
	dl = 240;
	printmessage();
	worktoscreenm();
	cx = 50;
	hangonp();
	showpanel();
	showman();
	examicon();
	data.byte(kCommandtype) = 255;
	worktoscreenm();
}

void DreamGenContext::cantdrop() {
	STACK_CHECK;
	data.byte(kCommandtype) = 255;
	delpointer();
	di = 76;
	bx = 21;
	al = 24;
	dl = 240;
	printmessage();
	worktoscreenm();
	cx = 50;
	hangonp();
	showpanel();
	showman();
	examicon();
	data.byte(kCommandtype) = 255;
	worktoscreenm();
}

void DreamGenContext::wornerror() {
	STACK_CHECK;
	data.byte(kCommandtype) = 255;
	delpointer();
	di = 76;
	bx = 21;
	al = 57;
	dl = 240;
	printmessage();
	worktoscreenm();
	cx = 50;
	hangonp();
	showpanel();
	showman();
	examicon();
	data.byte(kCommandtype) = 255;
	worktoscreenm();
}

void DreamGenContext::removeobfrominv() {
	STACK_CHECK;
	_cmp(data.byte(kCommand), 100);
	if (flags.z())
		return /* (obnotexist) */;
	getanyad();
	di = bx;
	cl = data.byte(kCommand);
	ch = 0;
	deleteexobject();
}

void DreamGenContext::selectopenob() {
	STACK_CHECK;
	al = data.byte(kCommand);
	getanyad();
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
	commandwithob();
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
	createpanel();
	showpanel();
	showman();
	examicon();
	showexit();
	openinv();
	openob();
	undertextline();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::useopened() {
	STACK_CHECK;
	_cmp(data.byte(kOpenedob), 255);
	if (flags.z())
		return /* (cannotuseopen) */;
	_cmp(data.byte(kPickup), 0);
	if (!flags.z())
		goto notout2;
	outofopen();
	return;
notout2:
	findopenpos();
	ax = es.word(bx);
	_cmp(al, 255);
	if (flags.z())
		goto canplace3;
	swapwithopen();
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
	commandwithob();
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
	geteitherad();
	isitworn();
	if (!flags.z())
		goto notworntoopen;
	wornerror();
	return;
notworntoopen:
	delpointer();
	al = data.byte(kItemframe);
	_cmp(al, data.byte(kOpenedob));
	if (!flags.z())
		goto isntsame;
	al = data.byte(kObjecttype);
	_cmp(al, data.byte(kOpenedtype));
	if (!flags.z())
		goto isntsame;
	errormessage1();
	return;
isntsame:
	checkobjectsize();
	_cmp(al, 0);
	if (flags.z())
		goto sizeok1;
	return;
sizeok1:
	data.byte(kPickup) = 0;
	al = data.byte(kItemframe);
	geteitherad();
	al = data.byte(kOpenedtype);
	es.byte(bx+2) = al;
	al = data.byte(kOpenedob);
	es.byte(bx+3) = al;
	al = data.byte(kLastinvpos);
	es.byte(bx+4) = al;
	al = data.byte(kReallocation);
	es.byte(bx+5) = al;
	fillopen();
	undertextline();
	readmouse();
	useopened();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::errormessage1() {
	STACK_CHECK;
	delpointer();
	di = 76;
	bx = 21;
	al = 58;
	dl = 240;
	printmessage();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
	cx = 50;
	hangonp();
	showpanel();
	showman();
	examicon();
	readmouse();
	useopened();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::errormessage2() {
	STACK_CHECK;
	data.byte(kCommandtype) = 255;
	delpointer();
	di = 76;
	bx = 21;
	al = 59;
	dl = 240;
	printmessage();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
	cx = 50;
	hangonp();
	showpanel();
	showman();
	examicon();
	readmouse();
	useopened();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::errormessage3() {
	STACK_CHECK;
	delpointer();
	di = 76;
	bx = 21;
	al = 60;
	dl = 240;
	printmessage();
	worktoscreenm();
	cx = 50;
	hangonp();
	showpanel();
	showman();
	examicon();
	readmouse();
	useopened();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::checkobjectsize() {
	STACK_CHECK;
	getopenedsize();
	push(ax);
	al = data.byte(kItemframe);
	geteitherad();
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
	errormessage3();
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
	errormessage2();
	goto sizewrong;
bothspecial:
	_sub(cl, 100);
	_cmp(al, cl);
	if (flags.z())
		goto sizeok;
	errormessage3();
sizewrong:
	al = 1;
	return;
sizeok:
	al = 0;
}

void DreamGenContext::outofopen() {
	STACK_CHECK;
	_cmp(data.byte(kOpenedob), 255);
	if (flags.z())
		goto cantuseopen;
	findopenpos();
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
	commandwithob();
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
	reexfromopen();
	return;
dogrb:
	delpointer();
	data.byte(kPickup) = 1;
	findopenpos();
	ax = es.word(bx);
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = ah;
	_cmp(ah, 4);
	if (!flags.z())
		goto makeintoex;
	geteitherad();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
	goto actuallyout;
makeintoex:
	transfertoex();
	data.byte(kItemframe) = al;
	data.byte(kObjecttype) = 4;
	geteitherad();
	es.byte(bx+2) = 20;
	es.byte(bx+3) = 255;
actuallyout:
	fillopen();
	undertextline();
	readmouse();
	useopened();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::transfertoex() {
	STACK_CHECK;
	emergencypurge();
	getexpos();
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
	transfermap();
	transferinv();
	transfertext();
	al = data.byte(kItemframe);
	ah = 0;
	bx = 16;
	_mul(bx);
	ds = data.word(kFreedat);
	si = ax;
	ds.byte(si+2) = 254;
	pickupconts();
	ax = pop();
}

void DreamGenContext::pickupconts() {
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
	transfercontoex();
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

void DreamGenContext::transfercontoex() {
	STACK_CHECK;
	push(es);
	push(bx);
	push(dx);
	push(es);
	push(bx);
	getexpos();
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
	transfermap();
	transferinv();
	transfertext();
	si = pop();
	ds = pop();
	ds.byte(si+2) = 255;
}

void DreamGenContext::transfertext() {
	STACK_CHECK;
	es = data.word(kExtras);
	al = data.byte(kExpos);
	ah = 0;
	_add(ax, ax);
	bx = (0+2080+30000+(16*114));
	_add(bx, ax);
	di = data.word(kExtextpos);
	es.word(bx) = di;
	_add(di, (0+2080+30000+(16*114)+((114+2)*2)));
	al = data.byte(kItemtotran);
	ah = 0;
	_add(ax, ax);
	ds = data.word(kFreedesc);
	bx = (0);
	_add(bx, ax);
	si = (0+(82*2));
	ax = ds.word(bx);
	_add(si, ax);
moretext:
	_lodsb();
	_stosb();
	_inc(data.word(kExtextpos));
	_cmp(al, 0);
	if (!flags.z())
		goto moretext;
}

void DreamGenContext::getexpos() {
	STACK_CHECK;
	es = data.word(kExtras);
	al = 0;
	di = (0+2080+30000);
tryanotherex:
	_cmp(es.byte(di+2), 255);
	if (flags.z())
		goto foundnewex;
	_add(di, 16);
	_inc(al);
	_cmp(al, (114));
	if (!flags.z())
		goto tryanotherex;
foundnewex:
	data.byte(kExpos) = al;
}

void DreamGenContext::purgealocation() {
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
	deleteexobject();
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

void DreamGenContext::emergencypurge() {
	STACK_CHECK;
checkpurgeagain:
	ax = data.word(kExframepos);
	_add(ax, 4000);
	_cmp(ax, (30000));
	if (flags.c())
		goto notnearframeend;
	purgeanitem();
	goto checkpurgeagain;
notnearframeend:
	ax = data.word(kExtextpos);
	_add(ax, 400);
	_cmp(ax, (18000));
	if (flags.c())
		return /* (notneartextend) */;
	purgeanitem();
	goto checkpurgeagain;
}

void DreamGenContext::purgeanitem() {
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
	deleteexobject();
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
	deleteexobject();
	return;
cantpurge2:
	_add(di, 16);
	_inc(cx);
	_cmp(cx, (114));
	if (!flags.z())
		goto lookforpurge2;
}

void DreamGenContext::deleteexobject() {
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
	deleteexframe();
	ax = pop();
	cl = al;
	_add(al, al);
	_add(al, cl);
	_inc(al);
	deleteexframe();
	ax = pop();
	deleteextext();
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
	deleteexobject();
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

void DreamGenContext::deleteexframe() {
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

void DreamGenContext::deleteextext() {
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

void DreamGenContext::blockget() {
	STACK_CHECK;
	ah = al;
	al = 0;
	ds = data.word(kBackdrop);
	si = (0+192);
	_add(si, ax);
}

void DreamGenContext::drawfloor() {
	STACK_CHECK;
	push(es);
	push(bx);
	eraseoldobs();
	drawflags();
	calcmapad();
	doblocks();
	showallobs();
	showallfree();
	showallex();
	paneltomap();
	initrain();
	data.byte(kNewobs) = 0;
	bx = pop();
	es = pop();
}

void DreamGenContext::calcmapad() {
	STACK_CHECK;
	getdimension();
	push(cx);
	push(dx);
	al = 11;
	_sub(al, dl);
	_sub(al, cl);
	_sub(al, cl);
	ax.cbw();
	bx = 8;
	_mul(bx);
	_add(ax, data.word(kMapoffsetx));
	data.word(kMapadx) = ax;
	dx = pop();
	cx = pop();
	al = 10;
	_sub(al, dh);
	_sub(al, ch);
	_sub(al, ch);
	ax.cbw();
	bx = 8;
	_mul(bx);
	_add(ax, data.word(kMapoffsety));
	data.word(kMapady) = ax;
}

void DreamGenContext::getdimension() {
	STACK_CHECK;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32));
	ch = 0;
dimloop1:
	addalong();
	_cmp(al, 0);
	if (!flags.z())
		goto finishdim1;
	_inc(ch);
	goto dimloop1;
finishdim1:
	bx = (0+(228*13)+32+60+(32*32));
	cl = 0;
dimloop2:
	push(bx);
	addlength();
	bx = pop();
	_cmp(al, 0);
	if (!flags.z())
		goto finishdim2;
	_inc(cl);
	_add(bx, 3);
	goto dimloop2;
finishdim2:
	bx = (0+(228*13)+32+60+(32*32))+(11*3*9);
	dh = 10;
dimloop3:
	push(bx);
	addalong();
	bx = pop();
	_cmp(al, 0);
	if (!flags.z())
		goto finishdim3;
	_dec(dh);
	_sub(bx, 11*3);
	goto dimloop3;
finishdim3:
	bx = (0+(228*13)+32+60+(32*32))+(3*10);
	dl = 11;
dimloop4:
	push(bx);
	addlength();
	bx = pop();
	_cmp(al, 0);
	if (!flags.z())
		goto finishdim4;
	_dec(dl);
	_sub(bx, 3);
	goto dimloop4;
finishdim4:
	al = cl;
	ah = 0;
	_shl(ax, 1);
	_shl(ax, 1);
	_shl(ax, 1);
	_shl(ax, 1);
	data.word(kMapxstart) = ax;
	al = ch;
	ah = 0;
	_shl(ax, 1);
	_shl(ax, 1);
	_shl(ax, 1);
	_shl(ax, 1);
	data.word(kMapystart) = ax;
	_sub(dl, cl);
	_sub(dh, ch);
	al = dl;
	ah = 0;
	_shl(ax, 1);
	_shl(ax, 1);
	_shl(ax, 1);
	_shl(ax, 1);
	data.byte(kMapxsize) = al;
	al = dh;
	ah = 0;
	_shl(ax, 1);
	_shl(ax, 1);
	_shl(ax, 1);
	_shl(ax, 1);
	data.byte(kMapysize) = al;
}

void DreamGenContext::addalong() {
	STACK_CHECK;
	ah = 11;
addloop:
	_cmp(es.byte(bx), 0);
	if (!flags.z())
		goto gotalong;
	_add(bx, 3);
	_dec(ah);
	if (!flags.z())
		goto addloop;
	al = 0;
	return;
gotalong:
	al = 1;
}

void DreamGenContext::addlength() {
	STACK_CHECK;
	ah = 10;
addloop2:
	_cmp(es.byte(bx), 0);
	if (!flags.z())
		goto gotlength;
	_add(bx, 3*11);
	_dec(ah);
	if (!flags.z())
		goto addloop2;
	al = 0;
	return;
gotlength:
	al = 1;
}

void DreamGenContext::drawflags() {
	STACK_CHECK;
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32));
	al = data.byte(kMapy);
	ah = 0;
	cx = (66);
	_mul(cx);
	bl = data.byte(kMapx);
	bh = 0;
	_add(ax, bx);
	si = (0);
	_add(si, ax);
	cx = 10;
_tmp28:
	push(cx);
	cx = 11;
_tmp28a:
	ds = data.word(kMapdata);
	_lodsb();
	ds = data.word(kBackdrop);
	push(si);
	push(ax);
	ah = 0;
	_add(ax, ax);
	si = (0);
	_add(si, ax);
	_movsw();
	ax = pop();
	_stosb();
	si = pop();
	if (--cx)
		goto _tmp28a;
	_add(si, (66)-11);
	cx = pop();
	if (--cx)
		goto _tmp28;
}

void DreamGenContext::showallobs() {
	STACK_CHECK;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32));
	data.word(kListpos) = bx;
	di = bx;
	cx = 128*5;
	al = 255;
	_stosb(cx, true);
	es = data.word(kSetframes);
	data.word(kFrsegment) = es;
	ax = (0);
	data.word(kDataad) = ax;
	ax = (0+2080);
	data.word(kFramesad) = ax;
	data.byte(kCurrentob) = 0;
	ds = data.word(kSetdat);
	si = 0;
	cx = 128;
showobsloop:
	push(cx);
	push(si);
	push(si);
	_add(si, 58);
	es = data.word(kSetdat);
	getmapad();
	si = pop();
	_cmp(ch, 0);
	if (flags.z())
		goto blankframe;
	al = es.byte(si+18);
	ah = 0;
	data.word(kCurrentframe) = ax;
	_cmp(al, 255);
	if (flags.z())
		goto blankframe;
	push(es);
	push(si);
	calcfrframe();
	finalframe();
	si = pop();
	es = pop();
	al = es.byte(si+18);
	es.byte(si+17) = al;
	_cmp(es.byte(si+8), 0);
	if (!flags.z())
		goto animating;
	_cmp(es.byte(si+5), 5);
	if (flags.z())
		goto animating;
	_cmp(es.byte(si+5), 6);
	if (flags.z())
		goto animating;
	ax = data.word(kCurrentframe);
	ah = 0;
	_add(di, data.word(kMapadx));
	_add(bx, data.word(kMapady));
	showframe();
	goto drawnsetob;
animating:
	makebackob();
drawnsetob:
	si = data.word(kListpos);
	es = data.word(kBuffers);
	al = data.byte(kSavex);
	ah = data.byte(kSavey);
	es.word(si) = ax;
	cx = ax;
	ax = data.word(kSavesize);
	_add(al, cl);
	_add(ah, ch);
	es.word(si+2) = ax;
	al = data.byte(kCurrentob);
	es.byte(si+4) = al;
	_add(si, 5);
	data.word(kListpos) = si;
blankframe:
	_inc(data.byte(kCurrentob));
	si = pop();
	cx = pop();
	_add(si, 64);
	_dec(cx);
	if (flags.z())
		return /* (finishedsetobs) */;
	goto showobsloop;
}

void DreamGenContext::makebackob() {
	STACK_CHECK;
	_cmp(data.byte(kNewobs), 0);
	if (flags.z())
		return /* (nomake) */;
	al = es.byte(si+5);
	ah = es.byte(si+8);
	push(si);
	push(ax);
	push(si);
	ax = data.word(kObjectx);
	bx = data.word(kObjecty);
	ah = bl;
	si = ax;
	cx = 49520;
	dx = data.word(kSetframes);
	di = (0);
	makesprite();
	ax = pop();
	es.word(bx+20) = ax;
	ax = pop();
	_cmp(al, 255);
	if (!flags.z())
		goto usedpriority;
	al = 0;
usedpriority:
	es.byte(bx+23) = al;
	es.byte(bx+30) = ah;
	es.byte(bx+16) = 0;
	es.byte(bx+18) = 0;
	es.byte(bx+19) = 0;
	si = pop();
}

void DreamGenContext::showallfree() {
	STACK_CHECK;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5));
	data.word(kListpos) = bx;
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5));
	cx = 80*5;
	al = 255;
	_stosb(cx, true);
	es = data.word(kFreeframes);
	data.word(kFrsegment) = es;
	ax = (0);
	data.word(kDataad) = ax;
	ax = (0+2080);
	data.word(kFramesad) = ax;
	al = 0;
	data.byte(kCurrentfree) = al;
	ds = data.word(kFreedat);
	si = 2;
	cx = 0;
loop127:
	push(cx);
	push(si);
	push(si);
	es = data.word(kFreedat);
	getmapad();
	si = pop();
	_cmp(ch, 0);
	if (flags.z())
		goto over138;
	al = data.byte(kCurrentfree);
	ah = 0;
	dx = ax;
	_add(ax, ax);
	_add(ax, dx);
	data.word(kCurrentframe) = ax;
	push(es);
	push(si);
	calcfrframe();
	es = data.word(kMapstore);
	ds = data.word(kFrsegment);
	finalframe();
	si = pop();
	es = pop();
	_cmp(cx, 0);
	if (flags.z())
		goto over138;
	ax = data.word(kCurrentframe);
	ah = 0;
	_add(di, data.word(kMapadx));
	_add(bx, data.word(kMapady));
	showframe();
	si = data.word(kListpos);
	es = data.word(kBuffers);
	al = data.byte(kSavex);
	ah = data.byte(kSavey);
	es.word(si) = ax;
	cx = ax;
	ax = data.word(kSavesize);
	_add(al, cl);
	_add(ah, ch);
	es.word(si+2) = ax;
	ax = pop();
	cx = pop();
	push(cx);
	push(ax);
	es.byte(si+4) = cl;
	_add(si, 5);
	data.word(kListpos) = si;
over138:
	_inc(data.byte(kCurrentfree));
	si = pop();
	cx = pop();
	_add(si, 16);
	_inc(cx);
	_cmp(cx, 80);
	if (flags.z())
		return /* (finfree) */;
	goto loop127;
}

void DreamGenContext::showallex() {
	STACK_CHECK;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5));
	data.word(kListpos) = bx;
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5));
	cx = 100*5;
	al = 255;
	_stosb(cx, true);
	es = data.word(kExtras);
	data.word(kFrsegment) = es;
	ax = (0);
	data.word(kDataad) = ax;
	ax = (0+2080);
	data.word(kFramesad) = ax;
	data.byte(kCurrentex) = 0;
	si = (0+2080+30000)+2;
	cx = 0;
exloop:
	push(cx);
	push(si);
	es = data.word(kExtras);
	push(si);
	ch = 0;
	_cmp(es.byte(si), 255);
	if (flags.z())
		goto notinroom;
	al = es.byte(si-2);
	_cmp(al, data.byte(kReallocation));
	if (!flags.z())
		goto notinroom;
	getmapad();
notinroom:
	si = pop();
	_cmp(ch, 0);
	if (flags.z())
		goto blankex;
	al = data.byte(kCurrentex);
	ah = 0;
	dx = ax;
	_add(ax, ax);
	_add(ax, dx);
	data.word(kCurrentframe) = ax;
	push(es);
	push(si);
	calcfrframe();
	es = data.word(kMapstore);
	ds = data.word(kFrsegment);
	finalframe();
	si = pop();
	es = pop();
	_cmp(cx, 0);
	if (flags.z())
		goto blankex;
	ax = data.word(kCurrentframe);
	ah = 0;
	_add(di, data.word(kMapadx));
	_add(bx, data.word(kMapady));
	showframe();
	si = data.word(kListpos);
	es = data.word(kBuffers);
	al = data.byte(kSavex);
	ah = data.byte(kSavey);
	es.word(si) = ax;
	cx = ax;
	ax = data.word(kSavesize);
	_add(al, cl);
	_add(ah, ch);
	es.word(si+2) = ax;
	ax = pop();
	cx = pop();
	push(cx);
	push(ax);
	es.byte(si+4) = cl;
	_add(si, 5);
	data.word(kListpos) = si;
blankex:
	_inc(data.byte(kCurrentex));
	si = pop();
	cx = pop();
	_add(si, 16);
	_inc(cx);
	_cmp(cx, 100);
	if (flags.z())
		return /* (finex) */;
	goto exloop;
}

void DreamGenContext::calcfrframe() {
	STACK_CHECK;
	dx = data.word(kFrsegment);
	ax = data.word(kFramesad);
	push(ax);
	cx = data.word(kDataad);
	ax = data.word(kCurrentframe);
	ds = dx;
	bx = 6;
	_mul(bx);
	_add(ax, cx);
	bx = ax;
	cx = ds.word(bx);
	ax = ds.word(bx+2);
	dx = ds.word(bx+4);
	bx = pop();
	push(dx);
	_add(ax, bx);
	data.word(kSavesource) = ax;
	data.word(kSavesize) = cx;
	ax = pop();
	push(ax);
	ah = 0;
	data.word(kOffsetx) = ax;
	ax = pop();
	al = ah;
	ah = 0;
	data.word(kOffsety) = ax;
	return;
	ax = pop();
	cx = 0;
	data.word(kSavesize) = cx;
}

void DreamGenContext::finalframe() {
	STACK_CHECK;
	ax = data.word(kObjecty);
	_add(ax, data.word(kOffsety));
	bx = data.word(kObjectx);
	_add(bx, data.word(kOffsetx));
	data.byte(kSavex) = bl;
	data.byte(kSavey) = al;
	di = data.word(kObjectx);
	bx = data.word(kObjecty);
}

void DreamGenContext::adjustlen() {
	STACK_CHECK;
	ah = al;
	_add(al, ch);
	_cmp(al, 100);
	if (flags.c())
		return /* (over242) */;
	al = 224;
	_sub(al, ch);
	ch = al;
}

void DreamGenContext::getmapad() {
	STACK_CHECK;
	getxad();
	_cmp(ch, 0);
	if (flags.z())
		return /* (over146) */;
	data.word(kObjectx) = ax;
	getyad();
	_cmp(ch, 0);
	if (flags.z())
		return /* (over146) */;
	data.word(kObjecty) = ax;
	ch = 1;
}

void DreamGenContext::getxad() {
	STACK_CHECK;
	cl = es.byte(si);
	_inc(si);
	al = es.byte(si);
	_inc(si);
	ah = es.byte(si);
	_inc(si);
	_cmp(cl, 0);
	if (!flags.z())
		goto over148;
	_sub(al, data.byte(kMapx));
	if (flags.c())
		goto over148;
	_cmp(al, 11);
	if (!flags.c())
		goto over148;
	cl = 4;
	_shl(al, cl);
	_or(al, ah);
	ah = 0;
	ch = 1;
	return;
over148:
	ch = 0;
}

void DreamGenContext::getyad() {
	STACK_CHECK;
	al = es.byte(si);
	_inc(si);
	ah = es.byte(si);
	_inc(si);
	_sub(al, data.byte(kMapy));
	if (flags.c())
		goto over147;
	_cmp(al, 10);
	if (!flags.c())
		goto over147;
	cl = 4;
	_shl(al, cl);
	_or(al, ah);
	ah = 0;
	ch = 1;
	return;
over147:
	ch = 0;
}

void DreamGenContext::autolook() {
	STACK_CHECK;
	ax = data.word(kMousex);
	_cmp(ax, data.word(kOldx));
	if (!flags.z())
		goto diffmouse;
	ax = data.word(kMousey);
	_cmp(ax, data.word(kOldy));
	if (!flags.z())
		goto diffmouse;
	_dec(data.word(kLookcounter));
	_cmp(data.word(kLookcounter), 0);
	if (!flags.z())
		return /* (noautolook) */;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		return /* (noautolook) */;
	dolook();
	return;
diffmouse:
	data.word(kLookcounter) = 1000;
}

void DreamGenContext::look() {
	STACK_CHECK;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		{ blank(); return; };
	_cmp(data.byte(kPointermode), 2);
	if (flags.z())
		{ blank(); return; };
	_cmp(data.byte(kCommandtype), 241);
	if (flags.z())
		goto alreadylook;
	data.byte(kCommandtype) = 241;
	al = 25;
	commandonly();
alreadylook:
	_cmp(data.word(kMousebutton), 1);
	if (!flags.z())
		return /* (nolook) */;
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (nolook) */;
	dolook();
}

void DreamGenContext::dolook() {
	STACK_CHECK;
	createpanel();
	showicon();
	undertextline();
	worktoscreenm();
	data.byte(kCommandtype) = 255;
	dumptextline();
	bl = data.byte(kRoomnum);
	_and(bl, 31);
	bh = 0;
	_add(bx, bx);
	es = data.word(kRoomdesc);
	_add(bx, (0));
	si = es.word(bx);
	_add(si, (0+(38*2)));
	findnextcolon();
	di = 66;
	_cmp(data.byte(kReallocation), 50);
	if (flags.c())
		goto notdream3;
	di = 40;
notdream3:
	bx = 80;
	dl = 241;
	printslow();
	_cmp(al, 1);
	if (flags.z())
		goto afterlook;
	cx = 400;
	hangonp();
afterlook:
	data.byte(kPointermode) = 0;
	data.byte(kCommandtype) = 0;
	redrawmainscrn();
	worktoscreenm();
}

void DreamGenContext::redrawmainscrn() {
	STACK_CHECK;
	data.word(kTimecount) = 0;
	createpanel();
	data.byte(kNewobs) = 0;
	drawfloor();
	printsprites();
	reelsonscreen();
	showicon();
	getunderzoom();
	undertextline();
	readmouse();
	data.byte(kCommandtype) = 255;
}

void DreamGenContext::getback1() {
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
	commandonly();
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
	createpanel();
	showpanel();
	showman();
	showexit();
	undertextline();
	convicons();
	starttalk();
	data.byte(kCommandtype) = 255;
	readmouse();
	showpointer();
	worktoscreen();
waittalk:
	delpointer();
	readmouse();
	animpointer();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	data.byte(kGetback) = 0;
	bx = 2660;
	checkcoords();
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
	redrawmainscrn();
	worktoscreenm();
	_cmp(data.byte(kSpeechloaded), 1);
	if (!flags.z())
		return /* (nospeech) */;
	cancelch1();
	data.byte(kVolumedirection) = -1;
	data.byte(kVolumeto) = 0;
}

void DreamGenContext::convicons() {
	STACK_CHECK;
	al = data.byte(kCharacter);
	_and(al, 127);
	getpersframe();
	di = 234;
	bx = 2;
	data.word(kCurrentframe) = ax;
	findsource();
	ax = data.word(kCurrentframe);
	_sub(ax, data.word(kTakeoff));
	ah = 0;
	showframe();
}

void DreamGenContext::getpersframe() {
	STACK_CHECK;
	ah = 0;
	_add(ax, ax);
	bx = ax;
	es = data.word(kPeople);
	_add(bx, (0));
	ax = es.word(bx);
}

void DreamGenContext::starttalk() {
	STACK_CHECK;
	data.byte(kTalkmode) = 0;
	al = data.byte(kCharacter);
	_and(al, 127);
	getpersontext();
	data.word(kCharshift) = 91+91;
	di = 66;
	bx = 64;
	dl = 241;
	al = 0;
	ah = 79;
	printdirect();
	data.word(kCharshift) = 0;
	di = 66;
	bx = 80;
	dl = 241;
	al = 0;
	ah = 0;
	printdirect();
	data.byte(kSpeechloaded) = 0;
	al = data.byte(kCharacter);
	_and(al, 127);
	ah = 0;
	cx = 64;
	_mul(cx);
	cl = 'C';
	dl = 'R';
	dh = data.byte(kReallocation);
	loadspeech();
	_cmp(data.byte(kSpeechloaded), 1);
	if (!flags.z())
		return /* (nospeech1) */;
	data.byte(kVolumedirection) = 1;
	data.byte(kVolumeto) = 6;
	al = 50+12;
	playchannel1();
}

void DreamGenContext::getpersontext() {
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

void DreamGenContext::moretalk() {
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
	commandonly();
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
	dosometalk();
}

void DreamGenContext::dosometalk() {
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
	createpanel();
	showpanel();
	showman();
	showexit();
	convicons();
	si = pop();
	es = pop();
	di = 164;
	bx = 64;
	dl = 144;
	al = 0;
	ah = 0;
	printdirect();
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
	loadspeech();
	_cmp(data.byte(kSpeechloaded), 0);
	if (flags.z())
		goto noplay1;
	al = 62;
	playchannel1();
noplay1:
	data.byte(kPointermode) = 3;
	worktoscreenm();
	cx = 180;
	hangonpq();
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
	createpanel();
	showpanel();
	showman();
	showexit();
	convicons();
	si = pop();
	es = pop();
	di = 48;
	bx = 128;
	dl = 144;
	al = 0;
	ah = 0;
	printdirect();
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
	loadspeech();
	_cmp(data.byte(kSpeechloaded), 0);
	if (flags.z())
		goto noplay2;
	al = 62;
	playchannel1();
noplay2:
	data.byte(kPointermode) = 3;
	worktoscreenm();
	cx = 180;
	hangonpq();
	if (!flags.c())
		goto skiptalk2;
	return;
skiptalk2:
	_inc(data.byte(kTalkpos));
	goto dospeech;
endheartalk:
	data.byte(kPointermode) = 0;
}

void DreamGenContext::hangonpq() {
	STACK_CHECK;
	data.byte(kGetback) = 0;
	bx = 0;
hangloopq:
	push(cx);
	push(bx);
	delpointer();
	readmouse();
	animpointer();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	bx = 2692;
	checkcoords();
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
	delpointer();
	data.byte(kPointermode) = 0;
	flags._c = false;
 	return;
quitconv:
	delpointer();
	data.byte(kPointermode) = 0;
	cancelch1();
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
	commandonly();
alreadyreds:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (!flags.z())
		goto doredes;
	return;
doredes:
	delpointer();
	createpanel();
	showpanel();
	showman();
	showexit();
	convicons();
	starttalk();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::newplace() {
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
	selectlocation();
}

void DreamGenContext::selectlocation() {
	STACK_CHECK;
	data.byte(kInmaparea) = 0;
	clearbeforeload();
	data.byte(kGetback) = 0;
	data.byte(kPointerframe) = 22;
	readcitypic();
	showcity();
	getridoftemp();
	readdesticon();
	loadtraveltext();
	showpanel();
	showman();
	showarrows();
	showexit();
	locationpic();
	undertextline();
	data.byte(kCommandtype) = 255;
	readmouse();
	data.byte(kPointerframe) = 0;
	showpointer();
	worktoscreen();
	al = 9;
	ah = 255;
	playchannel0();
	data.byte(kNewlocation) = 255;
select:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		goto quittravel;
	delpointer();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	_cmp(data.byte(kGetback), 1);
	if (flags.z())
		goto quittravel;
	bx = 2714;
	checkcoords();
	_cmp(data.byte(kNewlocation), 255);
	if (flags.z())
		goto select;
	al = data.byte(kNewlocation);
	_cmp(al, data.byte(kLocation));
	if (flags.z())
		goto quittravel;
	getridoftemp();
	getridoftemp2();
	getridoftemp3();
	es = data.word(kTraveltext);
	deallocatemem();
	return;
quittravel:
	al = data.byte(kReallocation);
	data.byte(kNewlocation) = al;
	data.byte(kGetback) = 0;
	getridoftemp();
	getridoftemp2();
	getridoftemp3();
	es = data.word(kTraveltext);
	deallocatemem();
}

void DreamGenContext::showcity() {
	STACK_CHECK;
	clearwork();
	ds = data.word(kTempgraphics);
	di = 57;
	bx = 32;
	al = 0;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = 120+57;
	bx = 32;
	al = 1;
	ah = 0;
	showframe();
}

void DreamGenContext::lookatplace() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 224);
	if (flags.z())
		goto alreadyinfo;
	data.byte(kCommandtype) = 224;
	al = 27;
	commandonly();
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
	delpointer();
	deltextline();
	getundercentre();
	ds = data.word(kTempgraphics3);
	al = 0;
	ah = 0;
	di = 60;
	bx = 72;
	showframe();
	al = 4;
	ah = 0;
	di = 60;
	bx = 72+55;
	showframe();
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto _tmp1;
	al = 4;
	ah = 0;
	di = 60;
	bx = 72+55+21;
	showframe();
_tmp1:
	bx = pop();
	bh = 0;
	_add(bx, bx);
	es = data.word(kTraveltext);
	si = es.word(bx);
	_add(si, (66*2));
	findnextcolon();
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
	printdirect();
	worktoscreenm();
	cx = 500;
	hangonp();
	data.byte(kPointermode) = 0;
	data.byte(kPointerframe) = 0;
	putundercentre();
	worktoscreenm();
}

void DreamGenContext::getundercentre() {
	STACK_CHECK;
	di = 58;
	bx = 72;
	ds = data.word(kMapstore);
	si = 0;
	cl = 254;
	ch = 110;
	multiget();
}

void DreamGenContext::putundercentre() {
	STACK_CHECK;
	di = 58;
	bx = 72;
	ds = data.word(kMapstore);
	si = 0;
	cl = 254;
	ch = 110;
	multiput();
}

void DreamGenContext::locationpic() {
	STACK_CHECK;
	getdestinfo();
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
	showframe();
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
	showframe();
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
	printdirect();
}

void DreamGenContext::getdestinfo() {
	STACK_CHECK;
	al = data.byte(kDestpos);
	ah = 0;
	push(ax);
	dx = data;
	es = dx;
	si = 8011;
	_add(si, ax);
	cl = es.byte(si);
	ax = pop();
	push(cx);
	dx = data;
	es = dx;
	si = 8027;
	_add(si, ax);
	ax = pop();
}

void DreamGenContext::showarrows() {
	STACK_CHECK;
	di = 116-12;
	bx = 16;
	ds = data.word(kTempgraphics);
	al = 0;
	ah = 0;
	showframe();
	di = 226+12;
	bx = 16;
	ds = data.word(kTempgraphics);
	al = 1;
	ah = 0;
	showframe();
	di = 280;
	bx = 14;
	ds = data.word(kTempgraphics);
	al = 2;
	ah = 0;
	showframe();
}

void DreamGenContext::nextdest() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 218);
	if (flags.z())
		goto alreadydu;
	data.byte(kCommandtype) = 218;
	al = 28;
	commandonly();
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
	getdestinfo();
	_cmp(al, 0);
	if (flags.z())
		goto searchdestup;
	data.byte(kNewtextline) = 1;
	deltextline();
	delpointer();
	showpanel();
	showman();
	showarrows();
	locationpic();
	undertextline();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::lastdest() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 219);
	if (flags.z())
		goto alreadydd;
	data.byte(kCommandtype) = 219;
	al = 29;
	commandonly();
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
	getdestinfo();
	_cmp(al, 0);
	if (flags.z())
		goto searchdestdown;
	data.byte(kNewtextline) = 1;
	deltextline();
	delpointer();
	showpanel();
	showman();
	showarrows();
	locationpic();
	undertextline();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::destselect() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 222);
	if (flags.z())
		goto alreadytrav;
	data.byte(kCommandtype) = 222;
	al = 30;
	commandonly();
alreadytrav:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (flags.z())
		return /* (notrav) */;
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notrav) */;
	getdestinfo();
	al = data.byte(kDestpos);
	data.byte(kNewlocation) = al;
}

void DreamGenContext::getlocation() {
	STACK_CHECK;
	ah = 0;
	bx = ax;
	dx = data;
	es = dx;
	_add(bx, 8011);
	al = es.byte(bx);
}

void DreamGenContext::setlocation() {
	STACK_CHECK;
	ah = 0;
	bx = ax;
	dx = data;
	es = dx;
	_add(bx, 8011);
	es.byte(bx) = 1;
}

void DreamGenContext::resetlocation() {
	STACK_CHECK;
	push(ax);
	_cmp(al, 5);
	if (!flags.z())
		goto notdelhotel;
	purgealocation();
	al = 21;
	purgealocation();
	al = 22;
	purgealocation();
	al = 27;
	purgealocation();
	goto clearedlocations;
notdelhotel:
	_cmp(al, 8);
	if (!flags.z())
		goto notdeltvstud;
	purgealocation();
	al = 28;
	purgealocation();
	goto clearedlocations;
notdeltvstud:
	_cmp(al, 6);
	if (!flags.z())
		goto notdelsarters;
	purgealocation();
	al = 20;
	purgealocation();
	al = 25;
	purgealocation();
	goto clearedlocations;
notdelsarters:
	_cmp(al, 13);
	if (!flags.z())
		goto notdelboathouse;
	purgealocation();
	al = 29;
	purgealocation();
	goto clearedlocations;
notdelboathouse:
clearedlocations:
	ax = pop();
	ah = 0;
	bx = ax;
	dx = data;
	es = dx;
	_add(bx, 8011);
	es.byte(bx) = 0;
}

void DreamGenContext::readdesticon() {
	STACK_CHECK;
	dx = 2013;
	loadintotemp();
	dx = 2026;
	loadintotemp2();
	dx = 1961;
	loadintotemp3();
}

void DreamGenContext::readcitypic() {
	STACK_CHECK;
	dx = 2000;
	loadintotemp();
}

void DreamGenContext::usemon() {
	STACK_CHECK;
	data.byte(kLasttrigger) = 0;
	es = cs;
	di = 2970+1;
	cx = 12;
	al = 32;
	_stosb(cx, true);
	es = cs;
	di = 2942+1;
	cx = 12;
	al = 32;
	_stosb(cx, true);
	es = cs;
	di = 2836;
	es.byte(di) = 1;
	_add(di, 26);
	cx = 3;
keyloop:
	es.byte(di) = 0;
	_add(di, 26);
	if (--cx)
		goto keyloop;
	createpanel();
	showpanel();
	showicon();
	drawfloor();
	getridofall();
	dx = 1974;
	loadintotemp();
	loadpersonal();
	loadnews();
	loadcart();
	dx = 1870;
	loadtempcharset();
	printoutermon();
	initialmoncols();
	printlogo();
	worktoscreen();
	turnonpower();
	fadeupyellows();
	fadeupmonfirst();
	data.word(kMonadx) = 76;
	data.word(kMonady) = 141;
	al = 1;
	monmessage();
	cx = 120;
	hangoncurs();
	al = 2;
	monmessage();
	cx = 60;
	randomaccess();
	al = 3;
	monmessage();
	cx = 100;
	hangoncurs();
	printlogo();
	scrollmonitor();
	data.word(kBufferin) = 0;
	data.word(kBufferout) = 0;
moreinput:
	di = data.word(kMonadx);
	bx = data.word(kMonady);
	push(di);
	push(bx);
	input();
	bx = pop();
	di = pop();
	data.word(kMonadx) = di;
	data.word(kMonady) = bx;
	execcommand();
	_cmp(al, 0);
	if (flags.z())
		goto moreinput;
	getridoftemp();
	getridoftempcharset();
	es = data.word(kTextfile1);
	deallocatemem();
	es = data.word(kTextfile2);
	deallocatemem();
	es = data.word(kTextfile3);
	deallocatemem();
	data.byte(kGetback) = 1;
	al = 26;
	playchannel1();
	data.byte(kManisoffscreen) = 0;
	restoreall();
	redrawmainscrn();
	worktoscreenm();
}

void DreamGenContext::printoutermon() {
	STACK_CHECK;
	di = 40;
	bx = 32;
	ds = data.word(kTempgraphics);
	al = 1;
	ah = 0;
	showframe();
	di = 264;
	bx = 32;
	ds = data.word(kTempgraphics);
	al = 2;
	ah = 0;
	showframe();
	di = 40;
	bx = 12;
	ds = data.word(kTempgraphics);
	al = 3;
	ah = 0;
	showframe();
	di = 40;
	bx = 164;
	ds = data.word(kTempgraphics);
	al = 4;
	ah = 0;
	showframe();
}

void DreamGenContext::loadpersonal() {
	STACK_CHECK;
	al = data.byte(kLocation);
	dx = 2052;
	_cmp(al, 0);
	if (flags.z())
		goto foundpersonal;
	_cmp(al, 42);
	if (flags.z())
		goto foundpersonal;
	dx = 2065;
	_cmp(al, 2);
	if (flags.z())
		goto foundpersonal;
foundpersonal:
	openfile();
	readheader();
	bx = es.word(di);
	push(bx);
	cl = 4;
	_shr(bx, cl);
	allocatemem();
	data.word(kTextfile1) = ax;
	ds = ax;
	cx = pop();
	dx = 0;
	readfromfile();
	closefile();
}

void DreamGenContext::loadnews() {
	STACK_CHECK;
	al = data.byte(kNewsitem);
	dx = 2078;
	_cmp(al, 0);
	if (flags.z())
		goto foundnews;
	dx = 2091;
	_cmp(al, 1);
	if (flags.z())
		goto foundnews;
	dx = 2104;
	_cmp(al, 2);
	if (flags.z())
		goto foundnews;
	dx = 2117;
foundnews:
	openfile();
	readheader();
	bx = es.word(di);
	push(bx);
	cl = 4;
	_shr(bx, cl);
	allocatemem();
	data.word(kTextfile2) = ax;
	ds = ax;
	cx = pop();
	dx = 0;
	readfromfile();
	closefile();
}

void DreamGenContext::loadcart() {
	STACK_CHECK;
	lookininterface();
	dx = 2130;
	_cmp(al, 0);
	if (flags.z())
		goto gotcart;
	dx = 2143;
	_cmp(al, 1);
	if (flags.z())
		goto gotcart;
	dx = 2156;
	_cmp(al, 2);
	if (flags.z())
		goto gotcart;
	dx = 2169;
	_cmp(al, 3);
	if (flags.z())
		goto gotcart;
	dx = 2182;
gotcart:
	openfile();
	readheader();
	bx = es.word(di);
	push(bx);
	cl = 4;
	_shr(bx, cl);
	allocatemem();
	data.word(kTextfile3) = ax;
	ds = ax;
	cx = pop();
	dx = 0;
	readfromfile();
	closefile();
}

void DreamGenContext::lookininterface() {
	STACK_CHECK;
	al = 'I';
	ah = 'N';
	cl = 'T';
	ch = 'F';
	findsetobject();
	ah = 1;
	checkinside();
	_cmp(cl, (114));
	if (flags.z())
		goto emptyinterface;
	al = es.byte(bx+15);
	_inc(al);
	return;
emptyinterface:
	al = 0;
}

void DreamGenContext::turnonpower() {
	STACK_CHECK;
	cx = 3;
powerloop:
	push(cx);
	powerlighton();
	cx = 30;
	hangon();
	powerlightoff();
	cx = 30;
	hangon();
	cx = pop();
	if (--cx)
		goto powerloop;
	powerlighton();
}

void DreamGenContext::randomaccess() {
	STACK_CHECK;
accessloop:
	push(cx);
	vsync();
	vsync();
	randomnum1();
	_and(al, 15);
	_cmp(al, 10);
	if (flags.c())
		goto off;
	accesslighton();
	goto chosenaccess;
off:
	accesslightoff();
chosenaccess:
	cx = pop();
	if (--cx)
		goto accessloop;
	accesslightoff();
}

void DreamGenContext::powerlighton() {
	STACK_CHECK;
	di = 257+4;
	bx = 182;
	ds = data.word(kTempgraphics);
	al = 6;
	ah = 0;
	push(di);
	push(bx);
	showframe();
	bx = pop();
	di = pop();
	cl = 12;
	ch = 8;
	multidump();
}

void DreamGenContext::powerlightoff() {
	STACK_CHECK;
	di = 257+4;
	bx = 182;
	ds = data.word(kTempgraphics);
	al = 5;
	ah = 0;
	push(di);
	push(bx);
	showframe();
	bx = pop();
	di = pop();
	cl = 12;
	ch = 8;
	multidump();
}

void DreamGenContext::accesslighton() {
	STACK_CHECK;
	di = 74;
	bx = 182;
	ds = data.word(kTempgraphics);
	al = 8;
	ah = 0;
	push(di);
	push(bx);
	showframe();
	bx = pop();
	di = pop();
	cl = 12;
	ch = 8;
	multidump();
}

void DreamGenContext::accesslightoff() {
	STACK_CHECK;
	di = 74;
	bx = 182;
	ds = data.word(kTempgraphics);
	al = 7;
	ah = 0;
	push(di);
	push(bx);
	showframe();
	bx = pop();
	di = pop();
	cl = 12;
	ch = 8;
	multidump();
}

void DreamGenContext::locklighton() {
	STACK_CHECK;
	di = 56;
	bx = 182;
	ds = data.word(kTempgraphics);
	al = 10;
	ah = 0;
	push(di);
	push(bx);
	showframe();
	bx = pop();
	di = pop();
	cl = 12;
	ch = 8;
	multidump();
}

void DreamGenContext::locklightoff() {
	STACK_CHECK;
	di = 56;
	bx = 182;
	ds = data.word(kTempgraphics);
	al = 9;
	ah = 0;
	push(di);
	push(bx);
	showframe();
	bx = pop();
	di = pop();
	cl = 12;
	ch = 8;
	multidump();
}

void DreamGenContext::input() {
	STACK_CHECK;
	es = cs;
	di = 8045;
	cx = 64;
	al = 0;
	_stosb(cx, true);
	data.word(kCurpos) = 0;
	al = '>';
	di = data.word(kMonadx);
	bx = data.word(kMonady);
	ds = data.word(kTempcharset);
	ah = 0;
	printchar();
	di = data.word(kMonadx);
	bx = data.word(kMonady);
	cl = 6;
	ch = 8;
	multidump();
	_add(data.word(kMonadx), 6);
	ax = data.word(kMonadx);
	data.word(kCurslocx) = ax;
	ax = data.word(kMonady);
	data.word(kCurslocy) = ax;
waitkey:
	printcurs();
	vsync();
	delcurs();
	readkey();
	al = data.byte(kCurrentkey);
	_cmp(al, 0);
	if (flags.z())
		goto waitkey;
	_cmp(al, 13);
	if (flags.z())
		return /* (endofinput) */;
	_cmp(al, 8);
	if (!flags.z())
		goto notdel;
	_cmp(data.word(kCurpos), 0);
	if (flags.z())
		goto waitkey;
	delchar();
	goto waitkey;
notdel:
	_cmp(data.word(kCurpos), 28);
	if (flags.z())
		goto waitkey;
	_cmp(data.byte(kCurrentkey), 32);
	if (!flags.z())
		goto notleadingspace;
	_cmp(data.word(kCurpos), 0);
	if (flags.z())
		goto waitkey;
notleadingspace:
	makecaps();
	es = cs;
	si = data.word(kCurpos);
	_add(si, si);
	_add(si, 8045);
	es.byte(si) = al;
	_cmp(al, 'Z'+1);
	if (!flags.c())
		goto waitkey;
	push(ax);
	push(es);
	push(si);
	di = data.word(kMonadx);
	bx = data.word(kMonady);
	ds = data.word(kMapstore);
	ax = data.word(kCurpos);
	_xchg(al, ah);
	si = ax;
	cl = 8;
	ch = 8;
	multiget();
	si = pop();
	es = pop();
	ax = pop();
	push(es);
	push(si);
	di = data.word(kMonadx);
	bx = data.word(kMonady);
	ds = data.word(kTempcharset);
	ah = 0;
	printchar();
	si = pop();
	es = pop();
	es.byte(si+1) = cl;
	ch = 0;
	_add(data.word(kMonadx), cx);
	_inc(data.word(kCurpos));
	_add(data.word(kCurslocx), cx);
	goto waitkey;
}

void DreamGenContext::makecaps() {
	STACK_CHECK;
	_cmp(al, 'a');
	if (flags.c())
		return /* (notupperc) */;
	_sub(al, 32);
}

void DreamGenContext::delchar() {
	STACK_CHECK;
	_dec(data.word(kCurpos));
	si = data.word(kCurpos);
	_add(si, si);
	es = cs;
	_add(si, 8045);
	es.byte(si) = 0;
	al = es.byte(si+1);
	ah = 0;
	_sub(data.word(kMonadx), ax);
	_sub(data.word(kCurslocx), ax);
	di = data.word(kMonadx);
	bx = data.word(kMonady);
	ds = data.word(kMapstore);
	ax = data.word(kCurpos);
	_xchg(al, ah);
	si = ax;
	cl = 8;
	ch = 8;
	multiput();
	di = data.word(kMonadx);
	bx = data.word(kMonady);
	cl = al;
	ch = 8;
	multidump();
}

void DreamGenContext::execcommand() {
	STACK_CHECK;
	es = cs;
	bx = 2776;
	ds = cs;
	si = 8045;
	al = ds.byte(si);
	_cmp(al, 0);
	if (!flags.z())
		goto notblankinp;
	scrollmonitor();
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
	neterror();
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
	dircom();
	al = 0;
	return;
signoncom:
	signon();
	al = 0;
	return;
accesscom:
	read();
	al = 0;
	return;
keyscom:
	showkeys();
	al = 0;
	return;
testcom:
	al = 6;
	monmessage();
	al = 0;
	return;
quitcom:
	al = 1;
}

void DreamGenContext::neterror() {
	STACK_CHECK;
	al = 5;
	monmessage();
	scrollmonitor();
}

void DreamGenContext::dircom() {
	STACK_CHECK;
	cx = 30;
	randomaccess();
	parser();
	_cmp(es.byte(di+1), 0);
	if (flags.z())
		goto dirroot;
	dirfile();
	return;
dirroot:
	data.byte(kLogonum) = 0;
	ds = cs;
	si = 2956;
	_inc(si);
	es = cs;
	di = 2970;
	_inc(di);
	cx = 12;
	_movsb(cx, true);
	monitorlogo();
	scrollmonitor();
	al = 9;
	monmessage();
	es = data.word(kTextfile1);
	searchforfiles();
	es = data.word(kTextfile2);
	searchforfiles();
	es = data.word(kTextfile3);
	searchforfiles();
	scrollmonitor();
}

void DreamGenContext::searchforfiles() {
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
	monprint();
	goto directloop1;
}

void DreamGenContext::signon() {
	STACK_CHECK;
	parser();
	_inc(di);
	ds = cs;
	si = 2836;
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
	makecaps();
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
	monmessage();
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
	monmessage();
	return;
notyetassigned:
	push(es);
	push(bx);
	scrollmonitor();
	al = 15;
	monmessage();
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
	si = 8045;
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
	scrollmonitor();
	al = 16;
	monmessage();
	return;
passpassed:
	al = 14;
	monmessage();
	bx = pop();
	es = pop();
	push(es);
	push(bx);
	_add(bx, 14);
	monprint();
	scrollmonitor();
	bx = pop();
	es = pop();
	es.byte(bx) = 1;
}

void DreamGenContext::showkeys() {
	STACK_CHECK;
	cx = 10;
	randomaccess();
	scrollmonitor();
	al = 18;
	monmessage();
	es = cs;
	bx = 2836;
	cx = 4;
keysloop:
	push(cx);
	push(bx);
	_cmp(es.byte(bx), 0);
	if (flags.z())
		goto notheld;
	_add(bx, 14);
	monprint();
notheld:
	bx = pop();
	cx = pop();
	_add(bx, 26);
	if (--cx)
		goto keysloop;
	scrollmonitor();
}

void DreamGenContext::read() {
	STACK_CHECK;
	cx = 40;
	randomaccess();
	parser();
	_cmp(es.byte(di+1), 0);
	if (!flags.z())
		goto okcom;
	neterror();
	return;
okcom:
	es = cs;
	di = 2970;
	ax = data.word(kTextfile1);
	data.word(kMonsource) = ax;
	ds = ax;
	si = (66*2);
	searchforstring();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile2;
	ax = data.word(kTextfile2);
	data.word(kMonsource) = ax;
	ds = ax;
	si = (66*2);
	searchforstring();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile2;
	ax = data.word(kTextfile3);
	data.word(kMonsource) = ax;
	ds = ax;
	si = (66*2);
	searchforstring();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile2;
	al = 7;
	monmessage();
	return;
foundfile2:
	getkeyandlogo();
	_cmp(al, 0);
	if (flags.z())
		goto keyok1;
	return;
keyok1:
	es = cs;
	di = 2942;
	ds = data.word(kMonsource);
	searchforstring();
	_cmp(al, 0);
	if (flags.z())
		goto findtopictext;
	al = data.byte(kOldlogonum);
	data.byte(kLogonum) = al;
	al = 11;
	monmessage();
	return;
findtopictext:
	_inc(bx);
	push(es);
	push(bx);
	monitorlogo();
	scrollmonitor();
	bx = pop();
	es = pop();
moretopic:
	monprint();
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
	processtrigger();
	cx = 24;
	randomaccess();
	bx = pop();
	es = pop();
	goto moretopic;
endoftopic:
	scrollmonitor();
}

void DreamGenContext::dirfile() {
	STACK_CHECK;
	al = 34;
	es.byte(di) = al;
	push(es);
	push(di);
	ds = data.word(kTextfile1);
	si = (66*2);
	searchforstring();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile;
	di = pop();
	es = pop();
	push(es);
	push(di);
	ds = data.word(kTextfile2);
	si = (66*2);
	searchforstring();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile;
	di = pop();
	es = pop();
	push(es);
	push(di);
	ds = data.word(kTextfile3);
	si = (66*2);
	searchforstring();
	_cmp(al, 0);
	if (flags.z())
		goto foundfile;
	di = pop();
	es = pop();
	al = 7;
	monmessage();
	return;
foundfile:
	ax = pop();
	ax = pop();
	getkeyandlogo();
	_cmp(al, 0);
	if (flags.z())
		goto keyok2;
	return;
keyok2:
	push(es);
	push(bx);
	ds = cs;
	si = 2942+1;
	es = cs;
	di = 2970+1;
	cx = 12;
	_movsb(cx, true);
	monitorlogo();
	scrollmonitor();
	al = 10;
	monmessage();
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
	monprint();
	goto directloop2;
endofdir2:
	scrollmonitor();
}

void DreamGenContext::getkeyandlogo() {
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
	bx = 2836;
	_add(bx, ax);
	al = es.byte(bx);
	_cmp(al, 1);
	if (flags.z())
		goto keyok;
	push(bx);
	push(es);
	al = 12;
	monmessage();
	es = pop();
	bx = pop();
	_add(bx, 14);
	monprint();
	scrollmonitor();
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

void DreamGenContext::searchforstring() {
	STACK_CHECK;
	dl = es.byte(di);
	cx = di;
restartlook:
	di = cx;
	bx = si;
	dh = 0;
keeplooking:
	_lodsb();
	makecaps();
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
	di = 2942;
	cx = 13;
	al = 0;
	_stosb(cx, true);
	di = 2942;
	al = '=';
	_stosb();
	ds = cs;
	si = 8045;
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
	di = 2942;
}

void DreamGenContext::scrollmonitor() {
	STACK_CHECK;
	push(ax);
	push(bx);
	push(cx);
	push(dx);
	push(di);
	push(si);
	push(es);
	push(ds);
	printlogo();
	di = data.word(kMonadx);
	bx = data.word(kMonady);
	printundermon();
	ax = data.word(kMonady);
	worktoscreen();
	al = 25;
	playchannel1();
	ds = pop();
	es = pop();
	si = pop();
	di = pop();
	dx = pop();
	cx = pop();
	bx = pop();
	ax = pop();
}

void DreamGenContext::monitorlogo() {
	STACK_CHECK;
	al = data.byte(kLogonum);
	_cmp(al, data.byte(kOldlogonum));
	if (flags.z())
		goto notnewlogo;
	data.byte(kOldlogonum) = al;
	printlogo();
	printundermon();
	worktoscreen();
	printlogo();
	printlogo();
	al = 26;
	playchannel1();
	cx = 20;
	randomaccess();
	return;
notnewlogo:
	printlogo();
}

void DreamGenContext::printlogo() {
	STACK_CHECK;
	di = 56;
	bx = 32;
	ds = data.word(kTempgraphics);
	al = 0;
	ah = 0;
	showframe();
	showcurrentfile();
}

void DreamGenContext::showcurrentfile() {
	STACK_CHECK;
	di = 178;
	bx = 37;
	si = 2970+1;
curfileloop:
	al = cs.byte(si);
	_cmp(al, 0);
	if (flags.z())
		return /* (finishfile) */;
	_inc(si);
	push(si);
	modifychar();
	ds = data.word(kTempcharset);
	ah = 0;
	printchar();
	si = pop();
	goto curfileloop;
}

void DreamGenContext::monmessage() {
	STACK_CHECK;
	es = data.word(kTextfile1);
	bx = (66*2);
	cl = al;
	ch = 0;
monmessageloop:
	al = es.byte(bx);
	_inc(bx);
	_cmp(al, '+');
	if (!flags.z())
		goto monmessageloop;
	if (--cx)
		goto monmessageloop;
	monprint();
}

void DreamGenContext::processtrigger() {
	STACK_CHECK;
	_cmp(data.byte(kLasttrigger), '1');
	if (!flags.z())
		goto notfirsttrigger;
	al = 8;
	setlocation();
	al = 45;
	triggermessage();
	return;
notfirsttrigger:
	_cmp(data.byte(kLasttrigger), '2');
	if (!flags.z())
		goto notsecondtrigger;
	al = 9;
	setlocation();
	al = 55;
	triggermessage();
	return;
notsecondtrigger:
	_cmp(data.byte(kLasttrigger), '3');
	if (!flags.z())
		return /* (notthirdtrigger) */;
	al = 2;
	setlocation();
	al = 59;
	triggermessage();
}

void DreamGenContext::triggermessage() {
	STACK_CHECK;
	push(ax);
	di = 174;
	bx = 153;
	cl = 200;
	ch = 63;
	ds = data.word(kMapstore);
	si = 0;
	multiget();
	ax = pop();
	findpuztext();
	di = 174;
	bx = 156;
	dl = 141;
	ah = 16;
	printdirect();
	cx = 140;
	hangon();
	worktoscreen();
	cx = 340;
	hangon();
	di = 174;
	bx = 153;
	cl = 200;
	ch = 63;
	ds = data.word(kMapstore);
	si = 0;
	multiput();
	worktoscreen();
	data.byte(kLasttrigger) = 0;
}

void DreamGenContext::printcurs() {
	STACK_CHECK;
	push(si);
	push(di);
	push(ds);
	push(dx);
	push(bx);
	push(es);
	di = data.word(kCurslocx);
	bx = data.word(kCurslocy);
	cl = 6;
	ch = 8;
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto _tmp1;
	_sub(bx, 3);
	ch = 11;
_tmp1:
	ds = data.word(kBuffers);
	si = (0);
	push(di);
	push(bx);
	multiget();
	bx = pop();
	di = pop();
	push(bx);
	push(di);
	_inc(data.word(kMaintimer));
	ax = data.word(kMaintimer);
	_and(al, 16);
	if (!flags.z())
		goto flashcurs;
	al = '/';
	_sub(al, 32);
	ah = 0;
	ds = data.word(kTempcharset);
	showframe();
flashcurs:
	di = pop();
	bx = pop();
	_sub(di, 6);
	cl = 12;
	ch = 8;
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto _tmp2;
	ch = 11;
_tmp2:
	multidump();
	es = pop();
	bx = pop();
	dx = pop();
	ds = pop();
	di = pop();
	si = pop();
}

void DreamGenContext::delcurs() {
	STACK_CHECK;
	push(es);
	push(bx);
	push(di);
	push(ds);
	push(dx);
	push(si);
	di = data.word(kCurslocx);
	bx = data.word(kCurslocy);
	cl = 6;
	ch = 8;
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto _tmp1;
	_sub(bx, 3);
	ch = 11;
_tmp1:
	push(di);
	push(bx);
	push(cx);
	ds = data.word(kBuffers);
	si = (0);
	multiput();
	cx = pop();
	bx = pop();
	di = pop();
	multidump();
	si = pop();
	dx = pop();
	ds = pop();
	di = pop();
	bx = pop();
	es = pop();
}

void DreamGenContext::useobject() {
	STACK_CHECK;
	data.byte(kWithobject) = 255;
	_cmp(data.byte(kCommandtype), 229);
	if (flags.z())
		goto alreadyuse;
	data.byte(kCommandtype) = 229;
	bl = data.byte(kCommand);
	bh = data.byte(kObjecttype);
	al = 51;
	commandwithob();
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
	useroutine();
}

void DreamGenContext::useroutine() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 50);
	if (flags.c())
		goto nodream7;
	_cmp(data.byte(kPointerpower), 0);
	if (!flags.z())
		goto powerok;
	return;
powerok:
	data.byte(kPointerpower) = 0;
nodream7:
	getanyad();
	dx = data;
	ds = dx;
	si = 2984;
checkuselist:
	push(si);
	_lodsb();
	_sub(al, 'A');
	_cmp(al, es.byte(bx+12));
	if (!flags.z())
		goto failed;
	_lodsb();
	_sub(al, 'A');
	_cmp(al, es.byte(bx+13));
	if (!flags.z())
		goto failed;
	_lodsb();
	_sub(al, 'A');
	_cmp(al, es.byte(bx+14));
	if (!flags.z())
		goto failed;
	_lodsb();
	_sub(al, 'A');
	_cmp(al, es.byte(bx+15));
	if (!flags.z())
		goto failed;
	_lodsw();
	si = pop();
	__dispatch_call(ax);
	return;
failed:
	si = pop();
	_add(si, 6);
	_cmp(ds.byte(si), 140);
	if (!flags.z())
		goto checkuselist;
	delpointer();
	getobtextstart();
	findnextcolon();
	_cmp(al, 0);
	if (flags.z())
		goto cantuse2;
	findnextcolon();
	_cmp(al, 0);
	if (flags.z())
		goto cantuse2;
	al = es.byte(si);
	_cmp(al, 0);
	if (flags.z())
		goto cantuse2;
	usetext();
	cx = 400;
	hangonp();
	putbackobstuff();
	return;
cantuse2:
	createpanel();
	showpanel();
	showman();
	showexit();
	obicons();
	di = 33;
	bx = 100;
	al = 63;
	dl = 241;
	printmessage();
	worktoscreenm();
	cx = 50;
	hangonp();
	putbackobstuff();
	data.byte(kCommandtype) = 255;
}

void DreamGenContext::wheelsound() {
	STACK_CHECK;
	al = 17;
	playchannel1();
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::runtap() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto tapwith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
fillcupfromtap:
	al = data.byte(kWithobject);
	getexad();
	es.byte(bx+15) = 'F'-'A';
	al = 8;
	playchannel1();
	cx = 300;
	al = 57;
	showpuztext();
	putbackobstuff();
	return;
cupfromtapfull:
	cx = 300;
	al = 58;
	showpuztext();
	putbackobstuff();
}

void DreamGenContext::playguitar() {
	STACK_CHECK;
	al = 14;
	playchannel1();
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::hotelcontrol() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 21);
	if (!flags.z())
		goto notrightcont;
	_cmp(data.byte(kMapx), 33);
	if (!flags.z())
		goto notrightcont;
	showfirstuse();
	putbackobstuff();
	return;
notrightcont:
	showseconduse();
	putbackobstuff();
}

void DreamGenContext::hotelbell() {
	STACK_CHECK;
	al = 12;
	playchannel1();
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::opentomb() {
	STACK_CHECK;
	_inc(data.byte(kProgresspoints));
	showfirstuse();
	data.word(kWatchingtime) = 35*2;
	data.word(kReeltowatch) = 1;
	data.word(kEndwatchreel) = 33;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::usetrainer() {
	STACK_CHECK;
	getanyad();
	_cmp(es.byte(bx+2), 4);
	if (!flags.z())
		goto notheldtrainer;
	_inc(data.byte(kProgresspoints));
	makeworn();
	showseconduse();
	putbackobstuff();
	return;
notheldtrainer:
	nothelderror();
}

void DreamGenContext::nothelderror() {
	STACK_CHECK;
	createpanel();
	showpanel();
	showman();
	showexit();
	obicons();
	di = 64;
	bx = 100;
	al = 63;
	ah = 1;
	dl = 201;
	printmessage2();
	worktoscreenm();
	cx = 50;
	hangonp();
	putbackobstuff();
}

void DreamGenContext::usepipe() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto pipewith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
fillcup:
	cx = 300;
	al = 36;
	showpuztext();
	putbackobstuff();
	al = data.byte(kWithobject);
	getexad();
	es.byte(bx+15) = 'F'-'A';
	return;
alreadyfull:
	cx = 300;
	al = 35;
	showpuztext();
	putbackobstuff();
}

void DreamGenContext::usefullcart() {
	STACK_CHECK;
	_inc(data.byte(kProgresspoints));
	al = 2;
	ah = data.byte(kRoomnum);
	_add(ah, 6);
	turnanypathon();
	data.byte(kManspath) = 4;
	data.byte(kFacing) = 4;
	data.byte(kTurntoface) = 4;
	data.byte(kFinaldest) = 4;
	findxyfrompath();
	data.byte(kResetmanxy) = 1;
	showfirstuse();
	data.word(kWatchingtime) = 72*2;
	data.word(kReeltowatch) = 58;
	data.word(kEndwatchreel) = 142;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useplinth() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto plinthwith;
	withwhat();
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
	showfirstuse();
	putbackobstuff();
	return;
isrightkey:
	_inc(data.byte(kProgresspoints));
	showseconduse();
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
	showfirstuse();
	getanyad();
	es.byte(bx+2) = 255;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useladder() {
	STACK_CHECK;
	showfirstuse();
	_sub(data.byte(kMapx), 11);
	findroominloc();
	data.byte(kFacing) = 6;
	data.byte(kTurntoface) = 6;
	data.byte(kManspath) = 0;
	data.byte(kDestination) = 0;
	data.byte(kFinaldest) = 0;
	findxyfrompath();
	data.byte(kResetmanxy) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useladderb() {
	STACK_CHECK;
	showfirstuse();
	_add(data.byte(kMapx), 11);
	findroominloc();
	data.byte(kFacing) = 2;
	data.byte(kTurntoface) = 2;
	data.byte(kManspath) = 1;
	data.byte(kDestination) = 1;
	data.byte(kFinaldest) = 1;
	findxyfrompath();
	data.byte(kResetmanxy) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::slabdoora() {
	STACK_CHECK;
	showfirstuse();
	data.byte(kGetback) = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 13;
	_cmp(data.byte(kDreamnumber), 3);
	if (!flags.z())
		goto slabawrong;
	_inc(data.byte(kProgresspoints));
	data.word(kWatchingtime) = 60;
	data.word(kEndwatchreel) = 42;
	data.byte(kNewlocation) = 47;
	return;
slabawrong:
	data.word(kWatchingtime) = 40;
	data.word(kEndwatchreel) = 34;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
}

void DreamGenContext::slabdoorb() {
	STACK_CHECK;
	_cmp(data.byte(kDreamnumber), 1);
	if (!flags.z())
		goto slabbwrong;
	al = 'S';
	ah = 'H';
	cl = 'L';
	ch = 'D';
	isryanholding();
	if (!flags.z())
		goto gotcrystal;
	al = 44;
	cx = 200;
	showpuztext();
	putbackobstuff();
	return;
gotcrystal:
	showfirstuse();
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
	showfirstuse();
	data.byte(kGetback) = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 44;
	data.word(kWatchingtime) = 40;
	data.word(kEndwatchreel) = 63;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
}

void DreamGenContext::slabdoord() {
	STACK_CHECK;
	showfirstuse();
	data.byte(kGetback) = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 75;
	_cmp(data.byte(kDreamnumber), 0);
	if (!flags.z())
		goto slabcwrong;
	_inc(data.byte(kProgresspoints));
	data.word(kWatchingtime) = 60;
	data.word(kEndwatchreel) = 102;
	data.byte(kNewlocation) = 47;
	return;
slabcwrong:
	data.word(kWatchingtime) = 40;
	data.word(kEndwatchreel) = 94;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
}

void DreamGenContext::slabdoorc() {
	STACK_CHECK;
	showfirstuse();
	data.byte(kGetback) = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 108;
	_cmp(data.byte(kDreamnumber), 4);
	if (!flags.z())
		goto slabdwrong;
	_inc(data.byte(kProgresspoints));
	data.word(kWatchingtime) = 60;
	data.word(kEndwatchreel) = 135;
	data.byte(kNewlocation) = 47;
	return;
slabdwrong:
	data.word(kWatchingtime) = 40;
	data.word(kEndwatchreel) = 127;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
}

void DreamGenContext::slabdoore() {
	STACK_CHECK;
	showfirstuse();
	data.byte(kGetback) = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 141;
	_cmp(data.byte(kDreamnumber), 5);
	if (!flags.z())
		goto slabewrong;
	_inc(data.byte(kProgresspoints));
	data.word(kWatchingtime) = 60;
	data.word(kEndwatchreel) = 168;
	data.byte(kNewlocation) = 47;
	return;
slabewrong:
	data.word(kWatchingtime) = 40;
	data.word(kEndwatchreel) = 160;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
}

void DreamGenContext::slabdoorf() {
	STACK_CHECK;
	showfirstuse();
	data.byte(kGetback) = 1;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kReeltowatch) = 171;
	_cmp(data.byte(kDreamnumber), 2);
	if (!flags.z())
		goto slabfwrong;
	_inc(data.byte(kProgresspoints));
	data.word(kWatchingtime) = 60;
	data.word(kEndwatchreel) = 197;
	data.byte(kNewlocation) = 47;
	return;
slabfwrong:
	data.word(kWatchingtime) = 40;
	data.word(kEndwatchreel) = 189;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
}

void DreamGenContext::useslab() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto slabwith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
nextslab:
	al = data.byte(kWithobject);
	getexad();
	es.byte(bx+2) = 0;
	al = data.byte(kCommand);
	push(ax);
	removesetobject();
	ax = pop();
	_inc(al);
	push(ax);
	placesetobject();
	ax = pop();
	_cmp(al, 54);
	if (!flags.z())
		goto notlastslab;
	al = 0;
	turnpathon();
	data.word(kWatchingtime) = 22;
	data.word(kReeltowatch) = 35;
	data.word(kEndwatchreel) = 48;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
notlastslab:
	_inc(data.byte(kProgresspoints));
	showfirstuse();
	data.byte(kGetback) = 1;
}

void DreamGenContext::usecart() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto cartwith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
nextcart:
	al = data.byte(kWithobject);
	getexad();
	es.byte(bx+2) = 0;
	al = data.byte(kCommand);
	push(ax);
	removesetobject();
	ax = pop();
	_inc(al);
	placesetobject();
	_inc(data.byte(kProgresspoints));
	al = 17;
	playchannel1();
	showfirstuse();
	data.byte(kGetback) = 1;
}

void DreamGenContext::useclearbox() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto clearboxwith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
openbox:
	_inc(data.byte(kProgresspoints));
	showfirstuse();
	data.word(kWatchingtime) = 80;
	data.word(kReeltowatch) = 67;
	data.word(kEndwatchreel) = 105;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::usecoveredbox() {
	STACK_CHECK;
	_inc(data.byte(kProgresspoints));
	showfirstuse();
	data.word(kWatchingtime) = 50;
	data.word(kReeltowatch) = 41;
	data.word(kEndwatchreel) = 66;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::userailing() {
	STACK_CHECK;
	showfirstuse();
	data.word(kWatchingtime) = 80;
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 30;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	data.byte(kMandead) = 4;
}

void DreamGenContext::useopenbox() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto openboxwith;
	withwhat();
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
	showfirstuse();
	return;
destoryopenbox:
	_inc(data.byte(kProgresspoints));
	cx = 300;
	al = 37;
	showpuztext();
	al = data.byte(kWithobject);
	getexad();
	es.byte(bx+15) = 'E'-'A';
	data.word(kWatchingtime) = 140;
	data.word(kReeltowatch) = 105;
	data.word(kEndwatchreel) = 181;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	al = 4;
	turnpathon();
	data.byte(kGetback) = 1;
	return;
openboxwrong:
	cx = 300;
	al = 38;
	showpuztext();
	putbackobstuff();
}

void DreamGenContext::wearwatch() {
	STACK_CHECK;
	_cmp(data.byte(kWatchon), 1);
	if (flags.z())
		goto wearingwatch;
	showfirstuse();
	data.byte(kWatchon) = 1;
	data.byte(kGetback) = 1;
	getanyad();
	makeworn();
	return;
wearingwatch:
	showseconduse();
	putbackobstuff();
}

void DreamGenContext::wearshades() {
	STACK_CHECK;
	_cmp(data.byte(kShadeson), 1);
	if (flags.z())
		goto wearingshades;
	data.byte(kShadeson) = 1;
	showfirstuse();
	data.byte(kGetback) = 1;
	getanyad();
	makeworn();
	return;
wearingshades:
	showseconduse();
	putbackobstuff();
}

void DreamGenContext::sitdowninbar() {
	STACK_CHECK;
	_cmp(data.byte(kWatchmode), -1);
	if (!flags.z())
		goto satdown;
	showfirstuse();
	data.word(kWatchingtime) = 50;
	data.word(kReeltowatch) = 55;
	data.word(kEndwatchreel) = 71;
	data.word(kReeltohold) = 73;
	data.word(kEndofholdreel) = 83;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	return;
satdown:
	showseconduse();
	putbackobstuff();
}

void DreamGenContext::usechurchhole() {
	STACK_CHECK;
	showfirstuse();
	data.byte(kGetback) = 1;
	data.word(kWatchingtime) = 28;
	data.word(kReeltowatch) = 13;
	data.word(kEndwatchreel) = 26;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
}

void DreamGenContext::usehole() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto holewith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
righthand:
	showfirstuse();
	al = 86;
	removesetobject();
	al = data.byte(kWithobject);
	getexad();
	es.byte(bx+2) = 255;
	data.byte(kCanmovealtar) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::usealtar() {
	STACK_CHECK;
	al = 'C';
	ah = 'N';
	cl = 'D';
	ch = 'A';
	findexobject();
	_cmp(al, (114));
	if (flags.z())
		goto thingsonaltar;
	al = 'C';
	ah = 'N';
	cl = 'D';
	ch = 'B';
	findexobject();
	_cmp(al, (114));
	if (flags.z())
		goto thingsonaltar;
	_cmp(data.byte(kCanmovealtar), 1);
	if (flags.z())
		goto movealtar;
	cx = 300;
	al = 23;
	showpuztext();
	data.byte(kGetback) = 1;
	return;
movealtar:
	_inc(data.byte(kProgresspoints));
	showseconduse();
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
	setuptimeduse();
	data.byte(kGetback) = 1;
	return;
thingsonaltar:
	showfirstuse();
	data.byte(kGetback) = 1;
}

void DreamGenContext::opentvdoor() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto tvdoorwith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
keyontv:
	showfirstuse();
	data.byte(kLockstatus) = 0;
	data.byte(kGetback) = 1;
}

void DreamGenContext::usedryer() {
	STACK_CHECK;
	al = 12;
	playchannel1();
	showfirstuse();
	data.byte(kGetback) = 1;
}

void DreamGenContext::openlouis() {
	STACK_CHECK;
	al = 5;
	ah = 2;
	cl = 3;
	ch = 8;
	entercode();
	data.byte(kGetback) = 1;
}

void DreamGenContext::nextcolon() {
	STACK_CHECK;
lookcolon:
	al = es.byte(si);
	_inc(si);
	_cmp(al, ':');
	if (!flags.z())
		goto lookcolon;
}

void DreamGenContext::openyourneighbour() {
	STACK_CHECK;
	al = 255;
	ah = 255;
	cl = 255;
	ch = 255;
	entercode();
	data.byte(kGetback) = 1;
}

void DreamGenContext::usewindow() {
	STACK_CHECK;
	_cmp(data.byte(kManspath), 6);
	if (!flags.z())
		goto notonbalc;
	_inc(data.byte(kProgresspoints));
	showfirstuse();
	data.byte(kNewlocation) = 29;
	data.byte(kGetback) = 1;
	return;
notonbalc:
	showseconduse();
	putbackobstuff();
}

void DreamGenContext::usebalcony() {
	STACK_CHECK;
	showfirstuse();
	al = 6;
	turnpathon();
	al = 0;
	turnpathoff();
	al = 1;
	turnpathoff();
	al = 2;
	turnpathoff();
	al = 3;
	turnpathoff();
	al = 4;
	turnpathoff();
	al = 5;
	turnpathoff();
	_inc(data.byte(kProgresspoints));
	data.byte(kManspath) = 6;
	data.byte(kDestination) = 6;
	data.byte(kFinaldest) = 6;
	findxyfrompath();
	switchryanoff();
	data.byte(kResetmanxy) = 1;
	data.word(kWatchingtime) = 30*2;
	data.word(kReeltowatch) = 183;
	data.word(kEndwatchreel) = 212;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::openryan() {
	STACK_CHECK;
	al = 5;
	ah = 1;
	cl = 0;
	ch = 6;
	entercode();
	data.byte(kGetback) = 1;
}

void DreamGenContext::openpoolboss() {
	STACK_CHECK;
	al = 5;
	ah = 2;
	cl = 2;
	ch = 2;
	entercode();
	data.byte(kGetback) = 1;
}

void DreamGenContext::openeden() {
	STACK_CHECK;
	al = 2;
	ah = 8;
	cl = 6;
	ch = 5;
	entercode();
	data.byte(kGetback) = 1;
}

void DreamGenContext::opensarters() {
	STACK_CHECK;
	al = 7;
	ah = 8;
	cl = 3;
	ch = 3;
	entercode();
	data.byte(kGetback) = 1;
}

void DreamGenContext::isitright() {
	STACK_CHECK;
	bx = data;
	es = bx;
	bx = 8573;
	_cmp(es.byte(bx+0), al);
	if (!flags.z())
		return /* (notright) */;
	_cmp(es.byte(bx+1), ah);
	if (!flags.z())
		return /* (notright) */;
	_cmp(es.byte(bx+2), cl);
	if (!flags.z())
		return /* (notright) */;
	_cmp(es.byte(bx+3), ch);
}

void DreamGenContext::drawitall() {
	STACK_CHECK;
	createpanel();
	drawfloor();
	printsprites();
	showicon();
}

void DreamGenContext::openhoteldoor() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto hoteldoorwith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
keyonhotel1:
	al = 16;
	playchannel1();
	showfirstuse();
	data.byte(kLockstatus) = 0;
	data.byte(kGetback) = 1;
}

void DreamGenContext::openhoteldoor2() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto hoteldoorwith2;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
keyonhotel2:
	al = 16;
	playchannel1();
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::grafittidoor() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto grafwith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
dograf:
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::trapdoor() {
	STACK_CHECK;
	_inc(data.byte(kProgresspoints));
	showfirstuse();
	switchryanoff();
	data.word(kWatchingtime) = 20*2;
	data.word(kReeltowatch) = 181;
	data.word(kEndwatchreel) = 197;
	data.byte(kNewlocation) = 26;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::callhotellift() {
	STACK_CHECK;
	al = 12;
	playchannel1();
	showfirstuse();
	data.byte(kCounttoopen) = 8;
	data.byte(kGetback) = 1;
	data.byte(kDestination) = 5;
	data.byte(kFinaldest) = 5;
	autosetwalk();
	al = 4;
	turnpathon();
}

void DreamGenContext::calledenslift() {
	STACK_CHECK;
	showfirstuse();
	data.byte(kCounttoopen) = 8;
	data.byte(kGetback) = 1;
	al = 2;
	turnpathon();
}

void DreamGenContext::calledensdlift() {
	STACK_CHECK;
	_cmp(data.byte(kLiftflag), 1);
	if (flags.z())
		goto edensdhere;
	showfirstuse();
	data.byte(kCounttoopen) = 8;
	data.byte(kGetback) = 1;
	al = 2;
	turnpathon();
	return;
edensdhere:
	showseconduse();
	putbackobstuff();
}

void DreamGenContext::usepoolreader() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto poolwith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
openpool:
	_cmp(data.byte(kTalkedtoattendant), 1);
	if (flags.z())
		goto canopenpool;
	showseconduse();
	putbackobstuff();
	return;
canopenpool:
	al = 17;
	playchannel1();
	showfirstuse();
	data.byte(kCounttoopen) = 6;
	data.byte(kGetback) = 1;
}

void DreamGenContext::uselighter() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotlighterwith;
	withwhat();
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
	showfirstuse();
	putbackobstuff();
	return;
cigarette:
	cx = 300;
	al = 9;
	showpuztext();
	al = data.byte(kWithobject);
	getexad();
	es.byte(bx+2) = 255;
	data.byte(kGetback) = 1;
}

void DreamGenContext::showseconduse() {
	STACK_CHECK;
	getobtextstart();
	nextcolon();
	nextcolon();
	nextcolon();
	usetext();
	cx = 400;
	hangonp();
}

void DreamGenContext::usecardreader1() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotreader1with;
	withwhat();
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
	showpuztext();
	putbackobstuff();
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
	showpuztext();
	putbackobstuff();
	return;
getscash:
	al = 16;
	playchannel1();
	cx = 300;
	al = 18;
	showpuztext();
	_inc(data.byte(kProgresspoints));
	data.word(kCard1money) = 12432;
	data.byte(kGetback) = 1;
	return;
notyet:
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::usecardreader2() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotreader2with;
	withwhat();
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
	showpuztext();
	putbackobstuff();
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
	playchannel1();
	cx = 300;
	al = 19;
	showpuztext();
	al = 94;
	placesetobject();
	data.byte(kGunpassflag) = 1;
	_sub(data.word(kCard1money), 2000);
	_inc(data.byte(kProgresspoints));
	data.byte(kGetback) = 1;
	return;
nocash:
	cx = 300;
	al = 20;
	showpuztext();
	putbackobstuff();
	return;
alreadygotnew:
	cx = 300;
	al = 22;
	showpuztext();
	putbackobstuff();
	return;
notyetboss:
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::usecardreader3() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotreader3with;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
rightcard:
	_cmp(data.byte(kTalkedtorecep), 0);
	if (flags.z())
		goto notyetrecep;
	_cmp(data.byte(kCardpassflag), 0);
	if (!flags.z())
		goto alreadyusedit;
	al = 16;
	playchannel1();
	cx = 300;
	al = 25;
	showpuztext();
	_inc(data.byte(kProgresspoints));
	_sub(data.word(kCard1money), 8300);
	data.byte(kCardpassflag) = 1;
	data.byte(kGetback) = 1;
	return;
alreadyusedit:
	cx = 300;
	al = 26;
	showpuztext();
	putbackobstuff();
	return;
notyetrecep:
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::usecashcard() {
	STACK_CHECK;
	getridofreels();
	loadkeypad();
	createpanel();
	showpanel();
	showexit();
	showman();
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
	showframe();
	ax = data.word(kCard1money);
	moneypoke();
	getobtextstart();
	nextcolon();
	nextcolon();
	di = 36;
	bx = 98;
	dl = 241;
	al = 0;
	ah = 0;
	printdirect();
	di = 160;
	bx = 155;
	es = cs;
	si = 3474;
	data.word(kCharshift) = 91*2+75;
	al = 0;
	ah = 0;
	dl = 240;
	printdirect();
	di = 187;
	bx = 155;
	es = cs;
	si = 3479;
	data.word(kCharshift) = 91*2+85;
	al = 0;
	ah = 0;
	dl = 240;
	printdirect();
	data.word(kCharshift) = 0;
	worktoscreenm();
	cx = 400;
	hangonp();
	getridoftemp();
	restorereels();
	putbackobstuff();
}

void DreamGenContext::lookatcard() {
	STACK_CHECK;
	data.byte(kManisoffscreen) = 1;
	getridofreels();
	loadkeypad();
	createpanel2();
	di = 160;
	bx = 80;
	ds = data.word(kTempgraphics);
	al = 42;
	ah = 128;
	showframe();
	getobtextstart();
	findnextcolon();
	findnextcolon();
	findnextcolon();
	di = 36;
	bx = 124;
	dl = 241;
	al = 0;
	ah = 0;
	printdirect();
	push(es);
	push(si);
	worktoscreenm();
	cx = 280;
	hangonw();
	createpanel2();
	di = 160;
	bx = 80;
	ds = data.word(kTempgraphics);
	al = 42;
	ah = 128;
	showframe();
	si = pop();
	es = pop();
	di = 36;
	bx = 130;
	dl = 241;
	al = 0;
	ah = 0;
	printdirect();
	worktoscreenm();
	cx = 200;
	hangonw();
	data.byte(kManisoffscreen) = 0;
	getridoftemp();
	restorereels();
	putbackobstuff();
}

void DreamGenContext::moneypoke() {
	STACK_CHECK;
	bx = 3474;
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
	bx = 3479;
	_add(al, 48);
	cs.byte(bx) = al;
}

void DreamGenContext::usecontrol() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotcontrolwith;
	withwhat();
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
	showfirstuse();
	putbackobstuff();
	return;
rightkey:
	al = 16;
	playchannel1();
	_cmp(data.byte(kLocation), 21);
	if (flags.z())
		goto goingdown;
	cx = 300;
	al = 0;
	showpuztext();
	data.byte(kNewlocation) = 21;
	data.byte(kCounttoclose) = 8;
	data.byte(kCounttoopen) = 0;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
	return;
goingdown:
	cx = 300;
	al = 3;
	showpuztext();
	data.byte(kNewlocation) = 30;
	data.byte(kCounttoclose) = 8;
	data.byte(kCounttoopen) = 0;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
	return;
jimmycontrols:
	al = 50;
	placesetobject();
	al = 51;
	placesetobject();
	al = 26;
	placesetobject();
	al = 30;
	placesetobject();
	al = 16;
	removesetobject();
	al = 17;
	removesetobject();
	al = 14;
	playchannel1();
	cx = 300;
	al = 10;
	showpuztext();
	_inc(data.byte(kProgresspoints));
	data.byte(kGetback) = 1;
	return;
axeoncontrols:
	cx = 300;
	al = 16;
	showpuztext();
	_inc(data.byte(kProgresspoints));
	putbackobstuff();
}

void DreamGenContext::usehatch() {
	STACK_CHECK;
	showfirstuse();
	data.byte(kNewlocation) = 40;
	data.byte(kGetback) = 1;
}

void DreamGenContext::usewire() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotwirewith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
wireaxe:
	cx = 300;
	al = 16;
	showpuztext();
	putbackobstuff();
	return;
wireknife:
	al = 51;
	removesetobject();
	al = 52;
	placesetobject();
	cx = 300;
	al = 11;
	showpuztext();
	_inc(data.byte(kProgresspoints));
	data.byte(kGetback) = 1;
}

void DreamGenContext::usehandle() {
	STACK_CHECK;
	al = 'C';
	ah = 'U';
	cl = 'T';
	ch = 'W';
	findsetobject();
	al = es.byte(bx+58);
	_cmp(al, 255);
	if (!flags.z())
		goto havecutwire;
	cx = 300;
	al = 12;
	showpuztext();
	data.byte(kGetback) = 1;
	return;
havecutwire:
	cx = 300;
	al = 13;
	showpuztext();
	data.byte(kNewlocation) = 22;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useelevator1() {
	STACK_CHECK;
	showfirstuse();
	selectlocation();
	data.byte(kGetback) = 1;
}

void DreamGenContext::showfirstuse() {
	STACK_CHECK;
	getobtextstart();
	findnextcolon();
	findnextcolon();
	usetext();
	cx = 400;
	hangonp();
}

void DreamGenContext::useelevator3() {
	STACK_CHECK;
	showfirstuse();
	data.byte(kCounttoclose) = 20;
	data.byte(kNewlocation) = 34;
	data.word(kReeltowatch) = 46;
	data.word(kEndwatchreel) = 63;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useelevator4() {
	STACK_CHECK;
	showfirstuse();
	data.word(kReeltowatch) = 0;
	data.word(kEndwatchreel) = 11;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kCounttoclose) = 20;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
	data.byte(kNewlocation) = 24;
}

void DreamGenContext::useelevator2() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 23);
	if (flags.z())
		goto inpoolhall;
	showfirstuse();
	data.byte(kNewlocation) = 23;
	data.byte(kCounttoclose) = 20;
	data.byte(kCounttoopen) = 0;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
	return;
inpoolhall:
	showfirstuse();
	data.byte(kNewlocation) = 31;
	data.byte(kCounttoclose) = 20;
	data.byte(kCounttoopen) = 0;
	data.word(kWatchingtime) = 80;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useelevator5() {
	STACK_CHECK;
	al = 4;
	placesetobject();
	al = 0;
	removesetobject();
	data.byte(kNewlocation) = 20;
	data.word(kWatchingtime) = 80;
	data.byte(kLiftflag) = 1;
	data.byte(kCounttoclose) = 8;
	data.byte(kGetback) = 1;
}

void DreamGenContext::usekey() {
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
	showpuztext();
	putbackobstuff();
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
	showpuztext();
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
	showpuztext();
	data.byte(kNewlocation) = 30;
	al = 2;
	fadescreendown();
	showfirstuse();
	putbackobstuff();
	return;
wrongroom1:
	cx = 200;
	al = 2;
	showpuztext();
	putbackobstuff();
}

void DreamGenContext::usestereo() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 0);
	if (flags.z())
		goto stereook;
	cx = 400;
	al = 4;
	showpuztext();
	putbackobstuff();
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
	showpuztext();
	putbackobstuff();
	return;
stereook2:
	al = 'C';
	ah = 'D';
	cl = 'P';
	ch = 'L';
	findsetobject();
	ah = 1;
	checkinside();
	_cmp(cl, (114));
	if (!flags.z())
		goto cdinside;
	al = 6;
	cx = 400;
	showpuztext();
	putbackobstuff();
	getanyad();
	al = 255;
	es.byte(bx+10) = al;
	return;
cdinside:
	getanyad();
	al = es.byte(bx+10);
	_xor(al, 1);
	es.byte(bx+10) = al;
	_cmp(al, 255);
	if (flags.z())
		goto stereoon;
	al = 7;
	cx = 400;
	showpuztext();
	putbackobstuff();
	return;
stereoon:
	al = 8;
	cx = 400;
	showpuztext();
	putbackobstuff();
}

void DreamGenContext::usecooker() {
	STACK_CHECK;
	al = data.byte(kCommand);
	ah = data.byte(kObjecttype);
	checkinside();
	_cmp(cl, (114));
	if (!flags.z())
		goto foodinside;
	showfirstuse();
	putbackobstuff();
	return;
foodinside:
	showseconduse();
	putbackobstuff();
}

void DreamGenContext::useaxe() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 22);
	if (!flags.z())
		goto notinpool;
	_cmp(data.byte(kMapy), 10);
	if (flags.z())
		goto axeondoor;
	showseconduse();
	_inc(data.byte(kProgresspoints));
	data.byte(kLastweapon) = 2;
	data.byte(kGetback) = 1;
	removeobfrominv();
	return;
notinpool:
	showfirstuse();
	return;
/*continuing to unbounded code: axeondoor from useelvdoor:19-30*/
axeondoor:
	al = 15;
	cx = 300;
	showpuztext();
	_inc(data.byte(kProgresspoints));
	data.word(kWatchingtime) = 46*2;
	data.word(kReeltowatch) = 31;
	data.word(kEndwatchreel) = 77;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::useelvdoor() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gotdoorwith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
axeondoor:
	al = 15;
	cx = 300;
	showpuztext();
	_inc(data.byte(kProgresspoints));
	data.word(kWatchingtime) = 46*2;
	data.word(kReeltowatch) = 31;
	data.word(kEndwatchreel) = 77;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::withwhat() {
	STACK_CHECK;
	createpanel();
	showpanel();
	showman();
	showexit();
	al = data.byte(kCommand);
	ah = data.byte(kObjecttype);
	es = cs;
	di = 5847;
	copyname();
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
	si = 5847;
	dl = 220;
	al = 0;
	ah = 0;
	printdirect();
	di = data.word(kLastxpos);
	_add(di, 5);
	bx = 21;
	dl = 200;
	al = 63;
	ah = 3;
	printmessage2();
	fillryan();
	data.byte(kCommandtype) = 255;
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
	data.byte(kInvopen) = 2;
}

void DreamGenContext::selectob() {
	STACK_CHECK;
	findinvpos();
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
	commandwithob();
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
	delpointer();
	data.byte(kInvopen) = 0;
	useroutine();
}

void DreamGenContext::compare() {
	STACK_CHECK;
	_sub(dl, 'A');
	_sub(dh, 'A');
	_sub(cl, 'A');
	_sub(ch, 'A');
	push(cx);
	push(dx);
	getanyaddir();
	dx = pop();
	cx = pop();
	_cmp(es.word(bx+12), cx);
	if (!flags.z())
		return /* (comparefin) */;
	_cmp(es.word(bx+14), dx);
}

void DreamGenContext::findsetobject() {
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

void DreamGenContext::findexobject() {
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

void DreamGenContext::isryanholding() {
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

void DreamGenContext::checkinside() {
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

void DreamGenContext::usetext() {
	STACK_CHECK;
	push(es);
	push(si);
	createpanel();
	showpanel();
	showman();
	showexit();
	obicons();
	si = pop();
	es = pop();
	di = 36;
	bx = 104;
	dl = 241;
	al = 0;
	ah = 0;
	printdirect();
	worktoscreenm();
}

void DreamGenContext::putbackobstuff() {
	STACK_CHECK;
	createpanel();
	showpanel();
	showman();
	obicons();
	showexit();
	obpicture();
	describeob();
	undertextline();
	data.byte(kCommandtype) = 255;
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::showpuztext() {
	STACK_CHECK;
	push(cx);
	findpuztext();
	push(es);
	push(si);
	createpanel();
	showpanel();
	showman();
	showexit();
	obicons();
	si = pop();
	es = pop();
	di = 36;
	bx = 104;
	dl = 241;
	ah = 0;
	printdirect();
	worktoscreenm();
	cx = pop();
	hangonp();
}

void DreamGenContext::findpuztext() {
	STACK_CHECK;
	ah = 0;
	si = ax;
	_add(si, si);
	es = data.word(kPuzzletext);
	ax = es.word(si);
	_add(ax, (66*2));
	si = ax;
}

void DreamGenContext::placesetobject() {
	STACK_CHECK;
	push(es);
	push(bx);
	cl = 0;
	ch = 0;
	findormake();
	getsetad();
	es.byte(bx+58) = 0;
	bx = pop();
	es = pop();
}

void DreamGenContext::removesetobject() {
	STACK_CHECK;
	push(es);
	push(bx);
	cl = 255;
	ch = 0;
	findormake();
	getsetad();
	es.byte(bx+58) = 255;
	bx = pop();
	es = pop();
}

void DreamGenContext::issetobonmap() {
	STACK_CHECK;
	push(es);
	push(bx);
	getsetad();
	al = es.byte(bx+58);
	bx = pop();
	es = pop();
	_cmp(al, 0);
}

void DreamGenContext::placefreeobject() {
	STACK_CHECK;
	push(es);
	push(bx);
	cl = 0;
	ch = 1;
	findormake();
	getfreead();
	es.byte(bx+2) = 0;
	bx = pop();
	es = pop();
}

void DreamGenContext::removefreeobject() {
	STACK_CHECK;
	push(es);
	push(bx);
	getfreead();
	es.byte(bx+2) = 255;
	bx = pop();
	es = pop();
}

void DreamGenContext::findormake() {
	STACK_CHECK;
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
	push(ax);
	es = data.word(kBuffers);
	ah = data.byte(kReallocation);
changeloop:
	_cmp(es.byte(bx), 255);
	if (flags.z())
		goto haventfound;
	_cmp(ax, es.word(bx));
	if (!flags.z())
		goto nofoundchange;
	_cmp(ch, es.byte(bx+3));
	if (flags.z())
		goto foundchange;
nofoundchange:
	_add(bx, 4);
	goto changeloop;
foundchange:
	ax = pop();
	es.byte(bx+2) = cl;
	return;
haventfound:
	es.word(bx) = ax;
	es.word(bx+2) = cx;
	ax = pop();
}

void DreamGenContext::switchryanon() {
	STACK_CHECK;
	data.byte(kRyanon) = 255;
}

void DreamGenContext::switchryanoff() {
	STACK_CHECK;
	data.byte(kRyanon) = 1;
}

void DreamGenContext::setallchanges() {
	STACK_CHECK;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
setallloop:
	ax = es.word(bx);
	_cmp(al, 255);
	if (flags.z())
		return /* (endsetloop) */;
	cx = es.word(bx+2);
	_add(bx, 4);
	_cmp(ah, data.byte(kReallocation));
	if (!flags.z())
		goto setallloop;
	push(es);
	push(bx);
	dochange();
	bx = pop();
	es = pop();
	goto setallloop;
}

void DreamGenContext::dochange() {
	STACK_CHECK;
	_cmp(ch, 0);
	if (flags.z())
		goto object;
	_cmp(ch, 1);
	if (flags.z())
		goto freeobject;
	push(cx);
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	push(ax);
	al = ch;
	_sub(al, 100);
	ah = 0;
	cx = 144;
	_mul(cx);
	bx = pop();
	_add(bx, ax);
	_add(bx, (0));
	es = data.word(kReels);
	cx = pop();
	es.byte(bx+6) = cl;
	return;
object:
	push(cx);
	getsetad();
	cx = pop();
	es.byte(bx+58) = cl;
	return;
freeobject:
	push(cx);
	getfreead();
	cx = pop();
	_cmp(es.byte(bx+2), 255);
	if (!flags.z())
		return /* (beenpickedup) */;
	es.byte(bx+2) = cl;
}

void DreamGenContext::autoappear() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 32);
	if (!flags.z())
		goto notinalley;
	al = 5;
	resetlocation();
	al = 10;
	setlocation();
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
	placesetobject();
	al = 18;
	placesetobject();
	al = 93;
	placesetobject();
	al = 92;
	removesetobject();
	al = 55;
	removesetobject();
	al = 75;
	removesetobject();
	al = 84;
	removesetobject();
	al = 85;
	removesetobject();
	return;
edenspart2:
	_cmp(data.byte(kSartaindead), 1);
	if (!flags.z())
		return /* (notedens2) */;
	al = 44;
	removesetobject();
	al = 93;
	removesetobject();
	al = 55;
	placesetobject();
	_inc(data.byte(kSartaindead));
	return;
notinedens:
	_cmp(data.byte(kReallocation), 25);
	if (!flags.z())
		goto notonsartroof;
	data.byte(kNewsitem) = 3;
	al = 6;
	resetlocation();
	al = 11;
	setlocation();
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
	placesetobject();
}

void DreamGenContext::dumptimedtext() {
	STACK_CHECK;
	_cmp(data.byte(kNeedtodumptimed), 1);
	if (!flags.z())
		return /* (nodumptimed) */;
	al = data.byte(kTimedy);
	_cmp(data.byte(kForeignrelease),  0);
	if (flags.z())
		goto _tmp1;
	_sub(al, 3);
_tmp1:
	ah = 0;
	bx = ax;
	al = data.byte(kTimedx);
	ah = 0;
	di = ax;
	cl = 240;
	ch = (30);
	multidump();
	data.byte(kNeedtodumptimed) = 0;
}

void DreamGenContext::setuptimeduse() {
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

void DreamGenContext::setuptimedtemp() {
	STACK_CHECK;
	_cmp(ah, 0);
	if (flags.z())
		goto notloadspeech3;
	push(ax);
	push(bx);
	push(cx);
	push(dx);
	dl = 'T';
	dh = ah;
	cl = 'T';
	ah = 0;
	loadspeech();
	_cmp(data.byte(kSpeechloaded), 1);
	if (!flags.z())
		goto _tmp1;
	al = 50+12;
	playchannel1();
_tmp1:
	dx = pop();
	cx = pop();
	bx = pop();
	ax = pop();
	_cmp(data.byte(kSpeechloaded), 1);
	if (!flags.z())
		goto notloadspeech3;
	_cmp(data.byte(kSubtitles),  1);
	if (flags.z())
		goto notloadspeech3;
	return;
notloadspeech3:
	_cmp(data.word(kTimecount), 0);
	if (!flags.z())
		return /* (cantsetup2) */;
	data.byte(kTimedy) = bh;
	data.byte(kTimedx) = bl;
	data.word(kCounttotimed) = cx;
	_add(dx, cx);
	data.word(kTimecount) = dx;
	bl = al;
	bh = 0;
	_add(bx, bx);
	es = data.word(kTextfile1);
	cx = (66*2);
	ax = es.word(bx);
	_add(ax, cx);
	bx = ax;
	data.word(kTimedseg) = es;
	data.word(kTimedoffset) = bx;
}

void DreamGenContext::edenscdplayer() {
	STACK_CHECK;
	showfirstuse();
	data.word(kWatchingtime) = 18*2;
	data.word(kReeltowatch) = 25;
	data.word(kEndwatchreel) = 42;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
}

void DreamGenContext::usewall() {
	STACK_CHECK;
	showfirstuse();
	_cmp(data.byte(kManspath), 3);
	if (flags.z())
		goto gobackover;
	data.word(kWatchingtime) = 30*2;
	data.word(kReeltowatch) = 2;
	data.word(kEndwatchreel) = 31;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	al = 3;
	turnpathon();
	al = 4;
	turnpathon();
	al = 0;
	turnpathoff();
	al = 1;
	turnpathoff();
	al = 2;
	turnpathoff();
	al = 5;
	turnpathoff();
	data.byte(kManspath) = 3;
	data.byte(kFinaldest) = 3;
	findxyfrompath();
	data.byte(kResetmanxy) = 1;
	switchryanoff();
	return;
gobackover:
	data.word(kWatchingtime) = 30*2;
	data.word(kReeltowatch) = 34;
	data.word(kEndwatchreel) = 60;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	al = 3;
	turnpathoff();
	al = 4;
	turnpathoff();
	al = 0;
	turnpathon();
	al = 1;
	turnpathon();
	al = 2;
	turnpathon();
	al = 5;
	turnpathon();
	data.byte(kManspath) = 5;
	data.byte(kFinaldest) = 5;
	findxyfrompath();
	data.byte(kResetmanxy) = 1;
	switchryanoff();
}

void DreamGenContext::usechurchgate() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto gatewith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
cutgate:
	showfirstuse();
	data.word(kWatchingtime) = 64*2;
	data.word(kReeltowatch) = 4;
	data.word(kEndwatchreel) = 70;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	_inc(data.byte(kProgresspoints));
	al = 3;
	turnpathon();
	_cmp(data.byte(kAidedead), 0);
	if (flags.z())
		return /* (notopenchurch) */;
	al = 2;
	turnpathon();
}

void DreamGenContext::usegun() {
	STACK_CHECK;
	_cmp(data.byte(kObjecttype), 4);
	if (flags.z())
		goto istakengun;
	showseconduse();
	putbackobstuff();
	return;
istakengun:
	_cmp(data.byte(kReallocation), 22);
	if (!flags.z())
		goto notinpoolroom;
	cx = 300;
	al = 34;
	showpuztext();
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
	showpuztext();
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
	showpuztext();
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
	issetobonmap();
	if (flags.z())
		goto notbystudio;
	_cmp(data.byte(kManspath), 9);
	if (flags.z())
		goto notbystudio;
	data.byte(kDestination) = 9;
	data.byte(kFinaldest) = 9;
	autosetwalk();
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
	issetobonmap();
	if (!flags.z())
		goto notsarters;
	data.byte(kDestination) = 1;
	data.byte(kFinaldest) = 1;
	autosetwalk();
	al = 5;
	removesetobject();
	al = 6;
	placesetobject();
	al = 1;
	ah = data.byte(kRoomnum);
	_dec(ah);
	turnanypathon();
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
	resetlocation();
	al = 12;
	setlocation();
	data.byte(kDestpos) = 12;
	data.byte(kDestination) = 2;
	data.byte(kFinaldest) = 2;
	autosetwalk();
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
	autosetwalk();
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
	autosetwalk();
pathoktv:
	data.byte(kLastweapon) = 1;
	data.byte(kGetback) = 1;
	return;
nottvsoldier:
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::useshield() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 20);
	if (!flags.z())
		goto notinsartroom;
	_cmp(data.byte(kCombatcount), 0);
	if (flags.z())
		goto notinsartroom;
	data.byte(kLastweapon) = 3;
	showseconduse();
	data.byte(kGetback) = 1;
	_inc(data.byte(kProgresspoints));
	removeobfrominv();
	return;
notinsartroom:
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::usebuttona() {
	STACK_CHECK;
	al = 95;
	issetobonmap();
	if (flags.z())
		goto donethisbit;
	showfirstuse();
	al = 0;
	ah = data.byte(kRoomnum);
	_dec(ah);
	turnanypathon();
	al = 9;
	removesetobject();
	al = 95;
	placesetobject();
	data.word(kWatchingtime) = 15*2;
	data.word(kReeltowatch) = 71;
	data.word(kEndwatchreel) = 85;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	data.byte(kGetback) = 1;
	_inc(data.byte(kProgresspoints));
	return;
donethisbit:
	showseconduse();
	putbackobstuff();
}

void DreamGenContext::useplate() {
	STACK_CHECK;
	_cmp(data.byte(kWithobject), 255);
	if (!flags.z())
		goto platewith;
	withwhat();
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
	showpuztext();
	putbackobstuff();
	return;
unscrewplate:
	al = 20;
	playchannel1();
	showfirstuse();
	al = 28;
	placesetobject();
	al = 24;
	placesetobject();
	al = 25;
	removesetobject();
	al = 0;
	placefreeobject();
	_inc(data.byte(kProgresspoints));
	data.byte(kGetback) = 1;
	return;
triedknife:
	cx = 300;
	al = 54;
	showpuztext();
	putbackobstuff();
}

void DreamGenContext::usewinch() {
	STACK_CHECK;
	al = 40;
	ah = 1;
	checkinside();
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
	showfirstuse();
	putbackobstuff();
}

void DreamGenContext::entercode() {
	STACK_CHECK;
	data.word(kKeypadax) = ax;
	data.word(kKeypadcx) = cx;
	getridofreels();
	loadkeypad();
	createpanel();
	showicon();
	showouterpad();
	showkeypad();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
	data.word(kPresspointer) = 0;
	data.byte(kGetback) = 0;
keypadloop:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		goto numberright;
	delpointer();
	readmouse();
	showkeypad();
	showpointer();
	vsync();
	_cmp(data.byte(kPresscount), 0);
	if (flags.z())
		goto nopresses;
	_dec(data.byte(kPresscount));
	goto afterpress;
nopresses:
	data.byte(kPressed) = 255;
	data.byte(kGraphicpress) = 255;
	vsync();
afterpress:
	dumppointer();
	dumpkeypad();
	dumptextline();
	bx = 3482;
	checkcoords();
	_cmp(data.byte(kGetback), 1);
	if (flags.z())
		goto numberright;
	_cmp(data.byte(kLightcount), 1);
	if (!flags.z())
		goto notendkey;
	_cmp(data.byte(kLockstatus), 0);
	if (flags.z())
		goto numberright;
	goto keypadloop;
notendkey:
	_cmp(data.byte(kPresscount), 40);
	if (!flags.z())
		goto keypadloop;
	addtopresslist();
	_cmp(data.byte(kPressed), 11);
	if (!flags.z())
		goto keypadloop;
	ax = data.word(kKeypadax);
	cx = data.word(kKeypadcx);
	isitright();
	if (!flags.z())
		goto incorrect;
	data.byte(kLockstatus) = 0;
	al = 11;
	playchannel1();
	data.byte(kLightcount) = 120;
	data.word(kPresspointer) = 0;
	goto keypadloop;
incorrect:
	al = 11;
	playchannel1();
	data.byte(kLightcount) = 120;
	data.word(kPresspointer) = 0;
	goto keypadloop;
numberright:
	data.byte(kManisoffscreen) = 0;
	getridoftemp();
	restorereels();
	redrawmainscrn();
	worktoscreenm();
}

void DreamGenContext::loadkeypad() {
	STACK_CHECK;
	dx = 1948;
	loadintotemp();
}

void DreamGenContext::quitkey() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 222);
	if (flags.z())
		goto alreadyqk;
	data.byte(kCommandtype) = 222;
	al = 4;
	commandonly();
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

void DreamGenContext::addtopresslist() {
	STACK_CHECK;
	_cmp(data.word(kPresspointer), 5);
	if (flags.z())
		return /* (nomorekeys) */;
	al = data.byte(kPressed);
	_cmp(al, 10);
	if (!flags.z())
		goto not10;
	al = 0;
not10:
	bx = data.word(kPresspointer);
	dx = data;
	es = dx;
	_add(bx, 8573);
	es.byte(bx) = al;
	_inc(data.word(kPresspointer));
}

void DreamGenContext::buttonone() {
	STACK_CHECK;
	cl = 1;
	buttonpress();
}

void DreamGenContext::buttontwo() {
	STACK_CHECK;
	cl = 2;
	buttonpress();
}

void DreamGenContext::buttonthree() {
	STACK_CHECK;
	cl = 3;
	buttonpress();
}

void DreamGenContext::buttonfour() {
	STACK_CHECK;
	cl = 4;
	buttonpress();
}

void DreamGenContext::buttonfive() {
	STACK_CHECK;
	cl = 5;
	buttonpress();
}

void DreamGenContext::buttonsix() {
	STACK_CHECK;
	cl = 6;
	buttonpress();
}

void DreamGenContext::buttonseven() {
	STACK_CHECK;
	cl = 7;
	buttonpress();
}

void DreamGenContext::buttoneight() {
	STACK_CHECK;
	cl = 8;
	buttonpress();
}

void DreamGenContext::buttonnine() {
	STACK_CHECK;
	cl = 9;
	buttonpress();
}

void DreamGenContext::buttonnought() {
	STACK_CHECK;
	cl = 10;
	buttonpress();
}

void DreamGenContext::buttonenter() {
	STACK_CHECK;
	cl = 11;
	buttonpress();
}

void DreamGenContext::buttonpress() {
	STACK_CHECK;
	ch = cl;
	_add(ch, 100);
	_cmp(data.byte(kCommandtype), ch);
	if (flags.z())
		goto alreadyb;
	data.byte(kCommandtype) = ch;
	al = cl;
	_add(al, 4);
	push(cx);
	commandonly();
	cx = pop();
alreadyb:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notb) */;
	_and(ax, 1);
	if (!flags.z())
		goto dob;
	return;
dob:
	data.byte(kPressed) = cl;
	_add(cl, 21);
	data.byte(kGraphicpress) = cl;
	data.byte(kPresscount) = 40;
	_cmp(cl, 32);
	if (flags.z())
		return /* (nonoise) */;
	al = 10;
	playchannel1();
}

void DreamGenContext::showouterpad() {
	STACK_CHECK;
	di = (36+112)-3;
	bx = (72)-4;
	ds = data.word(kTempgraphics);
	al = 1;
	ah = 0;
	showframe();
	di = (36+112)+74;
	bx = (72)+76;
	ds = data.word(kTempgraphics);
	al = 37;
	ah = 0;
	showframe();
}

void DreamGenContext::showkeypad() {
	STACK_CHECK;
	al = 22;
	di = (36+112)+9;
	bx = (72)+5;
	singlekey();
	al = 23;
	di = (36+112)+31;
	bx = (72)+5;
	singlekey();
	al = 24;
	di = (36+112)+53;
	bx = (72)+5;
	singlekey();
	al = 25;
	di = (36+112)+9;
	bx = (72)+23;
	singlekey();
	al = 26;
	di = (36+112)+31;
	bx = (72)+23;
	singlekey();
	al = 27;
	di = (36+112)+53;
	bx = (72)+23;
	singlekey();
	al = 28;
	di = (36+112)+9;
	bx = (72)+41;
	singlekey();
	al = 29;
	di = (36+112)+31;
	bx = (72)+41;
	singlekey();
	al = 30;
	di = (36+112)+53;
	bx = (72)+41;
	singlekey();
	al = 31;
	di = (36+112)+9;
	bx = (72)+59;
	singlekey();
	al = 32;
	di = (36+112)+31;
	bx = (72)+59;
	singlekey();
	_cmp(data.byte(kLightcount), 0);
	if (flags.z())
		return /* (notenter) */;
	_dec(data.byte(kLightcount));
	al = 36;
	bx = (72)-1+63;
	_cmp(data.byte(kLockstatus), 0);
	if (!flags.z())
		goto changelight;
	al = 41;
	bx = (72)+4+63;
changelight:
	_cmp(data.byte(kLightcount), 60);
	if (flags.c())
		goto gotlight;
	_cmp(data.byte(kLightcount), 100);
	if (!flags.c())
		goto gotlight;
	_dec(al);
gotlight:
	ds = data.word(kTempgraphics);
	ah = 0;
	di = (36+112)+60;
	showframe();
}

void DreamGenContext::singlekey() {
	STACK_CHECK;
	_cmp(data.byte(kGraphicpress), al);
	if (!flags.z())
		goto gotkey;
	_add(al, 11);
	_cmp(data.byte(kPresscount), 8);
	if (!flags.c())
		goto gotkey;
	_sub(al, 11);
gotkey:
	ds = data.word(kTempgraphics);
	_sub(al, 20);
	ah = 0;
	showframe();
}

void DreamGenContext::dumpkeypad() {
	STACK_CHECK;
	di = (36+112)-3;
	bx = (72)-4;
	cl = 120;
	ch = 90;
	multidump();
}

void DreamGenContext::usemenu() {
	STACK_CHECK;
	getridofreels();
	loadmenu();
	createpanel();
	showpanel();
	showicon();
	data.byte(kNewobs) = 0;
	drawfloor();
	printsprites();
	al = 4;
	ah = 0;
	di = (80+40)-48;
	bx = (60)-4;
	ds = data.word(kTempgraphics2);
	showframe();
	getundermenu();
	al = 5;
	ah = 0;
	di = (80+40)+54;
	bx = (60)+72;
	ds = data.word(kTempgraphics2);
	showframe();
	worktoscreenm();
	data.byte(kGetback) = 0;
menuloop:
	delpointer();
	putundermenu();
	showmenu();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumpmenu();
	dumptextline();
	bx = 3614;
	checkcoords();
	_cmp(data.byte(kGetback), 1);
	if (!flags.z())
		goto menuloop;
	data.byte(kManisoffscreen) = 0;
	redrawmainscrn();
	getridoftemp();
	getridoftemp2();
	restorereels();
	worktoscreenm();
}

void DreamGenContext::dumpmenu() {
	STACK_CHECK;
	di = (80+40);
	bx = (60);
	cl = 48;
	ch = 48;
	multidump();
}

void DreamGenContext::getundermenu() {
	STACK_CHECK;
	di = (80+40);
	bx = (60);
	cl = 48;
	ch = 48;
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4));
	multiget();
}

void DreamGenContext::putundermenu() {
	STACK_CHECK;
	di = (80+40);
	bx = (60);
	cl = 48;
	ch = 48;
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4));
	multiput();
}

void DreamGenContext::showoutermenu() {
	STACK_CHECK;
	al = 40;
	ah = 0;
	di = (80+40)-34;
	bx = (60)-40;
	ds = data.word(kTempgraphics);
	showframe();
	al = 41;
	ah = 0;
	di = (80+40)+64-34;
	bx = (60)-40;
	ds = data.word(kTempgraphics);
	showframe();
	al = 42;
	ah = 0;
	di = (80+40)-26;
	bx = (60)+57-40;
	ds = data.word(kTempgraphics);
	showframe();
	al = 43;
	ah = 0;
	di = (80+40)+64-26;
	bx = (60)+57-40;
	ds = data.word(kTempgraphics);
	showframe();
}

void DreamGenContext::showmenu() {
	STACK_CHECK;
	_inc(data.byte(kMenucount));
	_cmp(data.byte(kMenucount), 37*2);
	if (!flags.z())
		goto menuframeok;
	data.byte(kMenucount) = 0;
menuframeok:
	al = data.byte(kMenucount);
	_shr(al, 1);
	ah = 0;
	di = (80+40);
	bx = (60);
	ds = data.word(kTempgraphics);
	showframe();
}

void DreamGenContext::loadmenu() {
	STACK_CHECK;
	dx = 1832;
	loadintotemp();
	dx = 1987;
	loadintotemp2();
}

void DreamGenContext::viewfolder() {
	STACK_CHECK;
	data.byte(kManisoffscreen) = 1;
	getridofall();
	loadfolder();
	data.byte(kFolderpage) = 0;
	showfolder();
	worktoscreenm();
	data.byte(kGetback) = 0;
folderloop:
	delpointer();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	bx = 3636;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto folderloop;
	data.byte(kManisoffscreen) = 0;
	getridoftemp();
	getridoftemp2();
	getridoftemp3();
	getridoftempcharset();
	restoreall();
	redrawmainscrn();
	worktoscreenm();
}

void DreamGenContext::nextfolder() {
	STACK_CHECK;
	_cmp(data.byte(kFolderpage), 12);
	if (!flags.z())
		goto cannextf;
	blank();
	return;
cannextf:
	_cmp(data.byte(kCommandtype), 201);
	if (flags.z())
		goto alreadynextf;
	data.byte(kCommandtype) = 201;
	al = 16;
	commandonly();
alreadynextf:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notnextf) */;
	_cmp(ax, 1);
	if (flags.z())
		goto donextf;
	return;
donextf:
	_inc(data.byte(kFolderpage));
	folderhints();
	delpointer();
	showfolder();
	data.word(kMousebutton) = 0;
	bx = 3636;
	checkcoords();
	worktoscreenm();
}

void DreamGenContext::folderhints() {
	STACK_CHECK;
	_cmp(data.byte(kFolderpage), 5);
	if (!flags.z())
		goto notaideadd;
	_cmp(data.byte(kAidedead), 1);
	if (flags.z())
		goto notaideadd;
	al = 13;
	getlocation();
	_cmp(al, 1);
	if (flags.z())
		goto notaideadd;
	al = 13;
	setlocation();
	showfolder();
	al = 30;
	findtext1();
	di = 0;
	bx = 86;
	dl = 141;
	ah = 16;
	printdirect();
	worktoscreenm();
	cx = 200;
	hangonp();
	return;
notaideadd:
	_cmp(data.byte(kFolderpage), 9);
	if (!flags.z())
		return /* (notaristoadd) */;
	al = 7;
	getlocation();
	_cmp(al, 1);
	if (flags.z())
		return /* (notaristoadd) */;
	al = 7;
	setlocation();
	showfolder();
	al = 31;
	findtext1();
	di = 0;
	bx = 86;
	dl = 141;
	ah = 16;
	printdirect();
	worktoscreenm();
	cx = 200;
	hangonp();
}

void DreamGenContext::lastfolder() {
	STACK_CHECK;
	_cmp(data.byte(kFolderpage), 0);
	if (!flags.z())
		goto canlastf;
	blank();
	return;
canlastf:
	_cmp(data.byte(kCommandtype), 202);
	if (flags.z())
		goto alreadylastf;
	data.byte(kCommandtype) = 202;
	al = 17;
	commandonly();
alreadylastf:
	_cmp(data.byte(kFolderpage), 0);
	if (flags.z())
		return /* (notlastf) */;
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notlastf) */;
	_cmp(ax, 1);
	if (flags.z())
		goto dolastf;
	return;
dolastf:
	_dec(data.byte(kFolderpage));
	delpointer();
	showfolder();
	data.word(kMousebutton) = 0;
	bx = 3636;
	checkcoords();
	worktoscreenm();
}

void DreamGenContext::loadfolder() {
	STACK_CHECK;
	dx = 2299;
	loadintotemp();
	dx = 2312;
	loadintotemp2();
	dx = 2325;
	loadintotemp3();
	dx = 1883;
	loadtempcharset();
	dx = 2195;
	loadtemptext();
}

void DreamGenContext::showfolder() {
	STACK_CHECK;
	data.byte(kCommandtype) = 255;
	_cmp(data.byte(kFolderpage), 0);
	if (flags.z())
		goto closedfolder;
	usetempcharset();
	createpanel2();
	ds = data.word(kTempgraphics);
	di = 0;
	bx = 0;
	al = 0;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = 143;
	bx = 0;
	al = 1;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = 0;
	bx = 92;
	al = 2;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = 143;
	bx = 92;
	al = 3;
	ah = 0;
	showframe();
	folderexit();
	_cmp(data.byte(kFolderpage), 1);
	if (flags.z())
		goto noleftpage;
	showleftpage();
noleftpage:
	_cmp(data.byte(kFolderpage), 12);
	if (flags.z())
		goto norightpage;
	showrightpage();
norightpage:
	usecharset1();
	undertextline();
	return;
closedfolder:
	createpanel2();
	ds = data.word(kTempgraphics3);
	di = 143-28;
	bx = 0;
	al = 0;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics3);
	di = 143-28;
	bx = 92;
	al = 1;
	ah = 0;
	showframe();
	folderexit();
	undertextline();
}

void DreamGenContext::folderexit() {
	STACK_CHECK;
	ds = data.word(kTempgraphics2);
	di = 296;
	bx = 178;
	al = 6;
	ah = 0;
	showframe();
}

void DreamGenContext::showleftpage() {
	STACK_CHECK;
	ds = data.word(kTempgraphics2);
	di = 0;
	bx = 12;
	al = 3;
	ah = 0;
	showframe();
	bx = 12+5;
	cx = 9;
leftpageloop:
	push(cx);
	push(bx);
	ds = data.word(kTempgraphics2);
	di = 0;
	al = 4;
	ah = 0;
	showframe();
	bx = pop();
	cx = pop();
	_add(bx, 16);
	if (--cx)
		goto leftpageloop;
	ds = data.word(kTempgraphics2);
	di = 0;
	al = 5;
	ah = 0;
	showframe();
	data.word(kLinespacing) = 8;
	data.word(kCharshift) = 91;
	data.byte(kKerning) = 1;
	bl = data.byte(kFolderpage);
	_dec(bl);
	_dec(bl);
	_add(bl, bl);
	bh = 0;
	_add(bx, bx);
	es = data.word(kTextfile1);
	si = es.word(bx);
	_add(si, 66*2);
	di = 2;
	bx = 48;
	dl = 140;
	cx = 2;
twolotsleft:
	push(cx);
contleftpage:
	printdirect();
	_add(bx, data.word(kLinespacing));
	_cmp(al, 0);
	if (!flags.z())
		goto contleftpage;
	cx = pop();
	if (--cx)
		goto twolotsleft;
	data.byte(kKerning) = 0;
	data.word(kCharshift) = 0;
	data.word(kLinespacing) = 10;
	es = data.word(kWorkspace);
	ds = data.word(kWorkspace);
	di = (48*320)+2;
	si = (48*320)+2+130;
	cx = 120;
flipfolder:
	push(cx);
	push(di);
	push(si);
	cx = 65;
flipfolderline:
	al = es.byte(di);
	ah = es.byte(si);
	es.byte(di) = ah;
	es.byte(si) = al;
	_dec(si);
	_inc(di);
	if (--cx)
		goto flipfolderline;
	si = pop();
	di = pop();
	cx = pop();
	_add(si, 320);
	_add(di, 320);
	if (--cx)
		goto flipfolder;
}

void DreamGenContext::showrightpage() {
	STACK_CHECK;
	ds = data.word(kTempgraphics2);
	di = 143;
	bx = 12;
	al = 0;
	ah = 0;
	showframe();
	bx = 12+37;
	cx = 7;
rightpageloop:
	push(cx);
	push(bx);
	ds = data.word(kTempgraphics2);
	di = 143;
	al = 1;
	ah = 0;
	showframe();
	bx = pop();
	cx = pop();
	_add(bx, 16);
	if (--cx)
		goto rightpageloop;
	ds = data.word(kTempgraphics2);
	di = 143;
	al = 2;
	ah = 0;
	showframe();
	data.word(kLinespacing) = 8;
	data.byte(kKerning) = 1;
	bl = data.byte(kFolderpage);
	_dec(bl);
	_add(bl, bl);
	bh = 0;
	_add(bx, bx);
	es = data.word(kTextfile1);
	si = es.word(bx);
	_add(si, 66*2);
	di = 152;
	bx = 48;
	dl = 140;
	cx = 2;
twolotsright:
	push(cx);
contrightpage:
	printdirect();
	_add(bx, data.word(kLinespacing));
	_cmp(al, 0);
	if (!flags.z())
		goto contrightpage;
	cx = pop();
	if (--cx)
		goto twolotsright;
	data.byte(kKerning) = 0;
	data.word(kLinespacing) = 10;
}

void DreamGenContext::entersymbol() {
	STACK_CHECK;
	data.byte(kManisoffscreen) = 1;
	getridofreels();
	dx = 2338;
	loadintotemp();
	data.byte(kSymboltopx) = 24;
	data.byte(kSymboltopdir) = 0;
	data.byte(kSymbolbotx) = 24;
	data.byte(kSymbolbotdir) = 0;
	redrawmainscrn();
	showsymbol();
	undertextline();
	worktoscreenm();
	data.byte(kGetback) = 0;
symbolloop:
	delpointer();
	updatesymboltop();
	updatesymbolbot();
	showsymbol();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	dumpsymbol();
	bx = 3678;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto symbolloop;
	_cmp(data.byte(kSymbolbotnum), 3);
	if (!flags.z())
		goto symbolwrong;
	_cmp(data.byte(kSymboltopnum), 5);
	if (!flags.z())
		goto symbolwrong;
	al = 43;
	removesetobject();
	al = 46;
	placesetobject();
	ah = data.byte(kRoomnum);
	_add(ah, 12);
	al = 0;
	turnanypathon();
	data.byte(kManisoffscreen) = 0;
	redrawmainscrn();
	getridoftemp();
	restorereels();
	worktoscreenm();
	al = 13;
	playchannel1();
	return;
symbolwrong:
	al = 46;
	removesetobject();
	al = 43;
	placesetobject();
	ah = data.byte(kRoomnum);
	_add(ah, 12);
	al = 0;
	turnanypathoff();
	data.byte(kManisoffscreen) = 0;
	redrawmainscrn();
	getridoftemp();
	restorereels();
	worktoscreenm();
}

void DreamGenContext::quitsymbol() {
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
	commandonly();
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

void DreamGenContext::settopleft() {
	STACK_CHECK;
	_cmp(data.byte(kSymboltopdir), 0);
	if (!flags.z())
		{ blank(); return; };
	_cmp(data.byte(kCommandtype), 210);
	if (flags.z())
		goto alreadytopl;
	data.byte(kCommandtype) = 210;
	al = 19;
	commandonly();
alreadytopl:
	_cmp(data.word(kMousebutton), 0);
	if (flags.z())
		return /* (notopleft) */;
	data.byte(kSymboltopdir) = -1;
}

void DreamGenContext::settopright() {
	STACK_CHECK;
	_cmp(data.byte(kSymboltopdir), 0);
	if (!flags.z())
		{ blank(); return; };
	_cmp(data.byte(kCommandtype), 211);
	if (flags.z())
		goto alreadytopr;
	data.byte(kCommandtype) = 211;
	al = 20;
	commandonly();
alreadytopr:
	_cmp(data.word(kMousebutton), 0);
	if (flags.z())
		return /* (notopright) */;
	data.byte(kSymboltopdir) = 1;
}

void DreamGenContext::setbotleft() {
	STACK_CHECK;
	_cmp(data.byte(kSymbolbotdir), 0);
	if (!flags.z())
		{ blank(); return; };
	_cmp(data.byte(kCommandtype), 212);
	if (flags.z())
		goto alreadybotl;
	data.byte(kCommandtype) = 212;
	al = 21;
	commandonly();
alreadybotl:
	_cmp(data.word(kMousebutton), 0);
	if (flags.z())
		return /* (nobotleft) */;
	data.byte(kSymbolbotdir) = -1;
}

void DreamGenContext::setbotright() {
	STACK_CHECK;
	_cmp(data.byte(kSymbolbotdir), 0);
	if (!flags.z())
		{ blank(); return; };
	_cmp(data.byte(kCommandtype), 213);
	if (flags.z())
		goto alreadybotr;
	data.byte(kCommandtype) = 213;
	al = 22;
	commandonly();
alreadybotr:
	_cmp(data.word(kMousebutton), 0);
	if (flags.z())
		return /* (nobotright) */;
	data.byte(kSymbolbotdir) = 1;
}

void DreamGenContext::dumpsymbol() {
	STACK_CHECK;
	data.byte(kNewtextline) = 0;
	di = (64);
	bx = (56)+20;
	cl = 104;
	ch = 60;
	multidump();
}

void DreamGenContext::showsymbol() {
	STACK_CHECK;
	al = 12;
	ah = 0;
	di = (64);
	bx = (56);
	ds = data.word(kTempgraphics);
	showframe();
	al = data.byte(kSymboltopx);
	ah = 0;
	di = ax;
	_add(di, (64)-44);
	al = data.byte(kSymboltopnum);
	bx = (56)+20;
	ds = data.word(kTempgraphics);
	ah = 32;
	push(ax);
	push(di);
	push(bx);
	push(ds);
	showframe();
	ds = pop();
	bx = pop();
	di = pop();
	ax = pop();
	nextsymbol();
	_add(di, 49);
	push(ax);
	push(di);
	push(bx);
	push(ds);
	showframe();
	ds = pop();
	bx = pop();
	di = pop();
	ax = pop();
	nextsymbol();
	_add(di, 49);
	showframe();
	al = data.byte(kSymbolbotx);
	ah = 0;
	di = ax;
	_add(di, (64)-44);
	al = data.byte(kSymbolbotnum);
	_add(al, 6);
	bx = (56)+49;
	ds = data.word(kTempgraphics);
	ah = 32;
	push(ax);
	push(di);
	push(bx);
	push(ds);
	showframe();
	ds = pop();
	bx = pop();
	di = pop();
	ax = pop();
	nextsymbol();
	_add(di, 49);
	push(ax);
	push(di);
	push(bx);
	push(ds);
	showframe();
	ds = pop();
	bx = pop();
	di = pop();
	ax = pop();
	nextsymbol();
	_add(di, 49);
	showframe();
}

void DreamGenContext::nextsymbol() {
	STACK_CHECK;
	_inc(al);
	_cmp(al, 6);
	if (flags.z())
		goto topwrap;
	_cmp(al, 12);
	if (flags.z())
		goto botwrap;
	return;
topwrap:
	al = 0;
	return;
botwrap:
	al = 6;
}

void DreamGenContext::updatesymboltop() {
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

void DreamGenContext::updatesymbolbot() {
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

void DreamGenContext::dumpsymbox() {
	STACK_CHECK;
	_cmp(data.word(kDumpx), -1);
	if (flags.z())
		return /* (nodumpsym) */;
	di = data.word(kDumpx);
	bx = data.word(kDumpy);
	cl = 30;
	ch = 77;
	multidump();
	data.word(kDumpx) = -1;
}

void DreamGenContext::usediary() {
	STACK_CHECK;
	getridofreels();
	dx = 2039;
	loadintotemp();
	dx = 2208;
	loadtemptext();
	dx = 1883;
	loadtempcharset();
	createpanel();
	showicon();
	showdiary();
	undertextline();
	showdiarypage();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
	data.byte(kGetback) = 0;
diaryloop:
	delpointer();
	readmouse();
	showdiarykeys();
	showpointer();
	vsync();
	dumppointer();
	dumpdiarykeys();
	dumptextline();
	bx = 3740;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto diaryloop;
	getridoftemp();
	getridoftemptext();
	getridoftempcharset();
	restorereels();
	data.byte(kManisoffscreen) = 0;
	redrawmainscrn();
	worktoscreenm();
}

void DreamGenContext::showdiary() {
	STACK_CHECK;
	al = 1;
	ah = 0;
	di = (68+24);
	bx = (48+12)+37;
	ds = data.word(kTempgraphics);
	showframe();
	al = 2;
	ah = 0;
	di = (68+24)+176;
	bx = (48+12)+108;
	ds = data.word(kTempgraphics);
	showframe();
}

void DreamGenContext::showdiarykeys() {
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
	showframe();
	_cmp(data.byte(kPresscount), 1);
	if (!flags.z())
		return /* (notshown) */;
	showdiarypage();
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
	showframe();
	_cmp(data.byte(kPresscount), 1);
	if (!flags.z())
		return /* (notshowp) */;
	showdiarypage();
}

void DreamGenContext::dumpdiarykeys() {
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
	getlocation();
	_cmp(al, 1);
	if (flags.z())
		goto notsartadd;
	al = 6;
	setlocation();
	delpointer();
	al = 12;
	findtext1();
	di = 70;
	bx = 106;
	dl = 241;
	ah = 16;
	printdirect();
	worktoscreenm();
	cx = 200;
	hangonp();
	createpanel();
	showicon();
	showdiary();
	showdiarypage();
	worktoscreenm();
	showpointer();
	return;
notsartadd:
	di = (68+24)+48;
	bx = (48+12)+15;
	cl = 200;
	ch = 16;
	multidump();
notdumpdiary:
	di = (68+24)+94;
	bx = (48+12)+97;
	cl = 16;
	ch = 16;
	multidump();
	di = (68+24)+151;
	bx = (48+12)+71;
	cl = 16;
	ch = 16;
	multidump();
}

void DreamGenContext::diarykeyp() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 214);
	if (flags.z())
		goto alreadykeyp;
	data.byte(kCommandtype) = 214;
	al = 23;
	commandonly();
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
	playchannel1();
	data.byte(kPresscount) = 12;
	data.byte(kPressed) = 'P';
	_dec(data.byte(kDiarypage));
	_cmp(data.byte(kDiarypage), -1);
	if (!flags.z())
		return /* (notkeyp) */;
	data.byte(kDiarypage) = 11;
}

void DreamGenContext::diarykeyn() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 213);
	if (flags.z())
		goto alreadykeyn;
	data.byte(kCommandtype) = 213;
	al = 23;
	commandonly();
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
	playchannel1();
	data.byte(kPresscount) = 12;
	data.byte(kPressed) = 'N';
	_inc(data.byte(kDiarypage));
	_cmp(data.byte(kDiarypage), 12);
	if (!flags.z())
		return /* (notkeyn) */;
	data.byte(kDiarypage) = 0;
}

void DreamGenContext::showdiarypage() {
	STACK_CHECK;
	al = 0;
	ah = 0;
	di = (68+24);
	bx = (48+12);
	ds = data.word(kTempgraphics);
	showframe();
	al = data.byte(kDiarypage);
	findtext1();
	data.byte(kKerning) = 1;
	usetempcharset();
	di = (68+24)+48;
	bx = (48+12)+16;
	dl = 240;
	ah = 16;
	data.word(kCharshift) = 91+91;
	printdirect();
	di = (68+24)+129;
	bx = (48+12)+16;
	dl = 240;
	ah = 16;
	printdirect();
	di = (68+24)+48;
	bx = (48+12)+23;
	dl = 240;
	ah = 16;
	printdirect();
	data.byte(kKerning) = 0;
	data.word(kCharshift) = 0;
	usecharset1();
}

void DreamGenContext::findtext1() {
	STACK_CHECK;
	ah = 0;
	si = ax;
	_add(si, si);
	es = data.word(kTextfile1);
	ax = es.word(si);
	_add(ax, (66*2));
	si = ax;
}

void DreamGenContext::zoomonoff() {
	STACK_CHECK;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		{ blank(); return; };
	_cmp(data.byte(kPointermode), 2);
	if (flags.z())
		{ blank(); return; };
	_cmp(data.byte(kCommandtype), 222);
	if (flags.z())
		goto alreadyonoff;
	data.byte(kCommandtype) = 222;
	al = 39;
	commandonly();
alreadyonoff:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (nozoomonoff) */;
	_and(ax, 1);
	if (!flags.z())
		goto dozoomonoff;
	return;
dozoomonoff:
	al = data.byte(kZoomon);
	_xor(al, 1);
	data.byte(kZoomon) = al;
	createpanel();
	data.byte(kNewobs) = 0;
	drawfloor();
	printsprites();
	reelsonscreen();
	showicon();
	getunderzoom();
	undertextline();
	al = 39;
	commandonly();
	readmouse();
	worktoscreenm();
}

void DreamGenContext::saveload() {
	STACK_CHECK;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		{ blank(); return; };
	_cmp(data.byte(kPointermode), 2);
	if (flags.z())
		{ blank(); return; };
	_cmp(data.byte(kCommandtype), 253);
	if (flags.z())
		goto alreadyops;
	data.byte(kCommandtype) = 253;
	al = 43;
	commandonly();
alreadyops:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (noops) */;
	_and(ax, 1);
	if (flags.z())
		return /* (noops) */;
	dosaveload();
}

void DreamGenContext::dosaveload() {
	STACK_CHECK;
	data.byte(kPointerframe) = 0;
	data.word(kTextaddressx) = 70;
	data.word(kTextaddressy) = 182-8;
	data.byte(kTextlen) = 181;
	data.byte(kManisoffscreen) = 1;
	clearwork();
	createpanel2();
	undertextline();
	getridofall();
	loadsavebox();
	showopbox();
	showmainops();
	worktoscreen();
	goto donefirstops;
restartops:
	showopbox();
	showmainops();
	worktoscreenm();
donefirstops:
	data.byte(kGetback) = 0;
waitops:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		goto justret;
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	delpointer();
	bx = 3782;
	checkcoords();
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
	getridoftemp();
	restoreall();
	redrawmainscrn();
	worktoscreenm();
	data.byte(kCommandtype) = 200;
justret:
	data.byte(kManisoffscreen) = 0;
}

void DreamGenContext::getbackfromops() {
	STACK_CHECK;
	_cmp(data.byte(kMandead), 2);
	if (flags.z())
		goto opsblock1;
	getback1();
	return;
opsblock1:
	blank();
}

void DreamGenContext::showmainops() {
	STACK_CHECK;
	ds = data.word(kTempgraphics);
	di = (60)+10;
	bx = (52)+10;
	al = 8;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = (60)+59;
	bx = (52)+30;
	al = 7;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = (60)+128+4;
	bx = (52)+12;
	al = 1;
	ah = 0;
	showframe();
}

void DreamGenContext::showdiscops() {
	STACK_CHECK;
	ds = data.word(kTempgraphics);
	di = (60)+128+4;
	bx = (52)+12;
	al = 1;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = (60)+10;
	bx = (52)+10;
	al = 9;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = (60)+59;
	bx = (52)+30;
	al = 10;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = (60)+176+2;
	bx = (52)+60-4;
	al = 5;
	ah = 0;
	showframe();
}

void DreamGenContext::loadsavebox() {
	STACK_CHECK;
	dx = 1961;
	loadintotemp();
}

void DreamGenContext::loadgame() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 246);
	if (flags.z())
		goto alreadyload;
	data.byte(kCommandtype) = 246;
	al = 41;
	commandonly();
alreadyload:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (noload) */;
	_cmp(ax, 1);
	if (flags.z())
		goto doload;
	return;
doload:
	data.byte(kLoadingorsave) = 1;
	showopbox();
	showloadops();
	data.byte(kCurrentslot) = 0;
	showslots();
	shownames();
	data.byte(kPointerframe) = 0;
	worktoscreenm();
	namestoold();
	data.byte(kGetback) = 0;
loadops:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (quitloaded) */;
	delpointer();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	bx = 3824;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto loadops;
	_cmp(data.byte(kGetback), 2);
	if (flags.z())
		return /* (quitloaded) */;
	getridoftemp();
	dx = data;
	es = dx;
	bx = 7979;
	startloading();
	loadroomssample();
	data.byte(kRoomloaded) = 1;
	data.byte(kNewlocation) = 255;
	clearsprites();
	initman();
	initrain();
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	startup();
	worktoscreen();
	data.byte(kGetback) = 4;
}

void DreamGenContext::getbacktoops() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 201);
	if (flags.z())
		goto alreadygetops;
	data.byte(kCommandtype) = 201;
	al = 42;
	commandonly();
alreadygetops:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (nogetbackops) */;
	_and(ax, 1);
	if (!flags.z())
		goto dogetbackops;
	return;
dogetbackops:
	oldtonames();
	data.byte(kGetback) = 2;
}

void DreamGenContext::discops() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 249);
	if (flags.z())
		goto alreadydiscops;
	data.byte(kCommandtype) = 249;
	al = 43;
	commandonly();
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
	scanfornames();
	data.byte(kLoadingorsave) = 2;
	showopbox();
	showdiscops();
	data.byte(kCurrentslot) = 0;
	worktoscreenm();
	data.byte(kGetback) = 0;
discopsloop:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (quitdiscops) */;
	delpointer();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	bx = 3866;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto discopsloop;
}

void DreamGenContext::savegame() {
	STACK_CHECK;
	_cmp(data.byte(kMandead), 2);
	if (!flags.z())
		goto cansaveok;
	blank();
	return;
cansaveok:
	_cmp(data.byte(kCommandtype), 247);
	if (flags.z())
		goto alreadysave;
	data.byte(kCommandtype) = 247;
	al = 44;
	commandonly();
alreadysave:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (!flags.z())
		goto dosave;
	return;
dosave:
	data.byte(kLoadingorsave) = 2;
	showopbox();
	showsaveops();
	data.byte(kCurrentslot) = 0;
	showslots();
	shownames();
	worktoscreenm();
	namestoold();
	data.word(kBufferin) = 0;
	data.word(kBufferout) = 0;
	data.byte(kGetback) = 0;
saveops:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (quitsavegame) */;
	delpointer();
	checkinput();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	bx = 3908;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto saveops;
}

void DreamGenContext::actualsave() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 222);
	if (flags.z())
		goto alreadyactsave;
	data.byte(kCommandtype) = 222;
	al = 44;
	commandonly();
alreadyactsave:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (flags.z())
		return /* (noactsave) */;
	dx = data;
	ds = dx;
	si = 8579;
	al = data.byte(kCurrentslot);
	ah = 0;
	cx = 17;
	_mul(cx);
	_add(si, ax);
	_inc(si);
	_cmp(ds.byte(si), 0);
	if (flags.z())
		return /* (noactsave) */;
	al = data.byte(kLocation);
	ah = 0;
	cx = 32;
	_mul(cx);
	ds = cs;
	si = 6187;
	_add(si, ax);
	di = 7979;
	bx = di;
	es = cs;
	cx = 16;
	_movsw(cx, true);
	al = data.byte(kRoomssample);
	es.byte(bx+13) = al;
	al = data.byte(kMapx);
	es.byte(bx+15) = al;
	al = data.byte(kMapy);
	es.byte(bx+16) = al;
	al = data.byte(kLiftflag);
	es.byte(bx+20) = al;
	al = data.byte(kManspath);
	es.byte(bx+21) = al;
	al = data.byte(kFacing);
	es.byte(bx+22) = al;
	al = 255;
	es.byte(bx+27) = al;
	saveposition();
	getridoftemp();
	restoreall();
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	redrawmainscrn();
	worktoscreenm();
	data.byte(kGetback) = 4;
}

void DreamGenContext::actualload() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 221);
	if (flags.z())
		goto alreadyactload;
	data.byte(kCommandtype) = 221;
	al = 41;
	commandonly();
alreadyactload:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notactload) */;
	_cmp(ax, 1);
	if (!flags.z())
		return /* (notactload) */;
	dx = data;
	ds = dx;
	si = 8579;
	al = data.byte(kCurrentslot);
	ah = 0;
	cx = 17;
	_mul(cx);
	_add(si, ax);
	_inc(si);
	_cmp(ds.byte(si), 0);
	if (flags.z())
		return /* (notactload) */;
	loadposition();
	data.byte(kGetback) = 1;
}

void DreamGenContext::selectslot2() {
	STACK_CHECK;
	_cmp(data.word(kMousebutton), 0);
	if (flags.z())
		goto noselslot2;
	data.byte(kLoadingorsave) = 2;
noselslot2:
	selectslot();
}

void DreamGenContext::checkinput() {
	STACK_CHECK;
	_cmp(data.byte(kLoadingorsave), 3);
	if (flags.z())
		return /* (nokeypress) */;
	readkey();
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
	getnamepos();
	_dec(data.byte(kCursorpos));
	es.byte(bx) = 0;
	es.byte(bx+1) = 1;
	goto afterkey;
nodel2:
	_cmp(data.byte(kCursorpos), 14);
	if (flags.z())
		return /* (nokeypress) */;
	getnamepos();
	_inc(data.byte(kCursorpos));
	al = data.byte(kCurrentkey);
	es.byte(bx+1) = al;
	es.byte(bx+2) = 0;
	es.byte(bx+3) = 1;
	goto afterkey;
	return;
afterkey:
	showopbox();
	shownames();
	showslots();
	showsaveops();
	worktoscreenm();
}

void DreamGenContext::getnamepos() {
	STACK_CHECK;
	al = data.byte(kCurrentslot);
	ah = 0;
	cx = 17;
	_mul(cx);
	dx = data;
	es = dx;
	bx = 8579;
	_add(bx, ax);
	al = data.byte(kCursorpos);
	ah = 0;
	_add(bx, ax);
}

void DreamGenContext::showopbox() {
	STACK_CHECK;
	ds = data.word(kTempgraphics);
	di = (60);
	bx = (52);
	al = 0;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = (60);
	bx = (52)+55;
	al = 4;
	ah = 0;
	showframe();
}

void DreamGenContext::showloadops() {
	STACK_CHECK;
	ds = data.word(kTempgraphics);
	di = (60)+128+4;
	bx = (52)+12;
	al = 1;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = (60)+176+2;
	bx = (52)+60-4;
	al = 5;
	ah = 0;
	showframe();
	di = (60)+104;
	bx = (52)+14;
	al = 55;
	dl = 101;
	printmessage();
}

void DreamGenContext::showsaveops() {
	STACK_CHECK;
	ds = data.word(kTempgraphics);
	di = (60)+128+4;
	bx = (52)+12;
	al = 1;
	ah = 0;
	showframe();
	ds = data.word(kTempgraphics);
	di = (60)+176+2;
	bx = (52)+60-4;
	al = 5;
	ah = 0;
	showframe();
	di = (60)+104;
	bx = (52)+14;
	al = 54;
	dl = 101;
	printmessage();
}

void DreamGenContext::selectslot() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 244);
	if (flags.z())
		goto alreadysel;
	data.byte(kCommandtype) = 244;
	al = 45;
	commandonly();
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
	oldtonames();
	ax = data.word(kMousey);
	_sub(ax, (52)+4);
	cl = -1;
getslotnum:
	_inc(cl);
	_sub(ax, 11);
	if (!flags.c())
		goto getslotnum;
	data.byte(kCurrentslot) = cl;
	delpointer();
	showopbox();
	showslots();
	shownames();
	_cmp(data.byte(kLoadingorsave), 1);
	if (flags.z())
		goto isloadmode;
	showsaveops();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
	return;
isloadmode:
	showloadops();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
}

void DreamGenContext::showslots() {
	STACK_CHECK;
	di = (60)+7;
	bx = (52)+8;
	al = 2;
	ds = data.word(kTempgraphics);
	ah = 0;
	showframe();
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
	showframe();
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

void DreamGenContext::shownames() {
	STACK_CHECK;
	dx = data;
	es = dx;
	si = 8579+1;
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
	printdirect();
	si = pop();
	es.byte(si) = 0;
	es.byte(si+1) = 1;
	goto afterprintname;
loadmode:
	al = 0;
	dl = 200;
	ah = 0;
	data.word(kCharshift) = 91;
	printdirect();
	data.word(kCharshift) = 0;
	goto afterprintname;
nomatchslot2:
	dl = 200;
	ah = 0;
	printdirect();
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

void DreamGenContext::namestoold() {
	STACK_CHECK;
	ds = cs;
	si = 8579;
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
	es = data.word(kBuffers);
	cx = 17*4;
	_movsb(cx, true);
}

void DreamGenContext::oldtonames() {
	STACK_CHECK;
	es = cs;
	di = 8579;
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
	ds = data.word(kBuffers);
	cx = 17*4;
	_movsb(cx, true);
}

void DreamGenContext::saveposition() {
	STACK_CHECK;
	makeheader();
	al = data.byte(kCurrentslot);
	ah = 0;
	push(ax);
	cx = 13;
	_mul(cx);
	dx = data;
	ds = dx;
	dx = 8698;
	_add(dx, ax);
	openforsave();
	dx = data;
	ds = dx;
	dx = 6091;
	cx = (6187-6091);
	savefilewrite();
	dx = data;
	es = dx;
	di = 6141;
	ax = pop();
	cx = 17;
	_mul(cx);
	dx = data;
	ds = dx;
	dx = 8579;
	_add(dx, ax);
	saveseg();
	dx = data;
	ds = dx;
	dx = 0;
	saveseg();
	ds = data.word(kExtras);
	dx = (0);
	saveseg();
	ds = data.word(kBuffers);
	dx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
	saveseg();
	dx = data;
	ds = dx;
	dx = 7979;
	saveseg();
	dx = data;
	ds = dx;
	dx = 534;
	saveseg();
	closefile();
}

void DreamGenContext::loadposition() {
	STACK_CHECK;
	data.word(kTimecount) = 0;
	clearchanges();
	al = data.byte(kCurrentslot);
	ah = 0;
	push(ax);
	cx = 13;
	_mul(cx);
	dx = data;
	ds = dx;
	dx = 8698;
	_add(dx, ax);
	openfilefromc();
	ds = cs;
	dx = 6091;
	cx = (6187-6091);
	savefileread();
	es = cs;
	di = 6141;
	ax = pop();
	cx = 17;
	_mul(cx);
	dx = data;
	ds = dx;
	dx = 8579;
	_add(dx, ax);
	loadseg();
	dx = data;
	ds = dx;
	dx = 0;
	loadseg();
	ds = data.word(kExtras);
	dx = (0);
	loadseg();
	ds = data.word(kBuffers);
	dx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
	loadseg();
	dx = data;
	ds = dx;
	dx = 7979;
	loadseg();
	ds = cs;
	dx = 534;
	loadseg();
	closefile();
}

void DreamGenContext::makeheader() {
	STACK_CHECK;
	dx = data;
	es = dx;
	di = 6141;
	ax = 17;
	storeit();
	ax = (68-0);
	storeit();
	ax = (0+2080+30000+(16*114)+((114+2)*2)+18000);
	storeit();
	ax = (250)*4;
	storeit();
	ax = 48;
	storeit();
	ax = (991-534);
	storeit();
}

void DreamGenContext::storeit() {
	STACK_CHECK;
	_cmp(ax, 0);
	if (!flags.z())
		goto isntblank;
	_inc(ax);
isntblank:
	_stosw();
}

void DreamGenContext::findlen() {
	STACK_CHECK;
	_dec(bx);
	_add(bx, ax);
nextone:
	_cmp(cl, ds.byte(bx));
	if (!flags.z())
		return /* (foundlen) */;
	_dec(bx);
	_dec(ax);
	_cmp(ax, 0);
	if (!flags.z())
		goto nextone;
}

void DreamGenContext::scanfornames() {
	STACK_CHECK;
	dx = data;
	es = dx;
	di = 8579;
	dx = data;
	ds = dx;
	dx = 8698;
	cx = 7;
scanloop:
	push(es);
	push(ds);
	push(di);
	push(dx);
	push(cx);
	openfilefromc();
	if (flags.c())
		goto notexist;
	cx = pop();
	_inc(ch);
	push(cx);
	push(di);
	push(es);
	dx = data;
	ds = dx;
	dx = 6091;
	cx = (6187-6091);
	savefileread();
	dx = data;
	es = dx;
	di = 6141;
	ds = pop();
	dx = pop();
	loadseg();
	bx = data.word(kHandle);
	closefile();
notexist:
	cx = pop();
	dx = pop();
	di = pop();
	ds = pop();
	es = pop();
	_add(dx, 13);
	_add(di, 17);
	_dec(cl);
	if (!flags.z())
		goto scanloop;
	al = ch;
}

void DreamGenContext::decide() {
	STACK_CHECK;
	setmode();
	loadpalfromiff();
	clearpalette();
	data.byte(kPointermode) = 0;
	data.word(kWatchingtime) = 0;
	data.byte(kPointerframe) = 0;
	data.word(kTextaddressx) = 70;
	data.word(kTextaddressy) = 182-8;
	data.byte(kTextlen) = 181;
	data.byte(kManisoffscreen) = 1;
	loadsavebox();
	showdecisions();
	worktoscreen();
	fadescreenup();
	data.byte(kGetback) = 0;
waitdecide:
	_cmp(data.byte(kQuitrequested),  0);
	if (flags.z())
		goto _tmp1;
	return;
_tmp1:
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	delpointer();
	bx = 5057;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto waitdecide;
	_cmp(data.byte(kGetback), 4);
	if (flags.z())
		goto hasloadedroom;
	getridoftemp();
hasloadedroom:
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
}

void DreamGenContext::showdecisions() {
	STACK_CHECK;
	createpanel2();
	showopbox();
	ds = data.word(kTempgraphics);
	di = (60)+17;
	bx = (52)+13;
	al = 6;
	ah = 0;
	showframe();
	undertextline();
}

void DreamGenContext::newgame() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 251);
	if (flags.z())
		goto alreadynewgame;
	data.byte(kCommandtype) = 251;
	al = 47;
	commandonly();
alreadynewgame:
	ax = data.word(kMousebutton);
	_cmp(ax, 1);
	if (!flags.z())
		return /* (nonewgame) */;
	data.byte(kGetback) = 3;
}

void DreamGenContext::doload() {
	STACK_CHECK;
	data.byte(kLoadingorsave) = 1;
	showopbox();
	showloadops();
	data.byte(kCurrentslot) = 0;
	showslots();
	shownames();
	data.byte(kPointerframe) = 0;
	worktoscreenm();
	namestoold();
	data.byte(kGetback) = 0;
loadops:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (quitloaded) */;
	delpointer();
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	dumptextline();
	bx = 3824;
	checkcoords();
	_cmp(data.byte(kGetback), 0);
	if (flags.z())
		goto loadops;
	_cmp(data.byte(kGetback), 2);
	if (flags.z())
		return /* (quitloaded) */;
	getridoftemp();
	dx = data;
	es = dx;
	bx = 7979;
	startloading();
	loadroomssample();
	data.byte(kRoomloaded) = 1;
	data.byte(kNewlocation) = 255;
	clearsprites();
	initman();
	initrain();
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	startup();
	worktoscreen();
	data.byte(kGetback) = 4;
}

void DreamGenContext::loadold() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 252);
	if (flags.z())
		goto alreadyloadold;
	data.byte(kCommandtype) = 252;
	al = 48;
	commandonly();
alreadyloadold:
	ax = data.word(kMousebutton);
	_and(ax, 1);
	if (flags.z())
		return /* (noloadold) */;
	doload();
	_cmp(data.byte(kGetback), 4);
	if (flags.z())
		return /* (noloadold) */;
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (noloadold) */;
	showdecisions();
	worktoscreenm();
	data.byte(kGetback) = 0;
}

void DreamGenContext::createname() {
	STACK_CHECK;
	push(ax);
	di = 5105;
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

void DreamGenContext::trysoundalloc() {
	STACK_CHECK;
	_cmp(data.byte(kNeedsoundbuff), 1);
	if (flags.z())
		return /* (gotsoundbuff) */;
	_inc(data.byte(kSoundtimes));
	bx = (16384+2048)/16;
	allocatemem();
	data.word(kSoundbuffer) = ax;
	push(ax);
	al = ah;
	cl = 4;
	_shr(al, cl);
	data.byte(kSoundbufferpage) = al;
	ax = pop();
	cl = 4;
	_shl(ax, cl);
	data.word(kSoundbufferad) = ax;
	_cmp(ax, 0x0b7ff);
	if (!flags.c())
		goto soundfail;
	es = data.word(kSoundbuffer);
	di = 0;
	cx = 16384/2;
	ax = 0x7f7f;
	_stosw(cx, true);
	data.byte(kNeedsoundbuff) = 1;
	return;
soundfail:
	es = data.word(kSoundbuffer);
	deallocatemem();
}

void DreamGenContext::playchannel0() {
	STACK_CHECK;
	_cmp(data.byte(kSoundint), 255);
	if (flags.z())
		return /* (dontbother4) */;
	push(es);
	push(ds);
	push(bx);
	push(cx);
	push(di);
	push(si);
	data.byte(kCh0playing) = al;
	es = data.word(kSounddata);
	_cmp(al, 12);
	if (flags.c())
		goto notsecondbank;
	es = data.word(kSounddata2);
	_sub(al, 12);
notsecondbank:
	data.byte(kCh0repeat) = ah;
	ah = 0;
	_add(ax, ax);
	bx = ax;
	_add(ax, ax);
	_add(bx, ax);
	al = es.byte(bx);
	ah = 0;
	data.word(kCh0emmpage) = ax;
	ax = es.word(bx+1);
	data.word(kCh0offset) = ax;
	ax = es.word(bx+3);
	data.word(kCh0blockstocopy) = ax;
	_cmp(data.byte(kCh0repeat), 0);
	if (flags.z())
		goto nosetloop;
	ax = data.word(kCh0emmpage);
	data.word(kCh0oldemmpage) = ax;
	ax = data.word(kCh0offset);
	data.word(kCh0oldoffset) = ax;
	ax = data.word(kCh0blockstocopy);
	data.word(kCh0oldblockstocopy) = ax;
nosetloop:
	si = pop();
	di = pop();
	cx = pop();
	bx = pop();
	ds = pop();
	es = pop();
}

void DreamGenContext::playchannel1() {
	STACK_CHECK;
	_cmp(data.byte(kSoundint), 255);
	if (flags.z())
		return /* (dontbother5) */;
	_cmp(data.byte(kCh1playing), 7);
	if (flags.z())
		return /* (dontbother5) */;
	push(es);
	push(ds);
	push(bx);
	push(cx);
	push(di);
	push(si);
	data.byte(kCh1playing) = al;
	es = data.word(kSounddata);
	_cmp(al, 12);
	if (flags.c())
		goto notsecondbank1;
	es = data.word(kSounddata2);
	_sub(al, 12);
notsecondbank1:
	ah = 0;
	_add(ax, ax);
	bx = ax;
	_add(ax, ax);
	_add(bx, ax);
	al = es.byte(bx);
	ah = 0;
	data.word(kCh1emmpage) = ax;
	ax = es.word(bx+1);
	data.word(kCh1offset) = ax;
	ax = es.word(bx+3);
	data.word(kCh1blockstocopy) = ax;
	si = pop();
	di = pop();
	cx = pop();
	bx = pop();
	ds = pop();
	es = pop();
}

void DreamGenContext::makenextblock() {
	STACK_CHECK;
	volumeadjust();
	loopchannel0();
	_cmp(data.word(kCh1blockstocopy), 0);
	if (flags.z())
		goto mightbeonlych0;
	_cmp(data.word(kCh0blockstocopy), 0);
	if (flags.z())
		goto mightbeonlych1;
	_dec(data.word(kCh0blockstocopy));
	_dec(data.word(kCh1blockstocopy));
	bothchannels();
	return;
mightbeonlych1:
	data.byte(kCh0playing) = 255;
	_cmp(data.word(kCh1blockstocopy), 0);
	if (flags.z())
		return /* (notch1only) */;
	_dec(data.word(kCh1blockstocopy));
	channel1only();
	return;
mightbeonlych0:
	data.byte(kCh1playing) = 255;
	_cmp(data.word(kCh0blockstocopy), 0);
	if (flags.z())
		goto notch0only;
	_dec(data.word(kCh0blockstocopy));
	channel0only();
	return;
notch0only:
	es = data.word(kSoundbuffer);
	di = data.word(kSoundbufferwrite);
	cx = 1024;
	ax = 0x7f7f;
	_stosw(cx, true);
	_and(di, 16384-1);
	data.word(kSoundbufferwrite) = di;
}

void DreamGenContext::volumeadjust() {
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

void DreamGenContext::loopchannel0() {
	STACK_CHECK;
	_cmp(data.word(kCh0blockstocopy), 0);
	if (!flags.z())
		return /* (notloop) */;
	_cmp(data.byte(kCh0repeat), 0);
	if (flags.z())
		return /* (notloop) */;
	_cmp(data.byte(kCh0repeat), 255);
	if (flags.z())
		goto endlessloop;
	_dec(data.byte(kCh0repeat));
endlessloop:
	ax = data.word(kCh0oldemmpage);
	data.word(kCh0emmpage) = ax;
	ax = data.word(kCh0oldoffset);
	data.word(kCh0offset) = ax;
	ax = data.word(kCh0blockstocopy);
	_add(ax, data.word(kCh0oldblockstocopy));
	data.word(kCh0blockstocopy) = ax;
}

void DreamGenContext::channel0tran() {
	STACK_CHECK;
	_cmp(data.byte(kVolume), 0);
	if (!flags.z())
		goto lowvolumetran;
	cx = 1024;
	_movsw(cx, true);
	return;
lowvolumetran:
	cx = 1024;
	bh = data.byte(kVolume);
	bl = 0;
	_add(bx, 16384-256);
volloop:
	_lodsw();
	bl = al;
	al = es.byte(bx);
	bl = ah;
	ah = es.byte(bx);
	_stosw();
	if (--cx)
		goto volloop;
}

void DreamGenContext::domix() {
	STACK_CHECK;
	_cmp(data.byte(kVolume), 0);
	if (!flags.z())
		goto lowvolumemix;
slow:
	_lodsb();
	ah = ds.byte(bx);
	_inc(bx);
	_cmp(al, dh);
	if (!flags.c())
		goto toplot;
	_cmp(ah, dh);
	if (!flags.c())
		goto nodistort;
	_add(al, ah);
	if (flags.s())
		goto botok;
	_xor(al, al);
	_stosb();
	if (--cx)
		goto slow;
	return /* (doneit) */;
botok:
	_xor(al, dh);
	_stosb();
	if (--cx)
		goto slow;
	return /* (doneit) */;
toplot:
	_cmp(ah, dh);
	if (flags.c())
		goto nodistort;
	_add(al, ah);
	if (!flags.s())
		goto topok;
	al = dl;
	_stosb();
	if (--cx)
		goto slow;
	return /* (doneit) */;
topok:
	_xor(al, dh);
	_stosb();
	if (--cx)
		goto slow;
	return /* (doneit) */;
nodistort:
	_add(al, ah);
	_xor(al, dh);
	_stosb();
	if (--cx)
		goto slow;
	return /* (doneit) */;
lowvolumemix:
	_lodsb();
	push(bx);
	bh = data.byte(kVolume);
	_add(bh, 63);
	bl = al;
	al = es.byte(bx);
	bx = pop();
	ah = ds.byte(bx);
	_inc(bx);
	_cmp(al, dh);
	if (!flags.c())
		goto toplotv;
	_cmp(ah, dh);
	if (!flags.c())
		goto nodistortv;
	_add(al, ah);
	if (flags.s())
		goto botokv;
	_xor(al, al);
	_stosb();
	if (--cx)
		goto lowvolumemix;
	return /* (doneit) */;
botokv:
	_xor(al, dh);
	_stosb();
	if (--cx)
		goto lowvolumemix;
	return /* (doneit) */;
toplotv:
	_cmp(ah, dh);
	if (flags.c())
		goto nodistortv;
	_add(al, ah);
	if (!flags.s())
		goto topokv;
	al = dl;
	_stosb();
	if (--cx)
		goto lowvolumemix;
	return /* (doneit) */;
topokv:
	_xor(al, dh);
	_stosb();
	if (--cx)
		goto lowvolumemix;
	return /* (doneit) */;
nodistortv:
	_add(al, ah);
	_xor(al, dh);
	_stosb();
	if (--cx)
		goto lowvolumemix;
}

void DreamGenContext::entrytexts() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 21);
	if (!flags.z())
		goto notloc15;
	al = 28;
	cx = 60;
	dx = 11;
	bl = 68;
	bh = 64;
	setuptimeduse();
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
	setuptimeduse();
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
	setuptimeduse();
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
	setuptimeduse();
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
	setuptimeduse();
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
	setuptimeduse();
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
	setuptimeduse();
}

void DreamGenContext::entryanims() {
	STACK_CHECK;
	data.word(kReeltowatch) = -1;
	data.byte(kWatchmode) = -1;
	_cmp(data.byte(kLocation), 33);
	if (!flags.z())
		goto notinthebeach;
	switchryanoff();
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
	resetlocation();
	data.word(kWatchingtime) = 50*2;
	data.word(kReeltowatch) = 247;
	data.word(kEndwatchreel) = 297;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	switchryanoff();
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
	switchryanoff();
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
	switchryanoff();
	return;
notenterdream:
	_cmp(data.byte(kReallocation), 46);
	if (!flags.z())
		goto notcrystal;
	_cmp(data.byte(kSartaindead), 1);
	if (!flags.z())
		goto notcrystal;
	al = 0;
	removefreeobject();
	return;
notcrystal:
	_cmp(data.byte(kLocation), 9);
	if (!flags.z())
		goto nottopchurch;
	al = 2;
	checkifpathison();
	if (flags.z())
		goto nottopchurch;
	_cmp(data.byte(kAidedead), 0);
	if (flags.z())
		goto nottopchurch;
	al = 3;
	checkifpathison();
	if (!flags.z())
		goto makedoorsopen;
	al = 2;
	turnpathon();
makedoorsopen:
	al = 4;
	removesetobject();
	al = 5;
	placesetobject();
	return;
nottopchurch:
	_cmp(data.byte(kLocation), 47);
	if (!flags.z())
		goto notdreamcentre;
	al = 4;
	placesetobject();
	al = 5;
	placesetobject();
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
	switchryanoff();
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
	switchryanoff();
	return;
notalley:
	_cmp(data.byte(kLocation), 24);
	if (!flags.z())
		return /* (notedensagain) */;
	al = 2;
	ah = data.byte(kRoomnum);
	_dec(ah);
	turnanypathon();
}

void DreamGenContext::initialinv() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 24);
	if (flags.z())
		goto isedens;
	return;
isedens:
	al = 11;
	ah = 5;
	pickupob();
	al = 12;
	ah = 6;
	pickupob();
	al = 13;
	ah = 7;
	pickupob();
	al = 14;
	ah = 8;
	pickupob();
	al = 18;
	al = 18;
	ah = 0;
	pickupob();
	al = 19;
	ah = 1;
	pickupob();
	al = 20;
	ah = 9;
	pickupob();
	al = 16;
	ah = 2;
	pickupob();
	data.byte(kWatchmode) = 1;
	data.word(kReeltohold) = 0;
	data.word(kEndofholdreel) = 6;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	switchryanoff();
}

void DreamGenContext::pickupob() {
	STACK_CHECK;
	data.byte(kLastinvpos) = ah;
	data.byte(kObjecttype) = 2;
	data.byte(kItemframe) = al;
	data.byte(kCommand) = al;
	getanyad();
	transfertoex();
}

void DreamGenContext::checkforemm() {
	STACK_CHECK;
}

void DreamGenContext::checkbasemem() {
	STACK_CHECK;
	bx = data.word(kHowmuchalloc);
	_cmp(bx, 0x9360);
	if (!flags.c())
		return /* (enoughmem) */;
	data.byte(kGameerror) = 5;
	{ quickquit(); return; };
}

void DreamGenContext::allocatebuffers() {
	STACK_CHECK;
	bx = (0+2080+30000+(16*114)+((114+2)*2)+18000)/16;
	allocatemem();
	data.word(kExtras) = ax;
	trysoundalloc();
	bx = (0+(66*60))/16;
	allocatemem();
	data.word(kMapdata) = ax;
	trysoundalloc();
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+991-534+68-0)/16;
	allocatemem();
	data.word(kBuffers) = ax;
	trysoundalloc();
	bx = (16*80)/16;
	allocatemem();
	data.word(kFreedat) = ax;
	trysoundalloc();
	bx = (64*128)/16;
	allocatemem();
	data.word(kSetdat) = ax;
	trysoundalloc();
	bx = (22*8*20*8)/16;
	allocatemem();
	data.word(kMapstore) = ax;
	allocatework();
	bx = 2048/16;
	allocatemem();
	data.word(kSounddata) = ax;
	bx = 2048/16;
	allocatemem();
	data.word(kSounddata2) = ax;
}

void DreamGenContext::clearbuffers() {
	STACK_CHECK;
	es = data.word(kBuffers);
	cx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+991-534+68-0)/2;
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
	si = 534;
	cx = (991-534);
	_movsb(cx, true);
	es = data.word(kBuffers);
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+991-534);
	ds = cs;
	si = 0;
	cx = (68-0);
	_movsb(cx, true);
	clearchanges();
}

void DreamGenContext::clearchanges() {
	STACK_CHECK;
	es = data.word(kBuffers);
	cx = (250)*2;
	ax = 0x0ffff;
	di = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
	_stosw(cx, true);
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64));
	es = cs;
	di = 534;
	cx = (991-534);
	_movsb(cx, true);
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+991-534);
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
	di = 8011;
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

void DreamGenContext::clearbeforeload() {
	STACK_CHECK;
	_cmp(data.byte(kRoomloaded), 1);
	if (!flags.z())
		return /* (noclear) */;
	clearreels();
	clearrest();
	data.byte(kRoomloaded) = 0;
}

void DreamGenContext::clearreels() {
	STACK_CHECK;
	es = data.word(kReel1);
	deallocatemem();
	es = data.word(kReel2);
	deallocatemem();
	es = data.word(kReel3);
	deallocatemem();
}

void DreamGenContext::clearrest() {
	STACK_CHECK;
	es = data.word(kMapdata);
	cx = (66*60)/2;
	ax = 0;
	di = (0);
	_stosw(cx, true);
	es = data.word(kBackdrop);
	deallocatemem();
	es = data.word(kSetframes);
	deallocatemem();
	es = data.word(kReels);
	deallocatemem();
	es = data.word(kPeople);
	deallocatemem();
	es = data.word(kSetdesc);
	deallocatemem();
	es = data.word(kBlockdesc);
	deallocatemem();
	es = data.word(kRoomdesc);
	deallocatemem();
	es = data.word(kFreeframes);
	deallocatemem();
	es = data.word(kFreedesc);
	deallocatemem();
}

void DreamGenContext::parseblaster() {
	STACK_CHECK;
lookattail:
	al = es.byte(bx);
	_cmp(al, 0);
	if (flags.z())
		return /* (endtail) */;
	_cmp(al, 13);
	if (flags.z())
		return /* (endtail) */;
	_cmp(al, 'i');
	if (flags.z())
		goto issoundint;
	_cmp(al, 'I');
	if (flags.z())
		goto issoundint;
	_cmp(al, 'b');
	if (flags.z())
		goto isbright;
	_cmp(al, 'B');
	if (flags.z())
		goto isbright;
	_cmp(al, 'a');
	if (flags.z())
		goto isbaseadd;
	_cmp(al, 'A');
	if (flags.z())
		goto isbaseadd;
	_cmp(al, 'n');
	if (flags.z())
		goto isnosound;
	_cmp(al, 'N');
	if (flags.z())
		goto isnosound;
	_cmp(al, 'd');
	if (flags.z())
		goto isdma;
	_cmp(al, 'D');
	if (flags.z())
		goto isdma;
	_inc(bx);
	if (--cx)
		goto lookattail;
	return;
issoundint:
	al = es.byte(bx+1);
	_sub(al, '0');
	data.byte(kSoundint) = al;
	_inc(bx);
	goto lookattail;
isdma:
	al = es.byte(bx+1);
	_sub(al, '0');
	data.byte(kSounddmachannel) = al;
	_inc(bx);
	goto lookattail;
isbaseadd:
	push(cx);
	al = es.byte(bx+2);
	_sub(al, '0');
	ah = 0;
	cl = 4;
	_shl(ax, cl);
	_add(ax, 0x200);
	data.word(kSoundbaseadd) = ax;
	cx = pop();
	_inc(bx);
	goto lookattail;
isbright:
	data.byte(kBrightness) = 1;
	_inc(bx);
	goto lookattail;
isnosound:
	data.byte(kSoundint) = 255;
	_inc(bx);
	goto lookattail;
}

void DreamGenContext::startup() {
	STACK_CHECK;
	data.byte(kCurrentkey) = 0;
	data.byte(kMainmode) = 0;
	createpanel();
	data.byte(kNewobs) = 1;
	drawfloor();
	showicon();
	getunderzoom();
	spriteupdate();
	printsprites();
	undertextline();
	reelsonscreen();
	atmospheres();
}

void DreamGenContext::startup1() {
	STACK_CHECK;
	clearpalette();
	data.byte(kThroughdoor) = 0;
	data.byte(kCurrentkey) = '0';
	data.byte(kMainmode) = 0;
	createpanel();
	data.byte(kNewobs) = 1;
	drawfloor();
	showicon();
	getunderzoom();
	spriteupdate();
	printsprites();
	undertextline();
	reelsonscreen();
	atmospheres();
	worktoscreen();
	fadescreenup();
}

void DreamGenContext::screenupdate() {
	STACK_CHECK;
	newplace();
	mainscreen();
	animpointer();
	showpointer();
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		goto iswatchingmode;
	_cmp(data.byte(kNewlocation), 255);
	if (!flags.z())
		return /* (finishearly) */;
iswatchingmode:
	vsync();
	readmouse1();
	dumppointer();
	dumptextline();
	delpointer();
	autolook();
	spriteupdate();
	watchcount();
	zoom();
	showpointer();
	_cmp(data.byte(kWongame), 0);
	if (!flags.z())
		return /* (finishearly) */;
	vsync();
	readmouse2();
	dumppointer();
	dumpzoom();
	delpointer();
	deleverything();
	printsprites();
	reelsonscreen();
	afternewroom();
	showpointer();
	vsync();
	readmouse3();
	dumppointer();
	dumpmap();
	dumptimedtext();
	delpointer();
	showpointer();
	vsync();
	readmouse4();
	dumppointer();
	dumpwatch();
	delpointer();
}

void DreamGenContext::watchreel() {
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
	plotreel();
	ax = data.word(kReelpointer);
	data.word(kReeltowatch) = ax;
	checkforshake();
	return;
notplayingreel:
	_cmp(data.byte(kWatchmode), 1);
	if (!flags.z())
		goto notholdingreel;
	ax = data.word(kReeltohold);
	data.word(kReelpointer) = ax;
	plotreel();
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
	autosetwalk();
	return;
ismorereel2:
	ax = data.word(kReeltohold);
	data.word(kReelpointer) = ax;
	plotreel();
}

void DreamGenContext::checkforshake() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 26);
	if (!flags.z())
		return /* (notstartshake) */;
	_cmp(ax, 104);
	if (!flags.z())
		return /* (notstartshake) */;
	data.byte(kShakecounter) = -1;
}

void DreamGenContext::watchcount() {
	STACK_CHECK;
	_cmp(data.byte(kWatchon), 0);
	if (flags.z())
		return /* (nowatchworn) */;
	_inc(data.byte(kTimercount));
	_cmp(data.byte(kTimercount), 9);
	if (flags.z())
		goto flashdots;
	_cmp(data.byte(kTimercount), 18);
	if (flags.z())
		goto uptime;
	return;
flashdots:
	ax = 91*3+21;
	di = 268+4;
	bx = 21;
	ds = data.word(kCharset1);
	showframe();
	goto finishwatch;
uptime:
	data.byte(kTimercount) = 0;
	_add(data.byte(kSecondcount), 1);
	_cmp(data.byte(kSecondcount), 60);
	if (!flags.z())
		goto finishtime;
	data.byte(kSecondcount) = 0;
	_inc(data.byte(kMinutecount));
	_cmp(data.byte(kMinutecount), 60);
	if (!flags.z())
		goto finishtime;
	data.byte(kMinutecount) = 0;
	_inc(data.byte(kHourcount));
	_cmp(data.byte(kHourcount), 24);
	if (!flags.z())
		goto finishtime;
	data.byte(kHourcount) = 0;
finishtime:
	showtime();
finishwatch:
	data.byte(kWatchdump) = 1;
}

void DreamGenContext::showtime() {
	STACK_CHECK;
	_cmp(data.byte(kWatchon), 0);
	if (flags.z())
		return /* (nowatch) */;
	al = data.byte(kSecondcount);
	cl = 0;
	twodigitnum();
	push(ax);
	al = ah;
	ah = 0;
	_add(ax, 91*3+10);
	ds = data.word(kCharset1);
	di = 282+5;
	bx = 21;
	showframe();
	ax = pop();
	ah = 0;
	_add(ax, 91*3+10);
	ds = data.word(kCharset1);
	di = 282+9;
	bx = 21;
	showframe();
	al = data.byte(kMinutecount);
	cl = 0;
	twodigitnum();
	push(ax);
	al = ah;
	ah = 0;
	_add(ax, 91*3);
	ds = data.word(kCharset1);
	di = 270+5;
	bx = 21;
	showframe();
	ax = pop();
	ah = 0;
	_add(ax, 91*3);
	ds = data.word(kCharset1);
	di = 270+11;
	bx = 21;
	showframe();
	al = data.byte(kHourcount);
	cl = 0;
	twodigitnum();
	push(ax);
	al = ah;
	ah = 0;
	_add(ax, 91*3);
	ds = data.word(kCharset1);
	di = 256+5;
	bx = 21;
	showframe();
	ax = pop();
	ah = 0;
	_add(ax, 91*3);
	ds = data.word(kCharset1);
	di = 256+11;
	bx = 21;
	showframe();
	ax = 91*3+20;
	ds = data.word(kCharset1);
	di = 267+5;
	bx = 21;
	showframe();
}

void DreamGenContext::dumpwatch() {
	STACK_CHECK;
	_cmp(data.byte(kWatchdump), 1);
	if (!flags.z())
		return /* (nodumpwatch) */;
	di = 256;
	bx = 21;
	cl = 40;
	ch = 12;
	multidump();
	data.byte(kWatchdump) = 0;
}

void DreamGenContext::showbyte() {
	STACK_CHECK;
	dl = al;
	_shr(dl, 1);
	_shr(dl, 1);
	_shr(dl, 1);
	_shr(dl, 1);
	onedigit();
	es.byte(di) = dl;
	dl = al;
	_and(dl, 15);
	onedigit();
	es.byte(di+1) = dl;
	_add(di, 3);
}

void DreamGenContext::onedigit() {
	STACK_CHECK;
	_cmp(dl, 10);
	if (!flags.c())
		goto morethan10;
	_add(dl, '0');
	return;
morethan10:
	_sub(dl, 10);
	_add(dl, 'A');
}

void DreamGenContext::twodigitnum() {
	STACK_CHECK;
	ah = cl;
	_dec(ah);
numloop1:
	_inc(ah);
	_sub(al, 10);
	if (!flags.c())
		goto numloop1;
	_add(al, 10);
	_add(al, cl);
}

void DreamGenContext::showword() {
	STACK_CHECK;
	ch = 0;
	bx = 10000;
	cl = 47;
word1:
	_inc(cl);
	_sub(ax, bx);
	if (!flags.c())
		goto word1;
	_add(ax, bx);
	convnum();
	cs.byte(di) = cl;
	bx = 1000;
	cl = 47;
word2:
	_inc(cl);
	_sub(ax, bx);
	if (!flags.c())
		goto word2;
	_add(ax, bx);
	convnum();
	cs.byte(di+1) = cl;
	bx = 100;
	cl = 47;
word3:
	_inc(cl);
	_sub(ax, bx);
	if (!flags.c())
		goto word3;
	_add(ax, bx);
	convnum();
	cs.byte(di+2) = cl;
	bx = 10;
	cl = 47;
word4:
	_inc(cl);
	_sub(ax, bx);
	if (!flags.c())
		goto word4;
	_add(ax, bx);
	convnum();
	cs.byte(di+3) = cl;
	_add(al, 48);
	cl = al;
	convnum();
	cs.byte(di+4) = cl;
}

void DreamGenContext::convnum() {
	STACK_CHECK;
	_cmp(ch, 0);
	if (!flags.z())
		return /* (noconvnum) */;
	_cmp(cl, '0');
	if (!flags.z())
		goto notzeronum;
	cl = 32;
	return /* (noconvnum) */;
notzeronum:
	ch = 1;
}

void DreamGenContext::walkandexamine() {
	STACK_CHECK;
	finishedwalking();
	if (!flags.z())
		return /* (noobselect) */;
	al = data.byte(kWalkexamtype);
	data.byte(kCommandtype) = al;
	al = data.byte(kWalkexamnum);
	data.byte(kCommand) = al;
	data.byte(kWalkandexam) = 0;
	_cmp(data.byte(kCommandtype), 5);
	if (flags.z())
		return /* (noobselect) */;
	examineob();
	return;
wantstowalk:
	setwalk();
	data.byte(kReasseschanges) = 1;
	return;
diff:
	data.byte(kCommand) = al;
	data.byte(kCommandtype) = ah;
	_cmp(data.byte(kLinepointer), 254);
	if (!flags.z())
		goto middleofwalk;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		goto middleofwalk;
	al = data.byte(kFacing);
	_cmp(al, data.byte(kTurntoface));
	if (!flags.z())
		goto middleofwalk;
	_cmp(data.byte(kCommandtype), 3);
	if (!flags.z())
		goto notblock;
	bl = data.byte(kManspath);
	_cmp(bl, data.byte(kPointerspath));
	if (!flags.z())
		goto dontcheck;
	cl = data.byte(kRyanx);
	_add(cl, 12);
	ch = data.byte(kRyany);
	_add(ch, 12);
	checkone();
	_cmp(cl, 2);
	if (flags.c())
		goto isblock;
dontcheck:
	getflagunderp();
	_cmp(data.byte(kLastflag), 2);
	if (flags.c())
		goto isblock;
	_cmp(data.byte(kLastflag), 128);
	if (!flags.c())
		goto isblock;
	goto toofaraway;
notblock:
	bl = data.byte(kManspath);
	_cmp(bl, data.byte(kPointerspath));
	if (!flags.z())
		goto toofaraway;
	_cmp(data.byte(kCommandtype), 3);
	if (flags.z())
		goto isblock;
	_cmp(data.byte(kCommandtype), 5);
	if (flags.z())
		goto isaperson;
	examineobtext();
	return;
middleofwalk:
	blocknametext();
	return;
isblock:
	blocknametext();
	return;
isaperson:
	personnametext();
	return;
toofaraway:
	walktotext();
}

void DreamGenContext::mainscreen() {
	STACK_CHECK;
	data.byte(kInmaparea) = 0;
	bx = 5122;
	_cmp(data.byte(kWatchon), 1);
	if (flags.z())
		goto checkmain;
	bx = 5184;
checkmain:
	checkcoords();
	_cmp(data.byte(kWalkandexam), 0);
	if (flags.z())
		return /* (finishmain) */;
	walkandexamine();
}

void DreamGenContext::madmanrun() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 14);
	if (!flags.z())
		{ identifyob(); return; };
	_cmp(data.byte(kMapx), 22);
	if (!flags.z())
		{ identifyob(); return; };
	_cmp(data.byte(kPointermode), 2);
	if (!flags.z())
		{ identifyob(); return; };
	_cmp(data.byte(kMadmanflag), 0);
	if (!flags.z())
		{ identifyob(); return; };
	_cmp(data.byte(kCommandtype), 211);
	if (flags.z())
		goto alreadyrun;
	data.byte(kCommandtype) = 211;
	al = 52;
	commandonly();
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

void DreamGenContext::checkcoords() {
	STACK_CHECK;
	_cmp(data.byte(kNewlocation), 255);
	if (flags.z())
		goto loop048;
	return;
loop048:
	ax = cs.word(bx);
	_cmp(ax, 0x0ffff);
	if (flags.z())
		return /* (nonefound) */;
	push(bx);
	_cmp(data.word(kMousex), ax);
	if (flags.l())
		goto over045;
	ax = cs.word(bx+2);
	_cmp(data.word(kMousex), ax);
	if (!flags.l())
		goto over045;
	ax = cs.word(bx+4);
	_cmp(data.word(kMousey), ax);
	if (flags.l())
		goto over045;
	ax = cs.word(bx+6);
	_cmp(data.word(kMousey), ax);
	if (!flags.l())
		goto over045;
	ax = cs.word(bx+8);
	__dispatch_call(ax);
	ax = pop();
	return;
over045:
	bx = pop();
	_add(bx, 10);
	goto loop048;
}

void DreamGenContext::identifyob() {
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
	findpathofpoint();
	data.byte(kPointerspath) = dl;
	ax = pop();
	push(ax);
	findfirstpath();
	data.byte(kPointerfirstpath) = al;
	ax = pop();
	checkifex();
	if (!flags.z())
		return /* (finishidentify) */;
	checkiffree();
	if (!flags.z())
		return /* (finishidentify) */;
	checkifperson();
	if (!flags.z())
		return /* (finishidentify) */;
	checkifset();
	if (!flags.z())
		return /* (finishidentify) */;
	ax = data.word(kMousex);
	_sub(ax, data.word(kMapadx));
	cl = al;
	ax = data.word(kMousey);
	_sub(ax, data.word(kMapady));
	ch = al;
	checkone();
	_cmp(al, 0);
	if (flags.z())
		goto nothingund;
	_cmp(data.byte(kMandead), 1);
	if (flags.z())
		goto nothingund;
	ah = 3;
	obname();
	return;
nothingund:
	blank();
}

void DreamGenContext::checkifperson() {
	STACK_CHECK;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5));
	cx = 12;
identifyreel:
	push(cx);
	_cmp(es.byte(bx+4), 255);
	if (flags.z())
		goto notareelid;
	push(es);
	push(bx);
	push(ax);
	ax = es.word(bx+0);
	data.word(kReelpointer) = ax;
	getreelstart();
	_cmp(es.word(si+2), 0x0ffff);
	if (!flags.z())
		goto notblankpers;
	_add(si, 5);
notblankpers:
	cx = es.word(si+2);
	ax = es.word(si+0);
	push(cx);
	getreelframeax();
	cx = pop();
	_add(cl, es.byte(bx+4));
	_add(ch, es.byte(bx+5));
	dx = cx;
	_add(dl, es.byte(bx+0));
	_add(dh, es.byte(bx+1));
	ax = pop();
	bx = pop();
	es = pop();
	_cmp(al, cl);
	if (flags.c())
		goto notareelid;
	_cmp(ah, ch);
	if (flags.c())
		goto notareelid;
	_cmp(al, dl);
	if (!flags.c())
		goto notareelid;
	_cmp(ah, dh);
	if (!flags.c())
		goto notareelid;
	cx = pop();
	ax = es.word(bx+2);
	data.word(kPersondata) = ax;
	al = es.byte(bx+4);
	ah = 5;
	obname();
	al = 0;
	_cmp(al, 1);
	return;
notareelid:
	cx = pop();
	_add(bx, 5);
	_dec(cx);
	if (!flags.z())
		goto identifyreel;
}

void DreamGenContext::checkifset() {
	STACK_CHECK;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32))+(127*5);
	cx = 127;
identifyset:
	_cmp(es.byte(bx+4), 255);
	if (flags.z())
		goto notasetid;
	_cmp(al, es.byte(bx));
	if (flags.c())
		goto notasetid;
	_cmp(al, es.byte(bx+2));
	if (!flags.c())
		goto notasetid;
	_cmp(ah, es.byte(bx+1));
	if (flags.c())
		goto notasetid;
	_cmp(ah, es.byte(bx+3));
	if (!flags.c())
		goto notasetid;
	pixelcheckset();
	if (flags.z())
		goto notasetid;
	isitdescribed();
	if (flags.z())
		goto notasetid;
	al = es.byte(bx+4);
	ah = 1;
	obname();
	al = 0;
	_cmp(al, 1);
	return;
notasetid:
	_sub(bx, 5);
	_dec(cx);
	_cmp(cx, -1);
	if (!flags.z())
		goto identifyset;
}

void DreamGenContext::checkifex() {
	STACK_CHECK;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5))+(99*5);
	cx = 99;
identifyex:
	_cmp(es.byte(bx+4), 255);
	if (flags.z())
		goto notanexid;
	_cmp(al, es.byte(bx));
	if (flags.c())
		goto notanexid;
	_cmp(al, es.byte(bx+2));
	if (!flags.c())
		goto notanexid;
	_cmp(ah, es.byte(bx+1));
	if (flags.c())
		goto notanexid;
	_cmp(ah, es.byte(bx+3));
	if (!flags.c())
		goto notanexid;
	al = es.byte(bx+4);
	ah = 4;
	obname();
	al = 1;
	_cmp(al, 0);
	return;
notanexid:
	_sub(bx, 5);
	_dec(cx);
	_cmp(cx, -1);
	if (!flags.z())
		goto identifyex;
}

void DreamGenContext::checkiffree() {
	STACK_CHECK;
	es = data.word(kBuffers);
	bx = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5))+(79*5);
	cx = 79;
identifyfree:
	_cmp(es.byte(bx+4), 255);
	if (flags.z())
		goto notafreeid;
	_cmp(al, es.byte(bx));
	if (flags.c())
		goto notafreeid;
	_cmp(al, es.byte(bx+2));
	if (!flags.c())
		goto notafreeid;
	_cmp(ah, es.byte(bx+1));
	if (flags.c())
		goto notafreeid;
	_cmp(ah, es.byte(bx+3));
	if (!flags.c())
		goto notafreeid;
	al = es.byte(bx+4);
	ah = 2;
	obname();
	al = 0;
	_cmp(al, 1);
	return;
notafreeid:
	_sub(bx, 5);
	_dec(cx);
	_cmp(cx, -1);
	if (!flags.z())
		goto identifyfree;
}

void DreamGenContext::isitdescribed() {
	STACK_CHECK;
	push(ax);
	push(cx);
	push(es);
	push(bx);
	al = es.byte(bx+4);
	ah = 0;
	_add(ax, ax);
	bx = ax;
	es = data.word(kSetdesc);
	_add(bx, (0));
	ax = es.word(bx);
	_add(ax, (0+(130*2)));
	bx = ax;
	dl = es.byte(bx);
	bx = pop();
	es = pop();
	cx = pop();
	ax = pop();
	_cmp(dl, 0);
}

void DreamGenContext::findpathofpoint() {
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

void DreamGenContext::findfirstpath() {
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

void DreamGenContext::turnpathon() {
	STACK_CHECK;
	push(ax);
	push(ax);
	cl = 255;
	ch = data.byte(kRoomnum);
	_add(ch, 100);
	findormake();
	ax = pop();
	getroomspaths();
	ax = pop();
	_cmp(al, 255);
	if (flags.z())
		return /* (nopathon) */;
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	_add(bx, ax);
	al = 255;
	es.byte(bx+6) = al;
}

void DreamGenContext::turnpathoff() {
	STACK_CHECK;
	push(ax);
	push(ax);
	cl = 0;
	ch = data.byte(kRoomnum);
	_add(ch, 100);
	findormake();
	ax = pop();
	getroomspaths();
	ax = pop();
	_cmp(al, 255);
	if (flags.z())
		return /* (nopathoff) */;
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	_add(bx, ax);
	al = 0;
	es.byte(bx+6) = al;
}

void DreamGenContext::turnanypathon() {
	STACK_CHECK;
	push(ax);
	push(ax);
	cl = 255;
	ch = ah;
	_add(ch, 100);
	findormake();
	ax = pop();
	al = ah;
	ah = 0;
	cx = 144;
	_mul(cx);
	es = data.word(kReels);
	bx = (0);
	_add(bx, ax);
	ax = pop();
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	_add(bx, ax);
	al = 255;
	es.byte(bx+6) = al;
}

void DreamGenContext::turnanypathoff() {
	STACK_CHECK;
	push(ax);
	push(ax);
	cl = 0;
	ch = ah;
	_add(ch, 100);
	findormake();
	ax = pop();
	al = ah;
	ah = 0;
	cx = 144;
	_mul(cx);
	es = data.word(kReels);
	bx = (0);
	_add(bx, ax);
	ax = pop();
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	_add(bx, ax);
	al = 0;
	es.byte(bx+6) = al;
}

void DreamGenContext::checkifpathison() {
	STACK_CHECK;
	push(ax);
	getroomspaths();
	ax = pop();
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	_add(bx, ax);
	al = es.byte(bx+6);
	_cmp(al, 255);
}

void DreamGenContext::afternewroom() {
	STACK_CHECK;
	_cmp(data.byte(kNowinnewroom), 0);
	if (flags.z())
		return /* (notnew) */;
	data.word(kTimecount) = 0;
	createpanel();
	data.byte(kCommandtype) = 0;
	findroominloc();
	_cmp(data.byte(kRyanon), 1);
	if (flags.z())
		goto ryansoff;
	al = data.byte(kRyanx);
	_add(al, 12);
	ah = data.byte(kRyany);
	_add(ah, 12);
	findpathofpoint();
	data.byte(kManspath) = dl;
	findxyfrompath();
	data.byte(kResetmanxy) = 1;
ryansoff:
	data.byte(kNewobs) = 1;
	drawfloor();
	data.word(kLookcounter) = 160;
	data.byte(kNowinnewroom) = 0;
	showicon();
	spriteupdate();
	printsprites();
	undertextline();
	reelsonscreen();
	mainscreen();
	getunderzoom();
	zoom();
	worktoscreenm();
	walkintoroom();
	reminders();
	atmospheres();
}

void DreamGenContext::atmospheres() {
	STACK_CHECK;
	cl = data.byte(kMapx);
	ch = data.byte(kMapy);
	bx = 5246;
nextatmos:
	al = cs.byte(bx);
	_cmp(al, 255);
	if (flags.z())
		goto nomoreatmos;
	_cmp(al, data.byte(kReallocation));
	if (!flags.z())
		goto wrongatmos;
	ax = cs.word(bx+1);
	_cmp(ax, cx);
	if (!flags.z())
		goto wrongatmos;
	ax = cs.word(bx+3);
	_cmp(al, data.byte(kCh0playing));
	if (flags.z())
		goto playingalready;
	_cmp(data.byte(kLocation), 45);
	if (!flags.z())
		goto notweb;
	_cmp(data.word(kReeltowatch), 45);
	if (flags.z())
		goto wrongatmos;
notweb:
	playchannel0();
	_cmp(data.byte(kReallocation), 2);
	_cmp(data.byte(kMapy), 0);
	if (flags.z())
		goto fullvol;
	if (!flags.z())
		goto notlouisvol;
	_cmp(data.byte(kMapy), 10);
	if (!flags.z())
		goto notlouisvol;
	_cmp(data.byte(kMapx), 22);
	if (!flags.z())
		goto notlouisvol;
	data.byte(kVolume) = 5;
notlouisvol:
	_cmp(data.byte(kReallocation), 14);
	if (!flags.z())
		goto notmad1;
	_cmp(data.byte(kMapx), 33);
	if (flags.z())
		goto ismad2;
	_cmp(data.byte(kMapx), 22);
	if (!flags.z())
		goto notmad1;
	data.byte(kVolume) = 5;
	return;
ismad2:
	data.byte(kVolume) = 0;
	return;
notmad1:
playingalready:
	_cmp(data.byte(kReallocation), 2);
	if (!flags.z())
		return /* (notlouisvol2) */;
	_cmp(data.byte(kMapx), 22);
	if (flags.z())
		goto louisvol;
	_cmp(data.byte(kMapx), 11);
	if (!flags.z())
		return /* (notlouisvol2) */;
fullvol:
	data.byte(kVolume) = 0;
	return;
louisvol:
	data.byte(kVolume) = 5;
	return;
wrongatmos:
	_add(bx, 5);
	goto nextatmos;
nomoreatmos:
	cancelch0();
}

void DreamGenContext::walkintoroom() {
	STACK_CHECK;
	_cmp(data.byte(kLocation), 14);
	if (!flags.z())
		return /* (notlair) */;
	_cmp(data.byte(kMapx), 22);
	if (!flags.z())
		return /* (notlair) */;
	data.byte(kDestination) = 1;
	data.byte(kFinaldest) = 1;
	autosetwalk();
}

void DreamGenContext::afterintroroom() {
	STACK_CHECK;
	_cmp(data.byte(kNowinnewroom), 0);
	if (flags.z())
		return /* (notnewintro) */;
	clearwork();
	findroominloc();
	data.byte(kNewobs) = 1;
	drawfloor();
	reelsonscreen();
	spriteupdate();
	printsprites();
	worktoscreen();
	data.byte(kNowinnewroom) = 0;
}

void DreamGenContext::obname() {
	STACK_CHECK;
	_cmp(data.byte(kReasseschanges), 0);
	if (flags.z())
		goto notnewpath;
	data.byte(kReasseschanges) = 0;
	goto diff;
notnewpath:
	_cmp(ah, data.byte(kCommandtype));
	if (flags.z())
		goto notdiffob;
	goto diff;
notdiffob:
	_cmp(al, data.byte(kCommand));
	if (!flags.z())
		goto diff;
	_cmp(data.byte(kWalkandexam), 1);
	if (flags.z())
		goto walkandexamine;
	_cmp(data.word(kMousebutton), 0);
	if (flags.z())
		return /* (noobselect) */;
	_cmp(data.byte(kCommandtype), 3);
	if (!flags.z())
		goto isntblock;
	_cmp(data.byte(kLastflag), 2);
	if (flags.c())
		return /* (noobselect) */;
isntblock:
	bl = data.byte(kManspath);
	_cmp(bl, data.byte(kPointerspath));
	if (!flags.z())
		goto wantstowalk;
	_cmp(data.byte(kCommandtype), 3);
	if (flags.z())
		goto wantstowalk;
	finishedwalking();
	if (!flags.z())
		return /* (noobselect) */;
	_cmp(data.byte(kCommandtype), 5);
	if (flags.z())
		goto wantstotalk;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		return /* (noobselect) */;
	examineob();
	return;
wantstotalk:
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		return /* (noobselect) */;
	talk();
	return;
walkandexamine:
	finishedwalking();
	if (!flags.z())
		return /* (noobselect) */;
	al = data.byte(kWalkexamtype);
	data.byte(kCommandtype) = al;
	al = data.byte(kWalkexamnum);
	data.byte(kCommand) = al;
	data.byte(kWalkandexam) = 0;
	_cmp(data.byte(kCommandtype), 5);
	if (flags.z())
		return /* (noobselect) */;
	examineob();
	return;
wantstowalk:
	setwalk();
	data.byte(kReasseschanges) = 1;
	return;
diff:
	data.byte(kCommand) = al;
	data.byte(kCommandtype) = ah;
	_cmp(data.byte(kLinepointer), 254);
	if (!flags.z())
		goto middleofwalk;
	_cmp(data.word(kWatchingtime), 0);
	if (!flags.z())
		goto middleofwalk;
	al = data.byte(kFacing);
	_cmp(al, data.byte(kTurntoface));
	if (!flags.z())
		goto middleofwalk;
	_cmp(data.byte(kCommandtype), 3);
	if (!flags.z())
		goto notblock;
	bl = data.byte(kManspath);
	_cmp(bl, data.byte(kPointerspath));
	if (!flags.z())
		goto dontcheck;
	cl = data.byte(kRyanx);
	_add(cl, 12);
	ch = data.byte(kRyany);
	_add(ch, 12);
	checkone();
	_cmp(cl, 2);
	if (flags.c())
		goto isblock;
dontcheck:
	getflagunderp();
	_cmp(data.byte(kLastflag), 2);
	if (flags.c())
		goto isblock;
	_cmp(data.byte(kLastflag), 128);
	if (!flags.c())
		goto isblock;
	goto toofaraway;
notblock:
	bl = data.byte(kManspath);
	_cmp(bl, data.byte(kPointerspath));
	if (!flags.z())
		goto toofaraway;
	_cmp(data.byte(kCommandtype), 3);
	if (flags.z())
		goto isblock;
	_cmp(data.byte(kCommandtype), 5);
	if (flags.z())
		goto isaperson;
	examineobtext();
	return;
middleofwalk:
	blocknametext();
	return;
isblock:
	blocknametext();
	return;
isaperson:
	personnametext();
	return;
toofaraway:
	walktotext();
}

void DreamGenContext::finishedwalking() {
	STACK_CHECK;
	_cmp(data.byte(kLinepointer), 254);
	if (!flags.z())
		return /* (iswalking) */;
	al = data.byte(kFacing);
	_cmp(al, data.byte(kTurntoface));
}

void DreamGenContext::examineobtext() {
	STACK_CHECK;
	bl = data.byte(kCommand);
	bh = data.byte(kCommandtype);
	al = 1;
	commandwithob();
}

void DreamGenContext::commandwithob() {
	STACK_CHECK;
	push(ax);
	push(ax);
	push(bx);
	push(cx);
	push(dx);
	push(es);
	push(ds);
	push(si);
	push(di);
	deltextline();
	di = pop();
	si = pop();
	ds = pop();
	es = pop();
	dx = pop();
	cx = pop();
	bx = pop();
	ax = pop();
	push(bx);
	ah = 0;
	_add(ax, ax);
	bx = ax;
	es = data.word(kCommandtext);
	ax = es.word(bx);
	_add(ax, (66*2));
	si = ax;
	di = data.word(kTextaddressx);
	bx = data.word(kTextaddressy);
	dl = data.byte(kTextlen);
	al = 0;
	ah = 0;
	printdirect();
	ax = pop();
	di = 5847;
	copyname();
	ax = pop();
	di = data.word(kLastxpos);
	_cmp(al, 0);
	if (flags.z())
		goto noadd;
	_add(di, 5);
noadd:
	bx = data.word(kTextaddressy);
	es = cs;
	si = 5847;
	dl = data.byte(kTextlen);
	al = 0;
	ah = 0;
	printdirect();
	data.byte(kNewtextline) = 1;
}

void DreamGenContext::commandonly() {
	STACK_CHECK;
	push(ax);
	push(bx);
	push(cx);
	push(dx);
	push(es);
	push(ds);
	push(si);
	push(di);
	deltextline();
	di = pop();
	si = pop();
	ds = pop();
	es = pop();
	dx = pop();
	cx = pop();
	bx = pop();
	ax = pop();
	ah = 0;
	_add(ax, ax);
	bx = ax;
	es = data.word(kCommandtext);
	ax = es.word(bx);
	_add(ax, (66*2));
	si = ax;
	di = data.word(kTextaddressx);
	bx = data.word(kTextaddressy);
	dl = data.byte(kTextlen);
	al = 0;
	ah = 0;
	printdirect();
	data.byte(kNewtextline) = 1;
}

void DreamGenContext::printmessage() {
	STACK_CHECK;
	push(dx);
	push(bx);
	push(di);
	ah = 0;
	_add(ax, ax);
	bx = ax;
	es = data.word(kCommandtext);
	ax = es.word(bx);
	_add(ax, (66*2));
	si = ax;
	di = pop();
	bx = pop();
	dx = pop();
	al = 0;
	ah = 0;
	printdirect();
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
	findnextcolon();
	ax = pop();
	_dec(ah);
	if (!flags.z())
		goto searchmess;
	di = pop();
	bx = pop();
	dx = pop();
	al = 0;
	ah = 0;
	printdirect();
}

void DreamGenContext::blocknametext() {
	STACK_CHECK;
	bl = data.byte(kCommand);
	bh = data.byte(kCommandtype);
	al = 0;
	commandwithob();
}

void DreamGenContext::personnametext() {
	STACK_CHECK;
	bl = data.byte(kCommand);
	_and(bl, 127);
	bh = data.byte(kCommandtype);
	al = 2;
	commandwithob();
}

void DreamGenContext::walktotext() {
	STACK_CHECK;
	bl = data.byte(kCommand);
	bh = data.byte(kCommandtype);
	al = 3;
	commandwithob();
}

void DreamGenContext::getflagunderp() {
	STACK_CHECK;
	cx = data.word(kMousex);
	_sub(cx, data.word(kMapadx));
	ax = data.word(kMousey);
	_sub(ax, data.word(kMapady));
	ch = al;
	checkone();
	data.byte(kLastflag) = cl;
	data.byte(kLastflagex) = ch;
}

void DreamGenContext::setwalk() {
	STACK_CHECK;
	_cmp(data.byte(kLinepointer), 254);
	if (!flags.z())
		goto alreadywalking;
	al = data.byte(kPointerspath);
	_cmp(al, data.byte(kManspath));
	if (flags.z())
		goto cantwalk2;
	_cmp(data.byte(kWatchmode), 1);
	if (flags.z())
		goto holdingreel;
	_cmp(data.byte(kWatchmode), 2);
	if (flags.z())
		return /* (cantwalk) */;
	data.byte(kDestination) = al;
	data.byte(kFinaldest) = al;
	_cmp(data.word(kMousebutton), 2);
	if (!flags.z())
		goto notwalkandexam;
	_cmp(data.byte(kCommandtype), 3);
	if (flags.z())
		goto notwalkandexam;
	data.byte(kWalkandexam) = 1;
	al = data.byte(kCommandtype);
	data.byte(kWalkexamtype) = al;
	al = data.byte(kCommand);
	data.byte(kWalkexamnum) = al;
notwalkandexam:
	autosetwalk();
	return;
cantwalk2:
	facerightway();
	return;
alreadywalking:
	al = data.byte(kPointerspath);
	data.byte(kFinaldest) = al;
	return;
holdingreel:
	data.byte(kDestafterhold) = al;
	data.byte(kWatchmode) = 2;
}

void DreamGenContext::autosetwalk() {
	STACK_CHECK;
	al = data.byte(kManspath);
	_cmp(data.byte(kFinaldest), al);
	if (!flags.z())
		goto notsamealready;
	return;
notsamealready:
	getroomspaths();
	checkdest();
	push(bx);
	al = data.byte(kManspath);
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	_add(bx, ax);
	al = es.byte(bx);
	ah = 0;
	_sub(ax, 12);
	data.word(kLinestartx) = ax;
	al = es.byte(bx+1);
	ah = 0;
	_sub(ax, 12);
	data.word(kLinestarty) = ax;
	bx = pop();
	al = data.byte(kDestination);
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	_add(bx, ax);
	al = es.byte(bx);
	ah = 0;
	_sub(ax, 12);
	data.word(kLineendx) = ax;
	al = es.byte(bx+1);
	ah = 0;
	_sub(ax, 12);
	data.word(kLineendy) = ax;
	bresenhams();
	_cmp(data.byte(kLinedirection), 0);
	if (flags.z())
		goto normalline;
	al = data.byte(kLinelength);
	_dec(al);
	data.byte(kLinepointer) = al;
	data.byte(kLinedirection) = 1;
	return;
normalline:
	data.byte(kLinepointer) = 0;
}

void DreamGenContext::checkdest() {
	STACK_CHECK;
	push(bx);
	_add(bx, 12*8);
	ah = data.byte(kManspath);
	cl = 4;
	_shl(ah, cl);
	al = data.byte(kDestination);
	cl = 24;
	ch = data.byte(kDestination);
checkdestloop:
	dh = es.byte(bx);
	_and(dh, 0xf0);
	dl = es.byte(bx);
	_and(dl, 0xf);
	_cmp(ax, dx);
	if (!flags.z())
		goto nextcheck;
	al = es.byte(bx+1);
	_and(al, 15);
	data.byte(kDestination) = al;
	bx = pop();
	return;
nextcheck:
	dl = es.byte(bx);
	_and(dl, 0xf0);
	_shr(dl, 1);
	_shr(dl, 1);
	_shr(dl, 1);
	_shr(dl, 1);
	dh = es.byte(bx);
	_and(dh, 0xf);
	_shl(dh, 1);
	_shl(dh, 1);
	_shl(dh, 1);
	_shl(dh, 1);
	_cmp(ax, dx);
	if (!flags.z())
		goto nextcheck2;
	ch = es.byte(bx+1);
	_and(ch, 15);
nextcheck2:
	_add(bx, 2);
	_dec(cl);
	if (!flags.z())
		goto checkdestloop;
	data.byte(kDestination) = ch;
	bx = pop();
}

void DreamGenContext::bresenhams() {
	STACK_CHECK;
	workoutframes();
	dx = data;
	es = dx;
	di = 8173;
	si = 1;
	data.byte(kLinedirection) = 0;
	cx = data.word(kLineendx);
	_sub(cx, data.word(kLinestartx));
	if (flags.z())
		goto vertline;
	if (!flags.s())
		goto line1;
	_neg(cx);
	bx = data.word(kLineendx);
	_xchg(bx, data.word(kLinestartx));
	data.word(kLineendx) = bx;
	bx = data.word(kLineendy);
	_xchg(bx, data.word(kLinestarty));
	data.word(kLineendy) = bx;
	data.byte(kLinedirection) = 1;
line1:
	bx = data.word(kLineendy);
	_sub(bx, data.word(kLinestarty));
	if (flags.z())
		goto horizline;
	if (!flags.s())
		goto line3;
	_neg(bx);
	_neg(si);
line3:
	push(si);
	data.byte(kLineroutine) = 0;
	_cmp(bx, cx);
	if (flags.le())
		goto line4;
	data.byte(kLineroutine) = 1;
	_xchg(bx, cx);
line4:
	_shl(bx, 1);
	data.word(kIncrement1) = bx;
	_sub(bx, cx);
	si = bx;
	_sub(bx, cx);
	data.word(kIncrement2) = bx;
	ax = data.word(kLinestartx);
	bx = data.word(kLinestarty);
	ah = bl;
	_inc(cx);
	bx = pop();
	_cmp(data.byte(kLineroutine), 1);
	if (flags.z())
		goto hislope;
	goto loslope;
vertline:
	ax = data.word(kLinestarty);
	bx = data.word(kLineendy);
	cx = bx;
	_sub(cx, ax);
	if (!flags.l())
		goto line31;
	_neg(cx);
	ax = bx;
	data.byte(kLinedirection) = 1;
line31:
	_inc(cx);
	bx = data.word(kLinestartx);
	_xchg(ax, bx);
	ah = bl;
	bx = si;
line32:
	_stosw();
	_add(ah, bl);
	if (--cx)
		goto line32;
	goto lineexit;
horizline:
	ax = data.word(kLinestartx);
	bx = data.word(kLinestarty);
	ah = bl;
	_inc(cx);
horizloop:
	_stosw();
	_inc(al);
	if (--cx)
		goto horizloop;
	goto lineexit;
loslope:
loloop:
	_stosw();
	_inc(al);
	_or(si, si);
	if (!flags.s())
		goto line12;
	_add(si, data.word(kIncrement1));
	if (--cx)
		goto loloop;
	goto lineexit;
line12:
	_add(si, data.word(kIncrement2));
	_add(ah, bl);
	if (--cx)
		goto loloop;
	goto lineexit;
hislope:
hiloop:
	_stosw();
	_add(ah, bl);
	_or(si, si);
	if (!flags.s())
		goto line23;
	_add(si, data.word(kIncrement1));
	if (--cx)
		goto hiloop;
	goto lineexit;
line23:
	_add(si, data.word(kIncrement2));
	_inc(al);
	if (--cx)
		goto hiloop;
lineexit:
	_sub(di, 8173);
	ax = di;
	_shr(ax, 1);
	data.byte(kLinelength) = al;
}

void DreamGenContext::workoutframes() {
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

void DreamGenContext::getroomspaths() {
	STACK_CHECK;
	al = data.byte(kRoomnum);
	ah = 0;
	cx = 144;
	_mul(cx);
	es = data.word(kReels);
	bx = (0);
	_add(bx, ax);
}

void DreamGenContext::copyname() {
	STACK_CHECK;
	push(di);
	findobname();
	di = pop();
	es = cs;
	cx = 28;
make:
	_lodsb();
	_cmp(al, ':');
	if (flags.z())
		goto finishmakename;
	_cmp(al, 0);
	if (flags.z())
		goto finishmakename;
	_stosb();
	if (--cx)
		goto make;
finishmakename:
	_inc(cx);
	al = 0;
	_stosb();
	return;
	al = 255;
	_stosb(cx, true);
}

void DreamGenContext::findobname() {
	STACK_CHECK;
	push(ax);
	ah = 0;
	_add(ax, ax);
	bx = ax;
	ax = pop();
	_cmp(ah, 5);
	if (!flags.z())
		goto notpersonname;
	push(ax);
	_and(al, 127);
	ah = 0;
	bx = 64*2;
	_mul(bx);
	si = ax;
	ds = data.word(kPeople);
	_add(si, (0+24));
	cx = (0+24+(1026*2));
	ax = ds.word(si);
	_add(ax, cx);
	si = ax;
	ax = pop();
	return;
notpersonname:
	_cmp(ah, 4);
	if (!flags.z())
		goto notextraname;
	ds = data.word(kExtras);
	_add(bx, (0+2080+30000+(16*114)));
	ax = ds.word(bx);
	_add(ax, (0+2080+30000+(16*114)+((114+2)*2)));
	si = ax;
	return;
notextraname:
	_cmp(ah, 2);
	if (!flags.z())
		goto notfreename;
	ds = data.word(kFreedesc);
	_add(bx, (0));
	ax = ds.word(bx);
	_add(ax, (0+(82*2)));
	si = ax;
	return;
notfreename:
	_cmp(ah, 1);
	if (!flags.z())
		goto notsetname;
	ds = data.word(kSetdesc);
	_add(bx, (0));
	ax = ds.word(bx);
	_add(ax, (0+(130*2)));
	si = ax;
	return;
notsetname:
	ds = data.word(kBlockdesc);
	_add(bx, (0));
	ax = ds.word(bx);
	_add(ax, (0+(98*2)));
	si = ax;
}

void DreamGenContext::showicon() {
	STACK_CHECK;
	_cmp(data.byte(kReallocation), 50);
	if (!flags.c())
		goto isdream1;
	showpanel();
	showman();
	roomname();
	panelicons1();
	zoomicon();
	return;
isdream1:
	ds = data.word(kTempsprites);
	di = 72;
	bx = 2;
	al = 45;
	ah = 0;
	showframe();
	ds = data.word(kTempsprites);
	di = 72+47;
	bx = 2;
	al = 46;
	ah = 0;
	showframe();
	ds = data.word(kTempsprites);
	di = 69-10;
	bx = 21;
	al = 49;
	ah = 0;
	showframe();
	ds = data.word(kTempsprites);
	di = 160+88;
	bx = 2;
	al = 45;
	ah = 4;
	showframe();
	ds = data.word(kTempsprites);
	di = 160+43;
	bx = 2;
	al = 46;
	ah = 4;
	showframe();
	ds = data.word(kTempsprites);
	di = 160+101;
	bx = 21;
	al = 49;
	ah = 4;
	showframe();
	middlepanel();
}

void DreamGenContext::middlepanel() {
	STACK_CHECK;
	ds = data.word(kTempsprites);
	di = 72+47+20;
	bx = 0;
	al = 48;
	ah = 0;
	showframe();
	ds = data.word(kTempsprites);
	di = 72+19;
	bx = 21;
	al = 47;
	ah = 0;
	showframe();
	ds = data.word(kTempsprites);
	di = 160+23;
	bx = 0;
	al = 48;
	ah = 4;
	showframe();
	ds = data.word(kTempsprites);
	di = 160+71;
	bx = 21;
	al = 47;
	ah = 4;
	showframe();
}

void DreamGenContext::showman() {
	STACK_CHECK;
	ds = data.word(kIcons1);
	di = 0;
	bx = 0;
	al = 0;
	ah = 0;
	showframe();
	ds = data.word(kIcons1);
	di = 0;
	bx = 114;
	al = 1;
	ah = 0;
	showframe();
	_cmp(data.byte(kShadeson), 0);
	if (flags.z())
		return /* (notverycool) */;
	ds = data.word(kIcons1);
	di = 28;
	bx = 25;
	al = 2;
	ah = 0;
	showframe();
}

void DreamGenContext::showpanel() {
	STACK_CHECK;
	ds = data.word(kIcons1);
	di = 72;
	bx = 0;
	al = 19;
	ah = 0;
	showframe();
	ds = data.word(kIcons1);
	di = 192;
	bx = 0;
	al = 19;
	ah = 0;
	showframe();
}

void DreamGenContext::roomname() {
	STACK_CHECK;
	di = 88;
	bx = 18;
	al = 53;
	dl = 240;
	printmessage();
	bl = data.byte(kRoomnum);
	_cmp(bl, 32);
	if (flags.c())
		goto notover32;
	_sub(bl, 32);
notover32:
	bh = 0;
	_add(bx, bx);
	es = data.word(kRoomdesc);
	_add(bx, (0));
	ax = es.word(bx);
	_add(ax, (0+(38*2)));
	si = ax;
	data.word(kLinespacing) = 7;
	di = 88;
	bx = 25;
	dl = 120;
	_cmp(data.byte(kWatchon), 1);
	if (flags.z())
		goto gotpl;
	dl = 160;
gotpl:
	al = 0;
	ah = 0;
	printdirect();
	data.word(kLinespacing) = 10;
	usecharset1();
}

void DreamGenContext::usecharset1() {
	STACK_CHECK;
	ax = data.word(kCharset1);
	data.word(kCurrentset) = ax;
}

void DreamGenContext::usetempcharset() {
	STACK_CHECK;
	ax = data.word(kTempcharset);
	data.word(kCurrentset) = ax;
}

void DreamGenContext::showexit() {
	STACK_CHECK;
	ds = data.word(kIcons1);
	di = 274;
	bx = 154;
	al = 11;
	ah = 0;
	showframe();
}

void DreamGenContext::panelicons1() {
	STACK_CHECK;
	di = 0;
	_cmp(data.byte(kWatchon), 1);
	if (flags.z())
		goto watchison;
	di = 48;
watchison:
	push(di);
	ds = data.word(kIcons2);
	_add(di, 204);
	bx = 4;
	al = 2;
	ah = 0;
	showframe();
	di = pop();
	push(di);
	_cmp(data.byte(kZoomon), 1);
	if (flags.z())
		goto zoomisoff;
	ds = data.word(kIcons1);
	_add(di, 228);
	bx = 8;
	al = 5;
	ah = 0;
	showframe();
zoomisoff:
	di = pop();
	showwatch();
}

void DreamGenContext::showwatch() {
	STACK_CHECK;
	_cmp(data.byte(kWatchon), 0);
	if (flags.z())
		return /* (nowristwatch) */;
	ds = data.word(kIcons1);
	di = 250;
	bx = 1;
	al = 6;
	ah = 0;
	showframe();
	showtime();
}

void DreamGenContext::zoomicon() {
	STACK_CHECK;
	_cmp(data.byte(kZoomon), 0);
	if (flags.z())
		return /* (nozoom1) */;
	ds = data.word(kIcons1);
	di = (8);
	bx = (132)-1;
	al = 8;
	ah = 0;
	showframe();
}

void DreamGenContext::showblink() {
	STACK_CHECK;
	_cmp(data.byte(kManisoffscreen), 1);
	if (flags.z())
		return /* (finblink1) */;
	_inc(data.byte(kBlinkcount));
	_cmp(data.byte(kShadeson), 0);
	if (!flags.z())
		return /* (finblink1) */;
	_cmp(data.byte(kReallocation), 50);
	if (!flags.c())
		return /* (eyesshut) */;
	al = data.byte(kBlinkcount);
	_cmp(al, 3);
	if (!flags.z())
		return /* (finblink1) */;
	data.byte(kBlinkcount) = 0;
	al = data.byte(kBlinkframe);
	_inc(al);
	data.byte(kBlinkframe) = al;
	_cmp(al, 6);
	if (flags.c())
		goto nomorethan6;
	al = 6;
nomorethan6:
	ah = 0;
	bx = 5888;
	_add(bx, ax);
	al = cs.byte(bx);
	ds = data.word(kIcons1);
	di = 44;
	bx = 32;
	ah = 0;
	showframe();
}

void DreamGenContext::dumpblink() {
	STACK_CHECK;
	_cmp(data.byte(kShadeson), 0);
	if (!flags.z())
		return /* (nodumpeye) */;
	_cmp(data.byte(kBlinkcount), 0);
	if (!flags.z())
		return /* (nodumpeye) */;
	al = data.byte(kBlinkframe);
	_cmp(al, 6);
	if (!flags.c())
		return /* (nodumpeye) */;
	push(ds);
	di = 44;
	bx = 32;
	cl = 16;
	ch = 12;
	multidump();
	ds = pop();
}

void DreamGenContext::worktoscreenm() {
	STACK_CHECK;
	animpointer();
	readmouse();
	showpointer();
	vsync();
	worktoscreen();
	delpointer();
}

void DreamGenContext::blank() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 199);
	if (flags.z())
		return /* (alreadyblnk) */;
	data.byte(kCommandtype) = 199;
	al = 0;
	commandonly();
}

void DreamGenContext::allpointer() {
	STACK_CHECK;
	readmouse();
	showpointer();
	dumppointer();
}

void DreamGenContext::hangonp() {
	STACK_CHECK;
	push(cx);
	_add(cx, cx);
	ax = pop();
	_add(cx, ax);
	data.word(kMaintimer) = 0;
	al = data.byte(kPointerframe);
	ah = data.byte(kPickup);
	push(ax);
	data.byte(kPointermode) = 3;
	data.byte(kPickup) = 0;
	push(cx);
	data.byte(kCommandtype) = 255;
	readmouse();
	animpointer();
	showpointer();
	vsync();
	dumppointer();
	cx = pop();
hangloop:
	push(cx);
	delpointer();
	readmouse();
	animpointer();
	showpointer();
	vsync();
	dumppointer();
	cx = pop();
	ax = data.word(kMousebutton);
	_cmp(ax, 0);
	if (flags.z())
		goto notpressed;
	_cmp(ax, data.word(kOldbutton));
	if (!flags.z())
		goto getoutofit;
notpressed:
	if (--cx)
		goto hangloop;
getoutofit:
	delpointer();
	ax = pop();
	data.byte(kPointerframe) = al;
	data.byte(kPickup) = ah;
	data.byte(kPointermode) = 0;
}

void DreamGenContext::hangonw() {
	STACK_CHECK;
hangloopw:
	push(cx);
	delpointer();
	readmouse();
	animpointer();
	showpointer();
	vsync();
	dumppointer();
	cx = pop();
	if (--cx)
		goto hangloopw;
}

void DreamGenContext::hangoncurs() {
	STACK_CHECK;
monloop1:
	push(cx);
	printcurs();
	vsync();
	delcurs();
	cx = pop();
	if (--cx)
		goto monloop1;
}

void DreamGenContext::getunderzoom() {
	STACK_CHECK;
	di = (8)+5;
	bx = (132)+4;
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
	cl = 46;
	ch = 40;
	multiget();
}

void DreamGenContext::dumpzoom() {
	STACK_CHECK;
	_cmp(data.byte(kZoomon), 1);
	if (!flags.z())
		return /* (notzoomon) */;
	di = (8)+5;
	bx = (132)+4;
	cl = 46;
	ch = 40;
	multidump();
}

void DreamGenContext::putunderzoom() {
	STACK_CHECK;
	di = (8)+5;
	bx = (132)+4;
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
	cl = 46;
	ch = 40;
	multiput();
}

void DreamGenContext::crosshair() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 3);
	if (flags.z())
		goto nocross;
	_cmp(data.byte(kCommandtype), 10);
	if (!flags.c())
		goto nocross;
	es = data.word(kWorkspace);
	ds = data.word(kIcons1);
	di = (8)+24;
	bx = (132)+19;
	al = 9;
	ah = 0;
	showframe();
	return;
nocross:
	es = data.word(kWorkspace);
	ds = data.word(kIcons1);
	di = (8)+24;
	bx = (132)+19;
	al = 29;
	ah = 0;
	showframe();
}

void DreamGenContext::showpointer() {
	STACK_CHECK;
	showblink();
	di = data.word(kMousex);
	data.word(kOldpointerx) = di;
	bx = data.word(kMousey);
	data.word(kOldpointery) = bx;
	_cmp(data.byte(kPickup), 1);
	if (flags.z())
		goto itsanobject;
	push(bx);
	push(di);
	ds = data.word(kIcons1);
	al = data.byte(kPointerframe);
	_add(al, 20);
	ah = 0;
	_add(ax, ax);
	si = ax;
	_add(ax, ax);
	_add(si, ax);
	cx = ds.word(si);
	_cmp(cl, 12);
	if (!flags.c())
		goto notsmallx;
	cl = 12;
notsmallx:
	_cmp(ch, 12);
	if (!flags.c())
		goto notsmally;
	ch = 12;
notsmally:
	data.byte(kPointerxs) = cl;
	data.byte(kPointerys) = ch;
	push(ds);
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60);
	multiget();
	ds = pop();
	di = pop();
	bx = pop();
	push(di);
	push(bx);
	al = data.byte(kPointerframe);
	_add(al, 20);
	ah = 0;
	showframe();
	bx = pop();
	di = pop();
	return;
itsanobject:
	al = data.byte(kItemframe);
	ds = data.word(kExtras);
	_cmp(data.byte(kObjecttype), 4);
	if (flags.z())
		goto itsfrominv;
	ds = data.word(kFreeframes);
itsfrominv:
	cl = al;
	_add(al, al);
	_add(al, cl);
	_inc(al);
	ah = 0;
	push(ax);
	_add(ax, ax);
	si = ax;
	_add(ax, ax);
	_add(si, ax);
	ax = 2080;
	cx = ds.word(si);
	_cmp(cl, 12);
	if (!flags.c())
		goto notsmallx2;
	cl = 12;
notsmallx2:
	_cmp(ch, 12);
	if (!flags.c())
		goto notsmally2;
	ch = 12;
notsmally2:
	data.byte(kPointerxs) = cl;
	data.byte(kPointerys) = ch;
	ax = pop();
	push(di);
	push(bx);
	push(ax);
	push(bx);
	push(di);
	push(ds);
	al = cl;
	ah = 0;
	_shr(ax, 1);
	_sub(data.word(kOldpointerx), ax);
	_sub(di, ax);
	al = ch;
	_shr(ax, 1);
	_sub(data.word(kOldpointery), ax);
	_sub(bx, ax);
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60);
	multiget();
	ds = pop();
	di = pop();
	bx = pop();
	ax = pop();
	ah = 128;
	showframe();
	bx = pop();
	di = pop();
	ds = data.word(kIcons1);
	al = 3;
	ah = 128;
	showframe();
}

void DreamGenContext::delpointer() {
	STACK_CHECK;
	ax = data.word(kOldpointerx);
	_cmp(ax, 0x0ffff);
	if (flags.z())
		return /* (nevershown) */;
	data.word(kDelherex) = ax;
	ax = data.word(kOldpointery);
	data.word(kDelherey) = ax;
	cl = data.byte(kPointerxs);
	data.byte(kDelxs) = cl;
	ch = data.byte(kPointerys);
	data.byte(kDelys) = ch;
	ds = data.word(kBuffers);
	si = (0+(228*13)+32+60);
	di = data.word(kDelherex);
	bx = data.word(kDelherey);
	multiput();
}

void DreamGenContext::dumppointer() {
	STACK_CHECK;
	dumpblink();
	cl = data.byte(kDelxs);
	ch = data.byte(kDelys);
	di = data.word(kDelherex);
	bx = data.word(kDelherey);
	multidump();
	bx = data.word(kOldpointery);
	di = data.word(kOldpointerx);
	_cmp(di, data.word(kDelherex));
	if (!flags.z())
		goto difffound;
	_cmp(bx, data.word(kDelherey));
	if (flags.z())
		return /* (notboth) */;
difffound:
	cl = data.byte(kPointerxs);
	ch = data.byte(kPointerys);
	multidump();
}

void DreamGenContext::undertextline() {
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
	multiget();
}

void DreamGenContext::deltextline() {
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
	multiput();
}

void DreamGenContext::animpointer() {
	STACK_CHECK;
	_cmp(data.byte(kPointermode), 2);
	if (flags.z())
		goto combathand;
	_cmp(data.byte(kPointermode), 3);
	if (flags.z())
		goto mousehand;
	_cmp(data.word(kWatchingtime), 0);
	if (flags.z())
		goto notwatchpoint;
	data.byte(kPointerframe) = 11;
	return;
notwatchpoint:
	data.byte(kPointerframe) = 0;
	_cmp(data.byte(kInmaparea), 0);
	if (flags.z())
		return /* (gothand) */;
	_cmp(data.byte(kPointerfirstpath), 0);
	if (flags.z())
		return /* (gothand) */;
	getflagunderp();
	_cmp(cl, 2);
	if (flags.c())
		return /* (gothand) */;
	_cmp(cl, 128);
	if (!flags.c())
		return /* (gothand) */;
	data.byte(kPointerframe) = 3;
	_test(cl, 4);
	if (!flags.z())
		return /* (gothand) */;
	data.byte(kPointerframe) = 4;
	_test(cl, 16);
	if (!flags.z())
		return /* (gothand) */;
	data.byte(kPointerframe) = 5;
	_test(cl, 2);
	if (!flags.z())
		return /* (gothand) */;
	data.byte(kPointerframe) = 6;
	_test(cl, 8);
	if (!flags.z())
		return /* (gothand) */;
	data.byte(kPointerframe) = 8;
	return;
mousehand:
	_cmp(data.byte(kPointerspeed), 0);
	if (flags.z())
		goto rightspeed3;
	_dec(data.byte(kPointerspeed));
	goto finflashmouse;
rightspeed3:
	data.byte(kPointerspeed) = 5;
	_inc(data.byte(kPointercount));
	_cmp(data.byte(kPointercount), 16);
	if (!flags.z())
		goto finflashmouse;
	data.byte(kPointercount) = 0;
finflashmouse:
	al = data.byte(kPointercount);
	ah = 0;
	bx = 5895;
	_add(bx, ax);
	al = cs.byte(bx);
	data.byte(kPointerframe) = al;
	return;
combathand:
	data.byte(kPointerframe) = 0;
	_cmp(data.byte(kReallocation), 14);
	if (!flags.z())
		return /* (notarrow) */;
	_cmp(data.byte(kCommandtype), 211);
	if (!flags.z())
		return /* (notarrow) */;
	data.byte(kPointerframe) = 5;
}

void DreamGenContext::readmouse() {
	STACK_CHECK;
	ax = data.word(kMousebutton);
	data.word(kOldbutton) = ax;
	ax = data.word(kMousex);
	data.word(kOldx) = ax;
	ax = data.word(kMousey);
	data.word(kOldy) = ax;
	mousecall();
	data.word(kMousex) = cx;
	data.word(kMousey) = dx;
	data.word(kMousebutton) = bx;
}

void DreamGenContext::readmouse1() {
	STACK_CHECK;
	ax = data.word(kMousex);
	data.word(kOldx) = ax;
	ax = data.word(kMousey);
	data.word(kOldy) = ax;
	mousecall();
	data.word(kMousex) = cx;
	data.word(kMousey) = dx;
	data.word(kMousebutton1) = bx;
}

void DreamGenContext::readmouse2() {
	STACK_CHECK;
	ax = data.word(kMousex);
	data.word(kOldx) = ax;
	ax = data.word(kMousey);
	data.word(kOldy) = ax;
	mousecall();
	data.word(kMousex) = cx;
	data.word(kMousey) = dx;
	data.word(kMousebutton2) = bx;
}

void DreamGenContext::readmouse3() {
	STACK_CHECK;
	ax = data.word(kMousex);
	data.word(kOldx) = ax;
	ax = data.word(kMousey);
	data.word(kOldy) = ax;
	mousecall();
	data.word(kMousex) = cx;
	data.word(kMousey) = dx;
	data.word(kMousebutton3) = bx;
}

void DreamGenContext::readmouse4() {
	STACK_CHECK;
	ax = data.word(kMousebutton);
	data.word(kOldbutton) = ax;
	ax = data.word(kMousex);
	data.word(kOldx) = ax;
	ax = data.word(kMousey);
	data.word(kOldy) = ax;
	mousecall();
	data.word(kMousex) = cx;
	data.word(kMousey) = dx;
	ax = data.word(kMousebutton1);
	_or(ax, data.word(kMousebutton2));
	_or(ax, data.word(kMousebutton3));
	_or(bx, ax);
	data.word(kMousebutton) = bx;
}

void DreamGenContext::readkey() {
	STACK_CHECK;
	bx = data.word(kBufferout);
	_cmp(bx, data.word(kBufferin));
	if (flags.z())
		goto nokey;
	_inc(bx);
	_and(bx, 15);
	data.word(kBufferout) = bx;
	di = 5912;
	_add(di, bx);
	al = cs.byte(di);
	data.byte(kCurrentkey) = al;
	return;
nokey:
	data.byte(kCurrentkey) = 0;
}

void DreamGenContext::randomnum1() {
	STACK_CHECK;
	push(ds);
	push(es);
	push(di);
	push(bx);
	push(cx);
	randomnumber();
	cx = pop();
	bx = pop();
	di = pop();
	es = pop();
	ds = pop();
}

void DreamGenContext::randomnum2() {
	STACK_CHECK;
	push(ds);
	push(es);
	push(di);
	push(bx);
	push(ax);
	randomnumber();
	cl = al;
	ax = pop();
	bx = pop();
	di = pop();
	es = pop();
	ds = pop();
}

void DreamGenContext::hangon() {
	STACK_CHECK;
hangonloop:
	push(cx);
	vsync();
	cx = pop();
	if (--cx)
		goto hangonloop;
}

void DreamGenContext::loadtraveltext() {
	STACK_CHECK;
	dx = 2234;
	standardload();
	data.word(kTraveltext) = ax;
}

void DreamGenContext::loadintotemp() {
	STACK_CHECK;
	ds = cs;
	standardload();
	data.word(kTempgraphics) = ax;
}

void DreamGenContext::loadintotemp2() {
	STACK_CHECK;
	ds = cs;
	standardload();
	data.word(kTempgraphics2) = ax;
}

void DreamGenContext::loadintotemp3() {
	STACK_CHECK;
	ds = cs;
	standardload();
	data.word(kTempgraphics3) = ax;
}

void DreamGenContext::loadtempcharset() {
	STACK_CHECK;
	standardload();
	data.word(kTempcharset) = ax;
}

void DreamGenContext::standardload() {
	STACK_CHECK;
	openfile();
	readheader();
	bx = es.word(di);
	push(bx);
	cl = 4;
	_shr(bx, cl);
	allocatemem();
	ds = ax;
	cx = pop();
	push(ax);
	dx = 0;
	readfromfile();
	closefile();
	ax = pop();
}

void DreamGenContext::loadtemptext() {
	STACK_CHECK;
	standardload();
	data.word(kTextfile1) = ax;
}

void DreamGenContext::loadroom() {
	STACK_CHECK;
	data.byte(kRoomloaded) = 1;
	data.word(kTimecount) = 0;
	data.word(kMaintimer) = 0;
	data.word(kMapoffsetx) = 104;
	data.word(kMapoffsety) = 38;
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	al = data.byte(kNewlocation);
	data.byte(kLocation) = al;
	getroomdata();
	startloading();
	loadroomssample();
	switchryanon();
	drawflags();
	getdimension();
}

void DreamGenContext::loadroomssample() {
	STACK_CHECK;
	al = data.byte(kRoomssample);
	_cmp(al, 255);
	if (flags.z())
		return /* (loadedalready) */;
	_cmp(al, data.byte(kCurrentsample));
	if (flags.z())
		return /* (loadedalready) */;
	data.byte(kCurrentsample) = al;
	al = data.byte(kCurrentsample);
	cl = '0';
	twodigitnum();
	di = 1896;
	_xchg(al, ah);
	cs.word(di+10) = ax;
	dx = di;
	loadsecondsample();
}

void DreamGenContext::getridofreels() {
	STACK_CHECK;
	_cmp(data.byte(kRoomloaded), 0);
	if (flags.z())
		return /* (dontgetrid) */;
	es = data.word(kReel1);
	deallocatemem();
	es = data.word(kReel2);
	deallocatemem();
	es = data.word(kReel3);
	deallocatemem();
}

void DreamGenContext::getridofall() {
	STACK_CHECK;
	es = data.word(kBackdrop);
	deallocatemem();
	es = data.word(kSetframes);
	deallocatemem();
	es = data.word(kReel1);
	deallocatemem();
	es = data.word(kReel2);
	deallocatemem();
	es = data.word(kReel3);
	deallocatemem();
	es = data.word(kReels);
	deallocatemem();
	es = data.word(kPeople);
	deallocatemem();
	es = data.word(kSetdesc);
	deallocatemem();
	es = data.word(kBlockdesc);
	deallocatemem();
	es = data.word(kRoomdesc);
	deallocatemem();
	es = data.word(kFreeframes);
	deallocatemem();
	es = data.word(kFreedesc);
	deallocatemem();
}

void DreamGenContext::restorereels() {
	STACK_CHECK;
	_cmp(data.byte(kRoomloaded), 0);
	if (flags.z())
		return /* (dontrestore) */;
	al = data.byte(kReallocation);
	getroomdata();
	dx = bx;
	openfile();
	readheader();
	dontloadseg();
	dontloadseg();
	dontloadseg();
	dontloadseg();
	allocateload();
	data.word(kReel1) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReel2) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReel3) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	closefile();
}

void DreamGenContext::restoreall() {
	STACK_CHECK;
	al = data.byte(kLocation);
	getroomdata();
	dx = bx;
	openfile();
	readheader();
	allocateload();
	ds = ax;
	data.word(kBackdrop) = ax;
	dx = (0);
	loadseg();
	ds = data.word(kWorkspace);
	dx = (0);
	cx = 132*66;
	al = 0;
	fillspace();
	loadseg();
	sortoutmap();
	allocateload();
	data.word(kSetframes) = ax;
	ds = ax;
	dx = (0);
	loadseg();
	dontloadseg();
	allocateload();
	data.word(kReel1) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReel2) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReel3) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReels) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kPeople) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kSetdesc) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kBlockdesc) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kRoomdesc) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kFreeframes) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	dontloadseg();
	allocateload();
	data.word(kFreedesc) = ax;
	ds = ax;
	dx = (0);
	loadseg();
	closefile();
	setallchanges();
}

void DreamGenContext::sortoutmap() {
	STACK_CHECK;
	push(es);
	push(di);
	ds = data.word(kWorkspace);
	si = 0;
	es = data.word(kMapdata);
	di = 0;
	cx = (60);
blimey:
	push(cx);
	push(si);
	cx = (66);
	_movsb(cx, true);
	si = pop();
	cx = pop();
	_add(si, 132);
	if (--cx)
		goto blimey;
	di = pop();
	es = pop();
}

void DreamGenContext::startloading() {
	STACK_CHECK;
	data.byte(kCombatcount) = 0;
	al = cs.byte(bx+13);
	data.byte(kRoomssample) = al;
	al = cs.byte(bx+15);
	data.byte(kMapx) = al;
	al = cs.byte(bx+16);
	data.byte(kMapy) = al;
	al = cs.byte(bx+20);
	data.byte(kLiftflag) = al;
	al = cs.byte(bx+21);
	data.byte(kManspath) = al;
	data.byte(kDestination) = al;
	data.byte(kFinaldest) = al;
	al = cs.byte(bx+22);
	data.byte(kFacing) = al;
	data.byte(kTurntoface) = al;
	al = cs.byte(bx+23);
	data.byte(kCounttoopen) = al;
	al = cs.byte(bx+24);
	data.byte(kLiftpath) = al;
	al = cs.byte(bx+25);
	data.byte(kDoorpath) = al;
	data.byte(kLastweapon) = -1;
	al = cs.byte(bx+27);
	push(ax);
	al = cs.byte(bx+31);
	ah = data.byte(kReallocation);
	data.byte(kReallocation) = al;
	dx = bx;
	openfile();
	readheader();
	allocateload();
	ds = ax;
	data.word(kBackdrop) = ax;
	dx = (0);
	loadseg();
	ds = data.word(kWorkspace);
	dx = (0);
	cx = 132*66;
	al = 0;
	fillspace();
	loadseg();
	sortoutmap();
	allocateload();
	data.word(kSetframes) = ax;
	ds = ax;
	dx = (0);
	loadseg();
	ds = data.word(kSetdat);
	dx = 0;
	cx = (64*128);
	al = 255;
	fillspace();
	loadseg();
	allocateload();
	data.word(kReel1) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReel2) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReel3) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kReels) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kPeople) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kSetdesc) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kBlockdesc) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kRoomdesc) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	allocateload();
	data.word(kFreeframes) = ax;
	ds = ax;
	dx = 0;
	loadseg();
	ds = data.word(kFreedat);
	dx = 0;
	cx = (16*80);
	al = 255;
	fillspace();
	loadseg();
	allocateload();
	data.word(kFreedesc) = ax;
	ds = ax;
	dx = (0);
	loadseg();
	closefile();
	findroominloc();
	deletetaken();
	setallchanges();
	autoappear();
	al = data.byte(kNewlocation);
	getroomdata();
	data.byte(kLastweapon) = -1;
	data.byte(kMandead) = 0;
	data.word(kLookcounter) = 160;
	data.byte(kNewlocation) = 255;
	data.byte(kLinepointer) = 254;
	ax = pop();
	_cmp(al, 255);
	if (flags.z())
		goto dontwalkin;
	data.byte(kManspath) = al;
	push(bx);
	autosetwalk();
	bx = pop();
dontwalkin:
	findxyfrompath();
}

void DreamGenContext::disablepath() {
	STACK_CHECK;
	push(cx);
	_xchg(al, ah);
	cx = -6;
looky2:
	_add(cx, 6);
	_sub(al, 10);
	if (!flags.c())
		goto looky2;
	al = ah;
	_dec(cx);
lookx2:
	_inc(cx);
	_sub(al, 11);
	if (!flags.c())
		goto lookx2;
	al = cl;
	ah = 0;
	cx = 144;
	_mul(cx);
	es = data.word(kReels);
	bx = (0);
	_add(bx, ax);
	ax = pop();
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	_add(bx, ax);
	al = 0;
	es.byte(bx+6) = al;
}

void DreamGenContext::findxyfrompath() {
	STACK_CHECK;
	getroomspaths();
	al = data.byte(kManspath);
	ah = 0;
	_add(ax, ax);
	_add(ax, ax);
	_add(ax, ax);
	_add(bx, ax);
	ax = es.word(bx);
	_sub(al, 12);
	_sub(ah, 12);
	data.byte(kRyanx) = al;
	data.byte(kRyany) = ah;
}

void DreamGenContext::findroominloc() {
	STACK_CHECK;
	al = data.byte(kMapy);
	cx = -6;
looky:
	_add(cx, 6);
	_sub(al, 10);
	if (!flags.c())
		goto looky;
	al = data.byte(kMapx);
	_dec(cx);
lookx:
	_inc(cx);
	_sub(al, 11);
	if (!flags.c())
		goto lookx;
	data.byte(kRoomnum) = cl;
}

void DreamGenContext::getroomdata() {
	STACK_CHECK;
	ah = 0;
	cx = 32;
	_mul(cx);
	bx = 6187;
	_add(bx, ax);
}

void DreamGenContext::readheader() {
	STACK_CHECK;
	ds = cs;
	dx = 6091;
	cx = (6187-6091);
	readfromfile();
	es = cs;
	di = 6141;
}

void DreamGenContext::allocateload() {
	STACK_CHECK;
	push(es);
	push(di);
	bx = es.word(di);
	cl = 4;
	_shr(bx, cl);
	allocatemem();
	di = pop();
	es = pop();
}

void DreamGenContext::fillspace() {
	STACK_CHECK;
	push(es);
	push(ds);
	push(dx);
	push(di);
	push(bx);
	di = dx;
	es = ds;
	_stosb(cx, true);
	bx = pop();
	di = pop();
	dx = pop();
	ds = pop();
	es = pop();
}

void DreamGenContext::getridoftemp() {
	STACK_CHECK;
	es = data.word(kTempgraphics);
	deallocatemem();
}

void DreamGenContext::getridoftemptext() {
	STACK_CHECK;
	es = data.word(kTextfile1);
	deallocatemem();
}

void DreamGenContext::getridoftemp2() {
	STACK_CHECK;
	es = data.word(kTempgraphics2);
	deallocatemem();
}

void DreamGenContext::getridoftemp3() {
	STACK_CHECK;
	es = data.word(kTempgraphics3);
	deallocatemem();
}

void DreamGenContext::getridoftempcharset() {
	STACK_CHECK;
	es = data.word(kTempcharset);
	deallocatemem();
}

void DreamGenContext::getridoftempsp() {
	STACK_CHECK;
	es = data.word(kTempsprites);
	deallocatemem();
}

void DreamGenContext::readsetdata() {
	STACK_CHECK;
	dx = 1857;
	standardload();
	data.word(kCharset1) = ax;
	dx = 1922;
	standardload();
	data.word(kIcons1) = ax;
	dx = 1935;
	standardload();
	data.word(kIcons2) = ax;
	dx = 1819;
	standardload();
	data.word(kMainsprites) = ax;
	dx = 2221;
	standardload();
	data.word(kPuzzletext) = ax;
	dx = 2273;
	standardload();
	data.word(kCommandtext) = ax;
	ax = data.word(kCharset1);
	data.word(kCurrentset) = ax;
	_cmp(data.byte(kSoundint), 255);
	if (flags.z())
		return /* (novolumeload) */;
	dx = 2286;
	openfile();
	cx = 2048-256;
	ds = data.word(kSoundbuffer);
	dx = 16384;
	readfromfile();
	closefile();
}

void DreamGenContext::makename() {
	STACK_CHECK;
	si = dx;
	di = 6061;
transfer:
	al = cs.byte(si);
	cs.byte(di) = al;
	_inc(si);
	_inc(di);
	_cmp(al, 0);
	if (!flags.z())
		goto transfer;
	dx = 6059;
}

void DreamGenContext::dreamweb() {
	STACK_CHECK;
	seecommandtail();
	checkbasemem();
	soundstartup();
	setkeyboardint();
	setupemm();
	allocatebuffers();
	setmouse();
	fadedos();
	gettime();
	clearbuffers();
	clearpalette();
	set16colpalette();
	readsetdata();
	data.byte(kWongame) = 0;
	dx = 1909;
	loadsample();
	setsoundoff();
	scanfornames();
	_cmp(al, 0);
	if (!flags.z())
		goto dodecisions;
	setmode();
	loadpalfromiff();
	titles();
	credits();
	goto playgame;
dodecisions:
	cls();
	setmode();
	decide();
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (exitgame) */;
	_cmp(data.byte(kGetback), 4);
	if (flags.z())
		goto mainloop;
	titles();
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (exitgame) */;
	credits();
playgame:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (exitgame) */;
	clearchanges();
	setmode();
	loadpalfromiff();
	data.byte(kLocation) = 255;
	data.byte(kRoomafterdream) = 1;
	data.byte(kNewlocation) = 35;
	data.byte(kVolume) = 7;
	loadroom();
	clearsprites();
	initman();
	entrytexts();
	entryanims();
	data.byte(kDestpos) = 3;
	initialinv();
	data.byte(kLastflag) = 32;
	startup1();
	data.byte(kVolumeto) = 0;
	data.byte(kVolumedirection) = -1;
	data.byte(kCommandtype) = 255;
	goto mainloop;
loadnew:
	clearbeforeload();
	loadroom();
	clearsprites();
	initman();
	entrytexts();
	entryanims();
	data.byte(kNewlocation) = 255;
	startup();
	data.byte(kCommandtype) = 255;
	worktoscreenm();
	goto mainloop;
	data.byte(kNewlocation) = 255;
	clearsprites();
	initman();
	startup();
	data.byte(kCommandtype) = 255;
mainloop:
	_cmp(data.byte(kQuitrequested),  0);
	if (!flags.z())
		return /* (exitgame) */;
	screenupdate();
	_cmp(data.byte(kWongame), 0);
	if (!flags.z())
		goto endofgame;
	_cmp(data.byte(kMandead), 1);
	if (flags.z())
		goto gameover;
	_cmp(data.byte(kMandead), 2);
	if (flags.z())
		goto gameover;
	_cmp(data.word(kWatchingtime), 0);
	if (flags.z())
		goto notwatching;
	al = data.byte(kFinaldest);
	_cmp(al, data.byte(kManspath));
	if (!flags.z())
		goto mainloop;
	_dec(data.word(kWatchingtime));
	if (!flags.z())
		goto mainloop;
notwatching:
	_cmp(data.byte(kMandead), 4);
	if (flags.z())
		goto gameover;
	_cmp(data.byte(kNewlocation), 255);
	if (!flags.z())
		goto loadnew;
	goto mainloop;
gameover:
	clearbeforeload();
	showgun();
	fadescreendown();
	cx = 100;
	hangon();
	goto dodecisions;
endofgame:
	clearbeforeload();
	fadescreendowns();
	cx = 200;
	hangon();
	endgame();
	{ quickquit2(); return; };
}



void DreamGenContext::__start() { 
	static const uint8 src[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x13, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0xff, 0x00, 
		0xff, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x0d, 0x00, 0xb6, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x01, 0x2c, 0x00, 0x14, 0x00, 0x02, 0x00, 0x01, 0x01, 0x37, 
		0x00, 0x00, 0x00, 0x32, 0x14, 0x00, 0x18, 0x16, 0x00, 0x4a, 0x00, 0x01, 0x00, 0x00, 0x18, 0x21, 
		0x0a, 0x4b, 0x00, 0x01, 0x00, 0x01, 0x01, 0x2c, 0x00, 0x1b, 0x00, 0x02, 0x00, 0x02, 0x01, 0x2c, 
		0x00, 0x60, 0x00, 0x03, 0x00, 0x04, 0x01, 0x2c, 0x00, 0x76, 0x00, 0x02, 0x00, 0x05, 0x01, 0x2c, 
		0x0a, 0x00, 0x00, 0x02, 0x00, 0x00, 0x05, 0x16, 0x14, 0x35, 0x00, 0x03, 0x00, 0x00, 0x05, 0x16, 
		0x14, 0x28, 0x00, 0x01, 0x00, 0x02, 0x05, 0x16, 0x14, 0x32, 0x00, 0x01, 0x00, 0x03, 0x02, 0x0b, 
		0x0a, 0xc0, 0x00, 0x01, 0x00, 0x00, 0x02, 0x0b, 0x0a, 0xb6, 0x00, 0x02, 0x00, 0x01, 0x08, 0x0b, 
		0x0a, 0x00, 0x00, 0x02, 0x00, 0x01, 0x17, 0x00, 0x32, 0x00, 0x00, 0x03, 0x00, 0x00, 0x1c, 0x0b, 
		0x14, 0xfa, 0x00, 0x04, 0x00, 0x00, 0x17, 0x00, 0x32, 0x2b, 0x00, 0x02, 0x00, 0x08, 0x17, 0x0b, 
		0x28, 0x82, 0x00, 0x02, 0x00, 0x01, 0x17, 0x16, 0x28, 0x7a, 0x00, 0x02, 0x00, 0x02, 0x17, 0x16, 
		0x28, 0x69, 0x00, 0x02, 0x00, 0x03, 0x17, 0x16, 0x28, 0x51, 0x00, 0x02, 0x00, 0x04, 0x17, 0x0b, 
		0x28, 0x87, 0x00, 0x02, 0x00, 0x05, 0x17, 0x16, 0x28, 0x91, 0x00, 0x02, 0x00, 0x06, 0x04, 0x16, 
		0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x2d, 0x16, 0x1e, 0xc8, 0x00, 0x00, 0x00, 0x14, 0x2d, 0x16, 
		0x1e, 0x27, 0x00, 0x02, 0x00, 0x00, 0x2d, 0x16, 0x1e, 0x19, 0x00, 0x02, 0x00, 0x00, 0x08, 0x16, 
		0x28, 0x20, 0x00, 0x02, 0x00, 0x00, 0x07, 0x0b, 0x14, 0x40, 0x00, 0x02, 0x00, 0x00, 0x16, 0x16, 
		0x14, 0x52, 0x00, 0x02, 0x00, 0x00, 0x1b, 0x0b, 0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x14, 0x00, 
		0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x0e, 0x21, 0x28, 0x15, 0x00, 0x01, 0x00, 0x00, 0x1d, 0x0b, 
		0x0a, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x19, 0x00, 
		0x32, 0x04, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 0x1e, 0x79, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 
		0x1e, 0x00, 0x00, 0x14, 0x00, 0x00, 0x34, 0x16, 0x1e, 0xc0, 0x00, 0x02, 0x00, 0x00, 0x34, 0x16, 
		0x1e, 0xe9, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 0x28, 0x68, 0x00, 0x37, 0x00, 0x00, 0x35, 0x21, 
		0x00, 0x63, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 0x28, 0x00, 0x00, 0x03, 0x00, 0x00, 0x32, 0x16, 
		0x1e, 0xa2, 0x00, 0x02, 0x00, 0x00, 0x34, 0x16, 0x1e, 0x39, 0x00, 0x02, 0x00, 0x00, 0x34, 0x16, 
		0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x36, 0x00, 0x00, 0x48, 0x00, 0x03, 0x00, 0x00, 0x37, 0x2c, 
		0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x0e, 0x16, 
		0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x0e, 0x16, 0x00, 0x2c, 0x01, 0x01, 0x00, 0x00, 0x0a, 0x16, 
		0x1e, 0xae, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x16, 0x14, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0b, 0x0b, 
		0x14, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0b, 0x0b, 0x1e, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0b, 0x16, 
		0x14, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0e, 0x21, 0x28, 0x00, 0x00, 0x32, 0x14, 0x00, 0xff, 0x7c, 
		0xc0, 0x80, 0xc0, 0x1c, 0xc0, 0x20, 0xc0, 0x00, 0xc1, 0x10, 0xc0, 0x18, 0xc0, 0xf4, 0xc0, 0x0c, 
		0xc0, 0x24, 0xc0, 0x28, 0xc0, 0x2c, 0xc0, 0x30, 0xc0, 0x54, 0xc0, 0x78, 0xc0, 0x50, 0xc0, 0x74, 
		0xc0, 0x34, 0xc0, 0x38, 0xc0, 0x40, 0xc0, 0x44, 0xc0, 0x48, 0xc0, 0x3c, 0xc0, 0x14, 0xc0, 0x88, 
		0xc0, 0x8c, 0xc0, 0x90, 0xc0, 0x70, 0xc0, 0xfc, 0xc0, 0x6c, 0xc0, 0x58, 0xc0, 0x68, 0xc0, 0x04, 
		0xc1, 0x64, 0xc0, 0x60, 0xc0, 0x5c, 0xc0, 0x94, 0xc0, 0x04, 0xc0, 0xa4, 0xc0, 0x9c, 0xc0, 0xa0, 
		0xc0, 0xa8, 0xc0, 0xac, 0xc0, 0x98, 0xc0, 0xb0, 0xc0, 0xb4, 0xc0, 0xc8, 0xc0, 0xcc, 0xc0, 0xd4, 
		0xc0, 0xdc, 0xc0, 0xd8, 0xc0, 0x00, 0xc0, 0x08, 0xc0, 0x84, 0xc0, 0x84, 0xc0, 0x84, 0xc0, 0x84, 
		0xc0, 0x00, 0x3c, 0x21, 0x47, 0x0b, 0x52, 0x16, 0x5d, 0x01, 0x2c, 0x0a, 0x10, 0x04, 0x0b, 0x1e, 
		0x0e, 0x04, 0x16, 0x1e, 0x0e, 0x03, 0x21, 0x0a, 0x0e, 0x0a, 0x21, 0x1e, 0x0e, 0x0a, 0x16, 0x1e, 
		0x18, 0x09, 0x16, 0x0a, 0x0e, 0x02, 0x21, 0x00, 0x0e, 0x02, 0x16, 0x00, 0x0e, 0x06, 0x0b, 0x1e, 
		0x0e, 0x07, 0x0b, 0x14, 0x12, 0x07, 0x00, 0x14, 0x12, 0x07, 0x00, 0x1e, 0x12, 0x37, 0x2c, 0x00, 
		0x0e, 0x05, 0x16, 0x1e, 0x0e, 0x08, 0x00, 0x0a, 0x12, 0x08, 0x0b, 0x0a, 0x12, 0x08, 0x16, 0x0a, 
		0x12, 0x08, 0x21, 0x0a, 0x12, 0x08, 0x21, 0x14, 0x12, 0x08, 0x21, 0x1e, 0x12, 0x08, 0x21, 0x28, 
		0x12, 0x08, 0x16, 0x28, 0x12, 0x08, 0x0b, 0x28, 0x12, 0x15, 0x2c, 0x14, 0x12, 0xff, 0x2e, 0x05, 
		0x2f, 0x05, 0x33, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x46, 0x05, 0x2e, 0x05, 0x4d, 0x05, 
		0x5d, 0x05, 0x64, 0x05, 0x68, 0x05, 0x6c, 0x05, 0x70, 0x05, 0x7d, 0x05, 0x2e, 0x05, 0x2e, 0x05, 
		0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x9f, 0x05, 0x2e, 0x05, 0xb5, 0x05, 0xd4, 0x05, 0x2e, 0x05, 
		0xe1, 0x05, 0xf7, 0x05, 0x0d, 0x06, 0x26, 0x06, 0x39, 0x06, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 
		0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 
		0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x49, 0x06, 0x50, 0x06, 0x75, 0x06, 0x2e, 0x05, 
		0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x82, 0x06, 0x86, 0x06, 0x2e, 0x05, 0x8d, 0x06, 0xff, 0x0f, 
		0x01, 0x01, 0xff, 0x0c, 0x05, 0x00, 0x0d, 0x15, 0x00, 0x0f, 0x23, 0x00, 0x11, 0x32, 0x00, 0x12, 
		0x67, 0x00, 0x13, 0x6c, 0x00, 0xff, 0x12, 0x13, 0x00, 0x13, 0x17, 0x00, 0xff, 0x0c, 0x33, 0x00, 
		0x0d, 0x35, 0x00, 0x0e, 0x0e, 0x00, 0x0f, 0x14, 0x00, 0x00, 0x4e, 0x00, 0xff, 0x0c, 0x77, 0x00, 
		0x0c, 0x91, 0x00, 0xff, 0x0d, 0x10, 0x00, 0xff, 0x0d, 0x14, 0x00, 0xff, 0x0e, 0x10, 0x00, 0xff, 
		0x0f, 0x04, 0x00, 0x10, 0x08, 0x00, 0x11, 0x86, 0x00, 0x12, 0x99, 0x00, 0xff, 0x0d, 0x6c, 0x00, 
		0x0f, 0x46, 0x01, 0x0f, 0x4b, 0x01, 0x0f, 0x50, 0x01, 0x0f, 0x56, 0x01, 0x0f, 0x5c, 0x01, 0x0f, 
		0x62, 0x01, 0x12, 0x9f, 0x00, 0x12, 0xb2, 0x00, 0x93, 0xd9, 0x00, 0x54, 0xe4, 0x00, 0xff, 0x0d, 
		0x14, 0x00, 0x0d, 0x15, 0x00, 0x0f, 0x22, 0x00, 0x0d, 0x34, 0x00, 0x0d, 0x37, 0x00, 0x19, 0x39, 
		0x00, 0x15, 0x49, 0x00, 0xff, 0x0d, 0xc4, 0x00, 0x0d, 0xea, 0x00, 0x0d, 0x9c, 0x00, 0x0e, 0x81, 
		0x00, 0x0d, 0x7c, 0x00, 0x0f, 0xa2, 0x00, 0x0f, 0xc8, 0x00, 0x0f, 0xef, 0x00, 0x11, 0x63, 0x00, 
		0x0c, 0x34, 0x00, 0xff, 0x0f, 0x38, 0x00, 0x10, 0x40, 0x00, 0x13, 0x16, 0x00, 0x14, 0x21, 0x00, 
		0xff, 0x14, 0x0b, 0x00, 0x14, 0x0f, 0x00, 0x0f, 0x1c, 0x00, 0x0d, 0x50, 0x00, 0x15, 0x52, 0x00, 
		0x93, 0x57, 0x00, 0x57, 0x80, 0x00, 0xff, 0x0c, 0x0d, 0x00, 0x0e, 0x27, 0x00, 0x0c, 0x43, 0x00, 
		0x0c, 0x4b, 0x00, 0x0c, 0x53, 0x00, 0x0c, 0x5b, 0x00, 0x0f, 0x66, 0x00, 0xff, 0x16, 0x24, 0x00, 
		0x0d, 0x7d, 0x00, 0x12, 0x58, 0x00, 0x0f, 0x6b, 0x00, 0x0e, 0x7f, 0x00, 0x0e, 0x9a, 0x00, 0x93, 
		0xaa, 0x00, 0x57, 0xe8, 0x00, 0xff, 0x15, 0x10, 0x00, 0x15, 0x48, 0x00, 0x15, 0xcd, 0x00, 0x16, 
		0x3f, 0x00, 0x97, 0x63, 0x00, 0x58, 0x9e, 0x00, 0xff, 0x0d, 0x15, 0x00, 0x0e, 0x18, 0x00, 0x93, 
		0x32, 0x00, 0x57, 0x4b, 0x00, 0x18, 0x80, 0x00, 0xff, 0x53, 0x2e, 0x00, 0x10, 0xa7, 0x00, 0xff, 
		0x10, 0x13, 0x00, 0x0e, 0x24, 0x00, 0x10, 0x32, 0x00, 0x0e, 0x41, 0x00, 0x10, 0x51, 0x00, 0x0e, 
		0x60, 0x00, 0x10, 0x72, 0x00, 0x0e, 0x81, 0x00, 0x10, 0x93, 0x00, 0x0e, 0xa2, 0x00, 0x10, 0xb1, 
		0x00, 0x0e, 0xbf, 0x00, 0xff, 0x0d, 0x30, 0x00, 0x0e, 0x29, 0x00, 0x0f, 0x4e, 0x00, 0x10, 0x5c, 
		0x00, 0xff, 0x10, 0x73, 0x00, 0xff, 0x15, 0x67, 0x00, 0x14, 0xc7, 0x00, 0xff, 0x11, 0x35, 0x00, 
		0x11, 0x36, 0x00, 0x11, 0x37, 0x00, 0x11, 0x38, 0x00, 0x11, 0x39, 0x00, 0x11, 0x3a, 0x00, 0x11, 
		0x3b, 0x00, 0x11, 0x3d, 0x00, 0x11, 0x3f, 0x00, 0x11, 0x40, 0x00, 0x11, 0x41, 0x00, 0xff, 0x9c, 
		0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 
		0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 
		0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 
		0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 
		0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 
		0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 
		0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9c, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x53, 0x30, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 
		0x2e, 0x53, 0x30, 0x32, 0x00, 0x49, 0x4e, 0x53, 0x54, 0x41, 0x4c, 0x4c, 0x2e, 0x44, 0x41, 0x54, 
		0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x43, 0x30, 0x30, 0x00, 0x44, 0x52, 
		0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x43, 0x30, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x43, 0x30, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 
		0x2e, 0x56, 0x30, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x56, 0x39, 
		0x39, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x30, 0x00, 0x44, 
		0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 
		0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 
		0x42, 0x2e, 0x47, 0x30, 0x38, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 
		0x30, 0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x37, 0x00, 
		0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x34, 0x00, 0x44, 0x52, 0x45, 
		0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x35, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 
		0x45, 0x42, 0x2e, 0x47, 0x30, 0x36, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 
		0x47, 0x31, 0x34, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x30, 0x31, 
		0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x30, 0x32, 0x00, 0x44, 0x52, 
		0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x31, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x54, 0x31, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 
		0x2e, 0x54, 0x31, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x31, 
		0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x32, 0x30, 0x00, 0x44, 
		0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x32, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 
		0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x32, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 
		0x42, 0x2e, 0x54, 0x32, 0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 
		0x32, 0x34, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x35, 0x30, 0x00, 
		0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x35, 0x31, 0x00, 0x44, 0x52, 0x45, 
		0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x38, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 
		0x45, 0x42, 0x2e, 0x54, 0x38, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 
		0x54, 0x38, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x38, 0x33, 
		0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x38, 0x34, 0x00, 0x44, 0x52, 
		0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x56, 0x4f, 0x4c, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x39, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 
		0x2e, 0x47, 0x31, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 
		0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 0x32, 0x00, 0x44, 
		0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 
		0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 0x35, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 
		0x42, 0x2e, 0x49, 0x30, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 
		0x30, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 0x30, 0x32, 0x00, 
		0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 0x30, 0x33, 0x00, 0x44, 0x52, 0x45, 
		0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 0x30, 0x34, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 
		0x45, 0x42, 0x2e, 0x49, 0x30, 0x35, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 
		0x49, 0x30, 0x36, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 0x30, 0x37, 
		0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x50, 0x41, 0x4c, 0x00, 0x11, 0x01, 
		0x40, 0x01, 0x9d, 0x00, 0xc6, 0x00, 0x44, 0xc3, 0x04, 0x01, 0x2c, 0x01, 0x00, 0x00, 0x2c, 0x00, 
		0x80, 0xc5, 0xd2, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x2c, 0x00, 0xdc, 0xc3, 0x90, 0x00, 0xb0, 0x00, 
		0x40, 0x00, 0x60, 0x00, 0x80, 0xc3, 0x00, 0x00, 0x32, 0x00, 0x32, 0x00, 0xc8, 0x00, 0x84, 0xc3, 
		0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x11, 0x01, 0x40, 0x01, 
		0x9d, 0x00, 0xc6, 0x00, 0x44, 0xc3, 0xff, 0x00, 0x26, 0x01, 0x00, 0x00, 0x18, 0x00, 0xc8, 0xc3, 
		0xf7, 0x00, 0x2d, 0x01, 0x28, 0x00, 0x38, 0x00, 0x48, 0xc3, 0x50, 0x00, 0x00, 0x01, 0x9e, 0x00, 
		0xca, 0x00, 0xe0, 0xc3, 0x50, 0x00, 0x2c, 0x01, 0x3a, 0x00, 0x92, 0x00, 0x98, 0xc3, 0x00, 0x00, 
		0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x11, 0x01, 0x40, 0x01, 0x9d, 0x00, 
		0xc6, 0x00, 0x44, 0xc3, 0xf7, 0x00, 0x2d, 0x01, 0x28, 0x00, 0x38, 0x00, 0x48, 0xc3, 0x50, 0x00, 
		0x2c, 0x01, 0x3a, 0x00, 0x92, 0x00, 0xbc, 0xc6, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 
		0xa0, 0xca, 0xff, 0xff, 0x11, 0x01, 0x40, 0x01, 0x9d, 0x00, 0xc6, 0x00, 0x7c, 0xc4, 0xf0, 0x00, 
		0x22, 0x01, 0x02, 0x00, 0x2c, 0x00, 0x94, 0xc4, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 
		0xa0, 0xca, 0xff, 0xff, 0x11, 0x01, 0x40, 0x01, 0x9d, 0x00, 0xc6, 0x00, 0x7c, 0xc4, 0x00, 0x00, 
		0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0xee, 0x00, 0x02, 0x01, 0x04, 0x00, 
		0x2c, 0x00, 0xc8, 0xc4, 0x68, 0x00, 0x7c, 0x00, 0x04, 0x00, 0x2c, 0x00, 0xcc, 0xc4, 0x18, 0x01, 
		0x34, 0x01, 0x04, 0x00, 0x2c, 0x00, 0xb0, 0xc4, 0x68, 0x00, 0xd8, 0x00, 0x8a, 0x00, 0xc0, 0x00, 
		0xd0, 0xc4, 0x11, 0x01, 0x40, 0x01, 0x9d, 0x00, 0xc6, 0x00, 0x7c, 0xc4, 0x00, 0x00, 0x40, 0x01, 
		0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x45, 0x58, 0x49, 0x54, 0x20, 0x20, 0x20, 0x20, 
		0x20, 0x20, 0x48, 0x45, 0x4c, 0x50, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4c, 0x49, 0x53, 0x54, 
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x52, 0x45, 0x41, 0x44, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		0x4c, 0x4f, 0x47, 0x4f, 0x4e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4b, 0x45, 0x59, 0x53, 0x20, 0x20, 
		0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 0x50, 0x55, 0x42, 0x4c, 0x49, 0x43, 0x20, 0x20, 0x20, 0x20, 
		0x20, 0x20, 0x50, 0x55, 0x42, 0x4c, 0x49, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 
		0x42, 0x4c, 0x41, 0x43, 0x4b, 0x44, 0x52, 0x41, 0x47, 0x4f, 0x4e, 0x20, 0x52, 0x59, 0x41, 0x4e, 
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x48, 0x45, 0x4e, 0x44, 0x52, 0x49, 
		0x58, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4c, 0x4f, 0x55, 0x49, 0x53, 0x20, 0x20, 0x20, 0x20, 0x20, 
		0x20, 0x00, 0x00, 0x00, 0x53, 0x45, 0x50, 0x54, 0x49, 0x4d, 0x55, 0x53, 0x20, 0x20, 0x20, 0x20, 
		0x42, 0x45, 0x43, 0x4b, 0x45, 0x54, 0x54, 0x20, 0x20, 0x20, 0x20, 0x00, 0xff, 0xff, 0x20, 0x20, 
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x22, 0x52, 0x4f, 0x4f, 
		0x54, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x22, 0x20, 0x20, 0x20, 0x20, 0x20, 
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x4e, 0x45, 0x54, 0x57, 0xe8, 0xc4, 0x45, 0x4c, 
		0x56, 0x41, 0x8c, 0xc6, 0x45, 0x4c, 0x56, 0x42, 0x9c, 0xc6, 0x45, 0x4c, 0x56, 0x43, 0x94, 0xc6, 
		0x45, 0x4c, 0x56, 0x45, 0x98, 0xc6, 0x45, 0x4c, 0x56, 0x46, 0xa0, 0xc6, 0x43, 0x47, 0x41, 0x54, 
		0x30, 0xc7, 0x52, 0x45, 0x4d, 0x4f, 0xa8, 0xc6, 0x42, 0x55, 0x54, 0x41, 0x3c, 0xc7, 0x43, 0x42, 
		0x4f, 0x58, 0x44, 0xc7, 0x4c, 0x49, 0x54, 0x45, 0x5c, 0xc6, 0x50, 0x4c, 0x41, 0x54, 0x40, 0xc7, 
		0x4c, 0x49, 0x46, 0x54, 0x7c, 0xc6, 0x57, 0x49, 0x52, 0x45, 0x84, 0xc6, 0x48, 0x4e, 0x44, 0x4c, 
		0x88, 0xc6, 0x48, 0x41, 0x43, 0x48, 0x80, 0xc6, 0x44, 0x4f, 0x4f, 0x52, 0xb4, 0xc6, 0x43, 0x53, 
		0x48, 0x52, 0x70, 0xc6, 0x47, 0x55, 0x4e, 0x41, 0x34, 0xc7, 0x43, 0x52, 0x41, 0x41, 0x64, 0xc6, 
		0x43, 0x52, 0x42, 0x42, 0x68, 0xc6, 0x43, 0x52, 0x43, 0x43, 0x6c, 0xc6, 0x53, 0x45, 0x41, 0x54, 
		0xf8, 0xc5, 0x4d, 0x45, 0x4e, 0x55, 0x98, 0xc7, 0x43, 0x4f, 0x4f, 0x4b, 0xac, 0xc6, 0x45, 0x4c, 
		0x43, 0x41, 0x4c, 0xc6, 0x45, 0x44, 0x43, 0x41, 0x50, 0xc6, 0x44, 0x44, 0x43, 0x41, 0x54, 0xc6, 
		0x41, 0x4c, 0x54, 0x52, 0x04, 0xc6, 0x4c, 0x4f, 0x4b, 0x41, 0x3c, 0xc6, 0x4c, 0x4f, 0x4b, 0x42, 
		0x40, 0xc6, 0x45, 0x4e, 0x54, 0x41, 0x10, 0xc6, 0x45, 0x4e, 0x54, 0x42, 0x24, 0xc6, 0x45, 0x4e, 
		0x54, 0x45, 0x28, 0xc6, 0x45, 0x4e, 0x54, 0x43, 0x18, 0xc6, 0x45, 0x4e, 0x54, 0x44, 0x2c, 0xc6, 
		0x45, 0x4e, 0x54, 0x48, 0x30, 0xc6, 0x57, 0x57, 0x41, 0x54, 0xf0, 0xc5, 0x50, 0x4f, 0x4f, 0x4c, 
		0x58, 0xc6, 0x57, 0x53, 0x48, 0x44, 0xf4, 0xc5, 0x47, 0x52, 0x41, 0x46, 0x44, 0xc6, 0x54, 0x52, 
		0x41, 0x50, 0x48, 0xc6, 0x43, 0x44, 0x50, 0x45, 0x28, 0xc7, 0x44, 0x4c, 0x4f, 0x4b, 0x08, 0xc6, 
		0x48, 0x4f, 0x4c, 0x45, 0x00, 0xc6, 0x44, 0x52, 0x59, 0x52, 0x0c, 0xc6, 0x48, 0x4f, 0x4c, 0x59, 
		0xfc, 0xc5, 0x57, 0x41, 0x4c, 0x4c, 0x2c, 0xc7, 0x42, 0x4f, 0x4f, 0x4b, 0x08, 0xc8, 0x41, 0x58, 
		0x45, 0x44, 0xb0, 0xc6, 0x53, 0x48, 0x4c, 0x44, 0x38, 0xc7, 0x42, 0x43, 0x4e, 0x59, 0xe8, 0xc5, 
		0x4c, 0x49, 0x44, 0x43, 0xe4, 0xc5, 0x4c, 0x49, 0x44, 0x55, 0xe0, 0xc5, 0x4c, 0x49, 0x44, 0x4f, 
		0xec, 0xc5, 0x50, 0x49, 0x50, 0x45, 0xa8, 0xc5, 0x42, 0x41, 0x4c, 0x43, 0x20, 0xc6, 0x57, 0x49, 
		0x4e, 0x44, 0x1c, 0xc6, 0x50, 0x41, 0x50, 0x52, 0xb4, 0xc7, 0x55, 0x57, 0x54, 0x41, 0xa0, 0xc5, 
		0x55, 0x57, 0x54, 0x42, 0xa0, 0xc5, 0x53, 0x54, 0x41, 0x54, 0xd8, 0xc7, 0x54, 0x4c, 0x49, 0x44, 
		0x9c, 0xc5, 0x53, 0x4c, 0x41, 0x42, 0xd8, 0xc5, 0x43, 0x41, 0x52, 0x54, 0xdc, 0xc5, 0x46, 0x43, 
		0x41, 0x52, 0xac, 0xc5, 0x53, 0x4c, 0x42, 0x41, 0xc0, 0xc5, 0x53, 0x4c, 0x42, 0x42, 0xc4, 0xc5, 
		0x53, 0x4c, 0x42, 0x43, 0xcc, 0xc5, 0x53, 0x4c, 0x42, 0x44, 0xc8, 0xc5, 0x53, 0x4c, 0x42, 0x45, 
		0xd0, 0xc5, 0x53, 0x4c, 0x42, 0x46, 0xd4, 0xc5, 0x50, 0x4c, 0x49, 0x4e, 0xb0, 0xc5, 0x4c, 0x41, 
		0x44, 0x44, 0xb8, 0xc5, 0x4c, 0x41, 0x44, 0x42, 0xbc, 0xc5, 0x47, 0x55, 0x4d, 0x41, 0xb4, 0xc5, 
		0x53, 0x51, 0x45, 0x45, 0x88, 0xc5, 0x54, 0x41, 0x50, 0x50, 0x8c, 0xc5, 0x47, 0x55, 0x49, 0x54, 
		0x90, 0xc5, 0x43, 0x4f, 0x4e, 0x54, 0x94, 0xc5, 0x42, 0x45, 0x4c, 0x4c, 0x98, 0xc5, 0x8c, 0x8c, 
		0x8c, 0x8c, 0x30, 0x30, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 0x9d, 0x00, 0xb2, 0x00, 0x51, 0x00, 
		0x5e, 0x00, 0x58, 0xc7, 0xb3, 0x00, 0xc8, 0x00, 0x51, 0x00, 0x5e, 0x00, 0x5c, 0xc7, 0xc9, 0x00, 
		0xde, 0x00, 0x51, 0x00, 0x5e, 0x00, 0x60, 0xc7, 0x9d, 0x00, 0xb2, 0x00, 0x5f, 0x00, 0x70, 0x00, 
		0x64, 0xc7, 0xb3, 0x00, 0xc8, 0x00, 0x5f, 0x00, 0x70, 0x00, 0x68, 0xc7, 0xc9, 0x00, 0xde, 0x00, 
		0x5f, 0x00, 0x70, 0x00, 0x6c, 0xc7, 0x9d, 0x00, 0xb2, 0x00, 0x71, 0x00, 0x82, 0x00, 0x70, 0xc7, 
		0xb3, 0x00, 0xc8, 0x00, 0x71, 0x00, 0x82, 0x00, 0x74, 0xc7, 0xc9, 0x00, 0xde, 0x00, 0x71, 0x00, 
		0x82, 0x00, 0x78, 0xc7, 0x9d, 0x00, 0xb2, 0x00, 0x83, 0x00, 0x91, 0x00, 0x7c, 0xc7, 0xb3, 0x00, 
		0xde, 0x00, 0x83, 0x00, 0x91, 0x00, 0x80, 0xc7, 0xdc, 0x00, 0xea, 0x00, 0x98, 0x00, 0xa6, 0x00, 
		0x50, 0xc7, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0xae, 0x00, 
		0xbc, 0x00, 0x84, 0x00, 0x94, 0x00, 0x50, 0xc7, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 
		0xa0, 0xca, 0xff, 0xff, 0x18, 0x01, 0x40, 0x01, 0xa0, 0x00, 0xc8, 0x00, 0x50, 0xc7, 0x8f, 0x00, 
		0x2c, 0x01, 0x06, 0x00, 0xc2, 0x00, 0xb8, 0xc7, 0x00, 0x00, 0x8f, 0x00, 0x06, 0x00, 0xc2, 0x00, 
		0xc0, 0xc7, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x68, 0x00, 
		0x80, 0x00, 0x3a, 0x00, 0x48, 0x00, 0xdc, 0xc7, 0x40, 0x00, 0x74, 0x00, 0x4c, 0x00, 0x6a, 0x00, 
		0xe0, 0xc7, 0x74, 0x00, 0xa8, 0x00, 0x4c, 0x00, 0x6a, 0x00, 0xe4, 0xc7, 0x40, 0x00, 0x74, 0x00, 
		0x6a, 0x00, 0x88, 0x00, 0xe8, 0xc7, 0x74, 0x00, 0xa8, 0x00, 0x6a, 0x00, 0x88, 0x00, 0xec, 0xc7, 
		0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0xba, 0x00, 0xca, 0x00, 
		0x9d, 0x00, 0xad, 0x00, 0x1c, 0xc8, 0xf3, 0x00, 0x03, 0x01, 0x83, 0x00, 0x93, 0x00, 0x18, 0xc8, 
		0x0c, 0x01, 0x1c, 0x01, 0xa8, 0x00, 0xb8, 0x00, 0x50, 0xc7, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 
		0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x77, 0x00, 0xae, 0x00, 0x52, 0x00, 0x80, 0x00, 0x34, 0xc8, 
		0x46, 0x00, 0x89, 0x00, 0x3e, 0x00, 0x6f, 0x00, 0x80, 0xc8, 0xbc, 0x00, 0xfa, 0x00, 0x44, 0x00, 
		0x98, 0x00, 0x4c, 0xc8, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 
		0xec, 0x00, 0xfc, 0x00, 0x70, 0x00, 0x80, 0x00, 0x48, 0xc8, 0xbc, 0x00, 0xfa, 0x00, 0x40, 0x00, 
		0x98, 0x00, 0x58, 0xc8, 0x3e, 0x00, 0x98, 0x00, 0x38, 0x00, 0x85, 0x00, 0x74, 0xc8, 0x00, 0x00, 
		0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x77, 0x00, 0xae, 0x00, 0x52, 0x00, 
		0x80, 0x00, 0x44, 0xc8, 0x46, 0x00, 0x8b, 0x00, 0x3e, 0x00, 0x6f, 0x00, 0x50, 0xc8, 0xec, 0x00, 
		0xfc, 0x00, 0x70, 0x00, 0x80, 0x00, 0x48, 0xc8, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 
		0xa0, 0xca, 0xff, 0xff, 0xec, 0x00, 0xfc, 0x00, 0x70, 0x00, 0x80, 0x00, 0x48, 0xc8, 0xbc, 0x00, 
		0xfa, 0x00, 0x40, 0x00, 0x98, 0x00, 0x54, 0xc8, 0x3e, 0x00, 0x98, 0x00, 0x38, 0x00, 0x85, 0x00, 
		0x74, 0xc8, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x0d, 0x0a, 
		0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 
		0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 0x55, 0x6e, 0x61, 0x62, 0x6c, 0x65, 
		0x20, 0x74, 0x6f, 0x20, 0x61, 0x6c, 0x6c, 0x6f, 0x63, 0x61, 0x74, 0x65, 0x20, 0x45, 0x78, 0x70, 
		0x61, 0x6e, 0x64, 0x65, 0x64, 0x20, 0x4d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x2e, 0x0d, 0x0a, 0x0d, 
		0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x68, 
		0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 0x53, 0x6f, 
		0x75, 0x6e, 0x64, 0x20, 0x42, 0x6c, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x63, 0x61, 0x72, 0x64, 
		0x20, 0x6e, 0x6f, 0x74, 0x20, 0x66, 0x6f, 0x75, 0x6e, 0x64, 0x20, 0x61, 0x74, 0x20, 0x61, 0x64, 
		0x64, 0x72, 0x65, 0x73, 0x73, 0x20, 0x32, 0x32, 0x30, 0x20, 0x48, 0x65, 0x78, 0x2e, 0x0d, 0x0a, 
		0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 
		0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 0x4f, 
		0x75, 0x74, 0x20, 0x6f, 0x66, 0x20, 0x42, 0x61, 0x73, 0x65, 0x20, 0x4d, 0x65, 0x6d, 0x6f, 0x72, 
		0x79, 0x2e, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 
		0x77, 0x65, 0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 
		0x3a, 0x0d, 0x0a, 0x4d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x20, 0x44, 0x65, 0x61, 0x6c, 0x6c, 0x6f, 
		0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x70, 0x72, 0x6f, 0x62, 0x6c, 0x65, 0x6d, 0x2e, 0x0d, 
		0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 
		0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 
		0x41, 0x74, 0x20, 0x6c, 0x65, 0x61, 0x73, 0x74, 0x20, 0x35, 0x39, 0x30, 0x4b, 0x20, 0x6f, 0x66, 
		0x20, 0x62, 0x61, 0x73, 0x65, 0x20, 0x6d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x20, 0x69, 0x73, 0x20, 
		0x72, 0x65, 0x71, 0x75, 0x69, 0x72, 0x65, 0x64, 0x2e, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 
		0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 
		0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 0x53, 0x6f, 0x75, 0x6e, 0x64, 0x20, 
		0x42, 0x6c, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x66, 0x6f, 0x75, 0x6e, 
		0x64, 0x20, 0x6f, 0x6e, 0x20, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x75, 0x70, 0x74, 0x20, 0x30, 0x0d, 
		0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 
		0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 
		0x55, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x74, 0x6f, 0x20, 0x73, 0x65, 0x6c, 0x65, 0x63, 0x74, 
		0x20, 0x45, 0x4d, 0x4d, 0x20, 0x70, 0x61, 0x67, 0x65, 0x2e, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x0d, 
		0x0a, 0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 
		0x61, 0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 0x46, 0x69, 0x6c, 0x65, 0x20, 
		0x6e, 0x6f, 0x74, 0x20, 0x66, 0x6f, 0x75, 0x6e, 0x64, 0x2e, 0x63, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 
		0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x6c, 0x6f, 0x6f, 0x6b, 0x73, 0x20, 0x66, 
		0x6f, 0x72, 0x20, 0x53, 0x6f, 0x75, 0x6e, 0x64, 0x20, 0x42, 0x6c, 0x61, 0x73, 0x74, 0x65, 0x72, 
		0x20, 0x69, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x69, 0x6e, 0x0d, 
		0x0a, 0x74, 0x68, 0x65, 0x20, 0x42, 0x4c, 0x41, 0x53, 0x54, 0x45, 0x52, 0x20, 0x65, 0x6e, 0x76, 
		0x69, 0x72, 0x6f, 0x6e, 0x6d, 0x65, 0x6e, 0x74, 0x20, 0x76, 0x61, 0x72, 0x69, 0x61, 0x62, 0x6c, 
		0x65, 0x20, 0x28, 0x69, 0x6e, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x41, 0x55, 0x54, 0x4f, 0x45, 
		0x58, 0x45, 0x43, 0x2e, 0x42, 0x41, 0x54, 0x29, 0x0d, 0x0a, 0x0d, 0x0a, 0x49, 0x66, 0x20, 0x74, 
		0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x66, 0x6f, 0x75, 0x6e, 0x64, 
		0x20, 0x74, 0x68, 0x65, 0x6e, 0x20, 0x49, 0x52, 0x51, 0x20, 0x37, 0x2c, 0x20, 0x44, 0x4d, 0x41, 
		0x20, 0x63, 0x68, 0x61, 0x6e, 0x6e, 0x65, 0x6c, 0x20, 0x31, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x62, 
		0x61, 0x73, 0x65, 0x0d, 0x0a, 0x61, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x20, 0x32, 0x32, 0x30, 
		0x68, 0x20, 0x61, 0x72, 0x65, 0x20, 0x61, 0x73, 0x73, 0x75, 0x6d, 0x65, 0x64, 0x2e, 0x0d, 0x0a, 
		0x0d, 0x0a, 0x54, 0x6f, 0x20, 0x61, 0x6c, 0x74, 0x65, 0x72, 0x20, 0x61, 0x6e, 0x79, 0x20, 0x6f, 
		0x72, 0x20, 0x61, 0x6c, 0x6c, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x73, 0x65, 0x20, 0x73, 
		0x65, 0x74, 0x74, 0x69, 0x6e, 0x67, 0x73, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x63, 0x61, 0x6e, 0x20, 
		0x73, 0x70, 0x65, 0x63, 0x69, 0x66, 0x79, 0x20, 0x74, 0x68, 0x65, 0x6d, 0x0d, 0x0a, 0x6f, 0x6e, 
		0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64, 0x20, 0x6c, 0x69, 0x6e, 
		0x65, 0x2e, 0x20, 0x46, 0x6f, 0x72, 0x20, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x3a, 0x0d, 
		0x0a, 0x0d, 0x0a, 0x54, 0x79, 0x70, 0x65, 0x20, 0x20, 0x20, 0x20, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x20, 0x49, 0x37, 0x20, 0x41, 0x32, 0x32, 0x30, 0x20, 0x44, 0x31, 0x20, 0x20, 
		0x20, 0x20, 0x74, 0x6f, 0x20, 0x72, 0x75, 0x6e, 0x20, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 
		0x62, 0x20, 0x6f, 0x6e, 0x20, 0x49, 0x52, 0x51, 0x20, 0x37, 0x2c, 0x20, 0x44, 0x4d, 0x41, 0x0d, 
		0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		0x63, 0x68, 0x61, 0x6e, 0x6e, 0x65, 0x6c, 0x20, 0x31, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x62, 0x61, 
		0x73, 0x65, 0x20, 0x61, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x20, 0x32, 0x32, 0x30, 0x68, 0x0d, 
		0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 
		0x42, 0x20, 0x49, 0x35, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		0x74, 0x6f, 0x20, 0x72, 0x75, 0x6e, 0x20, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 
		0x6f, 0x6e, 0x20, 0x49, 0x52, 0x51, 0x20, 0x35, 0x20, 0x61, 0x6e, 0x64, 0x0d, 0x0a, 0x20, 0x20, 
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x64, 0x65, 0x66, 
		0x61, 0x75, 0x6c, 0x74, 0x20, 0x61, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x20, 0x6f, 0x66, 0x20, 
		0x32, 0x32, 0x30, 0x68, 0x2c, 0x20, 0x44, 0x4d, 0x41, 0x20, 0x31, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 
		0x0d, 0x0a, 0x0d, 0x0a, 0x54, 0x72, 0x79, 0x20, 0x74, 0x68, 0x65, 0x20, 0x44, 0x72, 0x65, 0x61, 
		0x6d, 0x77, 0x65, 0x62, 0x20, 0x43, 0x44, 0x20, 0x69, 0x6e, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 
		0x73, 0x74, 0x65, 0x72, 0x65, 0x6f, 0x2e, 0x2e, 0x2e, 0x2e, 0x0d, 0x0a, 0x0d, 0x0a, 0x0d, 0x0a, 
		0x24, 0x81, 0x00, 0xb8, 0x00, 0x52, 0x00, 0x80, 0x00, 0xc0, 0xc8, 0x50, 0x00, 0x93, 0x00, 0x3e, 
		0x00, 0x6f, 0x00, 0x80, 0xc8, 0xb7, 0x00, 0xfa, 0x00, 0x3e, 0x00, 0x6f, 0x00, 0xc4, 0xc8, 0x00, 
		0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x53, 0x50, 0x45, 0x45, 0x43, 
		0x48, 0x52, 0x32, 0x34, 0x43, 0x30, 0x30, 0x30, 0x35, 0x2e, 0x52, 0x41, 0x57, 0x00, 0x87, 0x83, 
		0x81, 0x82, 0x2c, 0x00, 0x46, 0x00, 0x20, 0x00, 0x2e, 0x00, 0x70, 0xc4, 0x00, 0x00, 0x32, 0x00, 
		0x00, 0x00, 0xb4, 0x00, 0x7c, 0xc3, 0xe2, 0x00, 0xf4, 0x00, 0x0a, 0x00, 0x1a, 0x00, 0x28, 0xc8, 
		0xe2, 0x00, 0xf4, 0x00, 0x1a, 0x00, 0x28, 0x00, 0x2c, 0xc8, 0xf0, 0x00, 0x04, 0x01, 0x64, 0x00, 
		0x7c, 0x00, 0xcc, 0xc9, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xd4, 0xc9, 0xff, 0xff, 
		0x2c, 0x00, 0x46, 0x00, 0x20, 0x00, 0x2e, 0x00, 0x70, 0xc4, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 
		0xb4, 0x00, 0x7c, 0xc3, 0x12, 0x01, 0x24, 0x01, 0x0a, 0x00, 0x1a, 0x00, 0x28, 0xc8, 0x12, 0x01, 
		0x24, 0x01, 0x1a, 0x00, 0x28, 0x00, 0x2c, 0xc8, 0xf0, 0x00, 0x04, 0x01, 0x64, 0x00, 0x7c, 0x00, 
		0xcc, 0xc9, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xd4, 0xc9, 0xff, 0xff, 0x00, 0x21, 
		0x0a, 0x0f, 0xff, 0x00, 0x16, 0x0a, 0x0f, 0xff, 0x00, 0x16, 0x00, 0x0f, 0xff, 0x00, 0x0b, 0x00, 
		0x0f, 0xff, 0x00, 0x0b, 0x0a, 0x0f, 0xff, 0x00, 0x00, 0x0a, 0x0f, 0xff, 0x01, 0x2c, 0x0a, 0x06, 
		0xff, 0x01, 0x2c, 0x00, 0x0d, 0xff, 0x02, 0x21, 0x00, 0x06, 0xff, 0x02, 0x16, 0x00, 0x05, 0xff, 
		0x02, 0x16, 0x0a, 0x10, 0xff, 0x02, 0x0b, 0x0a, 0x10, 0xff, 0x03, 0x2c, 0x00, 0x0f, 0xff, 0x03, 
		0x21, 0x0a, 0x06, 0xff, 0x03, 0x21, 0x00, 0x05, 0xff, 0x04, 0x0b, 0x1e, 0x06, 0xff, 0x04, 0x16, 
		0x1e, 0x05, 0xff, 0x04, 0x16, 0x14, 0x0d, 0xff, 0x0a, 0x21, 0x1e, 0x06, 0xff, 0x0a, 0x16, 0x1e, 
		0x06, 0xff, 0x09, 0x16, 0x0a, 0x06, 0xff, 0x09, 0x16, 0x14, 0x10, 0xff, 0x09, 0x16, 0x1e, 0x10, 
		0xff, 0x09, 0x16, 0x28, 0x10, 0xff, 0x09, 0x16, 0x32, 0x10, 0xff, 0x06, 0x0b, 0x1e, 0x06, 0xff, 
		0x06, 0x00, 0x0a, 0x0f, 0xff, 0x06, 0x00, 0x14, 0x0f, 0xff, 0x06, 0x0b, 0x14, 0x0f, 0xff, 0x06, 
		0x16, 0x14, 0x0f, 0xff, 0x07, 0x0b, 0x14, 0x06, 0xff, 0x07, 0x00, 0x14, 0x06, 0xff, 0x07, 0x00, 
		0x1e, 0x06, 0xff, 0x37, 0x2c, 0x00, 0x05, 0xff, 0x37, 0x2c, 0x0a, 0x05, 0xff, 0x05, 0x16, 0x1e, 
		0x06, 0xff, 0x05, 0x16, 0x14, 0x0f, 0xff, 0x05, 0x16, 0x0a, 0x0f, 0xff, 0x18, 0x16, 0x00, 0x0f, 
		0xff, 0x18, 0x21, 0x00, 0x0f, 0xff, 0x18, 0x2c, 0x00, 0x0f, 0xff, 0x18, 0x21, 0x0a, 0x0f, 0xff, 
		0x08, 0x00, 0x0a, 0x06, 0xff, 0x08, 0x0b, 0x0a, 0x06, 0xff, 0x08, 0x16, 0x0a, 0x06, 0xff, 0x08, 
		0x21, 0x0a, 0x06, 0xff, 0x08, 0x21, 0x14, 0x06, 0xff, 0x08, 0x21, 0x1e, 0x06, 0xff, 0x08, 0x21, 
		0x28, 0x06, 0xff, 0x08, 0x16, 0x28, 0x06, 0xff, 0x08, 0x0b, 0x28, 0x06, 0xff, 0x0b, 0x0b, 0x14, 
		0x0c, 0xff, 0x0b, 0x0b, 0x1e, 0x0c, 0xff, 0x0b, 0x16, 0x14, 0x0c, 0xff, 0x0b, 0x16, 0x1e, 0x0c, 
		0xff, 0x0c, 0x16, 0x14, 0x0c, 0xff, 0x0d, 0x16, 0x14, 0x0c, 0xff, 0x0d, 0x21, 0x14, 0x0c, 0xff, 
		0x0e, 0x2c, 0x14, 0x0c, 0xff, 0x0e, 0x21, 0x00, 0x0c, 0xff, 0x0e, 0x21, 0x0a, 0x0c, 0xff, 0x0e, 
		0x21, 0x14, 0x0c, 0xff, 0x0e, 0x21, 0x1e, 0x0c, 0xff, 0x0e, 0x21, 0x28, 0x0c, 0xff, 0x0e, 0x16, 
		0x00, 0x10, 0xff, 0x13, 0x00, 0x00, 0x0c, 0xff, 0x14, 0x00, 0x14, 0x10, 0xff, 0x14, 0x00, 0x1e, 
		0x10, 0xff, 0x14, 0x0b, 0x1e, 0x10, 0xff, 0x14, 0x00, 0x28, 0x10, 0xff, 0x14, 0x0b, 0x28, 0x10, 
		0xff, 0x15, 0x0b, 0x0a, 0x0f, 0xff, 0x15, 0x0b, 0x14, 0x0f, 0xff, 0x15, 0x00, 0x14, 0x0f, 0xff, 
		0x15, 0x16, 0x14, 0x0f, 0xff, 0x15, 0x21, 0x14, 0x0f, 0xff, 0x15, 0x2c, 0x14, 0x0f, 0xff, 0x15, 
		0x2c, 0x0a, 0x0f, 0xff, 0x16, 0x16, 0x0a, 0x10, 0xff, 0x16, 0x16, 0x14, 0x10, 0xff, 0x17, 0x16, 
		0x1e, 0x0d, 0xff, 0x17, 0x16, 0x28, 0x0d, 0xff, 0x17, 0x21, 0x28, 0x0d, 0xff, 0x17, 0x0b, 0x28, 
		0x0d, 0xff, 0x17, 0x00, 0x28, 0x0d, 0xff, 0x17, 0x00, 0x32, 0x0d, 0xff, 0x19, 0x0b, 0x28, 0x10, 
		0xff, 0x19, 0x0b, 0x32, 0x10, 0xff, 0x19, 0x00, 0x32, 0x10, 0xff, 0x1b, 0x0b, 0x14, 0x10, 0xff, 
		0x1b, 0x0b, 0x1e, 0x10, 0xff, 0x1d, 0x0b, 0x0a, 0x10, 0xff, 0x2d, 0x16, 0x1e, 0x0c, 0xff, 0x2d, 
		0x16, 0x28, 0x0c, 0xff, 0x2d, 0x16, 0x32, 0x0c, 0xff, 0x2e, 0x16, 0x28, 0x0c, 0xff, 0x2e, 0x0b, 
		0x32, 0x0c, 0xff, 0x2e, 0x16, 0x32, 0x0c, 0xff, 0x2e, 0x21, 0x32, 0x0c, 0xff, 0x2f, 0x00, 0x00, 
		0x0c, 0xff, 0x1a, 0x16, 0x14, 0x10, 0xff, 0x1a, 0x21, 0x0a, 0x10, 0xff, 0x1a, 0x21, 0x14, 0x10, 
		0xff, 0x1a, 0x21, 0x1e, 0x10, 0xff, 0x1a, 0x2c, 0x1e, 0x10, 0xff, 0x1a, 0x16, 0x1e, 0x10, 0xff, 
		0x1a, 0x0b, 0x1e, 0x10, 0xff, 0x1a, 0x0b, 0x14, 0x10, 0xff, 0x1a, 0x00, 0x14, 0x10, 0xff, 0x1a, 
		0x0b, 0x28, 0x10, 0xff, 0x1a, 0x00, 0x28, 0x10, 0xff, 0x1a, 0x16, 0x28, 0x10, 0xff, 0x1a, 0x0b, 
		0x32, 0x10, 0xff, 0x1c, 0x00, 0x1e, 0x0f, 0xff, 0x1c, 0x00, 0x14, 0x0f, 0xff, 0x1c, 0x00, 0x28, 
		0x0f, 0xff, 0x1c, 0x0b, 0x1e, 0x0f, 0xff, 0x1c, 0x0b, 0x14, 0x0f, 0xff, 0x1c, 0x16, 0x1e, 0x0f, 
		0xff, 0x1c, 0x16, 0x14, 0x0f, 0xff, 0xff, 0x4f, 0x42, 0x4a, 0x45, 0x43, 0x54, 0x20, 0x4e, 0x41, 
		0x4d, 0x45, 0x20, 0x4f, 0x4e, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
		0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 
		0x10, 0x12, 0x12, 0x11, 0x10, 0x10, 0x10, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 
		0x37, 0x38, 0x39, 0x30, 0x2d, 0x00, 0x08, 0x00, 0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 
		0x4f, 0x50, 0x00, 0x00, 0x0d, 0x00, 0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x3a, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x20, 0x44, 0x41, 0x54, 0x41, 0x20, 0x46, 0x49, 0x4c, 0x45, 0x20, 0x43, 0x4f, 
		0x50, 0x59, 0x52, 0x49, 0x47, 0x48, 0x54, 0x20, 0x31, 0x39, 0x39, 0x32, 0x20, 0x43, 0x52, 0x45, 
		0x41, 0x54, 0x49, 0x56, 0x45, 0x20, 0x52, 0x45, 0x41, 0x4c, 0x49, 0x54, 0x59, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x30, 0x00, 0x05, 0xff, 0x21, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x06, 0x02, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x31, 0x00, 0x01, 0xff, 0x2c, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x07, 0x02, 0xff, 0xff, 0xff, 0xff, 0x06, 0xff, 0xff, 0xff, 0x01, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x32, 0x00, 0x02, 0xff, 0x21, 0x00, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x00, 0xff, 0xff, 0x01, 0xff, 0x03, 0xff, 0xff, 0xff, 0x02, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x33, 0x00, 0x05, 0xff, 0x21, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x02, 0x02, 0x00, 0x02, 0x04, 0xff, 0x00, 0xff, 0xff, 0xff, 0x03, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x34, 0x00, 0x17, 0xff, 0x0b, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x04, 0x00, 0x05, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0x04, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x35, 0x00, 0x05, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x02, 0x00, 0x04, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0x05, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x36, 0x00, 0x05, 0xff, 0x0b, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x00, 0x00, 0x01, 0x02, 0xff, 0x00, 0xff, 0xff, 0xff, 0x06, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x37, 0x00, 0xff, 0xff, 0x00, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x02, 0x02, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x07, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x38, 0x00, 0x08, 0xff, 0x00, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x02, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0b, 0x28, 0x00, 0x08, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x39, 0x00, 0x09, 0xff, 0x16, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x04, 0x06, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x09, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x31, 0x30, 0x00, 0x0a, 0xff, 0x21, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x02, 0x00, 0xff, 0xff, 0x02, 0x02, 0x04, 0x16, 0x1e, 0xff, 0x0a, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x31, 0x31, 0x00, 0x0b, 0xff, 0x0b, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0b, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x31, 0x32, 0x00, 0x0c, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0c, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x31, 0x33, 0x00, 0x0c, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0d, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x31, 0x34, 0x00, 0x0e, 0xff, 0x2c, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x31, 0x39, 0x00, 0x13, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x13, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x30, 0x00, 0x16, 0xff, 0x00, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x04, 0x02, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x14, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x31, 0x00, 0x05, 0xff, 0x0b, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x04, 0x02, 0x0f, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0x15, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x32, 0x00, 0x16, 0xff, 0x16, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x04, 0xff, 0xff, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0x16, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x33, 0x00, 0x17, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x04, 0x02, 0x0f, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x34, 0x00, 0x05, 0xff, 0x2c, 0x00, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x06, 0x02, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x18, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x35, 0x00, 0x16, 0xff, 0x0b, 0x28, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x19, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x36, 0x00, 0x09, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x04, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x37, 0x00, 0x16, 0xff, 0x0b, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1b, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x38, 0x00, 0x05, 0xff, 0x0b, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x00, 0xff, 0xff, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1c, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x39, 0x00, 0x16, 0xff, 0x0b, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1d, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x35, 0x00, 0x05, 0xff, 0x16, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x04, 0x01, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x05, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x34, 0x00, 0x17, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x04, 0x02, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x04, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x31, 0x30, 0x00, 0x0a, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x03, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x31, 0x32, 0x00, 0x0c, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0c, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x33, 0x00, 0x05, 0xff, 0x2c, 0x00, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x06, 0x02, 0xff, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x34, 0x00, 0x05, 0xff, 0x16, 0x00, 0xff, 0xff, 0xff, 0x00, 
		0x03, 0x06, 0x00, 0xff, 0xff, 0xff, 0xff, 0x21, 0x00, 0x03, 0x18, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x32, 0x00, 0x16, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x16, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x32, 0x00, 0x16, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x16, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x31, 0x31, 0x00, 0x0b, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0b, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x38, 0x00, 0x05, 0xff, 0x0b, 0x14, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x06, 0xff, 0xff, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1c, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x31, 0x00, 0x05, 0xff, 0x0b, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x01, 0x04, 0x02, 0x0f, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0x15, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x32, 0x36, 0x00, 0x09, 0xff, 0x00, 0x28, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x31, 0x39, 0x00, 0x13, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 
		0x02, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x13, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x38, 0x00, 0x08, 0xff, 0x0b, 0x28, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x08, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x30, 0x31, 0x00, 0x01, 0xff, 0x2c, 0x0a, 0xff, 0xff, 0xff, 0x00, 
		0x03, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x34, 0x35, 0x00, 0x23, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x2d, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x34, 0x36, 0x00, 0x23, 0xff, 0x16, 0x28, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x2e, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x34, 0x37, 0x00, 0x23, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x2f, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x34, 0x35, 0x00, 0x23, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x04, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x2d, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x34, 0x36, 0x00, 0x23, 0xff, 0x16, 0x32, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x2e, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x35, 0x30, 0x00, 0x23, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x32, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x35, 0x31, 0x00, 0x23, 0xff, 0x0b, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x33, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x35, 0x32, 0x00, 0x23, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x34, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x35, 0x33, 0x00, 0x23, 0xff, 0x21, 0x00, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x35, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x35, 0x34, 0x00, 0x23, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x36, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x52, 0x35, 0x35, 0x00, 0x0e, 0xff, 0x2c, 0x00, 0xff, 0xff, 0xff, 0x00, 
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x02, 0x04, 
		0x01, 0x0a, 0x09, 0x08, 0x06, 0x0b, 0x04, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 
		0x45, 0x42, 0x2e, 0x44, 0x30, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 
		0x44, 0x30, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 0x30, 0x32, 
		0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 0x30, 0x33, 0x00, 0x44, 0x52, 
		0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 0x30, 0x34, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x44, 0x30, 0x35, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 
		0x2e, 0x44, 0x30, 0x36, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 0x45, 
		0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, };
	ds.assign(src, src + sizeof(src));
dreamweb(); 
}

void DreamGenContext::__dispatch_call(uint16 addr) {
	switch(addr) {
		case addr_alleybarksound: alleybarksound(); break;
		case addr_intromusic: intromusic(); break;
		case addr_foghornsound: foghornsound(); break;
		case addr_receptionist: receptionist(); break;
		case addr_smokebloke: smokebloke(); break;
		case addr_attendant: attendant(); break;
		case addr_manasleep: manasleep(); break;
		case addr_eden: eden(); break;
		case addr_edeninbath: edeninbath(); break;
		case addr_malefan: malefan(); break;
		case addr_femalefan: femalefan(); break;
		case addr_louis: louis(); break;
		case addr_louischair: louischair(); break;
		case addr_manasleep2: manasleep2(); break;
		case addr_mansatstill: mansatstill(); break;
		case addr_tattooman: tattooman(); break;
		case addr_drinker: drinker(); break;
		case addr_bartender: bartender(); break;
		case addr_othersmoker: othersmoker(); break;
		case addr_barwoman: barwoman(); break;
		case addr_interviewer: interviewer(); break;
		case addr_soldier1: soldier1(); break;
		case addr_rockstar: rockstar(); break;
		case addr_helicopter: helicopter(); break;
		case addr_mugger: mugger(); break;
		case addr_aide: aide(); break;
		case addr_businessman: businessman(); break;
		case addr_poolguard: poolguard(); break;
		case addr_security: security(); break;
		case addr_heavy: heavy(); break;
		case addr_bossman: bossman(); break;
		case addr_gamer: gamer(); break;
		case addr_sparkydrip: sparkydrip(); break;
		case addr_carparkdrip: carparkdrip(); break;
		case addr_keeper: keeper(); break;
		case addr_candles1: candles1(); break;
		case addr_smallcandle: smallcandle(); break;
		case addr_intromagic1: intromagic1(); break;
		case addr_candles: candles(); break;
		case addr_candles2: candles2(); break;
		case addr_gates: gates(); break;
		case addr_intromagic2: intromagic2(); break;
		case addr_intromagic3: intromagic3(); break;
		case addr_intromonks1: intromonks1(); break;
		case addr_intromonks2: intromonks2(); break;
		case addr_handclap: handclap(); break;
		case addr_monks2text: monks2text(); break;
		case addr_intro1text: intro1text(); break;
		case addr_intro2text: intro2text(); break;
		case addr_intro3text: intro3text(); break;
		case addr_monkandryan: monkandryan(); break;
		case addr_endgameseq: endgameseq(); break;
		case addr_rollendcredits: rollendcredits(); break;
		case addr_priest: priest(); break;
		case addr_madmanstelly: madmanstelly(); break;
		case addr_madman: madman(); break;
		case addr_madmantext: madmantext(); break;
		case addr_madmode: madmode(); break;
		case addr_priesttext: priesttext(); break;
		case addr_textforend: textforend(); break;
		case addr_textformonk: textformonk(); break;
		case addr_drunk: drunk(); break;
		case addr_advisor: advisor(); break;
		case addr_copper: copper(); break;
		case addr_sparky: sparky(); break;
		case addr_train: train(); break;
		case addr_addtopeoplelist: addtopeoplelist(); break;
		case addr_showgamereel: showgamereel(); break;
		case addr_checkspeed: checkspeed(); break;
		case addr_delsprite: delsprite(); break;
		case addr_checkone: checkone(); break;
		case addr_findsource: findsource(); break;
		case addr_mainman: mainman(); break;
		case addr_aboutturn: aboutturn(); break;
		case addr_facerightway: facerightway(); break;
		case addr_checkforexit: checkforexit(); break;
		case addr_adjustdown: adjustdown(); break;
		case addr_adjustup: adjustup(); break;
		case addr_adjustleft: adjustleft(); break;
		case addr_adjustright: adjustright(); break;
		case addr_reminders: reminders(); break;
		case addr_initrain: initrain(); break;
		case addr_splitintolines: splitintolines(); break;
		case addr_getblockofpixel: getblockofpixel(); break;
		case addr_showrain: showrain(); break;
		case addr_backobject: backobject(); break;
		case addr_liftnoise: liftnoise(); break;
		case addr_random: random(); break;
		case addr_steady: steady(); break;
		case addr_constant: constant(); break;
		case addr_doorway: doorway(); break;
		case addr_widedoor: widedoor(); break;
		case addr_lockeddoorway: lockeddoorway(); break;
		case addr_updatepeople: updatepeople(); break;
		case addr_getreelframeax: getreelframeax(); break;
		case addr_reelsonscreen: reelsonscreen(); break;
		case addr_plotreel: plotreel(); break;
		case addr_soundonreels: soundonreels(); break;
		case addr_reconstruct: reconstruct(); break;
		case addr_dealwithspecial: dealwithspecial(); break;
		case addr_movemap: movemap(); break;
		case addr_getreelstart: getreelstart(); break;
		case addr_showreelframe: showreelframe(); break;
		case addr_deleverything: deleverything(); break;
		case addr_dumpeverything: dumpeverything(); break;
		case addr_allocatework: allocatework(); break;
		case addr_showpcx: showpcx(); break;
		case addr_loadpalfromiff: loadpalfromiff(); break;
		case addr_setmode: setmode(); break;
		case addr_paneltomap: paneltomap(); break;
		case addr_maptopanel: maptopanel(); break;
		case addr_dumpmap: dumpmap(); break;
		case addr_pixelcheckset: pixelcheckset(); break;
		case addr_createpanel: createpanel(); break;
		case addr_createpanel2: createpanel2(); break;
		case addr_clearwork: clearwork(); break;
		case addr_vsync: vsync(); break;
		case addr_doshake: doshake(); break;
		case addr_zoom: zoom(); break;
		case addr_delthisone: delthisone(); break;
		case addr_doblocks: doblocks(); break;
		case addr_transferinv: transferinv(); break;
		case addr_transfermap: transfermap(); break;
		case addr_fadedos: fadedos(); break;
		case addr_dofade: dofade(); break;
		case addr_clearendpal: clearendpal(); break;
		case addr_clearpalette: clearpalette(); break;
		case addr_fadescreenup: fadescreenup(); break;
		case addr_fadetowhite: fadetowhite(); break;
		case addr_fadefromwhite: fadefromwhite(); break;
		case addr_fadescreenups: fadescreenups(); break;
		case addr_fadescreendownhalf: fadescreendownhalf(); break;
		case addr_fadescreenuphalf: fadescreenuphalf(); break;
		case addr_fadescreendown: fadescreendown(); break;
		case addr_fadescreendowns: fadescreendowns(); break;
		case addr_clearstartpal: clearstartpal(); break;
		case addr_showgun: showgun(); break;
		case addr_rollendcredits2: rollendcredits2(); break;
		case addr_rollem: rollem(); break;
		case addr_fadecalculation: fadecalculation(); break;
		case addr_greyscalesum: greyscalesum(); break;
		case addr_showgroup: showgroup(); break;
		case addr_paltostartpal: paltostartpal(); break;
		case addr_endpaltostart: endpaltostart(); break;
		case addr_startpaltoend: startpaltoend(); break;
		case addr_paltoendpal: paltoendpal(); break;
		case addr_allpalette: allpalette(); break;
		case addr_dumpcurrent: dumpcurrent(); break;
		case addr_fadedownmon: fadedownmon(); break;
		case addr_fadeupmon: fadeupmon(); break;
		case addr_fadeupmonfirst: fadeupmonfirst(); break;
		case addr_fadeupyellows: fadeupyellows(); break;
		case addr_initialmoncols: initialmoncols(); break;
		case addr_titles: titles(); break;
		case addr_endgame: endgame(); break;
		case addr_monkspeaking: monkspeaking(); break;
		case addr_showmonk: showmonk(); break;
		case addr_gettingshot: gettingshot(); break;
		case addr_credits: credits(); break;
		case addr_biblequote: biblequote(); break;
		case addr_hangone: hangone(); break;
		case addr_intro: intro(); break;
		case addr_runintroseq: runintroseq(); break;
		case addr_runendseq: runendseq(); break;
		case addr_loadintroroom: loadintroroom(); break;
		case addr_mode640x480: mode640x480(); break;
		case addr_set16colpalette: set16colpalette(); break;
		case addr_realcredits: realcredits(); break;
		case addr_waitframes: waitframes(); break;
		case addr_monprint: monprint(); break;
		case addr_fillryan: fillryan(); break;
		case addr_fillopen: fillopen(); break;
		case addr_findallryan: findallryan(); break;
		case addr_findallopen: findallopen(); break;
		case addr_obtoinv: obtoinv(); break;
		case addr_isitworn: isitworn(); break;
		case addr_makeworn: makeworn(); break;
		case addr_examineob: examineob(); break;
		case addr_makemainscreen: makemainscreen(); break;
		case addr_getbackfromob: getbackfromob(); break;
		case addr_incryanpage: incryanpage(); break;
		case addr_openinv: openinv(); break;
		case addr_showryanpage: showryanpage(); break;
		case addr_openob: openob(); break;
		case addr_obicons: obicons(); break;
		case addr_examicon: examicon(); break;
		case addr_obpicture: obpicture(); break;
		case addr_describeob: describeob(); break;
		case addr_additionaltext: additionaltext(); break;
		case addr_obsthatdothings: obsthatdothings(); break;
		case addr_getobtextstart: getobtextstart(); break;
		case addr_searchforsame: searchforsame(); break;
		case addr_findnextcolon: findnextcolon(); break;
		case addr_inventory: inventory(); break;
		case addr_setpickup: setpickup(); break;
		case addr_examinventory: examinventory(); break;
		case addr_reexfrominv: reexfrominv(); break;
		case addr_reexfromopen: reexfromopen(); break;
		case addr_swapwithinv: swapwithinv(); break;
		case addr_swapwithopen: swapwithopen(); break;
		case addr_intoinv: intoinv(); break;
		case addr_deletetaken: deletetaken(); break;
		case addr_outofinv: outofinv(); break;
		case addr_getfreead: getfreead(); break;
		case addr_getexad: getexad(); break;
		case addr_geteitherad: geteitherad(); break;
		case addr_getanyad: getanyad(); break;
		case addr_getanyaddir: getanyaddir(); break;
		case addr_getopenedsize: getopenedsize(); break;
		case addr_getsetad: getsetad(); break;
		case addr_findinvpos: findinvpos(); break;
		case addr_findopenpos: findopenpos(); break;
		case addr_dropobject: dropobject(); break;
		case addr_droperror: droperror(); break;
		case addr_cantdrop: cantdrop(); break;
		case addr_wornerror: wornerror(); break;
		case addr_removeobfrominv: removeobfrominv(); break;
		case addr_selectopenob: selectopenob(); break;
		case addr_useopened: useopened(); break;
		case addr_errormessage1: errormessage1(); break;
		case addr_errormessage2: errormessage2(); break;
		case addr_errormessage3: errormessage3(); break;
		case addr_checkobjectsize: checkobjectsize(); break;
		case addr_outofopen: outofopen(); break;
		case addr_transfertoex: transfertoex(); break;
		case addr_pickupconts: pickupconts(); break;
		case addr_transfercontoex: transfercontoex(); break;
		case addr_transfertext: transfertext(); break;
		case addr_getexpos: getexpos(); break;
		case addr_purgealocation: purgealocation(); break;
		case addr_emergencypurge: emergencypurge(); break;
		case addr_purgeanitem: purgeanitem(); break;
		case addr_deleteexobject: deleteexobject(); break;
		case addr_deleteexframe: deleteexframe(); break;
		case addr_deleteextext: deleteextext(); break;
		case addr_blockget: blockget(); break;
		case addr_drawfloor: drawfloor(); break;
		case addr_calcmapad: calcmapad(); break;
		case addr_getdimension: getdimension(); break;
		case addr_addalong: addalong(); break;
		case addr_addlength: addlength(); break;
		case addr_drawflags: drawflags(); break;
		case addr_showallobs: showallobs(); break;
		case addr_makebackob: makebackob(); break;
		case addr_showallfree: showallfree(); break;
		case addr_showallex: showallex(); break;
		case addr_calcfrframe: calcfrframe(); break;
		case addr_finalframe: finalframe(); break;
		case addr_adjustlen: adjustlen(); break;
		case addr_getmapad: getmapad(); break;
		case addr_getxad: getxad(); break;
		case addr_getyad: getyad(); break;
		case addr_autolook: autolook(); break;
		case addr_look: look(); break;
		case addr_dolook: dolook(); break;
		case addr_redrawmainscrn: redrawmainscrn(); break;
		case addr_getback1: getback1(); break;
		case addr_talk: talk(); break;
		case addr_convicons: convicons(); break;
		case addr_getpersframe: getpersframe(); break;
		case addr_starttalk: starttalk(); break;
		case addr_getpersontext: getpersontext(); break;
		case addr_moretalk: moretalk(); break;
		case addr_dosometalk: dosometalk(); break;
		case addr_hangonpq: hangonpq(); break;
		case addr_redes: redes(); break;
		case addr_newplace: newplace(); break;
		case addr_selectlocation: selectlocation(); break;
		case addr_showcity: showcity(); break;
		case addr_lookatplace: lookatplace(); break;
		case addr_getundercentre: getundercentre(); break;
		case addr_putundercentre: putundercentre(); break;
		case addr_locationpic: locationpic(); break;
		case addr_getdestinfo: getdestinfo(); break;
		case addr_showarrows: showarrows(); break;
		case addr_nextdest: nextdest(); break;
		case addr_lastdest: lastdest(); break;
		case addr_destselect: destselect(); break;
		case addr_getlocation: getlocation(); break;
		case addr_setlocation: setlocation(); break;
		case addr_resetlocation: resetlocation(); break;
		case addr_readdesticon: readdesticon(); break;
		case addr_readcitypic: readcitypic(); break;
		case addr_usemon: usemon(); break;
		case addr_printoutermon: printoutermon(); break;
		case addr_loadpersonal: loadpersonal(); break;
		case addr_loadnews: loadnews(); break;
		case addr_loadcart: loadcart(); break;
		case addr_lookininterface: lookininterface(); break;
		case addr_turnonpower: turnonpower(); break;
		case addr_randomaccess: randomaccess(); break;
		case addr_powerlighton: powerlighton(); break;
		case addr_powerlightoff: powerlightoff(); break;
		case addr_accesslighton: accesslighton(); break;
		case addr_accesslightoff: accesslightoff(); break;
		case addr_locklighton: locklighton(); break;
		case addr_locklightoff: locklightoff(); break;
		case addr_input: input(); break;
		case addr_makecaps: makecaps(); break;
		case addr_delchar: delchar(); break;
		case addr_execcommand: execcommand(); break;
		case addr_neterror: neterror(); break;
		case addr_dircom: dircom(); break;
		case addr_searchforfiles: searchforfiles(); break;
		case addr_signon: signon(); break;
		case addr_showkeys: showkeys(); break;
		case addr_read: read(); break;
		case addr_dirfile: dirfile(); break;
		case addr_getkeyandlogo: getkeyandlogo(); break;
		case addr_searchforstring: searchforstring(); break;
		case addr_parser: parser(); break;
		case addr_scrollmonitor: scrollmonitor(); break;
		case addr_monitorlogo: monitorlogo(); break;
		case addr_printlogo: printlogo(); break;
		case addr_showcurrentfile: showcurrentfile(); break;
		case addr_monmessage: monmessage(); break;
		case addr_processtrigger: processtrigger(); break;
		case addr_triggermessage: triggermessage(); break;
		case addr_printcurs: printcurs(); break;
		case addr_delcurs: delcurs(); break;
		case addr_useobject: useobject(); break;
		case addr_useroutine: useroutine(); break;
		case addr_wheelsound: wheelsound(); break;
		case addr_runtap: runtap(); break;
		case addr_playguitar: playguitar(); break;
		case addr_hotelcontrol: hotelcontrol(); break;
		case addr_hotelbell: hotelbell(); break;
		case addr_opentomb: opentomb(); break;
		case addr_usetrainer: usetrainer(); break;
		case addr_nothelderror: nothelderror(); break;
		case addr_usepipe: usepipe(); break;
		case addr_usefullcart: usefullcart(); break;
		case addr_useplinth: useplinth(); break;
		case addr_chewy: chewy(); break;
		case addr_useladder: useladder(); break;
		case addr_useladderb: useladderb(); break;
		case addr_slabdoora: slabdoora(); break;
		case addr_slabdoorb: slabdoorb(); break;
		case addr_slabdoord: slabdoord(); break;
		case addr_slabdoorc: slabdoorc(); break;
		case addr_slabdoore: slabdoore(); break;
		case addr_slabdoorf: slabdoorf(); break;
		case addr_useslab: useslab(); break;
		case addr_usecart: usecart(); break;
		case addr_useclearbox: useclearbox(); break;
		case addr_usecoveredbox: usecoveredbox(); break;
		case addr_userailing: userailing(); break;
		case addr_useopenbox: useopenbox(); break;
		case addr_wearwatch: wearwatch(); break;
		case addr_wearshades: wearshades(); break;
		case addr_sitdowninbar: sitdowninbar(); break;
		case addr_usechurchhole: usechurchhole(); break;
		case addr_usehole: usehole(); break;
		case addr_usealtar: usealtar(); break;
		case addr_opentvdoor: opentvdoor(); break;
		case addr_usedryer: usedryer(); break;
		case addr_openlouis: openlouis(); break;
		case addr_nextcolon: nextcolon(); break;
		case addr_openyourneighbour: openyourneighbour(); break;
		case addr_usewindow: usewindow(); break;
		case addr_usebalcony: usebalcony(); break;
		case addr_openryan: openryan(); break;
		case addr_openpoolboss: openpoolboss(); break;
		case addr_openeden: openeden(); break;
		case addr_opensarters: opensarters(); break;
		case addr_isitright: isitright(); break;
		case addr_drawitall: drawitall(); break;
		case addr_openhoteldoor: openhoteldoor(); break;
		case addr_openhoteldoor2: openhoteldoor2(); break;
		case addr_grafittidoor: grafittidoor(); break;
		case addr_trapdoor: trapdoor(); break;
		case addr_callhotellift: callhotellift(); break;
		case addr_calledenslift: calledenslift(); break;
		case addr_calledensdlift: calledensdlift(); break;
		case addr_usepoolreader: usepoolreader(); break;
		case addr_uselighter: uselighter(); break;
		case addr_showseconduse: showseconduse(); break;
		case addr_usecardreader1: usecardreader1(); break;
		case addr_usecardreader2: usecardreader2(); break;
		case addr_usecardreader3: usecardreader3(); break;
		case addr_usecashcard: usecashcard(); break;
		case addr_lookatcard: lookatcard(); break;
		case addr_moneypoke: moneypoke(); break;
		case addr_usecontrol: usecontrol(); break;
		case addr_usehatch: usehatch(); break;
		case addr_usewire: usewire(); break;
		case addr_usehandle: usehandle(); break;
		case addr_useelevator1: useelevator1(); break;
		case addr_showfirstuse: showfirstuse(); break;
		case addr_useelevator3: useelevator3(); break;
		case addr_useelevator4: useelevator4(); break;
		case addr_useelevator2: useelevator2(); break;
		case addr_useelevator5: useelevator5(); break;
		case addr_usekey: usekey(); break;
		case addr_usestereo: usestereo(); break;
		case addr_usecooker: usecooker(); break;
		case addr_useaxe: useaxe(); break;
		case addr_useelvdoor: useelvdoor(); break;
		case addr_withwhat: withwhat(); break;
		case addr_selectob: selectob(); break;
		case addr_compare: compare(); break;
		case addr_findsetobject: findsetobject(); break;
		case addr_findexobject: findexobject(); break;
		case addr_isryanholding: isryanholding(); break;
		case addr_checkinside: checkinside(); break;
		case addr_usetext: usetext(); break;
		case addr_putbackobstuff: putbackobstuff(); break;
		case addr_showpuztext: showpuztext(); break;
		case addr_findpuztext: findpuztext(); break;
		case addr_placesetobject: placesetobject(); break;
		case addr_removesetobject: removesetobject(); break;
		case addr_issetobonmap: issetobonmap(); break;
		case addr_placefreeobject: placefreeobject(); break;
		case addr_removefreeobject: removefreeobject(); break;
		case addr_findormake: findormake(); break;
		case addr_switchryanon: switchryanon(); break;
		case addr_switchryanoff: switchryanoff(); break;
		case addr_setallchanges: setallchanges(); break;
		case addr_dochange: dochange(); break;
		case addr_autoappear: autoappear(); break;
		case addr_dumptimedtext: dumptimedtext(); break;
		case addr_setuptimeduse: setuptimeduse(); break;
		case addr_setuptimedtemp: setuptimedtemp(); break;
		case addr_edenscdplayer: edenscdplayer(); break;
		case addr_usewall: usewall(); break;
		case addr_usechurchgate: usechurchgate(); break;
		case addr_usegun: usegun(); break;
		case addr_useshield: useshield(); break;
		case addr_usebuttona: usebuttona(); break;
		case addr_useplate: useplate(); break;
		case addr_usewinch: usewinch(); break;
		case addr_entercode: entercode(); break;
		case addr_loadkeypad: loadkeypad(); break;
		case addr_quitkey: quitkey(); break;
		case addr_addtopresslist: addtopresslist(); break;
		case addr_buttonone: buttonone(); break;
		case addr_buttontwo: buttontwo(); break;
		case addr_buttonthree: buttonthree(); break;
		case addr_buttonfour: buttonfour(); break;
		case addr_buttonfive: buttonfive(); break;
		case addr_buttonsix: buttonsix(); break;
		case addr_buttonseven: buttonseven(); break;
		case addr_buttoneight: buttoneight(); break;
		case addr_buttonnine: buttonnine(); break;
		case addr_buttonnought: buttonnought(); break;
		case addr_buttonenter: buttonenter(); break;
		case addr_buttonpress: buttonpress(); break;
		case addr_showouterpad: showouterpad(); break;
		case addr_showkeypad: showkeypad(); break;
		case addr_singlekey: singlekey(); break;
		case addr_dumpkeypad: dumpkeypad(); break;
		case addr_usemenu: usemenu(); break;
		case addr_dumpmenu: dumpmenu(); break;
		case addr_getundermenu: getundermenu(); break;
		case addr_putundermenu: putundermenu(); break;
		case addr_showoutermenu: showoutermenu(); break;
		case addr_showmenu: showmenu(); break;
		case addr_loadmenu: loadmenu(); break;
		case addr_viewfolder: viewfolder(); break;
		case addr_nextfolder: nextfolder(); break;
		case addr_folderhints: folderhints(); break;
		case addr_lastfolder: lastfolder(); break;
		case addr_loadfolder: loadfolder(); break;
		case addr_showfolder: showfolder(); break;
		case addr_folderexit: folderexit(); break;
		case addr_showleftpage: showleftpage(); break;
		case addr_showrightpage: showrightpage(); break;
		case addr_entersymbol: entersymbol(); break;
		case addr_quitsymbol: quitsymbol(); break;
		case addr_settopleft: settopleft(); break;
		case addr_settopright: settopright(); break;
		case addr_setbotleft: setbotleft(); break;
		case addr_setbotright: setbotright(); break;
		case addr_dumpsymbol: dumpsymbol(); break;
		case addr_showsymbol: showsymbol(); break;
		case addr_nextsymbol: nextsymbol(); break;
		case addr_updatesymboltop: updatesymboltop(); break;
		case addr_updatesymbolbot: updatesymbolbot(); break;
		case addr_dumpsymbox: dumpsymbox(); break;
		case addr_usediary: usediary(); break;
		case addr_showdiary: showdiary(); break;
		case addr_showdiarykeys: showdiarykeys(); break;
		case addr_dumpdiarykeys: dumpdiarykeys(); break;
		case addr_diarykeyp: diarykeyp(); break;
		case addr_diarykeyn: diarykeyn(); break;
		case addr_showdiarypage: showdiarypage(); break;
		case addr_findtext1: findtext1(); break;
		case addr_zoomonoff: zoomonoff(); break;
		case addr_saveload: saveload(); break;
		case addr_dosaveload: dosaveload(); break;
		case addr_getbackfromops: getbackfromops(); break;
		case addr_showmainops: showmainops(); break;
		case addr_showdiscops: showdiscops(); break;
		case addr_loadsavebox: loadsavebox(); break;
		case addr_loadgame: loadgame(); break;
		case addr_getbacktoops: getbacktoops(); break;
		case addr_discops: discops(); break;
		case addr_savegame: savegame(); break;
		case addr_actualsave: actualsave(); break;
		case addr_actualload: actualload(); break;
		case addr_selectslot2: selectslot2(); break;
		case addr_checkinput: checkinput(); break;
		case addr_getnamepos: getnamepos(); break;
		case addr_showopbox: showopbox(); break;
		case addr_showloadops: showloadops(); break;
		case addr_showsaveops: showsaveops(); break;
		case addr_selectslot: selectslot(); break;
		case addr_showslots: showslots(); break;
		case addr_shownames: shownames(); break;
		case addr_dosreturn: dosreturn(); break;
		case addr_error: error(); break;
		case addr_namestoold: namestoold(); break;
		case addr_oldtonames: oldtonames(); break;
		case addr_savefilewrite: savefilewrite(); break;
		case addr_savefileread: savefileread(); break;
		case addr_saveposition: saveposition(); break;
		case addr_loadposition: loadposition(); break;
		case addr_loadseg: loadseg(); break;
		case addr_makeheader: makeheader(); break;
		case addr_storeit: storeit(); break;
		case addr_saveseg: saveseg(); break;
		case addr_findlen: findlen(); break;
		case addr_scanfornames: scanfornames(); break;
		case addr_decide: decide(); break;
		case addr_showdecisions: showdecisions(); break;
		case addr_newgame: newgame(); break;
		case addr_loadold: loadold(); break;
		case addr_loadspeech: loadspeech(); break;
		case addr_createname: createname(); break;
		case addr_loadsample: loadsample(); break;
		case addr_loadsecondsample: loadsecondsample(); break;
		case addr_soundstartup: soundstartup(); break;
		case addr_trysoundalloc: trysoundalloc(); break;
		case addr_setsoundoff: setsoundoff(); break;
		case addr_checksoundint: checksoundint(); break;
		case addr_enablesoundint: enablesoundint(); break;
		case addr_disablesoundint: disablesoundint(); break;
		case addr_interupttest: interupttest(); break;
		case addr_soundend: soundend(); break;
		case addr_out22c: out22c(); break;
		case addr_playchannel0: playchannel0(); break;
		case addr_playchannel1: playchannel1(); break;
		case addr_makenextblock: makenextblock(); break;
		case addr_volumeadjust: volumeadjust(); break;
		case addr_loopchannel0: loopchannel0(); break;
		case addr_channel0only: channel0only(); break;
		case addr_channel1only: channel1only(); break;
		case addr_channel0tran: channel0tran(); break;
		case addr_bothchannels: bothchannels(); break;
		case addr_saveems: saveems(); break;
		case addr_restoreems: restoreems(); break;
		case addr_domix: domix(); break;
		case addr_dmaend: dmaend(); break;
		case addr_startdmablock: startdmablock(); break;
		case addr_setuppit: setuppit(); break;
		case addr_getridofpit: getridofpit(); break;
		case addr_pitinterupt: pitinterupt(); break;
		case addr_dreamweb: dreamweb(); break;
		case addr_entrytexts: entrytexts(); break;
		case addr_entryanims: entryanims(); break;
		case addr_initialinv: initialinv(); break;
		case addr_pickupob: pickupob(); break;
		case addr_setupemm: setupemm(); break;
		case addr_removeemm: removeemm(); break;
		case addr_checkforemm: checkforemm(); break;
		case addr_checkbasemem: checkbasemem(); break;
		case addr_allocatebuffers: allocatebuffers(); break;
		case addr_clearbuffers: clearbuffers(); break;
		case addr_clearchanges: clearchanges(); break;
		case addr_clearbeforeload: clearbeforeload(); break;
		case addr_clearreels: clearreels(); break;
		case addr_clearrest: clearrest(); break;
		case addr_deallocatemem: deallocatemem(); break;
		case addr_allocatemem: allocatemem(); break;
		case addr_parseblaster: parseblaster(); break;
		case addr_startup: startup(); break;
		case addr_startup1: startup1(); break;
		case addr_screenupdate: screenupdate(); break;
		case addr_watchreel: watchreel(); break;
		case addr_checkforshake: checkforshake(); break;
		case addr_watchcount: watchcount(); break;
		case addr_showtime: showtime(); break;
		case addr_dumpwatch: dumpwatch(); break;
		case addr_showbyte: showbyte(); break;
		case addr_onedigit: onedigit(); break;
		case addr_twodigitnum: twodigitnum(); break;
		case addr_showword: showword(); break;
		case addr_convnum: convnum(); break;
		case addr_mainscreen: mainscreen(); break;
		case addr_madmanrun: madmanrun(); break;
		case addr_checkcoords: checkcoords(); break;
		case addr_identifyob: identifyob(); break;
		case addr_checkifperson: checkifperson(); break;
		case addr_checkifset: checkifset(); break;
		case addr_checkifex: checkifex(); break;
		case addr_checkiffree: checkiffree(); break;
		case addr_isitdescribed: isitdescribed(); break;
		case addr_findpathofpoint: findpathofpoint(); break;
		case addr_findfirstpath: findfirstpath(); break;
		case addr_turnpathon: turnpathon(); break;
		case addr_turnpathoff: turnpathoff(); break;
		case addr_turnanypathon: turnanypathon(); break;
		case addr_turnanypathoff: turnanypathoff(); break;
		case addr_checkifpathison: checkifpathison(); break;
		case addr_afternewroom: afternewroom(); break;
		case addr_atmospheres: atmospheres(); break;
		case addr_walkintoroom: walkintoroom(); break;
		case addr_afterintroroom: afterintroroom(); break;
		case addr_obname: obname(); break;
		case addr_finishedwalking: finishedwalking(); break;
		case addr_examineobtext: examineobtext(); break;
		case addr_commandwithob: commandwithob(); break;
		case addr_commandonly: commandonly(); break;
		case addr_printmessage: printmessage(); break;
		case addr_printmessage2: printmessage2(); break;
		case addr_blocknametext: blocknametext(); break;
		case addr_personnametext: personnametext(); break;
		case addr_walktotext: walktotext(); break;
		case addr_getflagunderp: getflagunderp(); break;
		case addr_setwalk: setwalk(); break;
		case addr_autosetwalk: autosetwalk(); break;
		case addr_checkdest: checkdest(); break;
		case addr_bresenhams: bresenhams(); break;
		case addr_workoutframes: workoutframes(); break;
		case addr_getroomspaths: getroomspaths(); break;
		case addr_copyname: copyname(); break;
		case addr_findobname: findobname(); break;
		case addr_showicon: showicon(); break;
		case addr_middlepanel: middlepanel(); break;
		case addr_showman: showman(); break;
		case addr_showpanel: showpanel(); break;
		case addr_roomname: roomname(); break;
		case addr_usecharset1: usecharset1(); break;
		case addr_usetempcharset: usetempcharset(); break;
		case addr_showexit: showexit(); break;
		case addr_panelicons1: panelicons1(); break;
		case addr_showwatch: showwatch(); break;
		case addr_gettime: gettime(); break;
		case addr_zoomicon: zoomicon(); break;
		case addr_showblink: showblink(); break;
		case addr_dumpblink: dumpblink(); break;
		case addr_worktoscreenm: worktoscreenm(); break;
		case addr_blank: blank(); break;
		case addr_allpointer: allpointer(); break;
		case addr_hangonp: hangonp(); break;
		case addr_hangonw: hangonw(); break;
		case addr_hangoncurs: hangoncurs(); break;
		case addr_getunderzoom: getunderzoom(); break;
		case addr_dumpzoom: dumpzoom(); break;
		case addr_putunderzoom: putunderzoom(); break;
		case addr_crosshair: crosshair(); break;
		case addr_showpointer: showpointer(); break;
		case addr_delpointer: delpointer(); break;
		case addr_dumppointer: dumppointer(); break;
		case addr_undertextline: undertextline(); break;
		case addr_deltextline: deltextline(); break;
		case addr_animpointer: animpointer(); break;
		case addr_setmouse: setmouse(); break;
		case addr_readmouse: readmouse(); break;
		case addr_mousecall: mousecall(); break;
		case addr_readmouse1: readmouse1(); break;
		case addr_readmouse2: readmouse2(); break;
		case addr_readmouse3: readmouse3(); break;
		case addr_readmouse4: readmouse4(); break;
		case addr_readkey: readkey(); break;
		case addr_randomnum1: randomnum1(); break;
		case addr_randomnum2: randomnum2(); break;
		case addr_hangon: hangon(); break;
		case addr_loadtraveltext: loadtraveltext(); break;
		case addr_loadintotemp: loadintotemp(); break;
		case addr_loadintotemp2: loadintotemp2(); break;
		case addr_loadintotemp3: loadintotemp3(); break;
		case addr_loadtempcharset: loadtempcharset(); break;
		case addr_standardload: standardload(); break;
		case addr_loadtemptext: loadtemptext(); break;
		case addr_loadroom: loadroom(); break;
		case addr_loadroomssample: loadroomssample(); break;
		case addr_getridofreels: getridofreels(); break;
		case addr_getridofall: getridofall(); break;
		case addr_restorereels: restorereels(); break;
		case addr_restoreall: restoreall(); break;
		case addr_sortoutmap: sortoutmap(); break;
		case addr_startloading: startloading(); break;
		case addr_disablepath: disablepath(); break;
		case addr_findxyfrompath: findxyfrompath(); break;
		case addr_findroominloc: findroominloc(); break;
		case addr_getroomdata: getroomdata(); break;
		case addr_readheader: readheader(); break;
		case addr_dontloadseg: dontloadseg(); break;
		case addr_allocateload: allocateload(); break;
		case addr_fillspace: fillspace(); break;
		case addr_getridoftemp: getridoftemp(); break;
		case addr_getridoftemptext: getridoftemptext(); break;
		case addr_getridoftemp2: getridoftemp2(); break;
		case addr_getridoftemp3: getridoftemp3(); break;
		case addr_getridoftempcharset: getridoftempcharset(); break;
		case addr_getridoftempsp: getridoftempsp(); break;
		case addr_readsetdata: readsetdata(); break;
		case addr_createfile: createfile(); break;
		case addr_openfile: openfile(); break;
		case addr_openfilefromc: openfilefromc(); break;
		case addr_makename: makename(); break;
		case addr_openfilenocheck: openfilenocheck(); break;
		case addr_openforsave: openforsave(); break;
		case addr_closefile: closefile(); break;
		case addr_readfromfile: readfromfile(); break;
		case addr_setkeyboardint: setkeyboardint(); break;
		case addr_resetkeyboard: resetkeyboard(); break;
		case addr_keyboardread: keyboardread(); break;
		case addr_walkandexamine: walkandexamine(); break;
		case addr_doload: doload(); break;
		case addr_generalerror: generalerror(); break;
		default: ::error("invalid call to %04x dispatched", (uint16)ax);
	}
}

} /*namespace*/
