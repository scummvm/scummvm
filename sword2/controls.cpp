/* Copyright (C) 1994-2003 Revolution Software Ltd
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

#include "stdafx.h"
#include "bs2/driver/driver96.h"
#include "bs2/build_display.h"
#include "bs2/console.h"
#include "bs2/controls.h"
#include "bs2/defs.h"
#include "bs2/header.h"
#include "bs2/interpreter.h"
#include "bs2/layers.h"
#include "bs2/logic.h"
#include "bs2/maketext.h"			// for font resource variables
#include "bs2/mouse.h"
#include "bs2/protocol.h"
#include "bs2/resman.h"
#include "bs2/router.h"
#include "bs2/save_rest.h"
#include "bs2/sound.h"
#include "bs2/sword2.h"

namespace Sword2 {

#define	MAX_STRING_LEN		64	// 20 was too low; better to be safe ;)
#define CHARACTER_OVERLAP	 2	// overlap characters by 3 pixels

// our fonts start on SPACE character (32)
#define SIZE_OF_CHAR_SET (256 - 32)

Gui gui;

enum {
	kAlignLeft,
	kAlignRight,
	kAlignCenter
};

class FontRendererGui {
private:
	struct Glyph {
		uint8 *_data;
		int _width;
		int _height;
	} _glyph[SIZE_OF_CHAR_SET];
	int _fontId;

public:
	FontRendererGui(int fontId) : _fontId(fontId) {
		uint8 *font = res_man.open(fontId);
		_frameHeader *head;
		_spriteInfo sprite;

		sprite.type = RDSPR_NOCOMPRESSION | RDSPR_TRANS;

		for (int i = 0; i < SIZE_OF_CHAR_SET; i++) {
			head = (_frameHeader *) FetchFrameHeader(font, i);
			sprite.data = (uint8 *) (head + 1);
			sprite.w = head->width;
			sprite.h = head->height;
			g_display->createSurface(&sprite, &_glyph[i]._data);
			_glyph[i]._width = head->width;
			_glyph[i]._height = head->height;
		}

		res_man.close(fontId);
	}

	~FontRendererGui() {
		for (int i = 0; i < SIZE_OF_CHAR_SET; i++)
			g_display->deleteSurface(_glyph[i]._data);
	}

	void fetchText(int textId, char *buf) {
		uint8 *data = FetchTextLine(res_man.open(textId / SIZE), textId & 0xffff);
		int i;

		for (i = 0; data[i + 2]; i++) {
			if (buf)
				buf[i] = data[i + 2];
		}
			
		buf[i] = 0;
		res_man.close(textId / SIZE);
	}

	int getTextWidth(char *text) {
		int textWidth = 0;

		for (int i = 0; text[i]; i++)
			textWidth += (_glyph[text[i] - 32]._width - CHARACTER_OVERLAP);
		return textWidth;
	}

	int getTextWidth(int textId) {
		char text[MAX_STRING_LEN];

		fetchText(textId, text);
		return getTextWidth(text);
	}

	void drawText(char *text, int x, int y, int alignment = kAlignLeft);
	void drawText(int textId, int x, int y, int alignment = kAlignLeft);
};

void FontRendererGui::drawText(char *text, int x, int y, int alignment) {
	_spriteInfo sprite;
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
		sprite.w = _glyph[text[i] - 32]._width;
		sprite.h = _glyph[text[i] - 32]._height;

		g_display->drawSurface(&sprite, _glyph[text[i] - 32]._data);

		sprite.x += (_glyph[(int) text[i] - 32]._width - CHARACTER_OVERLAP);
	}
}

void FontRendererGui::drawText(int textId, int x, int y, int alignment) {
	char text[MAX_STRING_LEN];

	fetchText(textId, text);
	drawText(text, x, y, alignment);
}

class Dialog;

typedef struct Surface {
	uint8 *_surface;
	bool _original;
} WidgetSurface;

class Widget {
protected:
	Dialog *_parent;

	_spriteInfo *_sprites;
	WidgetSurface *_surfaces;
	int _numStates;
	int _state;

	Common::Rect _hitRect;

public:
	Widget(Dialog *parent, int states) :
			_parent(parent), _numStates(states), _state(0) {
		_sprites = (_spriteInfo *) calloc(states, sizeof(_spriteInfo));
		_surfaces = (WidgetSurface *) calloc(states, sizeof(WidgetSurface));

		_hitRect.left = _hitRect.right = _hitRect.top = _hitRect.bottom = -1;
	}

	virtual ~Widget() {
		for (int i = 0; i < _numStates; i++) {
			if (_surfaces[i]._original)
				g_display->deleteSurface(_surfaces[i]._surface);
		}
		free(_sprites);
		free(_surfaces);
	}

	void createSurfaceImage(int state, uint32 res, int x, int y, uint32 pc);
	void linkSurfaceImage(Widget *from, int state, int x, int y);

	void createSurfaceImages(uint32 res, int x, int y) {
		for (int i = 0; i < _numStates; i++)
			createSurfaceImage(i, res, x, y, i);
	}

	void linkSurfaceImages(Widget *from, int x, int y) {
		for (int i = 0; i < from->_numStates; i++)
			linkSurfaceImage(from, i, x, y);
	}

	void setHitRect(int x, int y, int width, int height) {
		_hitRect.left = x;
		_hitRect.right = x + width;
		_hitRect.top = y;
		_hitRect.bottom = y + height;
	}

	bool isHit(int16 x, int16 y) {
		return _hitRect.left >= 0 && _hitRect.contains(x, y);
	}

	void setState(int state) {
		if (state != _state) {
			_state = state;
			paint();
		}
	}

	int getState() {
		return _state;
	}

	virtual void paint(Common::Rect *clipRect = NULL) {
		g_display->drawSurface(&_sprites[_state], _surfaces[_state]._surface, clipRect);
	}

	virtual void onMouseEnter() {}
	virtual void onMouseExit() {}
	virtual void onMouseMove(int x, int y) {}
	virtual void onMouseDown(int x, int y) {}
	virtual void onMouseUp(int x, int y) {}
	virtual void onKey(_keyboardEvent *ke) {}
	virtual void onTick() {}

	virtual void releaseMouse(int x, int y) {}
};

void Widget::createSurfaceImage(int state, uint32 res, int x, int y, uint32 pc) {
	uint8 *file, *colTablePtr = NULL;
	_animHeader *anim_head;
	_frameHeader *frame_head;
	_cdtEntry *cdt_entry;
	uint32 spriteType = RDSPR_TRANS;

	// open anim resource file, point to base
	file = res_man.open(res);

	anim_head = FetchAnimHeader(file);
	cdt_entry = FetchCdtEntry(file, pc);
	frame_head = FetchFrameHeader(file, pc);

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
			anim_head->noAnimFrames * sizeof(_cdtEntry);
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

	g_display->createSurface(&_sprites[state], &_surfaces[state]._surface);
	_surfaces[state]._original = true;

	// Release the anim resource
	res_man.close(res);
};

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
};

#define MAX_WIDGETS 25

class Dialog {
private:
	int _numWidgets;
	Widget *_widgets[MAX_WIDGETS];
	bool _finish;
	int _result;

public:
	Dialog() : _numWidgets(0), _finish(false), _result(0) {
		SetFullPalette(CONTROL_PANEL_PALETTE);
	}

	virtual ~Dialog() {
		for (int i = 0; i < _numWidgets; i++)
			delete _widgets[i];
	}

	void registerWidget(Widget *widget) {
		if (_numWidgets < MAX_WIDGETS) {
			_widgets[_numWidgets++] = widget;
		}
	}

	virtual void onAction(Widget *widget, int result = 0) {}

	virtual void paint() {
		g_display->clearScene();
		for (int i = 0; i < _numWidgets; i++)
			_widgets[i]->paint();
	}

	virtual void setResult(int result) {
		_result = result;
		_finish = true;
	}

	int run();
};

int Dialog::run() {
	int i;

	paint();

	int16 oldMouseX = -1;
	int16 oldMouseY = -1;

	while (!_finish) {
		// So that the menu icons will reach their full size
		g_display->processMenu();
		g_display->updateDisplay();

		int16 newMouseX = g_display->_mouseX;
		int16 newMouseY = g_display->_mouseY + 40;

		_mouseEvent *me = MouseEvent();
		_keyboardEvent ke;
		int32 keyboardStatus = ReadKey(&ke);

		if (keyboardStatus == RD_OK) {
			if (ke.keycode == 27)
				setResult(0);
			else if (ke.keycode == '\n' || ke.keycode == '\r')
				setResult(1);
		}

		for (i = 0; i < _numWidgets; i++) {
			bool oldHit = _widgets[i]->isHit(oldMouseX, oldMouseY);
			bool newHit = _widgets[i]->isHit(newMouseX, newMouseY);

			if (!oldHit && newHit)
				_widgets[i]->onMouseEnter();
			if (oldHit && !newHit)
				_widgets[i]->onMouseExit();
			if (g_display->_mouseX != oldMouseX || g_display->_mouseY != oldMouseY)
				_widgets[i]->onMouseMove(newMouseX, newMouseY);

			if (me) {
				switch (me->buttons) {
				case RD_LEFTBUTTONDOWN:
					if (newHit)
						_widgets[i]->onMouseDown(newMouseX, newMouseY);
					break;
				case RD_LEFTBUTTONUP:
					if (newHit)
						_widgets[i]->onMouseUp(newMouseX, newMouseY);
					// So that slider widgets will know
					// when the user releases the mouse
					// button, even if the cursor is
					// outside of the slider's hit area.
					_widgets[i]->releaseMouse(newMouseX, newMouseY);
					break;
				}
			}

			if (keyboardStatus == RD_OK)
				_widgets[i]->onKey(&ke);

			_widgets[i]->onTick();
		}

		oldMouseX = newMouseX;
		oldMouseY = newMouseY;

		g_system->delay_msecs(20);
	}

	return _result;
}

class Button : public Widget {
public:
	Button(Dialog *parent, int x, int y, int w, int h) :
			Widget(parent, 2) {
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

class ScrollButton : public Widget {
private:
	uint32 _holdCounter;

public:
	ScrollButton(Dialog *parent, int x, int y, int w, int h) :
			Widget(parent, 2), _holdCounter(0) {
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

class Switch : public Widget {
private:
	bool _holding, _value;
	int _upState, _downState;

public:
	Switch(Dialog *parent, int x, int y, int w, int h) :
			Widget(parent, 2), _holding(false),
			_value(false), _upState(0), _downState(1) {
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
		int x, int y, int w, int h, Widget *base = NULL) :
			Widget(parent, 1), _background(background),
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

class MiniDialog : public Dialog {
private:
	int _textId;
	FontRendererGui *_fr;
	Widget *_panel;
	Button *_okButton;
	Button *_cancelButton;

public:
	MiniDialog(uint32 textId) : _textId(textId) {
		_fr = new FontRendererGui(g_sword2->_controlsFontId);

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

		_fr->drawText(_textId, 310, 134, kAlignCenter);
		_fr->drawText(149618688, 270, 214);	// ok
		_fr->drawText(149618689, 270, 276);	// cancel
	}

	virtual void onAction(Widget *widget, int result = 0) {
		if (widget == _okButton)
			setResult(1);
		else if (widget == _cancelButton)
			setResult(0);
	}
};

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

	int32 writeOptionSettings(void);

public:
	OptionsDialog() {
		_fr = new FontRendererGui(g_sword2->_controlsFontId);

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

		gui.readOptionSettings();

		_objectLabelsSwitch->setValue(gui._pointerTextSelected != 0);
		_subtitlesSwitch->setValue(gui._subtitles != 0);
		_reverseStereoSwitch->setValue(gui._stereoReversed != 0);
		_musicSwitch->setValue(g_sound->isMusicMute() == 0);
		_speechSwitch->setValue(g_sound->isSpeechMute() == 0);
		_fxSwitch->setValue(g_sound->isFxMute() == 0);
		_musicSlider->setValue(g_sound->getMusicVolume());
		_speechSlider->setValue(g_sound->getSpeechVolume());
		_fxSlider->setValue(g_sound->getFxVolume());
		_gfxSlider->setValue(g_display->getRenderLevel());
		_gfxPreview->setState(g_display->getRenderLevel());
	}

	~OptionsDialog() {
		delete _fr;
	}

	virtual void paint() {
		Dialog::paint();

		int maxWidth = 0;
		int width;

		int alignTextIds[] = {
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
		_fr->drawText(149618698, 321, 55, kAlignCenter);
		// Subtitles
		_fr->drawText(149618699, 500, 103, kAlignRight);
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
		_fr->drawText(149618688, 193, 382, kAlignRight);
		// Cancel
		_fr->drawText(149618689, 385, 382, kAlignRight);
	}

	virtual void onAction(Widget *widget, int result = 0) {
		// Since there is music playing while the dialog is displayed
		// we need to update music volume immediately. Everything else
		// is handled when the dialog is terminated.

		if (widget == _reverseStereoSwitch) {
			if (result != gui._stereoReversed)
				g_sound->reverseStereo();
			gui._stereoReversed = result;
		} else if (widget == _musicSwitch) {
			g_sound->muteMusic(result);
		} else if (widget == _musicSlider) {
			g_sound->setMusicVolume(result);
			g_sound->muteMusic(result == 0);
			_musicSwitch->setValue(result != 0);
		} else if (widget == _speechSlider) {
			_speechSwitch->setValue(result != 0);
		} else if (widget == _fxSlider) {
			_fxSwitch->setValue(result != 0);
		} else if (widget == _gfxSlider) {
			_gfxPreview->setState(result);
			gui.updateGraphicsLevel(result);
		} else if (widget == _okButton) {
			// Apply the changes
			g_sound->muteMusic(_musicSwitch->getValue() == 0);
			g_sound->muteSpeech(_speechSwitch->getValue() == 0);
			g_sound->muteFx(_fxSwitch->getValue() == 0);
			g_sound->setMusicVolume(_musicSlider->getValue());
			g_sound->setSpeechVolume(_speechSlider->getValue());
			g_sound->setFxVolume(_fxSlider->getValue());

			gui.updateGraphicsLevel(_gfxSlider->getValue());

			gui._subtitles = _subtitlesSwitch->getValue();
			gui._pointerTextSelected = _objectLabelsSwitch->getValue();
			gui._speechSelected = _speechSwitch->getValue();
			gui._stereoReversed = _reverseStereoSwitch->getValue();

			writeOptionSettings();
			setResult(1);
		} else if (widget == _cancelButton) {
			// Revert the changes
			gui.readOptionSettings();
			setResult(0);
		}
	}
};

int32 OptionsDialog::writeOptionSettings(void) {
	uint8 buff[10];
	char filename[256];
	SaveFile *fp;
	SaveFileManager *mgr = g_system->get_savefile_manager();
	
	sprintf(filename, "%s-settings.dat", g_sword2->_targetName);

	buff[0] = g_sound->getMusicVolume();
	buff[1] = g_sound->getSpeechVolume();
	buff[2] = g_sound->getFxVolume();
	buff[3] = g_sound->isMusicMute();
	buff[4] = g_sound->isSpeechMute();
	buff[5] = g_sound->isFxMute();
	buff[6] = g_display->getRenderLevel();
	buff[7] = gui._subtitles;
	buff[8] = gui._pointerTextSelected;
	buff[9] = gui._stereoReversed;
	
	if (!(fp = mgr->open_savefile(filename, g_sword2->getSavePath(), true)))
		return 1;

	if (fp->write(buff, 10) != 10) {
		delete fp;
		delete mgr;
		return 2;
	}

	delete fp;
	delete mgr;
	return 0;
}

enum {
	kSaveDialog,
	kLoadDialog
};

enum {
	kSelectSlot = -1,
	kDeselectSlot = -2,
	kStartEditing = 0,
	kCursorTick = 1
};

class Slot : public Widget {
private:
	int _mode;
	FontRendererGui *_fr;
	char _text[SAVE_DESCRIPTION_LEN];
	bool _clickable;
	bool _editable;

public:
	Slot(Dialog *parent, int x, int y, int w, int h) :
			Widget(parent, 2), _clickable(false),
			_editable(false) {
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

	void setText(FontRendererGui *fr, int slot, char *text) {
		_fr = fr;
		if (text)
			sprintf(_text, "%d.  %s", slot, text);
		else
			sprintf(_text, "%d.  ", slot);
	}

	char *getText() {
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

	virtual void onKey(_keyboardEvent *ke) {
		if (_editable) {
			if (ke->keycode == 8)
				_parent->onAction(this, 8);
			else if (ke->ascii >= ' ' && ke->ascii <= 'z')
				_parent->onAction(this, ke->ascii);
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
	char _editBuffer[SAVE_DESCRIPTION_LEN];
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

	void saveLoadError(char *text);

public:
	SaveLoadDialog(int mode) : _mode(mode), _selectedSlot(-1) {
		int i;

		// FIXME: The "control font" and the "red font" are currently
		// always the same font, so one should be eliminated.

		_fr1 = new FontRendererGui(g_sword2->_controlsFontId);
		_fr2 = new FontRendererGui(g_sword2->_redFontId);

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
			Slot *slot = _slotButton[(gui._baseSlot + i) % 8];
			FontRendererGui *fr;
			uint8 description[SAVE_DESCRIPTION_LEN];

			slot->setY(72 + i * 36);

			if (gui._baseSlot + i == _selectedSlot) {
				slot->setEditable(_mode == kSaveDialog);
				slot->setState(1);
				fr = _fr2;
			} else {
				slot->setEditable(false);
				slot->setState(0);
				fr = _fr1;
			}

			if (GetSaveDescription(gui._baseSlot + i, description) == SR_OK) {
				slot->setText(fr, gui._baseSlot + i, (char *) description);
				slot->setClickable(true);
			} else {
				slot->setText(fr, gui._baseSlot + i, NULL);
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
			if (gui._baseSlot > 0) {
				if (gui._baseSlot >= 8)
					gui._baseSlot -= 8;
				else
					gui._baseSlot = 0;
				updateSlots();
			}
		} else if (widget == _upButton) {
			if (gui._baseSlot > 0) {
				gui._baseSlot--;
				updateSlots();
			}
		} else if (widget == _downButton) {
			if (gui._baseSlot < 92) {
				gui._baseSlot++;
				updateSlots();
			}
		} else if (widget == _zdownButton) {
			if (gui._baseSlot < 92) {
				if (gui._baseSlot <= 84)
					gui._baseSlot += 8;
				else
					gui._baseSlot = 92;
				updateSlots();
			}
		} else if (widget == _okButton) {
			setResult(1);
		} else if (widget == _cancelButton) {
			setResult(0);
		} else {
			Slot *slot = (Slot *) widget;

			if (result >= kStartEditing) {
				if (result == kStartEditing) {
					if (_selectedSlot >= 10)
						_firstPos = 5;
					else
						_firstPos = 4;

					strcpy(_editBuffer, slot->getText());
					_editPos = strlen(_editBuffer);
					_cursorTick = 0;
					_editBuffer[_editPos] = '_';
					_editBuffer[_editPos + 1] = 0;
					slot->setEditable(true);
					drawEditBuffer(slot);
				} else if (result == kCursorTick) {
					_cursorTick++;
					if (_cursorTick == 7) {
						_editBuffer[_editPos] = ' ';
						drawEditBuffer(slot);
					} else if (_cursorTick == 14) {
						_cursorTick = 0;
						_editBuffer[_editPos] = '_';
						drawEditBuffer(slot);
					}
				} else if (result == 8) {
					if (_editPos > _firstPos) {
						_editBuffer[_editPos - 1] = _editBuffer[_editPos];
						_editBuffer[_editPos--] = 0;
						drawEditBuffer(slot);
					}
				} else {
					int textWidth;
					char tmp;

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
				}
			} else {
				if (result == kSelectSlot)
					_selectedSlot = gui._baseSlot + (slot->getY() - 72) / 35;
				else if (result == kDeselectSlot)
					_selectedSlot = -1;

				int i;

				for (i = 0; i < 8; i++)
					if (widget == _slotButton[i])
						break;

				for (int j = 0; j < 8; j++) {
					if (j != i) {
						_slotButton[j]->setEditable(false);
						_slotButton[j]->setState(0);
					}
				}
			}
		}
	}

	void drawEditBuffer(Slot *slot) {
		if (_selectedSlot == -1)
			return;

		// This will redraw a bit more than is strictly necessary,
		// but I doubt that will make any noticeable difference.

		slot->paint();
		_fr2->drawText(_editBuffer, 130, 78 + (_selectedSlot - gui._baseSlot) * 36);
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

			uint32 rv = SaveGame(_selectedSlot, (uint8 *) &_editBuffer[_firstPos]);

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

				saveLoadError((char*) (FetchTextLine(res_man.open(textId / SIZE), textId & 0xffff) + 2));
				result = 0;
			}
		} else {
			uint32 rv = RestoreGame(_selectedSlot);

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

				saveLoadError((char *) (FetchTextLine(res_man.open(textId / SIZE), textId & 0xffff) + 2));
				result = 0;
			} else {
				// Prime system with a game cycle

				// Reset the graphic 'buildit' list before a
				// new logic list (see FN_register_frame)
				Reset_render_lists();

				// Reset the mouse hot-spot list (see
				// FN_register_mouse and FN_register_frame)
				Reset_mouse_list();

				if (LLogic.processSession())
					Con_fatal_error("restore 1st cycle failed??");
			}
		}

		Dialog::setResult(result);
	}
};

void SaveLoadDialog::saveLoadError(char* text) {
	// Print a message on screen. Second parameter is duration.
	DisplayMsg((uint8 *) text, 0);

	// Wait for ESC or mouse click
	while (1) {
		_mouseEvent *me;

		g_display->updateDisplay();

		if (KeyWaiting()) {
			_keyboardEvent ke;

			ReadKey(&ke);
			if (ke.keycode == 27)
				break;
		}

		me = MouseEvent();
		if (me && (me->buttons & RD_LEFTBUTTONDOWN))
			break;

		g_system->delay_msecs(20);
	}

	// Remove the message.
	RemoveMsg();
}

uint32 Gui::restoreControl(void) {
	// returns 0 for no restore
	//         1 for restored ok

	SaveLoadDialog loadDialog(kLoadDialog);
	return loadDialog.run();
}

void Gui::saveControl(void) {
	SaveLoadDialog saveDialog(kSaveDialog);
	saveDialog.run();
}

void Gui::quitControl(void) {
	MiniDialog quitDialog(149618692);	// quit text

	if (!quitDialog.run()) {
		// just return to game
		return;
	}

	// close engine systems down
	Close_game();
	exit(0);
}

void Gui::restartControl(void) {
	uint32 temp_demo_flag;

	MiniDialog restartDialog(149618693);	// restart text

	if (!restartDialog.run()) {
		// just return to game
		return;
	}

	// Stop music instantly!
	Kill_music();

	//in case we were dead - well we're not anymore!
	DEAD = 0;

	g_display->clearScene();

	// restart the game
	// clear all memory and reset the globals

	temp_demo_flag = DEMO;

	// remove all resources from memory, including player object and
	// global variables
	res_man.removeAll();

	// reopen global variables resource & send address to interpreter -
	// it won't be moving
	SetGlobalInterpreterVariables((int32 *) (res_man.open(1) + sizeof(_standardHeader)));
	res_man.close(1);

	DEMO = temp_demo_flag;

	// free all the route memory blocks from previous game
	router.freeAllRouteMem();

	// call the same function that first started us up
	g_sword2->Start_game();

	// prime system with a game cycle

	// reset the graphic 'buildit' list before a new logic list
	// (see FN_register_frame)
	Reset_render_lists();

	// reset the mouse hot-spot list (see FN_register_mouse and
	// FN_register_frame)
	Reset_mouse_list();

	g_display->closeMenuImmediately();

	// FOR THE DEMO - FORCE THE SCROLLING TO BE RESET!
	// - this is taken from FN_init_background
	// switch on scrolling (2 means first time on screen)

	this_screen.scroll_flag = 2;

	if (LLogic.processSession())
		Con_fatal_error("restart 1st cycle failed??");

	// So palette not restored immediately after control panel - we want
	// to fade up instead!
 	this_screen.new_palette = 99;
}

int32 Gui::readOptionSettings(void) {
	// settings file is 9 bytes long:
	//   1 music volume
	//   2 speech volume
	//   3 fx volume
	//   4 music mute
	//   5 speech mute
	//   6 fx mute
	//   7 graphics level
	//   8 subtitles
	//   9 object labels

	uint8 buff[10];
	char filename[256];
	SaveFile *fp;
	SaveFileManager *mgr = g_system->get_savefile_manager();
	
	sprintf(filename, "%s-settings.dat", g_sword2->_targetName);

	if (!(fp = mgr->open_savefile(filename, g_sword2->getSavePath(), false)))
		return 1;

	if (fp->read(buff, 10) != 10) {
		delete fp;
		delete mgr;
		return 2;
	}

	delete fp;
	delete mgr;
	
	g_sound->setMusicVolume(buff[0]);
	g_sound->setSpeechVolume(buff[1]);
	g_sound->setFxVolume(buff[2]);
	g_sound->muteMusic(buff[3]);
	g_sound->muteSpeech(buff[4]);
	g_sound->muteFx(buff[5]);

	updateGraphicsLevel(buff[6]);

	_speechSelected = !buff[4];
	_subtitles = buff[7];
	_pointerTextSelected = buff[8];

	if (buff[9] != _stereoReversed)
		g_sound->reverseStereo();

	gui._stereoReversed = buff[9];
	return 0;
}

void Gui::optionControl(void) {
	OptionsDialog optionsDialog;

	optionsDialog.run();
	return;
}

void Gui::updateGraphicsLevel(uint8 newLevel) {
	g_display->setRenderLevel(newLevel);

	// update our global variable - which needs to be checked when dimming
	// the palette in PauseGame() in sword2.cpp (since palette-matching
	// cannot be done with dimmed palette so we turn down one notch while
	// dimmed, if at top level)

	_currentGraphicsLevel = newLevel;
}

} // End of namespace Sword2
