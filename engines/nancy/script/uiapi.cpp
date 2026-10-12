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

#include "common/memstream.h"

#include "engines/nancy/enginedata.h"
#include "engines/nancy/nancy.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/ndui/common.h"
#include "engines/nancy/ui/ndui/control.h"
#include "engines/nancy/ui/ndui/manager.h"
#include "engines/nancy/ui/ndui/panel.h"

namespace Nancy {
namespace Script {

// The UI of Nancy19 and up is made of the same panels and controls as the one of
// Nancy16, but scripts describe them, with tables, where NDUI chunks used to.
// A description gets written out the way a chunk has it, and read by the panel
// or control it is for.
//
// TODO: The textShadow and captureClicks fields, the ways of arranging controls
// other than left to right and right to left, and the events the controls don't
// send yet: enable, disable, mouseEnter and mouseLeave.

static const char *const kPanelType = "ND.Panel";

static const uint32 kDefaultColor = 0xFFFFFFFF;
static const uint32 kDefaultDisabledColor = 0xC8808080;
static const char *const kDefaultFontName = "UIFont";

struct NamedValue {
	const char *name;
	int32 value;
};

// The names scripts know events by
static const NamedValue kEventNames[] = {
	{ "buttonClicked", UI::NDUI::kEventClicked },
	{ "radioButtonChanged", UI::NDUI::kEventRadioChanged },
	{ "checkboxChecked", UI::NDUI::kEventChecked },
	{ "checkboxUnchecked", UI::NDUI::kEventUnchecked },
	{ "sliderValueChanged", UI::NDUI::kEventSliderChanged },
	{ "sliderReleased", UI::NDUI::kEventSliderReleased },
	{ "editboxString", UI::NDUI::kEventTextEntered },
	{ "editboxChange", UI::NDUI::kEventTextChanged },
	{ "listboxItemDblClick", UI::NDUI::kEventListDoubleClicked },
	{ "listboxSelection", UI::NDUI::kEventListSelected },
	{ "scrollBarValueChanged", UI::NDUI::kEventScrolled },
	{ "show", UI::NDUI::kEventShow },
	{ "hide", UI::NDUI::kEventHide },
	{ "load", UI::NDUI::kEventUpdate }
};

static const NamedValue kInitialStates[] = {
	{ "normal", UI::NDUI::kStartVisible },
	{ "disabled", UI::NDUI::kStartDisabled },
	{ "hidden", UI::NDUI::kStartHidden }
};

// The ways a panel can arrange its controls
static const NamedValue kLayoutOrders[] = {
	{ "manual", 0 },
	{ "leftToRight", 1 },
	{ "rightToLeft", 2 },
	{ "topToBottom", 3 },
	{ "bottomToTop", 4 },
	{ "centered", 5 }
};

// Where text goes in its control, as the NDUI chunks have it
enum HorizontalAlignment {
	kAlignLeft		= 0,
	kAlignRight		= 1,
	kAlignCenter	= 2
};

enum VerticalAlignment {
	kAlignVCenter	= 2,
	kAlignTop		= 3,
	kAlignBottom	= 4
};

static const NamedValue kHorizontalAlignments[] = {
	{ "left", kAlignLeft },
	{ "right", kAlignRight },
	{ "center", kAlignCenter }
};

static const NamedValue kVerticalAlignments[] = {
	{ "center", kAlignVCenter },
	{ "top", kAlignTop },
	{ "bottom", kAlignBottom }
};

// A function of a script, kept for as long as the UI can call it
class ScriptFunction {
public:
	// Takes the function at the top of the stack
	ScriptFunction(lua_State *L) : _lua(L), _reference(luaL_ref(L, LUA_REGISTRYINDEX)) {}
	~ScriptFunction() { luaL_unref(_lua, LUA_REGISTRYINDEX, _reference); }

	lua_State *push() const {
		lua_rawgeti(_lua, LUA_REGISTRYINDEX, _reference);
		return _lua;
	}

private:
	lua_State *_lua;
	int _reference;
};

// onEvent(eventName, value, text)
class ScriptEventHandler : public UI::NDUI::EventHandler {
public:
	ScriptEventHandler(lua_State *L) : _function(L) {}

	void onEvent(int32 eventType, bool isOwnEvent, double value, const Common::String &text) override;

private:
	ScriptFunction _function;
};

void ScriptEventHandler::onEvent(int32 eventType, bool isOwnEvent, double value, const Common::String &text) {
	// What a control reports about itself has a name of its own
	const char *eventName = isOwnEvent ? "default" : nullptr;
	for (uint i = 0; !eventName && i < ARRAYSIZE(kEventNames); ++i) {
		if (kEventNames[i].value == eventType) {
			eventName = kEventNames[i].name;
		}
	}

	if (!eventName) {
		return;
	}

	// The function can take its control down, and this handler with it
	lua_State *L = _function.push();
	lua_pushstring(L, eventName);
	lua_pushnumber(L, value);
	lua_pushstring(L, text.c_str());
	LuaState::call(L, 3, 0, "Error during UI event");
}

// createObject(name, templateName), which returns whether it made the control
class ScriptObjectFactory : public UI::NDUI::ObjectFactory {
public:
	ScriptObjectFactory(lua_State *L) : _function(L) {}

	bool createObject(const Common::String &name, const Common::String &templateName) override;

private:
	ScriptFunction _function;
};

bool ScriptObjectFactory::createObject(const Common::String &name, const Common::String &templateName) {
	lua_State *L = _function.push();
	lua_pushstring(L, name.c_str());
	lua_pushstring(L, templateName.c_str());
	if (!LuaState::call(L, 2, 1, "Error during UI object creation")) {
		return false;
	}

	bool created = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return created;
}

static UI::NDUI::Panel *checkPanel(lua_State *L, int index) {
	return *(UI::NDUI::Panel **)checkUserData(L, index, kPanelType);
}

// Pushes a field that is a table, or an empty table when there is no such field
static void pushTableField(lua_State *L, int tableIndex, const char *name) {
	if (!pushField(L, tableIndex, name)) {
		lua_newtable(L);
	} else if (!lua_istable(L, -1)) {
		luaL_error(L, "field %s must be a table", name);
	}
}

static void readRectField(lua_State *L, int tableIndex, const char *name, ScriptRect &rect) {
	if (pushField(L, tableIndex, name)) {
		rect = checkRect(L, -1);
		lua_pop(L, 1);
	}
}

static void readPointField(lua_State *L, int tableIndex, const char *name, ScriptPoint &point) {
	if (pushField(L, tableIndex, name)) {
		if (!readPoint(L, -1, point)) {
			luaL_error(L, "field %s must be a point", name);
		}

		lua_pop(L, 1);
	}
}

static bool readColorField(lua_State *L, int tableIndex, const char *name, uint32 &color) {
	if (!pushField(L, tableIndex, name)) {
		return false;
	}

	color = checkColor(L, -1);
	lua_pop(L, 1);
	return true;
}

// Reads a field that holds one of the given names; it keeps its value when it is missing
static void readNamedField(lua_State *L, int tableIndex, const char *name, const NamedValue *values, uint numValues, int32 &value) {
	Common::String valueName;
	if (!readField(L, tableIndex, name, valueName)) {
		return;
	}

	for (uint i = 0; i < numValues; ++i) {
		if (valueName.equalsIgnoreCase(values[i].name)) {
			value = values[i].value;
			return;
		}
	}

	luaL_error(L, "field %s can't be \"%s\"", name, valueName.c_str());
}

static void writeName(Common::WriteStream &out, const Common::String &name) {
	char buf[33] = {};
	Common::strlcpy(buf, name.c_str(), sizeof(buf));
	out.write(buf, sizeof(buf));
}

static void writeRect(Common::WriteStream &out, const ScriptRect &rect) {
	out.writeSint32LE(rect.left);
	out.writeSint32LE(rect.top);
	out.writeSint32LE(rect.right);
	out.writeSint32LE(rect.bottom);
}

// { ovl = <image name>, source = <rect> }
static void writeOverlay(lua_State *L, int tableIndex, const char *name, Common::WriteStream &out) {
	Common::String textureName;
	ScriptRect source;

	pushTableField(L, tableIndex, name);
	readField(L, -1, "ovl", textureName);
	readRectField(L, -1, "source", source);
	lua_pop(L, 1);

	writeName(out, textureName);
	writeRect(out, source);
}

// The colors something has in each state of its control:
// { normal = <color>, disabled = <color>, mouseOver = <color>, pressed = <color> }.
// A font takes its normal color in the other states that aren't given one.
static void writeStateColors(lua_State *L, int tableIndex, const char *name, bool isFont, Common::WriteStream &out) {
	uint32 normal = kDefaultColor;
	uint32 disabled = kDefaultDisabledColor;
	uint32 mouseOver = kDefaultColor;
	uint32 pressed = kDefaultColor;

	pushTableField(L, tableIndex, name);
	if (readColorField(L, -1, "normal", normal) && isFont) {
		mouseOver = normal;
		pressed = normal;
	}

	readColorField(L, -1, "disabled", disabled);
	readColorField(L, -1, "mouseOver", mouseOver);
	readColorField(L, -1, "pressed", pressed);
	lua_pop(L, 1);

	out.writeUint32BE(normal);
	out.writeUint32BE(disabled);
	out.writeUint32BE(mouseOver);
	out.writeUint32BE(pressed);
}

// The fields a table has for one of the things a control draws:
// overlay, renderStates, font = { name, bold, horzAlign, vertAlign, wrap }, fontStates
static void writeElement(lua_State *L, int tableIndex, HorizontalAlignment defaultAlignment, Common::WriteStream &out) {
	tableIndex = absoluteIndex(L, tableIndex);

	writeOverlay(L, tableIndex, "overlay", out);
	writeStateColors(L, tableIndex, "renderStates", false, out);

	Common::String fontName = kDefaultFontName;
	bool isBold = false;
	bool wraps = false;
	int32 horizontalAlignment = defaultAlignment;
	int32 verticalAlignment = kAlignTop;

	pushTableField(L, tableIndex, "font");
	readField(L, -1, "name", fontName);
	readField(L, -1, "bold", isBold);
	readNamedField(L, -1, "horzAlign", kHorizontalAlignments, ARRAYSIZE(kHorizontalAlignments), horizontalAlignment);
	readNamedField(L, -1, "vertAlign", kVerticalAlignments, ARRAYSIZE(kVerticalAlignments), verticalAlignment);
	readField(L, -1, "wrap", wraps);
	lua_pop(L, 1);

	writeName(out, fontName);
	out.writeByte(isBold);
	out.writeSint32LE(horizontalAlignment);
	out.writeSint32LE(verticalAlignment);
	out.writeByte(wraps);

	writeStateColors(L, tableIndex, "fontStates", true, out);
	out.writeByte(0);
}

void writeTextElement(lua_State *L, int tableIndex, const Common::String &fontName, bool isBold, uint32 color, Common::WriteStream &out) {
	tableIndex = absoluteIndex(L, tableIndex);

	bool wraps = false;
	int32 horizontalAlignment = kAlignCenter;
	int32 verticalAlignment = kAlignTop;

	pushTableField(L, tableIndex, "font");
	readNamedField(L, -1, "horzAlign", kHorizontalAlignments, ARRAYSIZE(kHorizontalAlignments), horizontalAlignment);
	readNamedField(L, -1, "vertAlign", kVerticalAlignments, ARRAYSIZE(kVerticalAlignments), verticalAlignment);
	readField(L, -1, "wrap", wraps);
	lua_pop(L, 1);

	// Nothing but the text gets drawn
	writeName(out, Common::String());
	writeRect(out, ScriptRect());
	for (uint i = 0; i < 4; ++i) {
		out.writeUint32BE(kDefaultColor);
	}

	writeName(out, fontName);
	out.writeByte(isBold);
	out.writeSint32LE(horizontalAlignment);
	out.writeSint32LE(verticalAlignment);
	out.writeByte(wraps);

	for (uint i = 0; i < 4; ++i) {
		out.writeUint32BE(color);
	}

	out.writeByte(0);
}

// The fields every control has: initialState, name, onScreen, text, tooltipText,
// and the ones of what it draws
static void writeControl(lua_State *L, int tableIndex, HorizontalAlignment defaultAlignment, Common::WriteStream &out) {
	tableIndex = absoluteIndex(L, tableIndex);
	luaL_checktype(L, tableIndex, LUA_TTABLE);

	int32 initialState = UI::NDUI::kStartVisible;
	Common::String name;
	Common::String text;
	Common::String tooltip;
	ScriptRect bounds;

	readNamedField(L, tableIndex, "initialState", kInitialStates, ARRAYSIZE(kInitialStates), initialState);
	if (!readField(L, tableIndex, "name", name)) {
		luaL_error(L, "A control must have a name field");
	}

	readRectField(L, tableIndex, "onScreen", bounds);
	readField(L, tableIndex, "text", text);
	readField(L, tableIndex, "tooltipText", tooltip);

	writeName(out, Common::String());	// version
	out.writeSint32LE(initialState);
	writeName(out, name);
	writeRect(out, bounds);
	writeName(out, text);
	writeName(out, tooltip);
	out.writeSint16LE(0);				// the events go to the script
	writeElement(L, tableIndex, defaultAlignment, out);
}

// upArrow, downArrow and handle are described like the scroll bar itself
static void writeScrollBar(lua_State *L, int tableIndex, Common::WriteStream &out) {
	static const char *const kPartNames[] = { "upArrow", "downArrow", "handle" };

	tableIndex = absoluteIndex(L, tableIndex);
	writeControl(L, tableIndex, kAlignCenter, out);

	for (const char *partName : kPartNames) {
		pushTableField(L, tableIndex, partName);
		writeElement(L, -1, kAlignCenter, out);
		lua_pop(L, 1);
	}

	int pixelsToScroll = 0;
	readField(L, tableIndex, "pixelsToScroll", pixelsToScroll);
	out.writeSint32LE(pixelsToScroll);
}

// Gives a control the function of its description that handles its events
static void setEventHandler(lua_State *L, int tableIndex, UI::NDUI::Control *control) {
	if (!pushField(L, tableIndex, "onEvent")) {
		return;
	}

	if (!lua_isfunction(L, -1)) {
		luaL_error(L, "onEvent field must be a function");
	}

	control->setEventHandler(new ScriptEventHandler(L));
}

// Makes the control described by the table at index 2, whose data has been written out
static UI::NDUI::Control *addControl(lua_State *L, int32 type, Common::MemoryWriteStreamDynamic &data) {
	UI::NDUI::Panel *panel = checkPanel(L, 1);
	if (!panel->isPlaced()) {
		luaL_error(L, "A panel needs a background before it can have controls");
	}

	Common::MemoryReadStream stream(data.getData(), data.size());
	UI::NDUI::Control *control = panel->addNewControl(type, stream);
	setEventHandler(L, 2, control);

	panel->growToControls();
	panel->arrangeControls();
	return control;
}

// The text kept under a key, or the key itself when there is none
static Common::String resolveText(const Common::String &key) {
	Common::String text = key;
	findText(key, text);
	return text;
}

// panel:AddBg{...}: the background names the panel and says where it goes
static int panelAddBg(lua_State *L) {
	UI::NDUI::Panel *panel = checkPanel(L, 1);
	if (panel->isPlaced()) {
		return luaL_error(L, "The panel already has a background");
	}

	// The background of a panel that scrolls only gives it its name and its place.
	// The task list names the picture of the panel around it there, which
	// isn't meant to show a second time.
	luaL_checktype(L, 2, LUA_TTABLE);
	if (panel->isScrolling()) {
		lua_pushnil(L);
		lua_setfield(L, 2, "overlay");
	}

	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeControl(L, 2, kAlignCenter, data);

	Common::MemoryReadStream stream(data.getData(), data.size());
	panel->readBackground(stream);
	setEventHandler(L, 2, panel->getBackground());

	panel->place();
	panel->registerGraphics();
	return 0;
}

static int panelAddStatic(lua_State *L) {
	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeControl(L, 2, kAlignCenter, data);
	addControl(L, UI::NDUI::kControlStatic, data);
	return 0;
}

// Adds foregroundOverlay, pressedOffset
static int panelAddButton(lua_State *L) {
	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeControl(L, 2, kAlignCenter, data);
	writeOverlay(L, 2, "foregroundOverlay", data);

	ScriptPoint pressedOffset;
	readPointField(L, 2, "pressedOffset", pressedOffset);
	data.writeSint32LE(pressedOffset.x);
	data.writeSint32LE(pressedOffset.y);

	addControl(L, UI::NDUI::kControlButton, data);
	return 0;
}

// Adds checkOverlay, initiallyChecked
static int panelAddCheckBox(lua_State *L) {
	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeControl(L, 2, kAlignCenter, data);
	writeOverlay(L, 2, "checkOverlay", data);

	bool isChecked = false;
	readField(L, 2, "initiallyChecked", isChecked);
	data.writeByte(isChecked);

	addControl(L, UI::NDUI::kControlCheckBox, data);
	return 0;
}

// Adds radioOverlay, groupName
static int panelAddRadioButton(lua_State *L) {
	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeControl(L, 2, kAlignCenter, data);
	writeOverlay(L, 2, "radioOverlay", data);

	Common::String groupName;
	if (!readField(L, 2, "groupName", groupName)) {
		return luaL_error(L, "Radio button must have a groupName field");
	}

	writeName(data, groupName);

	addControl(L, UI::NDUI::kControlRadioButton, data);
	return 0;
}

// Adds upArrow, downArrow, handle, pixelsToScroll
static int panelAddScrollBar(lua_State *L) {
	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeScrollBar(L, 2, data);
	addControl(L, UI::NDUI::kControlScrollBar, data);
	return 0;
}

// Adds handleOverlay, fillOverlay, min, max, initialValue
static int panelAddSlider(lua_State *L) {
	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeControl(L, 2, kAlignCenter, data);
	writeOverlay(L, 2, "handleOverlay", data);
	writeOverlay(L, 2, "fillOverlay", data);

	int minValue = 0;
	int maxValue = 0;
	int initialValue = 0;
	readField(L, 2, "min", minValue);
	readField(L, 2, "max", maxValue);
	readField(L, 2, "initialValue", initialValue);
	data.writeSint32LE(minValue);
	data.writeSint32LE(maxValue);
	data.writeSint32LE(initialValue);

	addControl(L, UI::NDUI::kControlSlider, data);
	return 0;
}

// Adds selected (how the selected line is drawn), scrollBar (a scroll bar of
// its own), border, margin
static int panelAddListBox(lua_State *L) {
	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeControl(L, 2, kAlignCenter, data);

	pushTableField(L, 2, "selected");
	writeElement(L, -1, kAlignCenter, data);
	lua_pop(L, 1);

	data.writeSint32LE(UI::NDUI::kControlScrollBar);
	pushTableField(L, 2, "scrollBar");
	writeScrollBar(L, -1, data);
	lua_pop(L, 1);

	int border = 0;
	int margin = 0;
	readField(L, 2, "border", border);
	readField(L, 2, "margin", margin);
	data.writeSint32LE(border);
	data.writeSint32LE(margin);

	data.writeSint16LE(0);				// the lines get added as the game runs
	writeName(data, Common::String());

	addControl(L, UI::NDUI::kControlListBox, data);
	return 0;
}

// Adds selectedTextColor, selectedTextBackColor, caretColor, spacing.
// The text starts at the left unless the font says otherwise.
static int panelAddEditBox(lua_State *L) {
	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeControl(L, 2, kAlignLeft, data);

	uint32 selectedTextColor = 0;
	uint32 selectionColor = 0;
	uint32 caretColor = 0;
	int spacing = 0;
	readColorField(L, 2, "selectedTextColor", selectedTextColor);
	readColorField(L, 2, "selectedTextBackColor", selectionColor);
	readColorField(L, 2, "caretColor", caretColor);
	readField(L, 2, "spacing", spacing);
	data.writeUint32BE(selectedTextColor);
	data.writeUint32BE(selectionColor);
	data.writeUint32BE(caretColor);
	data.writeSint32LE(spacing);

	addControl(L, UI::NDUI::kControlEditBox, data);
	return 0;
}

// Adds preText, postText, preDecimal, postDecimal: the texts around the number,
// and the digits it has
static int panelAddVarStatic(lua_State *L) {
	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeControl(L, 2, kAlignCenter, data);

	int preDecimal = 0;
	int postDecimal = 0;
	Common::String preText;
	Common::String postText;
	readField(L, 2, "preDecimal", preDecimal);
	readField(L, 2, "postDecimal", postDecimal);
	readField(L, 2, "preText", preText);
	readField(L, 2, "postText", postText);
	data.writeSint32LE(preDecimal);
	data.writeSint32LE(postDecimal);

	UI::NDUI::VarStatic *control = (UI::NDUI::VarStatic *)addControl(L, UI::NDUI::kControlVarStatic, data);
	control->setAffixes(resolveText(preText), resolveText(postText));
	return 0;
}

// Makes a panel out of: initialState, layout = { primaryOrder, secondaryOrder,
// offset, border }, color, save, createObject, and for the ones that scroll,
// scrollBarName and scrollPixels
static int newPanel(lua_State *L, byte kind) {
	luaL_checktype(L, 2, LUA_TTABLE);

	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	if (!ui) {
		return luaL_error(L, "There is no UI to add a panel to");
	}

	int32 initialState = UI::NDUI::kStartVisible;
	int32 primaryOrder = 0;
	int32 secondaryOrder = 0;
	ScriptPoint spacing;
	ScriptRect margins;
	uint32 color = 0;

	readNamedField(L, 2, "initialState", kInitialStates, ARRAYSIZE(kInitialStates), initialState);

	pushTableField(L, 2, "layout");
	readNamedField(L, -1, "primaryOrder", kLayoutOrders, ARRAYSIZE(kLayoutOrders), primaryOrder);
	readNamedField(L, -1, "secondaryOrder", kLayoutOrders, ARRAYSIZE(kLayoutOrders), secondaryOrder);
	readPointField(L, -1, "offset", spacing);
	readRectField(L, -1, "border", margins);
	lua_pop(L, 1);

	readColorField(L, 2, "color", color);

	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	writeName(data, Common::String());	// version
	data.writeSint32LE(initialState);
	data.writeSint32LE(primaryOrder);
	data.writeSint32LE(secondaryOrder);
	data.writeSint32LE(spacing.x);
	data.writeSint32LE(spacing.y);
	writeRect(data, margins);
	data.writeUint32BE(color);
	data.writeUint16LE(0);

	if (kind == UI::NDUI::kObjectScrollPanel) {
		Common::String scrollBarName;
		int scrollPixels = 0;
		if (!readField(L, 2, "scrollBarName", scrollBarName) || !readField(L, 2, "scrollPixels", scrollPixels)) {
			return luaL_error(L, "A scroll panel must have the scrollBarName and scrollPixels fields");
		}

		writeName(data, scrollBarName);
		data.writeSint32LE(scrollPixels);
	}

	UI::NDUI::Panel *panel = ui->addPanel(kind);
	Common::MemoryReadStream stream(data.getData(), data.size());
	panel->readHeader(stream);
	if (kind == UI::NDUI::kObjectScrollPanel) {
		panel->readScrolling(stream);
	}

	panel->addBuiltInTooltip();

	bool savesState = true;
	if (readField(L, 2, "save", savesState)) {
		panel->setSavesState(savesState);
	}

	if (pushField(L, 2, "createObject")) {
		if (!lua_isfunction(L, -1)) {
			return luaL_error(L, "field 'createObject' must be a function");
		}

		panel->setObjectFactory(new ScriptObjectFactory(L));
	}

	// The panels have to go before the scripts they call into do
	SceneContext *context = LuaState::getContext(L);
	if (context) {
		context->setMakesUI();
	}

	*(UI::NDUI::Panel **)pushUserData(L, sizeof(UI::NDUI::Panel *), kPanelType) = panel;
	return 1;
}

// panel.createObject = function: the one field that can be set once the panel is there
static int panelNewIndex(lua_State *L) {
	UI::NDUI::Panel *panel = checkPanel(L, 1);
	const char *key = luaL_checkstring(L, 2);

	if (strcmp(key, "createObject") != 0) {
		return luaL_error(L, "A panel has no field '%s' to set", key);
	}

	if (!lua_isfunction(L, 3)) {
		return luaL_error(L, "field 'createObject' must be a function");
	}

	lua_pushvalue(L, 3);
	panel->setObjectFactory(new ScriptObjectFactory(L));
	return 0;
}

// Panel:Regular{...}
static int panelRegular(lua_State *L) {
	return newPanel(L, UI::NDUI::kObjectPanel);
}

// Panel:Scroll{...}
static int panelScroll(lua_State *L) {
	return newPanel(L, UI::NDUI::kObjectScrollPanel);
}

void registerUIAPI(lua_State *L) {
	static const luaL_Reg kPanelFunctions[] = {
		{ "__index", indexFunctions },
		{ "__newindex", panelNewIndex },
		{ "AddBg", panelAddBg },
		{ "AddStatic", panelAddStatic },
		{ "AddButton", panelAddButton },
		{ "AddCheckBox", panelAddCheckBox },
		{ "AddRadioButton", panelAddRadioButton },
		{ "AddScrollBar", panelAddScrollBar },
		{ "AddSlider", panelAddSlider },
		{ "AddListBox", panelAddListBox },
		{ "AddEditBox", panelAddEditBox },
		{ "AddVarStatic", panelAddVarStatic },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kPanelHelperFunctions[] = {
		{ "__index", indexFunctions },
		{ "Regular", panelRegular },
		{ "Scroll", panelScroll },
		{ nullptr, nullptr }
	};

	registerType(L, kPanelType, kPanelFunctions);
	registerGlobal(L, "Panel", "ND.PanelHelper", kPanelHelperFunctions);
}

} // End of namespace Script
} // End of namespace Nancy
