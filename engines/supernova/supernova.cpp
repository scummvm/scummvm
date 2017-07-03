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
#include "audio/mods/protracker.h"
#include "common/memstream.h"
#include "common/endian.h"

#include "supernova/supernova.h"

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
    {53,     0,    -1},
    {54,     0,  8010},
    {54,  8010, 24020},
    {54, 24020, 30030},
    {54, 30030, 31040},
    {54, 31040,    -1}
};

const char *const Object::defaultDescription = "Es ist nichts Besonderes daran.";
const char *const Object::takeMessage = "Das mußt du erst nehmen.";

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
    , _currentImage(_images)
    , _brightness(255)
    , _menuBrightness(255)
    , _imageIndex(0)
    , _sectionIndex(0)
    , _delay(33)
    , _gameRunning(true)
    , _screenWidth(320)
    , _screenHeight(200)
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
	for (int i = 0; i < kAudioNumSamples; ++i) {
		delete[] _soundSamples[i]._buffer;
	}
	delete _soundMusic[0];
	delete _soundMusic[1];
}

Common::Error SupernovaEngine::run() {
	GameManager gm(this, &_event);
	initGraphics(_screenWidth, _screenHeight);
	_console = new Console(this, &gm);

	initData();
	initPalette();
	paletteFadeIn();

	CursorMan.replaceCursor(_mouseWait, 16, 16, 0, 0, kColorCursorTransparent);
	CursorMan.replaceCursorPalette(initVGAPalette, 0, 16);
	CursorMan.showMouse(true);

	while (_gameRunning) {

		while (g_system->getEventManager()->pollEvent(_event)) {
			switch (_event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				_gameRunning = false;
				break;

			case Common::EVENT_KEYDOWN:
				if (_event.kbd.keycode == Common::KEYCODE_d && _event.kbd.hasFlags(Common::KBD_CTRL)) {
					_console->attach();
				}
				break;

			case Common::EVENT_LBUTTONUP:
				break;
			case Common::EVENT_RBUTTONUP:
				// TODO: Determines verb depending on object properties
				break;
			case Common::EVENT_MOUSEMOVE:
				// TODO: Update status if mouse enters/leaves object
				break;
			default:
				break;
			}

//			gm.processInput();
		}

		_console->onFrame();
		renderText(Common::String::format("%3d | %3d", _imageIndex, _sectionIndex).c_str(),
		           10, 190, kColorLightGreen);
		_system->updateScreen();
		_system->delayMillis(_delay);
	}

	//deinit timer/sound/..
	stopSound();

	return Common::kNoError;
}

// Emulates DOS int 1A/00
int SupernovaEngine::getDOSTicks() {
	TimeDate systemTime;
	_system->getTimeAndDate(systemTime);

	return static_cast<int>((systemTime.tm_hour * 24 +
	                         systemTime.tm_min  * 60 +
	                         systemTime.tm_sec) * 18.2065);
}

void SupernovaEngine::updateEvents() {
}

void SupernovaEngine::initData() {
	// Images
	for (int i = 0; i < 44; ++i)
		_images[i].init(i);

	// Sound
	Common::File file;

	for (int i = 0; i < kAudioNumSamples; ++i) {
		if (!file.open(Common::String::format("msn_data.%03d", audioInfo[i]._filenumber))) {
			error("File %s could not be read!", file.getName());
		}

		if (audioInfo[i]._offsetEnd == -1) {
			file.seek(0, SEEK_END);
			_soundSamples[i]._length = file.pos() - audioInfo[i]._offsetStart;
		} else {
			_soundSamples[i]._length = audioInfo[i]._offsetEnd - audioInfo[i]._offsetStart;
		}
		_soundSamples[i]._buffer = new byte[_soundSamples[i]._length];
		file.seek(audioInfo[i]._offsetStart);
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

void SupernovaEngine::renderImage(MSNImageDecoder &image, int section, bool fullscreen) {
	if (section > image._numSections - 1)
		return;

	_currentImage = &image;
	_imageIndex = image._filenumber;
	_sectionIndex = section;

	_system->getPaletteManager()->setPalette(image.getPalette(), 16, 239);
	paletteBrightness();

	Common::Rect sectionRect(image._section[section].x1,
	                         image._section[section].y1,
	                         image._section[section].x2,
	                         image._section[section].y2);
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

	if (fullscreen) {
		_system->copyRectToScreen(image._sectionSurfaces[section]->getPixels(),
		                          image._pitch, 0, 0, _screenWidth, _screenHeight);
	} else {
		uint offset = image._section[section].y1 * image._pitch + image._section[section].x1;
		_system->copyRectToScreen(static_cast<const byte *>(image._sectionSurfaces[section]->getPixels()) + offset,
		                          image._pitch,
		                          sectionRect.top, sectionRect.left,
		                          sectionRect.width(), sectionRect.height());
	}
}

void SupernovaEngine::renderImage(int filenumber, int section, bool fullscreen) {
	if (filenumber > ARRAYSIZE(_images) - 1)
		return;

	renderImage(_images[filenumber], section, fullscreen);
}

void SupernovaEngine::saveScreen(int x, int y, int width, int height) {
	_screenBuffer.push(x, y, width, height);
}

void SupernovaEngine::restoreScreen() {
	_screenBuffer.restore();
}

void SupernovaEngine::renderRoom(Room &room) {
	for (int i = 0; i < kMaxSection; ++i) {
		if (room.isSectionVisible(i))
			renderImage(room.getFileNumber(), i);
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

		for (uint i = 0; i < 5; ++i) {
			++charWidth;
			if (font[c - 32][i] == 0xff) {
				break;
			}
		}
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
		textColor = kColorLightYellow;
		break;
	case kMessageCenter:
		x = rowWidthMax / 2 - 160;
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
	renderBox(message_columns, message_rows, message_width, message_height, HGR_MELD);
	for (uint i = 0; i < numRows; ++i) {
		renderText(row[i], x, y, textColor);
		y += 9;
	}

	_messageDisplayed = true;
//	_timer1 = (Common::strnlen(text, BUFSIZ) + 20) * textspeed / 10;
}

void SupernovaEngine::removeMessage() {
	// TODO: restore surface
	_messageDisplayed = false;
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

	uint numChars = cursor - basePtr;
	uint absPosition = y * kScreenWidth + x + numChars;
	_textCursorX = absPosition % kScreenWidth;
	_textCursorY = absPosition / kScreenWidth;
	_textColor = color;
}

void SupernovaEngine::renderText(const char *text) {
	renderText(text, _textCursorX, _textCursorY, _textColor);
}

void SupernovaEngine::renderBox(int x, int y, int width, int height, byte color) {
	Graphics::Surface *screen = _system->lockScreen();
	screen->fillRect(Common::Rect(x, y, width, height), color);
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

void SupernovaEngine::setColor63(byte value) {
	byte color[3] = {value, value, value};
	_system->getPaletteManager()->setPalette(color, 63, 1);
}


Inventory::Inventory()
    : _numObjects(0)
{}

// TODO: Update Inventory surface for scrolling
void Inventory::add(Object &obj) {
	if (_numObjects < kMaxCarry)
		_inventory[_numObjects] = &obj;

//	if (inventory_amount>8) inventory_scroll = ((inventory_amount+1)/2)*2-8;
//	show_inventory();
}

// TODO: Update Inventory surface for scrolling
void Inventory::remove(Object &obj) {
	for (uint i = 0; i < _numObjects; ++i) {
		if (_inventory[i] == &obj) {
			--_numObjects;
			while (i < _numObjects) {
				_inventory[i] = _inventory[i + 1];
				++i;
			}
			obj.disableProperty(CARRIED);
		}
	}
}

Object *Inventory::get(uint index) const {
	if (index < _numObjects)
		return _inventory[index];

	return NULL;
}

Object *Inventory::get(ObjectID id) const {
	for (uint i = 0; i < _numObjects; ++i) {
		if (_inventory[i]->_id == id)
			return _inventory[i];
	}

	return NULL;
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


static const char *timeToString(int t) {
	// TODO: Does ScummVM emulate PIT timings for DOS?

	static char s[9];
	strcpy(s," 0:00:00");
	s[7] = t % 10 + '0';
	t /= 10;
	s[6] = t %  6 + '0';
	t /=  6;
	s[4] = t % 10 + '0';
	t /= 10;
	s[3] = t %  6 + '0';
	t /=  6;
	s[1] = t % 10 + '0';
	t /= 10;
	if (t)
		s[0] = t+48;

	return(s);
}

GameManager::GameManager(SupernovaEngine *vm, Common::Event *event) {
	_rooms[INTRO] = StartingItems(vm, this);
	_rooms[CORRIDOR] = ShipCorridor(vm, this);
	_rooms[HALL] = ShipHall(vm, this);
	_rooms[SLEEP] = ShipSleepCabin(vm, this);
	_rooms[COCKPIT] = ShipCockpit(vm, this);
	_rooms[AIRLOCK] = ShipAirlock(vm, this);
	_rooms[HOLD] = ShipHold(vm, this);
	_rooms[LANDINGMODULE] = ShipLandingModule(vm, this);
	_rooms[GENERATOR] = ShipGenerator(vm, this);
	_rooms[OUTSIDE] = ShipOuterSpace(vm, this);
	_rooms[CABIN_R1] = ShipCabinR1(vm, this);
	_rooms[CABIN_R2] = ShipCabinR2(vm, this);
	_rooms[CABIN_R3] = ShipCabinR3(vm, this);
	_rooms[CABIN_L1] = ShipCabinL1(vm, this);
	_rooms[CABIN_L2] = ShipCabinL2(vm, this);
	_rooms[CABIN_L3] = ShipCabinL3(vm, this);
	_rooms[BATHROOM] = ShipCabinBathroom(vm, this);

//	_rooms[ROCKS]
//	_rooms[CAVE]
//	_rooms[MEETUP]
//	_rooms[ENTRANCE]
//	_rooms[REST]
//	_rooms[ROGER]
//	_rooms[GLIDER]
//	_rooms[MEETUP2]
//	_rooms[MEETUP3]

//	_rooms[CELL]
//	_rooms[CORRIDOR1]
//	_rooms[CORRIDOR2]
//	_rooms[CORRIDOR3]
//	_rooms[CORRIDOR4]
//	_rooms[CORRIDOR5]
//	_rooms[CORRIDOR6]
//	_rooms[CORRIDOR7]
//	_rooms[CORRIDOR8]
//	_rooms[CORRIDOR9]
//	_rooms[BCORRIDOR]
//	_rooms[GUARD]
//	_rooms[GUARD3]
//	_rooms[OFFICE_L1]
//	_rooms[OFFICE_L2]
//	_rooms[OFFICE_R1]
//	_rooms[OFFICE_R2]
//	_rooms[OFFICE_L]
//	_rooms[ELEVATOR]
//	_rooms[STATION]
//	_rooms[SIGN]

	_currentRoom = &_rooms[0];
	_vm = vm;
	_event = event;
}

bool GameManager::isHelmetOff() {
	Object *helmet = _inventory.get(HELMET);
	if (helmet && helmet->hasProperty(WORN)) {
		_vm->renderMessage("Irgendwie ist ein Raumhelm|beim Essen unpraktisch.");
		return false;
	}

	return true;
}

void GameManager::great(uint number) {
	if (number && (_state.greatF & (1 << number)))
		return;

	_vm->playSound(kAudioUndef7);
	_state.greatF |= 1 << number;
}

bool GameManager::airless() {
	return (
	((_currentRoom > &_rooms[AIRLOCK]) && (_currentRoom < &_rooms[CABIN_R1])) ||
	((_currentRoom >  &_rooms[BATHROOM])&& (_currentRoom < &_rooms[ENTRANCE])) ||
	((_currentRoom == &_rooms[AIRLOCK]) && (_currentRoom->getObject(1)->hasProperty(OPENED))) ||
	(_currentRoom  >= &_rooms[MEETUP2])
	);
}

void GameManager::processInput() {
	// STUB
}

void GameManager::turnOff() {
	if (_state.powerOff)
		return;

	_state.powerOff = true;
	roomBrightness();

}
void GameManager::turnOn() {
	// STUB
}

void GameManager::takeObject(Object &obj) {
	if (obj.hasProperty(CARRIED))
		return;

	if (obj._section != 0)
		_vm->renderImage(_currentRoom->getFileNumber(), obj._section);
	obj.setProperty(CARRIED);
	obj._click = obj._click2 = 255;
	_inventory.add(obj);
}

void GameManager::mouseInput() {
	// STUB
}

void GameManager::mouseInput2() {
	// STUB
}

void GameManager::mouseInput3() {
	// STUB
}

void GameManager::mouseWait(int delay) {
	// STUB
}

void GameManager::roomBrightness() {
	// STUB
}

void GameManager::loadTime() {
	// STUB
}

void GameManager::saveTime() {
	// STUB
}

bool GameManager::saveGame(int number) {
	// STUB
	return false;
}

void GameManager::errorTemp() {
	// STUB
}

void GameManager::wait2(int delay) {
	// STUB
}

void GameManager::screenShake() {
	// STUB
}

void GameManager::palette() {
	// STUB
	// Adjust palette to brightness parameters and make it current
}

void GameManager::shock() {
	// STUB
	_vm->playSound(kAudioShock);
	// die
}

void GameManager::showMenu() {
	// STUB
}

void GameManager::drawMapExits() {
	// STUB
}

void GameManager::animationOff() {
	// STUB
}

void GameManager::animationOn() {
	// STUB
}

void GameManager::edit(char *text, int x, int y, int length) {
	// STUB
}

void GameManager::loadOverlayStart() {
	// STUB
}

void GameManager::openLocker(const Room *room, Object *obj, Object *lock, int section) {
	_vm->renderImage(room->getFileNumber(), section);
	obj->setProperty(OPENED);
	lock->_click = 255;
	int i = obj->_click;
	obj->_click = obj->_click2;
	obj->_click2 = i;
}

void GameManager::closeLocker(const Room *room, Object *obj, Object *lock, int section) {
	// STUB
}

int GameManager::invertSection(int section) {
	if (section < 128)
		section += 128;
	else
		section -= 128;

	return section;
}


bool GameManager::genericInteract(Action verb, Object &obj1, Object &obj2) {
	Room *r;
	char t[150];

	if ((verb == ACTION_USE) && (obj1._id == SCHNUCK)) {
		if (isHelmetOff()) {
			takeObject(obj1);
			_vm->renderMessage("Schmeckt ganz gut.");
			_inventory.remove(obj1);
		}
	} else if ((verb == ACTION_USE) && (obj1._id == EGG)) {
		if (isHelmetOff()) {
			takeObject(obj1);
			if (obj1.hasProperty(OPENED))
				_vm->renderMessage("Schmeckt ganz gut.");
			else
				_vm->renderMessage("Da war irgendetwas drin,|aber jetzt hast du es|mit runtergeschluckt.");

			_inventory.remove(obj1);
		}
	} else if ((verb == ACTION_OPEN) && (obj1._id == EGG)) {
		takeObject(obj1);
		if (obj1.hasProperty(OPENED)) {
			_vm->renderMessage("Du hast es doch schon geffnet.");
		} else {
			takeObject(*_rooms[ENTRANCE].getObject(8));
			_vm->renderMessage("In dem Ei ist eine Tablette|in einer Plastikhlle.");
			obj1.setProperty(OPENED);
		}
	} else if ((verb == ACTION_USE) && (obj1._id == PILL)) {
		if (isHelmetOff()) {
			_vm->renderMessage("Du iát die Tablette und merkst,|daá sich irgendetwas verndert hat.");
			great(0);
			_inventory.remove(obj1);
			_state.language = 2;
			takeObject(*_rooms[ENTRANCE].getObject(17));
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == PILL_HULL) &&
	           (_state.language == 2)) {
		_vm->renderMessage("Komisch! Auf einmal kannst du die Schrift lesen!|Darauf steht:\"Wenn Sie diese Schrift jetzt|lesen knnen, hat die Tablette gewirkt.\"");
		_state.language = 1;
	} else if ((verb == ACTION_OPEN) && (obj1._id == WALLET)) {
		if (!_rooms[ROGER].getObject(3)->hasProperty(CARRIED)) {
			_vm->renderMessage("Das muát du erst nehmen.");
		} else if (_rooms[ROGER].getObject(7)->hasProperty(CARRIED)) {
			_vm->renderMessage("Sie ist leer.");
		} else {
			_vm->renderMessage("Du findest 10 Buckazoids und eine Keycard.");
			takeObject(*_rooms[ROGER].getObject(7));
			takeObject(*_rooms[ROGER].getObject(8));
		}
	} else if ((verb == ACTION_LOOK) && (obj1._id == NEWSPAPER)) {
		_vm->renderMessage("Es ist eine Art elektronische Zeitung.");
		mouseWait(_timer1);
		_vm->removeMessage();
		_vm->renderMessage("Halt, hier ist ein interessanter Artikel.");
		mouseWait(_timer1);
		_vm->removeMessage();
		_vm->renderImage(2,0);
		_vm->setColor63(40);
		mouseInput2();
		_vm->renderRoom(*_currentRoom);
		roomBrightness();
		palette();
		showMenu();
		drawMapExits();
		_vm->renderMessage("Hmm, irgendwie komme|ich mir verarscht vor.");
	} else if ((verb == ACTION_LOOK) && (obj1._id == KEYCARD2)) {
		_vm->renderMessage(obj1._description);
		obj1._description = "Es ist die Keycard des Commanders.";
	} else if ((verb == ACTION_LOOK) && (obj1._id == WATCH)) {
		_vm->renderMessage(Common::String::format(
		    "Es ist eine Uhr mit extra|lautem Wecker. "
		    "Sie hat einen|Knopf zum Verstellen der Alarmzeit.|"
		    "Uhrzeit: %s   Alarmzeit: %s",
		    timeToString(_vm->getDOSTicks() - _state.timeStarting),
		    timeToString(_state.timeAlarm)).c_str());
	} else if ((verb == ACTION_PRESS) && (obj1._id == WATCH)) {
		char *min;
		int hours, minutes;
		bool f;
		animationOff();
		_vm->saveScreen(88, 87, 144, 24);
		_vm->renderBox(88, 87, 144, 24, kColorWhite35);
		_vm->renderText("Neue Alarmzeit (hh:mm) :", 91, 90, kColorWhite99);
		do {
			t[0] = 0;
			_vm->renderBox(91, 99, 138, 9, kColorDarkBlue);
			do {
				edit(t, 91, 100, 5);
			} while ((_key != Common::ASCII_RETURN) && (_key != Common::ASCII_ESCAPE));
			f = false;
			if (t[0] == ':') {
				t[0] = 0;
				min = &(t[1]);
			} else if (t[1] == ':') {
				t[1] = 0;
				min = &(t[2]);
			} else if (t[2] == ':') {
				t[2] = 0;
				min = &(t[3]);
			} else {
				f = true;
			}

			for (uint i = 0; i < strlen(t); i++)
				if ((t[i] < '0') || (t[i] > '9')) f = true;
			for (uint i = 0; i < strlen(min); i++)
				if ((min[i] < '0') || (min[i] > '9')) f = true;
			hours = atoi(t);
			minutes = atoi(min);
			if ((hours > 23) || (minutes > 59)) f = true;
			animationOn();
		} while (f && (_key != Common::ASCII_ESCAPE));
		_vm->restoreScreen();
		if (_key != Common::ASCII_ESCAPE) {
			_state.timeAlarm = (hours * 60 + minutes) * 1092.3888 + 8;
			_state.timeAlarmSystem = _state.timeAlarm + _state.timeStarting;
			_state.alarmOn = (_state.timeAlarmSystem > _vm->getDOSTicks());
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, WIRE)) {
		r = &_rooms[CABIN_L3];
		if (!r->getObject(8)->hasProperty(CARRIED)) {
			if (r->isSectionVisible(26))
				_vm->renderMessage(Object::takeMessage);
			else
				return false;
		} else {
			r->getObject(8)->_name = "Leitung mit Lsterklemme";
			r = &_rooms[HOLD];
			_inventory.remove(*r->getObject(2));
			_state.terminalStripConnected = true;
			_state.terminalStripWire = true;
			_vm->renderMessage("Ok.");
		}
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, TERMINALSTRIP, SPOOL)) {
		r = &_rooms[CABIN_L2];
		takeObject(*r->getObject(9));
		r->getObject(9)->_name = "Kabelrolle mit Lsterklemme";
		r = &_rooms[HOLD];
		_inventory.remove(*r->getObject(2));
		_state.terminalStripConnected = true;
		_vm->renderMessage("Ok.");
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, SPOOL)) {
		r = &_rooms[CABIN_L3];
		if (!_state.terminalStripConnected) {
			if (r->isSectionVisible(26))
				_vm->renderMessage("Womit denn?");
			else
				return false;
		} else {
			if (!r->getObject(8)->hasProperty(CARRIED)) {
				_vm->renderMessage(Object::takeMessage);
			} else {
				r = &_rooms[CABIN_L2];
				takeObject(*r->getObject(9));
				r = &_rooms[CABIN_L3];
				r->getObject(8)->_name = "langes Kabel mit Stecker";
				r = &_rooms[CABIN_L2];
				_inventory.remove(*r->getObject(9));
				_state.cableConnected = true;
				_vm->renderMessage("Ok.");
			}
		}
	} else if ((verb == ACTION_USE) && (obj1._id == SUIT)) {
		takeObject(obj1);
		if ((_currentRoom >= &_rooms[ENTRANCE]) && (_currentRoom <= &_rooms[ROGER])) {
			if (obj1.hasProperty(WORN)) {
				_vm->renderMessage("Die Luft hier ist atembar,|du ziehst den Anzug aus.");
				_rooms[AIRLOCK].getObject(4)->disableProperty(WORN);
				_rooms[AIRLOCK].getObject(5)->disableProperty(WORN);
				_rooms[AIRLOCK].getObject(6)->disableProperty(WORN);
			} else
				_vm->renderMessage("Hier drinnen brauchtst du deinen Anzug nicht.");
		} else {
			if (obj1.hasProperty(WORN)) {
				r = &_rooms[AIRLOCK];
				if (r->getObject(4)->hasProperty(WORN)) {
					_vm->renderMessage("Du muát erst den Helm abnehmen.");
				} else if (r->getObject(6)->hasProperty(WORN)) {
					_vm->renderMessage("Du muát erst den Versorgungsteil abnehmen.");
				} else {
					obj1.disableProperty(WORN);
					_vm->renderMessage("Du ziehst den Raumanzug aus.");
				}
			} else {
				obj1.setProperty(WORN);
				_vm->renderMessage("Du ziehst den Raumanzug an.");
			}
		}
	} else if ((verb == ACTION_USE) && (obj1._id == HELMET)) {
		takeObject(obj1);
		if ((_currentRoom >= &_rooms[ENTRANCE]) && (_currentRoom <= &_rooms[ROGER])) {
			if (obj1.hasProperty(WORN)) {
				_vm->renderMessage("Die Luft hier ist atembar,|du ziehst den Anzug aus.");
				_rooms[AIRLOCK].getObject(4)->disableProperty(WORN);
				_rooms[AIRLOCK].getObject(5)->disableProperty(WORN);
				_rooms[AIRLOCK].getObject(6)->disableProperty(WORN);
			} else {
				_vm->renderMessage("Hier drinnen brauchtst du deinen Anzug nicht.");
			}
		} else {
			if (obj1.hasProperty(WORN)) {
				if (airless()) {
					//TODO: Death screen
//					longjmp(dead, "Den Helm httest du|besser angelassen!");
				}
				obj1.disableProperty(WORN);
				_vm->renderMessage("Du ziehst den Helm ab.");
			} else {
				r = &_rooms[AIRLOCK];
				if (r->getObject(5)->hasProperty(WORN)) {
					obj1.setProperty(WORN);
					_vm->renderMessage("Du ziehst den Helm auf.");
				} else {
					_vm->renderMessage("Du muát erst den Anzug anziehen.");
				}
			}
		}
	} else if ((verb == ACTION_USE) && (obj1._id == LIFESUPPORT)) {
		takeObject(obj1);
		if ((_currentRoom >= &_rooms[ENTRANCE]) && (_currentRoom <= &_rooms[ROGER])) {
			if (obj1.hasProperty(WORN)) {
				_vm->renderMessage("Die Luft hier ist atembar,|du ziehst den Anzug aus.");
				_rooms[AIRLOCK].getObject(4)->disableProperty(WORN);
				_rooms[AIRLOCK].getObject(5)->disableProperty(WORN);
				_rooms[AIRLOCK].getObject(6)->disableProperty(WORN);
			} else
				_vm->renderMessage("Hier drinnen brauchtst du deinen Anzug nicht.");
		} else {
			if (obj1.hasProperty(WORN)) {
				if (airless()) {
					//TODO: Death screen
//					longjmp(dead, "Den Versorungsteil httest du|besser nicht abgenommen!");
				}
				obj1.disableProperty(WORN);
				_vm->renderMessage("Du nimmst den Versorgungsteil ab.");
			} else {
				r = &_rooms[AIRLOCK];
				if (r->getObject(5)->hasProperty(WORN)) {
					obj1.setProperty(WORN);
					_vm->renderMessage("Du ziehst den Versorgungsteil an.");
				} else {
					_vm->renderMessage("Du muát erst den Anzug anziehen.");
				}
			}
		}
	} else if ((verb == ACTION_WALK) && (obj1._id == BATHROOM_DOOR)) {
//		*bathroom = current_room;
		return false;
	} else if ((verb == ACTION_USE) && Object::combine(obj1, obj2, WIRE, SOCKET))
		_vm->renderMessage("Die Leitung ist hier unntz.");
	else if ((verb == ACTION_LOOK) && (obj1._id == BOOK2)) {
		_vm->renderMessage("Stark, das ist ja die Fortsetzung zum \"Anhalter\":|\"Das Restaurant am Ende des Universums\".");
		mouseWait(_timer1);
		_vm->removeMessage();
		_vm->renderMessage("Moment mal, es ist ein Lesezeichen drin,|auf dem \"Zweiundvierzig\" steht.");
	} else {
		return false;
	}

	return true;
}

void GameManager::executeRoom() {
	bool validCommand = genericInteract(_inputVerb, _inputObject[0], _inputObject[1]);

	if (!validCommand) {
		validCommand = _currentRoom->interact(_inputVerb, _inputObject[0], _inputObject[1]);
		if (!validCommand) {
			switch (_inputVerb) {
			case ACTION_LOOK:
				_vm->renderMessage(_inputObject[0]._description);
				break;

			case ACTION_WALK:
				if (_inputObject[0].hasProperty(CARRIED)) {
					// You already carry this.
					_vm->renderMessage("Das trgst du doch bei dir.");
				} else if (!_inputObject[0].hasProperty(EXIT)) {
					// You're already there.
					_vm->renderMessage("Du bist doch schon da.");
				} else if (_inputObject[0].hasProperty(OPEN) && !_inputObject[0].hasProperty(OPENED)) {
					// This is closed
					_vm->renderMessage("Das ist geschlossen.");
				} else {
					_currentRoom = &_rooms[_inputObject[0]._exitRoom];
					return;
				}
				break;

			case ACTION_TAKE:
				if (_inputObject[0].hasProperty(OPENED)) {
					// You already have that
					_vm->renderMessage("Das hast du doch schon.");
				} else if (_inputObject[0].hasProperty(UNNECESSARY)) {
					// You do not need that.
					_vm->renderMessage("Das brauchst du nicht.");
				} else if (!_inputObject[0].hasProperty(TAKE)) {
					// You can't take that.
					_vm->renderMessage("Das kannst du nicht nehmen.");
				} else {
					takeObject(_inputObject[0]);
				}
				break;

			case ACTION_OPEN:
				if (!_inputObject[0].hasProperty(OPEN)) {
					// This can't be opened
					_vm->renderMessage("Das lát sich nicht ffnen.");
				} else if (_inputObject[0].hasProperty(OPENED)) {
					// This is already opened.
					_vm->renderMessage("Das ist schon offen.");
				} else if (_inputObject[0].hasProperty(CLOSED)) {
					// This is locked.
					_vm->renderMessage("Das ist verschlossen.");
				} else {
					_vm->renderImage(_currentRoom->getFileNumber(), _inputObject[0]._section);
					_inputObject[0].setProperty(OPENED);
					byte i = _inputObject[0]._click;
					_inputObject[0]._click  = _inputObject[0]._click2;
					_inputObject[0]._click2 = i;
					_vm->playSound(kAudioDoorOpen);
				}
				break;

			case ACTION_CLOSE:
				if (!_inputObject[0].hasProperty(OPEN) ||
				    (_inputObject[0].hasProperty(CLOSED) &&
				     _inputObject[0].hasProperty(OPENED))) {
					// This can't be closed.
					_vm->renderMessage("Das lát sich nicht schlieáen.");
				} else if (!_inputObject[0].hasProperty(OPENED)) {
					// This is already closed.
					_vm->renderMessage("Das ist schon geschlossen.");
				} else {
					_vm->renderImage(_currentRoom->getFileNumber(), invertSection(_inputObject[0]._section));
					_inputObject[0].disableProperty(OPENED);
					byte i = _inputObject[0]._click;
					_inputObject[0]._click  = _inputObject[0]._click2;
					_inputObject[0]._click2 = i;
					_vm->playSound(kAudioDoorClose);
				}
				break;

			case ACTION_GIVE:
				if (_inputObject[0].hasProperty(CARRIED)) {
					// Better keep it!
					_vm->renderMessage("Behalt es lieber!");
				}
				break;

			default:
				// This is not possible.
				_vm->renderMessage("Das geht nicht.");
			}

			if (_newOverlay) {
				loadOverlayStart();
				_newOverlay = false;
			}
			if (_newRoom) {
				_newRoom = false;
				return;
			}
		}
	}
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

	return new Common::MemoryReadStream(buffer.getData(), buffer.size());
}

}
