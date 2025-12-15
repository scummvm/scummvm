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

#include "pelrock/dialog.h"
#include "dialog.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

// Control character codes (negative values in signed char)
#define CHAR_SPACE 0x20        /* ' ' */
#define CHAR_END_MARKER_1 0xFD /* -3 (end of text marker) */
#define CHAR_END_MARKER_2 0xF4 /* -0xC (alternate end marker) */
#define CHAR_END_MARKER_3 0xF8 /* -8 (another end marker) */
#define CHAR_END_MARKER_4 0xF0 /* -0x10 (another end marker) */
#define CHAR_NEWLINE 0xF6      /* -10 (newline marker) */
#define CHAR_PAGE_BREAK 0xF9   /* marker inserted when switching pages */

// Conversation control bytes
#define CTRL_SPEAKER_ID 0x08        /* Next byte is speaker ID (color) */
#define CTRL_END_TEXT 0xFD          /* End of text segment */
#define CTRL_TEXT_TERMINATOR 0xFC   /* Text terminator */
#define CTRL_DIALOGUE_MARKER 0xFB   /* Choice marker */
#define CTRL_DISABLED_CHOICE 0xFA   /* Disabled choice marker */
#define CTRL_PAGE_BREAK_CONV 0xF9   /* Page break in conversation */
#define CTRL_ACTION_TRIGGER 0xF8    /* Action trigger */
#define CTRL_END_BRANCH 0xF7        /* End of branch */
#define CTRL_LINE_CONTINUE 0xF6     /* Line continue/newline */
#define CTRL_ALT_END_MARKER_1 0xF5  /* Alt end marker */
#define CTRL_END_CONVERSATION 0xF4  /* End conversation */
#define CTRL_DIALOGUE_MARKER_2 0xF1 /* Alt choice marker */
#define CTRL_GO_BACK 0xF0           /* Go back in conversation */
#define CTRL_ALT_END_MARKER_2 0xEB  /* Alt end marker 2 */
#define CTRL_ALT_END_MARKER_3 0xFE  /* Alt end marker 3 */

namespace Pelrock {

DialogManager::DialogManager(Graphics::Screen *screen, PelrockEventManager *events)
	: _screen(screen), _events(events) {
}

DialogManager::~DialogManager() {
	delete _currentChoices;
	_currentChoices = nullptr;
}

uint32 DialogManager::readTextBlock(
	const byte *data,
	uint32 dataSize,
	uint32 startPos,
	Common::String &outText,
	byte &outSpeakerId) {
	uint32 pos = startPos;
	outSpeakerId = ALFRED_COLOR; // Default to Alfred's color
	outText = "";

	// Skip control bytes at start
	while (pos < dataSize &&
		   (data[pos] == CTRL_ALT_END_MARKER_1 || data[pos] == CTRL_ALT_END_MARKER_2 ||
			data[pos] == CTRL_ALT_END_MARKER_3 || data[pos] == CTRL_TEXT_TERMINATOR ||
			data[pos] == CTRL_GO_BACK)) {
		pos++;
	}

	if (pos >= dataSize) {
		return pos;
	}

	// Check for speaker ID marker
	if (data[pos] == CTRL_SPEAKER_ID) {
		pos++;
		if (pos < dataSize) {
			outSpeakerId = data[pos];
			pos++;
		}
	}
	// Check for dialogue marker (choice text)
	else if (data[pos] == CTRL_DIALOGUE_MARKER || data[pos] == CTRL_DIALOGUE_MARKER_2) {
		pos++; // Skip marker

		// Skip choice index
		if (pos < dataSize) {
			pos++;
		}

		// Skip 2 bytes after choice index (speaker marker bytes)
		if (pos < dataSize) {
			pos++;
		}
		if (pos < dataSize) {
			pos++;
		}

		// Choice text is always spoken by ALFRED
		outSpeakerId = ALFRED_COLOR;
	}

	// Read text until control byte
	while (pos < dataSize) {
		byte b = data[pos];

		// End markers - stop reading text
		if (b == CTRL_END_TEXT || b == CTRL_END_CONVERSATION || b == CTRL_ACTION_TRIGGER ||
			b == CTRL_END_BRANCH || b == CTRL_DIALOGUE_MARKER || b == CTRL_DIALOGUE_MARKER_2 ||
			b == CTRL_TEXT_TERMINATOR || b == CTRL_ALT_END_MARKER_1 || b == CTRL_ALT_END_MARKER_2 ||
			b == CTRL_ALT_END_MARKER_3 || b == CTRL_GO_BACK || b == CTRL_SPEAKER_ID) {
			break;
		}

		if (b == CTRL_LINE_CONTINUE || b == CTRL_PAGE_BREAK_CONV) {
			outText += ' ';
			pos++;
			continue;
		}

		if (b == 0x0A || b == 0x0B || b == 0x00) {
			debug("Skipping byte 0x%02X at pos %u", b, pos);
			pos++; // Skip nulls and line feeds
			continue;
		}

		// Regular text - decode the character
		if (b >= 0x20 && b <= 0x7A) {
			outText += decodeChar(b);
		} else {
			// Try to decode special characters
			byte decoded = decodeChar(b);
			if (decoded != b || (decoded >= 0x20 && decoded <= 0x83)) {
				outText += (char)decoded;
			}
		}
		pos++;
	}

	return pos;
}

void DialogManager::displayChoices(Common::Array<ChoiceOption> *choices, byte *compositeBuffer) {
	int overlayHeight = choices->size() * kChoiceHeight + 2;
	int overlayY = 400 - overlayHeight;
	for (int x = 0; x < 640; x++) {
		for (int y = overlayY; y < 400; y++) {
			int index = y * 640 + x;
			compositeBuffer[index] = g_engine->_room->overlayRemap[compositeBuffer[index]];
		}
	}
	for (int i = 0; i < choices->size(); i++) {
		drawText(compositeBuffer, g_engine->_doubleSmallFont, (*choices)[i].text, 10, overlayY + 2 + i * kChoiceHeight, 620, 9);
	}
}

/**
 * Display dialogue text and wait for click to advance
 * @param text The text to display
 * @param speakerId The speaker ID which is used as color
 */
void DialogManager::displayDialogue(const Common::String &text, byte speakerId) {
	if (text.empty() || text.size() <= 1) {
		return;
	}

	// Clear any existing click state
	_events->_leftMouseClicked = false;

	// Render loop - display text and wait for click
	while (!g_engine->shouldQuit()) {
		_events->pollEvent();

		// Render the scene (keeps animations going)
		g_engine->renderScene(false);

		// Draw the dialogue text on top using speaker ID as color
		drawText(g_engine->_largeFont, text, _curSprite->x, _curSprite->y - 10, 640, speakerId);
		// drawText(g_engine->_largeFont, "Hola", 10, 10, 640, speakerId);

		// Present to screen
		_screen->markAllDirty();
		_screen->update();

		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;
			break;
		}
		g_system->delayMillis(10);
	}
}

/**
 * Select a choice from displayed options
 * Returns the index of the selected choice in the choices array
 */
int DialogManager::selectChoice(Common::Array<Common::String> &choices, byte *compositeBuffer) {
	// Clear any existing click state
	_events->_leftMouseClicked = false;

	int overlayHeight = choices.size() * kChoiceHeight + 2;
	int overlayY = 400 - overlayHeight;

	while (!g_engine->shouldQuit()) {
		_events->pollEvent();

		// Render the scene with choices overlay
		g_engine->renderScene(true);

		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;

			// Check if click is in the choices area
			if (_events->_mouseClickY >= overlayY) {
				int selectedIndex = (_events->_mouseClickY - overlayY - 2) / kChoiceHeight;
				if (selectedIndex >= 0 && selectedIndex < (int)choices.size()) {
					return selectedIndex;
				}
			}
		}
		g_engine->_screen->update();
		g_system->delayMillis(10);
	}

	return 0;
}

/**
 * Parse conversation choices from the data
 * @param data The conversation data
 * @param dataSize Size of data
 * @param startPos Starting position
 * @param outChoices Output: array of choice options
 * @return The position after parsing choices
 */
uint32 DialogManager::parseChoices(const byte *data, uint32 dataSize, uint32 startPos,
								   Common::Array<ChoiceOption> &outChoices) {
	uint32 pos = startPos;
	outChoices.clear();
	int firstChoiceIndex = -1;
	int choiceCount = 0;

	// Find first choice marker
	while (pos < dataSize) {
		byte b = data[pos];

		// Stop at end markers
		if (b == CTRL_ALT_END_MARKER_1 || b == CTRL_END_BRANCH || b == CTRL_ALT_END_MARKER_3) {
			break;
		}

		// Found first choice marker
		if (b == CTRL_DIALOGUE_MARKER || b == CTRL_DIALOGUE_MARKER_2) {
			if (pos + 1 < dataSize) {
				firstChoiceIndex = data[pos + 1];
				ChoiceOption opt;
				opt.index = firstChoiceIndex;
				opt.dataOffset = pos;
				opt.isDisabled = false;

				// Parse the choice text
				uint32 textPos = pos + 4; // Skip marker + index + 2 speaker bytes
				while (textPos < dataSize) {
					byte tb = data[textPos];
					if (tb == CTRL_END_TEXT || tb == CTRL_DIALOGUE_MARKER ||
						tb == CTRL_DIALOGUE_MARKER_2 || tb == CTRL_END_BRANCH ||
						tb == CTRL_ALT_END_MARKER_1) {
						break;
					}

					if (b == 0x0A || b == 0x0B || b == 0x00) {
						debug("Skipping byte 0x%02X at pos %u", b, pos);
						pos++; // Skip nulls and line feeds
						continue;
					}

					if (tb >= 0x20 && tb <= 0x7A) {
						opt.text += (char)tb;
					} else {
						byte decoded = decodeChar(tb);
						if (decoded != tb || (decoded >= 0x20 && decoded <= 0x83)) {
							opt.text += (char)decoded;
						}
					}
					textPos++;
				}

				outChoices.push_back(opt);
				choiceCount = 1;
				pos++;
				break;
			}
		}

		pos++;
	}

	// Scan for additional choices with SAME index
	while (pos < dataSize) {
		byte b = data[pos];

		// Stop at end markers
		if (b == CTRL_ALT_END_MARKER_1 || b == CTRL_END_BRANCH || b == CTRL_ALT_END_MARKER_3) {
			break;
		}

		// Found a dialogue marker
		if (b == CTRL_DIALOGUE_MARKER || b == CTRL_DIALOGUE_MARKER_2) {
			if (pos + 1 < dataSize) {
				int choiceIndex = data[pos + 1];

				// Only collect choices with same index
				if (choiceIndex == firstChoiceIndex) {
					// Check if disabled
					bool isDisabled = (b == CTRL_DISABLED_CHOICE);

					ChoiceOption opt;
					opt.index = choiceIndex;
					opt.dataOffset = pos;
					opt.isDisabled = isDisabled;

					// Parse the choice text
					uint32 textPos = pos + 4;
					while (textPos < dataSize) {
						byte tb = data[textPos];
						if (tb == CTRL_END_TEXT || tb == CTRL_DIALOGUE_MARKER ||
							tb == CTRL_DIALOGUE_MARKER_2 || tb == CTRL_END_BRANCH ||
							tb == CTRL_ALT_END_MARKER_1) {
							break;
						}

						if (b == 0x0A || b == 0x0B || b == 0x00) {
							debug("Skipping byte 0x%02X at pos %u", b, pos);
							pos++; // Skip nulls and line feeds
							continue;
						}

						if (tb >= 0x20 && tb <= 0x7A) {
							opt.text += (char)tb;
						} else {
							byte decoded = decodeChar(tb);
							if (decoded != tb || (decoded >= 0x20 && decoded <= 0x83)) {
								opt.text += (char)decoded;
							}
						}
						textPos++;
					}

					outChoices.push_back(opt);
					choiceCount++;
				} else if (choiceIndex < firstChoiceIndex) {
					// Different choice index - stop scanning
					break;
				}
			}
		}

		pos++;
	}

	return pos;
}

void DialogManager::startConversation(const byte *conversationData, uint32 dataSize, Sprite *animSet) {
	if (!conversationData || dataSize == 0) {
		debug("startConversation: No conversation data");
		return;
	}
	_curSprite = animSet;

	debug("Starting conversation with %u bytes of data", dataSize);

	uint32 position = 0;

	// Skip any junk at start until we find a speaker marker or choice marker
	while (position < dataSize &&
		   conversationData[position] != CTRL_SPEAKER_ID &&
		   conversationData[position] != CTRL_DIALOGUE_MARKER &&
		   conversationData[position] != CTRL_DIALOGUE_MARKER_2) {
		position++;
	}

	// OUTER LOOP: Continue until conversation ends
	while (position < dataSize && !g_engine->shouldQuit()) {
		// Skip control bytes that should be ignored
		while (position < dataSize &&
			   (conversationData[position] == CTRL_ALT_END_MARKER_1 ||
				conversationData[position] == CTRL_ALT_END_MARKER_2 ||
				conversationData[position] == CTRL_ALT_END_MARKER_3 ||
				conversationData[position] == CTRL_TEXT_TERMINATOR ||
				conversationData[position] == CTRL_GO_BACK)) {
			position++;
		}

		if (position >= dataSize) {
			debug("Reached end of data while skipping control bytes");
			break;
		}

		// 1. Read and display current dialogue
		Common::String text;
		byte speakerId;
		uint32 endPos = readTextBlock(conversationData, dataSize, position, text, speakerId);

		// Skip spurious single character artifacts
		if (!text.empty() && text.size() > 1) {
			debug("Dialogue: \"%s\" (Speaker ID: %u)", text.c_str(), speakerId);
			displayDialogue(text, speakerId);
		}

		// Move to end of text
		position = endPos;

		// 2. Check for end of conversation
		if (position >= dataSize) {
			debug("Reached end of data after reading dialogue");
			break;
		}

		byte controlByte = conversationData[position];

		if (controlByte == CTRL_END_CONVERSATION) {
			debug("End of conversation marker found");
			break;
		}

		// Move past control byte
		if (controlByte == CTRL_END_TEXT || controlByte == CTRL_ACTION_TRIGGER) {
			position++;
			if (position >= dataSize) {
				debug("Reached end of data after moving past control byte");
				break;
			}
		}

		// 3. Before parsing choices, check if we're at a choice marker
		// Skip control bytes to peek at next meaningful byte
		uint32 peekPos = position;
		while (peekPos < dataSize &&
			   (conversationData[peekPos] == CTRL_ALT_END_MARKER_1 ||
				conversationData[peekPos] == CTRL_ALT_END_MARKER_2 ||
				conversationData[peekPos] == CTRL_ALT_END_MARKER_3 ||
				conversationData[peekPos] == CTRL_TEXT_TERMINATOR ||
				conversationData[peekPos] == CTRL_GO_BACK)) {
			peekPos++;
		}

		// If not at a choice marker, there's more dialogue to read
		if (peekPos < dataSize &&
			conversationData[peekPos] != CTRL_DIALOGUE_MARKER &&
			conversationData[peekPos] != CTRL_DIALOGUE_MARKER_2 &&
			conversationData[peekPos] != CTRL_END_CONVERSATION) {
			continue;
		}

		// 4. Parse choices
		debug("Parsing choices at pos %u", position);
		Common::Array<ChoiceOption> choices;
		parseChoices(conversationData, dataSize, position, choices);
		debug("Parsed %u choices", choices.size());
		for (uint i = 0; i < choices.size(); i++) {
			debug(" Choice %u: \"%s\" (Disabled: %s)", i, choices[i].text.c_str(),
				  choices[i].isDisabled ? "Yes" : "No");
		}
		if (choices.empty()) {
			// No choices, continue reading dialogue
			position = peekPos;
			continue;
		}

		// 5. Display choices and get selection
		int selectedIndex = 0;

		// Check if this is auto-dialogue (only one choice)
		if (choices.size() == 1) {
			// Auto-dialogue: display it automatically
			displayDialogue(choices[0].text, ALFRED_COLOR);
			selectedIndex = 0;
		} else {
			// Real choice: show menu and wait for selection
			Common::Array<Common::String> choiceTexts;
			for (uint i = 0; i < choices.size(); i++) {
				if (choices[i].isDisabled) {
					choiceTexts.push_back("[DISABLED] " + choices[i].text);
				} else {
					choiceTexts.push_back(choices[i].text);
				}
			}
			_currentChoices = &choices;
			// Use displayChoices to show and select
			selectedIndex = selectChoice(choiceTexts, g_engine->_compositeBuffer);
		}

		// 6. Move position to after the selected choice
		if (selectedIndex >= 0 && selectedIndex < (int)choices.size()) {
			position = choices[selectedIndex].dataOffset;

			// Read and display the selected choice as dialogue
			Common::String choiceText;
			byte choiceSpeakerId;
			endPos = readTextBlock(conversationData, dataSize, position, choiceText, choiceSpeakerId);

			if (!choiceText.empty() && choiceText.size() > 1) {
				displayDialogue(choiceText, ALFRED_COLOR);
			}

			position = endPos;

			// Skip past end marker
			if (position < dataSize) {
				byte endByte = conversationData[position];
				if (endByte == CTRL_END_TEXT || endByte == CTRL_END_BRANCH ||
					endByte == CTRL_ACTION_TRIGGER) {
					position++;
				}
			}
		}
	}

	debug("Conversation ended");
	// Note: The caller should set inConversation = false after this returns
}
} // namespace Pelrock
