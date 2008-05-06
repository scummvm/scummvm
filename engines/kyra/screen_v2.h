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
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_SCREEN_V2_H
#define KYRA_SCREEN_V2_H

#include "kyra/screen.h"
#include "kyra/kyra_v2.h"

namespace Kyra {

class Screen_v2 : public Screen {
public:
	Screen_v2(KyraEngine *vm, OSystem *system) : Screen(vm, system) {}

	// screen page handling
	void copyWsaRect(int x, int y, int w, int h, int dimState, int plotFunc, const uint8 *src,
					int unk1, const uint8 *unkPtr1, const uint8 *unkPtr2);

	// palette handling
	uint8 *generateOverlay(const uint8 *palette, uint8 *buffer, int color, uint16 factor);
	void applyOverlay(int x, int y, int w, int h, int pageNum, const uint8 *overlay);
	int findLeastDifferentColor(const uint8 *paletteEntry, const uint8 *palette, uint16 numColors);

	// shape handling
	uint8 *getPtrToShape(uint8 *shpFile, int shape);
	const uint8 *getPtrToShape(const uint8 *shpFile, int shape);

	int getShapeScaledWidth(const uint8 *shpFile, int scale);
	int getShapeScaledHeight(const uint8 *shpFile, int scale);

	uint16 getShapeSize(const uint8 *shp);

	uint8 *makeShapeCopy(const uint8 *src, int index);

	// rect handling
	int getRectSize(int w, int h);

	// text display
	void setTextColorMap(const uint8 *cmap);

	// layer handling
	virtual int getLayer(int x, int y);
protected:
};

} // end of namespace Kyra

#endif 

