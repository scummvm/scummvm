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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 LUCERNA          The screen, [keyboard] and mouse handler. */

#define __lucerna_implementation__


#include "lucerna.h"


#include "Graph.h"
/*#include "Dos.h"*/
/*#include "Crt.h"*/
#include "trip5.h"
#include "Acci.h"
#include "pingo.h"
#include "scrolls.h"
#include "enhanced.h"
#include "dropdown.h"
#include "logger.h"
#include "visa.h"
#include "celer.h"
#include "timeout.h"
#include "basher.h"
#include "sequence.h"

namespace Avalanche {

/*$V-*/ /*$S-*/
boolean fxhidden;
array<0, 3, palettetype> fxpal;

void callverb(char n) {
	if (n == pardon) {
		display(string("The f5 key lets you do a particular action in certain ") +
		        "situations. However, at the moment there is nothing " +
		        "assigned to it. You may press alt-A to see what the " +
		        "current setting of this key is.");
	} else {
		weirdword = false;
		polite = true;
		verb = n;
		do_that();
	}
}

void draw_also_lines() {
	byte ff;
	byte squeaky_code;

	switch (visible) {
	case m_virtual : {
		squeaky_code = 1;
		off_virtual();
	}
	break;
	case m_no      :
		squeaky_code = 2;
		break;
	case m_yes     : {
		squeaky_code = 3;
		off();
	}
	break;
	}

	setactivepage(2);
	cleardevice();
	setcolor(15);
	rectangle(0, 45, 639, 160);
	for (ff = 1; ff <= 50; ff ++) {
		linetype &with = lines[ff];
		if (x1 != maxint) {
			setcolor(with.col);
			line(x1, y1, x2, y2);
		}
	}

	switch (squeaky_code) {
	case 1 :
		on_virtual();
		break;
	case 2 :
		;
		break; /* zzzz, it was off anyway */
	case 3 :
		on();
		break;
	}
}

void load_also(string n);

static untyped_file f;


static string nextstring() {
	byte l;
	string x;

	string nextstring_result;
	blockread(f, l, 1);
	blockread(f, x[1], l);
	x[0] = chr(l);
	nextstring_result = x;
	return nextstring_result;
}

static void unscramble();

static void scram1(string &x) {
	byte fz;
	;
	for (fz = 1; fz <= length(x); fz ++)
		x[fz] = chr(ord(x[fz]) ^ 177);
}



static void unscramble() {
	byte fv, ff;

	for (fv = 0; fv <= 30; fv ++)
		for (ff = 0; ff <= 1; ff ++)
			if (also[fv][ff] != nil)
				scram1(*also[fv][ff]);
	scram1(listen);
	scram1(flags);
	/*     for fz:=1 to length(also[fv,ff]^) do
	      also[fv,ff]^[fz]:=chr(ord(also[fv,ff]^[fz]) xor 177);*/
}

void load_also(string n) {
	byte minnames;
	byte ff, fv;


	for (fv = 0; fv <= 30; fv ++)
		for (ff = 0; ff <= 1; ff ++)
			if (also[fv][ff] != nil)  {
				delete also[fv][ff];
				also[fv][ff] = nil;
			}
	assign(f, string("also") + n + ".avd");
	/*$I-*/ reset(f, 1); /*$I+*/ if (ioresult != 0)  return; /* no Also file */
	seek(f, 128);
	blockread(f, minnames, 1);
	for (fv = 0; fv <= minnames; fv ++) {
		for (ff = 0; ff <= 1; ff ++) {
			also[fv][ff] = new string;
			*also[fv][ff] = nextstring();
		}
		*also[fv][0] = string('\235') + *also[fv][0] + '\235';
	}
	fillchar(lines, sizeof(lines), 0xff);

	fv = getpixel(0, 0);
	blockread(f, fv, 1);
	blockread(f, lines, sizeof(lines[1])*fv);
	blockread(f, fv, 1);
	fillchar(peds, sizeof(peds), '\261');
	blockread(f, peds, sizeof(peds[1])*fv);
	blockread(f, numfields, 1);
	blockread(f, fields, sizeof(fields[1])*numfields);
	blockread(f, magics, sizeof(magics));
	blockread(f, portals, sizeof(portals));
	blockread(f, flags, sizeof(flags));
	blockread(f, listen[0], 1);
	blockread(f, listen[1], length(listen));
	draw_also_lines();

	setactivepage(1);
	close(f);
	unscramble();
	for (fv = 0; fv <= minnames; fv ++)
		*also[fv][0] = string(',') + *also[fv][0] + ',';
}

void load(byte n) {     /* Load2, actually */
	byte a0;  /*absolute $A000:800;*/
	byte a1;  /*absolute $A000:17184;*/
	byte bit;
	untyped_file f;
	varying_string<2> xx;
	boolean was_virtual;

	was_virtual = visible == m_virtual;
	if (was_virtual)  off_virtual();
	else off();
	clear_vmc();

	xx = strf(n);
	flesh_colours();
	assign(f, string("place") + xx + ".avd");
	reset(f, 1);
	seek(f, 146);
	blockread(f, roomname, 30);
	/* Compression method byte follows this... */
	seek(f, 177);
	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockread(f, a0, 12080);
		move(a0, a1, 12080);
	}
	close(f);
	load_also(xx);
	load_chunks(xx);

	copy03();
	bit = getpixel(0, 0);
	log_newroom(roomname);

	if (was_virtual)  on_virtual();
	else on();
}

void zoomout(integer x, integer y) {
	integer x1, y1, x2, y2;
	byte fv;

	setcolor(white);
	setwritemode(xorput);
	setlinestyle(dottedln, 0, 1);

	for (fv = 1; fv <= 20; fv ++) {
		x1 = x - (x / 20) * fv;
		y1 = y - ((y - 10) / 20) * fv;
		x2 = x + (((639 - x) / 20) * fv);
		y2 = y + (((161 - y) / 20) * fv);

		rectangle(x1, y1, x2, y2);
		delay(17);
		rectangle(x1, y1, x2, y2);
	}
	setwritemode(copyput);
	setlinestyle(0, 0, 1);
}

void find_people(byte room) {
	char fv;

	for (fv = '\227'; fv <= '\262'; fv ++)
		if (whereis[fv] == room) {
			if (fv < '\257')  him = fv;
			else her = fv;
		}
}

void exitroom(byte x) {
	nosound;
	forget_chunks();
	seescroll = true;  /* This stops the trippancy system working over the
  length of this procedure. */

	{
		dnatype &with = dna;
		switch (x) {
		case r__spludwicks: {
			lose_timer(reason_avariciustalks);
			/* He doesn't HAVE to be talking for this to work. It just deletes it IF it
			exists. */        with.avaricius_talk = 0;
		}
		break;
		case r__bridge:
			if (with.drawbridge_open > 0) {
				with.drawbridge_open = 4; /* Fully open. */
				lose_timer(reason_drawbridgefalls);
			}
			break;
		case r__outsidecardiffcastle:
			lose_timer(reason_cardiffsurvey);
			break;

		case r__robins:
			lose_timer(reason_getting_tied_up);
			break;
		}
	}

	interrogation = 0; /* Leaving the room cancels all the questions automatically. */

	seescroll = false; /* Now it can work again! */

	dna.last_room = dna.room;
	if (dna.room != r__map)
		dna.last_room_not_map = dna.room;
}

void new_town() {   /* You've just entered a town from the map. */
	standard_bar();

	switch (dna.room) {
	case r__outsidenottspub: /* Entry into Nottingham. */
		if ((dna.rooms[r__robins] > 0) && (dna.been_tied_up) &&
		        (! dna.taken_mushroom))
			dna.mushroom_growing = true;
		break;
	case r__wisewomans: { /* Entry into Argent. */
		if (dna.talked_to_crapulus && (! dna.lustie_is_asleep)) {
			dna.spludwicks_here = !((dna.rooms[r__wisewomans] % 3) == 1);
			dna.crapulus_will_tell = ! dna.spludwicks_here;
		} else {
			dna.spludwicks_here = true;
			dna.crapulus_will_tell = false;
		}
		if (dna.box_contents == wine)  dna.winestate = 3; /* Vinegar */
	}
	break;
	}

	if (dna.room != r__outsideducks) {
		if ((dna.obj[onion]) && !(dna.onion_in_vinegar))
			dna.rotten_onion = true; /* You're holding the onion */
	}
}

void enterroom(byte x, byte ped);

static void put_geida_at(byte whichped, byte &ped) {
	if (ped == 0)  return;
	tr[2].init(5, false); /* load Geida */
	apped(2, whichped);
	tr[2].call_eachstep = true;
	tr[2].eachstep = procgeida_procs;
}

void enterroom(byte x, byte ped) {

	seescroll = true;  /* This stops the trippancy system working over the
  length of this procedure. */

	find_people(x);
	dna.room = x;
	if (ped != 0)  dna.rooms[x] += 1;

	load(x);

	if ((dna.rooms[x] == 0) && (! flagset('S')))  points(1);
	whereis[pavalot] = dna.room;
	if (dna.geida_follows)  whereis[pgeida] = x;
	roomtime = 0;

	{
		dnatype &with = dna;
		if ((with.last_room == r__map) && (with.last_room_not_map != with.room))
			new_town();
	}

	switch (x) {
	case r__yours:
		if (dna.avvy_in_bed) {
			show_one(3);
			set_up_timer(100, procarkata_shouts, reason_arkata_shouts);
		}
		break;

	case r__outsideyours:
		if (ped > 0) {
			if (! dna.talked_to_crapulus) {

				whereis[pcrapulus] = r__outsideyours;
				tr[2].init(8, false); /* load Crapulus */

				if (dna.rooms[r__outsideyours] == 1) {
					apped(2, 4); /* Start on the right-hand side of the screen. */
					tr[2].walkto(5); /* Walks up to greet you. */
				} else {
					apped(2, 5); /* Starts where he was before. */
					tr[2].face = 3;
				}

				tr[2].call_eachstep = true;
				tr[2].eachstep = procface_avvy; /* He always faces Avvy. */

			} else whereis[pcrapulus] = r__nowhere;

			if (dna.crapulus_will_tell) {
				tr[2].init(8, false);
				apped(2, 2);
				tr[2].walkto(4);
				set_up_timer(20, proccrapulus_splud_out, reason_crapulus_says_spludwick_out);
				dna.crapulus_will_tell = false;
			}
		}
		break;

	case r__outsidespludwicks:
		if ((dna.rooms[r__outsidespludwicks] == 1) && (ped == 1)) {
			set_up_timer(20, procbang, reason_explosion);
			dna.spludwicks_here = true;
		}
		break;

	case r__spludwicks:
		if (dna.spludwicks_here) {
			if (ped > 0) {
				tr[2].init(2, false); /* load Spludwick */
				apped(2, 2);
				whereis['\227'] = r__spludwicks;
			}

			dna.dogfoodpos = 0;  /* Also Spludwick pos. */

			tr[2].call_eachstep = true;
			tr[2].eachstep = procgeida_procs;
		} else whereis['\227'] = r__nowhere;
		break;

	case r__brummieroad: {
		if (dna.geida_follows)  put_geida_at(5, ped);
		if (dna.cwytalot_gone) {
			magics[lightred].op = nix;
			whereis[pcwytalot] = r__nowhere;
		} else {
			if (ped > 0) {
				tr[2].init(4, false); /* 4=Cwytalot*/
				tr[2].call_eachstep = true;
				tr[2].eachstep = procfollow_avvy_y;
				whereis[pcwytalot] = r__brummieroad;

				if (dna.rooms[r__brummieroad] == 1) { /* First time here... */
					apped(2, 2); /* He appears on the right of the screen... */
					tr[2].walkto(4); /* ...and he walks up... */
				} else {
					/* You've been here before. */
					apped(2, 4); /* He's standing in your way straight away... */
					tr[2].face = left;
				}
			}
		}
	}
	break;

	case r__argentroad: {
		dnatype &with = dna;
		if ((with.cwytalot_gone) && (! with.cwytalot_in_herts) && (ped == 2) &&
		        (dna.rooms[r__argentroad] > 3)) {
			tr[2].init(4, false); /* 4=Cwytalot again*/
			apped(2, 1);
			tr[2].walkto(2);
			tr[2].vanishifstill = true;
			with.cwytalot_in_herts = true;
			/*whereis[#157]:=r__Nowhere;*/ /* can we fit this in? */
			set_up_timer(20, proc_cwytalot_in_herts, reason_cwytalot_in_herts);
		}
	}
	break;

	case r__bridge: {
		if (dna.drawbridge_open == 4) { /*open*/
			show_one(3); /* Position of drawbridge */
			magics[green].op = nix; /* You may enter the drawbridge. */
		}
		if (dna.geida_follows)  put_geida_at(ped + 3, ped); /* load Geida */
	}
	break;

	case r__robins: {
		if (ped > 0) {
			if (! dna.been_tied_up) {
				/* A welcome party... or maybe not... */
				tr[2].init(6, false);
				apped(2, 2);
				tr[2].walkto(3);
				set_up_timer(36, procget_tied_up, reason_getting_tied_up);
			}
		}

		if (dna.been_tied_up) {
			whereis[probinhood] = 0;
			whereis[pfriartuck] = 0;
		}

		if (dna.tied_up)  show_one(2);

		if (! dna.mushroom_growing)  show_one(3);
	}
	break;

	case r__outsidecardiffcastle: {
		if (ped > 0)
			switch (dna.cardiff_things) {
			case 0 : { /* You've answered NONE of his questions. */
				tr[2].init(9, false);
				apped(2, 2);
				tr[2].walkto(3);
				set_up_timer(47, proccardiffsurvey, reason_cardiffsurvey);
			}
			break;
			case 5 :
				magics[2].op = nix;
				break; /* You've answered ALL his questions. => nothing happens. */
			default: { /* You've answered SOME of his questions. */
				tr[2].init(9, false);
				apped(2, 3);
				tr[2].face = right;
				set_up_timer(3, proccardiff_return, reason_cardiffsurvey);
			}
			}
		if (dna.cardiff_things < 5)
			interrogation = dna.cardiff_things;
		else interrogation = 0;
	}
	break;

	case r__map: {
		/* You're entering the map. */
		dawn();
		setactivepage(cp);
		if (ped > 0)  zoomout(peds[ped].x, peds[ped].y);
		setactivepage(1 - cp);

		{
			dnatype &with = dna;
			if ((with.obj[wine]) && (with.winestate != 3)) {
				dixi('q', 9); /* Don't want to waste the wine! */
				with.obj[wine] = false;
				objectlist();
			}
		}

		dixi('q', 69);
	}
	break;

	case r__catacombs: {
		if (set::of(0, 3, 5, 6, eos).has(ped)) {
			dnatype &with = dna;

			switch (ped) {
			case 3 : {
				with.cat_x = 8;
				with.cat_y = 4;
			}
			break; /* Enter from oubliette */
			case 5 : {
				with.cat_x = 8;
				with.cat_y = 7;
			}
			break; /* Enter from du Lustie's */
			case 6 : {
				with.cat_x = 4;
				with.cat_y = 1;
			}
			break; /* Enter from Geida's */
			}
			dna.enter_catacombs_from_lusties_room = true;
			catamove(ped);
			dna.enter_catacombs_from_lusties_room = false;
		}
	}
	break;

	case r__argentpub: {
		if (dna.wonnim)  show_one(1);   /* No lute by the settle. */
		dna.malagauche = 0; /* Ready to boot Malagauche */
		if (dna.givenbadgetoiby) {
			show_one(8);
			show_one(9);
		}
	}
	break;

	case r__lustiesroom: {
		dna.dogfoodpos = 1; /* Actually, du Lustie pos. */
		if (tr[1].whichsprite == 0) /* Avvy in his normal clothes */
			set_up_timer(3, proccallsguards, reason_du_lustie_talks);
		else if (! dna.entered_lusties_room_as_monk) /*already*/
			/* Presumably, Avvy dressed as a monk. */
			set_up_timer(3, procgreetsmonk, reason_du_lustie_talks);

		if (dna.geida_follows) {
			put_geida_at(5, ped);
			if (dna.lustie_is_asleep)  show_one(5);
		}
	}
	break;

	case r__musicroom: {
		if (dna.jacques_awake > 0) {
			dna.jacques_awake = 5;
			show_one(2);
			show_one(4);
			magics[brown].op = nix;
			whereis[pjacques] = 0;
		}
		if (ped != 0) {
			show_one(6);
			first_show(5);
			then_show(7);
			start_to_close();
		}
	}
	break;

	case r__outsidenottspub:
		if (ped == 2) {
			show_one(3);
			first_show(2);
			then_show(1);
			then_show(4);
			start_to_close();
		}
		break;

	case r__outsideargentpub:
		if (ped == 2)  {
			show_one(6);
			first_show(5);
			then_show(7);
			start_to_close();
		}
		break;

	case r__wisewomans: {
		tr[2].init(11, false);
		if ((dna.rooms[r__wisewomans] == 1) && (ped > 0)) {
			apped(2, 2); /* Start on the right-hand side of the screen. */
			tr[2].walkto(4); /* Walks up to greet you. */
		} else {
			apped(2, 4); /* Starts where she was before. */
			tr[2].face = 3;
		}

		tr[2].call_eachstep = true;
		tr[2].eachstep = procface_avvy; /* She always faces Avvy. */
	}
	break;

	case r__insidecardiffcastle:
		if (ped > 0) {
			tr[2].init(10, false); /* Define the dart. */
			first_show(1);
			if (dna.arrow_in_the_door)  then_show(3);
			else then_show(2);
			if (dna.taken_pen)  show_one(4);
			start_to_close();
		} else {
			show_one(1);
			if (dna.arrow_in_the_door)  show_one(3);
			else show_one(2);
		}
		break;

	case r__avvysgarden:
		if (ped == 1)  {
			show_one(2);
			first_show(1);
			then_show(3);
			start_to_close();
		}
		break;

	case r__entrancehall:
	case r__insideabbey:
		if (ped == 2)  {
			show_one(2);
			first_show(1);
			then_show(3);
			start_to_close();
		}
		break;

	case r__aylesoffice:
		if (dna.ayles_is_awake)  show_one(2);
		break; /* Ayles awake. */

	case r__geidas:
		put_geida_at(2, ped);
		break; /* load Geida */

	case r__easthall:
	case r__westhall:
		if (dna.geida_follows)  put_geida_at(ped + 2, ped);
		break;

	case r__lusties:
		if (dna.geida_follows)  put_geida_at(ped + 6, ped);
		break;

	case r__nottspub: {
		if (dna.sitting_in_pub)  show_one(3);
		dna.dogfoodpos = 1; /* Actually, du Lustie pos. */
	}
	break;

	case r__outsideducks:
		if (ped == 2) {
			/* Shut the door */
			show_one(3);
			first_show(2);
			then_show(1);
			then_show(4);
			start_to_close();
		}
		break;
	case r__ducks:
		dna.dogfoodpos = 1;
		break; /* Actually, Duck pos. */

	}

	seescroll = false; /* Now it can work again! */

}

void thinkabout(char z, boolean th) {     /* Hey!!! Get it and put it!!! */
	const integer x = 205;
	const integer y = 170;
	const integer picsize = 966;
	const bytefield thinkspace =
	{25, 170, 32, 200};
	untyped_file f;
	pointer p;
	byte fv;


	thinks = z;
	z -= 1;

	if (th) {
		/* Things */
		assign(f, "thinks.avd");
		wait();
		getmem(p, picsize);
		reset(f, 1);
		seek(f, ord(z)*picsize + 65);
		blockread(f, p, picsize);
		off();
		close(f);
	} else {
		/* People */
		assign(f, "folk.avd");
		wait();
		getmem(p, picsize);
		reset(f, 1);

		fv = ord(z) - 149;
		if (fv >= 25)  fv -= 8;
		if (fv == 20)  fv -= 1; /* Last time... */

		seek(f, fv * picsize + 65);
		blockread(f, p, picsize);
		off();
		close(f);
	}

	setactivepage(3);
	putimage(x, y, p, 0);
	setactivepage(1 - cp);

	for (fv = 0; fv <= 1; fv ++)
		getset[fv].remember(thinkspace);

	freemem(p, picsize);
	on();
	thinkthing = th;
}

void load_digits() {   /* Load the scoring digits & rwlites */
	const integer digitsize = 134;
	const integer rwlitesize = 126;
	untyped_file f;
	char fv;
	byte ff;

	assign(f, "digit.avd");
	reset(f, 1);
	for (fv = '0'; fv <= '9'; fv ++) {
		getmem(digit[fv], digitsize);
		blockread(f, digit[fv], digitsize);
	}
	for (ff = 0; ff <= 8; ff ++) {
		getmem(rwlite[ff], rwlitesize);
		blockread(f, rwlite[ff], rwlitesize);
	}
	close(f);
}

void toolbar() {
	untyped_file f;
	word s;
	byte fv;
	pointer p;

	assign(f, "useful.avd");
	reset(f, 1);
	s = filesize(f) - 40;
	getmem(p, s);
	seek(f, 40);
	blockread(f, p, s);
	close(f);
	/* off;*/

	setcolor(15); /* (And sent for chrysanthemums...) Yellow and white. */
	setfillstyle(1, 6);
	for (fv = 0; fv <= 1; fv ++) {
		setactivepage(fv);
		putimage(5, 169, p, 0);
		if (demo) {
			bar(264, 177, 307, 190);
			outtextxy(268, 188, "Demo!"); /* well... actually only white now. */
		}
	}

	/* on;*/
	freemem(p, s);
	oldrw = 177;
	showrw();
}

void showscore() {
	const bytefield scorespace = {33, 177, 39, 200};
	varying_string<3> q;
	byte fv;

	if (demo)  return;

	str(dna.score, q);
	while (q[0] < '\3')  q = string('0') + q;
	off();
	setactivepage(3);
	for (fv = 1; fv <= 3; fv ++)
		if (lastscore[fv] != q[fv])
			putimage(250 + fv * 15, 177, digit[q[fv]], 0);

	for (fv = 0; fv <= 1; fv ++)
		getset[fv].remember(scorespace);

	setactivepage(1 - cp);
	on();
	lastscore = q;
}

void points(byte num) {     /* Add on no. of points */
	byte q, fv;

	for (q = 1; q <= num; q ++) {
		dna.score += 1;
		if (soundfx)  for (fv = 1; fv <= 97; fv ++) sound(177 + dna.score * 3);
		nosound;
	}
	log_score(num, dna.score);
	showscore();
}

void topcheck() {
	{
		menuset &with = ddm_m;  /* Menuset */
		getmenu(mpx);
	} /* Do this one */
}

void mouseway() {
	byte col;

	off();
	col = getpixel(mx, my);
	on();
	{
		triptype &with = tr[1];
		{
			dnatype &with1 = dna;
			switch (col) {
			case green:        {
				dna.rw = up;
				rwsp(1, up);
				showrw();
			}
			break;
			case brown:        {
				dna.rw = down;
				rwsp(1, down);
				showrw();
			}
			break;
			case cyan:         {
				dna.rw = left;
				rwsp(1, left);
				showrw();
			}
			break;
			case lightmagenta: {
				dna.rw = right;
				rwsp(1, right);
				showrw();
			}
			break;
			case red:
			case white:
			case lightcyan:
			case yellow: {
				stopwalking();
				showrw();
			}
			break;
			}
		}
	}
}

void inkey() {
	char r;


	if (demo)  return; /* Demo handles this itself. */

	if (mousetext == "") {
		/* read keyboard */
		readkeye();
		if ((inchar == ' ') && ((shiftstate & 8) > 0)) {
			inchar = '\0';
			extd = '#'; /* alt-spacebar = alt-H */
		}
	} else {
		if (mousetext[1] == '`')  mousetext[1] = '\15'; /* Backquote = return in a macro */
		inchar = mousetext[1];
		mousetext = copy(mousetext, 2, 255);
	}
}

void posxy() {
	varying_string<3> xs, ys;

	setfillstyle(1, 0);
	setcolor(10);
	do {
		check();
		if (mpress == 1) {
			str(mx, xs);
			str(my, ys);
			off();
			bar(400, 160, 500, 168);
			outtextxy(400, 168, xs);
			outtextxy(440, 168, string(": ") + ys);
			on();
		}
	} while (!(my == 0));
	bar(400, 161, 640, 168);
}

void fxtoggle() {
	byte page_;
	const bytefield soundled =
	{52, 175, 55, 177};

	soundfx = ! soundfx;
	if (soundfx) {
		if (! fxhidden) {
			/* ...but *not* when the screen's dark. */
			sound(1770);
			delay(77);
			nosound;
		}
		setfillstyle(1, cyan);
	} else
		setfillstyle(1, black);
	setactivepage(3);
	bar(419, 175, 438, 177);
	setactivepage(1 - cp);
	for (page_ = 0; page_ <= 1; page_ ++) getset[page_].remember(soundled);
}

void objectlist() {
	char fv;

	dna.carrying = 0;
	if (thinkthing && ! dna.obj[thinks])
		thinkabout(money, a_thing); /* you always have money */
	for (fv = '\1'; fv <= numobjs; fv ++)
		if (dna.obj[fv]) {
			dna.carrying += 1;
			objlist[dna.carrying] = fv;
		}
}

void verte() {
	byte what;

	if (! dna.user_moves_avvy)  return;
	{
		triptype &with = tr[1];  /* that's the only one we're interested in here */


		if (mx < (cardinal)with.x)  what = 1;
		else if (mx > (unsigned char)(with.x + with.a.xl))  what = 2;
		else
			what = 0; /* On top */

		if (my < (cardinal)with.y)  what += 3;
		else if (my > (unsigned char)(with.y + with.a.yl))  what += 6;

		switch (what) {
		case 0:
			stopwalking();
			break; /* Clicked on Avvy- no movement */
		case 1:
			rwsp(1, left);
			break;
		case 2:
			rwsp(1, right);
			break;
		case 3:
			rwsp(1, up);
			break;
		case 4:
			rwsp(1, ul);
			break;
		case 5:
			rwsp(1, ur);
			break;
		case 6:
			rwsp(1, down);
			break;
		case 7:
			rwsp(1, dl);
			break;
		case 8:
			rwsp(1, dr);
			break;
		}    /* no other values are possible... */

		showrw();

	}
}

void checkclick() {
	bytefield b;

	check();
	ontoolbar = slow_computer && ((my >= 169) || (my <= 10));

	if (mrelease > 0)  after_the_scroll = false;
	switch (my) {
	case RANGE_11(0, 10):
		newpointer(1);
		break;                /* up arrow */
	case RANGE_11(159, 169):
		newpointer(8);
		break;                   /* I-beam */
	case 170 ... 200:
		newpointer(2);
		break; /* screwdriver */
	default: {
		if (! ddmnow) {     /* Dropdown can handle its own pointers. */
			if (((keystatus & 1) == 1) && (my >= 11) && (my <= 158)) {
				newpointer(7); /* Mark's crosshairs */
				verte();
				/* Normally, if you click on the picture, you're guiding Avvy around. */
			} else
				newpointer(4); /* fletch */
		}
	}
	}

	if (mpress > 0) {
		switch (mpy) {
		case RANGE_11(0, 10):
			if (dropsok)  topcheck();
			break;
		case 11 ... 158:
			if (! dropsok)
				mousetext = string('\15') + mousetext;
			break;                    /* But otherwise, it's
                                       equivalent to pressing Enter. */
		case RANGE_11(159, 169): { /* Click on command line */
			cursor_off();
			curpos = (mx - 16) / 8;
			if (curpos > length(current) + 1)  curpos = length(current) + 1;
			if (curpos < 1)  curpos = 1;
			cursor_on();
		}
		break;
		case 170 ... 200:
			switch (mpx) { /* bottom check */
			case 0 ... 207:
				mouseway();
				break;
			case 208 ... 260: { /* Examine the thing */
				do {
					check();
				} while (!(mrelease > 0));
				if (thinkthing) {
					thing = thinks;
					thing += 49;
					person = pardon;
				} else {
					person = thinks;
					thing = pardon;
				}
				callverb(vb_exam);
			}
			break;
			case 261 ... 319: {
				do {
					checkclick();
				} while (!(mrelease > 0));
				callverb(vb_score);
			}
			break;
			case 320 ... 357: {
				tr[1].xs = walk;
				newspeed();
			}
			break;
			case 358 ... 395: {
				tr[1].xs = run;
				newspeed();
			}
			break;
			case 396 ... 483:
				fxtoggle();
				break; /* "sound" */
				/*              484..534: begin { clock }
				                         off; if getpixel(mx,my)=14 then mousetext:='#'+mousetext; on;
				                        end;*/
			case 535 ... 640:
				mousetext = string('\15') + mousetext;
				break;
			}
			break;
		}
	}

	/* if mrelease>0 then
	 begin
	  if (cw<>177) and (mry>10) then
	   begin to_do:=(((mrx-20) div 100)*20)+(mry div 10); closewin; end;
	 end;*/
}

void mouse_init() {
	r.ax = 0;
	intr(0x33, r); /* Returns- no. keys in bx and whether present in ax. */
	wait();
}

void mousepage(word page_) {
	boolean onstate, wason;

	if (visible != m_virtual) {
		onstate = oncandopageswap;
		oncandopageswap = false;
		wason = visible == m_yes;
		if (wason)  off();
		{
			void &with = r;
			ax = 29;
			bx = page_;
		}
		intr(0x33, r);
		if (wason)  on();
		oncandopageswap = onstate;
	}
}

void errorled() {
	byte fv;

	state(0);
	for (fv = 0; fv <= 1; fv ++) {
		setactivepage(fv);
		off();
		setfillstyle(1, red);
		bar(419, 184, 438, 186);
		on();
	}
	for (fv = 177; fv >= 1; fv --) {
		sound(177 + (fv * 177177) / 999);
		delay(1);
		nosound;
	}
	for (fv = 0; fv <= 1; fv ++) {
		setactivepage(fv);
		off();
		setfillstyle(1, black);
		bar(419, 184, 438, 186);
		on();
	}
	state(defaultled);
	setactivepage(1 - cp);
}

shortint fades(shortint x) {
	byte r, g, b;

	shortint fades_result;
	r = x / 16;
	x = x % 16;
	g = x / 4;
	b = x % 4;
	if (r > 0)  r -= 1;
	if (g > 0)  g -= 1;
	if (b > 0)  b -= 1;
	fades_result = (16 * r + 4 * g + b);
	/* fades:=x-1;*/
	return fades_result;
}

void dusk();

static void fadeout(byte n) {
	byte fv;

	getpalette(fxpal[n]);
	for (fv = 1; fv <= fxpal[n].size - 1; fv ++)
		fxpal[n].colors[fv] = fades(fxpal[n].colors[fv]);
	setallpalette(fxpal[n]);
	/*delay(50);*/ slowdown();
}

void dusk() {
	byte fv;

	setbkcolor(0);
	if (fxhidden)  return;
	fxhidden = true;
	getpalette(fxpal[0]);
	for (fv = 1; fv <= 3; fv ++) fadeout(fv);
}

void dawn();

static void fadein(byte n) {
	setallpalette(fxpal[n]);
	/*delay(50);*/ slowdown();
}

void dawn() {
	byte fv;

	if ((holdthedawn) || (! fxhidden))  return;
	fxhidden = false;
	for (fv = 3; fv >= 0; fv --) fadein(fv);
	{
		dnatype &with = dna;
		if ((with.room == r__yours) && (with.avvy_in_bed) && (with.teetotal))  background(14);
	}
}

void showrw() {
	byte page_;

	{
		dnatype &with = dna;

		if (oldrw == with.rw)  return;
		oldrw = with.rw;
		off();
		for (page_ = 0; page_ <= 1; page_ ++) {
			setactivepage(page_);
			putimage(0, 161, rwlite[with.rw], 0);
		}
		on();
		setactivepage(1 - cp);
	}
}

void mblit(byte x1, byte y1, byte x2, byte y2, byte f, byte t) /* assembler; */
/* The Minstrel Blitter */
/* asm
{  ofsfr:=f*$4000+x1+y1*80;
   ofsto:=t*$4000+x1+y1*80;}

  mov bx,80; { We're multiplying by 80. }
  mov al,y1;
  mul bl;    { AX now contains y1*80. }
  xor cx,cx; { Zero CX. }
  mov cl,x1; { CX now equals x1 }
  add ax,cx; { AX now contains x1+y1*80. }
  mov si,ax;
  mov di,ax;

  mov ax,$4000;
  mov bl,f;
  mul bx; { Note that this is a *word*! }
  add si,ax;

  mov ax,$4000;
  mov bl,t;
  mul bx; { Note that this is a *word*! }
  add di,ax;

  push ds; { *** <<<< *** WE MUST PRESERVE THIS! }
  cld;  { Clear Direction flag - we're going forwards! }

  mov ax,$A000; { The screen memory. }
  mov ds,ax;
  mov es,ax; { The same. }

  { AH stores the number of bytes to copy. }
  { len:=(x2-x1)+1; }

  mov ah,x2;
  sub ah,x1;
  inc ah;

  { Firstly, let's decide how many times we're going round. }

  mov cl,y2; { How many numbers between y1 and y2? }
  sub cl,y1;
  inc cl; { Inclusive reckoning (for example, from 3 to 5 is 5-3+1=3 turns. }

  { We'll use SI and DI to be Ofsfr and Ofsto. }

  @Y_axis_loop:
   push cx;


   { OK... We've changed this loop from a for-next loop. "Bit" is
     represented by CX. }

{     port[$3c4]:=2; port[$3ce]:=4; }
   mov dx,$3c4;
   mov al,2;
   out dx,al;
   mov dx,$3ce;
   mov al,4;
   out dx,al;

   mov cx,4; { We have to copy planes 3, 2, 1 and Zero. We'll add 1 to the
    number, because at zero it stops. }

   mov bx,3; { This has a similar function to that of CX. }

   @start_of_loop:

    push cx;

{     port[$3C5]:=1 shl bit; }
    mov dx,$3C5;
    mov al,1;
    mov cl,bl; { BL = bit. }
    shl al,cl;
    out dx,al;
{     port[$3CF]:=bit; }
    mov dx,$3CF;
    mov al,bl; { BL = bit. }
    out dx,al;

{   move(mem[$A000:ofsfr],mem[$A000:ofsto],len); }

    xor ch,ch; { Clear CH. }
    mov cl,ah;

    repz movsb; { That's all we need to say! }

    mov cl,ah;
    sub si,cx; { This is MUCH, MUCH faster than pushing and popping them! }
    sub di,cx;

    pop cx; { Get the loop count back again. }
    dec bx; { One less... }
   loop @start_of_loop; { Until cx=0. }

   add si,80; { Do the next line... }
   add di,80;

   pop cx;
  loop @Y_axis_loop;

  pop ds; { Get it back again (or we'll be in trouble with TP!) }
*/
{
	;
}

void blitfix() {
	byte fv;

	fv = getpixel(0, 0); /* perform read & so cancel Xor effect! */
}

void clock();
const integer xm = 510;
const integer ym = 183;
static arccoordstype ah, am;
static word nh;

static void calchand(word ang, word length, arccoordstype &a, byte c) {
	if (ang > 900)  {
		a.xend = 177;
		return;
	}
	setcolor(c);
	arc(xm, ym, 449 - ang, 450 - ang, length);
	getarccoords(a);
}


static void hand(arccoordstype a, byte c) {
	if (a.xend == 177)  return;
	setcolor(c);
	line(xm, ym, a.xend, a.yend); /* "With a do-line???!", Liz said. */
}


static void chime() {
	word gd, gm, fv;

	if ((oh == 17717) || (! soundfx))  return; /* too high- must be first time around */
	fv = h % 12;
	if (fv == 0)  fv = 12;
	wait();
	for (gd = 1; gd <= fv; gd ++) {
		for (gm = 1; gm <= 3; gm ++) {
			sound((gd % 3) * 64 + 140 - gm * 30);
			delay(50 - gm * 12);
		}
		nosound;
		if (gd != fv)  delay(100);
	}
}



static void refresh_hands() {
	const bytefield clockspace = {61, 166, 66, 200};
	byte page_;

	for (page_ = 0; page_ <= 1; page_ ++)
		getset[page_].remember(clockspace);
}



static void plothands() {
	/*   off;*/
	setactivepage(3);
	calchand(onh, 14, ah, yellow);
	calchand(om * 6, 17, am, yellow);
	hand(ah, brown);
	hand(am, brown);
	calchand(nh, 14, ah, brown);
	calchand(m * 6, 17, am, brown);
	hand(ah, yellow);
	hand(am, yellow);
	setactivepage(1 - cp);

	refresh_hands();

	/*   on;*/
}

void clock() {
	/* ...Clock. */
	gettime(h, m, s, s1);
	nh = (h % 12) * 30 + m / 2;
	if (oh != h)  {
		plothands();
		chime();
	}
	if (om != m)  plothands();
	if ((h == 0) && (oh != 0) && (oh != 17717))
		display(string("Good morning!\r\rYes, it's just past midnight. Are you having") +
		        " an all-night Avvy session? Glad you like the game that much!");
	oh = h;
	onh = nh;
	om = m;
}

void flip_page() {
	if (! ddm_o.menunow) {
		cp = 1 - cp;
		setvisualpage(cp);
		setactivepage(1 - cp);
		/*mousepage(cp);*/
	}

}

void delavvy() {
	byte page_;

	off();
	{
		triptype &with = tr[1];
		for (page_ = 0; page_ <= 1; page_ ++)
			mblit(with.x / 8, with.y, (with.x + with.a.xl) / 8 + 1, with.y + with.a.yl, 3, page_);
	}
	blitfix();
	on();
}

void gameover() {
	byte fv;
	integer sx, sy;

	dna.user_moves_avvy = false;

	sx = tr[1].x;
	sy = tr[1].y;
	{
		triptype &with = tr[1];

		done();
		init(12, true);       /* 12 = Avalot falls */
		tr[1].step = 0;
		appear(sx, sy, 0);
	}
	set_up_timer(3, procavalot_falls, reason_falling_over);
	/* display(^m^m^m^m^m^m^i^i^i^i^i^i^s'Z'^v);*/
	alive = false;
}

/* OK. There are two kinds of redraw: Major and Minor. Minor is what happens
  when you load a game, etc. Major redraws EVERYTHING. */

void minor_redraw() {
	byte fv;

	dusk();
	enterroom(dna.room, 0); /* Ped unknown or non-existant. */

	for (fv = 0; fv <= 1; fv ++) {
		cp = 1 - cp;
		getback();
	}

	{
		dnatype &with = dna;

		lastscore = "TJA"; /* impossible digits */
		showscore();
	}

	dawn();
}

void major_redraw() {
	byte fv;

	dusk();
	setactivepage(0);
	cleardevice();

	toolbar();
	copy03();

	enterroom(dna.room, 0); /* 0 = ped unknown or non-existant. */
	for (fv = 0; fv <= 1; fv ++) {
		cp = 1 - cp;
		getback();
	}

	om = 177;
	clock();

	thinkabout(thinks, thinkthing);
	standard_bar();
	soundfx = ! soundfx;
	fxtoggle();
	for (fv = 0; fv <= 1; fv ++) {
		cp = 1 - cp;
		getback();
	}
	plottext();
	ledstatus = 177;
	state(2);

	{
		dnatype &with = dna;

		lastscore = "TJA"; /* impossible digits */
		showscore();
	}

	dawn();
}

word bearing(byte whichped)
/* Returns the bearing from ped Whichped to Avvy, in degrees. */
{
	const real rad2deg = 180 / pi;

	word bearing_result;
	{
		pedtype &with = peds[whichped];
		if (tr[1].x == with.x)
			bearing_result = 0; /* This would cause a division by zero if we let it through. */
		else
			/*
			 bearing:=trunc(((arctan((tr[1].y-y)/(tr[1].x-x)))*rad2deg)+90) mod 360*/
		{
			if (tr[1].x < with.x)
				bearing_result = trunc(atan((real)((tr[1].y - with.y)) / (tr[1].x - with.x)) * rad2deg) + 90;
			else
				bearing_result = trunc(atan((real)((tr[1].y - with.y)) / (tr[1].x - with.x)) * rad2deg) + 270;
		}
	}
	return bearing_result;
}

void flesh_colours()     /* assembler;
asm
  mov ax,$1012;
  mov bx,21;                 { 21 = light pink (why?) }
  mov cx,1;
  mov dx,seg    @flesh;
  mov es,dx;
  mov dx,offset @flesh;
  int $10;

  mov dx,seg    @darkflesh;
  mov es,dx;
  mov dx,offset @darkflesh;
  mov bx,5;                 { 5 = dark pink. }
  int $10;

  jmp @TheEnd;

 @flesh:
  db 56,35,35;

 @darkflesh:
  db 43,22,22;

 @TheEnd: */
{
	;
}

void sprite_run()
/* A sprite run is performed before displaying a scroll, if not all the
  sprites are still. It performs two fast cycles, only using a few of
  the links usually used, and without any extra animation. This should
  make the sprites the same on both pages. */
{
	byte fv;


	doing_sprite_run = true;

	for (fv = 0; fv <= 1; fv ++) {
		get_back_loretta();
		trippancy_link();

		flip_page();
	}

	doing_sprite_run = false;

}

void fix_flashers() {
	ledstatus = 177;
	oldrw = 177;
	state(2);
	showrw();
}

class unit_lucerna_initialize {
public:
	unit_lucerna_initialize();
};
static unit_lucerna_initialize lucerna_constructor;

unit_lucerna_initialize::unit_lucerna_initialize() {
	fxhidden = false;
	oh = 17717;
	om = 17717;
	if (atbios)  atkey = "f1";
	else atkey = "alt-";
}

} // End of namespace Avalanche.