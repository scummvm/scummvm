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
#include "pelrock/types.h"

namespace Pelrock {
/**
 * Structure to hold a parsed choice option
 */
struct ChoiceOption {
	int index;
	Common::String text;
	bool isDisabled;
	uint32 dataOffset;

	ChoiceOption() : index(-1), isDisabled(false), dataOffset(0) {}
};

class DialogManager {
private:
	Graphics::Screen *_screen = nullptr;
	PelrockEventManager *_events = nullptr;
	Sprite *_curSprite = nullptr;

	// Private helper functions for conversation parsing
	void displayDialogue(Common::Array<Common::Array<Common::String>> dialogueLines, byte speakerId);
	void displayDialogue(Common::String text, byte speakerId);
	uint32 readTextBlock(const byte *data, uint32 dataSize, uint32 startPos,
						 Common::String &outText, byte &outSpeakerId);
	uint32 parseChoices(const byte *data, uint32 dataSize, uint32 startPos, Common::Array<ChoiceOption> &outChoices);

	void checkMouse();

public:
	DialogManager(Graphics::Screen *screen, PelrockEventManager *events);
	~DialogManager();

	void displayChoices(Common::Array<ChoiceOption> *choices, byte *compositeBuffer);
	int selectChoice(Common::Array<Common::String> &choices, byte *compositeBuffer);
	void startConversation(const byte *conversationData, uint32 dataSize, Sprite *alfredAnimSet = nullptr);
	Common::Array<Common::Array<Common::String>> wordWrap(Common::String text);
	Common::Array<ChoiceOption> *_currentChoices = nullptr;
};

} // End of namespace Pelrock
#endif // PELROCK_DIALOG_H
