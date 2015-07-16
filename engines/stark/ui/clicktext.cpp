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

#include "engines/stark/services/services.h"

#include "engines/stark/visual/text.h"

namespace Stark {

ClickText::ClickText(const Common::String &text, Common::Point pos)
	: _position(pos),
	  _text(text) {
	_visualPassive = new VisualText(StarkGfx);
	_visualPassive->setText(_text);
	_visualPassive->setColor(0xFFFF0000);

	_visualActive = new VisualText(StarkGfx);
	_visualActive->setText(_text);
	_visualActive->setColor(0xFF00FF00);

	_curVisual = _visualPassive;
	_bbox = _curVisual->getRect();
}

ClickText::~ClickText() {
	delete _visualActive;
	delete _visualPassive;
}

void ClickText::render() {
	_curVisual->render(_position);
}

bool ClickText::containsPoint(Common::Point point) {
	Common::Rect r = _bbox;
	r.translate(_position.x, _position.y);
	return r.contains(point);
}

void ClickText::handleMouseOver() {
	_curVisual = _visualActive;
}

} // End of namespace Stark
