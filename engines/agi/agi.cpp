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
 * $URL$
 * $Id$
 *
 */

#include "common/md5.h"
#include "common/events.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/opcodes.h"
#include "agi/keyboard.h"
#include "agi/menu.h"
#include "agi/sound.h"

namespace Agi {

static uint32 g_tickTimer;
struct Mouse g_mouse;

void AgiEngine::allowSynthetic(bool allow) {
	_allowSynthetic = allow;
}

void AgiEngine::processEvents() {
	Common::Event event;
	int key = 0;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_PREDICTIVE_DIALOG:
			if (_predictiveDialogRunning)
				break;
			if (predictiveDialog()) {
				if (_game.inputMode == INPUT_NORMAL) {
					strcpy((char *)_game.inputBuffer, _predictiveResult);
					handleKeys(KEY_ENTER);
				} else if (_game.inputMode == INPUT_GETSTRING) {
					strcpy(_game.strings[_stringdata.str], _predictiveResult);
					newInputMode(INPUT_NORMAL);
					_gfx->printCharacter(_stringdata.x + strlen(_game.strings[_stringdata.str]) + 1,
							_stringdata.y, ' ', _game.colorFg, _game.colorBg);
				} else if (_game.inputMode == INPUT_NONE) {
					for (int n = 0; _predictiveResult[n]; n++)
						keyEnqueue(_predictiveResult[n]);
				}
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			key = BUTTON_LEFT;
			g_mouse.button = kAgiMouseButtonLeft;
			keyEnqueue(key);
			g_mouse.x = event.mouse.x;
			g_mouse.y = event.mouse.y;
			break;
		case Common::EVENT_RBUTTONDOWN:
			key = BUTTON_RIGHT;
			g_mouse.button = kAgiMouseButtonRight;
			keyEnqueue(key);
			g_mouse.x = event.mouse.x;
			g_mouse.y = event.mouse.y;
			break;
		case Common::EVENT_WHEELUP:
			key = WHEEL_UP;
			keyEnqueue(key);
			break;
		case Common::EVENT_WHEELDOWN:
			key = WHEEL_DOWN;
			keyEnqueue(key);
			break;
		case Common::EVENT_MOUSEMOVE:
			g_mouse.x = event.mouse.x;
			g_mouse.y = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			g_mouse.button = kAgiMouseButtonUp;
			g_mouse.x = event.mouse.x;
			g_mouse.y = event.mouse.y;
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.flags == Common::KBD_CTRL && event.kbd.keycode == Common::KEYCODE_d) {
				_console->attach();
				break;
			}

			switch (key = event.kbd.keycode) {
			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_LEFT;
				break;
			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_RIGHT;
				break;
			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_UP;
				break;
			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP2:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_DOWN;
				break;
			case Common::KEYCODE_PAGEUP:
			case Common::KEYCODE_KP9:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_UP_RIGHT;
				break;
			case Common::KEYCODE_PAGEDOWN:
			case Common::KEYCODE_KP3:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_DOWN_RIGHT;
				break;
			case Common::KEYCODE_HOME:
			case Common::KEYCODE_KP7:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_UP_LEFT;
				break;
			case Common::KEYCODE_END:
			case Common::KEYCODE_KP1:
				if (_allowSynthetic || !event.synthetic)
					key = KEY_DOWN_LEFT;
				break;
			case Common::KEYCODE_KP5:
				key = KEY_STATIONARY;
				break;
			case Common::KEYCODE_PLUS:
				key = '+';
				break;
			case Common::KEYCODE_MINUS:
				key = '-';
				break;
			case Common::KEYCODE_TAB:
				key = 0x0009;
				break;
			case Common::KEYCODE_F1:
				key = 0x3b00;
				break;
			case Common::KEYCODE_F2:
				key = 0x3c00;
				break;
			case Common::KEYCODE_F3:
				key = 0x3d00;
				break;
			case Common::KEYCODE_F4:
				key = 0x3e00;
				break;
			case Common::KEYCODE_F5:
				key = 0x3f00;
				break;
			case Common::KEYCODE_F6:
				key = 0x4000;
				break;
			case Common::KEYCODE_F7:
				key = 0x4100;
				break;
			case Common::KEYCODE_F8:
				key = 0x4200;
				break;
			case Common::KEYCODE_F9:
				key = 0x4300;
				break;
			case Common::KEYCODE_F10:
				key = 0x4400;
				break;
			case Common::KEYCODE_F11:
				key = KEY_STATUSLN;
				break;
			case Common::KEYCODE_F12:
				key = KEY_PRIORITY;
				break;
			case Common::KEYCODE_ESCAPE:
				key = 0x1b;
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				key = KEY_ENTER;
				break;
			case Common::KEYCODE_BACKSPACE:
				key = KEY_BACKSPACE;
				break;
			default:
				// Not a special key, so get the ASCII code for it
				key = event.kbd.ascii;

				// Function isalpha is defined in <ctype.h> so the following applies to it:
				//
				// The C Programming Language Standard states:
				//   The header <ctype.h> declares several functions useful for classifying
				//   and mapping characters. In all cases the argument is an int, the value
				//   of which shall be representable as an unsigned char or shall equal the
				//   value of the macro EOF. If the argument has any other value, the
				//   behavior is undefined.
				//
				// For a concrete example (e.g. in Microsoft Visual Studio 2003):
				//   When used with a debug CRT library, isalpha will display a CRT assert
				//   if passed a parameter that isn't EOF or in the range of 0 through 0xFF.
				if (key >= 0 && key <= 0xFF && isalpha(key)) {
					// Key is A-Z.
					// Map Ctrl-A to 1, Ctrl-B to 2, etc.
					if (event.kbd.flags & Common::KBD_CTRL) {
						key = toupper(key) - 'A' + 1;
					} else if (event.kbd.flags & Common::KBD_ALT) {
						// Map Alt-A, Alt-B etc. to special scancode values according to an internal scancode table.
						key = scancodeTable[toupper(key) - 'A'] << 8;
					}
				}
				break;
			}
			if (key)
				keyEnqueue(key);
			break;
		default:
			break;
		}
	}
}

void AgiEngine::checkQuickLoad() {
	if (ConfMan.hasKey("save_slot")) {
		char saveNameBuffer[256];

		snprintf (saveNameBuffer, 256, "%s.%03d", _targetName.c_str(), ConfMan.getInt("save_slot"));

		if (loadGame(saveNameBuffer, false) == errOK) {	 // Do not check game id
			_game.exitAllLogics = 1;
			_menu->enableAll();
		}
	}
}

int AgiEngine::agiIsKeypressLow() {
	processEvents();
	return _keyQueueStart != _keyQueueEnd;
}

void AgiEngine::agiTimerLow() {
	static uint32 m = 0;
	uint32 dm;

	if (g_tickTimer < m)
		m = 0;

	while ((dm = g_tickTimer - m) < 5) {
		processEvents();
		if (_console->isAttached())
			_console->onFrame();
		_system->delayMillis(10);
		_system->updateScreen();
	}
	m = g_tickTimer;
}

int AgiEngine::agiGetKeypressLow() {
	int k;

	while (_keyQueueStart == _keyQueueEnd)	// block
		agiTimerLow();
	keyDequeue(k);

	return k;
}

void AgiEngine::agiTimerFunctionLow(void *refCon) {
	g_tickTimer++;
}

void AgiEngine::clearImageStack(void) {
	_imageStack.clear();
}

void AgiEngine::releaseImageStack(void) {
	_imageStack.clear();
}

void AgiEngine::pause(uint32 msec) {
	uint32 endTime = _system->getMillis() + msec;

	_gfx->setCursor(_renderMode == Common::kRenderAmiga, true);

	while (_system->getMillis() < endTime) {
		processEvents();
		_system->updateScreen();
		_system->delayMillis(10);
	}
	_gfx->setCursor(_renderMode == Common::kRenderAmiga);
}

void AgiEngine::recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {
	ImageStackElement pnew;

	pnew.type = type;
	pnew.pad = 0;
	pnew.parm1 = p1;
	pnew.parm2 = p2;
	pnew.parm3 = p3;
	pnew.parm4 = p4;
	pnew.parm5 = p5;
	pnew.parm6 = p6;
	pnew.parm7 = p7;

	_imageStack.push(pnew);
}

void AgiEngine::replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {
	switch (type) {
	case ADD_PIC:
		debugC(8, kDebugLevelMain, "--- decoding picture %d ---", p1);
		agiLoadResource(rPICTURE, p1);
		_picture->decodePicture(p1, p2, p3 != 0);
		break;
	case ADD_VIEW:
		agiLoadResource(rVIEW, p1);
		_sprites->addToPic(p1, p2, p3, p4, p5, p6, p7);
		break;
	}
}

void AgiEngine::initPriTable() {
	int i, p, y = 0;

	for (p = 1; p < 15; p++) {
		for (i = 0; i < 12; i++) {
			_game.priTable[y++] = p < 4 ? 4 : p;
		}
	}
}

int AgiEngine::agiInit() {
	int ec, i;

	debug(2, "initializing");
	debug(2, "game.ver = 0x%x", _game.ver);

	// initialize with adj.ego.move.to.x.y(0, 0) so to speak
	_game.adjMouseX = _game.adjMouseY = 0;

	// reset all flags to false and all variables to 0
	for (i = 0; i < MAX_FLAGS; i++)
		_game.flags[i] = 0;
	for (i = 0; i < MAX_VARS; i++)
		_game.vars[i] = 0;

	// clear all resources and events
	for (i = 0; i < MAX_DIRS; i++) {
		memset(&_game.views[i], 0, sizeof(struct AgiView));
		memset(&_game.pictures[i], 0, sizeof(struct AgiPicture));
		memset(&_game.logics[i], 0, sizeof(struct AgiLogic));
		memset(&_game.sounds[i], 0, sizeof(class AgiSound *)); // _game.sounds contains pointers now
		memset(&_game.dirView[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirPic[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirLogic[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirSound[i], 0, sizeof(struct AgiDir));
	}

	// clear view table
	for (i = 0; i < MAX_VIEWTABLE; i++)
		memset(&_game.viewTable[i], 0, sizeof(VtEntry));

	initWords();

	if (!_menu)
		_menu = new Menu(this, _gfx, _picture);

	initPriTable();

	// clear string buffer
	for (i = 0; i < MAX_STRINGS; i++)
		_game.strings[i][0] = 0;

	// setup emulation

	switch (_loader->getIntVersion() >> 12) {
	case 2:
		report("Emulating Sierra AGI v%x.%03x\n",
				(int)(agiGetRelease() >> 12) & 0xF,
				(int)(agiGetRelease()) & 0xFFF);
		break;
	case 3:
		report("Emulating Sierra AGI v%x.002.%03x\n",
				(int)(agiGetRelease() >> 12) & 0xF,
				(int)(agiGetRelease()) & 0xFFF);
		break;
	}

	if (getPlatform() == Common::kPlatformAmiga)
		_game.gameFlags |= ID_AMIGA;

	if (getFeatures() & GF_AGDS)
		_game.gameFlags |= ID_AGDS;

	// Make the 256 color AGI screen the default AGI screen when AGI256 or AGI256-2 is used
	if (getFeatures() & (GF_AGI256 | GF_AGI256_2))
		_game.sbuf = _game.sbuf256c;

	if (_game.gameFlags & ID_AMIGA)
		report("Amiga padded game detected.\n");

	if (_game.gameFlags & ID_AGDS)
		report("AGDS mode enabled.\n");

	ec = _loader->init();	// load vol files, etc

	if (ec == errOK)
		ec = _loader->loadObjects(OBJECTS);

	// note: demogs has no words.tok
	if (ec == errOK)
		ec = _loader->loadWords(WORDS);

	// FIXME: load IIgs instruments and samples
	// load_instruments("kq.sys16");

	// Load logic 0 into memory
	if (ec == errOK)
		ec = _loader->loadResource(rLOGIC, 0);

#ifdef __DS__
	// Normally, the engine loads the predictive text dictionary when the predictive dialog
	// is shown.  On the DS version, the word completion feature needs the dictionary too.
	loadDict();
#endif

	return ec;
}

/*
 * Public functions
 */

void AgiEngine::agiUnloadResources() {
	int i;

	// Make sure logic 0 is always loaded
	for (i = 1; i < MAX_DIRS; i++) {
		_loader->unloadResource(rLOGIC, i);
	}
	for (i = 0; i < MAX_DIRS; i++) {
		_loader->unloadResource(rVIEW, i);
		_loader->unloadResource(rPICTURE, i);
		_loader->unloadResource(rSOUND, i);
	}
}

int AgiEngine::agiDeinit() {
	int ec;

	cleanInput();		// remove all words from memory
	agiUnloadResources();	// unload resources in memory
	_loader->unloadResource(rLOGIC, 0);
	ec = _loader->deinit();
	unloadObjects();
	unloadWords();

	clearImageStack();

	return ec;
}

int AgiEngine::agiDetectGame() {
	int ec = errOK;

	assert(_gameDescription != NULL);

	if (getVersion() <= 0x2999) {
		_loader = new AgiLoader_v2(this);
	} else {
		_loader = new AgiLoader_v3(this);
	}
	ec = _loader->detectGame();

	return ec;
}

int AgiEngine::agiVersion() {
	return _loader->version();
}

int AgiEngine::agiGetRelease() {
	return _loader->getIntVersion();
}

void AgiEngine::agiSetRelease(int n) {
	_loader->setIntVersion(n);
}

int AgiEngine::agiLoadResource(int r, int n) {
	int i;

	i = _loader->loadResource(r, n);

	// WORKAROUND: Patches broken picture 147 in a corrupted Amiga version of Gold Rush! (v2.05 1989-03-09).
	// The picture can be seen in room 147 after dropping through the outhouse's hole in room 146.
	if (i == errOK && getGameID() == GID_GOLDRUSH && r == rPICTURE && n == 147 && _game.dirPic[n].len == 1982) {
		uint8 *pic = _game.pictures[n].rdata;
		Common::MemoryReadStream picStream(pic, _game.dirPic[n].len);
		char md5str[32+1];
		Common::md5_file_string(picStream, md5str, _game.dirPic[n].len);
		if (scumm_stricmp(md5str, "1c685eb048656cedcee4eb6eca2cecea") == 0) {
			pic[0x042] = 0x4B; // 0x49 -> 0x4B
			pic[0x043] = 0x66; // 0x26 -> 0x66
			pic[0x204] = 0x68; // 0x28 -> 0x68
			pic[0x6C0] = 0x2D; // 0x25 -> 0x2D
			pic[0x6F0] = 0xF0; // 0x70 -> 0xF0
			pic[0x734] = 0x6F; // 0x2F -> 0x6F
		}
	}

	return i;
}

int AgiEngine::agiUnloadResource(int r, int n) {
	return _loader->unloadResource(r, n);
}

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

static const GameSettings agiSettings[] = {
	{"agi", "AGI game", GID_AGI, MDT_ADLIB, "OBJECT"},
	{NULL, NULL, 0, 0, NULL}
};

AgiTextColor AgiButtonStyle::getColor(bool hasFocus, bool pressed, bool positive) const {
	if (_amigaStyle) {
		if (positive) {
			if (pressed) { // Positive pressed Amiga-style button
				if (_olderAgi) {
					return AgiTextColor(amigaBlack, amigaOrange);
				} else {
					return AgiTextColor(amigaBlack, amigaPurple);
				}
			} else { // Positive unpressed Amiga-style button
				return AgiTextColor(amigaWhite, amigaGreen);
			}
		} else { // _amigaStyle && !positive
			if (pressed) { // Negative pressed Amiga-style button
				return AgiTextColor(amigaBlack, amigaCyan);
			} else { // Negative unpressed Amiga-style button
				return AgiTextColor(amigaWhite, amigaRed);
			}
		}
	} else { // PC-style button
		if (hasFocus || pressed) { // A pressed or in focus PC-style button
			return AgiTextColor(pcWhite, pcBlack);
		} else { // An unpressed PC-style button without focus
			return AgiTextColor(pcBlack, pcWhite);
		}
	}
}

AgiTextColor AgiButtonStyle::getColor(bool hasFocus, bool pressed, int baseFgColor, int baseBgColor) const {
	return getColor(hasFocus, pressed, AgiTextColor(baseFgColor, baseBgColor));
}

AgiTextColor AgiButtonStyle::getColor(bool hasFocus, bool pressed, const AgiTextColor &baseColor) const {
	if (hasFocus || pressed)
		return baseColor.swap();
	else
		return baseColor;
}

int AgiButtonStyle::getTextOffset(bool hasFocus, bool pressed) const {
	return (pressed && !_amigaStyle) ? 1 : 0;
}

bool AgiButtonStyle::getBorder(bool hasFocus, bool pressed) const {
	return _amigaStyle && !_authenticAmiga && (hasFocus || pressed);
}

void AgiButtonStyle::setAmigaStyle(bool amigaStyle, bool olderAgi, bool authenticAmiga) {
	_amigaStyle		= amigaStyle;
	_olderAgi		= olderAgi;
	_authenticAmiga	= authenticAmiga;
}

void AgiButtonStyle::setPcStyle(bool pcStyle) {
	setAmigaStyle(!pcStyle);
}

AgiButtonStyle::AgiButtonStyle(Common::RenderMode renderMode) {
	setAmigaStyle(renderMode == Common::kRenderAmiga);
}

AgiBase::AgiBase(OSystem *syst, const AGIGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_noSaveLoadAllowed = false;
}

AgiEngine::AgiEngine(OSystem *syst, const AGIGameDescription *gameDesc) : AgiBase(syst, gameDesc) {

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = agiSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

	_rnd = new Common::RandomSource();
	syst->getEventManager()->registerRandomSource(*_rnd, "agi");

	Common::addDebugChannel(kDebugLevelMain, "Main", "Generic debug level");
	Common::addDebugChannel(kDebugLevelResources, "Resources", "Resources debugging");
	Common::addDebugChannel(kDebugLevelSprites, "Sprites", "Sprites debugging");
	Common::addDebugChannel(kDebugLevelInventory, "Inventory", "Inventory debugging");
	Common::addDebugChannel(kDebugLevelInput, "Input", "Input events debugging");
	Common::addDebugChannel(kDebugLevelMenu, "Menu", "Menu debugging");
	Common::addDebugChannel(kDebugLevelScripts, "Scripts", "Scripts debugging");
	Common::addDebugChannel(kDebugLevelSound, "Sound", "Sound debugging");
	Common::addDebugChannel(kDebugLevelText, "Text", "Text output debugging");
	Common::addDebugChannel(kDebugLevelSavegame, "Savegame", "Saving & restoring game debugging");


	memset(&_game, 0, sizeof(struct AgiGame));
	memset(&_debug, 0, sizeof(struct AgiDebug));
	memset(&g_mouse, 0, sizeof(struct Mouse));

	_game.clockEnabled = false;
	_game.state = STATE_INIT;

	_keyQueueStart = 0;
	_keyQueueEnd = 0;

	_allowSynthetic = false;

	g_tickTimer = 0;

	_intobj = NULL;

	_menu = NULL;

	_lastSentence[0] = 0;
	memset(&_stringdata, 0, sizeof(struct StringData));

	_objects = NULL;

	_restartGame = false;

	_oldMode = -1;

	_predictiveDialogRunning = false;
	_predictiveDictText = NULL;
	_predictiveDictLine = NULL;
	_predictiveDictLineCount = 0;
	_firstSlot = 0;
}

void AgiEngine::initialize() {
	// TODO: Some sound emulation modes do not fit our current music
	//       drivers, and I'm not sure what they are. For now, they might
	//       as well be called "PC Speaker" and "Not PC Speaker".

	// If used platform is Apple IIGS then we must use Apple IIGS sound emulation
	// because Apple IIGS AGI games use only Apple IIGS specific sound resources.
	if (getPlatform() == Common::kPlatformApple2GS) {
		_soundemu = SOUND_EMU_APPLE2GS;
	} else {
		switch (MidiDriver::detectMusicDriver(MDT_PCSPK)) {
		case MD_PCSPK:
			_soundemu = SOUND_EMU_PC;
			break;
		default:
			_soundemu = SOUND_EMU_NONE;
			break;
		}
	}

	if (ConfMan.hasKey("render_mode")) {
		_renderMode = Common::parseRenderMode(ConfMan.get("render_mode").c_str());
	} else if (ConfMan.hasKey("platform")) {
		switch (Common::parsePlatform(ConfMan.get("platform"))) {
		case Common::kPlatformAmiga:
			_renderMode = Common::kRenderAmiga;
			break;
		case Common::kPlatformPC:
			_renderMode = Common::kRenderEGA;
			break;
		default:
			_renderMode = Common::kRenderEGA;
			break;
		}
	}

	_buttonStyle = AgiButtonStyle(_renderMode);
	_defaultButtonStyle = AgiButtonStyle();
	_console = new Console(this);
	_gfx = new GfxMgr(this);
	_sound = new SoundMgr(this, _mixer);
	_picture = new PictureMgr(this, _gfx);
	_sprites = new SpritesMgr(this, _gfx);

	_gfx->initMachine();

	_game.gameFlags = 0;

	_game.colorFg = 15;
	_game.colorBg = 0;

	_game.name[0] = '\0';

	_game.sbufOrig = (uint8 *)calloc(_WIDTH, _HEIGHT * 2); // Allocate space for two AGI screens vertically
	_game.sbuf16c  = _game.sbufOrig + SBUF16_OFFSET; // Make sbuf16c point to the 16 color (+control line & priority info) AGI screen
	_game.sbuf256c = _game.sbufOrig + SBUF256_OFFSET; // Make sbuf256c point to the 256 color AGI screen
	_game.sbuf     = _game.sbuf16c; // Make sbuf point to the 16 color (+control line & priority info) AGI screen by default

	_gfx->initVideo();
	_sound->initSound();

	_lastSaveTime = 0;

	_timer->installTimerProc(agiTimerFunctionLow, 10 * 1000, NULL);

	_game.ver = -1;		// Don't display the conf file warning

	debugC(2, kDebugLevelMain, "Detect game");


	if (agiDetectGame() == errOK) {
		_game.state = STATE_LOADED;
		debugC(2, kDebugLevelMain, "game loaded");
	} else {
		report("Could not open AGI game");
	}

	debugC(2, kDebugLevelMain, "Init sound");
}

AgiEngine::~AgiEngine() {
	// If the engine hasn't been initialized yet via AgiEngine::initialize(), don't attempt to free any resources,
	// as they haven't been allocated. Fixes bug #1742432 - AGI: Engine crashes if no game is detected
	if (_game.state == STATE_INIT) {
		delete _rnd;	// delete _rnd, as it is allocated in the constructor, not in initialize()
		return;
	}

	agiDeinit();
	delete _loader;
	_sound->deinitSound();
	delete _sound;
	_gfx->deinitVideo();
	delete _sprites;
	delete _picture;
	free(_game.sbufOrig);
	_gfx->deinitMachine();
	delete _gfx;
	delete _rnd;
	delete _console;

	free(_predictiveDictLine);
	free(_predictiveDictText);
}

Common::Error AgiBase::init() {

	// Initialize backend
	initGraphics(320, 200, false);

	initialize();

	_gfx->gfxSetPalette();

	return Common::kNoError;
}

Common::Error AgiEngine::go() {
	CursorMan.showMouse(true);

	report(" \nAGI engine %s is ready.\n", gScummVMVersion);
	if (_game.state < STATE_LOADED) {
		do {
			mainCycle();
		} while (_game.state < STATE_RUNNING);
		if (_game.ver < 0)
			_game.ver = 0;	// Enable conf file warning
	}

	runGame();

	return Common::kNoError;
}

void AgiEngine::syncSoundSettings() {
	// FIXME/TODO: Please explain why we are using "music_volume" for all
	// three different entries here.
	int soundVolumeMusic = ConfMan.getInt("music_volume");
	int soundVolumeSFX = ConfMan.getInt("music_volume");
	int soundVolumeSpeech = ConfMan.getInt("music_volume");

	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolumeSFX);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, soundVolumeSpeech);
}

} // End of namespace Agi
