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

#include "audio/mods/protracker.h"
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

#include "supernova/supernova.h"
#include "supernova/state.h"

namespace Supernova {

const AudioInfo audioInfo[kAudioNumSamples] = {
	{44,     0,    -1},
	{45,     0,    -1},
	{46,     0,  2510},
	{46,  2510,  4020},
	{46,  4020,    -1},
	{47,     0, 24010},
	{47, 24010,    -1},
	{48,     0,  2510},
	{48,  2510, 10520},
	{48, 10520, 13530},
	{48, 13530,    -1},
	{50,     0, 12786},
	{50, 12786,    -1},
	{51,     0,    -1},
	{53,     0,    -1},
	{54,     0,  8010},
	{54,  8010, 24020},
	{54, 24020, 30030},
	{54, 30030, 31040},
	{54, 31040,    -1}
};

const Object Object::nullObject = Object();

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
	, _console(NULL)
	, _gm(NULL)
	, _currentImage(_images)
	, _brightness(255)
	, _menuBrightness(255)
	, _imageIndex(0)
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

	// setup engine specific debug channels
	DebugMan.addDebugChannel(kDebugGeneral, "general", "Supernova general debug channel");

	_rnd = new Common::RandomSource("supernova");
}

SupernovaEngine::~SupernovaEngine() {
	DebugMan.clearAllDebugChannels();

	delete _rnd;
	delete _console;
	delete _gm;
	for (int i = 0; i < kAudioNumSamples; ++i) {
		delete[] _soundSamples[i]._buffer;
	}
	delete _soundMusic[0];
	delete _soundMusic[1];
}

Common::Error SupernovaEngine::run() {
	initGraphics(_screenWidth, _screenHeight);

	Common::Error status = loadGameStrings();
	if (status.getCode() != Common::kNoError)
		return status;

	_gm = new GameManager(this);
	_console = new Console(this, _gm);

	initData();
	initPalette();

	CursorMan.replaceCursor(_mouseNormal, 16, 16, 0, 0, kColorCursorTransparent);
	CursorMan.replaceCursorPalette(initVGAPalette, 0, 16);
	CursorMan.showMouse(true);

	setTotalPlayTime(0);

	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) {
		if (loadGameState(saveSlot).getCode() != Common::kNoError)
			error("Failed to load save game from slot %i", saveSlot);
	}

	while (!shouldQuit()) {
		uint32 start = _system->getMillis();
		updateEvents();
		_gm->executeRoom();
		_console->onFrame();
		_system->updateScreen();
		int end = _delay - (_system->getMillis() - start);
		if (end > 0)
			_system->delayMillis(end);
	}

	stopSound();

	return Common::kNoError;
}

void SupernovaEngine::updateEvents() {
	_gm->handleTime();
	if (_gm->_animationEnabled && _gm->_animationTimer == 0)
		_gm->_currentRoom->animation();

	_gm->_mouseClicked = false;
	_gm->_keyPressed = false;
	while (g_system->getEventManager()->pollEvent(_event)) {
		switch (_event.type) {
		case Common::EVENT_KEYDOWN:
			_gm->_keyPressed = true;
			if (_event.kbd.keycode == Common::KEYCODE_d &&
			    (_event.kbd.flags & Common::KBD_CTRL)) {
				_console->attach();
			}
			if (_event.kbd.keycode == Common::KEYCODE_x &&
			    (_event.kbd.flags & Common::KBD_CTRL)) {
				// TODO: Draw exit box
			}

			_gm->processInput(_event.kbd);
			break;

		case Common::EVENT_LBUTTONUP:
			// fallthrough
		case Common::EVENT_RBUTTONUP:
			if (_gm->_currentRoom->getId() != INTRO && _mixer->isSoundHandleActive(_soundHandle))
				return;
			_gm->_mouseClicked = true;
			// fallthrough
		case Common::EVENT_MOUSEMOVE:
			_gm->_mouseClickType = _event.type;
			_gm->_mouseX = _event.mouse.x;
			_gm->_mouseY = _event.mouse.y;
			if (_gm->_guiEnabled)
				_gm->processInput();
			break;

		default:
			break;
		}
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
	pauseTimer(pause);
}

void SupernovaEngine::pauseTimer(bool pause) {
	if (pause) {
		_timePaused = _gm->_state._time;
	} else {
		_gm->_state._time = _timePaused;
		_gm->_oldTime = _system->getMillis();
	}
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

void SupernovaEngine::initData() {
	// Images
	for (int i = 0; i < 44; ++i)
		_images[i].init(i);

	// Sound
	// Note:
	//   - samples start with a header of 6 bytes: 01 SS SS 00 AD 00
	//     where SS SS (LE uint16) is the size of the sound sample + 2
	//   - samples end with a footer of 4 bytes: 00 00
	// Skip those in the buffer
	Common::File file;

	for (int i = 0; i < kAudioNumSamples; ++i) {
		if (!file.open(Common::String::format("msn_data.%03d", audioInfo[i]._filenumber))) {
			error("File %s could not be read!", file.getName());
		}

		if (audioInfo[i]._offsetEnd == -1) {
			file.seek(0, SEEK_END);
			_soundSamples[i]._length = file.pos() - audioInfo[i]._offsetStart - 10;
		} else {
			_soundSamples[i]._length = audioInfo[i]._offsetEnd - audioInfo[i]._offsetStart - 10;
		}
		_soundSamples[i]._buffer = new byte[_soundSamples[i]._length];
		file.seek(audioInfo[i]._offsetStart + 6);
		file.read(_soundSamples[i]._buffer, _soundSamples[i]._length);
		file.close();
	}

	_soundMusic[0] = convertToMod("msn_data.049");
	_soundMusic[1] = convertToMod("msn_data.052");

	// Cursor
	const uint16 *bufferNormal = reinterpret_cast<const uint16 *>(mouseNormal);
	const uint16 *bufferWait = reinterpret_cast<const uint16 *>(mouseWait);
	for (uint i = 0; i < sizeof(mouseNormal) / 4; ++i) {
		for (uint bit = 0; bit < 16; ++bit) {
			uint mask = 0x8000 >> bit;
			uint bitIndex = i * 16 + bit;

			_mouseNormal[bitIndex] = (READ_LE_UINT16(bufferNormal + i) & mask) ? kColorCursorTransparent : kColorBlack;
			if (READ_LE_UINT16(bufferNormal + i + 16) & mask)
				_mouseNormal[bitIndex] = kColorLightRed;
			_mouseWait[bitIndex] = (READ_LE_UINT16(bufferWait + i) & mask) ? kColorCursorTransparent : kColorBlack;
			if (READ_LE_UINT16(bufferWait + i + 16) & mask)
				_mouseWait[bitIndex] = kColorLightRed;
		}
	}
}

void SupernovaEngine::initPalette() {
	_system->getPaletteManager()->setPalette(initVGAPalette, 0, 256);
}

void SupernovaEngine::playSound(AudioIndex sample) {
	if (sample > kAudioNumSamples - 1)
		return;

	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(
	            _soundSamples[sample]._buffer, _soundSamples[sample]._length,
	            11931, Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);
	stopSound();
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, audioStream);
}

void SupernovaEngine::stopSound() {
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
}

void SupernovaEngine::playSoundMod(int filenumber)
{
	if (filenumber != 49 && filenumber != 52) {
		return;
	}

	int index = filenumber == 49 ? 0 : 1;
	Audio::AudioStream *audioStream = Audio::makeProtrackerStream(_soundMusic[index]);
	stopSound();
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, audioStream,
	                   -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

void SupernovaEngine::renderImage(MSNImageDecoder &image, int section) {
	// Note: inverting means we are removing the section. So we should get the rect for that
	// section but draw the background (section 0) instead.
	bool invert = false;
	if (section > 128) {
		section -= 128;
		invert = true;
	}
	if (section > image._numSections - 1)
		return;

	_currentImage = &image;
	_imageIndex = image._filenumber;
	_system->getPaletteManager()->setPalette(image.getPalette(), 16, 239);
	paletteBrightness();

	Common::Rect sectionRect(image._section[section].x1,
	                         image._section[section].y1,
	                         image._section[section].x2 + 1,
	                         image._section[section].y2 + 1) ;
	if (image._filenumber == 1 || image._filenumber == 2) {
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
	if (invert) {
		offset = image._section[section].y1 * image._pitch + image._section[section].x1;
		section = 0;
	}

	_system->copyRectToScreen(static_cast<const byte *>(image._sectionSurfaces[section]->getPixels()) + offset,
	                          sectionRect.width(),
	                          sectionRect.left, sectionRect.top,
	                          sectionRect.width(), sectionRect.height());
}

void SupernovaEngine::renderImage(int filenumber, int section) {
	if (filenumber > ARRAYSIZE(_images) - 1)
		return;

	renderImage(_images[filenumber], section);
}

void SupernovaEngine::saveScreen(int x, int y, int width, int height) {
	_screenBuffer.push(x, y, width, height);
}

void SupernovaEngine::restoreScreen() {
	_screenBuffer.restore();
}

void SupernovaEngine::renderRoom(Room &room) {
	if (room.getFileNumber() != -1) {
		for (int i = 0; i < _currentImage->_numSections; ++i) {
			int section = i;
			if (room.isSectionVisible(section)) {
				do {
					renderImage(room.getFileNumber(), section);
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
//	_timer1 = (Common::strnlen(text, BUFSIZ) + 20) * textspeed / 10;
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

void SupernovaEngine::renderBox(int x, int y, int width, int height, byte color) {
	Graphics::Surface *screen = _system->lockScreen();
	screen->fillRect(Common::Rect(x, y, x + width, y + height), color);
	_system->unlockScreen();
}

void SupernovaEngine::paletteBrightness() {
	byte palette[768];

	_system->getPaletteManager()->grabPalette(palette, 0, 255);
	for (uint i = 0; i < 48; ++i) {
		palette[i] = (initVGAPalette[i] * _menuBrightness) >> 8;
	}
	for (uint i = 0; i < 717; ++i) {
		const byte *imagePalette;
		if (_currentImage->getPalette()) {
			imagePalette = _currentImage->getPalette();
		} else {
			imagePalette = palette;
		}
		palette[i + 48] = (imagePalette[i] * _brightness) >> 8;
	}
	_system->getPaletteManager()->setPalette(palette, 0, 255);
}

void SupernovaEngine::paletteFadeOut() {
	while (_brightness > 10) {
		_menuBrightness = _brightness;
		paletteBrightness();
		_brightness -= 10;
		_system->updateScreen();
		_system->delayMillis(_delay);
	}
	_menuBrightness = 0;
	_brightness = 0;
	paletteBrightness();
	_system->updateScreen();
}

void SupernovaEngine::paletteFadeIn() {
	while (_brightness < 245) {
		_menuBrightness = _brightness;
		paletteBrightness();
		_brightness += 10;
		_system->updateScreen();
		_system->delayMillis(_delay);
	}
	_menuBrightness = 255;
	_brightness = 255;
	paletteBrightness();
	_system->updateScreen();
}

void SupernovaEngine::setColor63(byte value) {
	byte color[3] = {value, value, value};
	_system->getPaletteManager()->setPalette(color, 63, 1);
}

Common::MemoryReadStream *SupernovaEngine::convertToMod(const char *filename, int version) {
	// MSN format
	struct {
		uint16 seg;
		uint16 start;
		uint16 end;
		uint16 loopStart;
		uint16 loopEnd;
		char volume;
		char dummy[5];
	} instr2[22];
	int nbInstr2; // 22 for version1, 15 for version 2
	int16 songLength;
	char arrangement[128];
	int16 patternNumber;
	int32 note2[28][64][4];

	nbInstr2 = ((version == 1) ? 22 : 15);

	Common::File msnFile;
	msnFile.open(filename);
	if (!msnFile.isOpen()) {
		warning("Data file '%s' not found", msnFile.getName());
		return NULL;
	}

	for (int i = 0 ; i < nbInstr2 ; ++i) {
		instr2[i].seg = msnFile.readUint16LE();
		instr2[i].start = msnFile.readUint16LE();
		instr2[i].end = msnFile.readUint16LE();
		instr2[i].loopStart = msnFile.readUint16LE();
		instr2[i].loopEnd = msnFile.readUint16LE();
		instr2[i].volume = msnFile.readByte();
		msnFile.read(instr2[i].dummy, 5);
	}
	songLength = msnFile.readSint16LE();
	msnFile.read(arrangement, 128);
	patternNumber = msnFile.readSint16LE();
	for (int p = 0 ; p < patternNumber ; ++p) {
		for (int n = 0 ; n < 64 ; ++n) {
			for (int k = 0 ; k < 4 ; ++k) {
				note2[p][n][k] = msnFile.readSint32LE();
			}
		}
	}

	/* MOD format */
	struct {
		char iname[22];
		uint16 length;
		char finetune;
		char volume;
		uint16 loopStart;
		uint16 loopLength;
	} instr[31];
	int32 note[28][64][4];

	// We can't recover some MOD effects since several of them are mapped to 0.
	// Assume the MSN effect of value 0 is Arpeggio (MOD effect of value 0).
	const char invConvEff[8] = {0, 1, 2, 3, 10, 12, 13 ,15};

	// Reminder from convertToMsn
	// 31 30 29 28 27 26 25 24 - 23 22 21 20 19 18 17 16 - 15 14 13 12 11 10 09 08 - 07 06 05 04 03 02 01 00
	//  h  h  h  h  g  g  g  g    f  f  f  f  e  e  e  e    d  d  d  d  c  c  c  c    b  b  b  b  a  a  a  a
	//
	// MSN:
	//  hhhh             (4 bits) Cleared to 0
	//  dddd c           (5 bits) Sample index   | after mapping through convInstr
	//        ccc        (3 bits) Effect type    | after mapping through convEff
	//  bbbb aaaa        (8 bits) Effect value   | unmodified
	//  gggg ffff eeee  (12 bits) Sample period  | unmodified
	//
	// MS2:
	//  hhhh             (4 bits) Cleared to 0
	//  dddd             (4 bits) Sample index   | after mapping through convInstr
	//  cccc             (4 bits) Effect type    | unmodified
	//  bbbb aaaa        (8 bits) Effect value   | unmodified
	//  gggg ffff eeee  (12 bits) Sample period  | transformed (0xE000 / p) - 256
	//
	// MOD:
	//  hhhh dddd        (8 bits) Sample index
	//  cccc             (4 bits) Effect type for this channel/division
	//  bbbb aaaa        (8 bits) Effect value
	//  gggg ffff eeee  (12 bits) Sample period

	// Can we recover the instruments mapping? I don't think so as part of the original instrument index is cleared.
	// And it doesn't really matter as long as we are consistent.
	// However we need to make sure 31 (or 15 in MS2) is mapped to 0 in MOD.
	// We just add 1 to all other values, and this means a 1 <-> 1 mapping for the instruments
	for (int p = 0; p < patternNumber; ++p) {
		for (int n = 0; n < 64; ++n) {
			for (int k = 0; k < 4; ++k) {
				int32* l = &(note[p][n][k]);
				*l = note2[p][n][k];
				int32 i = 0;
				if (nbInstr2 == 22) { // version 1
					i = ((*l & 0xF800) >> 11);
					int32 e = ((*l & 0x0700) >> 8);
					int32 e1 = invConvEff[e];
					*l &= 0x0FFF00FF;
					*l |= (e1 << 8);
				} else { // version 2
					int32 h = (*l >> 16);
					i = ((*l & 0xF000) >> 12);
					*l &= 0x00000FFF;
					if (h)
						h = 0xE000 / (h + 256);
					*l |= (h << 16);
					if (i == 15)
						i = 31;
				}

				// Add back index in note
				if (i != 31) {
					++i;
					*l |= ((i & 0x0F) << 12);
					*l |= ((i & 0xF0) << 24);
				}
			}
		}
	}

	for (int i = 0; i < 31; ++i) {
		// iname is not stored in the mod file. Just set it to 'instrument#'
		// finetune is not stored either. Assume 0.
		memset(instr[i].iname, 0, 22);
		sprintf(instr[i].iname, "instrument%d", i+1);
		instr[i].length = 0;
		instr[i].finetune = 0;
		instr[i].volume = 0;
		instr[i].loopStart = 0;
		instr[i].loopLength = 0;

		if (i < nbInstr2) {
			instr[i].length = ((instr2[i].end - instr2[i].start) >> 1);
			instr[i].loopStart = ((instr2[i].loopStart - instr2[i].start) >> 1);
			instr[i].loopLength = (( instr2[i].loopEnd - instr2[i].loopStart) >> 1);
			instr[i].volume = instr2[i].volume;
		}
	}

	// The ciaaSpeed is kind of useless and not present in the MSN file.
	// Traditionally 0x78 in SoundTracker. Was used in NoiseTracker as a restart point.
	// ProTracker uses 0x7F. FastTracker uses it as a restart point, whereas ScreamTracker 3 uses 0x7F like ProTracker.
	// You can use this to roughly detect which tracker made a MOD, and detection gets more accurate for more obscure MOD types.
	char ciaaSpeed = 0x7F;

	// The mark cannot be recovered either. Since we have 4 channels and 31 instrument it can be either ID='M.K.' or ID='4CHN'.
	// Assume 'M.K.'
	const char mark[4] = { 'M', '.', 'K', '.' };

	Common::MemoryWriteStreamDynamic buffer(DisposeAfterUse::NO);

	buffer.write(msnFile.getName(), 19);
	buffer.writeByte(0);

	for (int i = 0 ; i < 31 ; ++i) {
		buffer.write(instr[i].iname, 22);
		buffer.writeUint16BE(instr[i].length);
		buffer.writeByte(instr[i].finetune);
		buffer.writeByte(instr[i].volume);
		buffer.writeUint16BE(instr[i].loopStart);
		buffer.writeUint16BE(instr[i].loopLength);
	}
	buffer.writeByte((char)songLength);
	buffer.writeByte(ciaaSpeed);
	buffer.write(arrangement, 128);
	buffer.write(mark, 4);

	for (int p = 0 ; p < patternNumber ; ++p) {
		for (int n = 0 ; n < 64 ; ++n) {
			for (int k = 0 ; k < 4 ; ++k) {
//				buffer.writeUint32BE(*((uint32*)(note[p][n]+k)));
				buffer.writeSint32BE(note[p][n][k]);
			}
		}
	}

	uint nb;
	char buf[4096];
	while ((nb = msnFile.read(buf, 4096)) > 0)
		buffer.write(buf, nb);

	return new Common::MemoryReadStream(buffer.getData(), buffer.size(), DisposeAfterUse::YES);
}

bool SupernovaEngine::canLoadGameStateCurrently() {
	return _allowLoadGame;
}

Common::Error SupernovaEngine::loadGameState(int slot) {
	return (loadGame(slot) ? Common::kNoError : Common::kReadingFailed);
}

bool SupernovaEngine::canSaveGameStateCurrently() {
	return _allowSaveGame;
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

	savefile->finalize();
	delete savefile;

	return true;
}


ScreenBufferStack::ScreenBufferStack()
	: _last(_buffer) {
}

void ScreenBufferStack::push(int x, int y, int width, int height, int pitch) {
	if (_last == ARRAYEND(_buffer))
		return;

	_last->_pixels = new byte[width * height];
	byte *pixels = _last->_pixels;
	const byte *screen = static_cast<const byte *>(g_system->lockScreen()->getBasePtr(x, y));
	for (int i = 0; i < height; ++i) {
		Common::copy(screen, screen + width, pixels);
		screen += pitch;
		pixels += width;
	}
	g_system->unlockScreen();

	_last->_x = x;
	_last->_y = y;
	_last->_width = width;
	_last->_height = height;
	_last->_pitch = pitch;

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
