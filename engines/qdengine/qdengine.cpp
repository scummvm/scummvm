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

#include "backends/imgui/imgui.h"

#include "common/system.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "graphics/screen.h"

#include "qdengine/qdengine.h"
#include "qdengine/console.h"
#include "qdengine/resource.h"

#include "qdengine/debugger/debugtools.h"

#include "qdengine/parser/qdscr_parser.h"

#include "qdengine/qdcore/qd_file_manager.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"

#include "qdengine/qdcore/util/plaympp_api.h"
#include "qdengine/qdcore/util/ResourceDispatcher.h"
#include "qdengine/qdcore/util/splash_screen.h"

#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/rle_compress.h"

#include "qdengine/system/input/keyboard_input.h"
#include "qdengine/system/input/mouse_input.h"
#include "qdengine/system/input/input_wndproc.h"
#include "qdengine/system/sound/snd_dispatcher.h"

namespace QDEngine {

QDEngineEngine *g_engine;

static void generateTagMap(int date, bool verbose = true);
void searchTagMap(int id, int targetVal);
static int detectVersion(Common::String gameID);

static void restore_graphics();

static void qd_show_load_progress(int percents_loaded, void *p);


QDEngineEngine::QDEngineEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("QDEngine") {
	g_engine = this;
	_pixelformat = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);

	_screenW = 640;
	_screenH = 480;

	ConfMan.registerDefault("game_speed", 1);
	ConfMan.registerDefault("enable_sound", true);
	ConfMan.registerDefault("enable_music", true);
	ConfMan.registerDefault("logic_period", 25);
	ConfMan.registerDefault("logic_synchro_by_clock", true);
	ConfMan.registerDefault("music_volume", 255);
	ConfMan.registerDefault("show_fps", false);
	ConfMan.registerDefault("sound_volume", 255);
	ConfMan.registerDefault("splash_enabled", true);
	ConfMan.registerDefault("splash_time", 3000);

	memset(_tagMap, 0, sizeof(_tagMap));
}

QDEngineEngine::~QDEngineEngine() {
	delete _screen;
	cleanup_XML_Parser();
}

uint32 QDEngineEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::Language QDEngineEngine::getLanguage() const {
	return _gameDescription->language;
}

Common::String QDEngineEngine::getGameId() const {
	return _gameDescription->gameId;
}

const char *QDEngineEngine::getExeName() const {
	return _gameDescription->filesDescriptions[1].fileName;
}

void scan_qda();

Common::Error QDEngineEngine::run() {
	initGraphics(640, 480);
	_screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1) {
		warning("STUB: savegame loading is not yet supported");
		(void)loadGameState(saveSlot);
	}

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.qml");
	Common::ArchiveMemberPtr p = files.front();
	Common::String firstFileName;
	Common::String script_name;


	if (getFeatures() & GF_BROKEN_VIDEOS)
		warning("YEEEEES");

	warning("%s, %x", _gameDescription->filesDescriptions[0].md5, getFeatures());

	if (p) {
		firstFileName = p->getFileName();
		script_name = firstFileName.c_str();
	}

	if (script_name.empty()) {
		return Common::kNoGameDataFoundError;
	}

	_grD = new grDispatcher();

	SplashScreen sp;
	if (ConfMan.getBool("splash_enabled")) {
		sp.create(IDB_SPLASH);
		sp.set_mask(IDB_SPLASH_MASK);
		sp.show();
	}

	//searchTagMap(QDSCR_GAME_TITLE, 207);

	_gameVersion = detectVersion(g_engine->getGameId());

	generateTagMap(_gameVersion);

	grDispatcher::set_default_font(qdGameDispatcher::create_font(0));

	_gameD = new qdGameDispatcher;
	_gameD->load_script(script_name.c_str());

	if (ConfMan.getBool("dump_scripts")) {
		Common::String fname = "qd_game";

		if (debugChannelSet(-1, kDebugLog)) {
			fname += "_" + g_engine->getGameId();

			if (g_engine->getLanguage() != Common::RU_RUS)
				fname += "-" + Common::String(Common::getLanguageCode(g_engine->getLanguage()));
		}

		fname += ".xml";

		_gameD->save_script(fname.c_str());
		debug("Dumped %s%s", fname.c_str(), debugChannelSet(-1, kDebugLog) ? " in human-readable form" : "");
	}

	_gameD->set_scene_loading_progress_callback(qd_show_load_progress);

	if (ConfMan.getBool("splash_enabled")) {
		sp.wait(ConfMan.getInt("splash_time"));
		sp.destroy();
	}

	init_graphics();

	sndDispatcher *sndD = new sndDispatcher;

	g_engine->syncSoundSettings();

	winVideo::init();

	_gameD->load_resources();

	if (ConfMan.hasKey("boot_param")) {
		const char *scene_name = ""; // FIXME. Implement actual scene selection
		if (!_gameD->select_scene(scene_name))
			error("Cannot find the startup scene");
	} else {
		bool music_enabled = mpegPlayer::instance().is_enabled();
		mpegPlayer::instance().disable();

		_gameD->toggle_main_menu(true);
		if (!_gameD->start_intro_videos()) {
			if (music_enabled)
				mpegPlayer::instance().enable(true);
		} else {
			if (music_enabled)
				mpegPlayer::instance().enable(false);
		}
	}

	_gameD->update_time();
	_gameD->quant();

	ResourceDispatcher resD;
	resD.setTimer(ConfMan.getBool("logic_synchro_by_clock"), ConfMan.getInt("logic_period"), 300);
	resD.attach(new MemberFunctionCallResourceUser<qdGameDispatcher>(*_gameD, &qdGameDispatcher::quant, ConfMan.getInt("logic_period")));
	sndD->set_frequency_coeff(ConfMan.getFloat("game_speed"));
	resD.set_speed(ConfMan.getFloat("game_speed"));
	resD.start();

	bool exit_flag = false;
	bool was_inactive = false;

#ifdef USE_IMGUI
	ImGuiCallbacks callbacks;
	bool drawImGui = debugChannelSet(-1, kDebugImGui);
	callbacks.init = QDEngine::onImGuiInit;
	callbacks.render = drawImGui ? QDEngine::onImGuiRender : nullptr;
	callbacks.cleanup = QDEngine::onImGuiCleanup;
	_system->setImGuiCallbacks(callbacks);
#endif

	// Activate the window
	grDispatcher::activate(true);

	//scan_qda();

	Common::Event event;

	while (!exit_flag && !_gameD->need_exit()) {
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				if (!grDispatcher::instance()->is_in_reinit_mode())
					exit_flag = true;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.ascii == 'f')
					ConfMan.setBool("show_fps", !ConfMan.getBool("show_fps"));
#ifdef __QD_DEBUG_ENABLE__
				else if (event.kbd.keycode == Common::KEYCODE_PAGEDOWN) {
					float speed = ConfMan.getFloat("game_speed") * 0.9f;
					if (speed < 0.1f) speed = 0.1f;
					ConfMan.setFloat("game_speed", speed);
					sndD->set_frequency_coeff(speed);
					resD.set_speed(ConfMan.getFloat("game_speed"));
					ConfMan.flushToDisk();
				} else if (event.kbd.keycode == Common::KEYCODE_PAGEUP) {
					float speed = ConfMan.getFloat("game_speed") * 1.1f;
					if (speed > 10.0f) speed = 10.0f;
					ConfMan.setFloat("game_speed", speed);
					sndD->set_frequency_coeff(speed);
					resD.set_speed(ConfMan.getFloat("game_speed"));
					ConfMan.flushToDisk();
				} else if (event.kbd.keycode == Common::KEYCODE_HOME) {
					ConfMan.setFloat("game_speed", 1.0f);
					sndD->set_frequency_coeff(1.0f);
					resD.set_speed(ConfMan.getFloat("game_speed"));
					ConfMan.flushToDisk();
				} else if (event.kbd.ascii == 'g')
					qdGameConfig::get_config().toggle_show_grid();
#endif
				break;
			default:
				break;
			}

			input::keyboard_wndproc(event, keyboardDispatcher::instance());
			input::mouse_wndproc(event, mouseDispatcher::instance());
		}

		// For performance reasons, disable the renderer callback if the ImGui debug flag isn't set
#ifdef USE_IMGUI
		if (debugChannelSet(-1, kDebugImGui) != drawImGui) {
			drawImGui = !drawImGui;
			callbacks.render = drawImGui ? QDEngine::onImGuiRender : nullptr;
			_system->setImGuiCallbacks(callbacks);
		}
#endif

		if (grDispatcher::instance()->is_mouse_hidden())
			grDispatcher::instance()->set_null_mouse_cursor();
		else
			grDispatcher::instance()->set_default_mouse_cursor();

		if (grDispatcher::is_active()) {
			if (was_inactive) {
				was_inactive = false;
				// При активации ждем, чтобы звукововая система успела настроиться
				// на наше приложение (предположение)
				g_system->delayMillis(500);
			}
			resD.quant();
			_gameD->redraw();
		} else {
			was_inactive = true;
			g_system->delayMillis(100);
			resD.skip_time();
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	delete _gameD;

	grDispatcher::instance()->finit();

	qdFileManager::instance().Finit();

	delete sndD;
	delete _grD;

	winVideo::done();

	RLEBuffer::releaseBuffers();

	return Common::kNoError;
}

bool canSaveGameStateCurrently(Common::U32String *msg)  {
	return qdGameDispatcher::get_dispatcher()->get_active_scene() != nullptr;
}

Common::Error QDEngineEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (qdGameDispatcher::get_dispatcher()->save_save(stream))
		return Common::kNoError;

	return Common::kWritingFailed;
}

Common::Error QDEngineEngine::loadGameStream(Common::SeekableReadStream *stream) {
	if (qdGameDispatcher::get_dispatcher()->load_save(stream))
		return Common::kNoError;

	return Common::kReadingFailed;
}

void QDEngineEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	sndDispatcher::get_dispatcher()->syncSoundSettings();
	mpegPlayer::instance().syncMusicSettings();
}

static void generateTagMap(int date, bool verbose) {
	int n = 0;

	memset(g_engine->_tagMap, 0, QDSCR_MAX_KEYWORD_ID * sizeof(int));

	for (int i = 0; i < QDSCR_MAX_KEYWORD_ID; i++)
		if (idTagVersionAll[i * 2] <= date)
			g_engine->_tagMap[n++] = idTagVersionAll[i * 2 + 1];

	if (verbose)
		warning("Generated %d ids for version %d", n, date);
}

void searchTagMap(int id, int targetVal) {
	Common::HashMap<int, bool> dates;

	for (int i = 0; i < QDSCR_MAX_KEYWORD_ID; i++)
		dates[idTagVersionAll[i * 2]] = true;

	Common::Array<int> sdates;
	for (auto it : dates)
		sdates.push_back(it._key);

	Common::sort(sdates.begin(), sdates.end());

	for (auto d : sdates) {
		generateTagMap(d, false);

		int matchedId = -1;
		int len = QDSCR_MAX_KEYWORD_ID;

		for (int i = 0; i < QDSCR_MAX_KEYWORD_ID; i++) {
			if (!g_engine->_tagMap[i]) {
				len = i;
				break;
			}

			if (g_engine->_tagMap[i] == id)
				matchedId = i + 1;
		}

		warning("ver: %d  val: %d of %d", d, matchedId, len);

		if (g_engine->_tagMap[targetVal - 1] == id)
			warning("searchTagMap(): Matched version %d", d);
	}

	warning("searchTagMap(): No match");
}

static int detectVersion(Common::String gameID) {
	if (gameID == "karliknos") {
		return 20030919;		// QDSCR_GAME_TITLE = 182, 06b1cf45d (repo-vss)
	} else if (gameID == "nupogodi3" && g_engine->getLanguage() == Common::RU_RUS) {
		return 20031014;		// QDSCR_TEXT_DB = 184, d864cc279 (repo-vss)
	} else if (gameID == "nupogodi3" && g_engine->getLanguage() == Common::CS_CZE) {
		return 20031014;		// QDSCR_TEXT_DB = 184, d864cc279 (repo-vss)
	} else if (gameID == "nupogodi3" && g_engine->getLanguage() == Common::LT_LTU) {
		return 20031206;		// QDSCR_TEXT_DB = 185
	} else if (gameID == "pilots3") {
		return 20040519;		// QDSCR_GAME_TITLE = 203
	} else if (gameID == "rybalka") {
		return 20040601;		// QDSCR_GAME_TITLE = 206
	} else if (gameID == "pilots3d") {
		return 20040601;		// QDSCR_GAME_TITLE = 206
	} else if (gameID == "pilots3d-2") {
		return 20041201;		// QDSCR_GAME_TITLE = 207
	} else if (gameID == "mng") {
		return 20050101;		// QDSCR_GLOBAL_DEPEND = 214
	} else if (gameID == "maski") {
		return 20060129;		// QDSCR_GAME_TITLE = 214, 54bcf92 (repo-git)
	} else if (gameID == "3mice1" && (g_engine->getFeatures() & ADGF_DEMO)) {
		return 20060129;		// QDSCR_SCREEN_TRANSFORM = 232
	} else if (gameID == "3mice1") {
		return 20060715;		// QDSCR_SCREEN_TRANSFORM = 232
	} else if (gameID == "shveik") {
		return 20070503;		// QDSCR_GAME_TITLE = 231, QDSCR_RESOURCE_COMPRESSION = 243
	} else if (gameID == "klepa") {
		return 20070503;		// QDSCR_GAME_TITLE = 231, QDSCR_RESOURCE_COMPRESSION = 243
	} else if (gameID == "3mice2") {
		return 20070503;		// QDSCR_RESOURCE_COMPRESSION = 243
	} else if (gameID == "dogncat") {
		return 20070503;		// QDSCR_RESOURCE_COMPRESSION = 243
	} else if (gameID == "dogncat2") {
		return 20070503;		// QDSCR_RESOURCE_COMPRESSION = 243
	} else {
		warning("Unprocessed tagMap, switching to shveik");

		return 20070503;
	}
}

void QDEngineEngine::init_graphics() {
	grDispatcher::set_restore_handler(restore_graphics);
	grDispatcher::instance()->finit();

	grDispatcher::set_instance(_grD);

	grDispatcher::instance()->init(g_engine->_screenW, g_engine->_screenH, GR_RGB565);

	grDispatcher::instance()->setClip();
	grDispatcher::instance()->setClipMode(1);

	grDispatcher::instance()->fill(0);

	g_system->updateScreen();

	grDispatcher::instance()->flush();
}

void qd_show_load_progress(int percents_loaded, void *p) {
	const int rect_sx = 200;
	const int rect_sy = 10;

	int sx = rect_sx * percents_loaded / 100;
	if (sx < 0) sx = 0;
	if (sx > rect_sx) sx = rect_sx;

	int x = 10;
	int y = grDispatcher::instance()->get_SizeY() - rect_sy - 10;

	grDispatcher::instance()->rectangle(x, y, rect_sx, rect_sy, 0xFFFFFF, 0, GR_OUTLINED);
	grDispatcher::instance()->rectangle(x, y, sx, rect_sy, 0xFFFFFF, 0xFFFFFF, GR_FILLED);
	grDispatcher::instance()->flush(x, y, rect_sx, rect_sy);
}

void restore_graphics() {
	if (sndDispatcher * dp = sndDispatcher::get_dispatcher())
		dp->set_volume(dp->volume());

	if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher())
		dp->set_flag(qdGameDispatcher::FULLSCREEN_REDRAW_FLAG);
}

void scan_qda() {
	debug("======== QDA Scan start ========");

	for (int i = 0; i < 3; i++) {
		Common::Archive *archive = qdFileManager::instance().get_package(i);
		Common::ArchiveMemberList members;

		if (archive) {
			archive->listMembers(members);

			for (auto &it : members) {
				if (it->getFileName().hasSuffixIgnoreCase(".qda")) {
					Common::SeekableReadStream *fh;

					if (!qdFileManager::instance().open_file(&fh, it->getPathInArchive()))
						continue;

					int32 version = fh->readSint32LE();

					if (version < 105)
						continue;

					/* int sx = */fh->readSint32LE();
					/* int sy = */fh->readSint32LE();
					/* int length = */fh->readFloatLE();
					/* int32 fl = */fh->readSint32LE();
					/* int32 num_fr = */fh->readSint32LE();

					/*int num_scales = */fh->readSint32LE();

					char tile_flag = fh->readByte();

					if (tile_flag) {
						qdAnimation anim;

						anim.qda_load(it->getPathInArchive());
					}
				}
			}
		}
	}

	debug("======== QDA Scan end ========");
}

} // namespace QDEngine

// Translates cp-1251..utf-8
byte *transCyrillic(const Common::String &str) {
	const byte *s = (const byte *)str.c_str();
	static byte tmp[1024];

#ifndef WIN32
	static int trans[] = {
		0xa0, 0xc2a0,
		0xa8, 0xd081, 0xab, 0xc2ab, 0xb8, 0xd191, 0xbb, 0xc2bb, 0xc0, 0xd090,
		0xc1, 0xd091, 0xc2, 0xd092, 0xc3, 0xd093, 0xc4, 0xd094,
		0xc5, 0xd095, 0xc6, 0xd096, 0xc7, 0xd097, 0xc8, 0xd098,
		0xc9, 0xd099, 0xca, 0xd09a, 0xcb, 0xd09b, 0xcc, 0xd09c,
		0xcd, 0xd09d, 0xce, 0xd09e, 0xcf, 0xd09f, 0xd0, 0xd0a0,
		0xd1, 0xd0a1, 0xd2, 0xd0a2, 0xd3, 0xd0a3, 0xd4, 0xd0a4,
		0xd5, 0xd0a5, 0xd6, 0xd0a6, 0xd7, 0xd0a7, 0xd8, 0xd0a8,
		0xd9, 0xd0a9, 0xda, 0xd0aa, 0xdb, 0xd0ab, 0xdc, 0xd0ac,
		0xdd, 0xd0ad, 0xde, 0xd0ae, 0xdf, 0xd0af, 0xe0, 0xd0b0,
		0xe1, 0xd0b1, 0xe2, 0xd0b2, 0xe3, 0xd0b3, 0xe4, 0xd0b4,
		0xe5, 0xd0b5, 0xe6, 0xd0b6, 0xe7, 0xd0b7, 0xe8, 0xd0b8,
		0xe9, 0xd0b9, 0xea, 0xd0ba, 0xeb, 0xd0bb, 0xec, 0xd0bc,
		0xed, 0xd0bd, 0xee, 0xd0be, 0xef, 0xd0bf, 0xf0, 0xd180,
		0xf1, 0xd181, 0xf2, 0xd182, 0xf3, 0xd183, 0xf4, 0xd184,
		0xf5, 0xd185, 0xf6, 0xd186, 0xf7, 0xd187, 0xf8, 0xd188,
		0xf9, 0xd189, 0xfa, 0xd18a, 0xfb, 0xd18b, 0xfc, 0xd18c,
		0xfd, 0xd18d, 0xfe, 0xd18e, 0xff, 0xd18f, 0x00 };
#endif

	int i = 0;

	for (const byte *p = s; *p; p++) {
#ifdef WIN32
		// translate from cp1251 to cp866
		byte c = *p;
		if (c >= 0xC0 && c <= 0xEF)
			c = c - 0xC0 + 0x80;
		else if (c >= 0xF0)
			c = c - 0xF0 + 0xE0;
		else if (c == 0xA8)
			c = 0xF0;
		else if (c == 0xB8)
			c = 0xF1;
		tmp[i++] = c;
#else
		if (*p < 128) {
			tmp[i++] = *p;
		} else {
			int j;
			for (j = 0; trans[j]; j += 2) {
				if (trans[j] == *p) {
					tmp[i++] = (trans[j + 1] >> 8) & 0xff;
					tmp[i++] = trans[j + 1] & 0xff;
					break;
				}
			}

			if (*p == 0x85) {  // "…" -- Horizontal Ellipsis
				tmp[i++] = 0xE2;
				tmp[i++] = 0x80;
				tmp[i++] = 0xA6;
			} else if (*p == 0x96) {  // "–" -- EN DASH
				tmp[i++] = 0xE2;
				tmp[i++] = 0x80;
				tmp[i++] = 0x93;
			} else if (*p == 0x97) {  // "–" -- EM DASH
				tmp[i++] = 0xE2;
				tmp[i++] = 0x80;
				tmp[i++] = 0x94;
			} else if (*p == 0xB9) {  // "№" -- NUMERO DASH
				tmp[i++] = 0xE2;
				tmp[i++] = 0x84;
				tmp[i++] = 0x96;
			} else {
				if (!trans[j]) {
					warning("transCyrillic: no mapping for %d (0x%x)", *p, *p);
					tmp[i++] = '^';
				}
			}
		}
#endif
	}

	tmp[i] = 0;

	return tmp;
}
