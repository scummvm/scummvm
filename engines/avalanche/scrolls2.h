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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#ifndef SCROLLS2_H
#define SCROLLS2_H

#include "common/system.h"

namespace Avalanche {
class AvalancheEngine;

class Scrolls;

typedef void (Scrolls::*func2)();

class Scrolls {
public:
	bool aboutscroll; /* Is this the about box? */



	Scrolls();

	void setParent(AvalancheEngine *vm);

	void state(byte x);      /* Sets "Ready" light to whatever */

	void drawscroll(func2 gotoit);      /* This is one of the oldest funcs in the game. */

	void bubble(func2 gotoit);

	void resetscroll();

	void calldrivers();

	void display(Common::String z);

	bool ask(Common::String question);

	void natural();

	Common::String lsd();

	void okay();    /* Says "Okay!" */

	void musical_scroll();

private:
	AvalancheEngine *_vm;

	static const int16 roman;
	static const int16 italic;

	static const int16 halficonwidth; /* Half the width of an icon. */

	int16 dix, diy;
	raw ch[2];
	byte cfont; /* Current font */

	int16 dodgex, dodgey;
	byte param; /* For using arguments code */

	byte use_icon;

	void easteregg();

	void say(int16 x, int16 y, Common::String z);

	void normscroll();

	void dialogue();

	void store_(byte what, tunetype &played);

	bool they_match(tunetype &played);

	void music_scroll();

	void resetscrolldriver();

	void dingdongbell();

	void dodgem();

	void undodgem();

	void geticon(int16 x, int16 y, byte which);

	void block_drop(Common::String fn, int16 xl, int16 yl, int16 y);

	void strip(Common::String &q);

	void solidify(byte n);

	void loadfont();
};

} // End of namespace Avalanche

#endif // SCROLLS2_H
