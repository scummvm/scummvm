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

#include "engines/stark/gfx/texture.h"

#include "graphics/surface.h"

namespace Stark {
namespace Gfx {

Texture::Texture() :
		_width(0),
		_height(0) {
}

Texture::~Texture() {
}

TextureSet::TextureSet() {
}

TextureSet::~TextureSet() {
	for (TextureMap::iterator it = _texMap.begin(); it != _texMap.end(); ++it) {
		delete it->_value;
	}
}

void TextureSet::addTexture(const Common::String &name, Texture *texture) {
	if (_texMap.contains(name)) {
		error("A texture with the name '%s' already exists in the set.", name.c_str());
	}

	_texMap.setVal(name, texture);
}

const Texture *TextureSet::getTexture(const Common::String &name) const {
	TextureMap::const_iterator it = _texMap.find(name);
	if (it != _texMap.end())
		return it->_value;

	return nullptr;
}

} // End of namespace Gfx
} // End of namespace Stark
