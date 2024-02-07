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

#include "common/str.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "common/config-manager.h"
#include "common/text-to-speech.h"

#include "supernova/imageid.h"
#include "supernova/resman.h"
#include "supernova/game-manager.h"
#include "supernova/screen.h"
#include "supernova/supernova.h"

#include "supernova/screenstatic.cpp"

namespace Supernova {

ScreenBuffer::ScreenBuffer()
	: _x(0)
	, _y(0)
	, _width(0)
	, _height(0)
	, _pixels(nullptr) {
}

ScreenBufferStack::ScreenBufferStack()
	: _last(_buffer) {
}

void ScreenBufferStack::push(int x, int y, int width, int height) {
	if (_last == ARRAYEND(_buffer))
		return;

	Graphics::Surface *screenSurface = g_system->lockScreen();

	if (x < 0) {
		width += x;
		x = 0;
	}

	if (x + width > screenSurface->w)
		width = screenSurface->w - x;

	if (y < 0) {
		height += y;
		y = 0;
	}

	if (y + height > screenSurface->h)
		height = screenSurface->h - y;

	_last->_pixels = new byte[width * height];
	byte *pixels = _last->_pixels;
	const byte *screen = static_cast<const byte *>(screenSurface->getBasePtr(x, y));
	for (int i = 0; i < height; ++i) {
		Common::copy(screen, screen + width, pixels);
		screen += screenSurface->pitch;
		pixels += width;
	}
	g_system->unlockScreen();

	_last->_x = x;
	_last->_y = y;
	_last->_width = width;
	_last->_height = height;

	++_last;
}

void ScreenBufferStack::restore() {
	if (_last == _buffer)
		return;

	--_last;
	g_system->copyRectToScreen(_last->_pixels, _last->_width, _last->_x,
	                           _last->_y, _last->_width, _last->_height);

	delete[] _last->_pixels;
}

Marquee::Marquee(Screen *screen, MarqueeId id, const char *text)
	: _text(text)
	, _textBegin(text)
	, _delay(0)
	, _loop(false)
	, _oldColor(nullptr)
	, _screen(screen) {
	if (_screen->_vm->_MSPart == 1) {
		_color = kColorLightBlue;
		if (id == kMarqueeIntro) {
			_y = 191;
			_loop = true;
		} else if (id == kMarqueeOutro) {
			_y = 1;
		}
	} else if (_screen->_vm->_MSPart == 2) {
		byte purple[3] = {0x9b, 0x00, 0xfb};
		_oldColor = new byte[3];
		_screen->_vm->_system->getPaletteManager()->grabPalette(_oldColor, kColorPurple, 1);
		_screen->_vm->_system->getPaletteManager()->setPalette(purple, kColorPurple, 1);
		_color = kColorPurple;
		if (id == kMarqueeIntro) {
			_y = 191;
			_loop = true;
		} else if (id == kMarqueeOutro) {
			_y = 191;
		}
	}

	_textWidth = Screen::textWidth(_text);
	_x = kScreenWidth / 2 - _textWidth / 2;
	_screen->_textCursorX = _x;
	_screen->_textCursorY = _y;
	_screen->_textColor = _color;
}

Marquee::~Marquee() {
	if (_screen->_vm->_MSPart == 2) {
		_screen->_vm->_system->getPaletteManager()->setPalette(_oldColor, kColorPurple, 1);
		delete[] _oldColor;
	}
}

void Marquee::clearText() {
	_screen->renderBox(_x, _y - 1, _textWidth + 1, 9, kColorBlack);
}

void Marquee::reset() {
	_text = _textBegin;
	clearText();
	_textWidth = Screen::textWidth(_text);
	_x = kScreenWidth / 2 - _textWidth / 2;
	_screen->_textCursorX = _x;
}

bool Marquee::renderCharacter() {
	if (_delay != 0) {
		_delay--;
		return true;
	}

	switch (*_text) {
	case '\233':
		if (_loop) {
			_loop = false;
			_text = _textBegin;
			clearText();
			_textWidth = Screen::textWidth(_text);
			_x = kScreenWidth / 2 - _textWidth / 2;
			_screen->_textCursorX = _x;
		} else
			return false;
		break;
	case '\1':
		clearText();
		_text++;
		_textWidth = Screen::textWidth(_text);
		_x = kScreenWidth / 2 - _textWidth / 2;
		_screen->_textCursorX = _x;
		if (_screen->_vm->_MSPart == 1) {
			_color = kColorLightBlue;
			_screen->_textColor = _color;
		} else if (_screen->_vm->_MSPart == 2) {
			_color = kColorPurple;
			_screen->_textColor = _color;
		}
		break;
	case '^':
		_color = kColorLightYellow;
		_screen->_textColor = _color;
		_text++;
		break;
	case '#':
		_delay = 50;
		_text++;
		break;
	default:
		_screen->renderText((uint16)*_text++);
		_delay = 1;
		break;
	}
	return true;
}

Screen::Screen(SupernovaEngine *vm, ResourceManager *resMan)
	: _vm(vm)
	, _resMan(resMan)
	, _currentImage(nullptr)
	, _viewportBrightness(255)
	, _guiBrightness(255)
	, _screenWidth(320)
	, _screenHeight(200)
	, _textColor(kColorBlack)
	, _textCursorX(0)
	, _textCursorY(0)
	, _messageShown(false) {

	changeCursor(ResourceManager::kCursorNormal);
}

int Screen::getScreenWidth() const {
	return _screenWidth;
}

int Screen::getScreenHeight() const {
	return _screenHeight;
}

int Screen::getGuiBrightness() const {
	return _guiBrightness;
}

void Screen::setViewportBrightness(int brightness) {
	_viewportBrightness = brightness;
}

int Screen::getViewportBrightness() const {
	return _viewportBrightness;
}

void Screen::setGuiBrightness(int brightness) {
	_guiBrightness = brightness;
}

MSNImage *Screen::getCurrentImage() {
	return _currentImage;
}

const Screen::ImageInfo *Screen::getImageInfo(ImageId id) const {
	return &imageInfo[(int)id];
}

bool Screen::isMessageShown() const {
	return _messageShown;
}

Common::Point Screen::getTextCursorPos() {
	return Common::Point(_textCursorX, _textCursorY);
}

void Screen::setTextCursorPos(int x, int y) {
	_textCursorX = x;
	_textCursorY = y;
}

byte Screen::getTextCursorColor() {
	return _textColor;
}

void Screen::setTextCursorColor(byte color) {
	_textColor = color;
}

void Screen::renderMessage(int stringId, MessagePosition position,
						   Common::String var1, Common::String var2) {
	Common::String text = _vm->getGameString(stringId);

	if (!var1.empty()) {
		if (!var2.empty())
			text = Common::String::format(text.c_str(), var1.c_str(), var2.c_str());
		else
			text = Common::String::format(text.c_str(), var1.c_str());
	}


	renderMessage(text, position);
}

void Screen::renderMessage(const Common::String &text, MessagePosition position) {
	if (!text.empty())
		renderMessage(text.c_str(), position);
}

void Screen::renderText(const uint16 character) {
	char text[2];
	text[0] = character & 0xFF;
	text[1] = 0;
	renderText(text, _textCursorX, _textCursorY, _textColor);
}

void Screen::renderText(const char *text) {
	renderText(text, _textCursorX, _textCursorY, _textColor);
}

void Screen::renderText(int stringId) {
	renderText(_vm->getGameString(stringId));
}

void Screen::renderText(const Common::String &text) {
	if (!text.empty())
		renderText(text.c_str());
}

void Screen::renderText(const GuiElement &guiElement) {
	renderText(guiElement.getText(), guiElement.getTextPos().x,
			   guiElement.getTextPos().y, guiElement.getTextColor());
}

void Screen::renderText(const uint16 character, int x, int y, byte color) {
	char text[2];
	text[0] = character & 0xFF;
	text[1] = 0;
	renderText(text, x, y, color);
}

void Screen::renderText(const char *text, int x, int y, byte color) {
	Graphics::Surface *screen = _vm->_system->lockScreen();
	byte *cursor = static_cast<byte *>(screen->getBasePtr(x, y));
	const byte *basePtr = cursor;

	byte c;
	while ((c = *text++) != '\0') {
		if (c < 32) {
			continue;
		} else if (c == 225) {
			c = 128;
		}

		for (uint i = 0; i < 5; ++i) {
			if (font[c - 32][i] == 0xff) {
				break;
			}

			byte *ascentLine = cursor;
			for (byte j = font[c - 32][i]; j != 0; j >>= 1) {
				if (j & 1) {
					*cursor = color;
				}
				cursor += kScreenWidth;
			}
			cursor = ++ascentLine;
		}
		++cursor;
	}
	_vm->_system->unlockScreen();

	uint numChars = cursor - basePtr;
	uint absPosition = y * kScreenWidth + x + numChars;
	_textCursorX = absPosition % kScreenWidth;
	_textCursorY = absPosition / kScreenWidth;
	_textColor = color;
}

void Screen::renderText(const Common::String &text, int x, int y, byte color) {
	if (!text.empty())
		renderText(text.c_str(), x, y, color);
}

void Screen::renderText(int stringId, int x, int y, byte color) {
	renderText(_vm->getGameString(stringId), x, y, color);
}

void Screen::renderImageSection(const MSNImage *image, int section, bool invert) {
	// Note: inverting means we are removing the section. So we should get the rect for that
	// section but draw the background (section 0) instead.
	if (section > image->_numSections - 1)
		return;

	Common::Rect sectionRect(image->_section[section].x1,
							 image->_section[section].y1,
							 image->_section[section].x2 + 1,
							 image->_section[section].y2 + 1);
	bool bigImage = false;
	if (_vm->_MSPart == 1)
		bigImage = image->_filenumber == 1 || image->_filenumber == 2;
	else if (_vm->_MSPart == 2)
		bigImage = image->_filenumber == 38;
	if (bigImage) {
		sectionRect.setWidth(640);
		sectionRect.setHeight(480);
		if (_screenWidth != 640) {
			_screenWidth = 640;
			_screenHeight = 480;
			initGraphics(_screenWidth, _screenHeight);
		}
	} else {
		if (_screenWidth != 320) {
			_screenWidth = 320;
			_screenHeight = 200;
			initGraphics(_screenWidth, _screenHeight);
		}
	}

	uint offset = 0;
	int pitch = sectionRect.width();
	if (invert) {
		pitch = image->_pitch;
		offset = image->_section[section].y1 * pitch +
				 image->_section[section].x1;
		section = 0;
	}

	void *pixels = image->_sectionSurfaces[section]->getPixels();
	_vm->_system->copyRectToScreen(static_cast<const byte *>(pixels) + offset,
								   pitch, sectionRect.left, sectionRect.top,
								   sectionRect.width(), sectionRect.height());
}

void Screen::renderImage(ImageId id, bool removeImage) {
	ImageInfo info = imageInfo[id];
	const MSNImage *image = _resMan->getImage(info.filenumber);

	if (_currentImage != image)
		setCurrentImage(info.filenumber);

	do {
		renderImageSection(image, info.section, removeImage);
		info.section = image->_section[info.section].next;
	} while (info.section != 0);
}

void Screen::renderImage(int section) {
	bool removeImage = false;
	if (section > kSectionInvert) {
		removeImage = true;
		section -= kSectionInvert;
	}

	if (!_currentImage || section >= kMaxSection)
		return;

	do {
		renderImageSection(_currentImage, section, removeImage);
		section = _currentImage->_section[section].next;
	} while (section != 0);
}

bool Screen::setCurrentImage(int filenumber) {
	_currentImage = _resMan->getImage(filenumber);
	_vm->_system->getPaletteManager()->setPalette(_currentImage->getPalette(), 16, 239);
	paletteBrightness();

	return true;
}

void Screen::saveScreen(int x, int y, int width, int height) {
	_screenBuffer.push(x, y, width, height);
}

void Screen::saveScreen(const GuiElement &guiElement) {
	saveScreen(guiElement.left, guiElement.top, guiElement.width(), guiElement.height());
}

void Screen::restoreScreen() {
	_screenBuffer.restore();
}

void Screen::renderRoom(Room &room) {
	if (room.getId() == INTRO1 || room.getId() == INTRO2)
		return;

	if (setCurrentImage(room.getFileNumber())) {
		for (int i = 0; i < _currentImage->_numSections; ++i) {
			int section = i;
			if (room.isSectionVisible(section)) {
				do {
					renderImageSection(_currentImage, section, false);
					section = _currentImage->_section[section].next;
				} while (section != 0);
			}
		}
	}
}

int Screen::textWidth(const uint16 key) {
	char text[2];
	text[0] = key & 0xFF;
	text[1] = 0;
	return textWidth(text);
}

int Screen::textWidth(const char *text) {
	int charWidth = 0;
	while (*text != '\0' && *text != '\1') {
		byte c = *text++;
		if (c < 32 || c == 155) {
			// 155 is used for looping in Marquee text and is not used otherwise
			// (it is beyond the end of the font).
			continue;
		} else if (c == 225) {
			c = 35;
		}

		for (uint i = 0; i < 5; ++i) {
			if (font[c - 32][i] == 0xff) {
				break;
			}
			++charWidth;
		}
		++charWidth;
	}

	return charWidth;
}

int Screen::textWidth(const Common::String &text) {
	return Screen::textWidth(text.c_str());
}

void Screen::renderMessage(const char *text, MessagePosition position, int positionX, int positionY) {
	Common::String t(text);
	char *row[20];
	Common::String::iterator p = t.begin();
	uint numRows = 0;
	int rowWidthMax = 0;
	int x = 0;
	int y = 0;
	byte textColor = 0;

	while (*p != '\0') {
		row[numRows] = p;
		++numRows;
		while ((*p != '\0') && (*p != '|')) {
			++p;
		}
		if (*p == '|') {
			*p = '\0';
			++p;
		}
	}
	for (uint i = 0; i < numRows; ++i) {
		int rowWidth = textWidth(row[i]);
		if (rowWidth > rowWidthMax)
			rowWidthMax = rowWidth;
	}

	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr && ConfMan.getBool("tts_enabled")) {
		Common::String ttsText;
		for (uint i = 0; i < numRows; ++i) {
			if (!ttsText.empty())
				ttsText += ' ';
			ttsText += row[i];
		}
		ttsMan->say(ttsText,  Common::TextToSpeechManager::QUEUE_NO_REPEAT, Common::kDos850);
	}

	switch (position) {
	case kMessageNormal:
	default:
		x = 160 - rowWidthMax / 2;
		textColor = kColorWhite99;
		break;
	case kMessageTop:
		x = 160 - rowWidthMax / 2;
		textColor = kColorLightYellow;
		break;
	case kMessageCenter:
		x = 160 - rowWidthMax / 2;
		textColor = kColorLightRed;
		break;
	case kMessageLeft:
		x = 3;
		textColor = kColorLightYellow;
		break;
	case kMessageRight:
		x = 317 - rowWidthMax;
		textColor = kColorLightGreen;
		break;
	}

	if (position == kMessageNormal) {
		y = 70 - ((numRows * 9) / 2);
	} else if (position == kMessageTop) {
		y = 5;
	} else {
		y = 142;
	}

	if (positionX != -1 && positionY != -1) {
		x = positionX;
		y = positionY;
	}

	int message_columns = x - 3;
	int message_rows = y - 3;
	int message_width = rowWidthMax + 6;
	int message_height = numRows * 9 + 5;
	saveScreen(message_columns, message_rows, message_width, message_height);
	renderBox(message_columns, message_rows, message_width, message_height, kColorWhite35);
	for (uint i = 0; i < numRows; ++i) {
		renderText(row[i], x, y, textColor);
		y += 9;
	}

	_messageShown = true;
}

void Screen::removeMessage() {
	if (_messageShown) {
		restoreScreen();
		_messageShown = false;
	}
}

void Screen::renderBox(int x, int y, int width, int height, byte color) {
	_vm->_system->fillScreen(Common::Rect(x, y, x + width, y + height), color);
}

void Screen::renderBox(const GuiElement &guiElement) {
	renderBox(guiElement.left, guiElement.top, guiElement.width(),
			  guiElement.height(), guiElement.getBackgroundColor());
}

void Screen::initPalette() {
	g_system->getPaletteManager()->setPalette(initVGAPalette, 0, 256);
}

void Screen::paletteBrightness() {
	byte palette[768];

	_vm->_system->getPaletteManager()->grabPalette(palette, 0, 255);
	for (uint i = 0; i < 48; ++i) {
		palette[i] = (initVGAPalette[i] * _guiBrightness) >> 8;
	}
	for (uint i = 0; i < 717; ++i) {
		const byte *imagePalette;
		if (_currentImage && _currentImage->getPalette()) {
			imagePalette = _currentImage->getPalette();
		} else {
			imagePalette = palette + 48;
		}
		palette[i + 48] = (imagePalette[i] * _viewportBrightness) >> 8;
	}
	_vm->_system->getPaletteManager()->setPalette(palette, 0, 255);
}

void Screen::paletteFadeOut(int minBrightness) {
	while (_guiBrightness > minBrightness + 10) {
		_guiBrightness -= 10;
		if (_viewportBrightness > _guiBrightness)
			_viewportBrightness = _guiBrightness;
		paletteBrightness();
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(_vm->_delay);
	}
	_guiBrightness = minBrightness;
	_viewportBrightness = minBrightness;
	paletteBrightness();
	_vm->_system->updateScreen();
}

void Screen::paletteFadeIn(int maxViewportBrightness) {
	while (_guiBrightness < 245) {
		if (_viewportBrightness < maxViewportBrightness)
			_viewportBrightness += 10;
		_guiBrightness += 10;
		paletteBrightness();
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(_vm->_delay);
	}
	_guiBrightness = 255;
	_viewportBrightness = maxViewportBrightness;
	paletteBrightness();
	_vm->_system->updateScreen();
}

void Screen::setColor63(byte value) {
	byte color[3] = {value, value, value};
	_vm->_system->getPaletteManager()->setPalette(color, 63, 1);
}

void Screen::changeCursor(ResourceManager::CursorId id) {
	CursorMan.replaceCursor(_resMan->getCursor(id),
							16, 16, 0, 0, kColorCursorTransparent);
	CursorMan.replaceCursorPalette(initVGAPalette, 0, 16);
	CursorMan.showMouse(true);
}


}
