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

#ifndef NANCY_UI_NDUI_PANEL_H
#define NANCY_UI_NDUI_PANEL_H

#include "engines/nancy/renderobject.h"

#include "engines/nancy/ui/ndui/common.h"

namespace Nancy {

struct NancyInput;

namespace UI {
namespace NDUI {

class Manager;
class Control;
class Static;
class RadioButton;
class EditBox;
class InvButton;

// A window of the UI, like the inventory, the journal or the bars around the viewport.
// Each NDUI chunk describes one panel: its background, which is a Static that also gives
// the panel its name and position, its controls, and how they get laid out. The panel
// draws its controls into its own surface, at the screen's resolution, whenever one of
// them changes. It passes the mouse to the control under it, and actions to the control
// they name. Scroll panels move their controls along with a scroll bar, and inventory
// panels show a button for every item the player has.
class Panel : public RenderObject {
public:
	Panel(Manager *manager, uint16 z, byte kind);
	virtual ~Panel();

	void read(Common::SeekableReadStream &stream);

	void updateGraphics() override;

	// Returns true when the action was meant for this panel or one of its controls
	bool takeAction(const Action &action);

	// Returns true when the mouse is over the panel, or held by one of its controls.
	// Panels further down can't be hovered once one above took the mouse.
	bool handleInput(NancyInput &input, bool canHover, bool &isOverControl);

	const Common::String &getName() const;
	bool isEnabled() const { return _enabled; }
	Manager *getManager() const { return _manager; }

	void addControl(Control *control);
	void addRadioButton(RadioButton *button);
	void setDirty() { _isDirty = true; }

	// Reads a control of the given type and adds it
	Control *addNewControl(int32 type, Common::SeekableReadStream &stream);
	bool removeControl(const Common::String &name);
	InvButton *findItemButton(int16 itemID) const;

	// The edit box that gets typed into, when the panel is up
	EditBox *getKeyboardFocus() const;

	// Places the visible controls one after another, when the panel asks for it
	void arrangeControls();

	void uncheckRadioGroup(const Common::String &group);
	int getCheckedRadioIndex(const Common::String &group) const;

private:
	enum Arrangement {
		kArrangeNone		= 0,
		kArrangeLeftToRight	= 1,
		kArrangeRightToLeft	= 2
	};

	enum Mode {
		kModeVisible			= 0,
		kModeVisibleDisabled	= 1,
		kModeHidden				= 2
	};

	void readControl(Common::SeekableReadStream &stream);
	void show();
	void hide();
	void fireEvent(int32 eventType);
	void redraw();
	Control *getControlAt(const Common::Point &pos) const;
	void showItem(int16 itemID, bool show);

	Manager *_manager;
	byte _kind;

	uint32 _backgroundColor = 0;
	Static *_background = nullptr;
	Common::Array<Control *> _controls;
	Common::Array<Control *> _arrangeOrder;
	Common::Array<RadioButton *> _radioButtons;
	Common::Array<EditBox *> _editBoxes;
	int _nextID = 1;

	Arrangement _arrangement = kArrangeNone;
	Common::Point _spacing;
	Common::Rect _margins;

	// Scroll panels move their controls up and down, following a scroll bar
	Common::String _scrollBarName;
	int32 _scrollStep = 1;
	int32 _scrollOffset = 0;

	int _showCount = 0;
	bool _enabled = true;
	uint32 _lastUpdateTime = 0;
	bool _isDirty = true;
	Control *_capture = nullptr;
};

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_PANEL_H
