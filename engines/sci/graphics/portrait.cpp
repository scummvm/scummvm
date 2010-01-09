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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/portrait.h"
#include "sci/sound/audio.h"

namespace Sci {

Portrait::Portrait(ResourceManager *resMan, Screen *screen, SciPalette *palette, AudioPlayer *audio, Common::String resourceName)
	: _resMan(resMan), _screen(screen), _palette(palette), _audio(audio), _resourceName(resourceName) {
	init();
}

Portrait::~Portrait() {
}

void Portrait::init() {
	// .BIN files are loaded from actors directory and from .\ directory
	// header:
	// 3 bytes "WIN"
	// 2 bytes main width (should be the same as first bitmap header width)
	// 2 bytes main height (should be the same as first bitmap header height)
	// 2 bytes animation count
	// 2 bytes unknown
	// 2 bytes unknown
	// 4 bytes paletteSize (base 1)
	//  -> 17 bytes
	// paletteSize bytes paletteData
	// 14 bytes bitmap header
	//  -> 4 bytes unknown
	//  -> 2 bytes height
	//  -> 2 bytes width
	//  -> 6 bytes unknown
	// height * width bitmap data
	// another animation count times bitmap header and data
	Common::SeekableReadStream *file = 0;
	_fileName = "actors/" + _resourceName + ".bin";
	file = SearchMan.createReadStreamForMember(_fileName);
	if (!file) {
		_fileName = _resourceName + ".bin";
		file = SearchMan.createReadStreamForMember(_fileName);
		if (!file)
			error("portrait %s.bin not found", _resourceName.c_str());
	}
	_fileSize = file->size();
	_fileData = new byte[_fileSize];
	file->read(_fileData, _fileSize);
	delete file;

	if (strncmp((char *)_fileData, "WIN", 3)) {
		error("portrait %s doesn't have valid header", _resourceName.c_str());
	}
	_width = READ_LE_UINT16(_fileData + 3);
	_height = READ_LE_UINT16(_fileData + 5);
	_animationCount = READ_LE_UINT16(_fileData + 7);

	_portraitPaletteSize = READ_LE_UINT16(_fileData + 13);
	byte *data = _fileData + 17;
	// Read palette
	memset(&_portraitPalette, 0, sizeof(Palette));
	uint16 palSize = 0, palNr = 0;
	while (palSize < _portraitPaletteSize) {
		_portraitPalette.colors[palNr].b = *data++;
		_portraitPalette.colors[palNr].g = *data++;
		_portraitPalette.colors[palNr].r = *data++;
		_portraitPalette.colors[palNr].used = 1;
		_portraitPalette.intensity[palNr] = 100;
		palNr++; palSize += 3;
	}

	// Read main bitmap
	assert(READ_LE_UINT16(data + 4) == _height);
	assert(READ_LE_UINT16(data + 6) == _width);
	data += 14; // Skip over bitmap header
	_mainBitmapData = data;
	data += _height * _width;
	
	// TODO: Read animation bitmaps
}

void Portrait::doit(Common::Point position, uint16 resourceId, uint16 noun, uint16 verb, uint16 cond, uint16 seq) {
	_position = position;

	// Now init audio and sync resource
	uint32 audioNumber = ((noun & 0xff) << 24) | ((verb & 0xff) << 16) | ((cond & 0xff) << 8) | (seq & 0xff);
	ResourceId syncResourceId = ResourceId(kResourceTypeSync36, resourceId, noun, verb, cond, seq);
	Resource *syncResource = _resMan->findResource(syncResourceId, true);
	uint syncOffset = 0;

	// Draw base bitmap
	drawMainBitmap();

	// Start playing audio...
	_audio->stopAudio();
	_audio->startAudio(resourceId, audioNumber);

	// Do animation depending on sync resource till audio is done playing
	int16 syncCue;
	int timerPosition, curPosition;

	timerPosition = 0;
	while (syncOffset < syncResource->size - 2) {
		timerPosition += (int16)READ_LE_UINT16(syncResource->data + syncOffset);
		syncOffset += 2;
		if (syncOffset < syncResource->size - 2) {
			syncCue = (int16)READ_LE_UINT16(syncResource->data + syncOffset);
			syncOffset += 2;
		} else {
			syncCue = -1;
		}
		// Wait till syncTime passed, then show specific animation bitmap
		do {
			g_system->delayMillis(10);
			curPosition = _audio->getAudioPosition();
		} while ((curPosition != -1) && (curPosition < timerPosition));

		if (syncCue >= 0) {
			// Display animation bitmap
			drawBitmap(syncCue);
			warning("display animation %d", syncCue);
		}
	}
}

void Portrait::drawMainBitmap() {
	byte *data = _mainBitmapData;
	_palette->set(&_portraitPalette, 1);
	for (int y = 0; y < _height; y++) {
		for (int x = 0; x < _width; x++) {
			_screen->putPixelOnDisplay(_position.x + x, _position.y + y, _portraitPalette.mapping[*data++]);
		}
	}

	Common::Rect mainBitmap = Common::Rect(_width, _height);
	mainBitmap.moveTo(_position.x, _position.y);
	_screen->copyDisplayRectToScreen(mainBitmap);
	g_system->updateScreen();
}

void Portrait::drawBitmap(int16 bitmapNr) {
	// 0 seems to be main bitmap
}

} // End of namespace Sci
