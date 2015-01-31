/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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


#include "image/png.h"
#include "graphics/surface.h"
#include "engines/grim/remastered/overlay.h"
#include "engines/grim/resource.h"
#include "engines/grim/material.h"
#include "engines/grim/gfx_base.h"

namespace Grim {

Overlay::Overlay(const Common::String &filename, Common::SeekableReadStream *data) :
		_x(0), _y(0) {
	_material = g_resourceloader->loadMaterial(filename, NULL, true);
}

Overlay::~Overlay() {

}

void Overlay::draw() {
	_material->select();
	g_driver->drawOverlay(this);
}

int Overlay::getWidth() const {
	return _material->getData()->_textures[0]->_width;
}

int Overlay::getHeight() const {
	return _material->getData()->_textures[0]->_height;
}

}
