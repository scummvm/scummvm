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

#include "mohawk/bitmap.h"
#include "mohawk/cursors.h"
#include "mohawk/resource.h"
#include "mohawk/graphics.h"
#include "mohawk/myst.h"
#include "mohawk/riven_cursors.h"

#include "common/macresman.h"
#include "common/ne_exe.h"
#include "common/system.h"
#include "graphics/cursorman.h"

namespace Mohawk {

static const byte s_bwPalette[] = {
	0x00, 0x00, 0x00,	// Black
	0xFF, 0xFF, 0xFF	// White
};

void CursorManager::showCursor() {
	CursorMan.showMouse(true);
}

void CursorManager::hideCursor() {
	CursorMan.showMouse(false);
}

void CursorManager::setDefaultCursor() {
	static const byte defaultCursor[] = {
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0,
		1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0,
		1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0,
		1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0,
		1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0,
		1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0,
		1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
		1, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
		1, 2, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0,
		1, 2, 2, 1, 1, 2, 2, 1, 0, 0, 0, 0,
		1, 2, 1, 0, 1, 1, 2, 2, 1, 0, 0, 0,
		1, 1, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0
	};

	CursorMan.replaceCursor(defaultCursor, 12, 20, 0, 0, 0);
	CursorMan.replaceCursorPalette(s_bwPalette, 1, 2);
}

void CursorManager::setCursor(uint16 id) {
	// For the base class, just use the default cursor always
	setDefaultCursor();
}

void CursorManager::decodeMacXorCursor(Common::SeekableReadStream *stream, byte *cursor) {
	assert(stream);
	assert(cursor);

	// Get black and white data
	for (int i = 0; i < 32; i++) {
		byte imageByte = stream->readByte();
		for (int b = 0; b < 8; b++)
			cursor[i * 8 + b] = (imageByte & (0x80 >> b)) ? 1 : 2;
	}

	// Apply mask data
	for (int i = 0; i < 32; i++) {
		byte imageByte = stream->readByte();
		for (int b = 0; b < 8; b++)
			if ((imageByte & (0x80 >> b)) == 0)
				cursor[i * 8 + b] = 0;
	}
}

void CursorManager::setStandardCursor(Common::SeekableReadStream *stream) {
	// The Broderbund devs decided to rip off the Mac format, it seems.
	// However, they reversed the x/y hotspot. That makes it totally different!!!!
	assert(stream);

	byte cursorBitmap[16 * 16];
	decodeMacXorCursor(stream, cursorBitmap);
	uint16 hotspotY = stream->readUint16BE();
	uint16 hotspotX = stream->readUint16BE();

	CursorMan.replaceCursor(cursorBitmap, 16, 16, hotspotX, hotspotY, 0);
	CursorMan.replaceCursorPalette(s_bwPalette, 1, 2);

	delete stream;
}

void DefaultCursorManager::setCursor(uint16 id) {
	setStandardCursor(_vm->getResource(_tag, id));
}

MystCursorManager::MystCursorManager(MohawkEngine_Myst *vm) : _vm(vm) {
	_bmpDecoder = new MystBitmap();
}

MystCursorManager::~MystCursorManager() {
	delete _bmpDecoder;
}

void MystCursorManager::showCursor() {
	CursorMan.showMouse(true);
	_vm->_needsUpdate = true;
}

void MystCursorManager::hideCursor() {
	CursorMan.showMouse(false);
	_vm->_needsUpdate = true;
}

void MystCursorManager::setCursor(uint16 id) {
	// Both Myst and Myst ME use the "MystBitmap" format for cursor images.
	MohawkSurface *mhkSurface = _bmpDecoder->decodeImage(_vm->getResource(ID_WDIB, id));
	Graphics::Surface *surface = mhkSurface->getSurface();
	Common::SeekableReadStream *clrcStream = _vm->getResource(ID_CLRC, id);
	uint16 hotspotX = clrcStream->readUint16LE();
	uint16 hotspotY = clrcStream->readUint16LE();
	delete clrcStream;

	// Myst ME stores some cursors as 24bpp images instead of 8bpp
	if (surface->bytesPerPixel == 1) {
		CursorMan.replaceCursor((byte *)surface->pixels, surface->w, surface->h, hotspotX, hotspotY, 0);

		const byte *srcPal = mhkSurface->getPalette();
		byte pal[3*256];
		for (uint i = 0; i < 256; ++i) {
			pal[i * 3 + 0] = srcPal[i * 4 + 0];
			pal[i * 3 + 1] = srcPal[i * 4 + 1];
			pal[i * 3 + 2] = srcPal[i * 4 + 2];
		}

		CursorMan.replaceCursorPalette(pal, 0, 256);
	} else {
		Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
		CursorMan.replaceCursor((byte *)surface->pixels, surface->w, surface->h, hotspotX, hotspotY, pixelFormat.RGBToColor(255, 255, 255), 1, &pixelFormat);
	}

	_vm->_needsUpdate = true;
	delete mhkSurface;
}

void MystCursorManager::setDefaultCursor() {
	setCursor(kDefaultMystCursor);
}

void RivenCursorManager::setCursor(uint16 id) {
	// All of Riven's cursors are hardcoded. See riven_cursors.h for these definitions.

	switch (id) {
	case 1002:
		// Zip Mode
		CursorMan.replaceCursor(s_zipModeCursor, 16, 16, 8, 8, 0);
		CursorMan.replaceCursorPalette(s_zipModeCursorPalette, 1, ARRAYSIZE(s_zipModeCursorPalette) / 3);
		break;
	case 2003:
		// Hand Over Object
		CursorMan.replaceCursor(s_objectHandCursor, 16, 16, 8, 8, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 3);
		break;
	case 2004:
		// Grabbing/Using Object
		CursorMan.replaceCursor(s_grabbingHandCursor, 13, 13, 6, 6, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 3);
		break;
	case 3000:
		// Standard Hand
		CursorMan.replaceCursor(s_standardHandCursor, 15, 16, 6, 0, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 3);
		break;
	case 3001:
		// Pointing Left
		CursorMan.replaceCursor(s_pointingLeftCursor, 15, 13, 0, 3, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 3);
		break;
	case 3002:
		// Pointing Right
		CursorMan.replaceCursor(s_pointingRightCursor, 15, 13, 14, 3, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 3);
		break;
	case 3003:
		// Pointing Down (Palm Up)
		CursorMan.replaceCursor(s_pointingDownCursorPalmUp, 13, 16, 3, 15, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 3);
		break;
	case 3004:
		// Pointing Up (Palm Up)
		CursorMan.replaceCursor(s_pointingUpCursorPalmUp, 13, 16, 3, 0, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 3);
		break;
	case 3005:
		// Pointing Left (Curved)
		CursorMan.replaceCursor(s_pointingLeftCursorBent, 15, 13, 0, 5, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 3);
		break;
	case 3006:
		// Pointing Right (Curved)
		CursorMan.replaceCursor(s_pointingRightCursorBent, 15, 13, 14, 5, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 3);
		break;
	case 3007:
		// Pointing Down (Palm Down)
		CursorMan.replaceCursor(s_pointingDownCursorPalmDown, 15, 16, 7, 15, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 3);
		break;
	case 4001:
		// Red Marble
		CursorMan.replaceCursor(s_redMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_redMarbleCursorPalette, 1, ARRAYSIZE(s_redMarbleCursorPalette) / 3);
		break;
	case 4002:
		// Orange Marble
		CursorMan.replaceCursor(s_orangeMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_orangeMarbleCursorPalette, 1, ARRAYSIZE(s_orangeMarbleCursorPalette) / 3);
		break;
	case 4003:
		// Yellow Marble
		CursorMan.replaceCursor(s_yellowMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_yellowMarbleCursorPalette, 1, ARRAYSIZE(s_yellowMarbleCursorPalette) / 3);
		break;
	case 4004:
		// Green Marble
		CursorMan.replaceCursor(s_greenMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_greenMarbleCursorPalette, 1, ARRAYSIZE(s_greenMarbleCursorPalette) / 3);
		break;
	case 4005:
		// Blue Marble
		CursorMan.replaceCursor(s_blueMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_blueMarbleCursorPalette, 1, ARRAYSIZE(s_blueMarbleCursorPalette) / 3);
		break;
	case 4006:
		// Violet Marble
		CursorMan.replaceCursor(s_violetMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_violetMarbleCursorPalette, 1, ARRAYSIZE(s_violetMarbleCursorPalette) / 3);
		break;
	case 5000:
		// Pellet
		CursorMan.replaceCursor(s_pelletCursor, 8, 8, 4, 4, 0);
		CursorMan.replaceCursorPalette(s_pelletCursorPalette, 1, ARRAYSIZE(s_pelletCursorPalette) / 3);
		break;
	case 9000:
		// Hide Cursor
		CursorMan.showMouse(false);
		break;
	default:
		error("Cursor %d does not exist!", id);
	}

	if (id != 9000) // Show Cursor
		CursorMan.showMouse(true);

	// Should help in cases where we need to hide the cursor immediately.
	g_system->updateScreen();
}

void RivenCursorManager::setDefaultCursor() {
	setCursor(kRivenMainCursor);
}

NECursorManager::NECursorManager(const Common::String &appName) {
	_exe = new Common::NEResources();

	if (!_exe->loadFromEXE(appName)) {
		// Not all have cursors anyway, so this is not a problem
		delete _exe;
		_exe = 0;
	}
}

NECursorManager::~NECursorManager() {
	delete _exe;
}

void NECursorManager::setCursor(uint16 id) {
	if (!_exe) {
		Common::Array<Common::NECursorGroup> cursors = _exe->getCursors();

		for (uint32 i = 0; i < cursors.size(); i++) {
			if (cursors[i].id == id) {
				Common::NECursor *cursor = cursors[i].cursors[0];
				CursorMan.replaceCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(), cursor->getHotspotX(), cursor->getHotspotY(), 0);

				const byte *srcPal = cursor->getPalette();
				byte pal[3 * 256];
				for (uint j = 0; j < 256; ++j) {
					pal[j * 3 + 0] = srcPal[j * 4 + 0];
					pal[j * 3 + 1] = srcPal[j * 4 + 1];
					pal[j * 3 + 2] = srcPal[j * 4 + 2];
				}

				CursorMan.replaceCursorPalette(pal, 0, 256);
				return;
			}
		}
	}

	// Last resort (not all have cursors)
	setDefaultCursor();
}

MacCursorManager::MacCursorManager(const Common::String &appName) {
	if (!appName.empty()) {
		_resFork = new Common::MacResManager();

		if (!_resFork->open(appName)) {
			// Not all have cursors anyway, so this is not a problem
			delete _resFork;
			_resFork = 0;
		}
	}
}

MacCursorManager::~MacCursorManager() {
	delete _resFork;
}

void MacCursorManager::setCursor(uint16 id) {
	if (!_resFork) {
		setDefaultCursor();
		return;
	}

	Common::SeekableReadStream *stream = _resFork->getResource(MKID_BE('CURS'), id);

	if (!stream) {
		setDefaultCursor();
		return;
	}

	byte cursorBitmap[16 * 16];
	decodeMacXorCursor(stream, cursorBitmap);
	uint16 hotspotX = stream->readUint16BE();
	uint16 hotspotY = stream->readUint16BE();

	CursorMan.replaceCursor(cursorBitmap, 16, 16, hotspotX, hotspotY, 0);
	CursorMan.replaceCursorPalette(s_bwPalette, 1, 2);

	delete stream;
}

LivingBooksCursorManager_v2::LivingBooksCursorManager_v2() {
	// Try to open the system archive if we have it
	_sysArchive = new MohawkArchive();

	if (!_sysArchive->open("system.mhk")) {
		delete _sysArchive;
		_sysArchive = 0;
	}
}

LivingBooksCursorManager_v2::~LivingBooksCursorManager_v2() {
	delete _sysArchive;
}

void LivingBooksCursorManager_v2::setCursor(uint16 id) {
	if (_sysArchive && _sysArchive->hasResource(ID_TCUR, id)) {
		setStandardCursor(_sysArchive->getResource(ID_TCUR, id));
	} else {
		// TODO: Handle generated cursors
	}
}

} // End of namespace Mohawk
