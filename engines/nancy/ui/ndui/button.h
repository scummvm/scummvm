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

#ifndef NANCY_UI_NDUI_BUTTON_H
#define NANCY_UI_NDUI_BUTTON_H

#include "engines/nancy/enginedata.h"

#include "engines/nancy/ui/ndui/control.h"

namespace Nancy {
namespace UI {
namespace NDUI {

class Button : public Control {
public:
	Button(Panel *panel) : Control(panel) {}

	bool update(float seconds) override;
	void draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const override;

	bool containsPoint(const Common::Point &pos) const override { return _bounds.contains(pos); }
	bool onMouseDown(const Common::Point &pos) override;
	void onMouseUp(const Common::Point &pos) override;

protected:
	void readExtra(Common::SeekableReadStream &stream) override;
	bool doAction(const Action &action) override;
	bool getValue(int32 eventType, double &value, Common::String &text) const override;
	bool isOwnEvent(int32 eventType) const override { return eventType == kEventClicked; }
	ControlState getButtonState() const;

	Common::Point _pressedOffset;
};

// One inventory item; its text shows how many of the item the player has
class InvButton : public Button {
public:
	// What clicking an item does
	enum ItemKind {
		kItemPickUp				= 1,	// the item gets held
		kItemView				= 3,	// the item leaves the inventory while its scene is up
		kItemOpenCountedScene	= 4,	// opens a scene; the button shows a table value
		kItemOpenScene			= 6		// opens a scene
	};

	InvButton(Panel *panel) : Button(panel) {}

	// Nancy17 buttons are made from a template, and get their item afterwards
	void setItem(const INVD::Item &item);

	int16 getItemID() const { return _itemID; }
	int32 getKind() const { return _kind; }
	uint16 getScene() const { return (uint16)_scene; }
	int16 getValueIndex() const { return _valueIndex; }

protected:
	void readExtra(Common::SeekableReadStream &stream) override;
	bool doAction(const Action &action) override;
	bool getValue(int32 eventType, double &value, Common::String &text) const override;

	int16 _itemID = -1;
	int32 _kind = kItemPickUp;
	int32 _scene = 0;
	int16 _valueIndex = 0;
};

class CheckBox : public Control {
public:
	CheckBox(Panel *panel) : Control(panel) {}

	bool update(float seconds) override;
	void draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const override;

	bool containsPoint(const Common::Point &pos) const override { return _bounds.contains(pos); }
	bool onMouseDown(const Common::Point &pos) override;
	void onMouseUp(const Common::Point &pos) override;

	bool isChecked() const { return _checked; }
	void setChecked(bool checked) { _checked = checked; }

protected:
	void readExtra(Common::SeekableReadStream &stream) override;
	bool doAction(const Action &action) override;
	bool getValue(int32 eventType, double &value, Common::String &text) const override;
	bool isOwnEvent(int32 eventType) const override;

	virtual void onClicked();
	Common::Rect getBoxBounds() const;
	int getTextWidth() const override;
	int getMinHeight() const override { return _elements[0]._src.height(); }
	void readCheckTexture(Common::SeekableReadStream &stream);

	bool _checked = false;
};

// A check box that unchecks the other buttons of its group when it gets checked
class RadioButton : public CheckBox {
public:
	RadioButton(Panel *panel) : CheckBox(panel) {}

	const Common::String &getGroup() const { return _group; }
	void setGroup(const Common::String &group) { _group = group; }

protected:
	void readExtra(Common::SeekableReadStream &stream) override;
	bool doAction(const Action &action) override;
	bool getValue(int32 eventType, double &value, Common::String &text) const override;
	bool isOwnEvent(int32 eventType) const override { return eventType == kEventRadioChanged; }

	void onClicked() override;

	Common::String _group;
};

// Holds a set of radio buttons, which get added to the panel as controls of their own
class RadioGroup : public Static {
public:
	RadioGroup(Panel *panel) : Static(panel) {}

protected:
	void readExtra(Common::SeekableReadStream &stream) override;
};

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_BUTTON_H
