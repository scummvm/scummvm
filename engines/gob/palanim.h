/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef GOB_PALANIM_H
#define GOB_PALANIM_H

#include "gob/video.h"

namespace Gob {

class PalAnim {
public:
	bool fadeStep(int16 oper); // 0: all colors, 1: red, 2: green, 3: blue
	void fade(Video::PalDesc * palDesc, int16 fade, int16 all);

	PalAnim(GobEngine *vm);

protected:
	int16 _fadeValue;

	byte _toFadeRed[256];
	byte _toFadeGreen[256];
	byte _toFadeBlue[256];

	char *_palArray[3];
	byte *_fadeArray[3];

	GobEngine *_vm;

	bool fadeStepColor(int color);
	char fadeColor(int16 from, int16 to);
};

} // End of namespace Gob

#endif // GOB_PALANIM_H
