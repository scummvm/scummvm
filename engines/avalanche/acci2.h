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

/* ACCIDENCE II		The parser. */

#ifndef ACCI2_H
#define ACCI2_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Avalanche {
class AvalancheEngine;

class Acci {
public:
		/* verb codes */
	static const byte vb_exam = 1;
	static const byte vb_open = 2;
	static const byte vb_pause = 3;
	static const byte vb_get = 4;
	static const byte vb_drop = 5;
	static const byte vb_inv = 6;
	static const byte vb_talk = 7;
	static const byte vb_give = 8;
	static const byte vb_drink = 9;
	static const byte vb_load = 10;
	static const byte vb_save = 11;
	static const byte vb_pay = 12;
	static const byte vb_look = 13;
	static const byte vb_break = 14;
	static const byte vb_quit = 15;
	static const byte vb_sit = 16;
	static const byte vb_stand = 17;
	static const byte vb_go = 18;
	static const byte vb_info = 19;
	static const byte vb_undress = 20;
	static const byte vb_wear = 21;
	static const byte vb_play = 22;
	static const byte vb_ring = 23;
	static const byte vb_help = 24;
	static const byte vb_larrypass = 25;
	static const byte vb_phaon = 26;
	static const byte vb_boss = 27;
	static const byte vb_pee = 28;
	static const byte vb_cheat = 29;
	static const byte vb_magic = 30;
	static const byte vb_restart = 31;
	static const byte vb_eat = 32;
	static const byte vb_listen = 33;
	static const byte vb_buy = 34;
	static const byte vb_attack = 35;
	static const byte vb_password = 36;
	static const byte vb_dir = 37;
	static const byte vb_die = 38;
	static const byte vb_score = 39;
	static const byte vb_put = 40;
	static const byte vb_kiss = 41;
	static const byte vb_climb = 42;
	static const byte vb_jump = 43;
	static const byte vb_highscores = 44;
	static const byte vb_wake = 45;
	static const byte vb_hello = 46;
	static const byte vb_thanks = 47;

	static const byte vb_smartalec = 249;
	static const byte vb_expletive = 253;

	static const byte pardon = 254; /* =didn't understand / wasn't given. */

	static const int16 nowords = 277; /* how many words does the parser know? */
	static const byte nowt = 372;
	static const byte moved = 0; /* This word was moved. (Usually because it was the subject of
	  conversation.) */

	static const int16 first_password = 89; /* Words[first_password] should equal "TIROS". */



	struct vocab {
		byte n;
		Common::String w;
	};

	static const vocab words[nowords];

	static const char what[];



	struct ranktype {
		uint16 score;
		Common::String title;
	};

	static const ranktype ranks[9];



	Common::String thats;
	Common::String unknown;
	Common::String realwords[11];
	char verb, person, thing, thing2;
	bool polite;




	Acci(AvalancheEngine *vm);

	void init();

	void clearwords();
	void parse();
	void lookaround();
	void opendoor();
	void do_that();
	void verbopt(char n, Common::String &answer, char &anskey);
	void have_a_drink();

private:
	AvalancheEngine *_vm;

	byte fv;

	byte wordnum(Common::String x);

	void replace(Common::String old1, Common::String new1);

	Common::String rank();

	Common::String totaltime();

	void number(Common::String &codes);
	void cheatparse(Common::String codes);

	void punctustrip(Common::String &x);

	void displaywhat(char ch, bool animate, bool &ambigous);
	bool do_pronouns();

	void lowercase();
	void propernouns();
	void sayit();
	void store_interrogation(byte interrogation);

	void clearuint16s();

	void examobj();

	bool personshere();

	void exampers();

	bool holding();

	void special(bool before);
	void examine();

	void inv();

	void swallow();

	void others();

	void silly();
	void putproc();

	void not_in_order();
	void go_to_cauldron();
	bool give2spludwick();

	void cardiff_climbing();

	void already();
	void stand_up();

	void getproc(char thing);

	void give_geida_the_lute();

	void play_harp();

	void winsequence();

	void person_speaks();

	void heythanks();

	int16 pos(const Common::String &crit, const Common::String &src); // Returns the index of the first appearance of crit in src.

};

} // End of namespace Avalanche.

#endif // ACCI2_H
