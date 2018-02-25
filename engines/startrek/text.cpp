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

#include "startrek/graphics.h"

namespace StarTrek {

int Graphics::showText(TextGetterFunc textGetter, int var, int xoffset, int yoffset, int textColor, int argC, int maxTextLines, int arg10) {
	uint16 tmpTextboxVar1 = _textboxVar1;

	uint32 var7c = 8;
	if (_textboxVar3 > _textboxVar2+1) {
		var7c = 0x10;
	}

	int numChoicesWithNames = 0;
	int numTextboxLines = 0;
	int numChoices = 0;
	Common::String speakerText;

	while(true) {
		Common::String choiceText = (this->*textGetter)(numChoices, var, &speakerText);
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
	int var28 = 0;
	if (tmpTextboxVar1 != 0 && tmpTextboxVar1 != 1 && numChoices == 1
			&& _textboxVar5 != 0 && _textboxVar4 == 0)
		_textboxHasMultipleChoices = false;
	else
		_textboxHasMultipleChoices = true;

	if (tmpTextboxVar1 >= 0 && tmpTextboxVar1 <= 2 && _textboxVar5 == 1 && _textboxVar4 == 0)
		_textboxVar6 = true;
	else
		_textboxVar6 = false;

	int numPrintedLines;
	Common::String lineFormattedText = readLineFormattedText(textGetter, var, choiceIndex, textBitmap, numTextboxLines, &numPrintedLines);

	if (lineFormattedText.empty()) { // Technically should check for nullptr
		// TODO
	}
	else {
		loadMenuButtons("textbtns", xoffset+0x96, yoffset-0x11);

		Common::Point oldMousePos = _mousePos;
		SharedPtr<Bitmap> oldMouseBitmap = _mouseSprite.bitmap;

		_mousePos.x = xoffset + 0xde;
		_mousePos.y = yoffset - 0x08;
		_mouseSprite.pos = _mousePos;
		_mouseSprite.drawPriority = 15;

		_mouseSprite.setBitmap(loadBitmap("pushbtn"));
		warpMousePosition(_mousePos.x, _mousePos.y);

		uint16 tmpTextboxVar7 = _textboxVar7;
		_textboxVar7 = 0;

		int var80 = (numChoices > 1 ? 0x18 : 0);

		// TODO: sub_288FB function call
		// TODO: sub_28ACA(0x0002);

		if (var7c == 0) {
			// sub_28ACA(0x0001);
		}
		if (argC == 0) {
			// sub_28ACA(0x0008);
		}

		bool doneShowingText = false;

		while (!doneShowingText) {
			// TODO
			_vm->pollEvents();
		}

		_mousePos = oldMousePos;
		_mouseSprite.pos = _mousePos;
		_mouseSprite.drawPriority = 15;
		_mouseSprite.setBitmap(oldMouseBitmap);
		warpMousePosition(_mousePos.x, _mousePos.y);

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

Common::String Graphics::tmpFunction(int choiceIndex, int var, Common::String *speakerTextOutput) {
	if (speakerTextOutput != nullptr)
		*speakerTextOutput = "Speaker";

	if (choiceIndex >= 1)
		return NULL;
	return "Text test";
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

Common::String Graphics::skipOverAudioPrompt(const Common::String &str) {
	// TODO
	return str;
}

int Graphics::getNumLines(const Common::String &str) {
	// TODO
	return 1;
}

Common::String Graphics::readLineFormattedText(TextGetterFunc textGetter, int var, int choiceIndex, SharedPtr<TextBitmap> textBitmap, int numTextboxLines, int *numPrintedLines) {
	// TODO
	*numPrintedLines = 1;

	uint numChars = textBitmap->width*textBitmap->height;

	Common::String text = (this->*textGetter)(choiceIndex, var, nullptr);
	while (text.size() < numChars) text += ' ';

	byte *dest = textBitmap->pixels + TEXTBOX_WIDTH + 1;

	for (int y=0; y<*numPrintedLines; y++) {
		memcpy(dest, text.c_str(), TEXTBOX_WIDTH-2);
		dest += TEXTBOX_WIDTH;
	}

	return text;
}

void Graphics::loadMenuButtons(Common::String mnuFilename, int xpos, int ypos) {
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

void Graphics::warpMousePosition(int x, int y) {
	// TODO
}

}
