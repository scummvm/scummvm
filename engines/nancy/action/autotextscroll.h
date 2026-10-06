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

#ifndef NANCY_ACTION_AUTOTEXTSCROLL_H
#define NANCY_ACTION_AUTOTEXTSCROLL_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Nancy16 scrolling text, used for the end credits. Each of the listed AUTOTEXT
// entries is rendered as hypertext and scrolled through a rectangle, one after another.
// Once they have all scrolled out of view, an event flag gets set.
class AutotextScroll : public RenderActionRecord {
public:
	AutotextScroll(bool hasDrawingOrder = false) : RenderActionRecord(7), _hasDrawingOrder(hasDrawingOrder) {}
	virtual ~AutotextScroll();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "AutotextScroll"; }

	struct ScrollingText {
		Graphics::ManagedSurface image;
		int32 ticks = 0;
		int32 offset = 0;		// scroll position; the text starts this far above the top of the rect
	};

	void startText(const Common::String &key);
	bool scrollText(ScrollingText &text, int32 ticks);
	void update(int32 ticks);

	void redraw();

	// -- File data --
	bool _hasDrawingOrder;			// Nancy18 has a type that can be put above or below other records
	int32 _startDepth = 0;			// how far below the top of the rect the text starts
	Common::Rect _bounds;
	int32 _intervalTicks = 0;		// between two entries starting
	float _speed = 0;				// pixels per tick; negative scrolls up
	uint32 _fontID = 0;
	uint32 _colorID = 0;
	Common::Array<Common::String> _keys;
	FlagDescription _doneFlag;

	// -- Runtime state --
	uint _nextKey = 0;
	int32 _intervalCounter = 0;
	Common::Array<ScrollingText *> _texts;
	uint32 _lastTickTime = 0;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_AUTOTEXTSCROLL_H
