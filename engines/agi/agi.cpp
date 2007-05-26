/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
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

#include "common/stdafx.h"

#include "common/events.h"
#include "common/file.h"
//#include "common/fs.h"
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
		case Common::EVENT_QUIT:
			_gfx->deinitVideo();
			_gfx->deinitMachine();
			_system->quit();
			break;
		case Common::EVENT_LBUTTONDOWN:
			key = BUTTON_LEFT;
			g_mouse.button = 1;
			keyEnqueue(key);
			g_mouse.x = event.mouse.x;
			g_mouse.y = event.mouse.y;
			break;
		case Common::EVENT_RBUTTONDOWN:
			key = BUTTON_RIGHT;
			g_mouse.button = 2;
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
			g_mouse.button = 0;
			break;
		case Common::EVENT_KEYDOWN:
			_keyControl = 0;
			_keyAlt = 0;

			if (event.kbd.flags == Common::KBD_CTRL && event.kbd.keycode == 'd') {
				_console->attach();
				break;
			}

			if (event.kbd.flags & Common::KBD_CTRL)
				_keyControl = 1;

			if (event.kbd.flags & Common::KBD_ALT)
				_keyAlt = 1;

			switch (key = event.kbd.keycode) {
			case 256 + 20:	// left arrow
			case 260:	// key pad 4
				if (_allowSynthetic || !event.synthetic)
					key = KEY_LEFT;
				break;
			case 256 + 19:	// right arrow
			case 262:	// key pad 6
				if (_allowSynthetic || !event.synthetic)
					key = KEY_RIGHT;
				break;
			case 256 + 17:	// up arrow
			case 264:	// key pad 8
				if (_allowSynthetic || !event.synthetic)
					key = KEY_UP;
				break;
			case 256 + 18:	// down arrow
			case 258:	// key pad 2
				if (_allowSynthetic || !event.synthetic)
					key = KEY_DOWN;
				break;
			case 256 + 24:	// page up
			case 265:	// key pad 9
				if (_allowSynthetic || !event.synthetic)
					key = KEY_UP_RIGHT;
				break;
			case 256 + 25:	// page down
			case 259:	// key pad 3
				if (_allowSynthetic || !event.synthetic)
					key = KEY_DOWN_RIGHT;
				break;
			case 256 + 22:	// home
			case 263:	// key pad 7
				if (_allowSynthetic || !event.synthetic)
					key = KEY_UP_LEFT;
				break;
			case 256 + 23:	// end
			case 257:	// key pad 1
				if (_allowSynthetic || !event.synthetic)
					key = KEY_DOWN_LEFT;
				break;
			case 261:	// key pad 5
				key = KEY_STATIONARY;
				break;
			case '+':
				key = '+';
				break;
			case '-':
				key = '-';
				break;
			case 9:
				key = 0x0009;
				break;
			case 282:
				key = 0x3b00;
				break;
			case 283:
				key = 0x3c00;
				break;
			case 284:
				key = 0x3d00;
				break;
			case 285:
				key = 0x3e00;
				break;
			case 286:
				key = 0x3f00;
				break;
			case 287:
				key = 0x4000;
				break;
			case 288:
				key = 0x4100;
				break;
			case 289:
				key = 0x4200;
				break;
			case 290:
				key = 0x4300;
				break;
			case 291:
				key = 0x4400;
				break;
			case 292:
				key = KEY_STATUSLN;
				break;
			case 293:
				key = KEY_PRIORITY;
				break;
			case 27:
				key = 0x1b;
				break;
			case '\n':
			case '\r':
				key = KEY_ENTER;
				break;
			default:
				if (key < 256 && !isalpha(key)) {
					// Make sure backspace works right (this fixes a small issue on OS X)
					if (key != 8)
						key = event.kbd.ascii;
					break;
				}
				if (_keyControl)
					key = (key & ~0x20) - 0x40;
				else if (_keyAlt)
					key = scancodeTable[(key & ~0x20) - 0x41] << 8;
				else if (event.kbd.flags & Common::KBD_SHIFT)
					key = event.kbd.ascii;
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

	while (_keyQueueStart == _keyQueueEnd)	/* block */
		agiTimerLow();
	keyDequeue(k);

	return k;
}

void AgiEngine::agiTimerFunctionLow(void *refCon) {
	g_tickTimer++;
}

void AgiEngine::clearImageStack(void) {
	_imageStackPointer = 0;
}

void AgiEngine::releaseImageStack(void) {
	if (_imageStack)
		free(_imageStack);
	_imageStack = NULL;
	_stackSize = 0;
	_imageStackPointer = 0;
}

void AgiEngine::recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {
	struct ImageStackElement *pnew;

	if (_imageStackPointer == _stackSize) {
		if (_stackSize == 0) {	/* first call */
			_imageStack = (ImageStackElement *)malloc(INITIAL_IMAGE_STACK_SIZE * sizeof(ImageStackElement));
			_stackSize = INITIAL_IMAGE_STACK_SIZE;
		} else {	/* has to grow */
			struct ImageStackElement *newStack;
			newStack = (ImageStackElement *)malloc(2 * _stackSize * sizeof(ImageStackElement));
			memcpy(newStack, _imageStack, _stackSize * sizeof(ImageStackElement));
			free(_imageStack);
			_imageStack = newStack;
			_stackSize *= 2;
		}
	}

	pnew = &_imageStack[_imageStackPointer];
	_imageStackPointer++;

	pnew->type = type;
	pnew->parm1 = p1;
	pnew->parm2 = p2;
	pnew->parm3 = p3;
	pnew->parm4 = p4;
	pnew->parm5 = p5;
	pnew->parm6 = p6;
	pnew->parm7 = p7;
}

void AgiEngine::replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {
	switch (type) {
	case ADD_PIC:
		debugC(8, kDebugLevelMain, "--- decoding picture %d ---", p1);
		agiLoadResource(rPICTURE, p1);
		_picture->decodePicture(p1, p2);
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

	/* reset all flags to false and all variables to 0 */
	for (i = 0; i < MAX_FLAGS; i++)
		_game.flags[i] = 0;
	for (i = 0; i < MAX_VARS; i++)
		_game.vars[i] = 0;

	/* clear all resources and events */
	for (i = 0; i < MAX_DIRS; i++) {
		memset(&_game.views[i], 0, sizeof(struct AgiView));
		memset(&_game.pictures[i], 0, sizeof(struct AgiPicture));
		memset(&_game.logics[i], 0, sizeof(struct AgiLogic));
		memset(&_game.sounds[i], 0, sizeof(struct AgiSound));
		memset(&_game.dirView[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirPic[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirLogic[i], 0, sizeof(struct AgiDir));
		memset(&_game.dirSound[i], 0, sizeof(struct AgiDir));
	}

	/* clear view table */
	for (i = 0; i < MAX_VIEWTABLE; i++)
		memset(&_game.viewTable[i], 0, sizeof(VtEntry));

	initWords();

	if (!_menu)
		_menu = new Menu(this, _gfx, _picture);

	initPriTable();

	/* clear string buffer */
	for (i = 0; i < MAX_STRINGS; i++)
		_game.strings[i][0] = 0;

	/* setup emulation */

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

	if (_game.gameFlags & ID_AMIGA)
		report("Amiga padded game detected.\n");

	if (_game.gameFlags & ID_AGDS)
		report("AGDS mode enabled.\n");

	ec = _loader->init();	/* load vol files, etc */

	if (ec == errOK)
		ec = _loader->loadObjects(OBJECTS);

	/* note: demogs has no words.tok */
	if (ec == errOK)
		ec = _loader->loadWords(WORDS);

	/* FIXME: load IIgs instruments and samples */
	/* load_instruments("kq.sys16"); */

	/* Load logic 0 into memory */
	if (ec == errOK)
		ec = _loader->loadResource(rLOGIC, 0);


	if (ConfMan.hasKey("save_slot")) {
		char saveNameBuffer[256];

		snprintf (saveNameBuffer, 256, "%s.%03d", _targetName.c_str(), ConfMan.getInt("save_slot"));

		loadGame(saveNameBuffer, false); // Do not check game id
	}

	return ec;
}

/*
 * Public functions
 */

void AgiEngine::agiUnloadResources() {
	int i;

	/* Make sure logic 0 is always loaded */
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

	cleanInput();		/* remove all words from memory */
	agiUnloadResources();	/* unload resources in memory */
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

	if(getVersion() <= 0x2999) {
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

AgiEngine::AgiEngine(OSystem *syst) : Engine(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = agiSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

	_rnd = new Common::RandomSource();

	Common::addSpecialDebugLevel(kDebugLevelMain, "Main", "Generic debug level");
	Common::addSpecialDebugLevel(kDebugLevelResources, "Resources", "Resources debugging");
	Common::addSpecialDebugLevel(kDebugLevelSprites, "Sprites", "Sprites debugging");
	Common::addSpecialDebugLevel(kDebugLevelInventory, "Inventory", "Inventory debugging");
	Common::addSpecialDebugLevel(kDebugLevelInput, "Input", "Input events debugging");
	Common::addSpecialDebugLevel(kDebugLevelMenu, "Menu", "Menu debugging");
	Common::addSpecialDebugLevel(kDebugLevelScripts, "Scripts", "Scripts debugging");
	Common::addSpecialDebugLevel(kDebugLevelSound, "Sound", "Sound debugging");
	Common::addSpecialDebugLevel(kDebugLevelText, "Text", "Text output debugging");
	Common::addSpecialDebugLevel(kDebugLevelSavegame, "Savegame", "Saving & restoring game debugging");


	memset(&_game, 0, sizeof(struct AgiGame));
	memset(&_debug, 0, sizeof(struct AgiDebug));
	memset(&g_mouse, 0, sizeof(struct Mouse));

	_game.clockEnabled = false;
	_game.state = STATE_INIT;

	_keyQueueStart = 0;
	_keyQueueEnd = 0;

	_keyControl = 0;
	_keyAlt = 0;

	_allowSynthetic = false;

	g_tickTimer = 0;

	_intobj = NULL;

	_stackSize = 0;
	_imageStack = NULL;
	_imageStackPointer = 0;

	_menu = NULL;

	_lastSentence[0] = 0;
	memset(&_stringdata, 0, sizeof(struct StringData));

	_objects = NULL;

	_oldMode = -1;
	
	_searchTreeRoot = 0;
	_firstSlot = 0;
}

void AgiEngine::initialize() {
	// TODO: Some sound emulation modes do not fit our current music
	//       drivers, and I'm not sure what they are. For now, they might
	//       as well be called "PC Speaker" and "Not PC Speaker".

	switch (MidiDriver::detectMusicDriver(MDT_PCSPK)) {
	case MD_PCSPK:
		_soundemu = SOUND_EMU_PC;
		break;
	default:
		_soundemu = SOUND_EMU_NONE;
		break;
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

	_game.sbuf = (uint8 *)calloc(_WIDTH, _HEIGHT);

	_gfx->initVideo();
	_sound->initSound();

	_timer->installTimerProc(agiTimerFunctionLow, 10 * 1000, NULL);

	_game.ver = -1;		/* Don't display the conf file warning */

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
	agiDeinit();
	_sound->deinitSound();
	delete _sound;
	_gfx->deinitVideo();
	delete _sprites;
	free(_game.sbuf);
	_gfx->deinitMachine();
	delete _rnd;
	delete _console;
}

int AgiEngine::init() {

	// Detect game
	if (!initGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}


	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	initialize();

	_gfx->gfxSetPalette();

	return 0;
}

int AgiEngine::go() {
	CursorMan.showMouse(true);

	report(" \nAGI engine %s is ready.\n", gScummVMVersion);
	if (_game.state < STATE_LOADED) {
		do {
			mainCycle();
		} while (_game.state < STATE_RUNNING);
		if (_game.ver < 0)
			_game.ver = 0;	/* Enable conf file warning */
	}

	runGame();

	return 0;
}

} // End of namespace Agi
