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
#include "common/text-to-speech.h"
#include "engines/util.h"
#include "engines/advancedDetector.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/thumbnail.h"
#include "gui/saveload.h"

#include "supernova/resman.h"
#include "supernova/screen.h"
#include "supernova/sound.h"
#include "supernova/supernova.h"
#include "supernova/supernova1/state.h"
#include "supernova/supernova2/state.h"
#include "supernova/game-manager.h"

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
	, _gm(nullptr)
	, _sound(nullptr)
	, _resMan(nullptr)
	, _screen(nullptr)
	, _allowLoadGame(true)
	, _allowSaveGame(true)
	, _sleepAutoSave(nullptr)
	, _sleepAuoSaveVersion(-1)
	, _delay(33)
	, _textSpeed(kTextSpeed[2])
	, _improved(false) {
	if (ConfMan.hasKey("textspeed"))
		_textSpeed = ConfMan.getInt("textspeed");

	if (ConfMan.get("gameid") == "msn1")
		_MSPart = 1;
	else if (ConfMan.get("gameid") == "msn2")
		_MSPart = 2;
	else
		_MSPart = 0;

	_improved = ConfMan.getBool("improved");

}

SupernovaEngine::~SupernovaEngine() {
	delete _sleepAutoSave;
	delete _gm;
	delete _sound;
	delete _resMan;
	delete _screen;
}

Common::Error SupernovaEngine::run() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan != nullptr) {
		ttsMan->setLanguage(ConfMan.get("language"));
		ttsMan->enable(ConfMan.getBool("tts_enabled"));
	}

	init();

	while (!shouldQuit()) {
		uint32 start = _system->getMillis();
		_gm->updateEvents();
		_gm->executeRoom();
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

	_resMan = new ResourceManager(this);
	_sound = new Sound(_mixer, _resMan);
	_screen = new Screen(this, _resMan);
	if (_MSPart == 1)
		_gm = new GameManager1(this, _sound);
	else if (_MSPart == 2)
		_gm = new GameManager2(this, _sound);
	setDebugger(new Console(this, _gm));

	setTotalPlayTime(0);

	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) {
		if (loadGameState(saveSlot).getCode() != Common::kNoError)
			error("Failed to load save game from slot %i", saveSlot);
	}
}

bool SupernovaEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsReturnToLauncher:
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
	Common::String string_id("TEXT");

	Common::SeekableReadStream *stream = getBlockFromDatFile(string_id);

	if (stream == nullptr) {
		Common::Language l = Common::parseLanguage(ConfMan.get("language"));
		GUIErrorMessageFormat(_("Unable to locate the text for %s language in engine data file."), Common::getLanguageDescription(l));
		return Common::kReadingFailed;
	}

	int size = stream->size();
	while (size > 0) {
		Common::String s;
		char ch;
		while ((ch = (char)stream->readByte()) != '\0')
			s += ch;
		_gameStrings.push_back(s);
		size -= s.size() + 1;
	}

	return Common::kNoError;
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
	if (!shouldQuit())
		_sound->play(sample);
}

void SupernovaEngine::playSound(MusicId index) {
	if (!shouldQuit())
		_sound->play(index);
}

void SupernovaEngine::renderImage(int section) {
	_gm->_currentRoom->setSectionVisible(section, true);

	_screen->renderImage(section);
}

void SupernovaEngine::renderImage(ImageId id, bool removeImage) {
	_gm->_currentRoom->setSectionVisible(_screen->getImageInfo(id)->section, !removeImage);
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
	_gm->_messageDuration = (Common::strnlen(text, 512) + 20) * _textSpeed / 10;
	_screen->renderMessage(text, position);
}

void SupernovaEngine::renderMessage(const Common::String &text, MessagePosition position) {
	_gm->_messageDuration = (text.size() + 20) * _textSpeed / 10;
	_screen->renderMessage(text, position);
}

void SupernovaEngine::renderMessage(int stringId, MessagePosition position, Common::String var1, Common::String var2) {
	_gm->_messageDuration = (getGameString(stringId).size() + 20) * _textSpeed / 10;
	_screen->renderMessage(stringId, position, var1, var2);
}

void SupernovaEngine::renderMessage(int stringId, int x, int y) {
	_gm->_messageDuration = (getGameString(stringId).size() + 20) * _textSpeed / 10;
	_screen->renderMessage(getGameString(stringId).c_str(), kMessageNormal, x, y);
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

void SupernovaEngine::renderText(int stringId) {
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

void SupernovaEngine::renderText(int stringId, int x, int y, byte color) {
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

void SupernovaEngine::paletteFadeOut(int minBrightness) {
	if (!shouldQuit())
		_screen->paletteFadeOut(minBrightness);
}

void SupernovaEngine::paletteFadeIn() {
	if (!shouldQuit()) {
		_gm->roomBrightness();
		_screen->paletteFadeIn(_gm->_roomBrightness);
	}
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

	// Disable improved mode temporarilly so that Key 1-5 are received below
	// instead of being mapped to action selection.
	bool hasImprovedMode = _improved;
	_improved = false;

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

	_improved = hasImprovedMode;
}

void SupernovaEngine::showHelpScreen1() {
	if (_screen->isMessageShown())
		_screen->removeMessage();
	_gm->animationOff();
	_gm->saveTime();

	paletteFadeOut();
	renderImage(kImageHelpScreen);
	renderBox(100, 100, 192, 78, kColorWhite35);
	renderText(kStringHelpOverview1, 105, 105, kColorWhite99);
	renderText(kStringHelpOverview2, 105, 115, kColorWhite99);
	renderText(kStringHelpOverview3, 105, 125, kColorWhite99);
	renderText(kStringHelpOverview4, 105, 135, kColorWhite99);
	renderText(kStringHelpOverview5, 105, 145, kColorWhite99);
	renderText(kStringHelpOverview6, 105, 155, kColorWhite99);
	renderText(kStringHelpOverview7, 105, 165, kColorWhite99);
	paletteFadeIn();
	_gm->getInput(true);

	paletteFadeOut();

	_gm->loadTime();
	_gm->animationOn();
}

void SupernovaEngine::showHelpScreen2() {
	if (_screen->isMessageShown())
		_screen->removeMessage();
	_gm->animationOff();
	_gm->saveTime();

	paletteFadeOut();
	setCurrentImage(27);
	renderImage(0);
	paletteFadeIn();
	_gm->getInput(true);

	paletteFadeOut();

	_gm->loadTime();
	_gm->animationOn();
}

Common::SeekableReadStream *SupernovaEngine::getBlockFromDatFile(Common::String name) {
	Common::String cur_lang = ConfMan.get("language");

	// Validate the data file header
	Common::File f;
	char id[5], lang[5];
	id[4] = lang[4] = '\0';
	if (!f.open(SUPERNOVA_DAT)) {
		GUIErrorMessageFormat(_("Unable to locate the '%s' engine data file."), SUPERNOVA_DAT);
		return nullptr;
	}
	f.read(id, 3);
	if (strncmp(id, "MSN", 3) != 0) {
		GUIErrorMessageFormat(_("The '%s' engine data file is corrupt."), SUPERNOVA_DAT);
		return nullptr;
	}

	int version = f.readByte();
	if (version != SUPERNOVA_DAT_VERSION) {
		GUIErrorMessageFormat(
			_("Incorrect version of the '%s' engine data file found. Expected %d but got %d."),
			SUPERNOVA_DAT, SUPERNOVA_DAT_VERSION, version);
		return nullptr;
	}

	uint32 gameBlockSize = 0;
	while (!f.eos()) {
		int part = f.readByte();
		gameBlockSize = f.readUint32LE();
		if (f.eos()){
			GUIErrorMessageFormat(_("Unable to find block for part %d"), _MSPart);
			return nullptr;
		}
		if (part == _MSPart) {
			break;
		} else
			f.skip(gameBlockSize);
	}

	uint32 readSize = 0;

	while (readSize < gameBlockSize) {
		f.read(id, 4);
		f.read(lang, 4);
		uint32 size = f.readUint32LE();
		if (f.eos())
			break;
		if (name == id && cur_lang == lang) {
			return f.readStream(size);
		} else {
			f.skip(size);
			// size + 4 bytes for id + 4 bytes for lang + 4 bytes for size
			readSize += size + 12;
		}
	}

	return nullptr;
}

Common::Error SupernovaEngine::showTextReader(const char *extension) {
	Common::SeekableReadStream *stream;
	Common::String blockName;
	blockName = Common::String::format("%s%d", extension, _MSPart);
	blockName.toUppercase();
	if ((stream = getBlockFromDatFile(blockName)) == nullptr) {
		Common::File file;
		Common::Path filename;
		if (_MSPart == 1)
			filename = Common::Path(Common::String::format("msn.%s", extension));
		if (_MSPart == 2)
			filename = Common::Path(Common::String::format("ms2.%s", extension));

		if (!file.open(filename)) {
			GUIErrorMessageFormat(_("Unable to find '%s' in game folder or the engine data file."), filename.toString().c_str());
			return Common::kReadingFailed;
		}
		stream = file.readStream(file.size());
	}
	int linesInFile = 0;
	while (!stream->eos()) {
		stream->readLine();
		++linesInFile;
	}
	--linesInFile;
	stream->seek(0);
	stream->clearErr();

	if (_screen->isMessageShown())
		_screen->removeMessage();
	_gm->animationOff();
	_gm->saveTime();
	paletteFadeOut();
	g_system->fillScreen(kColorWhite35);
	for (int y = 6; y < (200 - kFontHeight); y += (kFontHeight + 2)) {
		Common::String line = stream->readLine();
		if (stream->eos())
			break;
		_screen->renderText(line, 6, y, kColorWhite99);
	}
	paletteFadeIn();

	const int linesPerPage = 19;
	int lineNumber = 0;
	bool exitReader = false;
	do {
		stream->seek(0);
		stream->clearErr();
		for (int i = 0; i < lineNumber; ++i)
			stream->readLine();
		g_system->fillScreen(kColorWhite35);
		for (int y = 6; y < (_screen->getScreenHeight() - kFontHeight); y += (kFontHeight + 2)) {
			Common::String line = stream->readLine();
			if (stream->eos())
				break;
			_screen->renderText(line, 6, y, kColorWhite99);
		}
		_gm->getInput(true);
		switch (_gm->_key.keycode) {
		case Common::KEYCODE_ESCAPE:
			exitReader = true;
			break;
		case Common::KEYCODE_UP:
			lineNumber = lineNumber > 0 ? lineNumber - 1 : 0;
			break;
		case Common::KEYCODE_DOWN:
			lineNumber = lineNumber < linesInFile - (linesPerPage + 1) ? lineNumber + 1
			                                                           : linesInFile - linesPerPage;
			break;
		case Common::KEYCODE_PAGEUP:
			lineNumber = lineNumber > linesPerPage ? lineNumber - linesPerPage : 0;
			break;
		case Common::KEYCODE_PAGEDOWN:
			lineNumber = lineNumber < linesInFile - (linesPerPage * 2) ? lineNumber + linesPerPage
			                                                           : linesInFile - linesPerPage;
			break;
		default:
			break;
		}
	} while (!exitReader && !shouldQuit());

	paletteFadeOut();
	_gm->loadTime();
	_gm->animationOn();

	return Common::kNoError;
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
	_gm->animationOn();

	return quit;
}


bool SupernovaEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return _allowLoadGame;
}

Common::Error SupernovaEngine::loadGameState(int slot) {
	return (loadGame(slot) ? Common::kNoError : Common::kReadingFailed);
}

bool SupernovaEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	// Do not allow saving when either _allowSaveGame, _animationEnabled or _guiEnabled is false
	return _allowSaveGame && _gm->canSaveGameStateCurrently();
}

Common::Error SupernovaEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
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

Common::String SupernovaEngine::getSaveStateName(int slot) const {
	if (_MSPart == 1)
		return Common::String::format("msn_save.%03d", slot);
	else if (_MSPart == 2)
		return Common::String::format("ms2_save.%03d", slot);

	return "";
}

bool SupernovaEngine::loadGame(int slot) {
	if (slot < 0)
		return false;

	// Stop any sound currently playing.
	_sound->stop();

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

	Common::String filename = getSaveStateName(slot);
	Common::InSaveFile *savefile = _saveFileMan->openForLoading(filename);
	if (!savefile)
		return false;

	uint saveHeader = savefile->readUint32LE();
	if ((_MSPart == 1 && saveHeader != SAVEGAME_HEADER) ||
		(_MSPart == 2 && saveHeader != SAVEGAME_HEADER2)) {
		warning("No header found in '%s'", filename.c_str());
		delete savefile;
		return false; //Common::kUnknownError
	}

	byte saveVersion = savefile->readByte();
	// Save version 1 was used during development and is no longer supported
	if (saveVersion > SAVEGAME_VERSION || saveVersion < 10) {
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

	Common::String filename = getSaveStateName(slot);
	Common::OutSaveFile *savefile = _saveFileMan->openForSaving(filename);
	if (!savefile)
		return false;

	if (_MSPart == 1)
		savefile->writeUint32LE(SAVEGAME_HEADER);
	else if (_MSPart == 2)
		savefile->writeUint32LE(SAVEGAME_HEADER2);
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
		? _("Failed to save temporary game state. Make sure your save game directory is set in ScummVM and that you can write to it.")
		: _("Failed to load temporary game state."));
	error("Unrecoverable error");
}

void SupernovaEngine::stopSound() {
	_sound->stop();
}

}
