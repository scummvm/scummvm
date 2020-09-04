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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCI_GRAPHICS_PICTURE_H
#define SCI_GRAPHICS_PICTURE_H

#include "sci/util.h"

namespace Sci {

#define SCI_PATTERN_CODE_RECTANGLE 0x10
#define SCI_PATTERN_CODE_USE_TEXTURE 0x20
#define SCI_PATTERN_CODE_PENSIZE 0x07

enum {
	SCI_PICTURE_TYPE_REGULAR		= 0,
	SCI_PICTURE_TYPE_SCI11		= 1,
	SCI_PICTURE_TYPE_SCI32		= 2
};

class GfxPorts;
class GfxScreen;
class GfxPalette;
class GfxCoordAdjuster16;
class ResourceManager;
class Resource;

/**
 * Picture class, handles loading and displaying of picture resources
 *  every picture resource has its own instance of this class
 */
class GfxPicture {
public:
	GfxPicture(ResourceManager *resMan, GfxCoordAdjuster16 *coordAdjuster, GfxPorts *ports, GfxScreen *screen, GfxPalette *palette, GuiResourceId resourceId, bool EGAdrawingVisualize = false);
	~GfxPicture();

	GuiResourceId getResourceId();
	void draw(bool mirroredFlag, bool addToFlag, int16 EGApaletteNo);

private:
	void initData(GuiResourceId resourceId);
	void reset();
	void drawSci11Vga();
	void drawCelData(const SciSpan<const byte> &inbuffer, int headerPos, int rlePos, int literalPos, int16 drawX, int16 drawY, int16 pictureX, int16 pictureY, bool isEGA);
	void drawVectorData(const SciSpan<const byte> &data);
	bool vectorIsNonOpcode(byte pixel);
	void vectorGetAbsCoords(const SciSpan<const byte> &data, uint &curPos, int16 &x, int16 &y);
	void vectorGetAbsCoordsNoMirror(const SciSpan<const byte> &data, uint &curPos, int16 &x, int16 &y);
	void vectorGetRelCoords(const SciSpan<const byte> &data, uint &curPos, int16 &x, int16 &y);
	void vectorGetRelCoordsMed(const SciSpan<const byte> &data, uint &curPos, int16 &x, int16 &y);
	void vectorGetPatternTexture(const SciSpan<const byte> &data, uint &curPos, int16 pattern_Code, int16 &pattern_Texture);
	void vectorFloodFill(int16 x, int16 y, byte color, byte prio, byte control);
	void vectorPattern(int16 x, int16 y, byte pic_color, byte pic_priority, byte pic_control, byte code, byte texture);
	void vectorPatternBox(Common::Rect box, byte color, byte prio, byte control);
	void vectorPatternTexturedBox(Common::Rect box, byte color, byte prio, byte control, byte texture);
	void vectorPatternCircle(Common::Rect box, byte size, byte color, byte prio, byte control);
	void vectorPatternTexturedCircle(Common::Rect box, byte size, byte color, byte prio, byte control, byte texture);

	ResourceManager *_resMan;
	GfxCoordAdjuster16 *_coordAdjuster;
	GfxPorts *_ports;
	GfxScreen *_screen;
	GfxPalette *_palette;

	int16 _resourceId;
	Resource *_resource;
	int _resourceType;

	bool _mirroredFlag;
	bool _addToFlag;
	int16 _EGApaletteNo;
	byte _priority;

	// If true, we will show the whole EGA drawing process...
	bool _EGAdrawingVisualize;
};

} // End of namespace Sci

#endif
