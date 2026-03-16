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
#include "common/stack.h"
#include "graphics/managed_surface.h"
#include "graphics/screen.h"

#include "pelrock/events.h"
#include "pelrock/fonts/large_font.h"
#include "pelrock/fonts/small_font.h"
#include "pelrock/graphics.h"
#include "pelrock/types.h"

namespace Pelrock {

// Control character codes
const byte kCtrlSpace                = 0x20; /* ' ' */
const byte kCtrlSpeakerId            = 0x08; /* Next byte is speaker ID (color) */
const byte kCtrlEndText              = 0xFD; /* End of text segment */
const byte kCtrlTextTerminator       = 0xFC; /* Text terminator */
const byte kCtrlDialogueMarker       = 0xF1; /* Choice marker that sticks */
const byte kCtrlDialogueMarkerOneoff = 0xFB; /* Choice marker that disappears after use */
const byte kCtrlDisabledChoice       = 0xFA; /* Disabled choice marker, generally only after usage */
const byte kCtrlPageBreakConv        = 0xF9; /* Page break in conversation */
const byte kCtrlActionAndEnd         = 0xF8; /* Action trigger and end conversation */
const byte kCtrlActionAndContinue    = 0xEB; /* Action-and-continue: dispatch action, conversation keeps going (unlike 0xF8 which exits) */
const byte kCtrlEndBranch            = 0xF7; /* End of branch */
const byte kCtrlLineContinue         = 0xF6; /* Line continue/newline */
const byte kCtrlAltEndMarker1        = 0xF5; /* Alt end marker - do nothing */
const byte kCtrlEndConversation      = 0xF4; /* End conversation and disable option */
const byte kCtrlGoBack               = 0xF0; /* Go back in conversation */
const byte kCtrlAltSpeakerRoot       = 0xFE; /* Separates conversations from different speakers */

const int kChoiceHeight = 16; 		// Height of each choice line in pixels
const int kMaxCharsPerLine = 47; 	// 47 characters
const int kMaxLines = 5;			// Maximum number of lines per page

// Helper structures for conversation state management
struct ConversationState {
	uint32 position;
	int currentChoiceLevel;
	ChoiceOption lastSelectedChoice;
	int currentRoot;
};

struct ConversationEndResult {
	bool shouldEnd;
	uint32 nextPosition;
	uint16 actionCode;
	bool hasAction;
};

class DialogManager {
	const static int kArrowWidth = 8;     // Width of arrow character for scroll
	const static int kChoicePadding = 16; // padding for the choice text surface
private:
	Graphics::Screen *_screen = nullptr;
	PelrockEventManager *_events = nullptr;
	GraphicsManager *_graphics = nullptr;
	// Current talking sprite, to disable and replace with talking animation
	Sprite *_curSprite = nullptr;

	// Private helper functions for conversation parsing
	void displayDialogue(Common::Array<Common::Array<Common::String>> dialogueLines, byte speakerId);
	void displayDialogue(Common::Array<Common::Array<Common::String>> dialogueLines, byte speakerId, int xBasePos, int yBasePos);
	void displayDialogue(Common::String text, byte speakerId);
	uint32 readTextBlock(const byte *data, uint32 dataSize, uint32 startPos, Common::String &outText, byte &outSpeakerId);
	uint32 parseChoices(const byte *data, uint32 dataSize, uint32 startPos, Common::Array<ChoiceOption> *outChoices);
	void setCurSprite(int index);
	bool checkAllSubBranchesExhausted(const byte *data, uint32 dataSize, uint32 startPos, int currentChoiceLevel);

	uint32 skipControlBytes(const byte *data, uint32 dataSize, uint32 position);
	uint32 peekNextMeaningfulByte(const byte *data, uint32 dataSize, uint32 position);
	ConversationState initializeConversation(const byte *data, uint32 dataSize, byte npcIndex);
	bool handleGoBack(const byte *data, Common::Stack<uint32> &positionStack, uint32 position, ConversationState &state);
	uint32 readAndDisplayDialogue(const byte *data, uint32 dataSize, uint32 position);
	ConversationEndResult checkConversationEnd(const byte *data, uint32 dataSize, uint32 position, int currentRoot = -1);
	void addGoodbyeOptionIfNeeded(Common::Array<ChoiceOption> *choices, int currentChoiceLevel, uint originalChoiceCount);
	uint32 processChoiceSelection(const byte *data, uint32 dataSize, Common::Array<ChoiceOption> *choices, int selectedIndex, ConversationState &state);
	void maybeDisableChoice(Common::Array<Pelrock::ChoiceOption> *choices, int selectedIndex, const byte *data, uint32 dataSize, uint32 endPos, Pelrock::ConversationState &state);

public:
	DialogManager(Graphics::Screen *screen, PelrockEventManager *events, GraphicsManager *graphics);
	~DialogManager();

	void displayChoices(Common::Array<ChoiceOption> *choices, Graphics::ManagedSurface &compositeBuffer);
	int selectChoice(Common::Array<Common::String> &choices, Graphics::ManagedSurface &compositeBuffer);
	void startConversation(const byte *conversationData, uint32 dataSize, byte npcIndex, Sprite *alfredAnimSet = nullptr);
	uint32 findRoot(int npc, int &currentRoot, uint32 position, uint32 dataSize, const byte *conversationData);
	uint32 findSpeaker(byte npcIndex, uint32 dataSize, const byte *conversationData);
	void sayAlfred(Description description);
	void sayAlfred(Common::StringArray texts);
	void say(Common::StringArray texts, byte spriteIndex = 0);
	void say(Common::StringArray texts, int16 x, int16 y);
	bool processColorAndTrim(Common::StringArray &lines, byte &speakerId);
	Graphics::Surface *getDialogueSurface(Common::Array<Common::String> dialogueLines, byte speakerId, Graphics::TextAlign alignment = Graphics::kTextAlignCenter);

	Common::Array<Common::Array<Common::String>> wordWrap(Common::String text);
	Common::Array<Common::Array<Common::String>> wordWrap(Common::StringArray texts);
	Common::Array<ChoiceOption> *_currentChoices = nullptr;

	// When true, the goodbye option is suppressed for all conversations in the current room.
	bool _goodbyeDisabled = false;

	// True while a blocking dialog or conversation is on screen.
	bool _dialogActive = false;
	bool _dismissDialog = false; // When true, the current dialog will be dismissed on the next iteration of the conversation loop (used for programmatically closing dialogs, e.g. when exiting a room)
	bool _disableClickToAdvance = false;
	bool _isNPCTalking = false;

	Common::String _leftArrow = Common::String(17);
	Common::String _rightArrow = Common::String(16);
};

} // End of namespace Pelrock
#endif // PELROCK_DIALOG_H
