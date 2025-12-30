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

namespace Pelrock {

DialogManager::DialogManager(Graphics::Screen *screen, PelrockEventManager *events, GraphicsManager *graphics)
	: _screen(screen), _events(events), _graphics(graphics) {
}

DialogManager::~DialogManager() {
	if (_currentChoices) {
		delete _currentChoices;
		_currentChoices = nullptr;
	}
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

	// pos += 2; // Skip line count and blank
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

		// Regular text - does not need decoding
		if (b >= 0x20 && b <= 0x83) {
			outText += b;
		}
		pos++;
	}

	return pos;
}

void DialogManager::displayChoices(Common::Array<ChoiceOption> *choices, byte *compositeBuffer) {

	int overlayHeight = choices->size() * kChoiceHeight + 2;
	Common::Point overlayPos = _graphics->showOverlay(overlayHeight, compositeBuffer);
	for (int i = 0; i < choices->size(); i++) {
		ChoiceOption choice = (*choices)[i];
		int choicePadding = 32;
		int width = g_engine->_doubleSmallFont->getStringWidth(choice.text);
		Common::Rect bbox(0, overlayPos.y + i * kChoiceHeight, width + choicePadding * 2, overlayPos.y + i * kChoiceHeight + kChoiceHeight);
		int color = 14;
		if (bbox.contains(_events->_mouseX, _events->_mouseY)) {
			color = 15;
		}
		drawText(compositeBuffer, g_engine->_doubleSmallFont, choice.text, choicePadding, overlayPos.y + 2 + i * kChoiceHeight, 620, color);
	}
}

Graphics::Surface DialogManager::getDialogueSurface(Common::Array<Common::String> dialogueLines, byte speakerId) {

	int maxWidth = 0;
	int height = dialogueLines.size() * 24;
	for (int i = 0; i < dialogueLines.size(); i++) {
		maxWidth = MAX(maxWidth, g_engine->_largeFont->getStringWidth(dialogueLines[i]));
	}

	Graphics::Surface s;
	s.create(maxWidth, height, Graphics::PixelFormat::createFormatCLUT8());
	s.fillRect(s.getRect(), 255); // Clear surface

	for (int i = 0; i < dialogueLines.size(); i++) {

		int xPos = 0;
		int yPos = i * 25; // Above sprite, adjust for line
		g_engine->_largeFont->drawString(&s, dialogueLines[i], xPos, yPos, maxWidth, speakerId, Graphics::kTextAlignCenter);
	}

	return s;
}
/**
 * Display dialogue text and wait for click to advance
 * @param text The text to display
 * @param speakerId The speaker ID which is used as color
 */
void DialogManager::displayDialogue(Common::Array<Common::Array<Common::String>> dialogueLines, byte speakerId) {
	if (dialogueLines.empty()) {
		return;
	}

	// Clear any existing click state
	_events->_leftMouseClicked = false;
	int curPage = 0;
	// Render loop - display text and wait for click
	while (!g_engine->shouldQuit()) {
		_events->pollEvent();

		// Render the scene (keeps animations going)
		g_engine->renderScene(OVERLAY_NONE);

		// Draw the dialogue text on top using speaker ID as color
		Common::Array<Common::String> textLines = dialogueLines[curPage];

		int maxWidth = 0;
		int height = textLines.size() * 24;
		for (int i = 0; i < textLines.size(); i++) {
			maxWidth = MAX(maxWidth, g_engine->_largeFont->getStringWidth(textLines[i]));
		}
		int xPos = 0;
		int yPos = 0;

		if (speakerId == ALFRED_COLOR) {
			if(g_engine->alfredState.animState != ALFRED_TALKING) {
				g_engine->alfredState.setState(ALFRED_TALKING);
			}
			if (_curSprite != nullptr) {
				_curSprite->isTalking = false;
			}
			// Offset X position for Alfred to avoid overlapping with his sprite
			xPos = g_engine->alfredState.x - maxWidth / 2; //+ kAlfredFrameWidth / 2 - maxWidth / 2;
			yPos = g_engine->alfredState.y - kAlfredFrameHeight - height; // Above sprite, adjust for line
		} else {
			g_engine->alfredState.setState(ALFRED_IDLE);
			_curSprite->isTalking = true;
			xPos = _curSprite->x + _curSprite->w / 2;
			yPos = _curSprite->y - height; // Above sprite, adjust for line
		}

		Graphics::Surface s = getDialogueSurface(textLines, speakerId);

		if (xPos + s.getRect().width() > 640) {
			xPos = 640 - s.getRect().width();
		}
		if (yPos + s.getRect().height() > 400) {
			yPos = 400 - s.getRect().height();
		}
		if (xPos < 0) {
			xPos = 0;
		}
		if (yPos < 0) {
			yPos = 0;
		}
		_screen->transBlitFrom(s, s.getRect(), Common::Point(xPos, yPos), 255);
		drawPos(_screen, xPos, yPos, speakerId);
		drawRect(_screen, xPos, yPos,
			s.getRect().width(),
			s.getRect().height(), speakerId);
		// Present to screen
		_screen->markAllDirty();
		_screen->update();

		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;
			debug("Dialogue click to advance, current page: %d, totalPages: %d", curPage, (int)dialogueLines.size());
			if (curPage < (int)dialogueLines.size() - 1) {
				curPage++;
			} else {
				break; // Exit dialogue on last page click
			}
		}
		g_system->delayMillis(10);
	}
	if (_curSprite != nullptr) {
		_curSprite->isTalking = false;
	}
	g_engine->alfredState.setState(ALFRED_IDLE);
}

void DialogManager::displayDialogue(Common::String text, byte speakerId) {
	displayDialogue(wordWrap(text), speakerId);
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
		g_engine->renderScene(OVERLAY_CHOICES);

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
uint32 DialogManager::parseChoices(const byte *data, uint32 dataSize, uint32 startPos, Common::Array<ChoiceOption> *outChoices) {
	uint32 pos = startPos;
	outChoices->clear();
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
				textPos += 2;
				while (textPos < dataSize) {
					byte tb = data[textPos];
					if (tb == CTRL_END_TEXT || tb == CTRL_DIALOGUE_MARKER ||
						tb == CTRL_DIALOGUE_MARKER_2 || tb == CTRL_END_BRANCH ||
						tb == CTRL_ALT_END_MARKER_1) {
						break;
					}

					if (tb >= 0x20 && tb < 0x7A) {
						opt.text += (char)tb;
					} else {
						byte decoded = decodeChar(tb);
						debug("Parsing choice char: 0x%02X, decoded: 0x%02X", tb, decoded);
						if (decoded != tb || (decoded >= 0x20 && decoded <= 0xB4)) {
							opt.text += (char)decoded;
						}
					}
					textPos++;
				}

				outChoices->push_back(opt);
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
					textPos += 2; // Skip marker + index + 2 speaker bytes
					while (textPos < dataSize) {
						byte tb = data[textPos];
						if (tb == CTRL_END_TEXT || tb == CTRL_DIALOGUE_MARKER ||
							tb == CTRL_DIALOGUE_MARKER_2 || tb == CTRL_END_BRANCH ||
							tb == CTRL_ALT_END_MARKER_1) {
							break;
						}

						if (tb >= 0x20 && tb <= 0x7A) {
							opt.text += (char)tb;
						} else {
							byte decoded = decodeChar(tb);
							debug("Parsing choice char: 0x%02X, decoded: 0x%02X", tb, decoded);
							if (decoded != tb || (decoded >= 0x20 && decoded <= 0xB4)) {
								opt.text += (char)decoded;
							}
						}
						textPos++;
					}

					outChoices->push_back(opt);
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

void DialogManager::setCurSprite(int index) {
	// Set current sprite based on index
	if (g_engine->_room == nullptr) {
		_curSprite = nullptr;
		return;
	}

	for (uint i = 0; i < g_engine->_room->_currentRoomAnims.size(); i++) {
		Sprite *sprite = &g_engine->_room->_currentRoomAnims[i];
		if (sprite->index == index) {
			_curSprite = sprite;
			return;
		}
	}

	_curSprite = nullptr;
}

void DialogManager::startConversation(const byte *conversationData, uint32 dataSize, Sprite *animSet) {
	if (!conversationData || dataSize == 0) {
		debug("startConversation: No conversation data");
		return;
	}
	setCurSprite(animSet ? animSet->index : -1);
	// _curSprite = animSet;

	debug("Starting conversation with %u bytes of data", dataSize);

	uint32 position = 0;
	int currentChoiceLevel = -1; // Track the current choice level

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
		Common::Array<Common::Array<Common::String>> wrappedText = wordWrap(text);
		debug("Word wrapping %s produces %d pages", text.c_str(), wrappedText.size());
		// Skip spurious single character artifacts
		if (!text.empty() && text.size() > 1) {
			displayDialogue(wrappedText, speakerId);
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
		Common::Array<ChoiceOption> *choices = new Common::Array<ChoiceOption>();
		parseChoices(conversationData, dataSize, position, choices);
		debug("Parsed %u choices", choices->size());
		for (uint i = 0; i < choices->size(); i++) {
			debug(" Choice %u (index %d): \"%s\" (Disabled: %s)", i, (*choices)[i].index, (*choices)[i].text.c_str(),
				  (*choices)[i].isDisabled ? "Yes" : "No");
		}
		if (choices->empty()) {
			// No choices, continue reading dialogue
			position = peekPos;
			continue;
		}

		// Check if we have a currentChoiceLevel and if these choices are at the next level
		if (currentChoiceLevel >= 0) {
			// We've already made a choice, check if the current choices are at the next level
			bool foundNextLevel = false;
			for (uint i = 0; i < choices->size(); i++) {
				if ((*choices)[i].index == currentChoiceLevel + 1) {
					foundNextLevel = true;
					break;
				}
			}

			if (!foundNextLevel) {
				debug("No choices found at level %d (current is %d), ending conversation", currentChoiceLevel + 1, currentChoiceLevel);
				break;
			}
		}

		// 5. Display choices and get selection
		int selectedIndex = 0;

		// Check if this is auto-dialogue (only one choice)
		if (choices->size() == 1) {
			// Auto-dialogue: display it automatically
			debug("Auto-selecting single choice: \"%s\"", (*choices)[0].text.c_str());
			selectedIndex = 0;
		} else {
			// Real choice: show menu and wait for selection
			Common::Array<Common::String> choiceTexts;
			for (uint i = 0; i < choices->size(); i++) {
				if ((*choices)[i].isDisabled) {
					choiceTexts.push_back("[DISABLED] " + (*choices)[i].text);
				} else {
					choiceTexts.push_back((*choices)[i].text);
				}
			}

			if (_currentChoices) {
				delete _currentChoices;
				_currentChoices = nullptr;
			}
			_currentChoices = choices;
			// Use displayChoices to show and select
			selectedIndex = selectChoice(choiceTexts, g_engine->_compositeBuffer);
		}

		// 6. Move position to after the selected choice
		if (selectedIndex >= 0 && selectedIndex < (int)choices->size()) {
			position = (*choices)[selectedIndex].dataOffset;
			currentChoiceLevel = (*choices)[selectedIndex].index;

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

void DialogManager::sayAlfred(Common::StringArray texts) {
	g_engine->alfredState.setState(ALFRED_TALKING);

	_curSprite = nullptr;
	Common::Array<Common::StringArray> textLines = wordWrap(texts);
	displayDialogue(textLines, ALFRED_COLOR);
}

void DialogManager::sayAlfred(Description description) {
	Common::StringArray texts;

	texts.push_back(description.text);
	sayAlfred(texts);
	if (description.isAction) {
		g_engine->performActionTrigger(description.actionTrigger);
	}
}

void DialogManager::say(Common::StringArray texts) {
	if (texts.empty()) {
		return;
	}
	byte speakerId;
	bool wasProcessed = processColorAndTrim(texts, speakerId);

	if (wasProcessed) {
		if (speakerId == ALFRED_COLOR) {
			sayAlfred(texts);
			return;
		} else {
			setCurSprite(0);
			Common::Array<Common::StringArray> textLines = wordWrap(texts);
			displayDialogue(textLines, speakerId);
		}
	} else {
		sayAlfred(texts);
	}
}

bool DialogManager::processColorAndTrim(Common::StringArray &lines, byte &speakerId) {
	int speakerMarker = lines[0][0];
	speakerId = lines[0][1];

	if (speakerMarker == '@') {

		for (int i = 0; i < lines.size(); i++) {
			// Remove first two marker bytes
			if (lines[i].size() > 2) {
				lines[i] = lines[i].substr(2);

				if (lines[i][0] == 0x78 && lines[i][1] == 0x78) { // Remove additional control chars
					lines[i] = lines[i].substr(2);
				}
			} else {
				lines[i] = "";
			}
		}
		return true;
	}
	return false;
}

bool isEndMarker(char char_byte) {
	return char_byte == CHAR_END_MARKER_1 || char_byte == CHAR_END_MARKER_2 || char_byte == CHAR_END_MARKER_3 || char_byte == CHAR_END_MARKER_4;
}

int calculateWordLength(Common::String text, int startPos, bool &isEnd) {
	int wordLength = 0;
	int pos = startPos;
	while (pos < text.size()) {
		char char_byte = text[pos];
		if (char_byte == CHAR_SPACE || isEndMarker(char_byte)) {
			break;
		}
		wordLength++;
		pos++;
	}
	// Check if we hit an end marker
	if (pos < text.size() && isEndMarker(text[pos])) {
		isEnd = true;
	}
	// Count ALL trailing spaces as part of this word
	if (pos < text.size() && !isEnd) {
		if (text[pos] == CHAR_END_MARKER_3) { // 0xF8 (-8) special case
			wordLength += 3;
		} else {
			// Count all consecutive spaces
			while (pos < text.size() && text[pos] == CHAR_SPACE) {
				wordLength++;
				pos++;
			}
		}
	}
	return wordLength;
}

Common::Array<Common::Array<Common::String>> DialogManager::wordWrap(Common::String text) {

	Common::Array<Common::Array<Common::String>> pages;
	Common::Array<Common::String> currentPage;
	Common::Array<Common::String> currentLine;
	int charsRemaining = MAX_CHARS_PER_LINE;
	int position = 0;
	int currentLineNum = 0;
	while (position < text.size()) {
		bool isEnd = false;
		int wordLength = calculateWordLength(text, position, isEnd);
		// # Extract the word (including trailing spaces)
		Common::String word = text.substr(position, wordLength);
		// # Key decision: if word_length > chars_remaining, wrap to next line
		if (wordLength > charsRemaining) {
			// Word is longer than the entire line - need to split
			currentPage.push_back(joinStrings(currentLine, ""));
			currentLine.clear();
			charsRemaining = MAX_CHARS_PER_LINE;
			currentLineNum++;

			if (currentLineNum >= MAX_LINES) {
				pages.push_back(currentPage);
				currentPage.clear();
				currentLineNum = 0;
			}
		}
		// Add word to current line
		currentLine.push_back(word);
		charsRemaining -= wordLength;

		if (charsRemaining == 0 && isEnd) {
			Common::String lineText = joinStrings(currentLine, "");
			while (lineText.lastChar() == CHAR_SPACE) {
				lineText = lineText.substr(0, lineText.size() - 1);
			}
			int trailingSpaces = currentLine.size() - lineText.size();
			if (trailingSpaces > 0) {
				currentPage.push_back(lineText);
				//  current_line = [' ' * trailing_spaces]
				Common::String currentLine(trailingSpaces, ' ');
				charsRemaining = MAX_CHARS_PER_LINE - trailingSpaces;
				currentLineNum += 1;

				if (currentLineNum >= MAX_LINES) {
					pages.push_back(currentPage);
					currentPage.clear();
					currentLineNum = 0;
				}
			}
		}

		position += wordLength;
		if (isEnd) {
			// End of sentence/paragraph/page
			break;
		}
	}
	if (currentLine.empty() == false) {
		Common::String lineText = joinStrings(currentLine, "");
		while (lineText.lastChar() == CHAR_SPACE) {
			lineText = lineText.substr(0, lineText.size() - 1);
		}
		currentPage.push_back(lineText);
	}
	if (currentPage.empty() == false) {
		pages.push_back(currentPage);
	}
	for (int i = 0; i < pages.size(); i++) {
		for (int j = 0; j < pages[i].size(); j++) {
		}
	}
	return pages;
}

Common::Array<Common::Array<Common::String>> DialogManager::wordWrap(Common::StringArray texts) {
	Common::Array<Common::Array<Common::String>> allWrappedLines;
	for (int i = 0; i < texts.size(); i++) {
		Common::Array<Common::Array<Common::String>> wrapped = wordWrap(texts[i]);
		for (int j = 0; j < wrapped.size(); j++) {
			allWrappedLines.push_back(wrapped[j]);
		}
	}
	return allWrappedLines;
}

} // namespace Pelrock
