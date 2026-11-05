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

#ifndef NANCY_SCRIPT_V5_WIDGETS_H
#define NANCY_SCRIPT_V5_WIDGETS_H

#include "engines/nancy/script/records.h"
#include "engines/nancy/script/types.h"

namespace Nancy {
namespace Script {

struct RefImage;

// Nancy26's place on the screen the mouse can be over and click, which NavLogic
// records are as well. The player's input goes to the hotspot the mouse is over
// that is closest; while a button is down, it stays with the one it went down on.
// A hotspot calls OnLeftDown and OnLeftUp (or the right ones) of its scripts,
// and OnMouseOver, OnMouseLeave and OnLoseMouseFocus. Once it was clicked, it
// changes the scene when it has one, and is done.
// The shape is a rect, or the part of a Ref image that isn't see-through or has
// a color; the rect and the source then lay the image over the screen.
class HotspotRecord : public Record {
public:
	HotspotRecord(SceneContext &context) : Record(context, "ND.Hotspot") {}

	bool isMouseRegion() const override { return true; }
	bool hitTest(const ScriptPoint &point) const override;
	void setMouseOver(bool isOver) override;
	bool pressRegion(int button) override;
	void releaseRegion(int button) override;
	bool getRegionCursor(uint &cursorType, bool &isHot) const override;

	bool isMouseOver() const { return _isMouseOver; }
	bool isDown() const { return _pressedButton >= 0; }
	const ScriptRect &getOnScreen() const { return _onScreen; }

protected:
	void readTable(int tableIndex) override;
	bool prepareReset() override;
	bool hasTransform() const override { return true; }
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	struct Cursor {
		bool isSet = false;
		uint type = 0;
		bool isHot = false;
	};

	void readCursorField(int valueIndex, Cursor &cursor);
	void pushCursorField(const Cursor &cursor);
	void fire();

	ScriptRect _onScreen;
	Common::String _sceneName;
	Common::String _streamName;
	int _frame = 0;
	bool _watchesLeft = true;
	bool _watchesRight = false;

	Cursor _cursor;			// while the mouse is over it
	Cursor _startCursor;	// while it is over it with an item
	Cursor _downCursor;		// while it is down and the mouse is over it
	Cursor _dragOnCursor;
	Cursor _dragOffCursor;	// while it is down and the mouse left it

	const RefImage *_ref = nullptr;
	bool _hasRefColor = false;
	uint32 _refColor = 0;
	ScriptRect _source;

	bool _isMouseOver = false;
	int _pressedButton = -1;	// 0 for the left one, 1 for the right one
};

// Nancy26: catches the player's input in a part of the screen, so whatever is
// behind it gets none. It can let the keys through, or the mouse.
class SinkRecord : public Record {
public:
	SinkRecord(SceneContext &context) : Record(context, "ND.SinkAR") {}

	bool isMouseRegion() const override { return _handlesMouse; }
	bool hitTest(const ScriptPoint &point) const override;
	bool pressRegion(int button) override { return true; }
	bool getRegionCursor(uint &cursorType, bool &isHot) const override;
	bool capturesKeys() const override { return _handlesKeys; }

protected:
	void readTable(int tableIndex) override;
	bool prepareReset() override { return true; }
	bool hasTransform() const override { return true; }
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	ScriptRect _onScreen;
	bool _handlesKeys = true;
	bool _handlesMouse = true;
	bool _hasCursor = false;
	uint _cursorType = 0;
	bool _cursorIsHot = false;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_WIDGETS_H
