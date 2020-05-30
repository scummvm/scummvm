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

#include "glk/comprehend/comprehend.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "glk/comprehend/dump_game_data.h"
#include "glk/comprehend/game.h"
#include "glk/comprehend/game_cc.h"
#include "glk/comprehend/game_data.h"
#include "glk/comprehend/game_oo.h"
#include "glk/comprehend/game_tm.h"
#include "glk/comprehend/game_tr.h"
#include "glk/quetzal.h"

namespace Glk {
namespace Comprehend {

Comprehend *g_comprehend;

struct DumpOption {
	const char *const option;
	unsigned flag;
};

static const DumpOption dump_options[] = {
    {"strings", DUMP_STRINGS},
    {"extra-strings", DUMP_EXTRA_STRINGS},
    {"rooms", DUMP_ROOMS},
    {"items", DUMP_ITEMS},
    {"dictionary", DUMP_DICTIONARY},
    {"word-pairs", DUMP_WORD_PAIRS},
    {"actions", DUMP_ACTIONS},
    {"functions", DUMP_FUNCTIONS},
    {"replace-words", DUMP_REPLACE_WORDS},
    {"header", DUMP_HEADER},
    {"all", DUMP_ALL},
};

#ifdef TODO
int main(int argc, char **argv) {
	option long_opts[] = {
	    {"debug", no_argument, 0, 'd'},
	    {"dump", required_argument, 0, 'D'},
	    {"no-play", no_argument, 0, 'p'},
	    {"no-graphics", no_argument, 0, 'g'},
	    {"no-floodfill", no_argument, 0, 'f'},
	    {"graphics-width", required_argument, 0, 'w'},
	    {"graphics-height", required_argument, 0, 'h'},
	    {"help", no_argument, 0, '?'},
	    {NULL, 0, 0, 0},
	};
	const char *short_opts = "dD:pgfw:h:?";
	ComprehendGame *game;
	const char *game_name, *game_dir;
	unsigned dump_flags = 0;
	int i, c, opt_index;
	unsigned graphics_width = G_RENDER_WIDTH,
	         graphics_height = G_RENDER_HEIGHT;
	bool play_game = true, graphics_enabled = true;

	while (1) {
		c = getopt_long(argc, argv, short_opts, long_opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case 'd':
			// FIXME
			debug_enable(DEBUG_FUNCTIONS);
			break;

		case 'D':
			for (i = 0; i < ARRAY_SIZE(dump_options); i++)
				if (strcmp(optarg, dump_options[i].option) == 0)
					break;
			if (i == ARRAY_SIZE(dump_options)) {
				printf("Invalid dump option '%s'\n", optarg);
				usage(argv[0]);
			}

			dump_flags |= dump_options[i].flag;
			break;

		case 'p':
			play_game = false;
			break;

		case 'g':
			graphics_enabled = false;
			break;

		case 'f':
			image_set_draw_flags(IMAGEF_NO_FLOODFILL);
			break;

		case 'w':
			graphics_width = strtoul(optarg, NULL, 0);
			break;

		case 'h':
			graphics_height = strtoul(optarg, NULL, 0);
			break;

		case '?':
			usage(argv[0]);
			break;

		default:
			printf("Invalid option\n");
			usage(argv[0]);
			break;
		}
	}

	if (optind >= argc || argc - optind != 2)
		usage(argv[0]);

	game_name = argv[optind++];
	game_dir = argv[optind++];
}
#endif

Comprehend::Comprehend(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
                                                                            _saveSlot(-1) {
	g_comprehend = this;
}

Comprehend::~Comprehend() {
	g_comprehend = nullptr;
}

void Comprehend::runGame() {
	initialize();

	// Lookup game
	ComprehendGame *game = createGame();

	comprehend_load_game(game);
	comprehend_play_game(game);

	delete game;
	deinitialize();
}

void Comprehend::initialize() {
	_topWindow = (GraphicsWindow *)glk_window_open(0, 0, 0, wintype_Graphics, 1);
	_bottomWindow = (TextBufferWindow *)glk_window_open(
	    _topWindow, winmethod_Below | winmethod_Fixed,
	    80, wintype_TextBuffer, 0);
	glk_set_window(_bottomWindow);
}

void Comprehend::deinitialize() {
	glk_window_close(_topWindow);
	glk_window_close(_bottomWindow);
}

ComprehendGame *Comprehend::createGame() {
	if (_gameDescription._gameId == "crimsoncrown")
		return new CrimsonCrownGame();
	if (_gameDescription._gameId == "ootopis")
		return new OOToposGame();
	if (_gameDescription._gameId == "talisman")
		return new OOToposGame();
	if (_gameDescription._gameId == "transylvania")
		return new TransylvaniaGame();

	error("Unknown game");
}

void Comprehend::print(const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	Common::String msg = Common::String::vformat(fmt, argp);
	va_end(argp);

	glk_put_string_stream(glk_window_get_stream(_bottomWindow),
		msg.c_str());
}

void Comprehend::readLine(char *buffer, size_t maxLen) {
	event_t ev;

	glk_request_line_event(_bottomWindow, buffer, maxLen - 1, 0);

	for (;;) {
		glk_select(&ev);
		if (ev.type == evtype_Quit) {
			glk_cancel_line_event(_bottomWindow, &ev);
			return;
		} else if (ev.type == evtype_LineInput)
			break;
	}

	buffer[ev.val1] = 0;
}


} // namespace Comprehend
} // namespace Glk
