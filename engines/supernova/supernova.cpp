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


namespace Supernova {

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
	GameManager gm(this, &_event);
	_console = new Console(this, &gm);

	initData();
	initPalette();
	paletteFadeIn();

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

void SupernovaEngine::playSoundMod(int filenumber)
{
	if (filenumber != 49 || filenumber != 52) {
		error("File not supposed to be played!");
	}

	Common::File *file = new Common::File;
	if (!file->open(Common::String::format("msn_data.%03d", filenumber))) {
		error("File %s could not be read!", file->getName());
	}

	// play Supernova MOD file
}

void SupernovaEngine::renderImage(MSNImageDecoder &image, int section, bool fullscreen) {
	_currentImage = &image;
	_imageIndex = image._filenumber;
	_sectionIndex = section;

	image.loadSection(section);
	_system->getPaletteManager()->setPalette(image.getPalette(), 16, 239);
	paletteBrightness();
	if (fullscreen) {
		_system->copyRectToScreen(image.getSurface()->getPixels(),
		                          image._pitch, 0, 0, kScreenWidth, kScreenHeight);
	} else {
		size_t offset = image._section[section].y1 * 320 + image._section[section].x1;
		_system->copyRectToScreen(static_cast<const byte *>(image.getSurface()->getPixels()) + offset,
		                          320,
		                          image._section[section].x1,
		                          image._section[section].y1,
		                          image._section[section].x2 - image._section[section].x1,
		                          image._section[section].y2 - image._section[section].y1);
	}
}

void SupernovaEngine::renderImage(int filenumber, int section, bool fullscreen) {
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

		for (size_t i = 0; i < 5; ++i) {
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
	for (size_t i = 0; i < numRows; ++i) {
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

	size_t numChars = cursor - basePtr;
	size_t absPosition = y * kScreenWidth + x + numChars;
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
	for (size_t i = 0; i < 48; ++i) {
		palette[i] = (initVGAPalette[i] * _menuBrightness) >> 8;
	}
	for (size_t i = 0; i < 717; ++i) {
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
	for (size_t i = 0; i < _numObjects; ++i) {
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

Object *Inventory::get(size_t index) const {
	if (index < _numObjects)
		return _inventory[index];

	return NULL;
}

Object *Inventory::get(ObjectID id) const {
	for (size_t i = 0; i < _numObjects; ++i) {
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

	byte *pixels = new byte[width * height];
	const byte *screen = static_cast<byte *>(g_system->lockScreen()->getBasePtr(x, y));
	for (int i = 0; i < height; ++i) {
		Common::copy(screen, screen + width, pixels);
		screen += pitch * i;
	}
	g_system->unlockScreen();

	_last->_x = x;
	_last->_y = y;
	_last->_width = width;
	_last->_height = height;
	_last->_pitch = pitch;
	_last->_pixels = pixels;

	++_last;
}

void ScreenBufferStack::restore() {
	if (_last == _buffer)
		return;

	g_system->lockScreen()->copyRectToSurface(
	            _last->_pixels, _last->_width, _last->_x, _last->_y,
	            _last->_width, _last->_height);
	g_system->unlockScreen();
	--_last;
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

	_vm->playSound(54, 8010);
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
	_vm->playSound(48, 10520);
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

			for (size_t i = 0; i < strlen(t); i++)
				if ((t[i] < '0') || (t[i] > '9')) f = true;
			for (size_t i = 0; i < strlen(min); i++)
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
					_vm->playSound(54, 30030);
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
					_vm->playSound(54, 31040);
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


}
