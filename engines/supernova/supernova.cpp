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
#include "supernova/screen.h"
#include "supernova/sound.h"
#include "supernova/supernova.h"
#include "supernova/state.h"

namespace Supernova {

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
	, _sound(nullptr)
	, _resMan(nullptr)
	, _screen(nullptr)
	, _allowLoadGame(true)
	, _allowSaveGame(true)
	, _sleepAutoSave(nullptr)
	, _sleepAuoSaveVersion(-1)
	, _delay(33)
	, _textSpeed(kTextSpeed[2]) {
	if (ConfMan.hasKey("textspeed"))
		_textSpeed = ConfMan.getInt("textspeed");

	DebugMan.addDebugChannel(kDebugGeneral, "general", "Supernova general debug channel");
}

SupernovaEngine::~SupernovaEngine() {
	DebugMan.clearAllDebugChannels();

	delete _sleepAutoSave;
	delete _console;
	delete _gm;
	delete _sound;
	delete _resMan;
	delete _screen;
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
	initGraphics(320, 200);

	Common::Error status = loadGameStrings();
	if (status.getCode() != Common::kNoError)
		error("Failed reading game strings");

	_resMan = new ResourceManager();
	_sound = new Sound(_mixer, _resMan);
	_gm = new GameManager(this, _sound);
	_screen = new Screen(this, _gm, _resMan);
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
		GUIErrorMessageFormat(_("Unable to locate the '%s' engine data file."), SUPERNOVA_DAT);
		return Common::kReadingFailed;
	}

	// Validate the data file header
	char id[5], lang[5];
	id[4] = lang[4] = '\0';
	f.read(id, 3);
	if (strncmp(id, "MSN", 3) != 0) {
		GUIErrorMessageFormat(_("The '%s' engine data file is corrupt."), SUPERNOVA_DAT);
		return Common::kReadingFailed;
	}

	int version = f.readByte();
	if (version != SUPERNOVA_DAT_VERSION) {
		GUIErrorMessageFormat(
			_("Incorrect version of the '%s' engine data file found. Expected %d but got %d."),
			SUPERNOVA_DAT, SUPERNOVA_DAT_VERSION, version);
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
	GUIErrorMessageFormat(_("Unable to locate the text for %s language in '%s' engine data file."), Common::getLanguageDescription(l), SUPERNOVA_DAT);
	return Common::kReadingFailed;
}

const Common::String &SupernovaEngine::getGameString(int idx) const {
	if (idx < 0 || idx >= (int)_gameStrings.size())
		return _nullString;
	return _gameStrings[idx];
}

void SupernovaEngine::setGameString(int idx, const Common::String &string) {
	if (idx < 0)
		return;
	while ((int)_gameStrings.size() <= idx)
		_gameStrings.push_back(Common::String());
	_gameStrings[idx] = string;
}

void SupernovaEngine::playSound(AudioId sample) {
	_sound->play(sample);
}

void SupernovaEngine::playSound(MusicId index) {
	_sound->play(index);
}

void SupernovaEngine::renderImage(int section) {
	_screen->renderImage(section);
}

void SupernovaEngine::renderImage(ImageId id, bool removeImage) {
	_screen->renderImage(id, removeImage);
}

bool SupernovaEngine::setCurrentImage(int filenumber) {
	return _screen->setCurrentImage(filenumber);
}

void SupernovaEngine::saveScreen(int x, int y, int width, int height) {
	_screen->saveScreen(x, y, width, height);
}
void SupernovaEngine::saveScreen(const GuiElement &guiElement) {
	_screen->saveScreen(guiElement);
}

void SupernovaEngine::restoreScreen() {
	_screen->restoreScreen();
}

void SupernovaEngine::renderRoom(Room &room) {
	_screen->renderRoom(room);
}

void SupernovaEngine::renderMessage(const char *text, MessagePosition position) {
	_screen->renderMessage(text, position);
}

void SupernovaEngine::renderMessage(const Common::String &text, MessagePosition position) {
	_screen->renderMessage(text, position);
}

void SupernovaEngine::renderMessage(StringId stringId, MessagePosition position, Common::String var1, Common::String var2) {
	_screen->renderMessage(stringId, position, var1, var2);
}

void SupernovaEngine::removeMessage() {
	_screen->removeMessage();
}

void SupernovaEngine::renderText(const uint16 character) {
	_screen->renderText(character);
}

void SupernovaEngine::renderText(const char *text) {
	_screen->renderText(text);
}

void SupernovaEngine::renderText(const Common::String &text) {
	_screen->renderText(text);
}

void SupernovaEngine::renderText(StringId stringId) {
	_screen->renderText(stringId);
}

void SupernovaEngine::renderText(const GuiElement &guiElement) {
	_screen->renderText(guiElement);
}

void SupernovaEngine::renderText(const uint16 character, int x, int y, byte color) {
	_screen->renderText(character, x, y, color);
}

void SupernovaEngine::renderText(const char *text, int x, int y, byte color) {
	_screen->renderText(text, x, y, color);
}

void SupernovaEngine::renderText(const Common::String &text, int x, int y, byte color) {
	_screen->renderText(text, x, y, color);
}

void SupernovaEngine::renderText(StringId stringId, int x, int y, byte color) {
	_screen->renderText(stringId, x, y, color);
}

void SupernovaEngine::renderBox(int x, int y, int width, int height, byte color) {
	_screen->renderBox(x, y, width, height, color);
}

void SupernovaEngine::renderBox(const GuiElement &guiElement) {
	_screen->renderBox(guiElement);
}
void SupernovaEngine::paletteBrightness() {
	_screen->paletteBrightness();
}

void SupernovaEngine::paletteFadeOut() {
	_screen->paletteFadeOut();
}

void SupernovaEngine::paletteFadeIn() {
	_screen->paletteFadeIn();
}

void SupernovaEngine::setColor63(byte value) {
	_screen->setColor63(value);
}

void SupernovaEngine::setTextSpeed() {
	const Common::String &textSpeedString = getGameString(kStringTextSpeed);
	int stringWidth = Screen::textWidth(textSpeedString);
	int textX = (kScreenWidth - stringWidth) / 2;
	int textY = 100;
	stringWidth += 4;
	int boxX = stringWidth > 110 ? (kScreenWidth - stringWidth) / 2 : 105;
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

bool SupernovaEngine::serialize(Common::WriteStream *out) {
	if (!_gm->serialize(out))
		return false;
	out->writeByte(_screen->getGuiBrightness());
	out->writeByte(_screen->getViewportBrightness());
	return true;
}

bool SupernovaEngine::deserialize(Common::ReadStream *in, int version) {
	if (!_gm->deserialize(in, version))
		return false;
	if (version >= 5) {
		_screen->setGuiBrightness(in->readByte());
		_screen->setViewportBrightness(in->readByte());
	} else {
		_screen->setGuiBrightness(255);
		_screen->setViewportBrightness(255);
	}
	return true;
}

bool SupernovaEngine::loadGame(int slot) {
	if (slot < 0)
		return false;

	// Make sure no message is displayed as this would otherwise delay the
	// switch to the new location until a mouse click.
	removeMessage();

	if (slot == kSleepAutosaveSlot) {
		if (_sleepAutoSave != nullptr && deserialize(_sleepAutoSave, _sleepAuoSaveVersion)) {
			// We no longer need the sleep autosave
			delete _sleepAutoSave;
			_sleepAutoSave = nullptr;
			return true;
		}
		// Old version used to save it literally in the kSleepAutosaveSlot, so
		// continue to try to load it from there.
	}

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

	int descriptionSize = savefile->readSint16LE();
	savefile->skip(descriptionSize);
	savefile->skip(6);
	setTotalPlayTime(savefile->readUint32LE() * 1000);
	Graphics::skipThumbnail(*savefile);
	if (!deserialize(savefile, saveVersion)) {
		delete savefile;
		return false;
	};

	// With version 9 onward the sleep auto-save is save at the end of a normal save.
	delete _sleepAutoSave;
	_sleepAutoSave = nullptr;
	if (saveVersion >= 9) {
		_sleepAuoSaveVersion = saveVersion;
		byte hasAutoSave = savefile->readByte();
		if (hasAutoSave) {
			_sleepAutoSave = new Common::MemoryReadWriteStream(DisposeAfterUse::YES);
			uint nb;
			char buf[4096];
			while ((nb = savefile->read(buf, 4096)) > 0)
				_sleepAutoSave->write(buf, nb);
		}
	}

	delete savefile;

	return true;
}

bool SupernovaEngine::saveGame(int slot, const Common::String &description) {
	if (slot < 0)
		return false;

	if (slot == kSleepAutosaveSlot) {
		delete _sleepAutoSave;
		_sleepAutoSave = new Common::MemoryReadWriteStream(DisposeAfterUse::YES);
		_sleepAuoSaveVersion = SAVEGAME_VERSION;
		serialize(_sleepAutoSave);
		return true;
	}

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
	serialize(savefile);

	if (_sleepAutoSave == nullptr)
		savefile->writeByte(0);
	else {
		savefile->writeByte(1);
		savefile->write(_sleepAutoSave->getData(), _sleepAutoSave->size());
	}

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


}
