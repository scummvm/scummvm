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

#include "engines/nancy/script/v5/bitmaptext.h"
#include "engines/nancy/script/v5/cliptransformer.h"

namespace Nancy {
namespace Script {

struct RefImage;
class WidgetRecord;

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

	RecordType getType() const override { return kRecordHotspot; }

	// A button or another widget takes over what the hotspot does
	void setWidget(WidgetRecord *widget) { _widget = widget; }

	bool isMouseRegion() const override { return true; }
	bool hitTest(const Common::Point32 &point) const override;
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
	WidgetRecord *_widget = nullptr;
};

// Nancy26: catches the player's input in a part of the screen, so whatever is
// behind it gets none. It can let the keys through, or the mouse.
class SinkRecord : public Record {
public:
	SinkRecord(SceneContext &context) : Record(context, "ND.SinkAR") {}

	RecordType getType() const override { return kRecordSink; }

	bool isMouseRegion() const override { return _handlesMouse; }
	bool hitTest(const Common::Point32 &point) const override;
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

// Nancy26's buttons, check boxes and radio buttons: a hotspot and overlays,
// shown one at a time for the state the widget is in. The records the widget
// is made of hang under it, so it hides, shows and moves them all.
class WidgetRecord : public ClipTransformerRecord {
public:
	WidgetRecord(SceneContext &context, RecordType type);

	RecordType getType() const override { return _type; }

	// What its hotspot tells it
	virtual void onHotspotOver(bool isOver);
	virtual void onHotspotDown(int button);
	virtual void onHotspotUp(int button, bool isOver);

	bool isSelected() const { return _isSelected; }
	void setSelected(bool isSelected, bool callsScripts);
	const Common::String &getGroupName() const { return _groupName; }

	void updateGraphics() override;

protected:
	void readTable(int tableIndex) override;
	bool prepareReset() override { return true; }
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

	// Picks the overlay for the state of the widget
	virtual void updateState();

	Record *readRecordField(int tableIndex, const char *name);
	void showOverlay(Record *overlay, bool isShown, bool isEnabled);

	RecordType _type;
	HotspotRecord *_hotspot = nullptr;
	Record *_baseOverlay = nullptr;
	Record *_upOverlay = nullptr;
	Record *_downOverlay = nullptr;
	Record *_overOverlay = nullptr;
	Record *_selectedOverlay = nullptr;
	Record *_text = nullptr;

	uint32 _baseColor = 0xFFFFFFFF;
	uint32 _disabledColor = 0xFF7F7F7F;

	bool _isSelected = false;
	Common::String _groupName;	// of a radio button
	bool _wasEnabled = true;
};

// Nancy26: a knob that the mouse drags around the knob area, or the wheel moves
// while the mouse is in the scroll area. "percent" is where the knob is, from
// 0 to 1 across and down.
class SliderRecord : public WidgetRecord {
public:
	SliderRecord(SceneContext &context) : WidgetRecord(context, kRecordSlider) {}

	void onHotspotDown(int button) override;
	void onHotspotUp(int button, bool isOver) override;

	// The wheel moves the knob while the mouse is in the scroll area
	bool scrollRegion(const Common::Point32 &point, int notches) override;

protected:
	void readTable(int tableIndex) override;
	void run(double sceneTime, double elapsedTime) override;
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;
	void updateState() override;

private:
	// How far the knob can go across and down
	Common::Point32 getKnobMax() const;
	void setKnobPos(const Common::Point32 &pos, bool callsScripts);
	void placeKnob();

	Record *_knobOverlay = nullptr;
	ScriptRect _knobArea;
	ScriptRect _scrollArea;
	Common::Point32 _scrollAmount;
	Common::Point32 _knobPos;
	Common::Point32 _dragOffset;	// from the knob to the mouse, while dragged
	bool _isDragging = false;
};

// Nancy26: a line of text the player types in, like the name of a saved game.
// A click gives it the keyboard; its scripts hear of every change (OnEdit), of
// Enter (OnEditComplete), of Tab, and of a letter that didn't fit (OnTooWide).
// What is selected gets replaced by what is typed next.
class EditBoxRecord : public Record {
public:
	EditBoxRecord(SceneContext &context);

	RecordType getType() const override { return kRecordEditBox; }
	~EditBoxRecord() override;

	bool isMouseRegion() const override { return true; }
	bool hitTest(const Common::Point32 &point) const override;
	bool pressRegion(int button) override;
	bool getRegionCursor(uint &cursorType, bool &isHot) const override;
	bool handleKey(Common::KeyCode key, uint16 ascii) override;

	void updateGraphics() override;
	void registerGraphics() override;

	void selectAll();
	void endSelection();

protected:
	void readTable(int tableIndex) override;
	bool prepareReset() override { return true; }
	void onZChanged() override;
	bool hasTransform() const override { return true; }
	void onTransformChanged() override { applyDest(); }
	bool getField(const Common::String &key) override;
	bool setField(const Common::String &key) override;

private:
	bool hasFocus() const;
	void applyDest();
	void setString(const Common::String &text, bool callsScripts);

	ScriptRect _dest;
	Common::String _text;
	bool _isAllSelected = false;

	int _fontID = 0;
	int _selectedColor = 0;		// font colors, while it has the keyboard and while not
	int _unselectedColor = 0;
	int _disabledSelectedColor = 0;
	int _disabledUnselectedColor = 0;
	uint32 _caretColor = 0xFFFFFFFF;
	uint32 _selectionColor = 0xFF3F3F3F;

	bool _hasCursor = false;
	uint _cursorType = 0;
	bool _cursorIsHot = false;

	RecordGraphic _graphic;
	BitmapText _layout;
	bool _isDirty = true;
	bool _hadFocus = false;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_WIDGETS_H
