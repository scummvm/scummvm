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
 */

#include "common/stream.h"
#include "graphics/cursorman.h"

#include "startrek/graphics.h"


// List of events that can be returned by handleTextEvents.
enum TextEvent {
	TEXTEVENT_RCLICK_OFFBUTTON = 0,
	TEXTEVENT_ENABLEINPUT,          // Makes buttons selectable (occurs after a delay)
	TEXTEVENT_RCLICK_ONBUTTON,
	TEXTEVENT_LCLICK_OFFBUTTON,
	TEXTEVENT_CONFIRM,
	TEXTEVENT_SCROLLUP,
	TEXTEVENT_SCROLLDOWN,
	TEXTEVENT_PREVCHOICE,
	TEXTEVENT_NEXTCHOICE,
	TEXTEVENT_SCROLLUP_ONELINE,
	TEXTEVENT_SCROLLDOWN_ONELINE,
	TEXTEVENT_GOTO_TOP,
	TEXTEVENT_GOTO_BOTTOM,
	TEXTEVENT_SPEECH_DONE
};

namespace StarTrek {

int Graphics::showText(TextGetterFunc textGetter, int var, int xoffset, int yoffset, int textColor, bool loopChoices, int maxTextLines, int arg10) {
	uint16 tmpTextboxVar1 = _textboxVar1;

	uint32 var7c = 8;
	if (_textboxVar3 > _textboxVar2+1) {
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

		int lines = getNumLines(choiceText);
		if (lines > numTextboxLines)
			numTextboxLines = lines;

		if (!speakerText.empty()) // Technically should check for nullptr
			numChoicesWithNames++;

		numChoices++;
	}

	if (maxTextLines == 0 || maxTextLines > 12)
		maxTextLines = 12;
	if (numTextboxLines > maxTextLines)
		numTextboxLines = maxTextLines;

	if (numChoicesWithNames != 0 && numChoices != numChoicesWithNames)
		error("showText: Not all choices have titles.");

	Sprite textboxSprite;
	SharedPtr<TextBitmap> textBitmap = initTextSprite(&xoffset, &yoffset, textColor, numTextboxLines, numChoicesWithNames, &textboxSprite);

	int choiceIndex = 0;
	int scrollOffset = 0;
	if (tmpTextboxVar1 != 0 && tmpTextboxVar1 != 1 && numChoices == 1
			&& _textboxVar5 != 0 && _textboxVar4 == 0)
		_textboxHasMultipleChoices = false;
	else
		_textboxHasMultipleChoices = true;

	if (tmpTextboxVar1 >= 0 && tmpTextboxVar1 <= 2 && _textboxVar5 == 1 && _textboxVar4 == 0)
		_textboxVar6 = true;
	else
		_textboxVar6 = false;

	int numTextLines;
	String lineFormattedText = readLineFormattedText(textGetter, var, choiceIndex, textBitmap, numTextboxLines, &numTextLines);

	if (lineFormattedText.empty()) { // Technically should check for nullptr
		// TODO
	}
	else {
		loadMenuButtons("textbtns", xoffset+0x96, yoffset-0x11);

		Common::Point oldMousePos = getMousePos();
		SharedPtr<Bitmap> oldMouseBitmap = _mouseBitmap;

		_vm->_system->warpMouse(xoffset + 0xde, yoffset - 0x08);
		setMouseCursor(loadBitmap("pushbtn"));

		uint16 tmpTextboxVar7 = _textboxVar7;
		_textboxVar7 = 0;

		int var80 = (numChoices > 1 ? 0x18 : 0);

		// TODO: sub_288FB function call
		// TODO: sub_28ACA(0x0002);

		if (var7c == 0) {
			// sub_28ACA(0x0001);
		}
		if (loopChoices == 0) {
			// sub_28ACA(0x0008);
		}

		bool doneShowingText = false;

		// Loop until text is done being displayed
		while (!doneShowingText) {
			int textboxReturnCode = handleTextboxEvents(var7c, true);

			if (var7c == 0) {
				clearMenuButtonVar2Bits(0x0001);
			}

			switch(textboxReturnCode) {

			case TEXTEVENT_RCLICK_OFFBUTTON:
			case TEXTEVENT_RCLICK_ONBUTTON:
				if (var7c == 0) {
					doneShowingText = true;
					if (arg10)
						choiceIndex = -1;
				}
				break;

			case TEXTEVENT_CONFIRM:
				doneShowingText = true;
				break;

			case TEXTEVENT_SCROLLUP:
				scrollOffset -= numTextboxLines;
				goto readjustScrollUp;

			case TEXTEVENT_SCROLLDOWN:
				scrollOffset += numTextboxLines;
				goto readjustScrollDown;

			case TEXTEVENT_SCROLLUP_ONELINE:
				scrollOffset--;
				goto readjustScrollUp;

			case TEXTEVENT_SCROLLDOWN_ONELINE:
				scrollOffset++;
				goto readjustScrollDown;

			case TEXTEVENT_GOTO_TOP:
				scrollOffset = 0;
				goto readjustScrollUp;

			case TEXTEVENT_GOTO_BOTTOM:
				scrollOffset = numTextLines - numTextboxLines;
				goto readjustScrollDown;

readjustScrollUp:
				clearMenuButtonVar2Bits(0x0004);
				if (scrollOffset < 0)
					scrollOffset = 0;
				if (scrollOffset == 0)
					setMenuButtonVar2Bits(0x0002);
				goto readjustScroll;

readjustScrollDown:
				clearMenuButtonVar2Bits(0x0002);
				if (scrollOffset >= numTextLines)
					scrollOffset -= numTextboxLines;
				if (scrollOffset > numTextLines-1)
					scrollOffset = numTextLines-1;
				if (scrollOffset+numTextboxLines >= numTextLines)
					setMenuButtonVar2Bits(0x0004);
				goto readjustScroll;

readjustScroll:
				textboxSprite.bitmapChanged = true;
				// sub_225d3(textBitmap, numTextLines-scrollOffset, numTextboxLines, lineFormattedText+scrollOffset*0x18);
				break;

			case TEXTEVENT_PREVCHOICE:
				choiceIndex--;
				if (!loopChoices && choiceIndex == 0) {
					setMenuButtonVar2Bits(0x0008);
				}
				else {
					if (choiceIndex < 0)
						choiceIndex = numChoices-1;
				}
				clearMenuButtonVar2Bits(0x0010);
				goto reloadText;

			case TEXTEVENT_NEXTCHOICE:
				clearMenuButtonVar2Bits(0x0008);
				choiceIndex++;
				if (!loopChoices && choiceIndex == numChoices-1) {
					setMenuButtonVar2Bits(0x0010);
				}
				else {
					choiceIndex %= numChoices;
				}
				goto reloadText;

reloadText:
				scrollOffset = 0;
				lineFormattedText = readLineFormattedText(textGetter, var, choiceIndex, textBitmap, numTextboxLines, &numTextLines);
				if (numTextLines <= numTextboxLines) {
					// sub_288FB(0x0019);
				}
				else {
					// sub_288FB(0x001F);
				}
				clearMenuButtonVar2Bits(0x0004);
				setMenuButtonVar2Bits(0x0002);
				textboxSprite.bitmapChanged = true;
				break;

			case TEXTEVENT_SPEECH_DONE:
				if (numChoices == 1)
					doneShowingText = true;
				break;

			case TEXTEVENT_ENABLEINPUT:
			case TEXTEVENT_LCLICK_OFFBUTTON:
			default:
				break;
			}
		}

		setMouseCursor(oldMouseBitmap);
		_vm->_system->warpMouse(oldMousePos.x, oldMousePos.y);

		_textboxVar7 = tmpTextboxVar7;
		// sub_29326();
		textboxSprite.field16 = 1;
		textboxSprite.bitmapChanged = 1;

		drawAllSprites();
		delSprite(&textboxSprite);
		// sub_272B4
	}

	_textboxVar2 = _textboxVar3;
	// sub_29EE3();
	return choiceIndex;
}

int Graphics::handleTextboxEvents(uint32 arg0, bool arg4) {
	// TODO
	while (true) {
		_vm->pollEvents();
	}
}

String Graphics::tmpFunction(int choiceIndex, int var, String *headerTextOutput) {
	if (headerTextOutput != nullptr)
		*headerTextOutput = "Speaker                 ";

	if (choiceIndex >= 1)
		return NULL;
	return "Text test";
}

String Graphics::readTextFromRdf(int choiceIndex, int rdfVar, String *headerTextOutput) {
	Room *room = _vm->getRoom();

	// FIXME: in original game "rdfVar" is a pointer to the variable holding the offset.
	// Currently treating it as the offset itself.
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
 * Creates a blank textbox in a TextBitmap, and initializes a sprite to use it.
 */
SharedPtr<TextBitmap> Graphics::initTextSprite(int *xoffsetPtr, int *yoffsetPtr, byte textColor, int numTextLines, bool withHeader, Sprite *sprite) {
	int linesBeforeTextStart = 2;
	if (withHeader != 0)
		linesBeforeTextStart = 4;

	int xoffset = *xoffsetPtr;
	int yoffset = *yoffsetPtr;

	int textHeight = numTextLines + linesBeforeTextStart;

	SharedPtr<TextBitmap> bitmap(new TextBitmap(TEXTBOX_WIDTH*8, textHeight*8));

	memset(sprite, 0, sizeof(Sprite));
	sprite->drawPriority = 15;
	sprite->field6 = 8;
	sprite->bitmap = bitmap;
	sprite->textColor = textColor;

	memset(bitmap->pixels, ' ', textHeight*TEXTBOX_WIDTH);

	int varC = SCREEN_WIDTH-1 - xoffset - (bitmap->width+0x1d)/2;
	if (varC < 0)
		xoffset += varC;

	varC = xoffset - (bitmap->width+0x1d)/2;
	if (varC < 1)
		xoffset += varC-1;

	varC = yoffset - (bitmap->height+0x11) - 20;
	if (varC < 0)
		yoffset -= varC;

	xoffset -= (bitmap->width+0x1d)/2;
	yoffset -= bitmap->height;

	bitmap->pixels[0] = 0x10;
	memset(&bitmap->pixels[1], 0x11, TEXTBOX_WIDTH-2);
	bitmap->pixels[TEXTBOX_WIDTH-1] = 0x12;

	byte *textAddr = bitmap->pixels+TEXTBOX_WIDTH;

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

	addSprite(sprite);
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
void Graphics::drawMainText(SharedPtr<TextBitmap> bitmap, int numTextLines, int numTextboxLines, const String &_text, bool withHeader) {
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

	debug("%d, %d\n", numTextLines, numTextboxLines);
	// Fill all remaining blank lines
	while (lineIndex != numTextboxLines) {
		memset(dest, ' ', TEXTBOX_WIDTH-2);
		dest += TEXTBOX_WIDTH;
		lineIndex++;
	}
}

/**
 * Returns the number of lines this string will take up in a textbox.
 */
int Graphics::getNumLines(const String &str) {
	const char *text = str.c_str();
	char line[TEXTBOX_WIDTH];

	int lines = 0;

	while (text != nullptr) {
		text = getNextTextLine(text, line, TEXTBOX_WIDTH-2);
		lines++;
	}
	return lines-1;
}

void Graphics::getTextboxHeader(String *headerTextOutput, String speakerText, int choiceIndex) {
	String header = speakerText;

	if (choiceIndex != 0)
		header += String::format(" choice %d", choiceIndex);

	if (header.size() > TEXTBOX_WIDTH-2)
		header.erase(TEXTBOX_WIDTH-2);
	while (header.size() < TEXTBOX_WIDTH-2)
		header += ' ';

	*headerTextOutput = header;
}

String Graphics::readLineFormattedText(TextGetterFunc textGetter, int var, int choiceIndex, SharedPtr<TextBitmap> textBitmap, int numTextboxLines, int *numTextLines) {
	String headerText;
	String text = (this->*textGetter)(choiceIndex, var, &headerText);

	if (_textboxVar1 == 2 && _textboxVar5 == 1 && _textboxVar4 == 1) {
		uint32 oldSize = text.size();
		text = playTextAudio(text);
		if (oldSize != text.size())
			_textboxHasMultipleChoices = true;
	}
	else if ((_textboxVar1 == 0 || _textboxVar1 == 1) && _textboxVar5 == 1 && _textboxVar4 == 1) {
		text = playTextAudio(text);
	}
	else {
		text = skipTextAudioPrompt(text);
	}

	if (_textboxHasMultipleChoices) {
		*numTextLines = getNumLines(text);

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

String Graphics::putTextIntoLines(const String &_text) {
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
 * Gets one line of text (does not include words that won't fit).
 * Returns position of text to continue from, or nullptr if done.
 */
const char *Graphics::getNextTextLine(const char *text, char *lineOutput, int lineWidth) {
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

String Graphics::skipTextAudioPrompt(const String &str) {
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

String Graphics::playTextAudio(const String &str) {
	// TODO
	return skipTextAudioPrompt(str);
}

void Graphics::loadMenuButtons(String mnuFilename, int xpos, int ypos) {
	SharedPtr<Menu> oldMenu = _activeMenu;
	_activeMenu = SharedPtr<Menu>(new Menu());
	_activeMenu->nextMenu = oldMenu;

	SharedPtr<FileStream> stream = _vm->openFile(mnuFilename + ".MNU");

	_activeMenu->menuFile = stream;
	_activeMenu->numButtons = _activeMenu->menuFile->size()/16;

	for (int i=0; i<_activeMenu->numButtons; i++) {
		memset(&_activeMenu->sprites[i], 0, sizeof(Sprite));
		addSprite(&_activeMenu->sprites[i]);
		_activeMenu->sprites[i].drawMode = 2;

		char bitmapBasename[11];
		stream->seek(i*16, SEEK_SET);
		stream->read(bitmapBasename, 10);
		for (int j=0; j<10; j++) {
			if (bitmapBasename[j] == ' ')
				bitmapBasename[j] = '\0';
		}
		bitmapBasename[10] = '\0';

		_activeMenu->sprites[i].bitmap = loadBitmap(bitmapBasename);
		_activeMenu->sprites[i].pos.x = stream->readUint16() + xpos;
		_activeMenu->sprites[i].pos.y = stream->readUint16() + ypos;
		_activeMenu->retvals[i] = stream->readUint16();

		_activeMenu->sprites[i].field6 = 8;
	}
}

// 0x0002: Disable scroll up
// 0x0004: Disable scroll down
// 0x0008: Disable prev choice
// 0x0010: Disable next choice
void Graphics::setMenuButtonVar2Bits(uint32 bits) {
	// TODO
}

void Graphics::clearMenuButtonVar2Bits(uint32 bits) {
	// TODO
}

}
