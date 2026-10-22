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

#ifndef NANCY_UI_NDUI_COMMON_H
#define NANCY_UI_NDUI_COMMON_H

#include "common/rect.h"
#include "common/str.h"
#include "common/stream.h"

#include "engines/nancy/puzzledata.h"

namespace Nancy {
namespace UI {
namespace NDUI {

// Nancy16 replaced the per-widget UI chunks with a small widget toolkit. The UI is a
// set of panels, each holding a list of controls, described by NDUI chunks spread
// over one IFF per widget. Controls react to the player by firing events, each of
// which performs an action on a named panel or control, or hands it to the engine.
// The original's toolkit, Ndui.dll, is built on the DXUT widgets of the DirectX SDK
// (CDXUTDialog and its controls), which the panels and controls here follow.

enum ControlType {
	kControlButton		= 1,
	kControlStatic		= 2,
	kControlSlider		= 3,
	kControlRadioGroup	= 5,
	kControlRadioButton	= 6,
	kControlEditBox		= 7,
	kControlListBox		= 8,
	kControlScrollBar	= 9,
	kControlCheckBox	= 10,
	kControlInvButton	= 12,
	kControlVarStatic	= 14,
	kControlTooltip		= 15	// Nancy17
};

// The kinds of objects an NDUI chunk describes
enum ObjectKind {
	kObjectPanel		= 0,
	kObjectScrollPanel	= 11,
	kObjectInvPanel		= 13,
	kObjectTemplate		= 40	// a control the engine creates copies of at runtime
};

// The states a control can be drawn in; every element carries a color for each
enum ControlState {
	kStateNormal		= 0,
	kStateDisabled		= 1,
	kStateHidden		= 2,
	kStateFocus			= 3,
	kStateMouseOver		= 4,
	kStatePressed		= 5,
	kNumStates			= 6
};

// What a panel or control starts as
enum InitialState {
	kStartVisible	= 0,
	kStartDisabled	= 1,
	kStartHidden	= 2
};

enum EventType {
	kEventOwn			= -1,	// whatever the control itself reports: a click, a new value...
	kEventClicked		= 1,
	kEventRadioChanged	= 3,
	kEventChecked		= 4,
	kEventUnchecked		= 5,
	kEventSliderChanged	= 6,
	kEventSliderReleased	= 7,
	kEventTextEntered	= 8,	// the player pressed Enter in an edit box
	kEventTextChanged	= 9,
	kEventListDoubleClicked	= 10,
	kEventListSelected	= 11,
	kEventScrolled		= 15,
	kEventShow			= 16,	// the panel holding the control was shown
	kEventHide			= 17,	// the panel holding the control was hidden
	kEventUpdate		= 18,	// sent every frame while the panel is visible; "load" to Nancy19 scripts
	kEventEnable		= 19,	// Nancy20: the panel holding the control was enabled...
	kEventDisable		= 20,	// ...or disabled
	kEventActionRecord	= 100	// sent by a scene's action record
};

enum ActionType {
	kActionNone				= 0,	// only means something to the scripts of Nancy19
	kActionShow				= 1,
	kActionHide				= 2,
	kActionToggle			= 3,
	kActionSetRange			= 5,	// scroll bars
	kActionExpand			= 5,	// Nancy20 panels, which can be collapsed...
	kActionCollapse			= 6,
	kActionToggleCollapsed	= 7,	// ...to their caption, which script panels don't have
	kActionEnable			= 8,
	kActionDisable			= 9,
	kActionToggleEnabled	= 10,
	kActionAddText			= 11,	// also adds a list box item
	kActionSetText			= 12,	// also empties a list box
	kActionSetValue			= 13,
	kActionSetTexture		= 14,
	kActionSetImage			= 15,	// Nancy21: a texture and a source rect, in one
	kActionQuery			= 30,	// asks the engine to send its value back with kActionSetValue
	kActionShowItem			= 40,	// inventory panels
	kActionHideItem			= 41
};

struct Action {
	void read(Common::SeekableReadStream &stream);

	int32 eventType = 0;		// only for actions bound to a control's event
	int32 type = 0;
	Common::String target;		// a panel, a control, or an "Engine_" handler
	bool hasValue = false;
	double value = 0;
	Common::String text;
	Common::String sender;		// the control that fired the event
};

// Nancy19 describes its UI with scripts in place of NDUI chunks. A control hands
// its events to a function of the script that made it, which stands in for the
// actions bound to them...
class EventHandler {
public:
	virtual ~EventHandler() {}
	virtual void onEvent(int32 eventType, bool isOwnEvent, double value, const Common::String &text) = 0;
};

// ...and a function of a panel's script stands in for the templates of the controls
// that get created while the game runs
class ObjectFactory {
public:
	virtual ~ObjectFactory() {}
	virtual bool createObject(const Common::String &name, const Common::String &templateName) = 0;
};

// Nancy21 passes rectangles around as the value of an action, with the four
// sides packed into the bytes of the number. A side that doesn't fit gives -1.
double encodeRect(int32 left, int32 top, int32 right, int32 bottom);
Common::Rect decodeRect(double value);

// Adds the actions that bring a panel or control from the state it starts in to the given one
void addStateChanges(Common::Array<UIStateData::Change> &changes, const Common::String &name, int32 initialState, bool visible, bool enabled);

// How quickly colors move toward the ones of a new state; a lower rate is faster
static const float kStaticBlendRate = 0.7f;
static const float kControlBlendRate = 0.8f;

// Long enough for any color to reach the one it is moving toward
static const float kFinishBlendingTime = 1000.0f;

// The lines a notch of the mouse wheel scrolls
static const int kWheelLines = 3;

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_COMMON_H
