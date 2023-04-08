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
*/

#ifndef WATCHMAKER_2D_STUFF_H
#define WATCHMAKER_2D_STUFF_H

#include "watchmaker/sysdef.h"
#include "watchmaker/t3d.h"
#include "watchmaker/types.h"
#include "watchmaker/struct.h"

namespace Watchmaker {

class Renderer;

class TwoDeeStuff {
	SDDBitmap DDBitmapsList[MAX_DD_BITMAPS];
	SDDText DDTextsList[MAX_DD_TEXTS];
	SDDText RendText[MAX_REND_TEXTS];

	Renderer *_renderer;
public:
	TwoDeeStuff(Renderer *renderer) : _renderer(renderer) {}
	void writeBitmapListTo(SDDBitmap* target);
	void garbageCollectPreRenderedText();

	void clearBitmapList();
	void clearTextList();

	int32 findFreeBitmap();

	void displayDDBitmap(int32 tnum, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy);
	void displayDDBitmap_NoFit(int32 tnum, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy);
	void displayDDText(char *text, FontKind font, FontColor color, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy);
private:
	int32 rendDDText(char *text, FontKind font, FontColor color);
};

} // End of namespace Watchmaker

#endif // SCUMMVM_2D_STUFF_H
