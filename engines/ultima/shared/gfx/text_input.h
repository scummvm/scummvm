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

#ifndef ULTIMA_SHARED_GFX_TEXT_INPUT_H
#define ULTIMA_SHARED_GFX_TEXT_INPUT_H

#include "ultima/shared/engine/events.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

class TextInput : public Shared::UIElement {
private:
	Common::String _text;
	bool _isNumeric = false;
	bool _visible = false;
	bool _caretVisible = false;

public:
	TextInput(const Common::String &name, UIElement *parent = nullptr);
	TextInput(const Common::String &name, UIElement *parent, const Common::Point &pos,
		int maxChars, bool isNumeric = false);
	~TextInput() override {
	}

	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void timeout() override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;

	void show(const Common::Point &pos, int maxChars, bool isNumeric = false);
	void show();
	void hide();
	void clear();
	void setTextPos(const Common::Point &pt);
};

} // namespace Gfx
} // namespace Shared
} // namespace Ultima

#endif
