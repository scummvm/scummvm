/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */
#ifndef GOB_PALANIM_H
#define GOB_PALANIM_H

namespace Gob {

class PalAnim {
public:
	int16 fadeValue;

	char fadeColor(char from, char to);
	char fadeStep(int16 oper);	// oper == 0 - fade all colors, 1, 2, 3 - red,green, blue
	void fade(Video::PalDesc * palDesc, int16 fade, int16 all);

	PalAnim(GobEngine *vm);

protected:
	byte toFadeRed[256];
	byte toFadeGreen[256];
	byte toFadeBlue[256];
	GobEngine *_vm;
};

}				// End of namespace Gob

#endif	/* __PALANIM_H */
