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
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"

#include "backends/fs/fs.h"

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

namespace Agi {

void gfx_set_palette();

extern int optind;

struct agi_options opt;
struct game_id_list game_info;
struct agi_game game;

static struct agi_loader *loader;	/* loader */

extern struct agi_loader agi_v2;
extern struct agi_loader agi_v3;

static volatile uint32 tick_timer = 0;

#define TICK_SECONDS 20

static int key_control = 0;
static int key_alt = 0;

#define KEY_QUEUE_SIZE 16

static int key_queue[KEY_QUEUE_SIZE];
static int key_queue_start = 0;
static int key_queue_end = 0;

#define key_enqueue(k) do { key_queue[key_queue_end++] = (k); \
	key_queue_end %= KEY_QUEUE_SIZE; } while (0)
#define key_dequeue(k) do { (k) = key_queue[key_queue_start++]; \
	key_queue_start %= KEY_QUEUE_SIZE; } while (0)

static void process_events() {
	OSystem::Event event;
	int key = 0;

	while (g_system->pollEvent(event)) {
		switch (event.type) {
		case OSystem::EVENT_QUIT:
			deinit_video();
			deinit_machine();
			g_system->quit();
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			key = BUTTON_LEFT;
			mouse.button = 1;
			key_enqueue(key);
			mouse.x = event.mouse.x;
			mouse.y = event.mouse.y;
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			key = BUTTON_RIGHT;
			mouse.button = 2;
			key_enqueue(key);
			mouse.x = event.mouse.x;
			mouse.y = event.mouse.y;
			break;
		case OSystem::EVENT_MOUSEMOVE:
			mouse.x = event.mouse.x;
			mouse.y = event.mouse.y;
			break;
		case OSystem::EVENT_LBUTTONUP:
		case OSystem::EVENT_RBUTTONUP:
			mouse.button = 0;
			break;
		case OSystem::EVENT_KEYDOWN:
			key_control = 0;
			key_alt = 0;
			if (event.kbd.flags == OSystem::KBD_CTRL) {
				key_control = 1;
				key = 0;
				break;
			} else if (event.kbd.flags == OSystem::KBD_ALT) {
				key_alt = 1;
				key = 0;
				break;
			} else if (event.kbd.flags == OSystem::KBD_SHIFT) {
				key = 0;
				break;
			}

			switch (key = event.kbd.keycode) {
			case 256 + 20:	// left arrow
				key = KEY_LEFT;
				break;
			case 256 + 19:	// right arrow
				key = KEY_RIGHT;
				break;
			case 256 + 17:	// up arrow
				key = KEY_UP;
				break;
			case 256 + 18:	// down arrow
				key = KEY_DOWN;
				break;
			case 256 + 24:	// page up
				key = KEY_UP_RIGHT;
				break;
			case 256 + 25:	// page down
				key = KEY_DOWN_RIGHT;
				break;
			case 256 + 22:	// home
				key = KEY_UP_LEFT;
				break;
			case 256 + 23:	// end
				key = KEY_DOWN_LEFT;
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
				if (key < 256 && !isalpha(key))
					break;
				if (key_control)
					key = (key & ~0x20) - 0x40;
				else if (key_alt)
					key = scancode_table[(key & ~0x20) - 0x41] << 8;
				break;
			}
			if (key)
				key_enqueue(key);
			break;
			break;
		default:
			break;
		}
	}
}

int agi_is_keypress_low() {
	process_events();
	return key_queue_start != key_queue_end;
}

void agi_timer_low() {
	static uint32 m = 0;
	uint32 dm;

	if (tick_timer < m)
		m = 0;

	while ((dm = tick_timer - m) < 5) {
		process_events();
		g_system->delayMillis(10);
		g_system->updateScreen();
	}
	m = tick_timer;
}

int agi_get_keypress_low() {
	int k;

	while (key_queue_start == key_queue_end)	/* block */
		agi_timer_low();
	key_dequeue(k);

	return k;
}

static uint32 agi_timer_function_low(uint32 i) {
	tick_timer++;
	return i;
}

static void init_pri_table() {
	int i, p, y = 0;

	for (p = 1; p < 15; p++) {
		for (i = 0; i < 12; i++) {
			game.pri_table[y++] = p < 4 ? 4 : p;
		}
	}
}

int agi_init() {
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
	}

	/* clear view table */
	for (i = 0; i < MAX_VIEWTABLE; i++)
		memset(&game.view_table[i], 0, sizeof(struct vt_entry));

	init_words();

	menu_init();
	init_pri_table();

	/* clear string buffer */
	for (i = 0; i < MAX_STRINGS; i++)
		game.strings[i][0] = 0;

	/* setup emulation */

	switch (loader->int_version >> 12) {
	case 2:
		report("Emulating Sierra AGI v%x.%03x\n",
				(int)(loader->int_version >> 12) & 0xF,
				(int)(loader->int_version) & 0xFFF);
		break;
	case 3:
		report("Emulating Sierra AGI v%x.002.%03x\n",
				(int)(loader->int_version >> 12) & 0xF,
				(int)(loader->int_version) & 0xFFF);
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

void agi_unload_resources() {
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

int agi_deinit() {
	int ec;

	clean_input();		/* remove all words from memory */
	agi_unload_resources();	/* unload resources in memory */
	loader->unload_resource(rLOGIC, 0);
	ec = loader->deinit();
	unload_objects();
	unload_words();

	clear_image_stack();

	return ec;
}

int agi_detect_game() {
	int ec = err_OK;

	loader = &agi_v2;
	ec = loader->detect_game();

	if (ec != err_OK) {
		loader = &agi_v3;
		ec = loader->detect_game();
	}

	return ec;
}

int agi_version() {
	return loader->version;
}

int agi_get_release() {
	return loader->int_version;
}

void agi_set_release(int n) {
	loader->int_version = n;
}

int agi_load_resource(int r, int n) {
	int i;

	i = loader->load_resource(r, n);
#ifdef PATCH_LOGIC
	if (r == rLOGIC)
		patch_logic(n);
#endif

	return i;
}

int agi_unload_resource(int r, int n) {
	return loader->unload_resource(r, n);
}

const char *_savePath;
extern AGIMusic *g_agi_music;

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

static const GameSettings agi_settings[] = {
	{"agi", "AGI game", GID_AGI, MDT_ADLIB, "VIEWDIR"},
	{NULL, NULL, 0, 0, NULL}
};

Common::RandomSource * rnd;

AgiEngine::AgiEngine(OSystem * syst) : Engine(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_savePath = _saveFileMan->getSavePath();

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = agi_settings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

	rnd = new Common::RandomSource();

	Common::addSpecialDebugLevel(kDebugLevelMain, "Main", "Generic debug level");
	Common::addSpecialDebugLevel(kDebugLevelResources, "Resources", "Resources debugging");
	Common::addSpecialDebugLevel(kDebugLevelSprites, "Sprites", "Sprites debugging");
	Common::addSpecialDebugLevel(kDebugLevelInventory, "Inventory", "Inventory debugging");
	Common::addSpecialDebugLevel(kDebugLevelInput, "Input", "Input events debugging");
	Common::addSpecialDebugLevel(kDebugLevelMenu, "Menu", "Menu debugging");
	Common::addSpecialDebugLevel(kDebugLevelScripts, "Scrpits", "Scripts debugging");
	Common::addSpecialDebugLevel(kDebugLevelSound, "Sound", "Sound debugging");
	Common::addSpecialDebugLevel(kDebugLevelText, "Text", "Text output debugging");

	game.clock_enabled = false;
	game.state = STATE_INIT;
}

void AgiEngine::initialize() {
	memset(&opt, 0, sizeof(struct agi_options));
	opt.gamerun = GAMERUN_RUNGAME;
#ifdef USE_HIRES
	opt.hires = true;
#endif
	opt.soundemu = SOUND_EMU_NONE;

	init_machine();

	game.color_fg = 15;
	game.color_bg = 0;

	*game.name = 0;

	game.sbuf = (uint8 *) calloc(_WIDTH, _HEIGHT);
#ifdef USE_HIRES
	game.hires = (uint8 *) calloc(_WIDTH * 2, _HEIGHT);
#endif

	init_sprites();
	init_video();

	tick_timer = 0;
	Common::g_timer->installTimerProc((Common::Timer::TimerProc) agi_timer_function_low, 10 * 1000, NULL);

	console_init();

	game.ver = -1;		/* Don't display the conf file warning */

	debugC(2, kDebugLevelMain, "Detect game");
	if (agi_detect_game() == err_OK) {
		game.state = STATE_LOADED;
		debugC(2, kDebugLevelMain, "game loaded");
	} else {
		report("Could not open AGI game");
	}

	debugC(2, kDebugLevelMain, "Init sound");
	init_sound();
	g_agi_music = new AGIMusic(_mixer);
}

AgiEngine::~AgiEngine() {
	agi_deinit();
	delete g_agi_music;
	deinit_sound();
	deinit_video();
	deinit_sprites();
#ifdef USE_HIRES
	free(game.hires);
#endif
	free(game.sbuf);
	deinit_machine();
	delete rnd;
}

void AgiEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int AgiEngine::init() {
	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	initialize();

	gfx_set_palette();

	return 0;
}

int AgiEngine::go() {
	_system->showMouse(true);

	report(" \nAGI engine " VERSION " is ready.\n");
	if (game.state < STATE_LOADED) {
		console_prompt();
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
			const char *gameName = file->displayName().c_str();

			if (0 == scumm_stricmp(g->detectname, gameName)) {
				// Match found, add to list of candidates, then abort inner loop.
				detectedGames.push_back(*g);
				break;
			}
		}
	}
	return detectedGames;
}

PluginError Engine_AGI_create(OSystem *syst, Engine **engine) {
	assert(engine);
	*engine = new Agi::AgiEngine(syst);
	return kNoError;
}

REGISTER_PLUGIN(AGI, "AGI Engine");
