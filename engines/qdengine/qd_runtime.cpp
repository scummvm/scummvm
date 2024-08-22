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

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/events.h"

#include "qdengine/resource.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_trigger_chain.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/system/sound/snd_dispatcher.h"
#include "qdengine/qdcore/qd_file_manager.h"
#include "qdengine/qdcore/util/plaympp_api.h"
#include "qdengine/qdcore/util/splash_screen.h"
#include "qdengine/qdcore/util/ResourceDispatcher.h"
#include "qdengine/qdcore/util/WinVideo.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/rle_compress.h"
#include "qdengine/system/input/input_wndproc.h"
#include "qdengine/system/input/mouse_input.h"
#include "qdengine/system/input/keyboard_input.h"

namespace QDEngine {

namespace qdrt {

void init_graphics();
bool init_graphics_dispatcher();
void restore_graphics();

void qd_show_load_progress(int percents_loaded, void *p);

void restore();

grDispatcher *grD = NULL;

qdGameDispatcher *qd_gameD = NULL;

}; // namespace qdrt

using namespace qdrt;

static void generateTagMap(int date, bool verbose = true) {
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
		return 20030919;		// QDSCR_GAME_TITLE = 182, 06b1cf45d
	} else if (gameID == "nupogodi3") {
		return 20031014;		// QDSCR_TEXT_DB = 184, d864cc279
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
		return 20060129;		// QDSCR_GAME_TITLE = 214
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

int QDEngineEngine::engineMain() {
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.qml");
	Common::ArchiveMemberPtr p = files.front();
	Common::String firstFileName;
	Common::String script_name;

	if (p) {
		firstFileName = p->getFileName();
		script_name = firstFileName.c_str();
	}

	if (script_name.empty()) {
		return 0;
	}

	grD = new grDispatcher();

	grD->hideMouse();

	grD->showMouse(); // FIXME HACK

	qdGameConfig::get_config().load();

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

	qd_gameD = new qdGameDispatcher;
	qd_gameD->load_script(script_name.c_str());

	if (ConfMan.getBool("dump_scripts")) {
		qd_gameD->save_script("qd_game.xml");
		debug("Dumped qd_game.xml");
	}

	qd_gameD->set_scene_loading_progress_callback(qd_show_load_progress);

	if (ConfMan.getBool("splash_enabled")) {
		sp.wait(ConfMan.getInt("splash_time"));
		sp.destroy();
	}

	init_graphics();

	sndDispatcher *sndD = new sndDispatcher;

	qdGameConfig::get_config().update_sound_settings();
	qdGameConfig::get_config().update_music_settings();

	winVideo::init();

	qd_gameD->load_resources();

	if (ConfMan.hasKey("boot_param")) {
		const char *scene_name = ""; // FIXME. Implement actual scene selection
		if (!qd_gameD->select_scene(scene_name))
			error("Cannot find the startup scene");
	} else {
		bool music_enabled = mpegPlayer::instance().is_enabled();
		mpegPlayer::instance().disable();

		qd_gameD->toggle_main_menu(true);
		if (!qd_gameD->start_intro_videos()) {
			if (music_enabled)
				mpegPlayer::instance().enable(true);
		} else {
			if (music_enabled)
				mpegPlayer::instance().enable(false);
		}
	}

	qd_gameD->update_time();
	qd_gameD->quant();

	ResourceDispatcher resD;
	resD.setTimer(ConfMan.getBool("logic_synchro_by_clock"), ConfMan.getInt("logic_period"), 300);
	resD.attach(new MemberFunctionCallResourceUser<qdGameDispatcher>(*qd_gameD, &qdGameDispatcher::quant, ConfMan.getInt("logic_period")));
	sndD->set_frequency_coeff(qdGameConfig::get_config().game_speed());
	resD.set_speed(qdGameConfig::get_config().game_speed());
	resD.start();

	bool exit_flag = false;
	bool was_inactive = false;

	// Activate the window
	grDispatcher::activate(true);

	Common::Event event;

	while (!exit_flag && !qd_gameD->need_exit()) {
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				if (!grDispatcher::instance()->is_in_reinit_mode())
					exit_flag = true;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.ascii == 'f')
					qdGameConfig::get_config().toggle_fps();
#ifdef __QD_DEBUG_ENABLE__
				else if (event.kbd.keycode == Common::KEYCODE_PAGEDOWN) {
					float speed = qdGameConfig::get_config().game_speed() * 0.9f;
					if (speed < 0.1f) speed = 0.1f;
					qdGameConfig::get_config().set_game_speed(speed);
					sndD->set_frequency_coeff(speed);
					resD.set_speed(qdGameConfig::get_config().game_speed());
				} else if (event.kbd.keycode == Common::KEYCODE_PAGEUP) {
					float speed = qdGameConfig::get_config().game_speed() * 1.1f;
					if (speed > 10.0f) speed = 10.0f;
					qdGameConfig::get_config().set_game_speed(speed);
					sndD->set_frequency_coeff(speed);
					resD.set_speed(qdGameConfig::get_config().game_speed());
				} else if (event.kbd.keycode == Common::KEYCODE_HOME) {
					qdGameConfig::get_config().set_game_speed(1.0f);
					sndD->set_frequency_coeff(1.0f);
					resD.set_speed(qdGameConfig::get_config().game_speed());
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
			qd_gameD->redraw();

		} else {
			was_inactive = true;
			g_system->delayMillis(100);
			resD.skip_time();
		}

		g_system->updateScreen();
	}

	delete qd_gameD;

	grDispatcher::instance()->finit();

	qdFileManager::instance().Finit();

	delete sndD;
	delete grD;

	winVideo::done();

	RLEBuffer::releaseBuffers();

	return 0;
}

namespace qdrt {

void init_graphics() {
	grDispatcher::set_restore_handler(restore_graphics);
	grDispatcher::instance()->finit();

	grDispatcher::set_instance(grD);

	if (!init_graphics_dispatcher())
		return;

	qdGameConfig::get_config().set_pixel_format(grDispatcher::instance()->pixel_format());

	grDispatcher::instance()->setClip();
	grDispatcher::instance()->setClipMode(1);

	grDispatcher::instance()->fill(0);

	g_system->updateScreen();

	grDispatcher::instance()->flush();
}

bool init_graphics_dispatcher() {
	grDispatcher::instance()->init(g_engine->_screenW, g_engine->_screenH, (grPixelFormat)qdGameConfig::get_config().pixel_format());
	return true;
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

void restore() {
}

}; // namespace main
} // namespace QDEngine
