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

#ifndef NANCY_UI_NDUI_SCROLLBAR_H
#define NANCY_UI_NDUI_SCROLLBAR_H

#include "engines/nancy/ui/ndui/control.h"

namespace Nancy {
namespace UI {
namespace NDUI {

class ScrollBar : public Control {
public:
	ScrollBar(Panel *panel) : Control(panel) {}

	bool update(float seconds) override;
	void draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const override;

	bool containsPoint(const Common::Point &pos) const override { return _bounds.contains(pos); }
	bool onMouseDown(const Common::Point &pos) override;
	void onMouseMove(const Common::Point &pos) override;
	void onMouseUp(const Common::Point &pos) override;
	bool onWheel(int notches) override;

	int32 getPosition() const { return _position; }
	void setRange(int32 end);
	void setPageSize(int32 pageSize);
	void scroll(int32 delta);

protected:
	void readExtra(Common::SeekableReadStream &stream) override;
	bool doAction(const Action &action) override;
	bool getValue(int32 eventType, double &value, Common::String &text) const override;
	bool isOwnEvent(int32 eventType) const override { return eventType == kEventScrolled; }

	enum ArrowState {
		kArrowNone,
		kArrowUp,
		kArrowDown
	};

	Common::Rect getUpButton() const;
	Common::Rect getDownButton() const;
	Common::Rect getTrack() const;
	void capPosition();
	void updateThumb();

	int32 _pageSize = 0;
	int32 _end = 0;
	int32 _position = 0;
	bool _showThumb = false;
	Common::Rect _thumb;

	bool _isDragging = false;
	int _dragOffset = 0;
	ArrowState _arrow = kArrowNone;
	float _arrowHeldTime = 0;
	float _arrowRepeatTime = 0;
	Common::Point _lastMouse;
};

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_SCROLLBAR_H
