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

#include "engines/stark/ui/world/clicktext.h"
#include "engines/stark/services/services.h"
#include "engines/stark/visual/text.h"

namespace Stark {

ClickText::ClickText(const Common::String &text, const Color &color) :
		_text(text),
		_color(color) {
	_visualPassive = new VisualText(StarkGfx);
	_visualPassive->setText(_text);
	_visualPassive->setColor(_color);
	_visualPassive->setFont(FontProvider::kBigFont);
	_visualPassive->setTargetWidth(600);

	_visualActive = new VisualText(StarkGfx);
	_visualActive->setText(_text);
	_visualActive->setColor(Color(0x00, 0x00, 0x00));
	_visualActive->setBackgroundColor(_color);
	_visualActive->setFont(FontProvider::kBigFont);
	_visualActive->setTargetWidth(600);

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

bool ClickText::containsPoint(const Common::Point &point) const {
	Common::Rect r = _bbox;
	r.translate(_position.x, _position.y);
	return r.contains(point);
}

} // End of namespace Stark
