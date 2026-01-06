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
#ifndef PELROCK_DIALOG_H
#define PELROCK_DIALOG_H

#include "common/scummsys.h"
#include "graphics/screen.h"

#include "pelrock/events.h"
#include "pelrock/fonts/large_font.h"
#include "pelrock/fonts/small_font.h"
#include "pelrock/graphics.h"
#include "pelrock/types.h"

namespace Pelrock {

// Control character codes (negative values in signed char)
#define CHAR_SPACE 0x20        /* ' ' */
#define CTRL_SPEAKER_ID 0x08        /* Next byte is speaker ID (color) */
#define CTRL_END_TEXT 0xFD          /* End of text segment */
#define CTRL_TEXT_TERMINATOR 0xFC   /* Text terminator */
#define CTRL_DIALOGUE_MARKER 0xF1   /* Choice marker that sticks */
#define CTRL_DISABLED_CHOICE 0xFA   /* Disabled choice marker */
#define CTRL_PAGE_BREAK_CONV 0xF9   /* Page break in conversation */
#define CTRL_ACTION_TRIGGER 0xF8    /* Action trigger */
#define CTRL_END_BRANCH 0xF7        /* End of branch */
#define CTRL_LINE_CONTINUE 0xF6     /* Line continue/newline */
#define CTRL_ALT_END_MARKER_1 0xF5  /* Alt end marker - do nothing */
#define CTRL_END_CONVERSATION 0xF4  /* End conversation and disable option */
#define CTRL_DIALOGUE_MARKER_2 0xFB /* Alt choice marker that disappears */
#define CTRL_GO_BACK 0xF0           /* Go back in conversation */
#define CTRL_ALT_END_MARKER_2 0xEB  /* Alt end marker 2 */
#define CTRL_ALT_END_MARKER_3 0xFE  /* Alt end marker 3 */

/**
 * Structure to hold a parsed choice option
 */
struct ChoiceOption {
	int index;
	Common::String text;
	bool isDisabled;
	uint32 dataOffset;
	bool shouldDisableOnSelect = false;

	ChoiceOption() : index(-1), isDisabled(false), dataOffset(0) {}
};

static void debugHexString(const Common::String &str, const char *label = nullptr) {
	if (label) {
		debug("%s:", label);
	}

	Common::String hexOutput;
	for (uint i = 0; i < str.size(); i++) {
		hexOutput += Common::String::format("%02X ", (unsigned char)str[i]);
	}
	debug("%s", hexOutput.c_str());
}

class DialogManager {
private:
	Graphics::Screen *_screen = nullptr;
	PelrockEventManager *_events = nullptr;
	GraphicsManager *_graphics = nullptr;
	Sprite *_curSprite = nullptr;

	// Private helper functions for conversation parsing
	void displayDialogue(Common::Array<Common::Array<Common::String>> dialogueLines, byte speakerId);
	void displayDialogue(Common::String text, byte speakerId);
	uint32 readTextBlock(const byte *data, uint32 dataSize, uint32 startPos, Common::String &outText, byte &outSpeakerId);
	uint32 parseChoices(const byte *data, uint32 dataSize, uint32 startPos, Common::Array<ChoiceOption> *outChoices);
	void setCurSprite(int index);
	void checkMouse();
	void sayAlfred(Common::StringArray texts);

public:
	DialogManager(Graphics::Screen *screen, PelrockEventManager *events, GraphicsManager *graphics);
	~DialogManager();

	void displayChoices(Common::Array<ChoiceOption> *choices, byte *compositeBuffer);
	int selectChoice(Common::Array<Common::String> &choices, byte *compositeBuffer);
	void startConversation(const byte *conversationData, uint32 dataSize, Sprite *alfredAnimSet = nullptr);
	void sayAlfred(Description description);
	void say(Common::StringArray texts);
	bool processColorAndTrim(Common::StringArray &lines, byte &speakerId);
	Graphics::Surface getDialogueSurface(Common::Array<Common::String> dialogueLines, byte speakerId);

	Common::Array<Common::Array<Common::String>> wordWrap(Common::String text);
	Common::Array<Common::Array<Common::String>> wordWrap(Common::StringArray texts);
	Common::Array<ChoiceOption> *_currentChoices = nullptr;
};

} // End of namespace Pelrock
#endif // PELROCK_DIALOG_H
