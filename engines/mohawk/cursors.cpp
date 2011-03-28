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

#include "mohawk/cursors.h"
#include "mohawk/mohawk.h"
#include "mohawk/resource.h"

#include "common/macresman.h"
#include "common/system.h"
#include "common/winexe_ne.h"
#include "common/winexe_pe.h"
#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#ifdef ENABLE_MYST
#include "mohawk/bitmap.h"
#include "mohawk/myst.h"
#endif

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

void CursorManager::setMacXorCursor(Common::SeekableReadStream *stream) {
	assert(stream);

	byte cursorBitmap[16 * 16];

	// Get black and white data
	for (int i = 0; i < 32; i++) {
		byte imageByte = stream->readByte();
		for (int b = 0; b < 8; b++)
			cursorBitmap[i * 8 + b] = (imageByte & (0x80 >> b)) ? 1 : 2;
	}

	// Apply mask data
	for (int i = 0; i < 32; i++) {
		byte imageByte = stream->readByte();
		for (int b = 0; b < 8; b++)
			if ((imageByte & (0x80 >> b)) == 0)
				cursorBitmap[i * 8 + b] = 0;
	}

	uint16 hotspotY = stream->readUint16BE();
	uint16 hotspotX = stream->readUint16BE();

	CursorMan.replaceCursor(cursorBitmap, 16, 16, hotspotX, hotspotY, 0);
	CursorMan.replaceCursorPalette(s_bwPalette, 1, 2);
}

void DefaultCursorManager::setCursor(uint16 id) {
	setMacXorCursor(_vm->getResource(_tag, id));
}

#ifdef ENABLE_MYST

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
		CursorMan.replaceCursorPalette(mhkSurface->getPalette(), 0, 256);
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

#endif

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
	if (_exe) {
		Graphics::WinCursorGroup *cursorGroup = Graphics::WinCursorGroup::createCursorGroup(*_exe, id);

		if (cursorGroup) {
			Graphics::WinCursor *cursor = cursorGroup->cursors[0].cursor;
			CursorMan.replaceCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(), cursor->getHotspotX(), cursor->getHotspotY(), cursor->getKeyColor());
			CursorMan.replaceCursorPalette(cursor->getPalette(), 0, 256);
			return;
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
	} else {
		_resFork = 0;
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

	// Try a color cursor first
	Common::SeekableReadStream *stream = _resFork->getResource(MKID_BE('crsr'), id);

	if (stream) {
		byte *cursor, *palette;
		int width, height, hotspotX, hotspotY, keyColor, palSize;

		_resFork->convertCrsrCursor(stream, &cursor, width, height, hotspotX, hotspotY, keyColor, true, &palette, palSize);

		CursorMan.replaceCursor(cursor, width, height, hotspotX, hotspotY, keyColor);
		CursorMan.replaceCursorPalette(palette, 0, palSize);

		delete[] cursor;
		delete[] palette;
		delete stream;
		return;
	}

	// Fall back to b&w cursors
	stream = _resFork->getResource(MKID_BE('CURS'), id);

	if (stream) {
		setMacXorCursor(stream);
		delete stream;
	} else {
		setDefaultCursor();
	}
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
		setMacXorCursor(_sysArchive->getResource(ID_TCUR, id));
	} else {
		// TODO: Handle generated cursors
	}
}

PECursorManager::PECursorManager(const Common::String &appName) {
	_exe = new Common::PEResources();

	if (!_exe->loadFromEXE(appName)) {
		// Not all have cursors anyway, so this is not a problem
		delete _exe;
		_exe = 0;
	}
}

PECursorManager::~PECursorManager() {
	delete _exe;
}

void PECursorManager::setCursor(uint16 id) {
	if (_exe) {
		Graphics::WinCursorGroup *cursorGroup = Graphics::WinCursorGroup::createCursorGroup(*_exe, id);

		if (cursorGroup) {
			Graphics::WinCursor *cursor = cursorGroup->cursors[0].cursor;
			CursorMan.replaceCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(), cursor->getHotspotX(), cursor->getHotspotY(), cursor->getKeyColor());
			CursorMan.replaceCursorPalette(cursor->getPalette(), 0, 256);
			return;
		}
	}

	// Last resort (not all have cursors)
	setDefaultCursor();
}

} // End of namespace Mohawk
