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


// Special events that can be returned by handleMenuEvents.
enum MenuEvent {
	MENUEVENT_RCLICK_OFFBUTTON = -4,
	MENUEVENT_ENABLEINPUT,          // Makes buttons selectable (occurs after a delay)
	MENUEVENT_RCLICK_ONBUTTON,
	MENUEVENT_LCLICK_OFFBUTTON
};

// Buttons for standard text display
enum TextButtons {
	TEXTBUTTON_CONFIRM = 0,
	TEXTBUTTON_SCROLLUP,
	TEXTBUTTON_SCROLLDOWN,
	TEXTBUTTON_PREVCHOICE,
	TEXTBUTTON_NEXTCHOICE,
	TEXTBUTTON_SCROLLUP_ONELINE,
	TEXTBUTTON_SCROLLDOWN_ONELINE,
	TEXTBUTTON_GOTO_TOP,
	TEXTBUTTON_GOTO_BOTTOM,
	TEXTBUTTON_SPEECH_DONE // "Virtual" button?
};

// Buttons for option menu (corresponding to button indices, not button retvals, which are
// different for some reason)
enum OptionMenuButtons {
	OPTIONBUTTON_TEXT,
	OPTIONBUTTON_SAVE,
	OPTIONBUTTON_LOAD,
	OPTIONBUTTON_ENABLEMUSIC,
	OPTIONBUTTON_DISABLEMUSIC,
	OPTIONBUTTON_ENABLESFX,
	OPTIONBUTTON_DISABLESFX,
	OPTIONBUTTON_QUIT
};

namespace StarTrek {

int Graphics::showText(TextGetterFunc textGetter, uintptr var, int xoffset, int yoffset, int textColor, bool loopChoices, int maxTextLines, int arg10) {
	int16 tmpTextDisplayMode = _textDisplayMode;

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
			&& numChoices == 1 && _vm->_sfxEnabled && !_vm->_sfxWorking)
		_textboxHasMultipleChoices = false;
	else
		_textboxHasMultipleChoices = true;

	if (tmpTextDisplayMode >= TEXTDISPLAY_WAIT && tmpTextDisplayMode <= TEXTDISPLAY_NONE
			&& _vm->_sfxEnabled && !_vm->_sfxWorking)
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

		Common::Point oldMousePos = getMousePos();
		SharedPtr<Bitmap> oldMouseBitmap = _mouseBitmap;

		_vm->_system->warpMouse(xoffset + 0xde, yoffset - 0x08);
		setMouseCursor(loadBitmap("pushbtn"));

		bool tmpMouseControllingShip = _vm->_mouseControllingShip;
		_vm->_mouseControllingShip = false;

		int var80 = (numChoices > 1 ? 0x18 : 0);

		// TODO: sub_288FB function call
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
					// sub_288FB(0x0019);
				}
				else {
					// sub_288FB(0x001F);
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

		setMouseCursor(oldMouseBitmap);
		_vm->_system->warpMouse(oldMousePos.x, oldMousePos.y);

		_vm->_mouseControllingShip = tmpMouseControllingShip;
		unloadMenuButtons();
		textboxSprite.field16 = true;
		textboxSprite.bitmapChanged = true;

		drawAllSprites();
		delSprite(&textboxSprite);
		// sub_272B4
	}

	_textboxVar2 = _textboxVar3;
	_vm->stopPlayingSpeech();
	return choiceIndex;
}

/**
 * This returns either a special menu event (negative number) or the retval of the button
 * clicked (usually an index, always positive).
 */
int Graphics::handleMenuEvents(uint32 ticksUntilClickingEnabled, bool arg4) {
	// TODO: finish

	uint32 tickWhenClickingEnabled = _vm->_clockTicks + ticksUntilClickingEnabled;

	while (true) {
		TrekEvent event;
		while (_vm->popNextEvent(&event)) {
			switch(event.type) {

			case TREKEVENT_TICK: {
			case TREKEVENT_MOUSEMOVE: // FIXME: actual game only uses TICK event here
				Common::Point mousePos = getMousePos();
				int buttonIndex = getMenuButtonAt(*_activeMenu, mousePos.x, mousePos.y);
				if (buttonIndex != -1) {
					if (_activeMenu->disabledButtons & (1<<buttonIndex))
						buttonIndex = -1;
				}

				if (buttonIndex != _activeMenu->selectedButton) {
					if (_activeMenu->selectedButton != -1) {
						Sprite &spr = _activeMenu->sprites[_activeMenu->selectedButton];
						drawMenuButtonOutline(spr.bitmap, 0x00);
						spr.bitmapChanged = true;
					}
					if (buttonIndex != -1) {
						Sprite &spr = _activeMenu->sprites[buttonIndex];
						drawMenuButtonOutline(spr.bitmap, 0xda);
						spr.bitmapChanged = true;
					}
					_activeMenu->selectedButton = buttonIndex;
				}
				// Not added: updating mouse position (scummvm handles that)

				// sub_10492();
				// sub_10A91();
				drawAllSprites();
				// sub_10BE7();
				// sub_2A4B1();

				if (_vm->_finishedPlayingSpeech != 0) {
					_vm->_finishedPlayingSpeech = 0;
					if (_textDisplayMode != TEXTDISPLAY_WAIT) {
						return TEXTBUTTON_SPEECH_DONE;
					}
				}
				// sub_1E88C();
				_textboxVar3++;

				if (ticksUntilClickingEnabled != 0 && _vm->_clockTicks >= tickWhenClickingEnabled)
					return MENUEVENT_ENABLEINPUT;
				break;
			}

			case TREKEVENT_LBUTTONDOWN:
				if (_activeMenu->selectedButton != -1) {
					_vm->playSoundEffectIndex(0x10);
					return _activeMenu->retvals[_activeMenu->selectedButton];
				}
				else {
					Common::Point mouse = getMousePos();
					if (getMenuButtonAt(*_activeMenu, mouse.x, mouse.y) == -1) {
						_vm->playSoundEffectIndex(0x10);
						return MENUEVENT_LCLICK_OFFBUTTON;
					}
				}
				break;

			case TREKEVENT_RBUTTONDOWN:
				// TODO
				break;

			case TREKEVENT_KEYDOWN:
				// TODO
				break;

			default:
				break;
			}
		}
	}
}

/**
 * Text getter for showText which reads from an rdf file.
 */
String Graphics::readTextFromRdf(int choiceIndex, uintptr data, String *headerTextOutput) {
	Room *room = _vm->getRoom();

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
String Graphics::readTextFromBuffer(int choiceIndex, uintptr data, String *headerTextOutput) {
	char buf[TEXTBOX_WIDTH];
	memcpy(buf, (byte*)data, TEXTBOX_WIDTH-2);
	buf[TEXTBOX_WIDTH-2] = '\0';

	*headerTextOutput = String(buf);

	char *text = (char*)data+TEXTBOX_WIDTH-2;
	return String(text);
}

/**
 * Text getter for showText which reads choices from an array of pointers.
 * Last element in the array must be an empty string.
 */
String Graphics::readTextFromArray(int choiceIndex, uintptr data, String *headerTextOutput) {
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

/**
 * Creates a blank textbox in a TextBitmap, and initializes a sprite to use it.
 */
SharedPtr<TextBitmap> Graphics::initTextSprite(int *xoffsetPtr, int *yoffsetPtr, byte textColor, int numTextLines, bool withHeader, Sprite *sprite) {
	int linesBeforeTextStart = 2;
	if (withHeader)
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

String Graphics::readLineFormattedText(TextGetterFunc textGetter, uintptr var, int choiceIndex, SharedPtr<TextBitmap> textBitmap, int numTextboxLines, int *numTextLines) {
	String headerText;
	String text = (this->*textGetter)(choiceIndex, var, &headerText);

	if (_textDisplayMode == TEXTDISPLAY_NONE && _vm->_sfxEnabled && _vm->_sfxWorking) {
		uint32 oldSize = text.size();
		text = playTextAudio(text);
		if (oldSize != text.size())
			_textboxHasMultipleChoices = true;
	}
	else if ((_textDisplayMode == TEXTDISPLAY_WAIT || _textDisplayMode == TEXTDISPLAY_SUBTITLES)
			&& _vm->_sfxEnabled && _vm->_sfxWorking) {
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

/**
 * Plays an audio prompt, if it exists, and returns the string starting at the end of the
 * prompt.
 */
String Graphics::playTextAudio(const String &str) {
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
	_vm->playSpeech(soundFile);

	return String(text+1);
}

/**
 * Returns the index of the button at the given position, or -1 if none.
 */
int Graphics::getMenuButtonAt(const Menu &menu, int x, int y) {
	for (int i = 0; i < menu.numButtons; i++) {
		const Sprite &spr = menu.sprites[i];

		if (spr.drawMode != 2)
			continue;

		int left = spr.pos.x - spr.bitmap->xoffset;
		int top = spr.pos.y - spr.bitmap->yoffset;

		// Oddly, this doesn't account for x/yoffset...
		int right = spr.pos.x + spr.bitmap->width - 1;
		int bottom = spr.pos.y + spr.bitmap->height - 1;

		if (x >= left && x <= right && y >= top && y <= bottom)
			return i;
	}

	return -1;
}

/**
 * Draws or removes the outline on menu buttons when the cursor hovers on them, or leaves
 * them.
 */
void Graphics::drawMenuButtonOutline(SharedPtr<Bitmap> bitmap, byte color) {
	int lineWidth = bitmap->width-2;
	int offsetToBottom = (bitmap->height-3)*bitmap->width;

	byte *dest = bitmap->pixels + bitmap->width + 1;

	while (lineWidth--) {
		*dest = color;
		*(dest+offsetToBottom) = color;
		dest++;
	}

	int lineHeight = bitmap->height - 2;
	int offsetToRight = bitmap->width - 3;

	dest = bitmap->pixels + bitmap->width + 1;

	while (lineHeight--) {
		*dest = color;
		*(dest+offsetToRight) = color;
		dest += bitmap->width;
	}
}

/**
 * Loads a .MNU file, which is a list of buttons to display.
 */
void Graphics::loadMenuButtons(String mnuFilename, int xpos, int ypos) {
	if (_activeMenu == nullptr)
		_keyboardControlsMouseOutsideMenu = _vm->_keyboardControlsMouse;

	SharedPtr<Menu> oldMenu = _activeMenu;
	_activeMenu = SharedPtr<Menu>(new Menu());
	_activeMenu->nextMenu = oldMenu;

	SharedPtr<FileStream> stream = _vm->openFile(mnuFilename + ".MNU");

	_activeMenu->menuFile = stream;
	_activeMenu->numButtons = _activeMenu->menuFile->size() / 16;

	for (int i = 0; i < _activeMenu->numButtons; i++) {
		memset(&_activeMenu->sprites[i], 0, sizeof(Sprite));
		addSprite(&_activeMenu->sprites[i]);
		_activeMenu->sprites[i].drawMode = 2;

		char bitmapBasename[11];
		stream->seek(i * 16, SEEK_SET);
		stream->read(bitmapBasename, 10);
		for (int j = 0; j < 10; j++) {
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

	if (_activeMenu->retvals[_activeMenu->numButtons - 1] == 0) {
		// Set default retvals for buttons
		for (int i = 0; i < _activeMenu->numButtons; i++)
			_activeMenu->retvals[i] = i;
	}

	_activeMenu->selectedButton = -1;
	_activeMenu->disabledButtons = 0;
	_vm->_keyboardControlsMouse = false;
}

void Graphics::unloadMenuButtons() {
	if (_activeMenu->selectedButton != -1)
		drawMenuButtonOutline(_activeMenu->sprites[_activeMenu->selectedButton].bitmap, 0x00);

	for (int i = 0; i < _activeMenu->numButtons; i++) {
		Sprite *sprite = &_activeMenu->sprites[i];
		if (sprite->drawMode == 2) {
			sprite->field16 = true;
			sprite->bitmapChanged = true;
		}
	}

	drawAllSprites();

	for (int i = 0; i < _activeMenu->numButtons; i++) {
		Sprite *sprite = &_activeMenu->sprites[i];
		sprite->bitmap.reset();
		if (sprite->drawMode == 2)
			delSprite(sprite);
	}

	_activeMenu = _activeMenu->nextMenu;

	if (_activeMenu == nullptr)
		_vm->_keyboardControlsMouse = _keyboardControlsMouseOutsideMenu;
}

/**
 * Disables the given bitmask of buttons.
 */
void Graphics::disableMenuButtons(uint32 bits) {
	_activeMenu->disabledButtons |= bits;
	if (_activeMenu->selectedButton != -1
			&& (_activeMenu->disabledButtons & (1 << _activeMenu->selectedButton))) {
		Sprite *sprite = &_activeMenu->sprites[_activeMenu->selectedButton];
		drawMenuButtonOutline(sprite->bitmap, 0x00);

		sprite->bitmapChanged = true;
		_activeMenu->selectedButton = -1;
	}
}

void Graphics::enableMenuButtons(uint32 bits) {
	_activeMenu->disabledButtons &= ~bits;
}

/**
 * This chooses a sprite from the list to place the mouse cursor at. The sprite it chooses
 * may be, for example, the top-leftmost one in the list. Exact behaviour is determined by
 * the "mode" parameter.
 *
 * If "containMouseSprite" is a valid index, it's ensured that the mouse is contained
 * within it. "mode" should be -1 in this case.
 */
void Graphics::choseMousePositionFromSprites(Sprite *sprites, int numSprites, int containMouseSprite, int mode) {
	uint16 mouseX1 = 0x7fff; // Candidate positions to warp mouse to
	uint16 mouseY1 = 0x7fff;
	uint16 mouseX2 = 0x7fff;
	uint16 mouseY2 = 0x7fff;

	Common::Point mousePos = getMousePos();

	// Ensure the cursor is contained within one of the sprites
	if (containMouseSprite >= 0 && containMouseSprite < numSprites) {
		Common::Rect rect = sprites[containMouseSprite].getRect();

		if (mousePos.x < rect.left || mousePos.x >= rect.right
				|| mousePos.y < rect.top || mousePos.y >= rect.bottom) {
			mousePos.x = (rect.left + rect.right) / 2;
			mousePos.y = (rect.top + rect.bottom) / 2;
		}
	}

	// Choose a sprite to warp the cursor to
	for (int i = 0; i < numSprites; i++) {
		Sprite *sprite = &sprites[i];
		if (sprite->drawMode != 2)
			continue;

		Common::Rect rect = sprite->getRect();

		int hCenter = (rect.left + rect.right) / 2;
		int vCenter = (rect.top + rect.bottom) / 2;

		// Choose which sprite is closest based on certain criteria?
		switch(mode) {
		case 0: // Choose topmost, leftmost sprite that's below the cursor
			if (((vCenter == mousePos.y && hCenter > mousePos.x) || vCenter > mousePos.y)
					&& (vCenter < mouseY1 || (vCenter == mouseY1 && hCenter < mouseX1))) {
				mouseX1 = hCenter;
				mouseY1 = vCenter;
			}
			// fall through

		case 4: // Choose topmost, leftmost sprite
			if (vCenter < mouseY2 || (vCenter == mouseY2 && hCenter < mouseX2)) {
				mouseX2 = hCenter;
				mouseY2 = vCenter;
			}
			break;

		case 1: // Choose bottommost, rightmost sprite that's above the cursor
			if (((vCenter == mousePos.y && hCenter < mousePos.x) || vCenter < mousePos.y)
					&& (mouseY1 == 0x7fff || vCenter > mouseY1
						|| (vCenter == mouseY1 && hCenter > mouseX1))) {
				mouseX1 = hCenter;
				mouseY1 = vCenter;
			}
			// fall through

		case 5: // Choose bottommost, rightmost sprite
			if (mouseY2 == 0x7fff || vCenter > mouseY2
					|| (vCenter == mouseY2 && hCenter > mouseX2)) {
				mouseX2 = hCenter;
				mouseY2 = vCenter;
			}
			break;

		case 2:
			// This seems broken... OR condition on first line has no affect on the logic...
			if ((vCenter < mousePos.y || (vCenter == mouseY1 && hCenter == mousePos.x))
					&& (mouseX1 == 0x7fff || vCenter >= mouseY1)) {
				mouseX1 = hCenter;
				mouseY1 = vCenter;
				debug("Try %d %d", mouseX1, mouseY1);
			}
			if (mouseX2 == 0x7fff || vCenter > mouseY2
					|| (hCenter == mouseX2 && vCenter == mouseY2)) {
				mouseX2 = hCenter;
				mouseY2 = vCenter;
			}
			break;

		case 3:
			// Similar to above...
			if ((vCenter > mousePos.y || (vCenter == mouseY1 && hCenter == mousePos.x))
					&& (mouseX1 == 0x7fff || vCenter <= mouseY1)) {
				mouseX1 = hCenter;
				mouseY1 = vCenter;
			}
			if (mouseX2 == 0x7fff || vCenter < mouseY2
					|| (hCenter == mouseX2 && vCenter == mouseY2)) {
				mouseX2 = hCenter;
				mouseY2 = vCenter;
			}
			break;
		}
	}

	// Warp mouse to one of the coordinates, if one is valid
	if (mouseX1 != 0x7fff) {
		mousePos.x = mouseX1;
		mousePos.y = mouseY1;
	}
	else if (mouseX2 != 0x7fff) {
		mousePos.x = mouseX2;
		mousePos.y = mouseY2;
	}

	_vm->_system->warpMouse(mousePos.x, mousePos.y);

}

void Graphics::showOptionsMenu(int x, int y) {
	bool tmpMouseControllingShip = _vm->_mouseControllingShip;
	_vm->_mouseControllingShip = false;

	Common::Point oldMousePos = getMousePos();
	SharedPtr<Bitmap> oldMouseBitmap = _mouseBitmap;

	setMouseCursor(loadBitmap("options"));
	loadMenuButtons("options", x, y);

	uint32 disabledButtons = 0;
	if (_vm->_musicWorking) {
		if (_vm->_musicEnabled)
			disabledButtons |= (1 << OPTIONBUTTON_ENABLEMUSIC);
		else
			disabledButtons |= (1 << OPTIONBUTTON_DISABLEMUSIC);
	}
	else
		disabledButtons |= (1 << OPTIONBUTTON_ENABLEMUSIC) | (1 << OPTIONBUTTON_DISABLEMUSIC);

	if (_vm->_sfxWorking) {
		if (_vm->_sfxEnabled)
			disabledButtons |= (1 << OPTIONBUTTON_ENABLESFX);
		else
			disabledButtons |= (1 << OPTIONBUTTON_DISABLESFX);
	}
	else
		disabledButtons |= (1 << OPTIONBUTTON_ENABLESFX) | (1 << OPTIONBUTTON_DISABLESFX);

	disableMenuButtons(disabledButtons);
	choseMousePositionFromSprites(_activeMenu->sprites, _activeMenu->numButtons, -1, 4);
	int event = handleMenuEvents(0, false);

	unloadMenuButtons();
	_vm->_mouseControllingShip = tmpMouseControllingShip;
	setMouseCursor(oldMouseBitmap);

	if (event != MENUEVENT_LCLICK_OFFBUTTON && event != MENUEVENT_RCLICK_OFFBUTTON)
		_vm->_system->warpMouse(oldMousePos.x, oldMousePos.y);


	// Can't use OPTIONBUTTON constants since the button retvals differ from the button
	// indices...
	switch(event) {
	case 0: // Save
		showSaveMenu();
		break;
	case 1: // Load
		showLoadMenu();
		break;
	case 2: // Enable music
		_vm->_sound->setMusicEnabled(true);
		break;
	case 3: // Disable music
		_vm->_sound->setMusicEnabled(false);
		break;
	case 4: // Enable sfx
		_vm->_sound->setSfxEnabled(true);
		break;
	case 5: // Disable sfx
		_vm->_sound->setSfxEnabled(false);
		break;
	case 6: // Quit
		showQuitGamePrompt(20, 20);
		break;
	case 7: // Text
		showTextConfigurationMenu(true);
		break;
	default:
		break;
	}
}

void Graphics::showSaveMenu() {
	// TODO
}

void Graphics::showLoadMenu() {
	// TODO
}

void Graphics::showQuitGamePrompt(int x, int y) {
	const char *options[] = {
		"Quit Game",
		"#GENE\\GENER028#Yes, quit the game.",
		"#GENE\\GENER008#No, do not quit the game.",
		""
	};

	if (_vm->_inQuitGameMenu)
		return;

	_vm->_inQuitGameMenu = true;
	int val = showText(&Graphics::readTextFromArray, (uintptr)options, x, y, 0xb0, true, 0, 1);
	_vm->_inQuitGameMenu = false;

	if (val == 0) {
		// sub_1e70d();
		_vm->_system->quit();
	}
}

/**
 * This can be called from startup or from the options menu.
 * On startup, this tries to load the setting without user input.
 */
void Graphics::showTextConfigurationMenu(bool fromOptionMenu) {
	const char *options[] = { // TODO: languages...
		"Text display",
		"Text subtitles.",
		"Display text until you press enter.",
		"No text displayed.",
		""
	};

	int val;
	if (fromOptionMenu || (val = loadTextDisplayMode()) == -1) {
		val = showText(&Graphics::readTextFromArray, (uintptr)options, 20, 30, 0xb0, true, 0, 1);
		saveTextDisplayMode(val);
	}

	switch(val) {
	case 0:
		_textDisplayMode = TEXTDISPLAY_SUBTITLES;
		break;
	case 1:
		_textDisplayMode = TEXTDISPLAY_WAIT;
		break;
	case 2:
		_textDisplayMode = TEXTDISPLAY_NONE;
		break;
	}
}

int Graphics::loadTextDisplayMode() {
	return -1; // TODO
}
void Graphics::saveTextDisplayMode(int value) {
	// TODO;
}

}
