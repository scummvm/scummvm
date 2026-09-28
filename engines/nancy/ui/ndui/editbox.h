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

#ifndef NANCY_UI_NDUI_EDITBOX_H
#define NANCY_UI_NDUI_EDITBOX_H

#include "common/keyboard.h"

#include "engines/nancy/ui/ndui/control.h"

namespace Nancy {
namespace UI {
namespace NDUI {

// A single line of text the player can type in
class EditBox : public Control {
public:
	EditBox(Panel *panel) : Control(panel) {}

	bool update(float seconds) override;
	void draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const override;

	bool containsPoint(const Common::Point &pos) const override { return _bounds.contains(pos); }
	bool onMouseDown(const Common::Point &pos) override;
	void onMouseMove(const Common::Point &pos) override;

	void handleKey(const Common::KeyState &key);

protected:
	void readExtra(Common::SeekableReadStream &stream) override;
	bool doAction(const Action &action) override;
	bool getValue(int32 eventType, double &value, Common::String &text) const override;
	bool isOwnEvent(int32 eventType) const override { return eventType == kEventTextChanged; }

	Common::Rect getTextBounds() const;
	int getCharX(int index) const;
	int getCharAt(int x) const;
	void deleteSelection();
	void moveCaret(int caret, bool select);
	void scrollToCaret();

	uint32 _selectedTextColor = 0;
	uint32 _selectionColor = 0;
	uint32 _caretColor = 0;
	int32 _border = 0;

	int _caret = 0;
	int _selectionStart = 0;	// equals the caret when nothing is selected
	int _scroll = 0;
	float _blinkTime = 0;
	bool _isCaretShown = true;
};

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_EDITBOX_H
