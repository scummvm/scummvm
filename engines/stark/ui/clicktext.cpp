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

#include "engines/stark/ui/clicktext.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/services/services.h"

#include "engines/stark/visual/image.h"

namespace Stark {

ClickText::ClickText(const Common::String &text, Common::Point pos)
	: _position(pos),
	  _text(text) {
	Gfx::Driver *gfx = StarkServices::instance().gfx;
	_texturePassive = gfx->createTextureFromString(text, 0xFFFF0000);
	_textureActive = gfx->createTextureFromString(_text, 0xFF00FF00);
	_curTexture = _texturePassive;
	_bbox = gfx->getBoundingBoxForString(text);
}

void ClickText::render() {
	Gfx::Driver *gfx = StarkServices::instance().gfx;
	gfx->drawSurface(_curTexture, _position);
}

bool ClickText::containsPoint(Common::Point point) {
	Common::Rect r = _bbox;
	r.translate(_position.x, _position.y);
	return r.contains(point);
}

void ClickText::handleMouseOver() {
	_curTexture = _textureActive;
}

} // End of namespace Stark
