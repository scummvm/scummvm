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

#include "common/system.h"
#include "graphics/cursorman.h"

namespace Mohawk {

void CursorManager::showCursor() {
	CursorMan.showMouse(true);
}

void CursorManager::hideCursor() {
	CursorMan.showMouse(false);
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
		CursorMan.replaceCursorPalette(mhkSurface->getPalette(), 0, 256);
	} else {
		Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
		CursorMan.replaceCursor((byte *)surface->pixels, surface->w, surface->h, hotspotX, hotspotY, pixelFormat.RGBToColor(255, 255, 255), 1, &pixelFormat);
	}

	_vm->_needsUpdate = true;
	delete mhkSurface;
}

void RivenCursorManager::setCursor(uint16 id) {
	// All of Riven's cursors are hardcoded. See riven_cursors.h for these definitions.

	switch (id) {
	case 1002:
		// Zip Mode
		CursorMan.replaceCursor(s_zipModeCursor, 16, 16, 8, 8, 0);
		CursorMan.replaceCursorPalette(s_zipModeCursorPalette, 1, ARRAYSIZE(s_zipModeCursorPalette) / 4);
		break;
	case 2003:
		// Hand Over Object
		CursorMan.replaceCursor(s_objectHandCursor, 16, 16, 8, 8, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 4);
		break;
	case 2004:
		// Grabbing/Using Object
		CursorMan.replaceCursor(s_grabbingHandCursor, 13, 13, 6, 6, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 4);
		break;
	case 3000:
		// Standard Hand
		CursorMan.replaceCursor(s_standardHandCursor, 15, 16, 6, 0, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 4);
		break;
	case 3001:
		// Pointing Left
		CursorMan.replaceCursor(s_pointingLeftCursor, 15, 13, 0, 3, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 4);
		break;
	case 3002:
		// Pointing Right
		CursorMan.replaceCursor(s_pointingRightCursor, 15, 13, 14, 3, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 4);
		break;
	case 3003:
		// Pointing Down (Palm Up)
		CursorMan.replaceCursor(s_pointingDownCursorPalmUp, 13, 16, 3, 15, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 4);
		break;
	case 3004:
		// Pointing Up (Palm Up)
		CursorMan.replaceCursor(s_pointingUpCursorPalmUp, 13, 16, 3, 0, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 4);
		break;
	case 3005:
		// Pointing Left (Curved)
		CursorMan.replaceCursor(s_pointingLeftCursorBent, 15, 13, 0, 5, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 4);
		break;
	case 3006:
		// Pointing Right (Curved)
		CursorMan.replaceCursor(s_pointingRightCursorBent, 15, 13, 14, 5, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 4);
		break;
	case 3007:
		// Pointing Down (Palm Down)
		CursorMan.replaceCursor(s_pointingDownCursorPalmDown, 15, 16, 7, 15, 0);
		CursorMan.replaceCursorPalette(s_handCursorPalette, 1, ARRAYSIZE(s_handCursorPalette) / 4);
		break;
	case 4001:
		// Red Marble
		CursorMan.replaceCursor(s_redMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_redMarbleCursorPalette, 1, ARRAYSIZE(s_redMarbleCursorPalette) / 4);
		break;
	case 4002:
		// Orange Marble
		CursorMan.replaceCursor(s_orangeMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_orangeMarbleCursorPalette, 1, ARRAYSIZE(s_orangeMarbleCursorPalette) / 4);
		break;
	case 4003:
		// Yellow Marble
		CursorMan.replaceCursor(s_yellowMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_yellowMarbleCursorPalette, 1, ARRAYSIZE(s_yellowMarbleCursorPalette) / 4);
		break;
	case 4004:
		// Green Marble
		CursorMan.replaceCursor(s_greenMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_greenMarbleCursorPalette, 1, ARRAYSIZE(s_greenMarbleCursorPalette) / 4);
		break;
	case 4005:
		// Blue Marble
		CursorMan.replaceCursor(s_blueMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_blueMarbleCursorPalette, 1, ARRAYSIZE(s_blueMarbleCursorPalette) / 4);
		break;
	case 4006:
		// Violet Marble
		CursorMan.replaceCursor(s_violetMarbleCursor, 12, 12, 5, 5, 0);
		CursorMan.replaceCursorPalette(s_violetMarbleCursorPalette, 1, ARRAYSIZE(s_violetMarbleCursorPalette) / 4);
		break;
	case 5000:
		// Pellet
		CursorMan.replaceCursor(s_pelletCursor, 8, 8, 4, 4, 0);
		CursorMan.replaceCursorPalette(s_pelletCursorPalette, 1, ARRAYSIZE(s_pelletCursorPalette) / 4);
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

} // End of namespace Mohawk
