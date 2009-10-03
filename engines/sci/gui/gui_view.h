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

namespace Sci {

struct sciViewCellInfo {
	int16 width, height;
	char displaceX;
	byte displaceY;
	byte clearKey;
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
	SciGUIview(OSystem *system, EngineState *state, SciGUIgfx *gfx, SciGUIscreen *screen, sciResourceId resourceId);
	~SciGUIview();

	// TODO: Remove gfx reference after putting palette things into SciGUIscreen

	sciResourceId getResourceId();
	int16 getWidth(uint16 loopNo, uint16 cellNo);
	int16 getHeight(uint16 loopNo, uint16 cellNo);
	sciViewCellInfo *getCellInfo(uint16 loop, uint16 cel);
	sciViewLoopInfo *getLoopInfo(uint16 loop);
	byte *getBitmap(uint16 loopNo, uint16 cellNo);
	void draw(Common::Rect rect, Common::Rect clipRect, uint16 loopNo, uint16 cellNo, byte priority, uint16 paletteNo);

private:
	void initData(sciResourceId resourceId);
	void unpackView(uint16 loopNo, uint16 cellNo, byte *outPtr, uint16 pixelCount);

	OSystem *_system;
	EngineState *_s;
	SciGUIgfx *_gfx;
	SciGUIscreen *_screen;

	sciResourceId _resourceId;
	byte *_resourceData;

	uint16 _loopCount;
	sciViewLoopInfo *_loop;
	bool _embeddedPal;
	sciPalette _palette;
};

} // end of namespace Sci
