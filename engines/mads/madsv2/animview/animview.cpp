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

#include "audio/audiostream.h"
#include "common/file.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/tile.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/animview/animview.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace AnimView {

#define MADS_FORMAT(BUF, SRC) Common::strcpy_s(BUF, in_mads_mode ? "*" : ""); \
	Common::strcat_s(BUF, SRC)

struct AnimEntry {
	char name[16];
	uint8 bg_load_status;
	uint8 sound_mode;
	uint8 show_bars;
	uint8 fx;
};
constexpr int MAX_ANIM = 40;
constexpr bool in_mads_mode = true;

static int anim_count;
static AnimEntry anim_list[MAX_ANIM];
static uint8 background_load_status;
static int16 sound_interrupts_mode;
static bool show_white_bars;
static int concat_mode;
static bool resync_timer1, resync_timer2;
static bool exit_immediately_at_end;
static bool do_not_clear_screen;
static bool has_sound_file;
static char sound_file_name[80];
static TileMapHeader picture_map, depth_map;
static TileResource picture_res, depth_res;
static CycleList cycle_list;
static Buffer scr_work_orig;
static Room *room;
static Anim *current_anim, *current_anim2;
static bool has_cycles;
static int viewing_at_y2;
constexpr int SPEECH_LINES_COUNT = 10;
static Audio::AudioStream *speech_lines[SPEECH_LINES_COUNT];
static int speech_lines_count;
static SeriesPtr animSeries;
static SpritePageInfoPtr pageInfo;
static SpritePageTablePtr pageTable;
static void *anim_buffer;
static bool foundSeries;
static int imageFlags, imageFlags2, imageFlags3;
static int imageSpriteId;
static int largeBufferSize;
static byte *largeBuffer, *largeBufferEnd;
static byte *largeBuffer1, *largeBuffer2;

/**
 * Initializes animview global variables
 */
static void init_globals() {
	anim_count = 0;
	background_load_status = 0xff;
	sound_interrupts_mode = -1;
	show_white_bars = true;
	concat_mode = 0;
	resync_timer1 = true;
	resync_timer2 = false;
	exit_immediately_at_end = false;
	do_not_clear_screen = false;
	has_sound_file = false;
	*sound_file_name = '\0';
	memset(&picture_map, 0, sizeof(TileMapHeader));
	memset(&depth_map, 0, sizeof(TileMapHeader));
	memset(&picture_res, 0, sizeof(TileResource));
	memset(&depth_res, 0, sizeof(TileResource));
	memset(&cycle_list, 0, sizeof(CycleList));
	memset(&scr_work_orig, 0, sizeof(Buffer));
	room = nullptr;
	current_anim = current_anim2 = nullptr;
	has_cycles = false;
	viewing_at_y2 = 0;
	memset(speech_lines, 0, sizeof(speech_lines));
	speech_lines_count = 0;
	animSeries = nullptr;
	pageInfo = nullptr;
	pageTable = nullptr;
	anim_buffer = nullptr;
	foundSeries = false;
	imageFlags = imageFlags2 = imageFlags3 = 0;
	imageSpriteId = 0;
	largeBufferSize = 0;
	largeBuffer = largeBufferEnd = nullptr;
	largeBuffer1 = largeBuffer2 = nullptr;
}

/**
 * Adds an animation to the list of .aa files to show in sequence
 * @param name		Animation resource name
 */
static void add_anim(const char *name) {
	static char buf[16];

	if (strlen(name) > 0 && anim_count < MAX_ANIM) {
		Common::strcpy_s(buf, name);
		if (!strchr(buf, '.'))
			Common::strcat_s(buf, ".aa");

		Common::strcpy_s(anim_list[anim_count].name, buf);
		anim_list[anim_count].bg_load_status = background_load_status;
		anim_list[anim_count].sound_mode = sound_interrupts_mode;
		anim_list[anim_count].show_bars = show_white_bars;
		++anim_count;
	}
}

/**
 * Parses a flag from an animation line in the resource file
 */
static void flag_parse(const char *param) {
	switch (tolower(*param++)) {
	case 'o':
		// Specify opening special effect
		assert(anim_count < MAX_ANIM);
		if (*param == ':')
			anim_list[anim_count].fx = atoi(param + 1);
		break;

	case 'r':
		// -r[:abn] Resynch timer (always, beginning, never)
		if (*param == ':') {
			switch (tolower(*++param)) {
			case 'n':
				resync_timer1 = true;
				resync_timer2 = false;
				break;
			case 'a':
				resync_timer1 = false;
				break;
			case 'b':
				resync_timer1 = true;
				resync_timer2 = true;
				break;
			default:
				break;
			}
		}
		break;

	case 'w':
		// Toggle white bars on or off
		show_white_bars = !show_white_bars;
		break;

	case 'x':
		// Exit immediately after last frame
		exit_immediately_at_end = true;
		break;

	case 'y':
		// Do not clear screen at start
		do_not_clear_screen = true;
		break;

	default:
		error("Unsupported animview flag - %c", *param);
		break;
	}
}

/**
 * Reads the contents of the resource file stream, and adds
 * entries to the anim_list for what to display
 */
static void read_resource(Common::SeekableReadStream *src) {
	while (!src->eos()) {
		Common::String line = src->readLine();
		line.trim();
		if (line.empty())
			continue;

		// Handle any flags at the start of the line
		const char *lineP = line.c_str();
		while (strchr("/-", *lineP)) {
			// It's a flag
			++lineP;
			const char *switchEnd = strchr(lineP, ' ');
			Common::String param;

			if (switchEnd) {
				// There's more line after the flag
				param = Common::String(lineP, switchEnd);
				for (lineP = switchEnd; *lineP == ' '; ++lineP) {
				}
			} else {
				// This is the last flag of the line
				param = Common::String(lineP);
				lineP = lineP + strlen(lineP);
			}

			flag_parse(param.c_str());
		}

		// As long as we're not at the end of the line, any remainder
		// should be the name of the animation resource to play
		if (*lineP)
			add_anim(lineP);
	}
}

static void run_animation() {
	// TODO
}

/**
 * Iterate over the entries in the anim_list, animating each
 * in sequence
 */
static void animate() {
	char buf[80], speech_name[80];
	AnimFile anim_in;
	int count, series_ctr, ctr;
	int soundLoadFlag = 0;
	bool foundSound;
	int oldMode;
	int imageIndex;
	static int packIndex = 0;

	himem_startup();
	(void)tile_setup();

	mcga_compute_retrace_parameters();
	memset(cycling_palette, 0, sizeof(Palette));
	pal_init(1, 8);
	mouse_hard_cursor_mode(2, master_palette);

	timer_install();
	matte_init(-1);

	// Preload resources used by the animations
	for (count = 0; count < anim_count && !g_engine->shouldQuit(); ++count) {
		AnimEntry &entry = anim_list[count];

		MADS_FORMAT(buf, entry.name);
		himem_preload_series(buf, 0);

		if (anim_get_header_info(buf, &anim_in))
			return;

		// Preload resources used by the animation
		if (anim_in.load_flags & AA_LOAD_FONT) {
			*buf = '\0';
			if (in_mads_mode)
				Common::strcpy_s(buf, "*");
			Common::strcat_s(buf, anim_in.font_file);
			himem_preload_series(buf, 0);
		}

		for (series_ctr = 0; series_ctr < anim_in.num_series; ++series_ctr) {
			MADS_FORMAT(buf, anim_in.series_name[series_ctr]);
			himem_preload_series(buf, 0);
		}

		if (anim_in.background_type == AA_ROOM) {
			static const char *EXT[5] = { ".dat", ".tt", ".mm", ".tt0", ".mm0" };
			for (int i = 0; i < 5; ++i) {
				env_get_level_path(buf, anim_in.background_room, EXT[i], 3, 0);
				himem_preload_series(buf, 0);
			}
		}
	}

	if (!do_not_clear_screen) {
		mcga_setpal(&cycling_palette);
	}

	speech_init();

	for (count = 0; count < anim_count; ++count) {
		MADS_FORMAT(buf, anim_list[count].name);

		foundSound = false;

		if (!has_sound_file) {
			if (anim_get_sound_info(buf, sound_file_name, &soundLoadFlag))
				goto done;

			if (soundLoadFlag) {
				oldMode = concat_mode;
				*speech_name = '\0';
				MADS_FORMAT(speech_name, sound_file_name);
				env_get_path(sound_file_name, speech_name);
				concat_mode = oldMode;

				// Original did setup of sound card driver type here. Not needed for ScummVM

				has_sound_file = Common::File::exists(sound_file_name);
			}
		}

		if (has_sound_file)
			// TODO: Load proper driver number
			g_engine->_soundManager->init(9);

		if (anim_list[count].bg_load_status) {
			buffer_free(&scr_depth);
			buffer_free(&scr_inter);
			tile_map_free(&picture_map);
			tile_map_free(&depth_map);

			if (room) {
				pal_deallocate(room->color_handle);
				mem_free(room);
			} else {
				pal_init(1, 8);
				mouse_hard_cursor_mode(2, &master_palette);
			}
		}

		int loadFlags = anim_list[count].bg_load_status ? ANIM_LOAD_BACKGROUND : 0;
		current_anim = anim_load(buf, &scr_work, &scr_depth,
			&picture_map, &depth_map, &picture_res, &depth_res, &room,
			&cycle_list, loadFlags);
		scr_work_orig = scr_work;

		if (!current_anim)
			error("Could not load anim for - %s", buf);

		tile_pan(&picture_map, current_anim->frame->view_x, current_anim->frame->view_y);
		tile_pan(&depth_map, current_anim->frame->view_x, current_anim->frame->view_y);

		if (current_anim->misc_any_packed) {
			packIndex = current_anim->misc_packed_series;
			animSeries = current_anim->series[packIndex];
			pageInfo = animSeries->page_info;
			pageTable = animSeries->page_table;
			mem_free(animSeries->arena);
			animSeries->arena = nullptr;
		}

		has_cycles = cycle_list.num_cycles > 0;
		current_anim2 = current_anim;

		int height = (scr_work.y == 200) ? 200 : 156;
		buffer_init(&scr_inter_orig, 320, height);
		scr_inter = scr_inter_orig;

		viewing_at_y = (height == 200) ? 0 : 200 - (height / 2);
		viewing_at_y2 = viewing_at_y;

		buffer_fill(scr_inter_orig, 0);

		// Speech handling
		speech_lines_count = 0;
		for (ctr = 0; ctr < current_anim->num_speech; ++ctr) {
			Speech &speech = current_anim->speech[ctr];
			speech.speech = nullptr;

			if ((speech.flags & 0x2000) && speech_lines_count < SPEECH_LINES_COUNT) {
				// Load the speech audio
				MADS_FORMAT(buf, current_anim->speech_file);
				speech_lines[speech_lines_count] = speech.speech =
					speech_load(buf, speech.resource_id);

				++speech_lines_count;
			}
		}

		if (current_anim->misc_any_packed) {
			largeBufferSize = 0xffff;
			largeBuffer = (byte *)mem_get(largeBufferSize);
			largeBufferEnd = largeBuffer + largeBufferSize - 1;
			largeBuffer1 = largeBuffer2 = largeBuffer;

			foundSeries = false;
			imageIndex = -1;
			for (ctr = 0; ctr < current_anim->num_images; ++ctr) {
				int seriesId = current_anim->series_id[packIndex];
				if (current_anim->image[ctr].series_id == seriesId) {
					imageIndex = ctr;
					foundSeries = true;
				}
			}

			if (foundSeries) {
				Image &img = current_anim->image[imageIndex];
				imageFlags = imageFlags2 = img.flags;
				imageFlags3 = imageFlags - 1;
				imageSpriteId = img.sprite_id;
			}
		}

		// Run the animation
		run_animation();

		for (auto &line : speech_lines) {
			mem_free(line);
			line = nullptr;
		}

		mem_free(anim_buffer);
		anim_buffer = nullptr;

		// Free the allocated sound driver
		g_engine->_soundManager->closeDriver();

		// Free surface
		buffer_free(&scr_inter_orig);
		anim_unload(current_anim);
		current_anim = nullptr;
	}
done:
	timer_activate_low_priority(nullptr);
	buffer_free(&scr_work);
	anim_unload(current_anim);
	buffer_free(&scr_depth);
	buffer_free(&scr_inter);
	tile_map_free(&picture_map);
	tile_map_free(&depth_map);

	if (room)
		mem_free(room);
	timer_set_sound_flag(false);

	timer_remove();
	himem_shutdown();
}

void animview_main(const char *resName) {
	char name[16];

	init_globals();

	pack_enable_pfab_explode();
	(void)env_verify();

	Common::strcpy_s(name, resName);
	if (*name == '@') {
		Common::strcpy_s(name, resName + 1);
		Common::strcat_s(name, ".res");
	}

	if (!Common::File::exists(name)) {
		Common::strcpy_s(name, "*");
		Common::strcat_s(name, resName);
	}

	Common::SeekableReadStream *file = env_open(name);
	if (!file)
		error("Could not open animview resource - %s", name);

	// Read in the resource lines
	read_resource(file);
	delete file;

	if (has_sound_file) {
		char snd_name[80];
		MADS_FORMAT(snd_name, sound_file_name);
		env_get_path(sound_file_name, snd_name);
	}

	animate();
}

} // namespace AnimView
} // namespace MADSV2
} // namespace MADS
