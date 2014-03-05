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
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/graphics.h"

namespace MADS {

void ScreenSurface::init() {
	setSize(g_system->getWidth(), g_system->getHeight());
}

void ScreenSurface::updateScreen() {
	g_system->copyRectToScreen((const byte *)pixels, pitch, 0, 0, w, h);
	g_system->updateScreen();
}

void ScreenSurface::transition(bool transitionFlag, bool surfaceFlag) {

}

void ScreenSurface::setPointer(MSurface *s) {
	_pixels = s->getData();
	_pitch = s->w;
}


} // End of namespace MADS
