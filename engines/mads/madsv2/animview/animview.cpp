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
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/tile.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/animview/animview.h"
#include "mads/madsv2/animview/anim_timer.h"
#include "mads/madsv2/animview/functions.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace AnimView {

#define MADS_FORMAT(BUF, SRC) Common::strcpy_s(BUF, in_mads_mode ? "*" : ""); \
	Common::strcat_s(BUF, SRC)

constexpr bool in_mads_mode = true;

Audio::AudioStream *speechStream;
int speechFlags;
int current_error_code;
int currentFrame;
int minFrame, maxFrame;
bool foundSeries;
int seriesMinFrame, seriesMaxFrame;
bool timerFlag1;
bool peelFlag;
int runCtr1;
int runFx;
long timer1, timer2;
AnimPtr current_anim;
AnimInterPtr current_anim_inter;
int speechIndex;
int speechLoops;
int runVal6, runVal7, runVal8;
bool loadFontFlag;
int imageFrame;
CycleList anim_cycle_list;
bool has_cycles;
int currentViewX, currentViewY;

static const byte FX_TIMES[16] = {
	0, 110, 110, 64, 64, 64, 64, 64, 64, 64, 64, 0, 0, 0
};

static int concat_mode;
static bool has_sound_file;
static char sound_file_name[80];
static TileMapHeader picture_map, depth_map;
static TileResource picture_res, depth_res;
static Buffer scr_work_orig;
static Room *room;
static int viewing_at_y2;
constexpr int SPEECH_LINES_COUNT = 10;
static Audio::AudioStream *speech_lines[SPEECH_LINES_COUNT];
static int speech_lines_count;
static SeriesPtr animSeries;
static SpritePageInfoPtr pageInfo;
static SpritePageTablePtr pageTable;
static int imageFrame1, imageFrame2;
static int largeBufferSize;
static byte *largeBuffer, *largeBufferEnd;
static byte *largeBuffer1, *largeBuffer2, *largeBuffer3;
static bool seriesFlag1, seriesFlag2;
static bool hasAnimInited;
static int runVal1, runVal2, runVal3;
static int runVal12;
static int error_code;
static bool wait_for_music_at_end;

/**
 * Initializes animview global variables
 */
static void init_globals() {
	anim_timer_init();
	functions_init();
	concat_mode = 0;
	has_sound_file = false;
	*sound_file_name = '\0';
	memset(&picture_map, 0, sizeof(TileMapHeader));
	memset(&depth_map, 0, sizeof(TileMapHeader));
	memset(&picture_res, 0, sizeof(TileResource));
	memset(&depth_res, 0, sizeof(TileResource));
	memset(&anim_cycle_list, 0, sizeof(CycleList));
	memset(&scr_work_orig, 0, sizeof(Buffer));
	room = nullptr;
	current_anim = nullptr;
	current_anim_inter = nullptr;
	has_cycles = false;
	viewing_at_y2 = 0;
	memset(speech_lines, 0, sizeof(speech_lines));
	speech_lines_count = 0;
	animSeries = nullptr;
	pageInfo = nullptr;
	pageTable = nullptr;
	foundSeries = false;
	imageFrame1 = imageFrame2 = seriesMaxFrame = 0;
	seriesMinFrame = 0;
	largeBufferSize = 0;
	largeBuffer = largeBufferEnd = nullptr;
	largeBuffer1 = largeBuffer2 = nullptr;
	largeBuffer3 = nullptr;
	seriesFlag1 = false;
	seriesFlag2 = true;
	hasAnimInited = false;
	minFrame = maxFrame = -1;
	timer1 = timer2 = 0;
	runVal1 = 0;
	runVal2 = runVal3 = -1;
	speechIndex = speechLoops = runVal6 = 0;
	runVal7 = runVal8 = 0;
	speechStream = nullptr;
	timerFlag1 = false;
	runVal12 = 0;
	loadFontFlag = 0;
	runFx = 0;
	runCtr1 = 0;
	currentFrame = 0;
	peelFlag = false;
	error_code = current_error_code = 0;
	wait_for_music_at_end = false;
	exit_immediately_at_end = false;
	imageFrame = 0;
}

static void anim_inter_timer() {
	error("TODO: Inter anim timer");
}

/**
 * Responsible for running a loaded animation
 */
static void run_animation(int animIndex) {
	int ctr;

	if (!hasAnimInited) {
		hasAnimInited = true;
		mouse_set_work_buffer(scr_work.data, scr_work.x);
		mouse_set_view_port_loc(0, viewing_at_y, scr_work.x, scr_work.y + viewing_at_y - 1);
		mouse_set_view_port(0, 0);
	}

	auto &screen = *g_engine->getScreen();
	if (viewing_at_y && anim_list[animIndex].show_bars) {
		screen.hLine(0, viewing_at_y - 2, 319, 253);
		screen.hLine(0, viewing_at_y + scr_work.y + 1, 319, 253);
	} else if (viewing_at_y) {
		screen.hLine(0, viewing_at_y - 2, 319, 0);
		screen.hLine(0, viewing_at_y + scr_work.y + 1, 319, 0);
	}

	buffer_fill(scr_work, 0);

	imageFrame = 0;
	currentViewX = currentViewY = -1;
	current_error_code = error_code;

	if (minFrame == -1)
		minFrame = 0;
	if (maxFrame == -1)
		maxFrame = current_anim->num_frames;

	minFrame = CLIP<int>(minFrame, 0, current_anim->num_frames);
	maxFrame = CLIP<int>(maxFrame, 0, current_anim->num_frames);
	if (maxFrame < minFrame)
		maxFrame = minFrame;

	if (animIndex == 0)
		timer1 = timer_read();

	speechIndex = -1;
	loadFontFlag = (current_anim->load_flags & AA_LOAD_FONT) != 0;

	speechLoops = runVal6 = runVal7 = runVal8 = 0;
	speechStream = 0;
	timerFlag1 = false;

	if (current_anim->background_type == AA_INTERFACE) {
		currentFrame = -1;
		runVal12 = -1;
		runFx = 0;

		for (ctr = 0; ctr < current_anim_inter->num_frames; ++ctr)
			current_anim_inter->segment[ctr].counter = -1;

		image_inter_marker = 1;
		image_inter_list[0].flags = -2;
		image_inter_list[0].segment_id = KERNEL_SEGMENT_SYSTEM;

		timer_activate_low_priority(anim_inter_timer);

	} else {
		timer1 += current_anim->frame[minFrame].ticks;
		runFx = anim_list[animIndex].fx;
		currentFrame = minFrame;
		runCtr1 = 0;
		peelFlag = current_anim->misc_peel_x || current_anim->misc_peel_y;
		timer2 = timer1;
		timer_activate_low_priority(anim_timer);
	}

	// Main animation loop
	while (currentFrame < maxFrame && !current_error_code) {
		if (speechStream) {
			if (current_anim->load_flags & AA_LOAD_SPEECH) {
				//char speechName[80];
				//MADS_FORMAT(speechName, current_anim->speech_file);

				g_engine->playSpeech(speechStream);
				//speech_play(speechName, speechStream);
			}

			timerFlag1 = true;
			speechStream = 0;
		}

		if (foundSeries && seriesMinFrame < seriesMaxFrame) {
			// Get the memory needed for the page, rounded up
			int frameIndex = seriesMinFrame - imageFrame1 + 2;
			int pageMemNeeded = (pageTable[frameIndex - 1].memory_needed & ~15) + 16;
			bool flag1 = false;

			if (!seriesFlag1) {
				if ((largeBufferEnd - largeBuffer2) <= pageMemNeeded) {
					flag1 = true;
				} else {
					seriesFlag1 = true;
					largeBuffer2 = largeBuffer;
				}
			}
			if (seriesFlag1) {
				if ((largeBuffer2 - largeBuffer1) <= pageMemNeeded)
					flag1 = true;
			}

			if (seriesFlag2 && currentFrame <= seriesMinFrame && !picture_map.one_to_one)
				flag1 = false;

			if (flag1)
				seriesFlag2 = false;

			if (flag1) {
				(void)sprite_data_load(animSeries, frameIndex, largeBuffer2);
				largeBuffer2 += pageMemNeeded;
				++seriesMinFrame;
			}

			while (((speechIndex == -1 && currentFrame > imageFrame2) ||
					(speechIndex != -1 && current_anim->speech[speechIndex].first_frame > imageFrame2)) &&
					imageFrame2 < seriesMaxFrame) {
				frameIndex = imageFrame2 - imageFrame1 + 1;
				pageMemNeeded = (pageTable[frameIndex - 1].memory_needed & ~15) + 16;
				largeBuffer1 += pageMemNeeded;
				++imageFrame2;

				if (animSeries->index[frameIndex].data != largeBuffer1) {
					largeBuffer1 = largeBuffer;
					seriesFlag1 = false;

					if (seriesMinFrame < imageFrame2)
						largeBuffer2 = largeBuffer;
				}
			}
		}

		if (g_engine->shouldQuit())
			current_error_code = 1;
		if (g_engine->hasPendingKey()) {
			g_engine->flushKeys();
			error_code = 1;
			current_error_code = 1;
		}
		if (mouse_get_status(&mouse_x, &mouse_y)) {
			current_error_code = -1;
			error_code = 3;
		}

		// Animation loop delay
		g_system->delayMillis(10);
	}

	cycling_threshold = 3;
	timer_activate_low_priority(cycling_active ? cycle_colors : nullptr);

	if (!current_error_code && current_anim->misc_slow_fade) {
		timer_activate_low_priority(nullptr);
		timer2 = timer_read();
		bool fadeFlag = true;

		while (fadeFlag && !current_error_code) {
			do {
				// Brief pause
				g_system->delayMillis(10);

				// Check for any keypress
				if (g_engine->hasPendingKey()) {
					g_engine->flushKeys();
					error_code = 1;
					current_error_code = 1;
				}

				if (g_engine->shouldQuit())
					current_error_code = 1;
			} while (timer_read() < timer2);

			if (peelFlag) {
				anim_peel();
				matte_frame(0, 0);
			}

			fadeFlag = anim_fade(&cycling_palette, current_anim->misc_slow_fade);
			mcga_setpal(&cycling_palette);
			timer2 += current_anim->misc_peel_rate;
		}
	}

	if ((animIndex == (anim_count - 1)) &&
			(wait_for_music_at_end || !exit_immediately_at_end)) {
		while (current_error_code == 0) {
			// Check for any keypress or mouse clicks
			if (g_engine->hasPendingKey()) {
				g_engine->flushKeys();
				error_code = 1;
				current_error_code = 1;
			}

			int mouseX = 0, mouseY = 0;
			if (mouse_get_status(&mouseX, &mouseY))
				current_error_code = 1;

			if (g_engine->shouldQuit())
				current_error_code = 1;

			if (!exit_immediately_at_end)
				continue;
			if (g_engine->_soundManager->command(8))
				continue;
			current_error_code = 1;
		}
	}

	// Teardown for final animation
	if (animIndex == (anim_count - 1) || current_error_code) {
		timer_activate_low_priority(nullptr);
	}

	g_engine->stopSpeech();
}

/**
 * Iterate over the entries in the anim_list, animating each
 * in sequence
 */
static void animate() {
	char buf[80];
	AnimFile anim_in;
	int count, series_ctr, ctr;
	int soundLoadFlag = 0;
	bool foundSound;
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
	cycle_list.num_cycles = 0;

	// Preload resources used by the animations
	for (count = 0; count < anim_count; ++count) {
		AnimEntry &entry = anim_list[count];

		MADS_FORMAT(buf, entry.name);
		himem_preload_series(buf, 0);

		if (anim_get_header_info(buf, &anim_in))
			continue;

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

		if (g_engine->shouldQuit())
			error_code = 1;
	}

	if (!do_not_clear_screen) {
		mcga_setpal(&cycling_palette);
	}

	speech_init();

	for (count = 0; count < anim_count && !error_code; ++count) {
		MADS_FORMAT(buf, anim_list[count].name);

		foundSound = false;

		if (!has_sound_file) {
			if (anim_get_sound_info(buf, sound_file_name, &soundLoadFlag))
				goto done;

			Common::String name(sound_file_name);
			has_sound_file = !name.empty() && Common::isDigit(name.lastChar());
		}

		if (has_sound_file) {
			// Initialize the sound driver
			int section = sound_file_name[strlen(sound_file_name) - 1] - '0';
			g_engine->_soundManager->init(section);
		}

		if (anim_list[count].bg_load_status) {
			buffer_free(&scr_depth);
			buffer_free(&scr_orig);
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
		current_anim = anim_load(buf, &scr_orig, &scr_depth,
			&picture_map, &depth_map, &picture_res, &depth_res, &room,
			&anim_cycle_list, loadFlags);
		scr_inter_orig = scr_orig;

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

		has_cycles = anim_cycle_list.num_cycles > 0;
		current_anim_inter = (AnimInterPtr)current_anim;

		int height = (scr_orig.y == 200) ? 200 : 156;
		buffer_init(&scr_work, 320, height);
		scr_inter = scr_work;
		assert(scr_work.data);

		viewing_at_y = (height == 200) ? 0 : (200 - height) / 2;
		viewing_at_y2 = viewing_at_y;

		buffer_fill(scr_work, 0);

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
			largeBufferSize = 0xfffff;	// Aribitrarily large value
			largeBuffer = (byte *)mem_get(largeBufferSize);
			assert(largeBuffer);

			largeBufferEnd = largeBuffer + largeBufferSize - 1;
			largeBuffer1 = largeBuffer2 = largeBuffer;

			foundSeries = true;
			imageIndex = -1;
			seriesFlag1 = false;

			for (ctr = 0; ctr < current_anim->num_images; ++ctr) {
				int seriesId = current_anim->series_id[packIndex];
				if (current_anim->image[ctr].series_id == seriesId) {
					imageIndex = ctr;
					break;
				}
			}

			if (imageIndex < 0)
				foundSeries = false;

			if (foundSeries) {
				Image &img = current_anim->image[imageIndex];
				imageFrame1 = imageFrame2 = img.flags;
				seriesMinFrame = img.flags - 1;
				seriesMaxFrame = img.flags + img.sprite_id - 1;
			}
		}

		// Run the animation
		run_animation(count);

		for (auto &line : speech_lines) {
			mem_free(line);
			line = nullptr;
		}

		mem_free(largeBuffer);
		largeBuffer = nullptr;

		minFrame = maxFrame = -1;

		// Free the allocated sound driver
		g_engine->_soundManager->closeDriver();
		has_sound_file = false;

		// Free surface
		buffer_free(&scr_work);
		anim_unload(current_anim);
		current_anim = nullptr;

		if (g_engine->shouldQuit())
			error_code = 1;
	}
done:
	timer_activate_low_priority(nullptr);
	buffer_free(&scr_work);
	anim_unload(current_anim);
	buffer_free(&scr_depth);
	buffer_free(&scr_orig);
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
	mouse_hide();

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
