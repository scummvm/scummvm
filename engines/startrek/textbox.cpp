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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/events.h"
#include "common/stream.h"
#include "graphics/cursorman.h"

#include "startrek/graphics.h"
#include "startrek/resource.h"
#include "startrek/room.h"


namespace StarTrek {

const char *StarTrekEngine::getNextTextLine(const char *text, char *lineOutput, int lineWidth) {
	*lineOutput = '\0';
	if (*text == '\0')
		return nullptr;

	const char *lastSpaceInput = nullptr;
	char *lastSpaceOutput = nullptr;
	int charIndex = 0;

	while (charIndex != lineWidth && *text != '\0') {
		char c = *text;

		if (c == '\n') {
			*lineOutput = '\0';
			return text + 1;
		}

		if (c == ' ') {
			lastSpaceInput = text;
			lastSpaceOutput = lineOutput;
		}

		if (c == '\r') {
			text++;
			charIndex--;
		} else {
			text++;
			*(lineOutput++) = c;
		}
		charIndex++;
	}

	if (*text == '\0') {
		*lineOutput = '\0';
		return text;
	}
	if (*text == ' ') {
		*lineOutput = '\0';
		return text + 1;
	}
	if (lastSpaceOutput == nullptr) { // Long word couldn't fit on line
		*lineOutput = '\0';
		return text;
	}

	// In the middle of a word; must go back to the start of it
	*lastSpaceOutput = '\0';
	return lastSpaceInput + 1;
}

void StarTrekEngine::drawTextLineToBitmap(const char *text, int textLen, int x, int y, Bitmap *bitmap) {
	const int charWidth = 8;

	int textOffset = 0;

	while (textOffset < textLen) {
		Common::Rect destRect(x, y, x + 8, y + 8);
		Common::Rect bitmapRect(bitmap->width, bitmap->height);

		if (destRect.intersects(bitmapRect)) {
			// drawRect = the rectangle within the 8x8 font character that will be drawn
			// (part of it may be clipped)
			Common::Rect drawRect;
			drawRect.left = bitmapRect.left - destRect.left;
			if (drawRect.left < destRect.left - destRect.left)
				drawRect.left = destRect.left - destRect.left;

			drawRect.right = bitmapRect.right - destRect.left;
			if (drawRect.right > destRect.right - destRect.left)
				drawRect.right = destRect.right - destRect.left;

			drawRect.top = bitmapRect.top - destRect.top;
			if (drawRect.top < destRect.top - destRect.top)
				drawRect.top = destRect.top - destRect.top;

			drawRect.bottom = bitmapRect.bottom - destRect.top;
			if (drawRect.bottom > destRect.bottom - destRect.top)
				drawRect.bottom = destRect.bottom - destRect.top;


			int16 destX = destRect.left - bitmapRect.left;
			if (destX < bitmapRect.right - bitmapRect.right)
				destX = bitmapRect.right - bitmapRect.right;

			int16 destY = destRect.top - bitmapRect.top;
			if (destY < bitmapRect.top - bitmapRect.top)
				destY = bitmapRect.top - bitmapRect.top;

			int16 srcRowDiff = charWidth - drawRect.width();
			int16 destRowDiff = bitmapRect.width() - drawRect.width();

			byte *srcPixels = _gfx->getFontGfx(text[textOffset]) + drawRect.top * charWidth + drawRect.left;
			byte *destPixels = bitmap->pixels + destY * bitmapRect.width() + destX;

			for (int i = 0; i < drawRect.height(); i++) {
				memcpy(destPixels, srcPixels, drawRect.width());
				destPixels += destRowDiff + drawRect.width();
				srcPixels += srcRowDiff + drawRect.width();
			}
		}

		x += charWidth;
		textOffset++;
	}
}

Common::String StarTrekEngine::centerTextboxHeader(Common::String headerText) {
	char text[TEXT_CHARS_PER_LINE + 1];
	memset(text, ' ', sizeof(text));
	text[TEXT_CHARS_PER_LINE] = '\0';

	int strlen = headerText.size();
	strlen = MIN(strlen, TEXT_CHARS_PER_LINE);

	memcpy(text + (TEXT_CHARS_PER_LINE - strlen) / 2, headerText.c_str(), strlen);

	return Common::String(text);
}

void StarTrekEngine::getTextboxHeader(Common::String *headerTextOutput, Common::String speakerText, int choiceIndex) {
	Common::String header = speakerText;

	if (choiceIndex != 0)
		header += Common::String::format(" choice %d", choiceIndex);

	*headerTextOutput = centerTextboxHeader(header);
}

Common::String StarTrekEngine::readTextFromRdf(int choiceIndex, uintptr data, Common::String *headerTextOutput) {
	Room *room = getRoom();

	int rdfVar = (size_t)data;

	uint16 textOffset = room->readRdfWord(rdfVar + (choiceIndex + 1) * 2);

	if (textOffset == 0)
		return "";

	if (headerTextOutput != nullptr) {
		uint16 speakerOffset = room->readRdfWord(rdfVar);
		if (speakerOffset == 0 || room->_rdfData[speakerOffset] == '\0')
			*headerTextOutput = "";
		else {
			char *speakerText = (char *)&room->_rdfData[speakerOffset];
			if (room->readRdfWord(rdfVar + 4) != 0) // Check if there's more than one option
				getTextboxHeader(headerTextOutput, speakerText, choiceIndex + 1);
			else
				getTextboxHeader(headerTextOutput, speakerText, 0);
		}
	}

	return (char *)&room->_rdfData[textOffset];
}

void StarTrekEngine::showTextbox(Common::String headerText, const Common::String &mainText, int xoffset, int yoffset, byte textColor, int maxTextLines) {
	if (!headerText.empty())
		headerText = centerTextboxHeader(headerText);

	int actionParam = (maxTextLines < 0 ? 0 : maxTextLines);

	if (maxTextLines < 0)
		maxTextLines = -maxTextLines;

	const char *strings[3];

	if (headerText.empty())
		strings[0] = nullptr;
	else
		strings[0] = headerText.c_str();
	strings[1] = mainText.c_str();
	strings[2] = "";

	showText(&StarTrekEngine::readTextFromArray, (uintptr)strings, xoffset, yoffset, textColor, false, maxTextLines, false);

	if (actionParam != 0)
		addAction(ACTION_TALK, actionParam, 0, 0);
}

Common::String StarTrekEngine::skipTextAudioPrompt(const Common::String &str) {
	const char *text = str.c_str();

	if (*text != '#')
		return str;

	text++;
	while (*text != '#') {
		if (*text == '\0')
			return str;
		text++;
	}

	return Common::String(text + 1);
}

Common::String StarTrekEngine::playTextAudio(const Common::String &str) {
	const char *text = str.c_str();
	Common::String soundFile;

	if (*text != '#')
		return str;

	int len = 0;
	text++;
	while (*text != '#') {
		if (*text == '\0' || len > 0xfa)
			return str;
		soundFile += *text++;
	}

	_sound->playSpeech(soundFile);

	return Common::String(text + 1);
}

int StarTrekEngine::showText(TextGetterFunc textGetter, uintptr var, int xoffset, int yoffset, int textColor, bool loopChoices, int maxTextLines, bool rclickCancelsChoice) {
	int16 tmpTextDisplayMode = _textDisplayMode;

	uint32 ticksUntilClickingEnabled = 8;
	if (_frameIndex > _textboxVar2 + 1) {
		ticksUntilClickingEnabled = 0x10;
	}

	int numChoicesWithNames = 0;
	int numTextboxLines = 0;
	int numChoices = 0;
	Common::String speakerText;

	while (true) {
		Common::String choiceText = (this->*textGetter)(numChoices, var, &speakerText);
		if (choiceText.empty())
			break;

		int lines = getNumTextboxLines(skipTextAudioPrompt(choiceText));
		if (lines > numTextboxLines)
			numTextboxLines = lines;

		if (!speakerText.empty()) // FIXME: Technically should check for nullptr
			numChoicesWithNames++;

		numChoices++;
	}

	if (maxTextLines == 0 || maxTextLines > MAX_TEXTBOX_LINES)
		maxTextLines = MAX_TEXTBOX_LINES;
	if (numTextboxLines > maxTextLines)
		numTextboxLines = maxTextLines;

	if (numChoicesWithNames != 0 && numChoices != numChoicesWithNames)
		error("showText: Not all choices have titles.");

	Sprite textboxSprite;
	TextBitmap *textBitmap = initTextSprite(&xoffset, &yoffset, textColor, numTextboxLines, numChoicesWithNames, &textboxSprite);

	int choiceIndex = 0;
	int scrollOffset = 0;
	if (tmpTextDisplayMode != TEXTDISPLAY_WAIT && tmpTextDisplayMode != TEXTDISPLAY_SUBTITLES
	        && numChoices == 1 && _sfxEnabled && !_sfxWorking)
		_textboxHasMultipleChoices = false;
	else
		_textboxHasMultipleChoices = true;

	if (tmpTextDisplayMode >= TEXTDISPLAY_WAIT && tmpTextDisplayMode <= TEXTDISPLAY_NONE
	        && _sfxEnabled && !_sfxWorking)
		_textboxVar6 = true;
	else
		_textboxVar6 = false;

	int numTextLines;
	Common::String lineFormattedText = readLineFormattedText(textGetter, var, choiceIndex, textBitmap, numTextboxLines, &numTextLines);

	if (lineFormattedText.empty()) { // Technically should check for nullptr
		_gfx->delSprite(&textboxSprite);

		// TODO
	} else {
		loadMenuButtons("textbtns", xoffset + 0x96, yoffset - 0x11);

		Common::Point oldMousePos = _gfx->getMousePos();
		_gfx->warpMouse(xoffset + 0xde, yoffset - 0x08);
		_gfx->setMouseBitmap("pushbtn");

		bool tmpMouseControllingShip = _mouseControllingShip;
		_mouseControllingShip = false;

		// Decide which buttons to show
		uint32 visibleButtons = (1 << TEXTBUTTON_CONFIRM);
		if (numChoices > 1)
			visibleButtons |= (1 << TEXTBUTTON_PREVCHOICE) | (1 << TEXTBUTTON_NEXTCHOICE);
		if (numTextLines > numTextboxLines)
			visibleButtons |= (1 << TEXTBUTTON_SCROLLUP) | (1 << TEXTBUTTON_SCROLLDOWN);
		setVisibleMenuButtons(visibleButtons);

		disableMenuButtons(1 << TEXTBUTTON_SCROLLUP); // Disable scroll up

		if (ticksUntilClickingEnabled != 0) // Disable done button
			disableMenuButtons(1 << TEXTBUTTON_CONFIRM);

		if (!loopChoices) // Disable prev button
			disableMenuButtons(1 << TEXTBUTTON_PREVCHOICE);

		bool doneShowingText = false;

		// Loop until text is done being displayed
		while (!doneShowingText) {
			int textboxReturnCode = handleMenuEvents(ticksUntilClickingEnabled, true);

			if (ticksUntilClickingEnabled != 0)
				enableMenuButtons(1 << TEXTBUTTON_CONFIRM);

			switch (textboxReturnCode) {

			case MENUEVENT_RCLICK_OFFBUTTON:
			case MENUEVENT_RCLICK_ONBUTTON:
				if (ticksUntilClickingEnabled == 0) {
					doneShowingText = true;
					if (rclickCancelsChoice)
						choiceIndex = -1;
				}
				break;

			case TEXTBUTTON_CONFIRM:
				doneShowingText = true;
				break;

			case TEXTBUTTON_SCROLLUP:
			case TEXTBUTTON_SCROLLUP_ONELINE:
				scrollOffset -= (textboxReturnCode == TEXTBUTTON_SCROLLUP ? numTextboxLines : 1);
				if (scrollOffset < 0)
					scrollOffset = 0;
				if (scrollOffset == 0)
					disableMenuButtons(1 << TEXTBUTTON_SCROLLUP);
				enableMenuButtons(1 << TEXTBUTTON_SCROLLDOWN);
				textboxSprite.bitmapChanged = true;
				drawMainText(
				    textBitmap,
				    numTextLines - scrollOffset,
				    numTextboxLines,
				    lineFormattedText.c_str() + scrollOffset * (TEXTBOX_WIDTH - 2),
				    numChoicesWithNames != 0);
				break;

			case TEXTBUTTON_GOTO_TOP:
				scrollOffset = 0;
				disableMenuButtons(1 << TEXTBUTTON_SCROLLUP);
				enableMenuButtons(1 << TEXTBUTTON_SCROLLDOWN);
				textboxSprite.bitmapChanged = true;
				drawMainText(
				    textBitmap,
				    numTextLines - scrollOffset,
				    numTextboxLines,
				    lineFormattedText.c_str() + scrollOffset * (TEXTBOX_WIDTH - 2),
				    numChoicesWithNames != 0);
				break;

			case TEXTBUTTON_SCROLLDOWN:
			case TEXTBUTTON_SCROLLDOWN_ONELINE:
				scrollOffset += (textboxReturnCode == TEXTBUTTON_SCROLLDOWN ? numTextboxLines : 1);
				enableMenuButtons(1 << TEXTBUTTON_SCROLLUP);
				if (scrollOffset >= numTextLines)
					scrollOffset -= numTextboxLines;
				if (scrollOffset > numTextLines - 1)
					scrollOffset = numTextLines - 1;
				if (scrollOffset + numTextboxLines >= numTextLines)
					disableMenuButtons(1 << TEXTBUTTON_SCROLLDOWN);
				textboxSprite.bitmapChanged = true;
				drawMainText(
				    textBitmap,
				    numTextLines - scrollOffset,
				    numTextboxLines,
				    lineFormattedText.c_str() + scrollOffset * (TEXTBOX_WIDTH - 2),
				    numChoicesWithNames != 0);
				break;

			case TEXTBUTTON_GOTO_BOTTOM:
				scrollOffset = numTextLines - numTextboxLines;
				enableMenuButtons(1 << TEXTBUTTON_SCROLLUP);
				disableMenuButtons(1 << TEXTBUTTON_SCROLLDOWN);
				textboxSprite.bitmapChanged = true;
				drawMainText(
				    textBitmap,
				    numTextLines - scrollOffset,
				    numTextboxLines,
				    lineFormattedText.c_str() + scrollOffset * (TEXTBOX_WIDTH - 2),
				    numChoicesWithNames != 0);
				break;

			case TEXTBUTTON_PREVCHOICE:
			case TEXTBUTTON_NEXTCHOICE:
				if (textboxReturnCode == TEXTBUTTON_PREVCHOICE) {
					choiceIndex--;
					if (!loopChoices && choiceIndex == 0) {
						disableMenuButtons(1 << TEXTBUTTON_PREVCHOICE);
					}
					else {
						if (choiceIndex < 0)
							choiceIndex = numChoices - 1;
					}
					enableMenuButtons(1 << TEXTBUTTON_NEXTCHOICE);
				} else {
					enableMenuButtons(1 << TEXTBUTTON_PREVCHOICE);
					choiceIndex++;
					if (!loopChoices && choiceIndex == numChoices - 1) {
						disableMenuButtons(1 << TEXTBUTTON_NEXTCHOICE);
					}
					else {
						choiceIndex %= numChoices;
					}
				}

				scrollOffset = 0;
				lineFormattedText = readLineFormattedText(textGetter, var, choiceIndex, textBitmap, numTextboxLines, &numTextLines);
				if (numTextLines <= numTextboxLines) {
					setVisibleMenuButtons((1 << TEXTBUTTON_CONFIRM) | (1 << TEXTBUTTON_PREVCHOICE) | (1 << TEXTBUTTON_NEXTCHOICE));
				} else {
					setVisibleMenuButtons((1 << TEXTBUTTON_CONFIRM) | (1 << TEXTBUTTON_SCROLLUP) | (1 << TEXTBUTTON_SCROLLDOWN) | (1 << TEXTBUTTON_PREVCHOICE) | (1 << TEXTBUTTON_NEXTCHOICE));
				}
				enableMenuButtons(1 << TEXTBUTTON_SCROLLDOWN);
				disableMenuButtons(1 << TEXTBUTTON_SCROLLUP);
				textboxSprite.bitmapChanged = true;
				break;

			case TEXTBUTTON_SPEECH_DONE:
				if (numChoices == 1)
					doneShowingText = true;
				break;

			case MENUEVENT_ENABLEINPUT:
			case MENUEVENT_LCLICK_OFFBUTTON:
			default:
				break;
			}

			ticksUntilClickingEnabled = 0;
		}

		_gfx->popMouseBitmap();
		_gfx->warpMouse(oldMousePos.x, oldMousePos.y);

		_mouseControllingShip = tmpMouseControllingShip;
		unloadMenuButtons();

		textboxSprite.dontDrawNextFrame();
		_gfx->drawAllSprites();
		//delete textBitmap;
		delete textboxSprite.bitmap;
		textboxSprite.bitmap = nullptr;
		_gfx->delSprite(&textboxSprite);
	}

	_textboxVar2 = _frameIndex;
	_sound->stopPlayingSpeech();

	return choiceIndex;
}

int StarTrekEngine::getNumTextboxLines(const Common::String &str) {
	const char *text = str.c_str();
	char line[TEXTBOX_WIDTH];

	int lines = 0;

	while (text != nullptr) {
		text = getNextTextLine(text, line, TEXTBOX_WIDTH - 2);
		lines++;
	}
	return lines - 1;
}

Common::String StarTrekEngine::putTextIntoLines(const Common::String &_text) {
	char line[TEXTBOX_WIDTH];

	const char *text = _text.c_str();
	String output;

	text = getNextTextLine(text, line, TEXTBOX_WIDTH - 2);

	while (text != nullptr) {
		int len = strlen(line);
		while (len != TEXTBOX_WIDTH - 2) {
			line[len++] = ' ';
			line[len] = '\0';
		}
		output += line;

		text = getNextTextLine(text, line, TEXTBOX_WIDTH - 2);
	}

	return output;
}

TextBitmap *StarTrekEngine::initTextSprite(int *xoffsetPtr, int *yoffsetPtr, byte textColor, int numTextLines, bool withHeader, Sprite *sprite) {
	int linesBeforeTextStart = 2;
	if (withHeader)
		linesBeforeTextStart = 4;

	int xoffset = *xoffsetPtr;
	int yoffset = *yoffsetPtr;

	int textHeight = numTextLines + linesBeforeTextStart;

	TextBitmap *bitmap = new TextBitmap(TEXTBOX_WIDTH * 8, textHeight * 8);

	*sprite = Sprite();
	sprite->drawPriority = 15;
	sprite->drawPriority2 = 8;
	sprite->bitmap = bitmap;	// This is deallocated explicitly at the end of showText()
	sprite->textColor = textColor;

	memset(bitmap->pixels, ' ', textHeight * TEXTBOX_WIDTH);

	int varC = SCREEN_WIDTH - 1 - xoffset - (bitmap->width + 0x1d) / 2;
	if (varC < 0)
		xoffset += varC;

	varC = xoffset - (bitmap->width + 0x1d) / 2;
	if (varC < 1)
		xoffset -= varC - 1;

	varC = yoffset - (bitmap->height + 0x11) - 20;
	if (varC < 0)
		yoffset -= varC;

	xoffset -= (bitmap->width + 0x1d) / 2;
	yoffset -= bitmap->height;

	bitmap->pixels[0] = 0x10;
	memset(&bitmap->pixels[1], 0x11, TEXTBOX_WIDTH - 2);
	bitmap->pixels[TEXTBOX_WIDTH - 1] = 0x12;

	byte *textAddr = bitmap->pixels + TEXTBOX_WIDTH;

	if (withHeader) {
		textAddr[0] = 0x13;
		textAddr[TEXTBOX_WIDTH - 1] = 0x14;
		textAddr += TEXTBOX_WIDTH;

		textAddr[0] = 0x13;
		memset(&textAddr[1], 0x19, TEXTBOX_WIDTH - 2);
		textAddr[TEXTBOX_WIDTH - 1] = 0x14;
		textAddr += TEXTBOX_WIDTH;
	}

	for (int line = 0; line < numTextLines; line++) {
		textAddr[0] = 0x13;
		textAddr[TEXTBOX_WIDTH - 1] = 0x14;
		textAddr += TEXTBOX_WIDTH;
	}

	textAddr[0] = 0x15;
	memset(&textAddr[1], 0x16, TEXTBOX_WIDTH - 2);
	textAddr[TEXTBOX_WIDTH - 1] = 0x17;

	_gfx->addSprite(sprite);
	sprite->drawMode = 3;
	sprite->pos.x = xoffset;
	sprite->pos.y = yoffset;
	sprite->drawPriority = 15;

	*xoffsetPtr = xoffset;
	*yoffsetPtr = yoffset;

	return bitmap;
}

void StarTrekEngine::drawMainText(TextBitmap *bitmap, int numTextLines, int numTextboxLines, const Common::String &_text, bool withHeader) {
	byte *dest = bitmap->pixels + TEXTBOX_WIDTH + 1; // Start of 2nd row
	const char *text = _text.c_str();

	if (numTextLines >= numTextboxLines)
		numTextLines = numTextboxLines;

	if (withHeader)
		dest += TEXTBOX_WIDTH * 2; // Start of 4th row

	int lineIndex = 0;
	while (lineIndex != numTextLines) {
		memcpy(dest, text, TEXTBOX_WIDTH - 2);
		text += TEXTBOX_WIDTH - 2;
		dest += TEXTBOX_WIDTH;
		lineIndex++;
	}

	// Fill all remaining blank lines
	while (lineIndex != numTextboxLines) {
		memset(dest, ' ', TEXTBOX_WIDTH - 2);
		dest += TEXTBOX_WIDTH;
		lineIndex++;
	}
}

Common::String StarTrekEngine::readLineFormattedText(TextGetterFunc textGetter, uintptr var, int choiceIndex, TextBitmap *textBitmap, int numTextboxLines, int *numTextLines) {
	Common::String headerText;
	Common::String text = (this->*textGetter)(choiceIndex, var, &headerText);

	if (_textDisplayMode == TEXTDISPLAY_NONE && _sfxEnabled && _sfxWorking) {
		uint32 oldSize = text.size();
		text = playTextAudio(text);
		if (oldSize != text.size())
			_textboxHasMultipleChoices = true;
	} else if ((_textDisplayMode == TEXTDISPLAY_WAIT || _textDisplayMode == TEXTDISPLAY_SUBTITLES)
	           && _sfxEnabled && _sfxWorking) {
		text = playTextAudio(text);
	} else {
		text = skipTextAudioPrompt(text);
	}

	if (_textboxHasMultipleChoices) {
		*numTextLines = getNumTextboxLines(text);

		bool hasHeader = !headerText.empty();

		String lineFormattedText = putTextIntoLines(text);
		drawMainText(textBitmap, *numTextLines, numTextboxLines, lineFormattedText, hasHeader);

		memcpy(textBitmap->pixels + TEXTBOX_WIDTH + 1, headerText.c_str(), headerText.size());

		return lineFormattedText;
	} else
		return NULL;
}

Common::String StarTrekEngine::readTextFromArray(int choiceIndex, uintptr data, Common::String *headerTextOutput) {
	const char **textArray = (const char **)data;

	const char *headerText = textArray[0];
	const char *mainText = textArray[choiceIndex + 1];

	if (*mainText == '\0')
		return Common::String(); // Technically should be nullptr...

	if (headerText == nullptr)
		*headerTextOutput = "";
	else
		*headerTextOutput = centerTextboxHeader(headerText);
	return Common::String(mainText);
}

Common::String StarTrekEngine::readTextFromArrayWithChoices(int choiceIndex, uintptr data, Common::String *headerTextOutput) {
	const char **textArray = (const char **)data;

	const char *headerText = textArray[0];
	const char *mainText = textArray[choiceIndex + 1];

	if (mainText == nullptr || *mainText == '\0')
		return Common::String(); // Technically should be nullptr...

	if (headerTextOutput != nullptr) {
		if (headerText == nullptr || headerText[0] == '\0')
			*headerTextOutput = "";
		else {
			if (textArray[2] != nullptr && textArray[2][0] != '\0') // More than one choice
				getTextboxHeader(headerTextOutput, headerText, choiceIndex + 1);
			else
				getTextboxHeader(headerTextOutput, headerText, 0);
		}
	}
	return Common::String(mainText);
}

Common::String StarTrekEngine::readTextFromFoundComputerTopics(int choiceIndex, uintptr data, Common::String *headerTextOutput) {
	if (choiceIndex >= 10)
		return Common::String();

	Common::String topicFile = (char *)data + 8 * choiceIndex;
	topicFile.trim();

	if (topicFile.empty())
		return Common::String();

	Common::MemoryReadStreamEndian *content = _resource->loadFile(Common::String(topicFile) + ".db");
	Common::String headerText = content->readLine();
	Common::String mainText;
	Common::String line;
	while (!content->eos() && !content->err()) {
		line = content->readLine();
		if (line == "$")
			break;
		mainText += line;
	}
	delete content;

	*headerTextOutput = "R: " + headerText;
	return mainText;
}

Common::String StarTrekEngine::showCodeInputBox() {
	memset(_textInputBuffer, 0, TEXT_INPUT_BUFFER_SIZE - 1);
	return showTextInputBox(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, "Code:\n                   ");
}

Common::String StarTrekEngine::showComputerInputBox() {
	memset(_textInputBuffer, 0, TEXT_INPUT_BUFFER_SIZE - 1);
	return showTextInputBox(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, "Computer:\n               ");
}

void StarTrekEngine::redrawTextInput() {
	char buf[MAX_TEXT_INPUT_LEN * 2 + 2];
	memset(buf, 0, MAX_TEXT_INPUT_LEN * 2);
	strcpy(buf, _textInputBuffer);

	if (_textInputCursorChar != 0)
		buf[_textInputCursorPos] = _textInputCursorChar;

	drawTextLineToBitmap(buf, MAX_TEXT_INPUT_LEN, 4, 12, _textInputSprite.bitmap);
	_textInputSprite.bitmapChanged = true;
	_gfx->drawAllSprites();
}

void StarTrekEngine::addCharToTextInputBuffer(char c) {
	Common::String str(_textInputBuffer);
	while ((int)str.size() < _textInputCursorPos) {
		str += " ";
	}

	str.insertChar(c, _textInputCursorPos);

	strncpy(_textInputBuffer, str.c_str(), MAX_TEXT_INPUT_LEN);
	_textInputBuffer[MAX_TEXT_INPUT_LEN] = '\0';
}

Common::String StarTrekEngine::showTextInputBox(int16 x, int16 y, const Common::String &headerText) {
	bool validInput = false;

	_keyboardControlsMouse = false;
	_textInputCursorPos = 0;

	initTextInputSprite(x, y, headerText);

	bool loop = true;

	while (loop) {
		TrekEvent event;
		if (!popNextEvent(&event))
			continue;

		switch (event.type) {
		case TREKEVENT_TICK:
			_gfx->incPaletteFadeLevel();
			_frameIndex++;
			_textInputCursorChar = ((_frameIndex & 2) ? 1 : 0);
			redrawTextInput();
			break;

		case TREKEVENT_LBUTTONDOWN:
			redrawTextInput();
			validInput = true;
			loop = false;
			break;

		case TREKEVENT_RBUTTONDOWN:
			loop = false;
			break;

		case TREKEVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_BACKSPACE:
				if (_textInputCursorPos > 0) {
					_textInputCursorPos--;
					Common::String str(_textInputBuffer);
					str.deleteChar(_textInputCursorPos);
					strcpy(_textInputBuffer, str.c_str());
				}
				redrawTextInput();
				break;

			case Common::KEYCODE_DELETE: { // ENHANCEMENT: Support delete key
				Common::String str(_textInputBuffer);
				if (_textInputCursorPos < (int)str.size()) {
					str.deleteChar(_textInputCursorPos);
					strcpy(_textInputBuffer, str.c_str());
					redrawTextInput();
				}
				break;
			}

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1:
				redrawTextInput();
				loop = false;
				validInput = true;
				break;

			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_F2:
				loop = false;
				break;

			case Common::KEYCODE_HOME:
			case Common::KEYCODE_KP7:
				_textInputCursorPos = 0;
				break;

			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				if (_textInputCursorPos > 0)
					_textInputCursorPos--;
				redrawTextInput();
				break;

			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				if (_textInputCursorPos < MAX_TEXT_INPUT_LEN - 1)
					_textInputCursorPos++;
				redrawTextInput();
				break;

			case Common::KEYCODE_END:
			case Common::KEYCODE_KP1:
				_textInputCursorPos = strlen(_textInputBuffer);
				// BUGFIX: Check that it doesn't exceed the buffer length.
				// Original game had a bug where you could crash the game by pressing
				// "end", writing a character, pressing "end" again, etc.
				if (_textInputCursorPos >= MAX_TEXT_INPUT_LEN)
					_textInputCursorPos = MAX_TEXT_INPUT_LEN - 1;
				break;

			default: // Typed any other character
				if (_gfx->_font->isDisplayableCharacter(event.kbd.ascii)) {
					addCharToTextInputBuffer(event.kbd.ascii);
					if (_textInputCursorPos < MAX_TEXT_INPUT_LEN - 1)
						_textInputCursorPos++;
					redrawTextInput();
				}
				break;
			}
			break;

		default:
			break;
		}
	}

	cleanupTextInputSprite();
	_keyboardControlsMouse = true;

	if (validInput)
		return _textInputBuffer;
	else
		return "";
}

void StarTrekEngine::initTextInputSprite(int16 textboxX, int16 textboxY, const Common::String &headerText) {
	int headerLen = headerText.size();

	if (headerLen > 25)
		headerLen = 25;

	char textBuf[TEXTBOX_WIDTH * 11 + 1];
	const char *headerPos = headerText.c_str();
	int row = 0;

	/*
	// TODO: investigate this (might be unused...)
	if (word_53100 != 0) {
		// ...
	}
	*/

	do {
		headerPos = getNextTextLine(headerPos, textBuf + row * TEXTBOX_WIDTH, headerLen);
		row++;
	} while (headerPos != 0 && row < 11);

	const int16 width = headerLen * 8 + 8;
	const int16 height = row * 8 + 8;


	_textInputSprite.bitmap = new Bitmap(width, height);

	_textInputSprite.bitmap->xoffset = width / 2;
	if (textboxX + width / 2 >= SCREEN_WIDTH)
		_textInputSprite.bitmap->xoffset += width / 2 + textboxX - (SCREEN_WIDTH - 1);
	if (textboxX - width / 2 < 0)
		_textInputSprite.bitmap->xoffset -= 0 - (textboxX - width / 2);

	_textInputSprite.bitmap->yoffset = height + 20;

	int16 xPos = 1;
	while (xPos < width - 1) {
		_textInputSprite.bitmap->pixels[1 * width + xPos] = 0x78;	// Top border
		_textInputSprite.bitmap->pixels[(height - 2) * width + xPos] = 0x78;	// Bottom border
		xPos++;
	}

	int16 yPos = 1;
	while (yPos < height - 1) {
		_textInputSprite.bitmap->pixels[yPos * width + 1] = 0x78;	// Left border
		_textInputSprite.bitmap->pixels[yPos * width + (width - 2)] = 0x78;	// Right border
		yPos++;
	}

	// Draw header text
	for (int r = 0; r < row; r++) {
		char *text = textBuf + r * TEXTBOX_WIDTH;
		drawTextLineToBitmap(text, strlen(text), 4, r * 8 + 4, _textInputSprite.bitmap);
	}

	_textInputSprite.drawMode = 2;
	_textInputSprite.field8 = "System";
	_textInputSprite.setXYAndPriority(textboxX, textboxY, 15);
	_textInputSprite.drawPriority2 = 8;
	_gfx->addSprite(&_textInputSprite);

	_gfx->drawAllSprites();
}

void StarTrekEngine::cleanupTextInputSprite() {
	_textInputSprite.dontDrawNextFrame();
	_gfx->drawAllSprites();
	_gfx->delSprite(&_textInputSprite);

	delete _textInputSprite.bitmap;
	_textInputSprite.bitmap = nullptr;
}

} // End of namespace StarTrek
