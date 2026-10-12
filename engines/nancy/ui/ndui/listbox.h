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

#ifndef NANCY_UI_NDUI_LISTBOX_H
#define NANCY_UI_NDUI_LISTBOX_H

#include "engines/nancy/ui/ndui/control.h"

namespace Nancy {
namespace UI {
namespace NDUI {

class ScrollBar;

class ListBox : public Control {
public:
	ListBox(Panel *panel) : Control(panel) {}
	~ListBox() override;

	bool update(float seconds) override;
	void draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const override;

	bool containsPoint(const Common::Point &pos) const override { return _bounds.contains(pos); }
	void onMouseHover(const Common::Point &pos) override;
	bool onMouseDown(const Common::Point &pos) override;
	void onMouseMove(const Common::Point &pos) override;
	void onMouseUp(const Common::Point &pos) override;
	bool onWheel(int notches) override;

protected:
	void readExtra(Common::SeekableReadStream &stream) override;
	bool doAction(const Action &action) override;
	bool getValue(int32 eventType, double &value, Common::String &text) const override;
	bool isOwnEvent(int32 eventType) const override { return eventType == kEventListSelected; }

	struct Item {
		Common::String text;
		float color[4] = {};
	};

	void addItem(const Common::String &text, bool atStart);
	void updateLayout();
	Common::Rect getSelectionBounds() const;
	Common::Rect getTextBounds() const;
	int getItemAt(const Common::Point &pos) const;

	ScrollBar *_scrollBar = nullptr;
	int32 _border = 0;
	int32 _margin = 0;
	Common::Array<Item> _items;
	int _selected = -1;
	int _hovered = -1;
	int _rowHeight = 0;
	bool _isDragging = false;
	bool _isScrollBarDragging = false;
	int _lastClicked = -1;
	uint32 _lastClickTime = 0;
};

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_LISTBOX_H
