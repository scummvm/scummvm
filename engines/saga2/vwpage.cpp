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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/rect.h"
#include "graphics/surface.h"
#include "saga2/std.h"
#include "saga2/vdraw.h"
#include "saga2/blitters.h"

namespace Saga2 {

vDisplayPage protoPage;

void vDisplayPage::fillRect(Rect16 r, uint8 color) {
	Graphics::Surface *surf = g_system->lockScreen();

	_FillRect((byte *)surf->getBasePtr(r.x, r.y), surf->pitch, r.width, r.height, color);

	g_system->unlockScreen();
}

//  Fill a rectangle on the SVGA display. Note the rect must
//  be correct -- there is no clipping or error checking...
void vDisplayPage::invertRect(Rect16 r, uint8 color) {
}

void vDisplayPage::writePixels(Rect16 &r, uint8 *pixPtr, uint16 pixMod) {
	g_system->copyRectToScreen(pixPtr, pixMod, r.x, r.y, r.width, r.height);
}

void vDisplayPage::writeTransPixels(Rect16 &r, uint8 *pixPtr, uint16 pixMod) {
	Graphics::Surface *surf = g_system->lockScreen();

	_BltPixelsT(pixPtr, pixMod, (byte *)surf->getBasePtr(r.x, r.y), surf->pitch, r.width, r.height );

	g_system->unlockScreen();
}

void vDisplayPage::readPixels(Rect16 &r, uint8 *pixPtr, uint16 pixMod) {
	warning("STUB: vWDisplayPage::readPixels()");
}

//  Function to quickly transfer pixels from an off-screen
//  buffer to a rectangle on the SVGA display;
void vDisplayPage::writeColorPixels(Rect16 r, uint8 *pixPtr, uint16 pixMod, uint8 color) {
	warning("STUB: writeColorPixels");
	writePixels(r, pixPtr, pixMod);
}

//  Function to quickly transfer pixels from an off-screen
//  buffer to a rectangle on the SVGA display;
void vDisplayPage::writeComplementPixels(Rect16 r, uint8 *pixPtr, uint16 pixMod, uint8 color) {
	warning("STUB: writeComplementPixels");
	writePixels(r, pixPtr, pixMod);
}

} // end of namespace Saga2
