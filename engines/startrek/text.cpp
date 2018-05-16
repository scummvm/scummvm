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


namespace StarTrek {

/**
 * Gets one line of text (does not include words that won't fit).
 * Returns position of text to continue from, or nullptr if done.
 */
const char *StarTrekEngine::getNextTextLine(const char *text, char *lineOutput, int lineWidth) {
	*lineOutput = '\0';
	if (*text == '\0')
		return nullptr;

	const char *lastSpaceInput = nullptr;
	char *lastSpaceOutput = nullptr;
	int var4;
	int charIndex = 0;

	while (charIndex != lineWidth && *text != '\0') {
		char c = *text;

		if (c == '\n') {
			*lineOutput = '\0';
			return text+1;
		}

		if (c == ' ') {
			var4 = charIndex;
			lastSpaceInput = text;
			lastSpaceOutput = lineOutput;
		}

		if (c == '\r') {
			text++;
			charIndex--;
		}
		else {
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
		return text+1;
	}
	if (lastSpaceOutput == nullptr) { // Long word couldn't fit on line
		*lineOutput = '\0';
		return text;
	}

	// In the middle of a word; must go back to the start of it
	*lastSpaceOutput = '\0';
	return lastSpaceInput+1;
}

void StarTrekEngine::getTextboxHeader(String *headerTextOutput, String speakerText, int choiceIndex) {
	String header = speakerText;

	if (choiceIndex != 0)
		header += String::format(" choice %d", choiceIndex);

	if (header.size() > TEXTBOX_WIDTH-2)
		header.erase(TEXTBOX_WIDTH-2);
	while (header.size() < TEXTBOX_WIDTH-2)
		header += ' ';

	*headerTextOutput = header;
}

/**
 * Text getter for showText which reads from an rdf file.
 */
String StarTrekEngine::readTextFromRdf(int choiceIndex, uintptr data, String *headerTextOutput) {
	SharedPtr<Room> room = getRoom();

	int rdfVar = (size_t)data;

	uint16 textOffset = room->readRdfWord(rdfVar + (choiceIndex+1)*2);

	if (textOffset == 0)
		return "";

	if (headerTextOutput != nullptr) {
		uint16 speakerOffset = room->readRdfWord(rdfVar);
		if (speakerOffset == 0 || room->_rdfData[speakerOffset] == '\0')
			*headerTextOutput = "";
		else {
			char *speakerText = (char*)&room->_rdfData[speakerOffset];
			if (room->readRdfWord(rdfVar+4) != 0) // Check if there's more than one option
				getTextboxHeader(headerTextOutput, speakerText, choiceIndex+1);
			else
				getTextboxHeader(headerTextOutput, speakerText, 0);
		}
	}

	return (char*)&room->_rdfData[textOffset];
}

/**
 * Text getter for showText which reads from a given buffer.
 */
String StarTrekEngine::readTextFromBuffer(int choiceIndex, uintptr data, String *headerTextOutput) {
	char buf[TEXTBOX_WIDTH];
	memcpy(buf, (byte*)data, TEXTBOX_WIDTH-2);
	buf[TEXTBOX_WIDTH-2] = '\0';

	*headerTextOutput = String(buf);

	char *text = (char*)data+TEXTBOX_WIDTH-2;
	return String(text);
}

String StarTrekEngine::skipTextAudioPrompt(const String &str) {
	const char *text = str.c_str();

	if (*text != '#')
		return str;

	text++;
	while (*text != '#') {
		if (*text == '\0')
			return str;
		text++;
	}

	return String(text+1);
}

/**
 * Plays an audio prompt, if it exists, and returns the string starting at the end of the
 * prompt.
 */
String StarTrekEngine::playTextAudio(const String &str) {
	const char *text = str.c_str();
	char soundFile[0x100];

	if (*text != '#')
		return str;

	int len = 0;
	text++;
	while (*text != '#') {
		if (*text == '\0' || len > 0xfa)
			return str;
		soundFile[len++] = *text++;
	}

	soundFile[len] = '\0';
	playSpeech(soundFile);

	return String(text+1);
}

int StarTrekEngine::showText(TextGetterFunc textGetter, uintptr var, int xoffset, int yoffset, int textColor, bool loopChoices, int maxTextLines, int arg10) {
	int16 tmpTextDisplayMode = _textDisplayMode;

	uint32 var7c = 8;
	if (_frameIndex > _textboxVar2+1) {
		var7c = 0x10;
	}

	int numChoicesWithNames = 0;
	int numTextboxLines = 0;
	int numChoices = 0;
	String speakerText;

	while(true) {
		String choiceText = (this->*textGetter)(numChoices, var, &speakerText);
		if (choiceText.empty())
			break;

		int lines = getNumTextboxLines(choiceText);
		if (lines > numTextboxLines)
			numTextboxLines = lines;

		if (!speakerText.empty()) // Technically should check for nullptr
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
	SharedPtr<TextBitmap> textBitmap = initTextSprite(&xoffset, &yoffset, textColor, numTextboxLines, numChoicesWithNames, &textboxSprite);

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
	String lineFormattedText = readLineFormattedText(textGetter, var, choiceIndex, textBitmap, numTextboxLines, &numTextLines);

	if (lineFormattedText.empty()) { // Technically should check for nullptr
		// TODO
	}
	else {
		loadMenuButtons("textbtns", xoffset + 0x96, yoffset - 0x11);

		Common::Point oldMousePos = _gfx->getMousePos();
		SharedPtr<Bitmap> oldMouseBitmap = _gfx->getMouseBitmap();

		_gfx->warpMouse(xoffset + 0xde, yoffset - 0x08);
		_gfx->setMouseBitmap(_gfx->loadBitmap("pushbtn"));

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

		if (var7c == 0) { // Disable done button
			disableMenuButtons(1 << TEXTBUTTON_CONFIRM);
		}
		if (!loopChoices) { // Disable prev button
			disableMenuButtons(1 << TEXTBUTTON_PREVCHOICE);
		}

		bool doneShowingText = false;

		// Loop until text is done being displayed
		while (!doneShowingText) {
			int textboxReturnCode = handleMenuEvents(var7c, true);

			if (var7c == 0) {
				enableMenuButtons(1 << TEXTBUTTON_CONFIRM);
			}

			switch(textboxReturnCode) {

			case MENUEVENT_RCLICK_OFFBUTTON:
			case MENUEVENT_RCLICK_ONBUTTON:
				if (var7c == 0) {
					doneShowingText = true;
					if (arg10)
						choiceIndex = -1;
				}
				break;

			case TEXTBUTTON_CONFIRM:
				doneShowingText = true;
				break;

			case TEXTBUTTON_SCROLLUP:
				scrollOffset -= numTextboxLines;
				goto readjustScrollUp;

			case TEXTBUTTON_SCROLLDOWN:
				scrollOffset += numTextboxLines;
				goto readjustScrollDown;

			case TEXTBUTTON_SCROLLUP_ONELINE:
				scrollOffset--;
				goto readjustScrollUp;

			case TEXTBUTTON_SCROLLDOWN_ONELINE:
				scrollOffset++;
				goto readjustScrollDown;

			case TEXTBUTTON_GOTO_TOP:
				scrollOffset = 0;
				goto readjustScrollUp;

			case TEXTBUTTON_GOTO_BOTTOM:
				scrollOffset = numTextLines - numTextboxLines;
				goto readjustScrollDown;

readjustScrollUp:
				enableMenuButtons(1 << TEXTBUTTON_SCROLLDOWN);
				if (scrollOffset < 0)
					scrollOffset = 0;
				if (scrollOffset == 0)
					disableMenuButtons(1 << TEXTBUTTON_SCROLLUP);
				goto readjustScroll;

readjustScrollDown:
				enableMenuButtons(1 << TEXTBUTTON_SCROLLUP);
				if (scrollOffset >= numTextLines)
					scrollOffset -= numTextboxLines;
				if (scrollOffset > numTextLines-1)
					scrollOffset = numTextLines-1;
				if (scrollOffset+numTextboxLines >= numTextLines)
					disableMenuButtons(1 << TEXTBUTTON_SCROLLDOWN);
				goto readjustScroll;

readjustScroll:
				textboxSprite.bitmapChanged = true;
				drawMainText(
						textBitmap,
						numTextLines-scrollOffset,
						numTextboxLines,
						lineFormattedText.c_str() + scrollOffset*(TEXTBOX_WIDTH-2),
						numChoicesWithNames != 0);
				break;

			case TEXTBUTTON_PREVCHOICE:
				choiceIndex--;
				if (!loopChoices && choiceIndex == 0) {
					disableMenuButtons(1 << TEXTBUTTON_PREVCHOICE);
				}
				else {
					if (choiceIndex < 0)
						choiceIndex = numChoices-1;
				}
				enableMenuButtons(1 << TEXTBUTTON_NEXTCHOICE);
				goto reloadText;

			case TEXTBUTTON_NEXTCHOICE:
				enableMenuButtons(1 << TEXTBUTTON_PREVCHOICE);
				choiceIndex++;
				if (!loopChoices && choiceIndex == numChoices-1) {
					disableMenuButtons(1 << TEXTBUTTON_NEXTCHOICE);
				}
				else {
					choiceIndex %= numChoices;
				}
				goto reloadText;

reloadText:
				scrollOffset = 0;
				lineFormattedText = readLineFormattedText(textGetter, var, choiceIndex, textBitmap, numTextboxLines, &numTextLines);
				if (numTextLines <= numTextboxLines) {
					setVisibleMenuButtons((1 << TEXTBUTTON_CONFIRM) | (1 << TEXTBUTTON_PREVCHOICE) | (1 << TEXTBUTTON_NEXTCHOICE));
				}
				else {
					setVisibleMenuButtons((1 << TEXTBUTTON_CONFIRM) | (1 << TEXTBUTTON_SCROLLUP) | (1 << TEXTBUTTON_SCROLLDOWN)| (1 << TEXTBUTTON_PREVCHOICE) | (1 << TEXTBUTTON_NEXTCHOICE));
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
		}

		_gfx->setMouseBitmap(oldMouseBitmap);
		_gfx->warpMouse(oldMousePos.x, oldMousePos.y);

		_mouseControllingShip = tmpMouseControllingShip;
		unloadMenuButtons();
		textboxSprite.field16 = true;
		textboxSprite.bitmapChanged = true;

		_gfx->drawAllSprites();
		_gfx->delSprite(&textboxSprite);
		// sub_272B4
	}

	_textboxVar2 = _frameIndex;
	stopPlayingSpeech();
	return choiceIndex;
}

/**
 * Returns the number of lines this string will take up in a textbox.
 */
int StarTrekEngine::getNumTextboxLines(const String &str) {
	const char *text = str.c_str();
	char line[TEXTBOX_WIDTH];

	int lines = 0;

	while (text != nullptr) {
		text = getNextTextLine(text, line, TEXTBOX_WIDTH-2);
		lines++;
	}
	return lines-1;
}

String StarTrekEngine::putTextIntoLines(const String &_text) {
	char line[TEXTBOX_WIDTH];

	const char *text = _text.c_str();
	String output;

	text = getNextTextLine(text, line, TEXTBOX_WIDTH-2);

	while (text != nullptr) {
		int len = strlen(line);
		while (len != TEXTBOX_WIDTH-2) {
			line[len++] = ' ';
			line[len] = '\0';
		}
		output += line;

		text = getNextTextLine(text, line, TEXTBOX_WIDTH-2);
	}

	return output;
}

/**
 * Creates a blank textbox in a TextBitmap, and initializes a sprite to use it.
 */
SharedPtr<TextBitmap> StarTrekEngine::initTextSprite(int *xoffsetPtr, int *yoffsetPtr, byte textColor, int numTextLines, bool withHeader, Sprite *sprite) {
	int linesBeforeTextStart = 2;
	if (withHeader)
		linesBeforeTextStart = 4;

	int xoffset = *xoffsetPtr;
	int yoffset = *yoffsetPtr;

	int textHeight = numTextLines + linesBeforeTextStart;

	SharedPtr<TextBitmap> bitmap(new TextBitmap(TEXTBOX_WIDTH*8, textHeight*8));

	memset(sprite, 0, sizeof(Sprite));
	sprite->drawPriority = 15;
	sprite->drawPriority2 = 8;
	sprite->bitmap = bitmap;
	sprite->textColor = textColor;

	memset(bitmap->pixels, ' ', textHeight*TEXTBOX_WIDTH);

	int varC = SCREEN_WIDTH-1 - xoffset - (bitmap->width + 0x1d) / 2;
	if (varC < 0)
		xoffset += varC;

	varC = xoffset - (bitmap->width + 0x1d) / 2;
	if (varC < 1)
		xoffset -= varC - 1;

	varC = yoffset - (bitmap->height + 0x11) - 20;
	if (varC < 0)
		yoffset -= varC;

	xoffset -= (bitmap->width + 0x1d)/2;
	yoffset -= bitmap->height;

	bitmap->pixels[0] = 0x10;
	memset(&bitmap->pixels[1], 0x11, TEXTBOX_WIDTH-2);
	bitmap->pixels[TEXTBOX_WIDTH-1] = 0x12;

	byte *textAddr = bitmap->pixels + TEXTBOX_WIDTH;

	if (withHeader) {
		textAddr[0] = 0x13;
		textAddr[TEXTBOX_WIDTH-1] = 0x14;
		textAddr += TEXTBOX_WIDTH;

		textAddr[0] = 0x13;
		memset(&textAddr[1], 0x19, TEXTBOX_WIDTH-2);
		textAddr[TEXTBOX_WIDTH-1] = 0x14;
		textAddr += TEXTBOX_WIDTH;
	}

	for (int line=0; line<numTextLines; line++) {
		textAddr[0] = 0x13;
		textAddr[TEXTBOX_WIDTH-1] = 0x14;
		textAddr += TEXTBOX_WIDTH;
	}

	textAddr[0] = 0x15;
	memset(&textAddr[1], 0x16, TEXTBOX_WIDTH-2);
	textAddr[TEXTBOX_WIDTH-1] = 0x17;

	_gfx->addSprite(sprite);
	sprite->drawMode = 3;
	sprite->pos.x = xoffset;
	sprite->pos.y = yoffset;
	sprite->drawPriority = 15;

	*xoffsetPtr = xoffset;
	*yoffsetPtr = yoffset;

	return bitmap;
}

/**
 * Draws the "main" text (everything but the header which includes the speaker) to
 * a TextBitmap.
 */
void StarTrekEngine::drawMainText(SharedPtr<TextBitmap> bitmap, int numTextLines, int numTextboxLines, const String &_text, bool withHeader) {
	byte *dest = bitmap->pixels + TEXTBOX_WIDTH + 1; // Start of 2nd row
	const char *text = _text.c_str();

	if (numTextLines >= numTextboxLines)
		numTextLines = numTextboxLines;

	if (withHeader)
		dest += TEXTBOX_WIDTH*2; // Start of 4th row

	int lineIndex = 0;
	while (lineIndex != numTextLines) {
		memcpy(dest, text, TEXTBOX_WIDTH-2);
		text += TEXTBOX_WIDTH-2;
		dest += TEXTBOX_WIDTH;
		lineIndex++;
	}

	// Fill all remaining blank lines
	while (lineIndex != numTextboxLines) {
		memset(dest, ' ', TEXTBOX_WIDTH-2);
		dest += TEXTBOX_WIDTH;
		lineIndex++;
	}
}

String StarTrekEngine::readLineFormattedText(TextGetterFunc textGetter, uintptr var, int choiceIndex, SharedPtr<TextBitmap> textBitmap, int numTextboxLines, int *numTextLines) {
	String headerText;
	String text = (this->*textGetter)(choiceIndex, var, &headerText);

	if (_textDisplayMode == TEXTDISPLAY_NONE && _sfxEnabled && _sfxWorking) {
		uint32 oldSize = text.size();
		text = playTextAudio(text);
		if (oldSize != text.size())
			_textboxHasMultipleChoices = true;
	}
	else if ((_textDisplayMode == TEXTDISPLAY_WAIT || _textDisplayMode == TEXTDISPLAY_SUBTITLES)
			&& _sfxEnabled && _sfxWorking) {
		text = playTextAudio(text);
	}
	else {
		text = skipTextAudioPrompt(text);
	}

	if (_textboxHasMultipleChoices) {
		*numTextLines = getNumTextboxLines(text);

		bool hasHeader = !headerText.empty();

		String lineFormattedText = putTextIntoLines(text);
		drawMainText(textBitmap, *numTextLines, numTextboxLines, lineFormattedText, hasHeader);

		assert(headerText.size() == TEXTBOX_WIDTH-2);
		memcpy(textBitmap->pixels+TEXTBOX_WIDTH+1, headerText.c_str(), TEXTBOX_WIDTH-2);

		return lineFormattedText;
	}
	else
		return nullptr;

	/* Barebones implementation
	*numTextLines = 1;

	uint numChars = textBitmap->width*textBitmap->height;

	String text = (this->*textGetter)(choiceIndex, var, nullptr);
	while (text.size() < numChars) text += ' ';

	byte *dest = textBitmap->pixels + TEXTBOX_WIDTH + 1;

	for (int y=0; y<*numTextLines; y++) {
		memcpy(dest, text.c_str(), TEXTBOX_WIDTH-2);
		dest += TEXTBOX_WIDTH;
	}

	return text;
	*/
}

/**
 * Text getter for showText which reads choices from an array of pointers.
 * Last element in the array must be an empty string.
 */
String StarTrekEngine::readTextFromArray(int choiceIndex, uintptr data, String *headerTextOutput) {
	const char **textArray = (const char**)data;

	const char *headerText = textArray[0];
	const char *mainText = textArray[choiceIndex+1];

	if (*mainText == '\0')
		return Common::String(); // Technically should be nullptr...

	*headerTextOutput = headerText;
	while (headerTextOutput->size() < TEXTBOX_WIDTH-2)
		*headerTextOutput += ' ';
	return String(mainText);
}

}
