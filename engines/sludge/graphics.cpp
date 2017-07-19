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

#include "sludge/backdrop.h"
#include "sludge/graphics.h"

namespace Sludge {

GraphicsManager::GraphicsManager() {
	_parallaxStuff = new Parallax;
}

GraphicsManager::~GraphicsManager() {
	delete _parallaxStuff;
	_parallaxStuff = nullptr;
}

bool GraphicsManager::loadParallax(uint16 v, uint16 fracX, uint16 fracY) {
	if (!_parallaxStuff)
		_parallaxStuff = new Parallax;
	return _parallaxStuff->add(v, fracX, fracY);
}

void GraphicsManager::killParallax() {
	if (!_parallaxStuff)
		return;
	_parallaxStuff->kill();
}

void GraphicsManager::saveParallax(Common::WriteStream *fp) {
	if (_parallaxStuff)
		_parallaxStuff->save(fp);
}

void GraphicsManager::drawParallax() {
	if (_parallaxStuff)
		_parallaxStuff->draw();
}



} // End of namespace Sludge
