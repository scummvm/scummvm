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

#ifndef SCI_GRAPHICS_PORTRAITS_H
#define SCI_GRAPHICS_PORTRAITS_H

namespace Sci {

struct PortraitBitmap {
	int16 width, height;
	int16 extraBytesPerLine;
	uint16 displaceX, displaceY;
	byte *rawBitmap;
};

class Portrait {
public:
	Portrait(ResourceManager *resMan, SciEvent *event, SciGui *gui, Screen *screen, SciPalette *palette, AudioPlayer *audio, Common::String resourceName);
	~Portrait();

	void setupAudio(uint16 resourceId, uint16 noun, uint16 verb, uint16 cond, uint16 seq);
	void doit(Common::Point position, uint16 resourceId, uint16 noun, uint16 verb, uint16 cond, uint16 seq);

private:
	void init(Common::String resourceName, SciPalette *palette);
	void drawBitmap(uint16 bitmapNr);
	void bitsShow();

	ResourceManager *_resMan;
	SciEvent *_event;
	SciGui *_gui;
	Screen *_screen;
	AudioPlayer *_audio;

	uint16 _height;
	uint16 _width;
	Palette _portraitPalette;

	uint16 _bitmapCount;
	PortraitBitmap *_bitmaps;

	byte *_fileData;

	Common::Point _position;
};

} // End of namespace Sci

#endif
