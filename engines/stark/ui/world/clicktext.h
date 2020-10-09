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

#ifndef STARK_UI_CLICK_TEXT_H
#define STARK_UI_CLICK_TEXT_H

#include "engines/stark/visual/text.h"

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str.h"

namespace Stark {

class VisualText;

class ClickText {
public:
	ClickText(const Common::String &text, const Color &color);
	~ClickText();

	void setPosition(const Common::Point &pos) { _position = pos; }
	void render();
	bool containsPoint(const Common::Point &point) const;
	int getHeight() const { return _bbox.height(); }

	void setActive() { _curVisual = _visualActive; }
	void setPassive() { _curVisual = _visualPassive; }

private:
	VisualText *_visualPassive;
	VisualText *_visualActive;
	VisualText *_curVisual;
	Common::Point _position;
	Common::String _text;
	Common::Rect _bbox;
	Color _color;
};

} // End of namespace Stark

#endif // STARK_UI_CLICK_TEXT_H
