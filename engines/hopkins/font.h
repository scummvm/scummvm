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
	bool textOn;
	int field2;
	Common::String filename;
	int16 xp;
	int16 yp;
	int messageId;
	int fieldE;
	int field10;
	int lineCount;
	Common::String lines[10];
	int field3FC;
	int field3FE;
	byte *textBlock;
	int16 width;
	int16 height;
	bool textLoaded;
	int colour;
};

struct ListeTxtItem {
	bool enabled;
	int16 xp;
	int16 yp;
	int16 width;
	int16 height;
};


class FontManager {
private:
	HopkinsEngine *_vm;
public:
	TxtItem Txt[12];
	ListeTxtItem ListeTxt[12];
	int TRIER_TEXT[21];
	Common::String oldname;
	Common::String nom_index;
	int Index[4048];
	byte *texte_tmp;
public:
	void setParent(HopkinsEngine *vm);
	void clearAll();

	void TEXTE_ON(int idx);
	void TEXTE_OFF(int idx);
	void COUL_TXT(int idx, byte colByte);
	void OPTI_COUL_TXT(int idx1, int idx2, int idx3, int idx4);
	void DOS_TEXT(int idx, int messageId, const Common::String &filename, int xp, int yp, int a6, int a7, int a8, int a9, int a10);
	void BOITE(int idx, int messageId, const Common::String &filename, int xp, int yp);
	void TEXT_NOW1(int xp, int yp, const Common::String &message, int colour);
};

} // End of namespace Hopkins

#endif /* HOPKINS_FONT_H */
