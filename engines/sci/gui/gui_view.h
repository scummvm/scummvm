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

#ifndef SCI_GUI_VIEW_H
#define SCI_GUI_VIEW_H

namespace Sci {

struct sciViewCellInfo {
	int16 width, height;
	char displaceX;
	byte displaceY;
	byte clearKey;
	uint16 offsetEGA;
	uint16 offsetRLE;
	uint16 offsetLiteral;
	byte *rawBitmap;
};

struct sciViewLoopInfo {
	bool mirrorFlag;
	uint16 cellCount;
	sciViewCellInfo *cell;
};

class SciGUIview {
public:
	SciGUIview(OSystem *system, EngineState *state, SciGUIgfx *gfx, SciGUIscreen *screen, GUIResourceId resourceId);
	~SciGUIview();

	// TODO: Remove gfx reference after putting palette things into SciGUIscreen

	GUIResourceId getResourceId();
	int16 getWidth(GUIViewLoopNo loopNo, GUIViewCellNo cellNo);
	int16 getHeight(GUIViewLoopNo loopNo, GUIViewCellNo cellNo);
	sciViewCellInfo *getCellInfo(GUIViewLoopNo loopNo, GUIViewCellNo cellNo);
	sciViewLoopInfo *getLoopInfo(GUIViewLoopNo loopNo);
	void getCellRect(GUIViewLoopNo loopNo, GUIViewCellNo cellNo, int16 x, int16 y, int16 z, Common::Rect *outRect);
	byte *getBitmap(GUIViewLoopNo loopNo, GUIViewCellNo cellNo);
	void draw(Common::Rect rect, Common::Rect clipRect, GUIViewLoopNo loopNo, GUIViewCellNo cellNo, byte priority, uint16 paletteNo);

private:
	void initData(GUIResourceId resourceId);
	void unpackCel(GUIViewLoopNo loopNo, GUIViewCellNo cellNo, byte *outPtr, uint16 pixelCount);

	OSystem *_system;
	EngineState *_s;
	SciGUIgfx *_gfx;
	SciGUIscreen *_screen;

	GUIResourceId _resourceId;
	byte *_resourceData;

	uint16 _loopCount;
	sciViewLoopInfo *_loop;
	bool _embeddedPal;
	GUIPalette _palette;
};

} // End of namespace Sci

#endif
