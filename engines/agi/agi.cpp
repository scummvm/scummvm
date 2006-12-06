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
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"

#include "graphics/cursorman.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

#include "agi/agi.h"
#include "agi/text.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/opcodes.h"
#include "agi/keyboard.h"
#include "agi/menu.h"
#include "agi/savegame.h"
#include "agi/sound.h"

namespace Agi {

static uint32 g_tick_timer;
struct Mouse g_mouse;

#define key_enqueue(k) do { _key_queue[_key_queue_end++] = (k); \
	_key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = _key_queue[_key_queue_start++]; \
	_key_queue_start %= KEY_QUEUE_SIZE; } while (0)

void AgiEngine::processEvents() {
	OSystem::Event event;
	int key = 0;

	while (g_system->pollEvent(event)) {
		switch (event.type) {
		case OSystem::EVENT_QUIT:
			_gfx->deinitVideo();
			_gfx->deinitMachine();
			g_system->quit();
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			key = BUTTON_LEFT;
			g_mouse.button = 1;
			key_enqueue(key);
			g_mouse.x = event.mouse.x;
			g_mouse.y = event.mouse.y;
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			key = BUTTON_RIGHT;
			g_mouse.button = 2;
			key_enqueue(key);
			g_mouse.x = event.mouse.x;
			g_mouse.y = event.mouse.y;
			break;
		case OSystem::EVENT_MOUSEMOVE:
			g_mouse.x = event.mouse.x;
			g_mouse.y = event.mouse.y;
			break;
		case OSystem::EVENT_LBUTTONUP:
		case OSystem::EVENT_RBUTTONUP:
			g_mouse.button = 0;
			break;
		case OSystem::EVENT_KEYDOWN:
			_key_control = 0;
			_key_alt = 0;

			if (event.kbd.flags == OSystem::KBD_CTRL && event.kbd.keycode == 'd') {
				_console->attach();
				break;
			}

			if (event.kbd.flags & OSystem::KBD_CTRL)
				_key_control = 1;
			
			if (event.kbd.flags & OSystem::KBD_ALT)
				_key_alt = 1;

			switch (key = event.kbd.keycode) {
			case 256 + 20:	// left arrow
			case 260:	// key pad 4
				key = KEY_LEFT;
				break;
			case 256 + 19:	// right arrow
			case 262:	// key pad 6
				key = KEY_RIGHT;
				break;
			case 256 + 17:	// up arrow
			case 264:	// key pad 8
				key = KEY_UP;
				break;
			case 256 + 18:	// down arrow
			case 258:	// key pad 2
				key = KEY_DOWN;
				break;
			case 256 + 24:	// page up
			case 265:	// key pad 9
				key = KEY_UP_RIGHT;
				break;
			case 256 + 25:	// page down
			case 259:	// key pad 3
				key = KEY_DOWN_RIGHT;
				break;
			case 256 + 22:	// home
			case 263:	// key pad 7
				key = KEY_UP_LEFT;
				break;
			case 256 + 23:	// end
			case 257:	// key pad 1
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
				if (_key_control)
					key = (key & ~0x20) - 0x40;
				else if (_key_alt)
					key = scancode_table[(key & ~0x20) - 0x41] << 8;
				else if (event.kbd.flags & OSystem::KBD_SHIFT)
					key = event.kbd.ascii;
				break;
			}
			if (key)
				key_enqueue(key);
			break;
		default:
			break;
		}
	}
}

int AgiEngine::agiIsKeypressLow() {
	processEvents();
	return _key_queue_start != _key_queue_end;
}

void AgiEngine::agiTimerLow() {
	static uint32 m = 0;
	uint32 dm;

	if (g_tick_timer < m)
		m = 0;

	while ((dm = g_tick_timer - m) < 5) {
		processEvents();
		if (_console->isAttached())
			_console->onFrame();
		g_system->delayMillis(10);
		g_system->updateScreen();
	}
	m = g_tick_timer;
}

int AgiEngine::agiGetKeypressLow() {
	int k;

	while (_key_queue_start == _key_queue_end)	/* block */
		agiTimerLow();
	key_dequeue(k);

	return k;
}

void AgiEngine::agiTimerFunctionLow(void *refCon) {
	g_tick_timer++;
}

void AgiEngine::clear_image_stack(void) {
	image_stack_pointer = 0;
}

void AgiEngine::release_image_stack(void) {
	if (image_stack)
		free(image_stack);
	image_stack = NULL;
	stack_size = image_stack_pointer = 0;
}

void AgiEngine::record_image_stack_call(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {
	struct image_stack_element *pnew;

	if (image_stack_pointer == stack_size) {
		if (stack_size == 0) {	/* first call */
			image_stack = (struct image_stack_element *)
					malloc(INITIAL_IMAGE_STACK_SIZE * sizeof(struct image_stack_element));
			stack_size = INITIAL_IMAGE_STACK_SIZE;
		} else {	/* has to grow */
			struct image_stack_element *new_stack;
			new_stack = (struct image_stack_element *)
					malloc(2 * stack_size * sizeof(struct image_stack_element));
			memcpy(new_stack, image_stack, stack_size * sizeof(struct image_stack_element));
			free(image_stack);
			image_stack = new_stack;
			stack_size *= 2;
		}
	}

	pnew = &image_stack[image_stack_pointer];
	image_stack_pointer++;

	pnew->type = type;
	pnew->parm1 = p1;
	pnew->parm2 = p2;
	pnew->parm3 = p3;
	pnew->parm4 = p4;
	pnew->parm5 = p5;
	pnew->parm6 = p6;
	pnew->parm7 = p7;
}

void AgiEngine::replay_image_stack_call(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {
	switch (type) {
	case ADD_PIC:
		debugC(8, kDebugLevelMain, "--- decoding picture %d ---", p1);
		agiLoadResource(rPICTURE, p1);
		_picture->decode_picture(p1, p2);
		break;
	case ADD_VIEW:
		agiLoadResource(rVIEW, p1);
		_sprites->add_to_pic(p1, p2, p3, p4, p5, p6, p7);
		break;
	}
}

void AgiEngine::initPriTable() {
	int i, p, y = 0;

	for (p = 1; p < 15; p++) {
		for (i = 0; i < 12; i++) {
			game.pri_table[y++] = p < 4 ? 4 : p;
		}
	}
}

int AgiEngine::agiInit() {
	int ec, i;

	debug(2, "initializing");
	debug(2, "game.ver = 0x%x", game.ver);

	/* reset all flags to false and all variables to 0 */
	for (i = 0; i < MAX_FLAGS; i++)
		game.flags[i] = 0;
	for (i = 0; i < MAX_VARS; i++)
		game.vars[i] = 0;

	/* clear all resources and events */
	for (i = 0; i < MAX_DIRS; i++) {
		memset(&game.views[i], 0, sizeof(struct agi_view));
		memset(&game.pictures[i], 0, sizeof(struct agi_picture));
		memset(&game.logics[i], 0, sizeof(struct agi_logic));
		memset(&game.sounds[i], 0, sizeof(struct agi_sound));
		memset(&game.dir_view[i], 0, sizeof(struct agi_dir));
		memset(&game.dir_pic[i], 0, sizeof(struct agi_dir));
		memset(&game.dir_logic[i], 0, sizeof(struct agi_dir));
		memset(&game.dir_sound[i], 0, sizeof(struct agi_dir));
	}

	/* clear view table */
	for (i = 0; i < MAX_VIEWTABLE; i++)
		memset(&game.view_table[i], 0, sizeof(struct vt_entry));

	init_words();

	if (!menu)
		menu = new Menu(this, _gfx, _picture);

	initPriTable();

	/* clear string buffer */
	for (i = 0; i < MAX_STRINGS; i++)
		game.strings[i][0] = 0;

	/* setup emulation */

	switch (loader->getIntVersion() >> 12) {
	case 2:
		report("Emulating Sierra AGI v%x.%03x\n",
				(int)(loader->version() >> 12) & 0xF,
				(int)(loader->version()) & 0xFFF);
		break;
	case 3:
		report("Emulating Sierra AGI v%x.002.%03x\n",
				(int)(loader->version() >> 12) & 0xF,
				(int)(loader->version()) & 0xFFF);
		break;
	}

	game.game_flags |= opt.amiga ? ID_AMIGA : 0;
	game.game_flags |= opt.agds ? ID_AGDS : 0;

	if (game.game_flags & ID_AMIGA)
		report("Amiga padded game detected.\n");

	if (game.game_flags & ID_AGDS)
		report("AGDS mode enabled.\n");

	ec = loader->init();	/* load vol files, etc */

	if (ec == err_OK)
		ec = loader->load_objects(OBJECTS);

	/* note: demogs has no words.tok */
	if (ec == err_OK)
		ec = loader->load_words(WORDS);

	/* FIXME: load IIgs instruments and samples */
	/* load_instruments("kq.sys16"); */

	/* Load logic 0 into memory */
	if (ec == err_OK)
		ec = loader->load_resource(rLOGIC, 0);

	return ec;
}

/*
 * Public functions
 */

void AgiEngine::agiUnloadResources() {
	int i;

	/* Make sure logic 0 is always loaded */
	for (i = 1; i < MAX_DIRS; i++) {
		loader->unload_resource(rLOGIC, i);
	}
	for (i = 0; i < MAX_DIRS; i++) {
		loader->unload_resource(rVIEW, i);
		loader->unload_resource(rPICTURE, i);
		loader->unload_resource(rSOUND, i);
	}
}

int AgiEngine::agiDeinit() {
	int ec;

	clean_input();		/* remove all words from memory */
	agiUnloadResources();	/* unload resources in memory */
	loader->unload_resource(rLOGIC, 0);
	ec = loader->deinit();
	unload_objects();
	unload_words();

	clear_image_stack();

	return ec;
}

int AgiEngine::agiDetectGame() {
	int ec = err_OK;

	loader = new AgiLoader_v2(this);
	ec = loader->detect_game();

	if (ec != err_OK) {
		loader = new AgiLoader_v3(this);
		ec = loader->detect_game();
	}

	return ec;
}

int AgiEngine::agiVersion() {
	return loader->version();
}

int AgiEngine::agiGetRelease() {
	return loader->getIntVersion();
}

void AgiEngine::agiSetRelease(int n) {
	loader->setIntVersion(n);
}

int AgiEngine::agiLoadResource(int r, int n) {
	int i;

	i = loader->load_resource(r, n);
#ifdef PATCH_LOGIC
	if (r == rLOGIC)
		patch_logic(n);
#endif

	return i;
}

int AgiEngine::agiUnloadResource(int r, int n) {
	return loader->unload_resource(r, n);
}

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

static const GameSettings agi_settings[] = {
	{"agi", "AGI game", GID_AGI, MDT_ADLIB, "OBJECT"},
	{NULL, NULL, 0, 0, NULL}
};

Common::RandomSource * rnd;

AgiEngine::AgiEngine(OSystem *syst) : Engine(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_savePath = _saveFileMan->getSavePath();	// FIXME: Get rid of this

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = agi_settings; g->gameid; ++g)
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


	memset(&game, 0, sizeof(struct agi_game));
	memset(&_debug, 0, sizeof(struct agi_debug));
	memset(&g_mouse, 0, sizeof(struct Mouse));

	game.clock_enabled = false;
	game.state = STATE_INIT;

	_key_queue_start = 0;
	_key_queue_end = 0;

	_key_control = 0;
	_key_alt = 0;

	g_tick_timer = 0;

	intobj = NULL;

	stack_size = 0;
	image_stack = NULL;
	image_stack_pointer = 0;

	menu = NULL;

	last_sentence[0] = 0;
	memset(&stringdata, 0, sizeof(struct string_data));

	objects = NULL;
}

void AgiEngine::initialize() {
	memset(&opt, 0, sizeof(struct agi_options));
	opt.gamerun = GAMERUN_RUNGAME;
	opt.hires = true;

	// TODO: Some sound emulation modes do not fit our current music
	//       drivers, and I'm not sure what they are. For now, they might
	//       as well be called "PC Speaker" and "Not PC Speaker".

	switch (MidiDriver::detectMusicDriver(MDT_PCSPK)) {
	case MD_PCSPK:
		opt.soundemu = SOUND_EMU_PC;
		break;
	default:
		opt.soundemu = SOUND_EMU_NONE;
		break;
	}

	if (ConfMan.hasKey("render_mode"))
		opt.renderMode = Common::parseRenderMode(ConfMan.get("render_mode").c_str());

	_console = new Console(this);
	_gfx = new GfxMgr(this);
	_sound = new SoundMgr(this, _mixer);
	_picture = new PictureMgr(this, _gfx);
	_sprites = new SpritesMgr(this, _gfx);
	_saveGameMgr = new SaveGameMgr(this, _sprites, _gfx, _sound, _picture);

	_gfx->initMachine();

	game.game_flags = 0;

	game.color_fg = 15;
	game.color_bg = 0;

	*game.name = NULL;

	game.sbuf = (uint8 *)calloc(_WIDTH, _HEIGHT);
	game.hires = (uint8 *)calloc(_WIDTH * 2, _HEIGHT);

	_gfx->initVideo();
	_sound->init_sound();

	_timer->installTimerProc(agiTimerFunctionLow, 10 * 1000, NULL);

	game.ver = -1;		/* Don't display the conf file warning */

	debugC(2, kDebugLevelMain, "Detect game");
	if (agiDetectGame() == err_OK) {
		game.state = STATE_LOADED;
		debugC(2, kDebugLevelMain, "game loaded");
	} else {
		report("Could not open AGI game");
	}

	debugC(2, kDebugLevelMain, "Init sound");
}

AgiEngine::~AgiEngine() {
	agiDeinit();
	_sound->deinit_sound();
	delete _sound;
	_gfx->deinitVideo();
	delete _sprites;
	free(game.hires);
	free(game.sbuf);
	_gfx->deinitMachine();
	delete rnd;
	delete _console;
}

int AgiEngine::init() {
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

	report(" \nAGI engine " VERSION " is ready.\n");
	if (game.state < STATE_LOADED) {
		do {
			main_cycle();
		} while (game.state < STATE_RUNNING);
		if (game.ver < 0)
			game.ver = 0;	/* Enable conf file warning */
	}

	run_game();

	return 0;
}

}                             // End of namespace Agi

GameList Engine_AGI_gameIDList() {
	GameList games;
	const Agi::GameSettings *g = Agi::agi_settings;

	while (g->gameid) {
		games.push_back(*g);
		g++;
	}

	return games;
}

GameDescriptor Engine_AGI_findGameID(const char *gameid) {
	const Agi::GameSettings *g = Agi::agi_settings;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			break;
		g++;
	}
	return *g;
}

DetectedGameList Engine_AGI_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const Agi::GameSettings * g;

	for (g = Agi::agi_settings; g->gameid; ++g) {
		// Iterate over all files in the given directory
		for (FSList::const_iterator file = fslist.begin();
		    file != fslist.end(); ++file) {
			const char *fileName = file->name().c_str();

			if (0 == scumm_stricmp(g->detectname, fileName)) {
				// Match found, add to list of candidates, then abort inner loop.
				detectedGames.push_back(*g);
				break;
			}
		}
	}
	return detectedGames;
}

PluginError Engine_AGI_create(OSystem *syst, Engine **engine) {
	assert(syst);
	assert(engine);

	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		warning("AgiEngine: invalid game path '%s'", dir.path().c_str());
		return kInvalidPathError;
	}

	// Invoke the detector
	Common::String gameid = ConfMan.get("gameid");
	DetectedGameList detectedGames = Engine_AGI_detectGames(fslist);

	for (uint i = 0; i < detectedGames.size(); i++) {
		if (detectedGames[i].gameid == gameid) {
			*engine = new Agi::AgiEngine(syst);
			return kNoError;
		}
	}

	warning("AgiEngine: Unable to locate game data at path '%s'", dir.path().c_str());
	return kNoGameDataFoundError;
}

REGISTER_PLUGIN(AGI, "AGI Engine", "TODO (C) TODO");
