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

#ifndef XEEN_DIALOGS_H
#define XEEN_DIALOGS_H

#include "common/array.h"
#include "common/stack.h"
#include "common/rect.h"
#include "mm/xeen/cutscenes.h"
#include "mm/xeen/sprites.h"
#include "mm/shared/xeen/xsurface.h"

namespace MM {
namespace Xeen {

class XeenEngine;

class UIButton {
public:
	Common::Rect _bounds;
	SpriteResource *_sprites;
	int _value;
	uint _frameNum, _selectedFrame;
	bool _draw;
#ifdef USE_TTS
	uint8 _ttsIndex;
	bool _canVoice;
#endif

	/**
	 * Constructor
	 */
	UIButton(const Common::Rect &bounds, int value, uint frameNum, SpriteResource *sprites, bool draw, uint8 ttsIndex, bool ttsCanVoice) :
		_bounds(bounds), _value(value), _frameNum(frameNum), _selectedFrame(frameNum | 1),
		_sprites(sprites), _draw(draw) {
#ifdef USE_TTS
		_ttsIndex = ttsIndex;
		_canVoice = ttsCanVoice;	
#endif
	}

	/**
	 * Constructor
	 */
	UIButton() : _value(0), _frameNum(0), _selectedFrame(0), _sprites(nullptr), _draw(false) {
#ifdef USE_TTS
		_ttsIndex = 0;
		_canVoice = false;
#endif
	}

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
#ifdef USE_TTS
	Common::StringArray _buttonTexts;
	int _previousButton;
#endif

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
	void loadStrings(const Common::Path &name);

	/**
	 * Load a set of text strings from the given resource
	 * @param name		Name of resource containing strings
	 * @param ccMode	Optional cc file number to explicitly use
	 */
	void loadStrings(const Common::Path &name, int ccMode);

	/**
	 * Sets the wait bounds, the area where clicking with the mouse generates
	 * the equivalent of a space bar press, to the main interface area
	 */
	void setWaitBounds();

#ifdef USE_TTS
	/**
	 * Reads one or more sections of text, then combines them
	 * @param text			Text to take the sections from. Each section should be separated by one or more newlines
	 * @param index			Starting index, which is moved to the start of the following section, or npos if no sections remain
	 * @param count			How many sections to read
	 * @param separator		What to separate sections with
	 * @returns				The text sections combined, each separated by the separator
	 */
	Common::String getNextTextSection(const Common::String &text, uint &index, uint count, const char *separator) const;

	/**
	 * Reads one or more sections of text, then combines them. Newlines are kept intact
	 * @param text			Text to take the sections from. Each section should be separated by one or more newlines
	 * @param index			Starting index, which is moved to the start of the following section, or npos if no sections remain
	 * @param count			How many sections to read
	 * @returns				The text sections combined 
	 */
	Common::String getNextTextSection(const Common::String &text, uint &index, uint count) const;

	/**
	 * Reads one section of text
	 * @param text			Text to take the section from. Each section should be separated by one or more newlines
	 * @param index			Starting index, which is moved to the start of the following section, or npos if no sections remain
	 * @returns				The text section
	 */
	Common::String getNextTextSection(const Common::String &text, uint &index) const;

	/**
	 * Reads one section of text and adds it to the button texts
	 * @param text		Text to take the section from. Each section should be separated by one or more newlines
	 * @param index		Starting index, which is moved to the start of the following section, or npos if no sections remain
	 * @returns			The text section
	 */
	Common::String addNextTextToButtons(const Common::String &text, uint &index);

	/**
	 * Reads one or more sections of text and adds them to the button texts
	 * @param text		Text to take the sections from. Each section should be separated by one or more newlines
	 * @param index		Starting index, which is moved to the start of the following section, or npos if no sections remain
	 * @param count		How many sections to read
	 * @returns			The text sections combined
	 */
	Common::String addNextTextToButtons(const Common::String &text, uint &index, uint count);

	/**
	 * Sets the text of each button for use by TTS
	 * @param text	Text for buttons. Each button's text should be separated by newlines
	 */
	void setButtonTexts(const Common::String &text);

	/**
	 * Disables the voicing of buttons from the start index to the end index
	 * @param startIndex	Starting index
	 * @param endIndex		Ending index
	 */
	void disableButtonVoicing(uint startIndex, uint endIndex);

	/**
	 * Enables the voicing of buttons from the start index to the end index
	 * @param startIndex	Starting index
	 * @param endIndex		Ending index
	 */
	void enableButtonVoicing(uint startIndex, uint endIndex);
#endif
public:
	ButtonContainer(XeenEngine *vm) : Cutscenes(vm), _buttonValue(0) {
#ifdef USE_TTS
		_previousButton = -1;
#endif
	}

	/**
	 * Saves the current list of buttons
	 */
	void saveButtons();

	void clearButtons();

	void restoreButtons();

	void addButton(const Common::Rect &bounds, int val,
		SpriteResource *sprites = nullptr, uint8 ttsIndex = UINT8_MAX);
	void addButton(const Common::Rect &bounds, int val,
		int frameNum, SpriteResource *sprites = nullptr, uint8 ttsIndex = UINT8_MAX);

	void addPartyButtons(XeenEngine *vm);

	/**
	 * Draws the buttons onto the passed surface
	 */
	void drawButtons(XSurface *surface);

	/**
	 * Clears any currently set button value
	 */
	void clearEvents() { _buttonValue = 0; }

#ifdef USE_TTS
	/**
	 * Checks if a button is being hovered over and voices its text with TTS, if there is text
	 */
	void checkHoverOverButton();
#endif
};

class SettingsBaseDialog : public ButtonContainer {
protected:
	virtual void showContents(SpriteResource &title1, bool mode);
public:
	SettingsBaseDialog(XeenEngine *vm) : ButtonContainer(vm) {}

	~SettingsBaseDialog() override {}
};

} // End of namespace Xeen
} // End of namespace MM

#endif
