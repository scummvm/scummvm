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

#ifndef NANCY_UI_NDUI_SLIDER_H
#define NANCY_UI_NDUI_SLIDER_H

#include "engines/nancy/ui/ndui/control.h"

namespace Nancy {
namespace UI {
namespace NDUI {

class Slider : public Control {
public:
	Slider(Panel *panel) : Control(panel) {}

	bool update(float seconds) override;
	void draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const override;

	bool containsPoint(const Common::Point &pos) const override { return _bounds.contains(pos); }
	Common::Rect getDrawBounds() const override;
	bool onMouseDown(const Common::Point &pos) override;
	void onMouseMove(const Common::Point &pos) override;
	void onMouseUp(const Common::Point &pos) override;

protected:
	void readExtra(Common::SeekableReadStream &stream) override;
	bool doAction(const Action &action) override;
	bool getValue(int32 eventType, double &value, Common::String &text) const override;
	bool isOwnEvent(int32 eventType) const override { return eventType == kEventSliderChanged; }

	Common::Rect getThumbBounds() const;
	int valueAt(int x) const;
	void setValue(int value, bool byPlayer);

	bool _hasFill = false;		// a third element, drawn over the track up to the thumb
	int32 _min = 0;
	int32 _max = 0;
	int32 _value = 0;
	int _dragOffset = 0;
};

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_SLIDER_H
