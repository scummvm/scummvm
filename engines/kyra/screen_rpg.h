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

#ifndef KYRA_SCREEN_RPG_H
#define KYRA_SCREEN_RPG_H

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/screen.h"
#include "common/system.h"

namespace Kyra {

class Screen;
class KyraEngine_v1;

class Screen_Rpg : virtual public Screen {
public:
	Screen_Rpg(KyraEngine_v1 *vm, OSystem *system, const ScreenDim *dimTable, int dimTableSize);
	virtual ~Screen_Rpg();

	virtual void setScreenDim(int dim);
	virtual const ScreenDim *getScreenDim(int dim);
	virtual int screenDimTableCount() const = 0;

	void modifyScreenDim(int dim, int x, int y, int w, int h);
	int curDimIndex() const { return _curDimIndex; }

	void crossFadeRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage);

private:
	int _curDimIndex;
	int _screenDimTableCount;
	ScreenDim **_customDimTable;
	const ScreenDim *_screenDimTable;
};

}	// End of namespace Kyra

#endif // ENABLE_EOB || ENABLE_LOL

#endif
