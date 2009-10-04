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

#ifndef SCI_GUI_PICTURE_H
#define SCI_GUI_PICTURE_H

namespace Sci {

#define SCI_PAL_FORMAT_VARIABLE_FLAGS 0
#define SCI_PAL_FORMAT_CONSTANT_FLAGS 1

#define PIC_STYLE_MIRRORED 0x4000

class SciGUIpicture {
public:
	SciGUIpicture(EngineState *state, SciGUIgfx *gfx, SciGUIscreen *screen, GUIResourceId resourceId);
	~SciGUIpicture();

	GUIResourceId getResourceId();
	void draw(uint16 style, bool addToFlag, int16 EGApaletteNo);

private:
	void initData(GUIResourceId resourceId);
	void reset();
	void draw11();
	void decodeRLE(byte *rledata, byte *pixeldata, byte *outbuffer, int size);
	void drawCel(int16 x, int16 y, byte *pdata, int size);
	void drawCelAmiga(int16 x, int16 y, byte *pdata, int size);
	void drawVectorData(byte *data, int size);
	bool vectorIsNonOpcode(byte byte);
	void vectorGetAbsCoords(byte *data, int &curPos, int16 &x, int16 &y);
	void vectorGetRelCoords(byte *data, int &curPos, int16 oldx, int16 oldy, int16 &x, int16 &y);
	void vectorGetRelCoordsMed(byte *data, int &curPos, int16 oldx, int16 oldy, int16 &x, int16 &y);
	void vectorGetPatternTexture(byte *data, int &curPos, int16 pattern_Code, int16 &pattern_Texture);

	EngineState *_s;
	SciGUIgfx *_gfx;
	SciGUIscreen *_screen;

	int16 _resourceId;
	Resource *_resource;

	GUIPort *_curPort;
	uint16 _style;
	bool _addToFlag;
	int16 _EGApaletteNo;
	byte _priority;
};

} // End of namespace Sci

#endif
