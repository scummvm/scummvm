/* Copyright (C) 1994-2004 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/rect.h"
#include "common/config-manager.h"
#include "sword2/sword2.h"
#include "sword2/controls.h"
#include "sword2/defs.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/driver/d_draw.h"
#include "sword2/driver/d_sound.h"

#define	MAX_STRING_LEN		64	// 20 was too low; better to be safe ;)
#define CHARACTER_OVERLAP	 2	// overlap characters by 3 pixels

// our fonts start on SPACE character (32)
#define SIZE_OF_CHAR_SET	(256 - 32)

#define MAX_WIDGETS 25

namespace Sword2 {

class Widget;

/**
 * Base class for all dialogs.
 */

class Dialog {
private:
	int _numWidgets;
	Widget *_widgets[MAX_WIDGETS];
	bool _finish;
	int _result;

public:
	Gui *_gui;

	Dialog(Gui *gui);
	virtual ~Dialog();

	void registerWidget(Widget *widget);

	virtual void paint();
	virtual void setResult(int result);

	int run();

	virtual void onAction(Widget *widget, int result = 0) {}
};

/**
 * Base class for all widgets.
 */

class Widget {
protected:
	Dialog *_parent;

	SpriteInfo *_sprites;

	struct WidgetSurface {
		uint8 *_surface;
		bool _original;
	};
 
	WidgetSurface *_surfaces;
	int _numStates;
	int _state;

	Common::Rect _hitRect;

public:
	Widget(Dialog *parent, int states);

	virtual ~Widget();

	void createSurfaceImage(int state, uint32 res, int x, int y, uint32 pc);
	void linkSurfaceImage(Widget *from, int state, int x, int y);

	void createSurfaceImages(uint32 res, int x, int y);
	void linkSurfaceImages(Widget *from, int x, int y);

	void setHitRect(int x, int y, int width, int height);
	bool isHit(int16 x, int16 y);

	void setState(int state);
	int getState();

	virtual void paint(Common::Rect *clipRect = NULL);

	virtual void onMouseEnter() {}
	virtual void onMouseExit() {}
	virtual void onMouseMove(int x, int y) {}
	virtual void onMouseDown(int x, int y) {}
	virtual void onMouseUp(int x, int y) {}
	virtual void onWheelUp(int x, int y) {}
	virtual void onWheelDown(int x, int y) {}
	virtual void onKey(KeyboardEvent *ke) {}
	virtual void onTick() {}

	virtual void releaseMouse(int x, int y) {}
};

/**
 * This class is used to draw text in dialogs, buttons, etc.
 */

class FontRendererGui {
private:
	Gui *_gui;
	
	struct Glyph {
		uint8 *_data;
		int _width;
		int _height;
	};

	Glyph _glyph[SIZE_OF_CHAR_SET];

	int _fontId;

public:
	enum {
		kAlignLeft,
		kAlignRight,
		kAlignCenter
	};

	FontRendererGui(Gui *gui, int fontId);
	~FontRendererGui();

	void fetchText(uint32 textId, uint8 *buf);

	int getCharWidth(uint8 c);
	int getCharHeight(uint8 c);

	int getTextWidth(uint8 *text);
	int getTextWidth(uint32 textId);

	void drawText(uint8 *text, int x, int y, int alignment = kAlignLeft);
	void drawText(uint32 textId, int x, int y, int alignment = kAlignLeft);
};

FontRendererGui::FontRendererGui(Gui *gui, int fontId)
	: _gui(gui), _fontId(fontId) {
	uint8 *font = _gui->_vm->_resman->openResource(fontId);
	FrameHeader *head;
	SpriteInfo sprite;

	sprite.type = RDSPR_NOCOMPRESSION | RDSPR_TRANS;

	for (int i = 0; i < SIZE_OF_CHAR_SET; i++) {
		head = (FrameHeader *) _gui->_vm->fetchFrameHeader(font, i);
		sprite.data = (uint8 *) (head + 1);
		sprite.w = head->width;
		sprite.h = head->height;
		_gui->_vm->_graphics->createSurface(&sprite, &_glyph[i]._data);
		_glyph[i]._width = head->width;
		_glyph[i]._height = head->height;
	}

	_gui->_vm->_resman->closeResource(fontId);
}

FontRendererGui::~FontRendererGui() {
	for (int i = 0; i < SIZE_OF_CHAR_SET; i++)
		_gui->_vm->_graphics->deleteSurface(_glyph[i]._data);
}

void FontRendererGui::fetchText(uint32 textId, uint8 *buf) {
	uint8 *data = _gui->_vm->fetchTextLine(_gui->_vm->_resman->openResource(textId / SIZE), textId & 0xffff);
	int i;

	for (i = 0; data[i + 2]; i++) {
		if (buf)
			buf[i] = data[i + 2];
	}
			
	buf[i] = 0;
	_gui->_vm->_resman->closeResource(textId / SIZE);
}

int FontRendererGui::getCharWidth(uint8 c) {
	if (c < 32)
		return 0;
	return _glyph[c - 32]._width;
}

int FontRendererGui::getCharHeight(uint8 c) {
	if (c < 32)
		return 0;
	return _glyph[c - 32]._height;
}

int FontRendererGui::getTextWidth(uint8 *text) {
	int textWidth = 0;

	for (int i = 0; text[i]; i++)
		if (text[i] >= ' ')
			textWidth += (getCharWidth(text[i]) - CHARACTER_OVERLAP);
	return textWidth;
}

int FontRendererGui::getTextWidth(uint32 textId) {
	uint8 text[MAX_STRING_LEN];

	fetchText(textId, text);
	return getTextWidth(text);
}

void FontRendererGui::drawText(uint8 *text, int x, int y, int alignment) {
	SpriteInfo sprite;
	int i;

	if (alignment != kAlignLeft) {
		int textWidth = getTextWidth(text);

		switch (alignment) {
		case kAlignRight:
			x -= textWidth;
			break;
		case kAlignCenter:
			x -= (textWidth / 2);
			break;
		}
	}

	sprite.x = x;
	sprite.y = y;

	for (i = 0; text[i]; i++) {
		if (text[i] >= ' ') {
			sprite.w = getCharWidth(text[i]);
			sprite.h = getCharHeight(text[i]);

			_gui->_vm->_graphics->drawSurface(&sprite, _glyph[text[i] - 32]._data);

			sprite.x += (getCharWidth(text[i]) - CHARACTER_OVERLAP);
		}
	}
}

void FontRendererGui::drawText(uint32 textId, int x, int y, int alignment) {
	uint8 text[MAX_STRING_LEN];

	fetchText(textId, text);
	drawText(text, x, y, alignment);
}

//
// Dialog class functions
//

Dialog::Dialog(Gui *gui)
	: _numWidgets(0), _finish(false), _result(0), _gui(gui) {
	_gui->_vm->setFullPalette(CONTROL_PANEL_PALETTE);
	_gui->_vm->_graphics->clearScene();

	// HACK: Since the dialogs don't do normal scene updates we need to
	// trigger a full redraw manually.

	_gui->_vm->_graphics->setNeedFullRedraw();
	_gui->_vm->_graphics->updateDisplay();
}

Dialog::~Dialog() {
	for (int i = 0; i < _numWidgets; i++)
		delete _widgets[i];
}

void Dialog::registerWidget(Widget *widget) {
	if (_numWidgets < MAX_WIDGETS)
		_widgets[_numWidgets++] = widget;
}

void Dialog::paint() {
	_gui->_vm->_graphics->clearScene();
	for (int i = 0; i < _numWidgets; i++)
		_widgets[i]->paint();
}

void Dialog::setResult(int result) {
	_result = result;
	_finish = true;
}

int Dialog::run() {
	int i;

	paint();

	int16 oldMouseX = -1;
	int16 oldMouseY = -1;

	while (!_finish) {
		// So that the menu icons will reach their full size
		_gui->_vm->_graphics->processMenu();
		_gui->_vm->_graphics->updateDisplay(false);

		int16 newMouseX = _gui->_vm->_input->_mouseX;
		int16 newMouseY = _gui->_vm->_input->_mouseY + 40;

		MouseEvent *me = _gui->_vm->_input->mouseEvent();
		KeyboardEvent ke;
		int32 keyboardStatus = _gui->_vm->_input->readKey(&ke);

		if (keyboardStatus == RD_OK) {
			if (ke.keycode == 27)
				setResult(0);
			else if (ke.keycode == '\n' || ke.keycode == '\r')
				setResult(1);
		}

		int oldHit = -1;
		int newHit = -1;

		// Find out which widget the mouse was over the last time, and
		// which it is currently over. This assumes the widgets do not
		// overlap.

		for (i = 0; i < _numWidgets; i++) {
			if (_widgets[i]->isHit(oldMouseX, oldMouseY))
				oldHit = i;
			if (_widgets[i]->isHit(newMouseX, newMouseY))
				newHit = i;
		}

		// Was the mouse inside a widget the last time?

		if (oldHit >= 0) {
			if (newHit != oldHit)
				_widgets[oldHit]->onMouseExit();
		}

		// Is the mouse currently in a widget?

		if (newHit >= 0) {
			if (newHit != oldHit)
				_widgets[newHit]->onMouseEnter();

			if (me) {
				switch (me->buttons) {
				case RD_LEFTBUTTONDOWN:
					_widgets[newHit]->onMouseDown(newMouseX, newMouseY);
					break;
				case RD_LEFTBUTTONUP:
					_widgets[newHit]->onMouseUp(newMouseX, newMouseY);
					break;
				case RD_WHEELUP:
					_widgets[newHit]->onWheelUp(newMouseX, newMouseY);
					break;
				case RD_WHEELDOWN:
					_widgets[newHit]->onWheelDown(newMouseX, newMouseY);
					break;
				}
			}
		}

		// Some events are passed to the widgets regardless of where
		// the mouse cursor is.

		for (i = 0; i < _numWidgets; i++) {
			if (me && me->buttons == RD_LEFTBUTTONUP) {
				// So that slider widgets will know when the
				// user releases the mouse button, even if the
				// cursor is outside of the slider's hit area.
				_widgets[i]->releaseMouse(newMouseX, newMouseY);
			}

			// This is to make it easier to drag the slider widget

			if (newMouseX != oldMouseX || newMouseY != oldMouseY)
				_widgets[i]->onMouseMove(newMouseX, newMouseY);

			if (keyboardStatus == RD_OK)
				_widgets[i]->onKey(&ke);

			_widgets[i]->onTick();
		}

		oldMouseX = newMouseX;
		oldMouseY = newMouseY;

		_gui->_vm->_system->delay_msecs(20);

		if (_gui->_vm->_quit)
			setResult(0);
	}

	return _result;
}

//
// Widget functions
//

Widget::Widget(Dialog *parent, int states)
	: _parent(parent), _numStates(states), _state(0) {
	_sprites = (SpriteInfo *) calloc(states, sizeof(SpriteInfo));
	_surfaces = (WidgetSurface *) calloc(states, sizeof(WidgetSurface));

	_hitRect.left = _hitRect.right = _hitRect.top = _hitRect.bottom = -1;
}

Widget::~Widget() {
	for (int i = 0; i < _numStates; i++) {
		if (_surfaces[i]._original)
			_parent->_gui->_vm->_graphics->deleteSurface(_surfaces[i]._surface);
	}
	free(_sprites);
	free(_surfaces);
}

void Widget::createSurfaceImage(int state, uint32 res, int x, int y, uint32 pc) {
	uint8 *file, *colTablePtr = NULL;
	AnimHeader *anim_head;
	FrameHeader *frame_head;
	CdtEntry *cdt_entry;
	uint32 spriteType = RDSPR_TRANS;

	// open anim resource file, point to base
	file = _parent->_gui->_vm->_resman->openResource(res);

	anim_head = _parent->_gui->_vm->fetchAnimHeader(file);
	cdt_entry = _parent->_gui->_vm->fetchCdtEntry(file, pc);
	frame_head = _parent->_gui->_vm->fetchFrameHeader(file, pc);

	// If the frame is flipped. (Only really applicable to frames using
	// offsets.)

	if (cdt_entry->frameType & FRAME_FLIPPED)
		spriteType |= RDSPR_FLIP;

	// Which compression was used?

	switch (anim_head->runTimeComp) {
	case NONE:
		spriteType |= RDSPR_NOCOMPRESSION;
		break;
	case RLE256:
		spriteType |= RDSPR_RLE256;
		break;
	case RLE16:
		spriteType |= RDSPR_RLE256;
		// Points to just after last cdt_entry, i.e. start of colour
		// table
		colTablePtr = (uint8 *) (anim_head + 1) +
			anim_head->noAnimFrames * sizeof(CdtEntry);
		break;
	}

	_sprites[state].x = x;
	_sprites[state].y = y;
	_sprites[state].w = frame_head->width;
	_sprites[state].h = frame_head->height;
	_sprites[state].scale = 0;
	_sprites[state].type = spriteType;
	_sprites[state].blend = anim_head->blend;

	// Points to just after frame header, ie. start of sprite data
	_sprites[state].data = (uint8 *) (frame_head + 1);

	_parent->_gui->_vm->_graphics->createSurface(&_sprites[state], &_surfaces[state]._surface);
	_surfaces[state]._original = true;

	// Release the anim resource
	_parent->_gui->_vm->_resman->closeResource(res);
}

void Widget::linkSurfaceImage(Widget *from, int state, int x, int y) {
	_sprites[state].x = x;
	_sprites[state].y = y;
	_sprites[state].w = from->_sprites[state].w;
	_sprites[state].h = from->_sprites[state].h;
	_sprites[state].scale = from->_sprites[state].scale;
	_sprites[state].type = from->_sprites[state].type;
	_sprites[state].blend = from->_sprites[state].blend;

	_surfaces[state]._surface = from->_surfaces[state]._surface;
	_surfaces[state]._original = false;
}

void Widget::createSurfaceImages(uint32 res, int x, int y) {
	for (int i = 0; i < _numStates; i++)
		createSurfaceImage(i, res, x, y, i);
}

void Widget::linkSurfaceImages(Widget *from, int x, int y) {
	for (int i = 0; i < from->_numStates; i++)
		linkSurfaceImage(from, i, x, y);
}

void Widget::setHitRect(int x, int y, int width, int height) {
	_hitRect.left = x;
	_hitRect.right = x + width;
	_hitRect.top = y;
	_hitRect.bottom = y + height;
}

bool Widget::isHit(int16 x, int16 y) {
	return _hitRect.left >= 0 && _hitRect.contains(x, y);
}

void Widget::setState(int state) {
	if (state != _state) {
		_state = state;
		paint();
	}
}

int Widget::getState() {
	return _state;
}

void Widget::paint(Common::Rect *clipRect) {
	_parent->_gui->_vm->_graphics->drawSurface(&_sprites[_state], _surfaces[_state]._surface, clipRect);
}

/**
 * Standard button class.
 */

class Button : public Widget {
public:
	Button(Dialog *parent, int x, int y, int w, int h)
		: Widget(parent, 2) {
		setHitRect(x, y, w, h);
	}

	virtual void onMouseExit() {
		setState(0);
	}

	virtual void onMouseDown(int x, int y) {
		setState(1);
	}

	virtual void onMouseUp(int x, int y) {
		if (getState() != 0) {
			setState(0);
			_parent->onAction(this);
		}
	}
};

/**
 * Scroll buttons are used to scroll the savegame list. The difference between
 * this and a normal button is that we want this to repeat.
 */

class ScrollButton : public Widget {
private:
	uint32 _holdCounter;

public:
	ScrollButton(Dialog *parent, int x, int y, int w, int h)
		: Widget(parent, 2), _holdCounter(0) {
		setHitRect(x, y, w, h);
	}

	virtual void onMouseExit() {
		setState(0);
	}

	virtual void onMouseDown(int x, int y) {
		setState(1);
		_parent->onAction(this);
		_holdCounter = 0;
	}

	virtual void onMouseUp(int x, int y) {
		setState(0);
	}

	virtual void onTick() {
		if (getState() != 0) {
			_holdCounter++;
			if (_holdCounter > 16 && (_holdCounter % 4) == 0)
				_parent->onAction(this);
		}
	}
};

/**
 * A switch is a button that changes state when clicked, and keeps that state
 * until clicked again.
 */

class Switch : public Widget {
private:
	bool _holding, _value;
	int _upState, _downState;

public:
	Switch(Dialog *parent, int x, int y, int w, int h)
		: Widget(parent, 2), _holding(false), _value(false),
		  _upState(0), _downState(1) {
		setHitRect(x, y, w, h);
	}

	// The sound mute switches have 0 as their "down" state and 1 as
	// their "up" state, so this function is needed to get consistent
	// behaviour.

	void reverseStates() {
		_upState = 1;
		_downState = 0;
	}

	void setValue(bool value) {
		_value = value;
		if (_value)
			setState(_downState);
		else
			setState(_upState);
	}

	bool getValue() {
		return _value;
	}

	virtual void onMouseExit() {
		if (_holding && !_value)
			setState(_upState);
		_holding = false;
	}

	virtual void onMouseDown(int x, int y) {
		_holding = true;
		setState(_downState);
	}

	virtual void onMouseUp(int x, int y) {
		if (_holding) {
			_holding = false;
			_value = !_value;
			if (_value)
				setState(_downState);
			else
				setState(_upState);
			_parent->onAction(this, getState());
		}
	}
};

/**
 * A slider is used to specify a value within a pre-defined range.
 */

class Slider : public Widget {
private:
	Widget *_background;
	bool _dragging;
	int _value, _targetValue;
	int _maxValue;
	int _dragOffset;

	int posFromValue(int value) {
		return _hitRect.left + (value * (_hitRect.width() - 38)) / _maxValue;
	}

	int valueFromPos(int x) {
		return (int) ((double) (_maxValue * (x - _hitRect.left)) / (double) (_hitRect.width() - 38) + 0.5);
	}

public:
	Slider(Dialog *parent, Widget *background, int max,
		int x, int y, int w, int h, Widget *base = NULL)
		: Widget(parent, 1), _background(background),
		  _dragging(false), _value(0), _targetValue(0),
		  _maxValue(max) {
		setHitRect(x, y, w, h);

		if (base)
			linkSurfaceImages(base, x, y);
		else
			createSurfaceImages(3406, x, y);
	}

	virtual void paint(Common::Rect *clipRect = NULL) {
		// This will redraw a bit more than is strictly necessary,
		// but I doubt that will make any noticeable difference.

		_background->paint(&_hitRect);
		Widget::paint(clipRect);
	}

	void setValue(int value) {
		_value = value;
		_targetValue = value;
		_sprites[0].x = posFromValue(_value);
		paint();
	}

	int getValue() {
		return _value;
	}

	virtual void onMouseMove(int x, int y) {
		if (_dragging) {
			int newX = x - _dragOffset;
			int newValue;

			if (newX < _hitRect.left)
				newX = _hitRect.left;
			else if (newX + 38 > _hitRect.right)
				newX = _hitRect.right - 38;

			_sprites[0].x = newX;

			newValue = valueFromPos(newX);
			if (newValue != _value) {
				_value = newValue;
				_targetValue = newValue;
				_parent->onAction(this, newValue);
			}

			paint();
		}
	}

	virtual void onMouseDown(int x, int y) {
		if (x >= _sprites[0].x && x < _sprites[0].x + 38) {
			_dragging = true;
			_dragOffset = x - _sprites[0].x;
		} else if (x < _sprites[0].x) {
			if (_targetValue > 0)
				_targetValue--;
		} else {
			if (_targetValue < _maxValue)
				_targetValue++;
		}
	}

	virtual void releaseMouse(int x, int y) {
		if (_dragging)
			_dragging = false;
	}

	virtual void onTick() {
		if (!_dragging) {
			int target = posFromValue(_targetValue);

			if (target != _sprites[0].x) {
				if (target < _sprites[0].x) {
					_sprites[0].x -= 4;
					if (_sprites[0].x < target)
						_sprites[0].x = target;
				} else if (target > _sprites[0].x) {
					_sprites[0].x += 4;
					if (_sprites[0].x > target)
						_sprites[0].x = target;
				}

				int newValue = valueFromPos(_sprites[0].x);
				if (newValue != _value) {
					_value = newValue;
					_parent->onAction(this, newValue);
				}

				paint();
			}
		}
	}
};

/**
 * A "mini" dialog is usually a yes/no question, but also used for the
 * restart/restore dialog at the beginning of the game.
 */

class MiniDialog : public Dialog {
private:
	uint32 _headerTextId;
	uint32 _okTextId;
	uint32 _cancelTextId;
	FontRendererGui *_fr;
	Widget *_panel;
	Button *_okButton;
	Button *_cancelButton;

public:
	MiniDialog(Gui *gui, uint32 headerTextId, uint32 okTextId = 149618688, uint32 cancelTextId = 149618689)
		: Dialog(gui), _headerTextId(headerTextId), _okTextId(okTextId), _cancelTextId(cancelTextId) {
		_fr = new FontRendererGui(_gui, _gui->_vm->_controlsFontId);

		_panel = new Widget(this, 1);
		_panel->createSurfaceImages(1996, 203, 104);

		_okButton = new Button(this, 243, 214, 24, 24);
		_okButton->createSurfaceImages(2002, 243, 214);

		_cancelButton = new Button(this, 243, 276, 24, 24);
		_cancelButton->linkSurfaceImages(_okButton, 243, 276);

		registerWidget(_panel);
		registerWidget(_okButton);
		registerWidget(_cancelButton);
	}

	~MiniDialog() {
		delete _fr;
	}

	virtual void paint() {
		Dialog::paint();

		if (_headerTextId)
			_fr->drawText(_headerTextId, 310, 134, FontRendererGui::kAlignCenter);
		_fr->drawText(_okTextId, 270, 214);
		_fr->drawText(_cancelTextId, 270, 276);
	}

	virtual void onAction(Widget *widget, int result = 0) {
		if (widget == _okButton)
			setResult(1);
		else if (widget == _cancelButton)
			setResult(0);
	}
};

/**
 * The game settings dialog.
 */

class OptionsDialog : public Dialog {
private:
	FontRendererGui *_fr;
	Widget *_panel;
	Switch *_objectLabelsSwitch;
	Switch *_subtitlesSwitch;
	Switch *_reverseStereoSwitch;
	Switch *_musicSwitch;
	Switch *_speechSwitch;
	Switch *_fxSwitch;
	Slider *_musicSlider;
	Slider *_speechSlider;
	Slider *_fxSlider;
	Slider *_gfxSlider;
	Widget *_gfxPreview;
	Button *_okButton;
	Button *_cancelButton;

public:
	OptionsDialog(Gui *gui) : Dialog(gui) {
		_fr = new FontRendererGui(gui, gui->_vm->_controlsFontId);

		_panel = new Widget(this, 1);
		_panel->createSurfaceImages(3405, 0, 40);

		_objectLabelsSwitch = new Switch(this, 304, 100, 53, 32);
		_objectLabelsSwitch->createSurfaceImages(3687, 304, 100);

		_subtitlesSwitch = new Switch(this, 510, 100, 53, 32);
		_subtitlesSwitch->linkSurfaceImages(_objectLabelsSwitch, 510, 100);

		_reverseStereoSwitch = new Switch(this, 304, 293, 53, 32);
		_reverseStereoSwitch->linkSurfaceImages(_objectLabelsSwitch, 304, 293);

		_musicSwitch = new Switch(this, 516, 157, 40, 32);
		_musicSwitch->createSurfaceImages(3315, 516, 157);
		_musicSwitch->reverseStates();

		_speechSwitch = new Switch(this, 516, 205, 40, 32);
		_speechSwitch->linkSurfaceImages(_musicSwitch, 516, 205);
		_speechSwitch->reverseStates();

		_fxSwitch = new Switch(this, 516, 250, 40, 32);
		_fxSwitch->linkSurfaceImages(_musicSwitch, 516, 250);
		_fxSwitch->reverseStates();

		_musicSlider = new Slider(this, _panel, 16, 309, 161, 170, 27);
		_speechSlider = new Slider(this, _panel, 14, 309, 208, 170, 27, _musicSlider);
		_fxSlider = new Slider(this, _panel, 14, 309, 254, 170, 27, _musicSlider);
		_gfxSlider = new Slider(this, _panel, 3, 309, 341, 170, 27, _musicSlider);

		_gfxPreview = new Widget(this, 4);
		_gfxPreview->createSurfaceImages(256, 495, 310);

		_okButton = new Button(this, 203, 382, 53, 32);
		_okButton->createSurfaceImages(901, 203, 382);

		_cancelButton = new Button(this, 395, 382, 53, 32);
		_cancelButton->linkSurfaceImages(_okButton, 395, 382);

		registerWidget(_panel);
		registerWidget(_objectLabelsSwitch);
		registerWidget(_subtitlesSwitch);
		registerWidget(_reverseStereoSwitch);
		registerWidget(_musicSwitch);
		registerWidget(_speechSwitch);
		registerWidget(_fxSwitch);
		registerWidget(_musicSlider);
		registerWidget(_speechSlider);
		registerWidget(_fxSlider);
		registerWidget(_gfxSlider);
		registerWidget(_gfxPreview);
		registerWidget(_okButton);
		registerWidget(_cancelButton);

		_gui->readOptionSettings();

		_objectLabelsSwitch->setValue(_gui->_pointerTextSelected);
		_subtitlesSwitch->setValue(_gui->_subtitles);
		_reverseStereoSwitch->setValue(_gui->_stereoReversed);
		_musicSwitch->setValue(!_gui->_vm->_sound->isMusicMute());
		_speechSwitch->setValue(!_gui->_vm->_sound->isSpeechMute());
		_fxSwitch->setValue(!_gui->_vm->_sound->isFxMute());
		_musicSlider->setValue(_gui->_vm->_sound->getMusicVolume());
		_speechSlider->setValue(_gui->_vm->_sound->getSpeechVolume());
		_fxSlider->setValue(_gui->_vm->_sound->getFxVolume());
		_gfxSlider->setValue(_gui->_vm->_graphics->getRenderLevel());
		_gfxPreview->setState(_gui->_vm->_graphics->getRenderLevel());
	}

	~OptionsDialog() {
		delete _fr;
	}

	virtual void paint() {
		Dialog::paint();

		int maxWidth = 0;
		int width;

		uint32 alignTextIds[] = {
			149618700,	// object labels
			149618702,	// music volume
			149618703,	// speech volume
			149618704,	// fx volume
			149618705,	// graphics quality
			149618709,	// reverse stereo
		};

		for (int i = 0; i < ARRAYSIZE(alignTextIds); i++) {
			width = _fr->getTextWidth(alignTextIds[i]);
			if (width > maxWidth)
				maxWidth = width;
		}

		// Options
		_fr->drawText(149618698, 321, 55, FontRendererGui::kAlignCenter);
		// Subtitles
		_fr->drawText(149618699, 500, 103, FontRendererGui::kAlignRight);
		// Object labels
		_fr->drawText(149618700, 299 - maxWidth, 103);
		// Music volume
		_fr->drawText(149618702, 299 - maxWidth, 161);
		// Speech volume
		_fr->drawText(149618703, 299 - maxWidth, 208);
		// FX volume
		_fr->drawText(149618704, 299 - maxWidth, 254);
		// Reverse stereo
		_fr->drawText(149618709, 299 - maxWidth, 296);
		// Graphics quality
		_fr->drawText(149618705, 299 - maxWidth, 341);
		// Ok
		_fr->drawText(149618688, 193, 382, FontRendererGui::kAlignRight);
		// Cancel
		_fr->drawText(149618689, 385, 382, FontRendererGui::kAlignRight);
	}

	virtual void onAction(Widget *widget, int result = 0) {
		// Since there is music playing while the dialog is displayed
		// we need to update music volume immediately.

		if (widget == _musicSwitch) {
			_gui->_vm->_sound->muteMusic(result != 0);
		} else if (widget == _musicSlider) {
			_gui->_vm->_sound->setMusicVolume(result);
			_gui->_vm->_sound->muteMusic(result == 0);
			_musicSwitch->setValue(result != 0);
		} else if (widget == _speechSlider) {
			_speechSwitch->setValue(result != 0);
		} else if (widget == _fxSlider) {
			_fxSwitch->setValue(result != 0);
		} else if (widget == _gfxSlider) {
			_gfxPreview->setState(result);
			_gui->updateGraphicsLevel(result);
		} else if (widget == _okButton) {
			_gui->_subtitles = _subtitlesSwitch->getValue();
			_gui->_pointerTextSelected = _objectLabelsSwitch->getValue();
			_gui->_stereoReversed = _reverseStereoSwitch->getValue();

			// Apply the changes
			_gui->_vm->_sound->muteMusic(!_musicSwitch->getValue());
			_gui->_vm->_sound->muteSpeech(!_speechSwitch->getValue());
			_gui->_vm->_sound->muteFx(!_fxSwitch->getValue());
			_gui->_vm->_sound->setMusicVolume(_musicSlider->getValue());
			_gui->_vm->_sound->setSpeechVolume(_speechSlider->getValue());
			_gui->_vm->_sound->setFxVolume(_fxSlider->getValue());
			_gui->_vm->_sound->buildPanTable(_gui->_stereoReversed);

			_gui->updateGraphicsLevel(_gfxSlider->getValue());

			_gui->writeOptionSettings();
			setResult(1);
		} else if (widget == _cancelButton) {
			// Revert the changes
			_gui->readOptionSettings();
			setResult(0);
		}
	}
};

// FIXME: Move these into some class

enum {
	kSaveDialog,
	kLoadDialog
};

// Slot button actions. Note that keyboard input generates positive actions

enum {
	kSelectSlot = -1,
	kDeselectSlot = -2,
	kWheelDown = -3,
	kWheelUp = -4,
	kStartEditing = -5,
	kCursorTick = -6
};

class Slot : public Widget {
private:
	int _mode;
	FontRendererGui *_fr;
	uint8 _text[SAVE_DESCRIPTION_LEN];
	bool _clickable;
	bool _editable;

public:
	Slot(Dialog *parent, int x, int y, int w, int h)
		: Widget(parent, 2), _clickable(false), _editable(false) {
		setHitRect(x, y, w, h);
		_text[0] = 0;
	}

	void setMode(int mode) {
		_mode = mode;
	}

	void setClickable(bool clickable) {
		_clickable = clickable;
	}

	void setEditable(bool editable) {
		_editable = editable;
	}

	bool isEditable() {
		return _editable;
	}

	void setText(FontRendererGui *fr, int slot, uint8 *text) {
		_fr = fr;
		if (text)
			sprintf((char *) _text, "%d.  %s", slot, text);
		else
			sprintf((char *) _text, "%d.  ", slot);
	}

	uint8 *getText() {
		return &_text[0];
	}

	virtual void paint(Common::Rect *clipRect = NULL) {
		Widget::paint();

		// HACK: The main dialog is responsible for drawing the text
		// when in editing mode.

		if (!_editable)
			_fr->drawText(_text, _sprites[0].x + 16, _sprites[0].y + 4 + 2 * getState());
	}

	virtual void onMouseDown(int x, int y) {
		if (_clickable) {
			if (getState() == 0) {
				setState(1);
				_parent->onAction(this, kSelectSlot);
				if (_mode == kSaveDialog)
					_parent->onAction(this, kStartEditing);
			} else if (_mode == kLoadDialog) {
				setState(0);
				_parent->onAction(this, kDeselectSlot);
			}
		}
	}

	virtual void onWheelUp(int x, int y) {
		_parent->onAction(this, kWheelUp);
	}

	virtual void onWheelDown(int x, int y) {
		_parent->onAction(this, kWheelDown);
	}

	virtual void onKey(KeyboardEvent *ke) {
		if (_editable) {
			if (ke->keycode == 8)
				_parent->onAction(this, 8);
			else if (ke->ascii >= ' ' && ke->ascii <= 255) {
				// Accept the character if the font renderer
				// has what looks like a valid glyph for it.
				if (_fr->getCharWidth(ke->ascii) > 0)
					_parent->onAction(this, ke->ascii);
			}
		}
	}

	virtual void onTick() {
		if (_editable)
			_parent->onAction(this, kCursorTick);
	}

	void setY(int y) {
		for (int i = 0; i < _numStates; i++)
			_sprites[i].y = y;
		setHitRect(_hitRect.left, y, _hitRect.width(), _hitRect.height());
	}

	int getY() {
		return _sprites[0].y;
	}
};

class SaveLoadDialog : public Dialog {
private:
	int _mode, _selectedSlot;
	uint8 _editBuffer[SAVE_DESCRIPTION_LEN];
	int _editPos, _firstPos;
	int _cursorTick;

	FontRendererGui *_fr1;
	FontRendererGui *_fr2;
	Widget *_panel;
	Slot *_slotButton[8];
	ScrollButton *_zupButton;
	ScrollButton *_upButton;
	ScrollButton *_downButton;
	ScrollButton *_zdownButton;
	Button *_okButton;
	Button *_cancelButton;

	void saveLoadError(uint8 *text);

public:
	SaveLoadDialog(Gui *gui, int mode)
		: Dialog(gui), _mode(mode), _selectedSlot(-1) {
		int i;

		// FIXME: The "control font" and the "red font" are currently
		// always the same font, so one should be eliminated.

		_fr1 = new FontRendererGui(_gui, _gui->_vm->_controlsFontId);
		_fr2 = new FontRendererGui(_gui, _gui->_vm->_redFontId);

		_panel = new Widget(this, 1);
		_panel->createSurfaceImages(2016, 0, 40);

		for (i = 0; i < 4; i++) {
			_slotButton[i] = new Slot(this, 114, 0, 384, 36);
			_slotButton[i]->createSurfaceImages(2006 + i, 114, 0);
			_slotButton[i]->setMode(mode);
			_slotButton[i + 4] = new Slot(this, 114, 0, 384, 36);
			_slotButton[i + 4]->linkSurfaceImages(_slotButton[i], 114, 0);
			_slotButton[i + 4]->setMode(mode);
		}

		updateSlots();

		_zupButton = new ScrollButton(this, 516, 65, 17, 17);
		_zupButton->createSurfaceImages(1982, 516, 65);

		_upButton = new ScrollButton(this, 516, 85, 17, 17);
		_upButton->createSurfaceImages(2067, 516, 85);

		_downButton = new ScrollButton(this, 516, 329, 17, 17);
		_downButton->createSurfaceImages(1986, 516, 329);

		_zdownButton = new ScrollButton(this, 516, 350, 17, 17);
		_zdownButton->createSurfaceImages(1988, 516, 350);

		_okButton = new Button(this, 130, 377, 24, 24);
		_okButton->createSurfaceImages(2002, 130, 377);

		_cancelButton = new Button(this, 350, 377, 24, 24);
		_cancelButton->linkSurfaceImages(_okButton, 350, 377);

		registerWidget(_panel);

		for (i = 0; i < 8; i++)
			registerWidget(_slotButton[i]);

		registerWidget(_zupButton);
		registerWidget(_upButton);
		registerWidget(_downButton);
		registerWidget(_zdownButton);
		registerWidget(_okButton);
		registerWidget(_cancelButton);
	}

	~SaveLoadDialog() {
		delete _fr1;
		delete _fr2;
	}

	// There aren't really a hundred different button objects of course,
	// there are only eight. Re-arrange them to simulate scrolling.

	void updateSlots() {
		for (int i = 0; i < 8; i++) {
			Slot *slot = _slotButton[(_gui->_baseSlot + i) % 8];
			FontRendererGui *fr;
			uint8 description[SAVE_DESCRIPTION_LEN];

			slot->setY(72 + i * 36);

			if (_gui->_baseSlot + i == _selectedSlot) {
				slot->setEditable(_mode == kSaveDialog);
				slot->setState(1);
				fr = _fr2;
			} else {
				slot->setEditable(false);
				slot->setState(0);
				fr = _fr1;
			}

			if (_gui->_vm->getSaveDescription(_gui->_baseSlot + i, description) == SR_OK) {
				slot->setText(fr, _gui->_baseSlot + i, description);
				slot->setClickable(true);
			} else {
				slot->setText(fr, _gui->_baseSlot + i, NULL);
				slot->setClickable(_mode == kSaveDialog);
			}

			if (slot->isEditable())
				drawEditBuffer(slot);
			else
				slot->paint();
		}
	}

	virtual void onAction(Widget *widget, int result = 0) {
		if (widget == _zupButton) {
			if (_gui->_baseSlot > 0) {
				if (_gui->_baseSlot >= 8)
					_gui->_baseSlot -= 8;
				else
					_gui->_baseSlot = 0;
				updateSlots();
			}
		} else if (widget == _upButton) {
			if (_gui->_baseSlot > 0) {
				_gui->_baseSlot--;
				updateSlots();
			}
		} else if (widget == _downButton) {
			if (_gui->_baseSlot < 92) {
				_gui->_baseSlot++;
				updateSlots();
			}
		} else if (widget == _zdownButton) {
			if (_gui->_baseSlot < 92) {
				if (_gui->_baseSlot <= 84)
					_gui->_baseSlot += 8;
				else
					_gui->_baseSlot = 92;
				updateSlots();
			}
		} else if (widget == _okButton) {
			setResult(1);
		} else if (widget == _cancelButton) {
			setResult(0);
		} else {
			Slot *slot = (Slot *) widget;
			int textWidth;
			uint8 tmp;
			int i;
			int j;

			switch (result) {
			case kWheelUp:
				onAction(_upButton);
				break;
			case kWheelDown:
				onAction(_downButton);
				break;
			case kSelectSlot:
			case kDeselectSlot:
				if (result == kSelectSlot)
					_selectedSlot = _gui->_baseSlot + (slot->getY() - 72) / 35;
				else if (result == kDeselectSlot)
					_selectedSlot = -1;

				for (i = 0; i < 8; i++)
					if (widget == _slotButton[i])
						break;

				for (j = 0; j < 8; j++) {
					if (j != i) {
						_slotButton[j]->setEditable(false);
						_slotButton[j]->setState(0);
					}
				}
				break;
			case kStartEditing:
				if (_selectedSlot >= 10)
					_firstPos = 5;
				else
					_firstPos = 4;

				strcpy((char *) _editBuffer, (char *) slot->getText());
				_editPos = strlen((char *) _editBuffer);
				_cursorTick = 0;
				_editBuffer[_editPos] = '_';
				_editBuffer[_editPos + 1] = 0;
				slot->setEditable(true);
				drawEditBuffer(slot);
				break;
			case kCursorTick:
				_cursorTick++;
				if (_cursorTick == 7) {
					_editBuffer[_editPos] = ' ';
					drawEditBuffer(slot);
				} else if (_cursorTick == 14) {
					_cursorTick = 0;
					_editBuffer[_editPos] = '_';
					drawEditBuffer(slot);
				}
				break;
			case 8:
				if (_editPos > _firstPos) {
					_editBuffer[_editPos - 1] = _editBuffer[_editPos];
					_editBuffer[_editPos--] = 0;
					drawEditBuffer(slot);
				}
				break;
			default:
				tmp = _editBuffer[_editPos];
				_editBuffer[_editPos] = 0;
				textWidth = _fr2->getTextWidth(_editBuffer);
				_editBuffer[_editPos] = tmp;

				if (textWidth < 340 && _editPos < SAVE_DESCRIPTION_LEN - 2) {
					_editBuffer[_editPos + 1] = _editBuffer[_editPos];
					_editBuffer[_editPos + 2] = 0;
					_editBuffer[_editPos++] = result;
					drawEditBuffer(slot);
				}
				break;
			}
		}
	}

	void drawEditBuffer(Slot *slot) {
		if (_selectedSlot == -1)
			return;

		// This will redraw a bit more than is strictly necessary,
		// but I doubt that will make any noticeable difference.

		slot->paint();
		_fr2->drawText(_editBuffer, 130, 78 + (_selectedSlot - _gui->_baseSlot) * 36);
	}

	virtual void paint() {
		Dialog::paint();

		if (_mode == kLoadDialog) {
			// Restore
			_fr1->drawText(149618690, 165, 377);
		} else {
			// Save
			_fr1->drawText(149618691, 165, 377);
		}
		// Cancel
		_fr1->drawText(149618689, 382, 377);
	}

	virtual void setResult(int result) {
		// Cancel

		if (result == 0) {
			Dialog::setResult(result);
			return;
		}

		// Save / Restore

		if (_selectedSlot == -1)
			return;

		if (_mode == kSaveDialog) {
			if (_editPos <= _firstPos)
				return;

			_editBuffer[_editPos] = 0;

			uint32 rv = _gui->_vm->saveGame(_selectedSlot, (uint8 *) &_editBuffer[_firstPos]);

			if (rv != SR_OK) {
				uint32 textId;

				switch (rv) {
				case SR_ERR_FILEOPEN:
					textId = 213516674;
					break;
				default:	// SR_ERR_WRITEFAIL
					textId = 213516676;
					break;
				}

				saveLoadError(_gui->_vm->fetchTextLine(_gui->_vm->_resman->openResource(textId / SIZE), textId & 0xffff) + 2);
				result = 0;
			}
		} else {
			uint32 rv = _gui->_vm->restoreGame(_selectedSlot);

			if (rv != SR_OK) {
				uint32 textId;

				switch (rv) {
				case SR_ERR_FILEOPEN:
					textId = 213516670;
					break;
				case SR_ERR_INCOMPATIBLE:
					textId = 213516671;
					break;
				default:	// SR_ERR_READFAIL
					textId = 213516673;
					break;
				}

				saveLoadError(_gui->_vm->fetchTextLine(_gui->_vm->_resman->openResource(textId / SIZE), textId & 0xffff) + 2);
				result = 0;
			} else {
				// Prime system with a game cycle

				// Reset the graphic 'BuildUnit' list before a
				// new logic list (see fnRegisterFrame)
				_gui->_vm->resetRenderLists();

				// Reset the mouse hot-spot list (see
				// fnRegisterMouse and fnRegisterFrame)
				_gui->_vm->resetMouseList();

				if (_gui->_vm->_logic->processSession())
					error("restore 1st cycle failed??");
			}
		}

		Dialog::setResult(result);
	}
};

void SaveLoadDialog::saveLoadError(uint8* text) {
	// Print a message on screen. Second parameter is duration.
	_gui->_vm->displayMsg((uint8 *) text, 0);

	// Wait for ESC or mouse click
	while (1) {
		MouseEvent *me;

		_gui->_vm->_graphics->updateDisplay();

		if (_gui->_vm->_input->keyWaiting()) {
			KeyboardEvent ke;

			_gui->_vm->_input->readKey(&ke);
			if (ke.keycode == 27)
				break;
		}

		me = _gui->_vm->_input->mouseEvent();
		if (me && (me->buttons & RD_LEFTBUTTONDOWN))
			break;

		_gui->_vm->_system->delay_msecs(20);
	}

	// Remove the message.
	_gui->_vm->removeMsg();
}

Gui::Gui(Sword2Engine *vm) : _vm(vm), _baseSlot(0) {
	int i;

	for (i = 0; i < ARRAYSIZE(_musicVolume); i++) {
		_musicVolume[i] = (i * 255) / (ARRAYSIZE(_musicVolume) - 1);
		if ((i * 255) % (ARRAYSIZE(_musicVolume) - 1))
			_musicVolume[i]++;
	}

	for (i = 0; i < ARRAYSIZE(_soundVolume); i++) {
		_soundVolume[i] = (i * 255) / (ARRAYSIZE(_soundVolume) - 1);
		if ((i * 255) % (ARRAYSIZE(_soundVolume) - 1))
			_soundVolume[i]++;
	}

	ConfMan.registerDefault("music_volume", _musicVolume[12]);
	ConfMan.registerDefault("speech_volume", _soundVolume[10]);
	ConfMan.registerDefault("sfx_volume", _soundVolume[10]);
	ConfMan.registerDefault("music_mute", false);
	ConfMan.registerDefault("speech_mute", false);
	ConfMan.registerDefault("sfx_mute", false);
	ConfMan.registerDefault("gfx_details", 2);
	ConfMan.registerDefault("subtitles", false);
	ConfMan.registerDefault("object_labels", true);
	ConfMan.registerDefault("reverse_stereo", false);
}

void Gui::readOptionSettings(void) {
	_subtitles = ConfMan.getBool("subtitles");
	_pointerTextSelected = ConfMan.getBool("object_labels");
	_stereoReversed = ConfMan.getBool("reverse_stereo");

	updateGraphicsLevel((uint8) ConfMan.getInt("gfx_details"));

	_vm->_sound->setMusicVolume((16 * ConfMan.getInt("music_volume")) / 255);
	_vm->_sound->setSpeechVolume((14 * ConfMan.getInt("speech_volume")) / 255);
	_vm->_sound->setFxVolume((14 * ConfMan.getInt("sfx_volume")) / 255);
	_vm->_sound->muteMusic(ConfMan.getBool("music_mute"));
	_vm->_sound->muteSpeech(ConfMan.getBool("speech_mute"));
	_vm->_sound->muteFx(ConfMan.getBool("sfx_mute"));
	_vm->_sound->buildPanTable(_stereoReversed);
}

void Gui::writeOptionSettings(void) {
	ConfMan.set("music_volume", _musicVolume[_vm->_sound->getMusicVolume()]);
	ConfMan.set("speech_volume", _soundVolume[_vm->_sound->getSpeechVolume()]);
	ConfMan.set("sfx_volume", _soundVolume[_vm->_sound->getFxVolume()]);
	ConfMan.set("music_mute", _vm->_sound->isMusicMute());
	ConfMan.set("speech_mute", _vm->_sound->isSpeechMute());
	ConfMan.set("sfx_mute", _vm->_sound->isFxMute());
	ConfMan.set("gfx_details", _vm->_graphics->getRenderLevel());
	ConfMan.set("subtitles", _subtitles);
	ConfMan.set("object_labels", _pointerTextSelected);
	ConfMan.set("reverse_stereo", _stereoReversed);

	ConfMan.flushToDisk();
}

uint32 Gui::restoreControl(void) {
	// returns 0 for no restore
	//         1 for restored ok

	SaveLoadDialog loadDialog(this, kLoadDialog);
	return loadDialog.run();
}

void Gui::saveControl(void) {
	SaveLoadDialog saveDialog(this, kSaveDialog);
	OSystem::Property prop;
	prop.show_keyboard = true;
	_vm->_system->property(OSystem::PROP_TOGGLE_VIRTUAL_KEYBOARD, &prop);
	saveDialog.run();
	prop.show_keyboard = false;
	_vm->_system->property(OSystem::PROP_TOGGLE_VIRTUAL_KEYBOARD, &prop);
}

bool Gui::startControl(void) {
	while (1) {
		MiniDialog startDialog(this, 0, 149618693, 149618690);

		if (startDialog.run())
			return true;

		if (_vm->_quit)
			return false;

		if (restoreControl())
			return false;

		if (_vm->_quit)
			return false;
	}

	return true;
}

void Gui::quitControl(void) {
	MiniDialog quitDialog(this, 149618692);

	if (quitDialog.run())
		_vm->closeGame();
}

void Gui::restartControl(void) {
	uint32 temp_demo_flag;

	MiniDialog restartDialog(this, 149618693);

	if (!restartDialog.run())
		return;

	// Restart the game. To do this, we must...

	// Stop music instantly!
	_vm->killMusic();

	// In case we were dead - well we're not anymore!
	DEAD = 0;

	// Restart the game. Clear all memory and reset the globals
	temp_demo_flag = DEMO;

	// Remove all resources from memory, including player object and
	// global variables
	_vm->_resman->removeAll();

	// Reopen global variables resource & send address to interpreter -
	// it won't be moving
	_vm->_logic->setGlobalInterpreterVariables((int32 *) (_vm->_resman->openResource(1) + sizeof(StandardHeader)));
	_vm->_resman->closeResource(1);

	DEMO = temp_demo_flag;

	// Rree all the route memory blocks from previous game
	_vm->_logic->_router->freeAllRouteMem();

	// Call the same function that first started us up
	_vm->startGame();

	// Prime system with a game cycle

	// Reset the graphic 'BuildUnit' list before a new logic list
	// (see fnRegisterFrame)
	_vm->resetRenderLists();

	// Reset the mouse hot-spot list (see fnRegisterMouse and
	// fnRegisterFrame)
	_vm->resetMouseList();

	_vm->_graphics->closeMenuImmediately();

	// FOR THE DEMO - FORCE THE SCROLLING TO BE RESET!
	// - this is taken from fnInitBackground
	// switch on scrolling (2 means first time on screen)
	_vm->_thisScreen.scroll_flag = 2;

	if (_vm->_logic->processSession())
		error("restart 1st cycle failed??");

	// So palette not restored immediately after control panel - we want
	// to fade up instead!
	_vm->_thisScreen.new_palette = 99;
}

void Gui::optionControl(void) {
	OptionsDialog optionsDialog(this);

	optionsDialog.run();
	return;
}

void Gui::updateGraphicsLevel(int newLevel) {
	if (newLevel < 0)
		newLevel = 0;
	else if (newLevel > 3)
		newLevel = 3;

	_vm->_graphics->setRenderLevel(newLevel);

	// update our global variable - which needs to be checked when dimming
	// the palette in PauseGame() in sword2.cpp (since palette-matching
	// cannot be done with dimmed palette so we turn down one notch while
	// dimmed, if at top level)

	_currentGraphicsLevel = newLevel;
}

} // End of namespace Sword2
