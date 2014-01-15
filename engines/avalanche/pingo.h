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

/* PINGO	Full-screen sub-parts of the game. */

#ifndef AVALANCHE_PINGO_H
#define AVALANCHE_PINGO_H

#include "common/str.h"

namespace Avalanche {
class AvalancheEngine;

class Pingo {
public:
	Pingo(AvalancheEngine *vm);

	void bossKey();
	void copy02();
	void copy03();
	void copyPage(byte frp, byte top);
	void winningPic();

private:
	AvalancheEngine *_vm;

	void dPlot(int16 x, int16 y, Common::String z);
};

} // End of namespace Avalanche.

#endif // AVALANCHE_PINGO_H
