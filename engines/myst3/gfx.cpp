/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#if defined(WIN32)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#endif

#include "engines/myst3/gfx.h"

namespace Myst3 {

BaseRenderer::BaseRenderer(OSystem *system)
		: _system(system), _font(NULL) { }

BaseRenderer::~BaseRenderer() {
	if (_font)
		freeTexture(_font);
}

void BaseRenderer::init(Graphics::PixelBuffer &screenBuffer) {
}

void BaseRenderer::initFont(const Graphics::Surface *surface) {
	_font = createTexture(surface);
}

Texture *BaseRenderer::createTexture(const Graphics::Surface *surface) {
	return NULL;
}

void BaseRenderer::freeTexture(Texture *texture) {
}

Common::Rect BaseRenderer::getFontCharacterRect(uint8 character) {
	uint index = 0;

	if (character == ' ')
		index = 0;
	else if (character >= '0' && character <= '9')
		index = 1 + character - '0';
	else if (character >= 'A' && character <= 'Z')
		index = 1 + 10 + character - 'A';
	else if (character == '|')
		index = 1 + 10 + 26;

	return Common::Rect(16 * index, 0, 16 * (index + 1), 32);
}

} // End of namespace Myst3
