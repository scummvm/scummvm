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
#include "common/endian.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"
#include "graphics/thumbnail.h"
#include "gui/saveload.h"

#include "supernova/resman.h"
#include "supernova/sound.h"
#include "supernova/supernova.h"
#include "supernova/state.h"

namespace Supernova {


const Object Object::nullObject;

ObjectType operator|(ObjectType a, ObjectType b) {
	return static_cast<ObjectType>(+a | +b);
}

ObjectType operator&(ObjectType a, ObjectType b) {
	return static_cast<ObjectType>(+a & +b);
}

ObjectType operator^(ObjectType a, ObjectType b) {
	return static_cast<ObjectType>(+a ^ +b);
}

ObjectType &operator|=(ObjectType &a, ObjectType b) {
	return a = a | b;
}

ObjectType &operator&=(ObjectType &a, ObjectType b) {
	return a = a & b;
}

ObjectType &operator^=(ObjectType &a, ObjectType b) {
	return a = a ^ b;
}

SupernovaEngine::SupernovaEngine(OSystem *syst)
	: Engine(syst)
	, _console(nullptr)
	, _gm(nullptr)
	, _currentImage(nullptr)
	, _rnd("supernova")
	, _brightness(255)
	, _menuBrightness(255)
	, _delay(33)
	, _textSpeed(kTextSpeed[2])
	, _screenWidth(320)
	, _screenHeight(200)
	, _messageDisplayed(false)
	, _allowLoadGame(true)
	, _allowSaveGame(true)
{
//	const Common::FSNode gameDataDir(ConfMan.get("path"));
//	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	if (ConfMan.hasKey("textspeed"))
		_textSpeed = ConfMan.getInt("textspeed");

	// setup engine specific debug channels
	DebugMan.addDebugChannel(kDebugGeneral, "general", "Supernova general debug channel");
}

SupernovaEngine::~SupernovaEngine() {
	DebugMan.clearAllDebugChannels();

	delete _currentImage;
	delete _console;
	delete _gm;
	delete _sound;
	delete _resMan;
}

Common::Error SupernovaEngine::run() {
	init();

	while (!shouldQuit()) {
		uint32 start = _system->getMillis();
		_gm->updateEvents();
		_gm->executeRoom();
		_console->onFrame();
		_system->updateScreen();
		int end = _delay - (_system->getMillis() - start);
		if (end > 0)
			_system->delayMillis(end);
	}

	_mixer->stopAll();

	return Common::kNoError;
}

void SupernovaEngine::init() {
	Graphics::ModeList modes;
	modes.push_back(Graphics::Mode(320, 200));
	modes.push_back(Graphics::Mode(640, 480));
	initGraphicsModes(modes);
	initGraphics(_screenWidth, _screenHeight);

	Common::Error status = loadGameStrings();
	if (status.getCode() != Common::kNoError)
		error("Failed reading game strings");

	_resMan = new ResourceManager(this);
	_sound = new Sound(_mixer, _resMan);
	_gm = new GameManager(this, _sound);
	_console = new Console(this, _gm);

	setTotalPlayTime(0);

	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) {
		if (loadGameState(saveSlot).getCode() != Common::kNoError)
			error("Failed to load save game from slot %i", saveSlot);
	}
}

bool SupernovaEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsRTL:
		return true;
	case kSupportsLoadingDuringRuntime:
		return true;
	case kSupportsSavingDuringRuntime:
		return true;
	default:
		return false;
	}
}

void SupernovaEngine::pauseEngineIntern(bool pause) {
	_mixer->pauseAll(pause);
	_gm->pauseTimer(pause);
}

Common::Error SupernovaEngine::loadGameStrings() {
	Common::String cur_lang = ConfMan.get("language");
	Common::String string_id("TEXT");

	// Note: we don't print any warning or errors here if we cannot find the file
	// or the format is not as expected. We will get those warning when reading the
	// strings anyway (actually the engine will even refuse to start).
	Common::File f;
	if (!f.open(SUPERNOVA_DAT)) {
		Common::String msg = Common::String::format(_("Unable to locate the '%s' engine data file."), SUPERNOVA_DAT);
		GUIErrorMessage(msg);
		return Common::kReadingFailed;
	}

	// Validate the data file header
	char id[5], lang[5];
	id[4] = lang[4] = '\0';
	f.read(id, 3);
	if (strncmp(id, "MSN", 3) != 0) {
		Common::String msg = Common::String::format(_("The '%s' engine data file is corrupt."), SUPERNOVA_DAT);
		GUIErrorMessage(msg);
		return Common::kReadingFailed;
	}

	int version = f.readByte();
	if (version != SUPERNOVA_DAT_VERSION) {
		Common::String msg = Common::String::format(
			_("Incorrect version of the '%s' engine data file found. Expected %d but got %d."),
			SUPERNOVA_DAT, SUPERNOVA_DAT_VERSION, version);
		GUIErrorMessage(msg);
		return Common::kReadingFailed;
	}

	while (!f.eos()) {
		f.read(id, 4);
		f.read(lang, 4);
		uint32 size = f.readUint32LE();
		if (f.eos())
			break;
		if (string_id == id && cur_lang == lang) {
			while (size > 0) {
				Common::String s;
				char ch;
				while ((ch = (char)f.readByte()) != '\0')
					s += ch;
				_gameStrings.push_back(s);
				size -= s.size() + 1;
			}
			return Common::kNoError;
		} else
			f.skip(size);
	}

	Common::Language l = Common::parseLanguage(cur_lang);
	Common::String msg = Common::String::format(_("Unable to locate the text for %s language in '%s' engine data file."), Common::getLanguageDescription(l), SUPERNOVA_DAT);
	GUIErrorMessage(msg);
	return Common::kReadingFailed;
}

void SupernovaEngine::playSound(AudioIndex sample) {
	_sound->play(sample);
}

void SupernovaEngine::playSound(MusicIndex index) {
	_sound->play(index);
}

void SupernovaEngine::renderImageSection(int section) {
	// Note: inverting means we are removing the section. So we should get the rect for that
	// section but draw the background (section 0) instead.
	bool invert = false;
	if (section > 128) {
		section -= 128;
		invert = true;
	}
	if (!_currentImage || section > _currentImage->_numSections - 1)
		return;

	Common::Rect sectionRect(_currentImage->_section[section].x1,
							 _currentImage->_section[section].y1,
							 _currentImage->_section[section].x2 + 1,
							 _currentImage->_section[section].y2 + 1) ;
	if (_currentImage->_filenumber == 1 || _currentImage->_filenumber == 2) {
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
		pitch = _currentImage->_pitch;
		offset = _currentImage->_section[section].y1 * pitch + _currentImage->_section[section].x1;
		section = 0;
	}

	_system->copyRectToScreen(static_cast<const byte *>(_currentImage->_sectionSurfaces[section]->getPixels()) + offset,
							  pitch,
							  sectionRect.left, sectionRect.top,
							  sectionRect.width(), sectionRect.height());
}

void SupernovaEngine::renderImage(int section) {
	if (!_currentImage)
		return;

	bool sectionVisible = true;

	if (section > 128) {
		sectionVisible = false;
		section -= 128;
	}

	_gm->_currentRoom->setSectionVisible(section, sectionVisible);

	do {
		if (sectionVisible)
			renderImageSection(section);
		else
			renderImageSection(section + 128);
		section = _currentImage->_section[section].next;
	} while (section != 0);
}

bool SupernovaEngine::setCurrentImage(int filenumber) {
	if (_currentImage && _currentImage->_filenumber == filenumber)
		return true;

	delete _currentImage;
	_currentImage = new MSNImage();
	if (!_currentImage->init(filenumber)) {
		delete _currentImage;
		_currentImage = nullptr;
		return false;
	}

	_system->getPaletteManager()->setPalette(_currentImage->getPalette(), 16, 239);
	paletteBrightness();
	return true;
}

void SupernovaEngine::saveScreen(int x, int y, int width, int height) {
	_screenBuffer.push(x, y, width, height);
}
void SupernovaEngine::saveScreen(const GuiElement &guiElement) {
	saveScreen(guiElement.left, guiElement.top, guiElement.width(), guiElement.height());
}

void SupernovaEngine::restoreScreen() {
	_screenBuffer.restore();
}

void SupernovaEngine::renderRoom(Room &room) {
	if (room.getId() == INTRO)
		return;

	if (setCurrentImage(room.getFileNumber())) {
		for (int i = 0; i < _currentImage->_numSections; ++i) {
			int section = i;
			if (room.isSectionVisible(section)) {
				do {
					renderImageSection(section);
					section = _currentImage->_section[section].next;
				} while (section != 0);
			}
		}
	}
}

int SupernovaEngine::textWidth(const uint16 key) {
	char text[2];
	text[0] = key & 0xFF;
	text[1] = 0;
	return textWidth(text);
}

int SupernovaEngine::textWidth(const char *text) {
	int charWidth = 0;
	while (*text != '\0') {
		byte c = *text++;
		if (c < 32) {
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

void SupernovaEngine::renderMessage(const char *text, MessagePosition position) {
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

	switch (position) {
	case kMessageNormal:
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

	_messageDisplayed = true;
	_gm->_messageDuration = (Common::strnlen(text, 512) + 20) * _textSpeed / 10;
}

void SupernovaEngine::removeMessage() {
	if (_messageDisplayed) {
		restoreScreen();
		_messageDisplayed = false;
	}
}

void SupernovaEngine::renderText(const char *text, int x, int y, byte color) {
	Graphics::Surface *screen = _system->lockScreen();
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
	_system->unlockScreen();

	uint numChars = cursor - basePtr;
	uint absPosition = y * kScreenWidth + x + numChars;
	_textCursorX = absPosition % kScreenWidth;
	_textCursorY = absPosition / kScreenWidth;
	_textColor = color;
}

void SupernovaEngine::renderText(const uint16 character, int x, int y, byte color) {
	char text[2];
	text[0] = character & 0xFF;
	text[1] = 0;
	renderText(text, x, y, color);
}

void SupernovaEngine::renderText(const char *text) {
	renderText(text, _textCursorX, _textCursorY, _textColor);
}

void SupernovaEngine::renderText(const uint16 character) {
	char text[2];
	text[0] = character & 0xFF;
	text[1] = 0;
	renderText(text, _textCursorX, _textCursorY, _textColor);
}
void SupernovaEngine::renderText(const GuiElement &guiElement) {
	renderText(guiElement.getText(), guiElement.getTextPos().x,
			   guiElement.getTextPos().y, guiElement.getTextColor());
}

void SupernovaEngine::renderBox(int x, int y, int width, int height, byte color) {
	Graphics::Surface *screen = _system->lockScreen();
	screen->fillRect(Common::Rect(x, y, x + width, y + height), color);
	_system->unlockScreen();
}

void SupernovaEngine::renderBox(const GuiElement &guiElement) {
	renderBox(guiElement.left, guiElement.top, guiElement.width(),
			  guiElement.height(), guiElement.getBackgroundColor());
}
void SupernovaEngine::paletteBrightness() {
	byte palette[768];

	_system->getPaletteManager()->grabPalette(palette, 0, 255);
	for (uint i = 0; i < 48; ++i) {
		palette[i] = (initVGAPalette[i] * _menuBrightness) >> 8;
	}
	for (uint i = 0; i < 717; ++i) {
		const byte *imagePalette;
		if (_currentImage && _currentImage->getPalette()) {
			imagePalette = _currentImage->getPalette();
		} else {
			imagePalette = palette + 48;
		}
		palette[i + 48] = (imagePalette[i] * _brightness) >> 8;
	}
	_system->getPaletteManager()->setPalette(palette, 0, 255);
}

void SupernovaEngine::paletteFadeOut() {
	while (_menuBrightness > 10) {
		_menuBrightness -= 10;
		if (_brightness > _menuBrightness)
			_brightness = _menuBrightness;
		paletteBrightness();
		_system->updateScreen();
		_system->delayMillis(_delay);
	}
	_menuBrightness = 0;
	_brightness = 0;
	paletteBrightness();
	_system->updateScreen();
}

void SupernovaEngine::paletteFadeIn() {
	while (_menuBrightness < 245) {
		if (_brightness < _gm->_roomBrightness)
			_brightness += 10;
		_menuBrightness += 10;
		paletteBrightness();
		_system->updateScreen();
		_system->delayMillis(_delay);
	}
	_menuBrightness = 255;
	_brightness = _gm->_roomBrightness;
	paletteBrightness();
	_system->updateScreen();
}

void SupernovaEngine::setColor63(byte value) {
	byte color[3] = {value, value, value};
	_system->getPaletteManager()->setPalette(color, 63, 1);
}

void SupernovaEngine::setTextSpeed() {
	const Common::String& textSpeedString = getGameString(kStringTextSpeed);
	int stringWidth = textWidth(textSpeedString);
	int textX = (320 - stringWidth) / 2;
	int textY = 100;
	stringWidth += 4;
	int boxX = stringWidth > 110 ? (320 - stringWidth) / 2 : 105;
	int boxY = 97;
	int boxWidth = stringWidth > 110 ? stringWidth : 110;
	int boxHeight = 27;

	_gm->animationOff();
	_gm->saveTime();
	saveScreen(boxX, boxY, boxWidth, boxHeight);

	renderBox(boxX, boxY, boxWidth, boxHeight, kColorBlue);
	renderText(textSpeedString, textX, textY, kColorWhite99); // Text speed

	// Find the closest index in kTextSpeed for the current _textSpeed.
	// Important note: values in kTextSpeed decrease with the index.
	int speedIndex = 0;
	while (speedIndex < 4 && _textSpeed < (kTextSpeed[speedIndex] + kTextSpeed[speedIndex+1]) / 2)
		++speedIndex;

	char nbString[2];
	nbString[1] = 0;
	for (int i = 0; i < 5; ++i) {
		byte color = i == speedIndex ? kColorWhite63 : kColorWhite35;
		renderBox(110 + 21 * i, 111, 16, 10, color);

		nbString[0] = '1' + i;
		renderText(nbString, 115 + 21 * i, 112, kColorWhite99);
	}
	do {
		_gm->getInput();
		int key = _gm->_keyPressed ? _gm->_key.keycode : Common::KEYCODE_INVALID;
		if (!_gm->_keyPressed && _gm->_mouseClicked && _gm->_mouseY >= 111 && _gm->_mouseY < 121 && (_gm->_mouseX + 16) % 21 < 16)
			key = Common::KEYCODE_0 - 5 + (_gm->_mouseX + 16) / 21;
		if (key == Common::KEYCODE_ESCAPE)
			break;
		else if (key >= Common::KEYCODE_1 && key <= Common::KEYCODE_5) {
			speedIndex = key - Common::KEYCODE_1;
			_textSpeed = kTextSpeed[speedIndex];
			ConfMan.setInt("textspeed", _textSpeed);
			break;
		}
	} while (!shouldQuit());
	_gm->resetInputState();

	restoreScreen();
	_gm->loadTime();
	_gm->animationOn();
}

bool SupernovaEngine::quitGameDialog() {
	bool quit = false;

	GuiElement guiQuitBox;
	guiQuitBox.setColor(kColorRed, kColorWhite99, kColorRed, kColorWhite99);
	guiQuitBox.setSize(112, 97, 112 + 96, 97 + 27);
	guiQuitBox.setText(getGameString(kStringLeaveGame).c_str());
	guiQuitBox.setTextPosition(guiQuitBox.left + 3, guiQuitBox.top + 3);
	GuiElement guiQuitYes;
	guiQuitYes.setColor(kColorWhite35, kColorWhite99, kColorWhite35, kColorWhite99);
	guiQuitYes.setSize(115, 111, 158, 121);
	guiQuitYes.setText(getGameString(kStringYes).c_str());
	guiQuitYes.setTextPosition(132, 112);
	GuiElement guiQuitNo;
	guiQuitNo.setColor(kColorWhite35, kColorWhite99, kColorWhite35, kColorWhite99);
	guiQuitNo.setSize(162, 111, 205, 121);
	guiQuitNo.setText(getGameString(kStringNo).c_str());
	guiQuitNo.setTextPosition(173, 112);

	_gm->animationOff();
	_gm->saveTime();
	saveScreen(guiQuitBox);

	renderBox(guiQuitBox);
	renderText(guiQuitBox);
	renderBox(guiQuitYes);
	renderText(guiQuitYes);
	renderBox(guiQuitNo);
	renderText(guiQuitNo);

	do {
		_gm->getInput();
		if (_gm->_keyPressed) {
			if (_gm->_key.keycode == Common::KEYCODE_j) {
				quit = true;
				break;
			} else if (_gm->_key.keycode == Common::KEYCODE_n) {
				quit = false;
				break;
			}
		}
		if (_gm->_mouseClicked) {
			if (guiQuitYes.contains(_gm->_mouseX, _gm->_mouseY)) {
				quit = true;
				break;
			} else if (guiQuitNo.contains(_gm->_mouseX, _gm->_mouseY)) {
				quit = false;
				break;
			}
		}
	} while (true);

	_gm->resetInputState();
	restoreScreen();
	_gm->loadTime();
	_gm->animationOn();

	return quit;
}


bool SupernovaEngine::canLoadGameStateCurrently() {
	return _allowLoadGame;
}

Common::Error SupernovaEngine::loadGameState(int slot) {
	return (loadGame(slot) ? Common::kNoError : Common::kReadingFailed);
}

bool SupernovaEngine::canSaveGameStateCurrently() {
	// Do not allow saving when either _allowSaveGame, _animationEnabled or _guiEnabled is false
	return _allowSaveGame && _gm->_animationEnabled && _gm->_guiEnabled;
}

Common::Error SupernovaEngine::saveGameState(int slot, const Common::String &desc) {
	return (saveGame(slot, desc) ? Common::kNoError : Common::kWritingFailed);
}

bool SupernovaEngine::loadGame(int slot) {
	if (slot < 0)
		return false;

	Common::String filename = Common::String::format("msn_save.%03d", slot);
	Common::InSaveFile *savefile = _saveFileMan->openForLoading(filename);
	if (!savefile)
		return false;

	uint saveHeader = savefile->readUint32LE();
	if (saveHeader != SAVEGAME_HEADER) {
		warning("No header found in '%s'", filename.c_str());
		delete savefile;
		return false; //Common::kUnknownError
	}

	byte saveVersion = savefile->readByte();
	// Save version 1 was used during development and is no longer supported
	if (saveVersion > SAVEGAME_VERSION || saveVersion == 1) {
		warning("Save game version %i not supported", saveVersion);
		delete savefile;
		return false; //Common::kUnknownError;
	}

	// Make sure no message is displayed as this would otherwise delay the
	// switch to the new location until a mouse click.
	removeMessage();

	int descriptionSize = savefile->readSint16LE();
	savefile->skip(descriptionSize);
	savefile->skip(6);
	setTotalPlayTime(savefile->readUint32LE() * 1000);
	Graphics::skipThumbnail(*savefile);
	_gm->deserialize(savefile, saveVersion);

	if (saveVersion >= 5) {
		_menuBrightness = savefile->readByte();
		_brightness = savefile->readByte();
	} else {
		_menuBrightness = _brightness = 255;
	}

	delete savefile;

	return true;
}

bool SupernovaEngine::saveGame(int slot, const Common::String &description) {
	if (slot < 0)
		return false;

	Common::String filename = Common::String::format("msn_save.%03d", slot);
	Common::OutSaveFile *savefile = _saveFileMan->openForSaving(filename);
	if (!savefile)
		return false;

	savefile->writeUint32LE(SAVEGAME_HEADER);
	savefile->writeByte(SAVEGAME_VERSION);

	TimeDate currentDate;
	_system->getTimeAndDate(currentDate);
	uint32 saveDate = (currentDate.tm_mday & 0xFF) << 24 | ((currentDate.tm_mon + 1) & 0xFF) << 16 | ((currentDate.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = (currentDate.tm_hour & 0xFF) << 8 | ((currentDate.tm_min) & 0xFF);

	savefile->writeSint16LE(description.size() + 1);
	savefile->write(description.c_str(), description.size() + 1);
	savefile->writeUint32LE(saveDate);
	savefile->writeUint16LE(saveTime);
	savefile->writeUint32LE(getTotalPlayTime() / 1000);
	Graphics::saveThumbnail(*savefile);
	_gm->serialize(savefile);

	savefile->writeByte(_menuBrightness);
	savefile->writeByte(_brightness);

	savefile->finalize();
	delete savefile;

	return true;
}

void SupernovaEngine::errorTempSave(bool saving) {
	GUIErrorMessage(saving
		? "Failed to save temporary game state. Make sure your save game directory is set in ScummVM and that you can write to it."
		: "Failed to load temporary game state.");
	error("Unrecoverable error");
}

ScreenBufferStack::ScreenBufferStack()
	: _last(_buffer) {
}

void ScreenBufferStack::push(int x, int y, int width, int height) {
	if (_last == ARRAYEND(_buffer))
		return;

	Graphics::Surface* screenSurface = g_system->lockScreen();

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
	g_system->lockScreen()->copyRectToSurface(
				_last->_pixels, _last->_width, _last->_x, _last->_y,
				_last->_width, _last->_height);
	g_system->unlockScreen();

	delete[] _last->_pixels;
}

}
