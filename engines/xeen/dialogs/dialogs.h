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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XEEN_DIALOGS_H
#define XEEN_DIALOGS_H

#include "common/array.h"
#include "common/stack.h"
#include "common/rect.h"
#include "xeen/cutscenes.h"
#include "xeen/sprites.h"
#include "xeen/xsurface.h"

namespace Xeen {

class XeenEngine;

class UIButton {
public:
	Common::Rect _bounds;
	SpriteResource *_sprites;
	int _value;
	uint _frameNum, _selectedFrame;
	bool _draw;

	/**
	 * Constructor
	 */
	UIButton(const Common::Rect &bounds, int value, uint frameNum, SpriteResource *sprites, bool draw) :
		_bounds(bounds), _value(value), _frameNum(frameNum), _selectedFrame(frameNum | 1),
		_sprites(sprites), _draw(draw) {}

	/**
	 * Constructor
	 */
	UIButton() : _value(0), _frameNum(0), _selectedFrame(0), _sprites(nullptr), _draw(false) {}

	/**
	 * Set the frame
	 */
	void setFrame(uint frameNum) {
		_frameNum = frameNum;
		_selectedFrame = frameNum | 1;
	}

	/**
	 * Set the frame
	 */
	void setFrame(uint frameNum, uint selectedFrame) {
		_frameNum = frameNum;
		_selectedFrame = selectedFrame;
	}
};

class ButtonContainer : public Cutscenes {
private:
	Common::Stack< Common::Array<UIButton> > _savedButtons;
protected:
	Common::Array<UIButton> _buttons;
	Common::StringArray _textStrings;
	Common::Rect _waitBounds;
	int _buttonValue;

	bool checkEvents(XeenEngine *vm);

	/**
	 * Draws the scroll in the background
	 * @param rollUp	If true, rolls up the scroll. If false, unrolls.
	 * @param fadeIn	If true, does an initial fade in
	 * @returns		True if key or mouse pressed
	 */
	bool doScroll(bool rollUp, bool fadeIn) override;

	/**
	 * Load a set of text strings from the given resource
	 * @param name		Name of resource containing strings
	 */
	void loadStrings(const Common::String &name);

	/**
	 * Load a set of text strings from the given resource
	 * @param name		Name of resource containing strings
	 * @param ccMode	Optional cc file number to explicitly use
	 */
	void loadStrings(const Common::String &name, int ccMode);

	/**
	 * Sets the wait bounds, the area where clicking with the mouse generates
	 * the equivalent of a space bar press, to the main interface area
	 */
	void setWaitBounds();
public:
	ButtonContainer(XeenEngine *vm) : Cutscenes(vm), _buttonValue(0) {}

	/**
	 * Saves the current list of buttons
	 */
	void saveButtons();

	void clearButtons();

	void restoreButtons();

	void addButton(const Common::Rect &bounds, int val,
		SpriteResource *sprites = nullptr);
	void addButton(const Common::Rect &bounds, int val,
		int frameNum, SpriteResource *sprites = nullptr);

	void addPartyButtons(XeenEngine *vm);

	/**
	 * Draws the buttons onto the passed surface
	 */
	void drawButtons(XSurface *surface);

	/**
	 * Clears any currently set button value
	 */
	void clearEvents() { _buttonValue = 0; }
};

class SettingsBaseDialog : public ButtonContainer {
protected:
	virtual void showContents(SpriteResource &title1, bool mode);
public:
	SettingsBaseDialog(XeenEngine *vm) : ButtonContainer(vm) {}

	~SettingsBaseDialog() override {}
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_H */
