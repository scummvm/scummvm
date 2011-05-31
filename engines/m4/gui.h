/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef M4_GUI_H
#define M4_GUI_H

#include "common/list.h"
#include "common/rect.h"

#include "m4/viewmgr.h"
#include "m4/events.h"
#include "m4/globals.h"
#include "m4/graphics.h"
#include "m4/saveload.h"
#include "m4/sprite.h"
#include "m4/assets.h"

namespace M4 {

class GUIObject;
class MenuObject;
class GUIObjectButton;
class SpriteAsset;
class View;

enum MenuType {GAME_MENU, OPTIONS_MENU, SAVE_MENU, LOAD_MENU, MAIN_MENU};

enum {
	BTNID_QUIT = 1,
	BTNID_OPTIONS = 2,
	BTNID_RESUME = 3,
	BTNID_SAVE = 4,
	BTNID_LOAD = 5,
	BTNID_MAIN = 6,

	OPTIONID_DONE = 1,
	OPTIONID_CANCEL = 2,
	OPTIONID_HSLIDER_DIGI = 3,
	OPTIONID_HSLIDER_MIDI = 4,

	SAVELOADID_SAVE = 1,
	SAVELOADID_CANCELSAVE = 2,
	SAVELOADID_LOAD = 3,
	SAVELOADID_CANCELLOAD = 4,

	SLTAG_SAVELOAD = 100,
	SLTAG_SAVELOAD_LABEL = 101,
	SLTAG_CANCEL = 102,
	SLTAG_VSLIDER = 103,
	SLTAG_THUMBNAIL = 104,

	SLTAG_SLOTS_START = 1001,
	SLTAG_TEXTFIELD = 2000
};

enum ObjectType {
	OBJTYPE_BUTTON,

	OBJTYPE_SLIDER,
	OBJTYPE_OM_SWITCH_ON,
	OBJTYPE_OM_SWITCH_OFF,
	OBJTYPE_OM_DONE,
	OBJTYPE_OM_CANCEL,

	OBJTYPE_SL_SAVE,
	OBJTYPE_SL_LOAD,
	OBJTYPE_SL_CANCEL,
	OBJTYPE_SL_TEXT
};

enum GameMenuSpriteType {
	GM_DIALOG_BOX,

	GM_BUTTON_GREYED,
	GM_BUTTON_NORMAL,
	GM_BUTTON_MOUSEOVER,
	GM_BUTTON_PRESSED
};

enum OptionMenuSpriteType {
	OM_DIALOG_BOX,

	OM_SLIDER_BTN_NORMAL,
	OM_SLIDER_BTN_MOUSEOVER,
	OM_SLIDER_BTN_PRESSED,
	OM_SLIDER_BAR,
	OM_DONE_BTN_GREYED,
	OM_DONE_BTN_NORMAL,
	OM_DONE_BTN_MOUSEOVER,
	OM_DONE_BTN_PRESSED,
	OM_CANCEL_BTN_NORMAL,
	OM_CANCEL_BTN_MOUSEOVER,
	OM_CANCEL_BTN_PRESSED
};


enum SaveLoadSpriteType {
	SL_DIALOG_BOX,
	SL_EMPTY_THUMBNAIL,

	SL_SAVE_BTN_GREYED,
	SL_SAVE_BTN_NORMAL,
	SL_SAVE_BTN_MOUSEOVER,
	SL_SAVE_BTN_PRESSED,

	SL_LOAD_BTN_GREYED,
	SL_LOAD_BTN_NORMAL,
	SL_LOAD_BTN_MOUSEOVER,
	SL_LOAD_BTN_PRESSED,

	SL_CANCEL_BTN_NORMAL,
	SL_CANCEL_BTN_MOUSEOVER,
	SL_CANCEL_BTN_PRESSED,

	SL_UP_BTN_GREYED,
	SL_UP_BTN_NORMAL,
	SL_UP_BTN_MOUSEOVER,
	SL_UP_BTN_PRESSED,

	SL_DOWN_BTN_GREYED,
	SL_DOWN_BTN_NORMAL,
	SL_DOWN_BTN_MOUSEOVER,
	SL_DOWN_BTN_PRESSED,

	SL_SAVE_LABEL,
	SL_LOAD_LABEL,

	SL_SLIDER_BTN_NORMAL,
	SL_SLIDER_BTN_MOUSEOVER,
	SL_SLIDER_BTN_PRESSED,

	SL_LINE_NORMAL,
	SL_LINE_MOUSEOVER,
	SL_LINE_PRESSED,

	SL_SCROLLBAR
};

enum TextColors {
	TEXT_COLOR_NORMAL = 1,
	TEXT_COLOR_GREYED = 1,
	TEXT_COLOR_MOUSEOVER = 2,
	TEXT_COLOR_PRESSED = 2,

	TEXT_COLOR_GREYED_HILIGHT    = 236,
	TEXT_COLOR_GREYED_FOREGROUND = 131,
	TEXT_COLOR_GREYED_SHADOW     = 186,

	TEXT_COLOR_NORMAL_HILIGHT    = 129,
	TEXT_COLOR_NORMAL_FOREGROUND = 130,
	TEXT_COLOR_NORMAL_SHADOW     = 236,

	TEXT_COLOR_MOUSEOVER_HILIGHT    = 129,
	TEXT_COLOR_MOUSEOVER_FOREGROUND = 130,
	TEXT_COLOR_MOUSEOVER_SHADOW     = 236,

	TEXT_COLOR_PRESSED_HILIGHT    = 236,
	TEXT_COLOR_PRESSED_FOREGROUND = 130,
	TEXT_COLOR_PRESSED_SHADOW     = 129
};

#define MENU_SS_FIELD_NORMAL 5
#define MENU_SS_FIELD_MOUSEOVER 6

#define MENU_SS_H_SLIDER_NORMAL 5
#define MENU_SS_H_SLIDER_MOUSE_OVER 6
#define MENU_SS_H_SLIDER_PRESSED 7

#define MENU_SS_SWITCH_ON_NORMAL 8
#define MENU_SS_SWITCH_ON_MOUSEOVER 9
#define MENU_SS_SWITCH_ON_PRESSED 13

#define MENU_SS_SWITCH_OFF_PRESSED 10
#define MENU_SS_SWITCH_OFF_NORMAL 11
#define MENU_SS_SWITCH_OFF_MOUSEOVER 12

#define MENU_GUI "gui menu"
#define MENU_GAME "gamemenu"
#define MENU_OPTIONS "opmenu"
#define MENU_ERROR "errmenu"
#define MENU_SAVELOAD "slmenu"
#define MENU_BURGMAIN "903menu"
#define MENU_BURGDEMOMAIN "901menu"

#define SL_NUM_VISIBLE_SLOTS 8
#define SL_THUMBNAIL_WIDTH  215
#define SL_THUMBNAIL_HEIGHT 162

enum MenuObjectState {OS_GREYED = 0, OS_NORMAL = 1, OS_MOUSEOVER = 2, OS_PRESSED = 3};

class DialogView : public View {
public:
	DialogView(MadsM4Engine *Vm, const Common::Rect &viewBounds, bool transparent = false):
		View(Vm, viewBounds, transparent) {}
	DialogView(MadsM4Engine *Vm, int x = 0, int y = 0, bool transparent = false):
		View(Vm, x, y, transparent) {}

	MadsM4Engine *vm() { return _vm; }
	virtual SpriteAsset *sprites() = 0;
	virtual MenuType getMenuType() = 0;
	virtual MenuObject *getItem(int objectId) { return NULL; }
	virtual void refresh(const Common::Rect &areaRect) {}
	virtual void close();

	int _topSaveSlotIndex, _selectedSlot;
	int _highlightedSlot;
	bool _deleteSaveDesc;
	M4Surface *_savegameThumbnail;
};

class GUIObject {
protected:
	View *_parent;
	Common::Rect _bounds;
	M4Surface *_background;
public:
	GUIObject(View *owner, const Common::Rect &bounds);
	virtual ~GUIObject() {}

	bool isInside(int x, int y) { return _bounds.contains(x, y); }
	Common::Rect getBounds() const { return _bounds; }

	virtual void onRefresh() {}
};

class MenuObject : public GUIObject {
public:
	typedef void (*Callback)(DialogView *view, MenuObject *item);
protected:
	MenuObject::Callback _callback;
	ObjectType _objectType;
	MenuObjectState	_objectState;
	bool _transparent;
	int _objectId;
public:
	MenuObject(DialogView *owner, int objectId, int xs, int ys, int width, int height,
			   bool greyed = false, bool transparent = false);
	~MenuObject();

	DialogView *parent() { return (DialogView *)_parent; }
	MenuObjectState getState() { return _objectState; }
	virtual void setState(MenuObjectState state) {
		_objectState = state;
		onRefresh();
	}
	int getObjectId() { return _objectId; }

	void onExecute();
	virtual bool onEvent(M4EventType event, int32 param, int x, int y, MenuObject *&currentItem) { return false; }
};

class MenuButton : public MenuObject {
public:
	MenuButton(DialogView *owner, int buttonId, int xs, int ys, int width, int height,
		Callback callbackFn = NULL, bool greyed = false, bool transparent = false,
		ObjectType buttonType = OBJTYPE_BUTTON);

	void onRefresh();
	bool onEvent(M4EventType event, int32 param, int x, int y, MenuObject *&currentItem);
	bool getToggled() { return _objectType == OBJTYPE_OM_SWITCH_ON; }
};

enum MenuHorizSliderState {HSLIDER_THUMB_NORMAL = 0, HSLIDER_THUMB_MOUSEOVER = 1, HSLIDER_THUMB_PRESSED = 2};
#define SLIDER_BAR_COLOR 129

class MenuHorizSlider : public MenuObject {
protected:
	MenuHorizSliderState _sliderState;
	Common::Point _thumbSize;
	int _maxThumbX;
	int _percent;
	int _thumbX;
public:
	MenuHorizSlider(DialogView *owner, int sliderId, int xs, int ys, int width, int height,
		int initialPercentage, Callback callbackFn = NULL, bool transparent = false);

	void onRefresh();
	bool onEvent(M4EventType event, int32 param, int x, int y, MenuObject *&currentItem);
	int percent() { return _percent; }
};

enum MenuVertSliderState {
	VSLIDER_NONE	  = 0x0000,
	VSLIDER_UP        = 0x0010,
	VSLIDER_PAGE_UP   = 0x0020,
	VSLIDER_THUMBNAIL = 0x0030,
	VSLIDER_PAGE_DOWN = 0x0040,
	VSLIDER_DOWN      = 0x0050
};

class MenuVertSlider : public MenuObject {
protected:
	MenuVertSliderState _sliderState;
	Common::Point _thumbSize;
	int _percent;
	int _thumbY;
	int _minThumbY;
	int _maxThumbY;

	MenuVertSliderState getSliderArea(int y);
	void updateThumbnails() {}
public:
	MenuVertSlider(DialogView *owner, int sliderId, int xs, int ys, int width, int height,
		int initialPercentage, Callback callbackFn = NULL, bool transparent = false);

	void onRefresh();
	bool onEvent(M4EventType event, int32 param, int x, int y, MenuObject *&currentItem);
	MenuVertSliderState sliderState() { return _sliderState; }
	int percent() { return _percent; }
	void setPercentage(int value);
};

class MenuMessage : public MenuObject {
public:
	MenuMessage(DialogView *owner, int objectId, int x, int y, int w, int h, bool transparent = false);

	void onRefresh();
};

class MenuImage : public MenuObject {
private:
	M4Surface *_sprite;
public:
	MenuImage(DialogView *owner, int objectId, int xs, int ys, int width, int height,
		M4Surface *image, bool transparent = false);

	void onRefresh();
	const M4Surface *sprite() { return _sprite; }
	void setSprite(M4Surface *v) {
		_sprite = v;
		onRefresh();
	}
};

class MenuSaveLoadText : public MenuButton {
private:
	bool _loadFlag;
	const char *_displayText;
	int _displayValue;
	int _index;
	bool _visible;
public:
	MenuSaveLoadText(DialogView *owner, int textId, int xs, int ys, int width, int height,
		Callback callbackFn = NULL, bool greyed = false, bool transparent = false,
		bool loadFlag = false, const char *displayText = NULL, int displayValue = 0);

	void onRefresh();
	bool onEvent(M4EventType event, int32 param, int x, int y, MenuObject *&currentItem);
	void setDisplay(int value, const char *text) { _displayValue = value; _displayText = text; }
	int getIndex() { return _index; }
	const char *getText() { return _displayText; }
	bool getVisible() const { return _visible; }
	void setVisible(bool value);
};

class MenuTextField : public MenuObject {
private:
	int _displayValue;
	char _displayText[MAX_SAVEGAME_NAME];
	int _pixelWidth;
	char *_promptEnd;
	char *_cursor;
public:
	MenuTextField(DialogView *owner, int fieldId, int xs, int ys, int width, int height,
		bool greyed, Callback callbackFn = NULL, const char *displayText = NULL,
		int displayValue = 0, bool transparent = true);

	void onRefresh();
	bool onEvent(M4EventType event, int32 param, int x, int y, MenuObject *&currentItem);

	const char *getText() { return _displayText; }
	int getDisplayValue() { return _displayValue; }

};

class GUIRect : public GUIObject {
private:
	int _tag;
public:
	GUIRect(View *owner, const Common::Rect &bounds, int tag): GUIObject(owner, bounds) { _tag = tag; }

	virtual bool onEvent(M4EventType eventType, int32 param, int x, int y, GUIObject *&currentItem) { return false; }
	int getTag() const { return _tag; }
};

enum GUIButtonState {BUTTON_NORMAL, BUTTON_MOUSEOVER, BUTTON_PRESSED};

class GUIButton : public GUIRect {
protected:
	M4Surface *_normalSprite, *_mouseOverSprite, *_pressedSprite;
	GUIButtonState _buttonState;
	bool _visible;
	bool _tracking;
public:
	GUIButton(View *owner, const Common::Rect &bounds, int tag,
		M4Surface *normalSprite, M4Surface *mouseOverSprite, M4Surface *pressedSprite);

	void onRefresh();
	bool onEvent(M4EventType eventType, int32 param, int x, int y, GUIObject *&currentItem);
	GUIButtonState getState() const { return _buttonState; }
};

class GUITextField : public GUIRect {
private:
	Common::String _text;
public:
	GUITextField(View *owner, const Common::Rect &bounds);
	void setText(const char *value) {
		_text = value;
		onRefresh();
	}

	void onRefresh();
};


class Dialogs {
public:
	void keyMouseCollision() {}
};

class GameInterfaceView : public View {
public:
	GameInterfaceView(MadsM4Engine *vm, const Common::Rect &rect): View(vm, rect) {}
	~GameInterfaceView() {}

	virtual void initialize() {}
	virtual void setSelectedObject(int objectNumber) {}
	virtual void addObjectToInventory(int objectNumber) {}
};

}

#endif
