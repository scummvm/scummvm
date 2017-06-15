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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/str.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"

#include "supernova/supernova.h"
#include "supernova/msn_def.h"
//#include "supernova/rooms.h"


namespace Supernova {

const char *const Object::defaultDescription = "Es ist nichts Besonderes daran.";

ObjectType operator|(ObjectType a, ObjectType b) {
	return static_cast<ObjectType>(+a | +b);
}

ObjectType operator^(ObjectType a, ObjectType b) {
	return static_cast<ObjectType>(+a ^ +b);
}

ObjectType &operator|=(ObjectType &a, ObjectType b) {
	return a = a | b;
}

ObjectType &operator^=(ObjectType &a, ObjectType b) {
	return a = a ^ b;
}

SupernovaEngine::SupernovaEngine(OSystem *syst)
	: Engine(syst)
	, _console(NULL)
    , _brightness(255)
    , _menuBrightness(255)
    , _imageIndex(10)
    , _sectionIndex(0)
    , _delay(33)
{
//	const Common::FSNode gameDataDir(ConfMan.get("path"));
//	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	// setup engine specific debug channels
	DebugMan.addDebugChannel(kDebugGeneral, "general", "Supernova general debug channel");

	_rnd = new Common::RandomSource("supernova");
}

SupernovaEngine::~SupernovaEngine() {
	DebugMan.clearAllDebugChannels();
	
	delete _rnd;
	delete _console;
}

Common::Error SupernovaEngine::run() {
	initGraphics(kScreenWidth, kScreenHeight);
	debug(_system->getScreenFormat().toString().c_str());
	_console = new Console(this);

	initData();
	initPalette();
	paletteFadeIn();

	_gameRunning = true;
	while (_gameRunning) {
		updateEvents();
		
		renderImage(_imageIndex, _sectionIndex);
		renderText(Common::String::format("%u | %u", _imageIndex, _sectionIndex).c_str(), 0, 190, 15);
		_system->updateScreen();
		_system->delayMillis(_delay);
	}
	
	//deinit timer/sound/..
	stopSound();

	return Common::kNoError;
}

void SupernovaEngine::updateEvents() {
	Common::Event event;
	
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RTL:
			_gameRunning = false;
			break;
		
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_d && event.kbd.hasFlags(Common::KBD_CTRL)) {
				paletteFadeOut();
			}
			if (event.kbd.keycode == Common::KEYCODE_d && !event.kbd.hasFlags(Common::KBD_CTRL)) {
				paletteFadeIn();
			}
			if (event.kbd.keycode == Common::KEYCODE_q) {
				playSound(48, 13530);
			}
			if (event.kbd.keycode == Common::KEYCODE_w) {
				_sectionIndex = 0;
				++_imageIndex;
				if (_imageIndex == 31) {
					renderText("Das Schicksal", 44, 132, 4);
					renderText("des Horst Hummel", 35, 142, 4);
					renderText("Teil 1:", 64, 120, 12);
				}
			}
			if (event.kbd.keycode == Common::KEYCODE_e) {
				renderImage(_imageIndex, 0);
				renderImage(_imageIndex, ++_sectionIndex);
			}
			break;
		default:
			break;
		}
	}
}

void SupernovaEngine::initData() {
}

void SupernovaEngine::initPalette() {
	_system->getPaletteManager()->setPalette(initVGAPalette, 0, 256);
}

void SupernovaEngine::playSound(int filenumber, int offset) {
	Common::File *file = new Common::File;
	if (!file->open(Common::String::format("msn_data.0%2d", filenumber))) {
		error("File %s could not be read!", file->getName());
	}
	
	file->seek(offset);
	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(file, 11931, Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN);
	stopSound();
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, audioStream);
}

void SupernovaEngine::stopSound() {
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
}

void playSoundMod(int filenumber)
{
	if (filenumber != 49 || filenumber != 52) {
		error("File not supposed to be played!");
	}
	
	Common::File *file = new Common::File;
	if (!file->open(Common::String::format("msn_data.0%2d", filenumber))) {
		error("File %s could not be read!", file->getName());
	}
	
	// play Supernova MOD file
}

void SupernovaEngine::renderImage(int filenumber, int section, bool fullscreen) {
	Common::File file;
	if (!file.open(Common::String::format("msn_data.0%2d", filenumber))) {
		error("File %s could not be read!", file.getName());
	}
	
	_image.loadStream(file);
	_image.loadSection(section);
	_system->getPaletteManager()->setPalette(_image.getPalette(), 16, 239);
	paletteBrightness();
	if (fullscreen) {
		_system->copyRectToScreen(_image.getSurface()->getPixels(), 320, 0, 0, 320, 200);
	} else {
		size_t offset = _image._section[section].y1 * 320 + _image._section[section].x1;
		_system->copyRectToScreen(static_cast<const byte *>(_image.getSurface()->getPixels()) + offset,
								  320,
								  _image._section[section].x1,
								  _image._section[section].y1,
								  _image._section[section].x2 - _image._section[section].x1,
								  _image._section[section].y2 - _image._section[section].y1);
	}
}

static int characterWidth(const char *text) {
	int charWidth = 0;
	while (*text != '\0') {
		byte c = *text++;
		if (c < 32) {
			continue;
		} else if (c == 225) {
			c = 35;
		}

		for (size_t i = 0; i < 5; ++i) {
			++charWidth;
			if (font[c - 32][i] == 0xff) {
				break;
			}
		}
	}

	return charWidth;
}

void SupernovaEngine::renderMessage(char *text, MessagePosition position) {
	char *row[20];
	char *p = text;
	size_t numRows = 0;
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
	for (size_t i = 0; i < numRows; ++i) {
		int rowWidth = characterWidth(row[i]);
		if (rowWidth > rowWidthMax)
			rowWidthMax = rowWidth;
	}
	
	switch (position) {
	case kMessageNormal:
		x = rowWidthMax / 2 - 160;
		textColor = COL_MELD;
		break;
	case kMessageTop:
		x = rowWidthMax / 2 - 160;
		textColor = 14;
		break;
	case kMessageCenter:
		x = rowWidthMax / 2 - 160;
		textColor = 15;
		break;
	case kMessageLeft:
		x = 3;
		textColor = 14;
		break;
	case kMessageRight:
		x = 317 - rowWidthMax;
		textColor = 13;
		break;
	}
	
	if (position == kMessageNormal) {
		y = 70 - ((numRows * 9) / 2);
	} else if (position == kMessageTop) {
		y = 5;
	} else {
		y = 142;
	}
	
	int message_columns = x - 3;
	int message_rows = y - 3;
	int message_width = rowWidthMax + 6;
	int message_height = numRows * 9 + 5;
	renderBox(message_columns,message_rows,message_width,message_height,HGR_MELD);
	for (size_t i = 0; i < numRows; ++i) {
		renderText(row[i], x, y, textColor);
		y += 9;
	}
	
//	timer1 = (Common::strnlen(text, BUFSIZ) + 20) * textspeed / 10;
}

void SupernovaEngine::renderText(const char *text, int x, int y, byte color) {
	Graphics::Surface *screen = _system->lockScreen();
	byte *cursor = static_cast<byte *>(screen->getBasePtr(x, y));
	byte c;
	while ((c = *text++) != '\0') {
		if (c < 32) {
			continue;
		} else if (c == 225) {
			c = 128;
		}
		
		for (size_t i = 0; i < 5; ++i) {
			if (font[c - 32][i] == 0xff) {
				++cursor;
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
	}
	_system->unlockScreen();
}

void SupernovaEngine::renderBox(int x, int y, int width, int height, byte color) {
	Graphics::Surface *screen = _system->lockScreen();
	screen->fillRect(Common::Rect(x, y, width, height), color);
	_system->unlockScreen();
}

void SupernovaEngine::paletteBrightness() {
	byte palette[768];
	
	_system->getPaletteManager()->grabPalette(palette, 0, 255);
	for (size_t i = 0; i < 48; ++i) {
		palette[i] = (initVGAPalette[i] * _menuBrightness) >> 8;
	}
	for (size_t i = 0; i < 717; ++i) {
		const byte *imagePalette;
		if (_image.getPalette()) {
			imagePalette = _image.getPalette();
		} else {
			imagePalette = palette;
		}
		palette[i + 48] = (imagePalette[i] * _brightness) >> 8;
	}
	_system->getPaletteManager()->setPalette(palette, 0, 255);
}

void SupernovaEngine::paletteFadeOut() {
	// TODO: scene 0 (newspaper article in intro, mode 0x11)
	//       needs to be handled differently
	
	while (_brightness > 20) {
		_menuBrightness = _brightness;
		paletteBrightness();
		_brightness -= 20;
		_system->updateScreen();
		_system->delayMillis(_delay);
	}
	_menuBrightness = 0;
	_brightness = 0;
	paletteBrightness();
	_system->updateScreen();
}

void SupernovaEngine::paletteFadeIn() {
	// TODO: scene 0 (newspaper article in intro, mode 0x11)
	//       needs to be handled differently
	
	while (_brightness < 235) {
		_menuBrightness = _brightness;
		paletteBrightness();
		_brightness += 20;
		_system->updateScreen();
		_system->delayMillis(_delay);
	}
	_menuBrightness = 255;
	_brightness = 255;
	paletteBrightness();
	_system->updateScreen();
}

}
