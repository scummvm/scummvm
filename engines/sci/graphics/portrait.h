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

#ifndef SCI_GRAPHICS_PORTRAIT_H
#define SCI_GRAPHICS_PORTRAIT_H

#include "sci/util.h"

namespace Sci {

struct PortraitBitmap {
	int16 width, height;
	int16 extraBytesPerLine;
	uint16 displaceX, displaceY;
	SciSpan<const byte> rawBitmap;
};

/**
 * This class is used to handle all the hi-res portraits used in the Windows
 * release of KQ6. These are all in external data files, and handled separately
 * from the rest of the engine (originally, inside rave.dll)
 */
class Portrait {
public:
	Portrait(ResourceManager *resMan, EventManager *event, GfxScreen *screen, GfxPalette *palette, AudioPlayer *audio, const Common::String &resourceName);

	void setupAudio(uint16 resourceId, uint16 noun, uint16 verb, uint16 cond, uint16 seq);
	void doit(Common::Point position, uint16 resourceId, uint16 noun, uint16 verb, uint16 cond, uint16 seq);

	Common::String getResourceName() { return _resourceName; }

private:
	void init();
	void drawBitmap(uint16 bitmapNr, bool show);

	int16 raveGetTicks(Resource *resource, uint *offset);
	uint16 raveGetID(Resource *resource, uint *offset);
	SciSpan<const byte> raveGetLipSyncData(const uint16 raveID);

	ResourceManager *_resMan;
	EventManager *_event;
	GfxPalette *_palette;
	GfxScreen *_screen;
	AudioPlayer *_audio;

	uint16 _height;
	uint16 _width;
	Palette _portraitPalette;

	Common::Array<PortraitBitmap> _bitmaps;

	Common::String _resourceName;

	Common::SpanOwner<SciSpan<const byte> > _fileData;

	uint32 _lipSyncIDCount;
	SciSpan<const byte> _lipSyncIDTable;

	SciSpan<const byte> _lipSyncData;
	Common::Array<uint16> _lipSyncDataOffsetTable;

	Common::Point _position;
};

} // End of namespace Sci

#endif // SCI_GRAPHICS_PORTRAIT_H
