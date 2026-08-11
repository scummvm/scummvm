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

#ifndef XEEN_DIALOGS_CONTROL_PANEL_H
#define XEEN_DIALOGS_CONTROL_PANEL_H

#include "mm/xeen/dialogs/dialogs.h"

namespace MM {
namespace Xeen {

class ControlPanel : public ButtonContainer {
private:
	SpriteResource _iconSprites;
	Common::String _btnSoundText, _btnMusicText;
	bool _debugFlag;
private:
	ControlPanel(XeenEngine *vm) : ButtonContainer(vm), _debugFlag(false) {}

	/**
	 * Inner handler for showing the dialog
	 */
	int execute();

	/**
	 * Loads the buttons for the dialog
	 */
	void loadButtons();

	/**
	 * Gets the text for the dialog buttons
	 */
	Common::String getButtonText();

	/**
	 * Gets the current time
	 */
	Common::String getTimeText() const;

#ifdef USE_TTS
	/**
	 * Voices text as text-to-speech and sets the buttons
	 * @param text	Text of the control panel, each piece separated by a newline
	 */
	void speakText(const Common::String &text);

	/**
	 * Voices "On" or "Off" text and changes the TTS text of an on/off button to this text
	 * @param on				Whether to voice "on" or "off"
	 * @param buttonTextIndex	The index of the button text to change
	 */
	void voiceOnOffText(bool on, uint buttonTextIndex);
#endif
public:
	/**
	 * Show the control panel
	 */
	static int show(XeenEngine *vm);
};

} // End of namespace Xeen
} // End of namespace MM

#endif
