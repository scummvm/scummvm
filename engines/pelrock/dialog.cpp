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
#include "common/stack.h"

#include "dialog.h"
#include "pelrock/dialog.h"
#include "pelrock/offsets.h"
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
	outSpeakerId = kAlfredColor; // Default to Alfred's color
	outText = "";

	// Skip control bytes at start
	if (data[pos] == kCtrlTextTerminator) {
		pos += 2;
	}

	if (pos >= dataSize) {
		return pos;
	}

	// Check for speaker ID marker
	if (data[pos] == kCtrlSpeakerId) {
		pos++;
		if (pos < dataSize) {
			outSpeakerId = data[pos];
		}
	}
	// Check for dialogue marker (choice text)
	else if (data[pos] == kCtrlDialogueMarker || data[pos] == kCtrlDialogueMarkerOneoff) {
		pos++; // Skip marker

		// Skip choice index
		if (pos < dataSize) {
			pos++;
		}
		// Choice text is always spoken by ALFRED
		outSpeakerId = kAlfredColor;
		pos += 2;
	}


	// Line index could be useful for translations
	/*int lineIndex =  data[++pos]; */
	pos++; // Skip line index
	pos++; // blank
	// Read text until control byte
	while (pos < dataSize) {
		byte b = data[pos];

		// End markers - stop reading text
		if (b == kCtrlEndText || b == kCtrlEndConversation || b == kCtrlActionAndEnd ||
			b == kCtrlEndBranch || b == kCtrlDialogueMarker || b == kCtrlDialogueMarkerOneoff ||
			b == kCtrlTextTerminator || b == kCtrlAltEndMarker1 || b == kCtrlActionAndContinue ||
			b == kCtrlGoBack || b == kCtrlSpeakerId) {
			break;
		}

		if (b == kCtrlLineContinue || b == kCtrlPageBreakConv) {
			warning("Found unexpected line/page break control code in readTextBlock at pos %u", pos);
			outText += ' ';
			pos++;
			continue;
		}

		// Regular text - does not need decoding
		if (b >= kCtrlSpace && b <= 0x83) {
			outText += b;
		}
		pos++;
	}

	return pos;
}

void DialogManager::displayChoices(Common::Array<ChoiceOption> *choices, Graphics::ManagedSurface &compositeBuffer) {

	int overlayHeight = choices->size() * kChoiceHeight + 2;
	// Grab the overlay position to start drawing the choices, and draw the choices there
	Common::Point overlayPos = _graphics->showOverlay(overlayHeight, compositeBuffer);
	for (uint i = 0; i < choices->size(); i++) {
		ChoiceOption choice = (*choices)[i];
		int yPos = overlayPos.y + 2 + i * kChoiceHeight;
		Common::Rect bbox(kChoicePadding, yPos, kChoicePadding + 600, yPos + kChoiceHeight);
		Common::Rect leftArrowBox(0, yPos, kChoicePadding, yPos + kChoiceHeight);
		Common::Rect rightArrowBox(640 - kChoicePadding, yPos, 640, yPos + kChoiceHeight);
		int choiceColor = 14;
		int lArrowColor = 14;
		int rArrowColor = 14;

		if (bbox.contains(_events->_mouseX, _events->_mouseY)) {
			choiceColor = 15;
		} else if (leftArrowBox.contains(_events->_mouseX, _events->_mouseY)) { // scroll left
			if (choice.charOffset > 0) {
				choice.charOffset--;
				choices->remove_at(i);
				choices->insert_at(i, choice);
			}
			lArrowColor = 15;
		} else if (rightArrowBox.contains(_events->_mouseX, _events->_mouseY)) { // scroll right
			if (choice.charOffset + 76 < choice.text.size()) {
				choice.charOffset = choice.charOffset + 1;
				choices->remove_at(i);
				choices->insert_at(i, choice);
			}
			rArrowColor = 15;
		}

		if (choice.charOffset > 0) {
			//draw left arrow
			drawText(compositeBuffer, g_engine->_doubleSmallFont, _leftArrow, 0, yPos, g_engine->_doubleSmallFont->getCharWidth(17), lArrowColor);
		}
		drawText(compositeBuffer, g_engine->_doubleSmallFont, choice.text.substr(choice.charOffset, 76), kChoicePadding, yPos, 620, choiceColor);

		if (choice.charOffset + 76 < choice.text.size()) {
			//draw right arrow
			drawText(compositeBuffer, g_engine->_doubleSmallFont, _rightArrow, 640 - kArrowWidth, yPos, g_engine->_doubleSmallFont->getCharWidth(16), rArrowColor);
		}
	}
}

/**
 * In order to display multi-line text whilst still centered we create a surface with
 * the maxWidth + height then print the text onto that surface with the appropriate alignment,
 * then blit that surface to the screen.
 */
Graphics::Surface DialogManager::getDialogueSurface(Common::Array<Common::String> dialogueLines, byte speakerId, Graphics::TextAlign alignment) {

	int maxWidth = 0;
	int height = dialogueLines.size() * 25; // Add some padding
	for (uint i = 0; i < dialogueLines.size(); i++) {
		maxWidth = MAX(maxWidth, g_engine->_largeFont->getStringWidth(dialogueLines[i]));
	}

	Graphics::Surface s;
	s.create(maxWidth + 1, height + 1, Graphics::PixelFormat::createFormatCLUT8());
	s.fillRect(s.getRect(), 255); // Clear surface

	for (uint i = 0; i < dialogueLines.size(); i++) {

		int xPos = 0;
		int yPos = i * 25; // Above sprite, adjust for line
		// debug("Drawing dialogue line %d: \"%s\" at position (%d, %d) with speaker ID %d", i, dialogueLines[i].c_str(), xPos, yPos, speakerId);
		g_engine->_largeFont->drawString(&s, dialogueLines[i], xPos, yPos, maxWidth, speakerId, alignment);
	}

	return s;
}

void DialogManager::displayDialogue(Common::Array<Common::Array<Common::String>> dialogueLines, byte speakerId) {
	int16 xBasePos = 0;
	int16 yBasePos = 0;
	if (speakerId == kAlfredColor) {
		if (g_engine->_state->getFlag(FLAG_FROM_INTRO) == true) {
			// Different talking animation for the post-intro sequence in which Alfred speaks in bed
			g_engine->_alfredState.setState(ALFRED_SPECIAL_ANIM);
		} else {
			g_engine->_alfredState.setState(ALFRED_TALKING);
		}
		if (_curSprite != nullptr) {
			_curSprite->isTalking = false;
		}

		xBasePos = g_engine->_alfredState.x;
		yBasePos = g_engine->_alfredState.y - g_engine->_alfredState.h; // Above scaled sprite top
	} else {
		g_engine->_alfredState.setState(ALFRED_IDLE);
		if (_curSprite != nullptr) {
			_curSprite->isTalking = true;
			xBasePos = _curSprite->x + _curSprite->w / 2;
			yBasePos = _curSprite->y; // Above sprite, adjust for line

			// Set NPC talk speed byte for original timing.
			TalkingAnims *th = &g_engine->_room->_talkingAnimHeader;
			g_engine->_npcTalkSpeedByte = _curSprite->talkingAnimIndex ? th->speedByteB : th->speedByteA;
		}
	}
	displayDialogue(dialogueLines, speakerId, xBasePos, yBasePos); // Default position
}

/**
 * Simply wait tick period * char count
 */
uint32 calcPageTtlMs(Common::Array<Common::String> dialogueLine) {
    uint32 charCount = 0;
    for (uint i = 0; i < dialogueLine.size(); i++) {
        charCount += dialogueLine[i].size();
    }
    return charCount * kTickMs;
}

/**
 * Display dialogue text and wait for click or TTL to advance.
 *
 * TTL ->  Each page auto-advances after (char_count * kTickMs) milliseconds.
 * Where char_count = total visible characters on the page.
 *
 * Skip -> All dialogs are click-skippable unless _disableClickToAdvance flag is enabled; those
 * dialogs still auto-advance via TTL.
 */
void DialogManager::displayDialogue(Common::Array<Common::Array<Common::String>> dialogueLines, byte speakerId, int xBasePos, int yBasePos) {
	if (dialogueLines.empty()) {
		return;
	}
	_dismissDialog = false;
	// Clear any existing click state
	_events->_leftMouseClicked = false;
	_dialogActive = true;
	int curPage = 0;
	bool fromIntro = g_engine->_state->getFlag(FLAG_FROM_INTRO) == true;

	uint32 pageTtlMs = calcPageTtlMs(dialogueLines[curPage]);
	uint32 pageStartMs = g_system->getMillis();

	if(speakerId != kAlfredColor) {
		_isNPCTalking = true;
	}
	// Render loop - display text and wait for click or TTL
	while (!g_engine->shouldQuit()) {
		_events->pollEvent();

		// Render the scene (keeps animations going)
		g_engine->renderScene(OVERLAY_NONE);

		// Draw the dialogue text on top using speaker ID as color
		Common::Array<Common::String> textLines = dialogueLines[curPage];

		int maxWidth = 0;
		int height = textLines.size() * 24;
		for (uint i = 0; i < textLines.size(); i++) {
			maxWidth = MAX(maxWidth, g_engine->_largeFont->getStringWidth(textLines[i]));
		}

		int xPos = xBasePos - maxWidth / 2;
		int yPos = yBasePos - height;

		Graphics::Surface s = getDialogueSurface(textLines, speakerId);

		// Clamp to screen bounds (original game: min Y = 1, max X = 639 - width)
		xPos = CLIP(xPos, 0, 639 - maxWidth);
		yPos = CLIP(yPos, 1, 400 - (int)s.getRect().height());

		if (g_engine->_shakeEffectState.enabled) {
			debug("Applying shake effect to dialogue, shakeX: %d", g_engine->_shakeEffectState.shakeX);
			xPos -= g_engine->_shakeEffectState.shakeX;
		}

		_screen->transBlitFrom(s, s.getRect(), Common::Point(xPos, yPos), 255);
		// drawPos(_screen, xPos, yPos, speakerId);

		_screen->markAllDirty();
		_screen->update();

		// Check if TTL expired for this page (always applies, even for _disableClickToAdvance)
		bool ttlExpired = !fromIntro && (pageTtlMs > 0) && (g_system->getMillis() - pageStartMs >= pageTtlMs);

		// Click-to-advance (disabled for special intro sequences)
		bool clickAdvance = _events->_leftMouseClicked && !_disableClickToAdvance;
		if (_events->_leftMouseClicked)
			_events->_leftMouseClicked = false;

		if (clickAdvance || ttlExpired) {
			if (curPage < (int)dialogueLines.size() - 1) {
				curPage++;
				pageStartMs = g_system->getMillis();
				pageTtlMs = calcPageTtlMs(dialogueLines[curPage]);
			} else {
				_dismissDialog = true;
			}
		}

		if (_dismissDialog) {
			_dismissDialog = false;
			break; // Exit dialogue if dismissed programmatically
		}

		if (fromIntro && g_engine->_res->_isSpecialAnimFinished) {
			// in post-intro, text stops only after the animation is done!
			break;
		}

		g_system->delayMillis(10);
	}
	if (_curSprite != nullptr) {
		_curSprite->isTalking = false;
	}
	_dialogActive = false;
	_isNPCTalking = false;
	g_engine->_alfredState.setState(ALFRED_IDLE);
}

void DialogManager::displayDialogue(Common::String text, byte speakerId) {
	displayDialogue(wordWrap(text), speakerId);
}

/**
 * Select a choice from displayed options
 * Returns the index of the selected choice in the choices array
 */
int DialogManager::selectChoice(Common::Array<Common::String> &choices, Graphics::ManagedSurface &compositeBuffer) {
	_events->_leftMouseClicked = false;
	_dialogActive = true;
	g_engine->_chrono->pauseCounter();

	int overlayHeight = choices.size() * kChoiceHeight + 2;
	int overlayY = 400 - overlayHeight;

	while (!g_engine->shouldQuit()) {
		_events->pollEvent();

		g_engine->renderScene(OVERLAY_CHOICES);

		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;

			if (_events->_mouseClickY >= overlayY) {
				int selectedIndex = (_events->_mouseClickY - overlayY - 2) / kChoiceHeight;
				if (selectedIndex >= 0 && selectedIndex < (int)choices.size()) {
					_dialogActive = false;
					g_engine->_chrono->resumeCounter();
					return selectedIndex;
				}
			}
		}
		g_engine->_screen->update();
		g_system->delayMillis(10);
	}

	_dialogActive = false;
	g_engine->_chrono->resumeCounter();
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

	// Scan for choices with SAME index
	// Choices with the same index are not contiguous; for each choice the entire branch is laid out first.
	// So we must scan past higher-index choices to find all choices at our level, but stop when we hit a LOWER index.
	while (pos < dataSize) {
		byte b = data[pos];

		// Stop at end markers
		if (b == kCtrlAltEndMarker1 || b == kCtrlEndBranch || b == kCtrlAltSpeakerRoot) {
			break;
		}

		// Found a dialogue marker
		if (b == kCtrlDialogueMarker || b == kCtrlDialogueMarkerOneoff) {
			if (pos + 1 < dataSize) {
				int choiceIndex = data[pos + 1];

				// Set firstChoiceIndex from first non-disabled choice, or first choice if all disabled
				if (firstChoiceIndex == -1) {
					firstChoiceIndex = choiceIndex;
				}

				// Only collect choices with same index as the first one we found
				if (choiceIndex == firstChoiceIndex) {
					ChoiceOption opt;
					opt.room = g_engine->_room->_currentRoomNumber;
					opt.shouldDisableOnSelect = b == kCtrlDialogueMarkerOneoff;
					opt.choiceIndex = choiceIndex;
					opt.dataOffset = pos;
					pos += 2; // Move past marker + index
					if (data[pos] == kCtrlDisabledChoice) {
						opt.isDisabled = true;
					}
					// Parse the choice text
					uint32 textPos = pos + 4;
					while (textPos < dataSize) {
						byte tb = data[textPos];
						if (tb == kCtrlEndText || tb == kCtrlDialogueMarker ||
							tb == kCtrlDialogueMarkerOneoff || tb == kCtrlEndBranch ||
							tb == kCtrlAltEndMarker1) {
							// Check if there is a terminator (F4 or F8) at the end of this choice's response
							// Scan forward but stop at another choice marker or branch end
							uint32 scanPos = textPos;
							while (scanPos < dataSize) {
								byte sb = data[scanPos];
								// Stop scanning at another choice marker or branch boundaries
								if (sb == kCtrlDialogueMarker || sb == kCtrlDialogueMarkerOneoff ||
									sb == kCtrlEndBranch || sb == kCtrlAltEndMarker1 ||
									sb == kCtrlAltSpeakerRoot) {
									break;
								}
								// Found a conversation terminator - this choice ends the conversation
								if (sb == kCtrlEndConversation || sb == kCtrlActionAndEnd) {
									opt.hasConversationEndMarker = true;
									break;
								}
								scanPos++;
							}
							break;
						}

						if (tb >= 0x20 && tb <= 0x7A) {
							opt.text += (char)tb;
						} else {
							byte decoded = decodeChar(tb);
							if (decoded != tb || (decoded >= 0x20 && decoded <= 0xB4)) {
								opt.text += (char)decoded;
							}
						}
						textPos++;
					}
					if (!opt.isDisabled) {
						opt.text = "  " + opt.text; // Adds three spaces for padding to allow for scrolling past the indentation
						outChoices->push_back(opt);
					}
				} else if (choiceIndex < firstChoiceIndex) {
					// Hit a choice at a LOWER level - stop scanning
					// This means we've gone past all choices at our level
					break;
				}
			}
		}

		pos++;
	}

	return pos;
}

/**
 * Check if all sub-branches of the current choice level are exhausted to mark the choice for disabling.
 *
 * Returns true if we should disable the current choice, which happens when:
 * - There are no FB sub-branches at higher indices, OR
 * - All FB sub-branches at higher indices already have FA (are disabled)
 *
 * F1 markers (repeatable) never get disabled.
 */
bool DialogManager::checkAllSubBranchesExhausted(const byte *data, uint32 dataSize, uint32 startPos, int currentChoiceLevel) {
	uint32 pos = startPos;

	// Fix for room 26 where an endless loop will happen due to buggy command codes that casued the conversation
	// to go back with a single choice and thus repeating over and over.
	int room = g_engine->_room->_currentRoomNumber;
	bool f0IsBoundary = (room == 26 && currentChoiceLevel == 2 && startPos < dataSize && data[startPos] == 0x0C);

	while (pos < dataSize) {
		byte b = data[pos];

		// Stop at TRUE branch boundary markers (0xF5, 0xF7, 0xFE)
		// NOTE: Do NOT stop at F4 (CTRL_END_CONVERSATION) - F4 markers appear between
		// choices as terminators for each choice's response path. We need to scan
		// past them to find all choices at the target level.
		if (b == kCtrlAltEndMarker1 || b == kCtrlEndBranch ||
			b == kCtrlAltSpeakerRoot) {
			break;
		}

		// For that one bug in room 26
		// treat F0 as a boundary to prevent scanning past unreachable choices
		if (b == kCtrlGoBack && f0IsBoundary) {
			break;
		}

		// Found FB (one-time choice marker)
		if (b == kCtrlDialogueMarkerOneoff && pos + 2 < dataSize) {
			byte choiceIdx = data[pos + 1];

			// Only check sub-branches (higher index = deeper level)
			if (choiceIdx > currentChoiceLevel) {
				// Check if NOT disabled (no FA at pos+2)
				if (data[pos + 2] != kCtrlDisabledChoice) {
					return false; // Don't disable parent
				}
			} else if (choiceIdx <= currentChoiceLevel) {
				// Hit choice at same or lower level - stop scanning
				// All choices at higher levels before this point have been checked
				break;
			}
		}

		pos++;
	}

	return true;
}

void DialogManager::setCurSprite(int index) {
	// Set current sprite based on index
	if (g_engine->_room == nullptr) {
		_curSprite = nullptr;
		return;
	}

	for (uint i = 0; i < g_engine->_room->_currentRoomAnims.size(); i++) {
		Sprite *sprite = &g_engine->_room->_currentRoomAnims[i];
		if (sprite->index == index && (sprite->actionFlags & 16)) {
			_curSprite = sprite;
			return;
		}
	}

	_curSprite = nullptr;
}

void DialogManager::startConversation(const byte *conversationData, uint32 dataSize, byte npcIndex, Sprite *animSet) {
	if (!conversationData || dataSize == 0) {
		warning("startConversation: No conversation data");
		return;
	}
	setCurSprite(animSet ? animSet->index : -1);

	// Initialize conversation state
	ConversationState state = initializeConversation(conversationData, dataSize, npcIndex);
	bool skipToChoices = false;
	Common::Stack<uint32> positionStack; // Stack to handle nested branches for "go back" functionality

	// Main conversation loop
	while (state.position < dataSize && !g_engine->shouldQuit()) {
		state.position = skipControlBytes(conversationData, dataSize, state.position);

		if (state.position < dataSize && conversationData[state.position] == kCtrlGoBack) {
			if (handleGoBack(conversationData, positionStack, state.position, state)) {
				skipToChoices = true;
			} else {
				break; // End conversation if no previous menu
			}
		}

		if (state.position >= dataSize) {
			debug("Reached end of data while skipping control bytes");
			break;
		}

		if (!skipToChoices) {
			state.position = readAndDisplayDialogue(conversationData, dataSize, state.position);
		}

		if (!skipToChoices) {
			ConversationEndResult endResult = checkConversationEnd(conversationData, dataSize, state.position, state.currentRoot);
			// Dispatch action for both 0xF8 (action+end) and 0xEB (action+continue)
			if (endResult.hasAction) {
				g_engine->dialogActionTrigger(endResult.actionCode, g_engine->_room->_currentRoomNumber, state.currentRoot);
			}
			if (endResult.shouldEnd) {
				break;
			}
			state.position = endResult.nextPosition;
		}
		// Check for choice markers
		uint32 peekPos = state.position;
		if (!skipToChoices) {
			peekPos = peekNextMeaningfulByte(conversationData, dataSize, state.position);

			// If not at a choice marker, there's more dialogue to read
			if (peekPos < dataSize &&
				conversationData[peekPos] != kCtrlDialogueMarker &&
				conversationData[peekPos] != kCtrlDialogueMarkerOneoff &&
				conversationData[peekPos] != kCtrlEndConversation &&
				conversationData[peekPos] != kCtrlDisabledChoice) {
				continue;
			}
		}

		// Parse choices
		skipToChoices = false;
		uint32 positionAtChoices = state.position;

		Common::Array<ChoiceOption> *choices = new Common::Array<ChoiceOption>();
		parseChoices(conversationData, dataSize, state.position, choices);

		uint originalChoiceCount = choices->size();
		addGoodbyeOptionIfNeeded(choices, state.currentChoiceLevel, originalChoiceCount);

		if (choices->empty()) {
			state.position = positionStack.empty() ? 0 : positionStack.pop();
			if (state.position == 0) {
				// No choices and no previous position to go back to, ending conversation
				break;
			}
			checkAllSubBranchesExhausted(conversationData, dataSize, state.position, state.currentChoiceLevel - 1);
			// No choices found, popping back to previous choice menu, position %u
			skipToChoices = true;
			continue;
		}

		positionStack.push(positionAtChoices); // Push position of this choice menu onto stack for potential "go back"

		// Validate choice level
		if (state.currentChoiceLevel >= 0) {
			bool foundExpectedLevel = false;
			for (uint i = 0; i < choices->size(); i++) {
				if ((*choices)[i].choiceIndex == state.currentChoiceLevel ||
					(*choices)[i].choiceIndex == state.currentChoiceLevel + 1) {
					foundExpectedLevel = true;
					state.currentChoiceLevel = (*choices)[i].choiceIndex;
					break;
				}
			}

			if (!foundExpectedLevel) {
				break;
			}
		}

		// Process user selection
		int selectedIndex = 0;
		if (choices->size() == 1) {
			// Auto-dialogue: display it automatically if ony one choice!
			selectedIndex = 0;
		} else {
			// Real choice: show menu and wait for selection
			Common::Array<Common::String> choiceTexts;
			for (uint i = 0; i < choices->size(); i++) {
				choiceTexts.push_back((*choices)[i].text);
			}

			if (_currentChoices) {
				delete _currentChoices;
				_currentChoices = nullptr;
			}
			_currentChoices = choices;
			selectedIndex = selectChoice(choiceTexts, g_engine->_compositeBuffer);
		}

		state.position = processChoiceSelection(conversationData, dataSize, choices, selectedIndex, state);
	}

	debug("Conversation ended");
}

/**
 * Finds a conversation root for a given NPC
 */
uint32 DialogManager::findRoot(int npc, int &currentRoot, uint32 currentPosition, uint32 dataSize, const byte *conversationData) {
	// Check if a specific root has been set for this room
	int targetRoot = g_engine->_state->getCurrentRoot(g_engine->_room->_currentRoomNumber, npc);

	if (targetRoot >= 0) {
		// Skip to the specified root
		while (currentRoot < targetRoot && currentPosition < dataSize) {
			if (conversationData[currentPosition] == kCtrlEndBranch) {
				currentPosition++; // Move past end branch marker
				currentRoot++;
			} else {
				currentPosition++;
			}
		}
	}
	// If targetRoot is -1 or not set, use the first root (default behavior)
	return currentPosition;
}

/**
 * Find the tree for the given NPC.
 */
uint32 DialogManager::findSpeaker(byte npcIndex, uint32 dataSize, const byte *conversationData) {
	// Find the speaker tree for this NPC; they are marked by 0xFE 0xXX where XX is NPC index + 1
	bool speakerTreeOffsetFound = false;
	int currentConversationTree = npcIndex + 1;
	uint32 position = 0;
	while (position < dataSize && !speakerTreeOffsetFound) {
		if (conversationData[position] == kCtrlAltSpeakerRoot && conversationData[position + 1] == currentConversationTree) {
			speakerTreeOffsetFound = true;
			position += 2; // Move past the speaker tree marker and npc index
		} else {
			position++;
		}
	}
	return position;
}

// Skip control bytes that should be ignored
uint32 DialogManager::skipControlBytes(const byte *data, uint32 dataSize, uint32 position) {
	while (position < dataSize &&
		   data[position] == kCtrlAltEndMarker1) {
		position++;
	}
	return position;
}

// Peek at next meaningful byte after skipping control bytes
uint32 DialogManager::peekNextMeaningfulByte(const byte *data, uint32 dataSize, uint32 position) {
	uint32 peekPos = position;
	while (peekPos < dataSize &&
		   (data[peekPos] == kCtrlAltEndMarker1 ||
			data[peekPos] == kCtrlTextTerminator)) {
		peekPos++;
	}
	return peekPos;
}

// Initialize conversation by finding speaker and root
ConversationState DialogManager::initializeConversation(const byte *data, uint32 dataSize, byte npcIndex) {
	ConversationState state;
	state.position = findSpeaker(npcIndex, dataSize, data);
	state.currentRoot = 0;
	state.position = findRoot(npcIndex, state.currentRoot, state.position, dataSize, data);
	state.currentChoiceLevel = -1;
	state.lastSelectedChoice = ChoiceOption();

	// Skip any junk at start until we find a speaker marker
	while (state.position < dataSize && data[state.position] != kCtrlSpeakerId) {
		state.position++;
	}

	return state;
}

// Handle F0 "Go Back" control byte
// When F0 is hit, all choices at the current level have been exhausted.
// The cascading disable in disableChoiceIfNeeded should have already disabled
// the choices that led here. We just need to go back to the parent level.
bool DialogManager::handleGoBack(const byte *data, Common::Stack<uint32> &positionStack, uint32 position, ConversationState &state) {
	if (data[position] != kCtrlGoBack) {
		return false;
	}

	// Pop position stack - we're going back to parent level
	uint32 parentPos = positionStack.empty() ? 0 : positionStack.pop();

	if (parentPos == 0) {
		// F0: No parent position on stack, ending conversation
		return false;
	}

	// Go up one level
	state.currentChoiceLevel--;
	state.position = parentPos;

	return true;
}

uint32 DialogManager::readAndDisplayDialogue(const byte *data, uint32 dataSize, uint32 position) {
	Common::String text;
	byte speakerId;
	uint32 endPos = readTextBlock(data, dataSize, position, text, speakerId);
	Common::Array<Common::Array<Common::String>> wrappedText = wordWrap(text);

	// Skip spurious single character artifacts
	if (!text.empty() && text.size() > 1) {
		displayDialogue(wrappedText, speakerId);
	}

	return endPos;
}

ConversationEndResult DialogManager::checkConversationEnd(const byte *data, uint32 dataSize, uint32 position, int currentRoot) {
	ConversationEndResult result;
	result.shouldEnd = false;
	result.nextPosition = position;
	result.hasAction = false;
	result.actionCode = 0;

	if (position >= dataSize) {
		debug("Reached end of data after reading dialogue");
		result.shouldEnd = true;
		return result;
	}

	byte controlByte = data[position];

	if (controlByte == kCtrlEndConversation) {
		// Bug in the original in room 45, root 1: The conversation data has F4 (END_CONV) after
		// the opening NPC text instead of FD (END_TEXT), so the 3 choices that follow are
		// unreachable. Treat F4 as FD specifically for this root to restore them.
		int room = g_engine->_room->_currentRoomNumber;
		uint32 peekPos = position + 1;
		if (room == 45 && currentRoot == 1 &&
			peekPos < dataSize &&
			(data[peekPos] == kCtrlDialogueMarker || data[peekPos] == kCtrlDialogueMarkerOneoff)) {
			result.nextPosition = position + 1;
			return result;
		}
		result.shouldEnd = true;
		return result;
	}

	if (controlByte == kCtrlActionAndEnd) {
		result.actionCode = data[position + 1] | (data[position + 2] << 8);
		// Action-and-end trigger encountered
		result.shouldEnd = true;
		result.hasAction = true;
		return result;
	}

	if (controlByte == kCtrlActionAndContinue) {
		if (position + 2 < dataSize) {
			result.actionCode = data[position + 1] | (data[position + 2] << 8);
			// Action-and-continue trigger encountered
			result.hasAction = true;
		}
		result.shouldEnd = false;
		result.nextPosition = position + 3;
		return result;
	}

	// Move past control byte
	if (controlByte == kCtrlEndText) {
		result.nextPosition = position + 1;
	}

	return result;
}

/**
 * When there are no choices that lead to ending conversation, a generic option is added, as long as
 * _goodbyeDisabled is not set for the room.
 *
 * So Goodbye is added if:
 * - Goodbye is not globally disabled for this room, AND
 * - There are multiple choices (if only 1, it's auto-dialogue and shouldn't have goodbye), AND
 * - None of the choices already have a conversation terminator (F4 or F8) in their response path
 */
void DialogManager::addGoodbyeOptionIfNeeded(Common::Array<ChoiceOption> *choices, int currentChoiceLevel, uint originalChoiceCount) {
	// Room entry handlers can globally disable the goodbye option for certain rooms
	if (_goodbyeDisabled) {
		return;
	}

	// Only consider adding goodbye if there are MULTIPLE choices
	// If there's only 1 choice, it's auto-dialogue and should not have goodbye
	if (originalChoiceCount <= 1) {
		return;
	}

	// Check if ANY choice has a conversation terminator (F4 or F8)
	// If so, there's already a way to exit - don't add goodbye option
	bool anyChoiceTerminatesConversation = false;
	for (uint i = 0; i < choices->size(); i++) {
		if ((*choices)[i].hasConversationEndMarker) {
			anyChoiceTerminatesConversation = true;
			break;
		}
	}

	if (!anyChoiceTerminatesConversation) {
		ChoiceOption termChoice;
		termChoice.choiceIndex = currentChoiceLevel;
		termChoice.isTerminator = true;
		termChoice.isDisabled = false;
		termChoice.shouldDisableOnSelect = false;
		termChoice.text = "  " + g_engine->_res->_conversationTerminator;
		choices->push_back(termChoice);
	}
}

// Handle choice selection and response
uint32 DialogManager::processChoiceSelection(
	const byte *data,
	uint32 dataSize,
	Common::Array<ChoiceOption> *choices,
	int selectedIndex,
	ConversationState &state) {

	if (selectedIndex < 0 || selectedIndex >= (int)choices->size()) {
		return state.position;
	}

	// Save this choice in case we hit F0 and need to disable it
	state.lastSelectedChoice = (*choices)[selectedIndex];

	if (state.lastSelectedChoice.isTerminator) {
		displayDialogue(state.lastSelectedChoice.text, kAlfredColor);
		return dataSize; // End conversation
	}

	uint32 position = (*choices)[selectedIndex].dataOffset;
	state.currentChoiceLevel = (*choices)[selectedIndex].choiceIndex;

	// Read and display the selected choice as dialogue
	Common::String choiceText;
	byte choiceSpeakerId;
	uint32 endPos = readTextBlock(data, dataSize, position, choiceText, choiceSpeakerId);

	if (!choiceText.empty() && choiceText.size() > 1) {
		displayDialogue(choiceText, kAlfredColor);
		//Will check if choice should be disabled after displaying dialogue
		maybeDisableChoice(choices, selectedIndex, data, dataSize, endPos, state);
	}

	position = endPos;

	// Skip past end marker
	if (position < dataSize) {
		byte endByte = data[position];
		if (endByte == kCtrlEndText || endByte == kCtrlEndBranch ||
			endByte == kCtrlActionAndEnd) {
			position++;
		}
	}

	return position;
}

void DialogManager::maybeDisableChoice(Common::Array<Pelrock::ChoiceOption> *choices, int selectedIndex, const byte *data, uint32 dataSize, uint32 endPos, Pelrock::ConversationState &state) {
	// Cascading parent disable:
	// 1. Check if current choice's sub-branches are exhausted
	// 2. If so AND it's 0xFB, disable the current choice
	// 3. Go up to parent level, check if parent's sub-branches are exhausted
	// 4. Continue until we find a level with active sub-branches or reach level 1

	// Basically this means if the current choice getting disabled was also the last sub-branch of the previous level, that choice also
	// has to be disabled.

	// Start with the currently selected choice
	int currentLevel = state.currentChoiceLevel;
	uint32 currentChoicePos = (*choices)[selectedIndex].dataOffset;
	bool isCurrentFB = (*choices)[selectedIndex].shouldDisableOnSelect;

	while (currentLevel >= 1) {
		// Check if all sub-branches at this level are exhausted
		bool allExhausted = checkAllSubBranchesExhausted(data, dataSize, currentChoicePos + 4, currentLevel);

		if (!allExhausted) {
			break;
		}

		// Check if this choice is F1 (repeatable) - don't disable
		if (!isCurrentFB) {
			break;
		}

		// Disable this one-time choice
		ChoiceOption choiceToDisable;
		choiceToDisable.room = g_engine->_room->_currentRoomNumber;
		choiceToDisable.dataOffset = currentChoicePos;
		choiceToDisable.choiceIndex = currentLevel;
		choiceToDisable.shouldDisableOnSelect = true;
		g_engine->_room->addDisabledChoice(choiceToDisable);

		// Stop if we've reached level 1
		if (currentLevel <= 1) {
			// Reached level 1, stopping cascading disable
			break;
		}

		// Go up one level - scan backwards to find the parent FB/F1 marker at (currentLevel - 1)
		currentLevel--;
		uint32 scanPos = currentChoicePos;
		bool foundParent = false;

		while (scanPos > 0) {
			scanPos--;
			byte b = data[scanPos];

			// Found 0xFB marker
			if (b == kCtrlDialogueMarkerOneoff && scanPos + 1 < dataSize) {
				byte idx = data[scanPos + 1];
				if (idx == (byte)currentLevel) {
					currentChoicePos = scanPos;
					isCurrentFB = true;
					foundParent = true;
					debug("Found parent FB at level %d, pos %u", currentLevel, currentChoicePos);
					break;
				}
			}
			// Found 0xF1 marker (repeatable)
			else if (b == kCtrlDialogueMarker && scanPos + 1 < dataSize) {
				byte idx = data[scanPos + 1];
				if (idx == (byte)currentLevel) {
					// Found 0xF1 parent - will stop cascade on next iteration
					currentChoicePos = scanPos;
					isCurrentFB = false;
					foundParent = true;
					break;
				}
			}

			// Hit boundary markers - stop searching
			if (b == kCtrlAltSpeakerRoot || b == kCtrlEndBranch || b == kCtrlAltEndMarker1) {
				break;
			}
		}

		if (!foundParent) {
			break;
		}
	}
}

/**
 * Convenience method if we know it's Alfred talking
 */
void DialogManager::sayAlfred(Common::StringArray texts) {
	if (g_engine->_state->getFlag(FLAG_FROM_INTRO) == true) {
		g_engine->_alfredState.setState(ALFRED_SPECIAL_ANIM);
	} else {
		g_engine->_alfredState.setState(ALFRED_TALKING);
	}

	_curSprite = nullptr;
	Common::Array<Common::StringArray> textLines = wordWrap(texts);
	displayDialogue(textLines, kAlfredColor);
}

/**
 * Convenience method for Descriptions
 */
void DialogManager::sayAlfred(Description description) {
	Common::StringArray texts;

	texts.push_back(description.text);
	sayAlfred(texts);
	if (description.isAction) {
		g_engine->performActionTrigger(description.actionTrigger);
	}
}

/**
 * Convenience method when we want to enforce a specific NPC to speak the line.
 * Used mostly when it's npc index 1
 */
void DialogManager::say(Common::StringArray texts, byte spriteIndex) {
	if (texts.empty()) {
		return;
	}
	byte speakerId;
	bool wasProcessed = processColorAndTrim(texts, speakerId);

	if (wasProcessed) {
		if (speakerId == kAlfredColor) {
			sayAlfred(texts);
			return;
		} else {
			setCurSprite(spriteIndex);
			Common::Array<Common::StringArray> textLines = wordWrap(texts);
			displayDialogue(textLines, speakerId);
		}
	} else {
		sayAlfred(texts);
	}
}

/**
 * Convenience method to say a line normally but enforce x and y.
 * Regular path will simply use the sprite's x and y
 */
void DialogManager::say(Common::StringArray texts, int16 x, int16 y) {
	if (texts.empty()) {
		return;
	}
	byte speakerId;
	bool wasProcessed = processColorAndTrim(texts, speakerId);

	if (wasProcessed) {
		// Create a temporary sprite at the specified position
		Common::Array<Common::StringArray> textLines = wordWrap(texts);
		displayDialogue(textLines, speakerId, x, y);
	} else {
		sayAlfred(texts);
	}
}

/**
 * Read from the formatted string the color code, and trim text and control chars
 */
bool DialogManager::processColorAndTrim(Common::StringArray &lines, byte &speakerId) {
	int speakerMarker = lines[0][0];
	speakerId = lines[0][1];

	if (speakerMarker == '@') {
		for (uint i = 0; i < lines.size(); i++) {
			// Remove first two marker bytes
			if (i == 0) {
				if (lines[i].size() > 2) {
					lines[i] = lines[i].substr(2);

					if (lines[i][0] == 0x78 && lines[i][1] == 0x78) { // Remove additional control chars
						lines[i] = lines[i].substr(2);
					}
				} else {
					lines[i] = "";
				}
			}
		}
		return true;
	} else {
		debug("No speaker marker found, defaulting to Alfred");
	}
	return false;
}

bool isEndMarker(byte char_byte) {
	return char_byte == kCtrlEndText || char_byte == kCtrlEndConversation || char_byte == kCtrlActionAndEnd || char_byte == kCtrlGoBack;
}

int calculateWordLength(Common::String text, uint startPos, bool &isEnd) {
	int wordLength = 0;
	uint pos = startPos;
	while (pos < text.size()) {
		char char_byte = text[pos];
		if (char_byte == kCtrlSpace || isEndMarker(char_byte)) {
			break;
		}
		wordLength++;
		pos++;
	}
	// Check if we hit an end marker
	if (pos < text.size() && isEndMarker(text[pos])) {
		isEnd = true;
	}
	if (pos < text.size() && !isEnd) {
		if ((byte)text[pos] == kCtrlActionAndEnd) { // 0xF8 (-8) special case
			wordLength += 3;
		} else {
			// Count all consecutive spaces
			while (pos < text.size() && text[pos] == kCtrlSpace) {
				wordLength++;
				pos++;
			}
		}
	}
	return wordLength;
}

/**
 * Wrap a String into pages of multiple Strings.
 * The game enforces a maximum of 47 characters per line and 5 lines per page.
 * If a String is longer than that it gets broken down into multiple pages.
 */
Common::Array<Common::Array<Common::String>> DialogManager::wordWrap(Common::String text) {
	Common::Array<Common::Array<Common::String>> pages;
	Common::Array<Common::String> currentPage;
	Common::Array<Common::String> currentLine;
	int charsRemaining = kMaxCharsPerLine;
	uint position = 0;
	int currentLineNum = 0;
	while (position < text.size()) {
		bool isEnd = false;
		int wordLength = calculateWordLength(text, position, isEnd);
		// Extract the word (including trailing spaces)
		Common::String word = text.substr(position, wordLength);
		// if word_length > chars_remaining, wrap to next line
		if (wordLength > charsRemaining) {
			// Word is longer than the entire line - need to split
			currentPage.push_back(joinStrings(currentLine, ""));
			currentLine.clear();
			charsRemaining = kMaxCharsPerLine;
			currentLineNum++;

			if (currentLineNum >= kMaxLines) {
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
			while (lineText.lastChar() == kCtrlSpace) {
				lineText = lineText.substr(0, lineText.size() - 1);
			}
			int trailingSpaces = currentLine.size() - lineText.size();
			if (trailingSpaces > 0) {
				currentPage.push_back(lineText);
				//  current_line = [' ' * trailing_spaces]
				Common::String current_line(trailingSpaces, ' ');
				charsRemaining = kMaxCharsPerLine - trailingSpaces;
				currentLineNum += 1;

				if (currentLineNum >= kMaxLines) {
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

	if (!currentLine.empty()) {
		Common::String lineText = joinStrings(currentLine, "");
		while (lineText.lastChar() == kCtrlSpace) {
			lineText = lineText.substr(0, lineText.size() - 1);
		}
		currentPage.push_back(lineText);
	}

	if (!currentPage.empty()) {
		pages.push_back(currentPage);
	}

	return pages;
}

Common::Array<Common::StringArray> DialogManager::wordWrap(Common::StringArray texts) {
	// Sometimes we already get a pre-processed list of strings the character has to speak
	// but we still need to add line breaks if they exceed the max chars.
	// That means if we receive 3 lines but one is longer than the max we need to make 4 lines.
	// if we already have 5 pages, but one of them exceeds the max, we need to construct a new page.
	// for this is not enough to just push the result of wordWrap(String) becasue we still need to calculate new pages.

	Common::Array<Common::StringArray> pages;
	Common::Array<Common::String> currentPage;
	int currentLineNum = 0;
	for (uint i = 0; i < texts.size(); i++) {
		Common::String thisLine = texts[i];
		Common::Array<Common::Array<Common::String>> wrapped = wordWrap(thisLine);
		for (uint j = 0; j < wrapped.size(); j++) {
			for (uint k = 0; k < wrapped[j].size(); k++) {
				if (currentLineNum < kMaxLines) {
					currentPage.push_back(wrapped[j][k]);
					currentLineNum++;
				} else {
					pages.push_back(currentPage);
					currentPage.clear();
					currentPage.push_back(wrapped[j][k]);
					currentLineNum = 1;
				}
			}
		}
	}

	if (!currentPage.empty()) {
		pages.push_back(currentPage);
	}

	return pages;
}
} // namespace Pelrock
