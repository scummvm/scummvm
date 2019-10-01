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

#include "glk/quest/quest.h"
#include "glk/quest/geas_glk.h"
#include "glk/quest/geas_glk.h"
#include "glk/quest/streams.h"
#include "common/config-manager.h"
#include "common/translation.h"

namespace Glk {
namespace Quest {

Quest *g_vm;

Quest::Quest(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		_saveSlot(-1), _runner(nullptr) {
	g_vm = this;
}

void Quest::runGame() {
	// Check for savegame
	_saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	if (!initialize()) {
		GUIErrorMessage(_("Could not start Quest game"));
		return;
	}

	playGame();

	deinitialize();
}

void Quest::playGame() {
	char cur_buf[1024];
	char buf[200];

	// Check for savegame to load immediate
	_saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	// Set initial game state
	_runner->set_game(String(getFilename().c_str()));

	if (_saveSlot != -1) {
		int saveSlot = _saveSlot;
		_saveSlot = -1;

		if (loadGameState(saveSlot).getCode() == Common::kNoError)
			_runner->run_command("look");
	}

	banner = _runner->get_banner();
	draw_banner();

	while (_runner->is_running()) {
		if (inputwin != mainglkwin)
			glk_window_clear(inputwin);
		else
			glk_put_cstring("\n");

		sprintf(cur_buf, "> ");
		glk_put_string_stream(inputwinstream, cur_buf);

		glk_request_line_event(inputwin, buf, (sizeof buf) - 1, 0);

		event_t ev;
		ev.type = evtype_None;

		while (ev.type != evtype_LineInput) {
			glk_select(&ev);
			if (shouldQuit())
				return;

			switch (ev.type) {
			case evtype_LineInput:
				if (ev.window == inputwin) {
					String cmd = String(buf, ev.val1);
					if (inputwin == mainglkwin)
						ignore_lines = 2;
					_runner->run_command(cmd);
				}
				break;

			case evtype_Timer:
				_runner->tick_timers();
				break;

			case evtype_Arrange:
			case evtype_Redraw:
				draw_banner();
				break;
			
			default:
				break;
			}
		}
	}
}

bool Quest::initialize() {
	Streams::initialize();
	glk_stylehint_set(wintype_TextBuffer, style_User2, stylehint_ReverseColor, 1);

	// Open the main window
	mainglkwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	if (!mainglkwin)
		return false;
	glk_set_window(mainglkwin);

	glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);
	bannerwin = glk_window_open(mainglkwin,
		winmethod_Above | winmethod_Fixed,
		1, wintype_TextGrid, 0);

	if (use_inputwindow)
		inputwin = glk_window_open(mainglkwin,
			winmethod_Below | winmethod_Fixed,
			1, wintype_TextBuffer, 0);
	else
		inputwin = NULL;

	if (!inputwin)
		inputwin = mainglkwin;

	inputwinstream = glk_window_get_stream(inputwin);

	if (!glk_gestalt(gestalt_Timer, 0)) {
		const char *err = "\nNote -- The underlying Glk library does not support"
			" timers.  If this game tries to use timers, then some"
			" functionality may not work correctly.\n\n";
		glk_put_string(err);
	}

	glk_request_timer_events(1000);
	ignore_lines = 0;

	_runner = GeasRunner::get_runner(new GeasGlkInterface());

	return true;
}

void Quest::deinitialize() {
	Streams::deinitialize();

	delete _runner;
}

Common::Error Quest::readSaveData(Common::SeekableReadStream *rs) {
	GeasState *gs = _runner->getState();
	gs->load(rs);
	return Common::kNoError;
}

Common::Error Quest::writeGameData(Common::WriteStream *ws) {
	GeasState *gs = _runner->getState();
	gs->save(ws);

	return Common::kNoError;
}

} // End of namespace Quest
} // End of namespace Glk
