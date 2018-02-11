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

#include "engines/stark/ui/world/button.h"

#include "engines/stark/services/services.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/visual/image.h"
#include "engines/stark/visual/text.h"

namespace Stark {

Button::Button(const Common::String &text, StaticProvider::UIElement stockElement, const Common::Point &pos, HintAlign align, const Common::Point &hintPos) :
		_position(pos),
		_stockElement(stockElement),
		_text(text),
		_hintPosition(hintPos),
		_align(align),
		_mouseText(nullptr),
		_renderHint(false) {
}

Button::~Button() {
	delete _mouseText;
}

void Button::render() {
	VisualImageXMG *image = StarkStaticProvider->getUIElement(_stockElement);
	image->render(_position, false);
	if (_renderHint) {
		Common::Point pos(_hintPosition);
		if (_align == kAlignRight) {
			pos.x -= _mouseText->getRect().width();
		}
		_mouseText->render(pos);
	}
	_renderHint = false;
}

bool Button::containsPoint(Common::Point point) {
	VisualImageXMG *image = StarkStaticProvider->getUIElement(_stockElement);

	Common::Rect r;
	r.left = _position.x;
	r.top = _position.y;
	r.setWidth(image->getWidth());
	r.setHeight(image->getHeight());
	return r.contains(point);
}

void Button::showButtonHint() {
	if (!_mouseText) {
		_mouseText = new VisualText(StarkGfx);
		_mouseText->setText(_text);
		_mouseText->setColor(0xFFFFFFFF);
		_mouseText->setFont(FontProvider::kSmallFont);
		_mouseText->setTargetWidth(96);
	}
	_renderHint = true;
}

void Button::resetHintVisual() {
	delete _mouseText;
	_mouseText = nullptr;
}

} // End of namespace Stark
