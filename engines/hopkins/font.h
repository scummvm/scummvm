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

#ifndef HOPKINS_FONT_H
#define HOPKINS_FONT_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

struct TxtItem {
	int field0;
	int field2;
	int field4;
	int field8;
	int fieldA;
	int fieldC;
	int fieldE;
	int field10;
	int field3FC;
	int field3FE;
	int field40A;
};

class FontManager {
private:
	HopkinsEngine *_vm;
public:
	TxtItem Txt[12];
public:
	void setParent(HopkinsEngine *vm);

	void TEXTE_OFF(int idx);
};

} // End of namespace Hopkins

#endif /* HOPKINS_FONT_H */
