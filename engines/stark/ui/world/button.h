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

#ifndef STARK_UI_BUTTON_H
#define STARK_UI_BUTTON_H

#include "engines/stark/services/staticprovider.h"

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str.h"

namespace Stark {

class VisualImageXMG;
class VisualText;

namespace Resources {
class Anim;
}

class Button {
public:
	enum HintAlign { kLeft, kRight };

	Button(const Common::String &text, StaticProvider::UIElement stockElement, Common::Point pos, HintAlign align, Common::Point hintPos);
	~Button();

	void setPosition(const Common::Point &pos) { _position = pos; }
	void setHintPosition(const Common::Point &pos) { _hintPosition = pos; }
	/** Set hint to render for one frame */
	void showButtonHint();
	void render();
	bool containsPoint(Common::Point point);
	Common::String getText() const { return _text; }
private:
	StaticProvider::UIElement _stockElement;
	Common::Point _position;
	Common::Point _hintPosition;
	Common::String _text;
	VisualText *_mouseText;
	const HintAlign _align;
	bool _renderHint;
};

} // End of namespace Stark

#endif // STARK_UI_BUTTON_H
