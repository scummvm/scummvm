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
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#ifndef MORTEVIELLE_OUTTEXT_H
#define MORTEVIELLE_OUTTEXT_H

#include "common/str.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

const int kAdrAni = 0x7314;

class TextHandler {
private:
	int  nextWord(int p, const char *ch, int &tab);
public:
	void displayStr(Common::String inputStr, int x, int y, int dx, int dy, int typ);
	void loadDesFile(Common::String filename, int32 passe, int long_);
	void loadAniFile(Common::String filename, int32 skipSize, int length);
	void taffich();
};

} // End of namespace Mortevielle
#endif
