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

#include "m4/m4_types.h"
#include "m4/adv_r/adv_control.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/graphics/krn_pal.h"
#include "m4/graphics/gr_pal.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/platform/keys.h"
#include "m4/vars.h"
#include "m4/m4.h"

namespace M4 {

#define _GP(X) _G(krnPal)._##X

#define BACKGROUND_HEIGHT  (int32)639

#define GREY_START   32
#define NUM_GREYS    32					// gotta have 32 greys to fade to (hardcoded algorithm) 
#define GREY_END     GREY_START+NUM_GREYS      		

#define FREE_START   GREY_END+1
#define FREE_END     255
#define NUM_FREE     FREE_END-(FREE_START)+1

static HotkeyCB remember_esc_key;
static HotSpotRec *exam_saved_hotspots;

void krn_pal_game_task() {
	g_engine->pal_game_task();
}

static int32 screen_height(Buffer *grey_screen) {
	return imath_min(BACKGROUND_HEIGHT + _G(kernel).letter_box_y, grey_screen->h);
}

static void grey_fade(RGB8 *pal, int32 to_from_flag, int32 from, int32 to, int32 steps, int32 delay) {
	int i, j;
	RGB8 *working = (RGB8 *)mem_alloc(sizeof(RGB8) * 256, STR_FADEPAL);

	// perform the fade
	for (i = 1; i < steps; i++) {
		for (j = from; j <= to; j++) {
			if (to_from_flag == TO_GREY) {      	// fade to grey from full color
				working[j].r = (Byte)((int)pal[j].r + ((((int)_GP(fadeToMe)[j].r - (int)pal[j].r) * i) / steps));
				working[j].g = (Byte)((int)pal[j].g + ((((int)_GP(fadeToMe)[j].g - (int)pal[j].g) * i) / steps));
				working[j].b = (Byte)((int)pal[j].b + ((((int)_GP(fadeToMe)[j].b - (int)pal[j].b) * i) / steps));
			} else if (to_from_flag == TO_COLOR) {  // fade from grey to full color
				working[j].r = (Byte)((int)_GP(fadeToMe)[j].r + ((((int)pal[j].r - (int)_GP(fadeToMe)[j].r) * i) / steps));
				working[j].g = (Byte)((int)_GP(fadeToMe)[j].g + ((((int)pal[j].g - (int)_GP(fadeToMe)[j].g) * i) / steps));
				working[j].b = (Byte)((int)_GP(fadeToMe)[j].b + ((((int)pal[j].b - (int)_GP(fadeToMe)[j].b) * i) / steps));
			} else {											//fade from grey to black
				working[j].r = (Byte)((int)_GP(fadeToMe)[j].r - ((((int)_GP(fadeToMe)[j].r) * i) / steps));
				working[j].g = (Byte)((int)_GP(fadeToMe)[j].g - ((((int)_GP(fadeToMe)[j].g) * i) / steps));
				working[j].b = (Byte)((int)_GP(fadeToMe)[j].b - ((((int)_GP(fadeToMe)[j].b) * i) / steps));
			}
		}

		gr_pal_set_range(working, from, to - from + 1);   ///set pal 21-255

		// Time delay of "delay" milliseconds
		g_events->delay(delay);
	}

	// Eliminate round off error
	if (to_from_flag == TO_GREY) {
		gr_pal_set_range(_GP(fadeToMe), from, to - from + 1);   ///set pal 21-255
	} else if (to_from_flag == TO_COLOR) {
		gr_pal_set_range(pal, from, to - from + 1);   ///set pal 21-255
	} else {
		for (i = from; i <= to; i++) {
			pal[i].r = pal[i].g = pal[i].b = 0;
		}
		gr_pal_set_range(pal, from, to - from + 1);   ///set pal 21-255
	}

	mem_free(working);
}



// screen is the currently displayed screen
// screenPicture is the data to restore the screen with
// note: color 0 doesn't fade.

static void create_luminance_map(RGB8 *pal) {
	for (int i = GREY_START; i <= FREE_END; i++) {
		Byte luminance = (Byte)((pal[i].r + pal[i].g + pal[i].b) / 3);
		_GP(fadeToMe)[i].g = (Byte)imath_min(255, luminance);		 // New green screen!
		_GP(fadeToMe)[i].r = _GP(fadeToMe)[i].b = 0;
	}
}

// finds the best macthes for the in the greys in the grey ramp range using the free range greys
// used to map greys out of the grey ramp area, and then again to map the grey ramp out of the grey ramp area!
static void make_translation_table(RGB8 *pal) {
	int32    i, j, bestMatch, minDist;

	for (i = 0; i < NUM_GREYS; i++) {
		bestMatch = FREE_START;  // assume the first of the free indexes is best match to start with
		minDist = 255;	  // assume that it's really far away to start with

		if (!(i & 0x3ff)) {
			digi_read_another_chunk();
			midi_loop();
		}

		// look for best match in the free indexes for the greys in GREY_START-GREY_END range (we need these available)
		int32 matchGrey = pal[GREY_START + i].g;	  // Use green instead of red cause we're having a green screen

		for (j = FREE_START; j <= FREE_END; j++) {
			int32 tryGrey = pal[j].g;
			if (imath_abs(tryGrey - matchGrey) < minDist) {
				minDist = imath_abs(tryGrey - matchGrey);
				bestMatch = j;
			}
			if (minDist == 0)
				break;        // no need to continue searching if we found a perfect match
		}
		_GP(translation)[i] = (uint8)bestMatch;
	}
}

void krn_fade_to_grey(RGB8 *pal, int32 steps, int32 delay) {
	int32 i, j, bestMatch, minDist;
	uint8 *tempPtr;

	if (_G(kernel).fading_to_grey) {
		return;
	}
	_G(kernel).fading_to_grey = true;

	Buffer *grey_screen = _G(gameDrawBuff)->get_buffer();

	_GP(fadeToMe) = (RGB8 *)mem_alloc(sizeof(RGB8) * 256, STR_FADEPAL);
	_GP(trick) = (RGB8 *)mem_alloc(sizeof(RGB8) * 256, STR_FADEPAL);
	_GP(picPal) = (RGB8 *)mem_alloc(sizeof(RGB8) * 256, STR_FADEPAL);

	memcpy(_GP(picPal), pal, sizeof(RGB8) * 256);
	create_luminance_map(pal);

	grey_fade(pal, TO_GREY, 21, 255, steps, delay);

	// Make translation table to translate colors using entries 59-255 into 21-58 range

	for (i = 0; i < 32; i++) {
		bestMatch = 65;
		minDist = 255;

		if (!(i & 0x3ff)) {
			_G(digi).task();
			_G(midi).task();
		}

		for (j = 59; j <= 255; j++) {
			if (imath_abs((_GP(fadeToMe)[j].r >> 2) - i) < minDist) {
				minDist = imath_abs((_GP(fadeToMe)[j].r >> 2) - i);
				bestMatch = j;
			}
			if (minDist == 0)
				// No need to continue searching if we found a perfect match
				break;
		}

		_GP(translation)[i] = (uint8)bestMatch;
	}

	// Palette now grey scale. Remap any pixels which are in the range 21-58 to the range 53-255
	// because we need to use those palette entries soon

	tempPtr = grey_screen->data;

	// Note: this loop should be y0 to y1, x0 to x1, not a stride*h loop.
	for (i = 0; i < (grey_screen->stride * grey_screen->h); i++) {
		if ((*tempPtr >= GREY_START) && (*tempPtr <= GREY_END)) {
			// Must move the pixel index to the best match in FREE_START-FREE_END range with _GP(translation) table
			*tempPtr = _GP(translation)[*tempPtr - GREY_START];
		}
		tempPtr++;

		if (!(i & 0x3ff)) {
			_G(digi).task();
			_G(midi).task();
		}

	}

	RestoreScreens(MIN_VIDEO_X, MIN_VIDEO_Y, MAX_VIDEO_X, MAX_VIDEO_Y);

	// Make new trickPal with grey-scale ramp entries and load it into VGA registers
	memcpy(_GP(trick), _GP(fadeToMe), sizeof(RGB8) * 256);	// trick pal is the greyed version plus the grey ramp overlayed on top
	int8 grey_step = 256 / NUM_GREYS;
	int8 grey_ramp = 0;
	for (i = GREY_START; i <= GREY_END; i++) {
		_GP(trick)[i].g = (Byte)(grey_ramp);		  // New green screen
		_GP(trick)[i].r = _GP(trick)[i].b = 0;
		grey_ramp += grey_step;
	}

	gr_pal_set_range(_GP(trick), GREY_START, NUM_GREYS);
	remap_buffer_with_luminance_map(grey_screen, 0, 0, grey_screen->w - 1, screen_height(grey_screen) - 1);

	_G(gameDrawBuff)->release();
	RestoreScreens(MIN_VIDEO_X, MIN_VIDEO_Y, MAX_VIDEO_X, MAX_VIDEO_Y);
}

void krn_fade_from_grey(RGB8 *pal, int32 steps, int32 delay, int32 fadeType) {
	uint8 *tempPtr;
	int32 i;

	if (!_G(kernel).fading_to_grey) {
		return;
	}

	// Get the screen
	Buffer *grey_screen = _G(gameDrawBuff)->get_buffer();

	// load original faded greys into the free indexes (no pixels have these indexs yet)
	gr_pal_set_range(_GP(fadeToMe), FREE_START, NUM_FREE);   // Load fadeToMe colors into VGA

	make_translation_table(_GP(trick)); // This is used in fade_to_grey too!

	// for every pixel in the screen, move any pixel in the GREY_START-GREY_END range out in to the free range
	tempPtr = grey_screen->data;
	// note: this loop should be y0 to y1, x0 to x1, not a stride*h loop.
	for (i = 0; i < (grey_screen->stride * grey_screen->h); ++i) {
		if (!(i & 0x3ff)) {
			_G(digi).task();
			_G(midi).task();
		}

		// if the pixel is within the GREY range, move it to where the _GP(translation) table says
		if ((*tempPtr >= GREY_START) && (*tempPtr <= GREY_END)) {
			*tempPtr = _GP(translation)[*tempPtr - GREY_START];
		}
		tempPtr++;
	}

	// Term_message ("remaped indexes out of grey ramp");
	RestoreScreens(MIN_VIDEO_X, MIN_VIDEO_Y, MAX_VIDEO_X, MAX_VIDEO_Y);
	// Term_message ("setting grey ramp indexes back to picture greys");
	gr_pal_set_range(_GP(fadeToMe), GREY_START, NUM_GREYS);   // get the rest of the original re-luminance colors

	//recopy screenPicture to screen to restore original pixels
	krn_UnsetGreyVideoMode();
	RestoreScreens(0, 0, MAX_VIDEO_X, MAX_VIDEO_Y);

	memcpy(pal, _GP(picPal), sizeof(RGB8) * 256);

	ws_RefreshWoodscriptBuffer(_G(game_bgBuff)->get_buffer(), &(_G(currentSceneDef).depth_table[0]),
		_G(screenCodeBuff)->get_buffer(), (uint8 *)&_G(master_palette)[0], _G(inverse_pal)->get_ptr());
	_G(game_bgBuff)->release();
	_G(inverse_pal)->release();

	RestoreScreens(MIN_VIDEO_X, MIN_VIDEO_Y, MAX_VIDEO_X, MAX_VIDEO_Y);

	grey_fade(pal, fadeType, GREY_START, FREE_END, steps, delay);

	mem_free((char *)_GP(trick));
	mem_free((char *)_GP(fadeToMe));
	mem_free((char *)_GP(picPal));
	_G(kernel).fading_to_grey = false;
	_G(gameDrawBuff)->release();
	gr_pal_set(_G(master_palette));
}

void kernel_examine_inventory_object(const char *picName, RGB8 *pal, int steps, int delay,
	int32 x, int32 y, int32 triggerNum, const char *digiName, int32 digiTrigger) {

	remember_esc_key = GetSystemHotkey(KEY_ESCAPE);
	RemoveSystemHotkey(KEY_ESCAPE);

	interface_hide();

	exam_saved_hotspots = _G(currentSceneDef).hotspots;
	_G(currentSceneDef).hotspots = nullptr;

	_GP(myFadeTrigger) = kernel_trigger_create(triggerNum);

	krn_fade_to_grey(pal, steps, delay);

	_GP(seriesHash) = series_load(picName, -1, pal);                    // preload sprite so we can unload it
	gr_pal_set_range(pal, FREE_START, 197);                                  // set that series colors into VGA
	RestoreScreens(MIN_VIDEO_X, MIN_VIDEO_Y, MAX_VIDEO_X, MAX_VIDEO_Y);

	Buffer *grey_screen = _G(gameDrawBuff)->get_buffer();
	krn_SetGreyVideoMode(
		// Grey rectangle
		0, 0, MAX_VIDEO_X, screen_height(grey_screen) + _G(kernel).letter_box_y,

		// Color rectangle
		x, y, x + ws_get_sprite_width(_GP(seriesHash), 0) - 1, y + ws_get_sprite_height(_GP(seriesHash), 0) - 1);
	_G(gameDrawBuff)->release();

	// Play the sprite series as a loop
	int32 status;
	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
	_GP(seriesAnim8) = series_play_xy(picName, -1, FORWARD,
		x - game_buff_ptr->x1, y - game_buff_ptr->y1, 100, 0, 7, -1);

	if (digiName) {
		digi_play(digiName, 1, 255, digiTrigger);
	}

	player_set_commands_allowed(true);

	cycleEngines(_G(game_bgBuff)->get_buffer(),
		&(_G(currentSceneDef).depth_table[0]),
		_G(screenCodeBuff)->get_buffer(),
		(uint8 *)&_G(master_palette)[0],
		_G(inverse_pal)->get_ptr(), true);

	game_pause(true);

	_G(inverse_pal)->release();
	_G(game_bgBuff)->release();

	pauseEngines();
}

void kernel_examine_inventory_object(const char *picName, int steps, int delay,
		int32 x, int32 y, int32 triggerNum, const char *digiName, int32 digiTrigger) {
	kernel_examine_inventory_object(picName, _G(master_palette), steps, delay,
		x, y, triggerNum, digiName, digiTrigger);
}

void kernel_unexamine_inventory_object(RGB8 *pal, int steps, int delay) {
	if (!_GP(seriesAnim8) || _GP(seriesHash) < 0)
		return;

	player_set_commands_allowed(false);
	game_pause(false);
	unpauseEngines();

	terminateMachine(_GP(seriesAnim8));
	series_unload(_GP(seriesHash));
	_GP(seriesAnim8) = nullptr;
	_GP(seriesHash) = 0;

	Buffer *grey_screen = _G(gameDrawBuff)->get_buffer();
	krn_SetGreyVideoMode(0, 0, MAX_VIDEO_X, screen_height(grey_screen) + _G(kernel).letter_box_y, -1, -1, -1, -1);
	_G(gameDrawBuff)->release();

	krn_pal_game_task();

	krn_fade_from_grey(pal, steps, delay, TO_COLOR);

	krn_pal_game_task();

	// set in kernel_examine_inventory_object (above)
	kernel_trigger_dispatchx(_GP(myFadeTrigger));

	//	gr_pal_set(master_palette);

	RestoreScreens(0, 0, MAX_VIDEO_X, MAX_VIDEO_Y);

	_G(currentSceneDef).hotspots = _GP(exam_saved_hotspots);

	interface_show();
	AddSystemHotkey(KEY_ESCAPE, remember_esc_key);
}


// This is an inplace remap
// fadeToMe must already have been set up to correspond to the image on the screen
void remap_buffer_with_luminance_map(Buffer *src, int32 x1, int32 y1, int32 x2, int32 y2) {
	uint8 *ptr;
	int32 x, y;
	if ((!src) || (!src->data)) return;
	if ((x2 - x1 < 0) || (y2 - y1 < 0)) return;
	if (x2 - x1 + 1 > src->w) x2 = src->w - 1;
	if (y2 - y1 + 1 > src->h) y2 = src->h - 1;

	x2 -= x1;
	y2 -= y1;
	for (y = 0; y <= y2; y++) {
		ptr = &src->data[(y + y1) * src->stride + x1];
		for (x = 0; x <= x2; x++)								  // for each pixel in row

			// remap the greyed out pixel to the closest grey in GREY_START to GREY_END range
			// shift right 3, takes a 255 value and makes it out of 32 (the number of greys in reduced grey ramp)
			ptr[x] = (uint8)(GREY_START + (_GP(fadeToMe)[ptr[x]].g >> 3));	 // Use green instead of red cause we're having a green screen 

		if (!(y & 0xff)) {
			_G(digi).task();
			_G(midi).task();
		}
	}
}

void krn_SetGreyVideoMode(int32 grey_x1, int32 grey_y1, int32 grey_x2, int32 grey_y2, int32 color_x1, int32 color_y1, int32 color_x2, int32 color_y2) {
	_GP(greyAreaX1) = grey_x1;
	_GP(greyAreaY1) = grey_y1;
	_GP(greyAreaX2) = grey_x2;
	_GP(greyAreaY2) = grey_y2;

	_GP(colorAreaX1) = color_x1;
	_GP(colorAreaY1) = color_y1;
	_GP(colorAreaX2) = color_x2;
	_GP(colorAreaY2) = color_y2;

	_GP(greyVideoMode) = true;
}

void krn_UnsetGreyVideoMode() {
	_GP(greyAreaX1) = -1;
	_GP(greyAreaY1) = -1;
	_GP(greyAreaX2) = -1;
	_GP(greyAreaY2) = -1;

	_GP(colorAreaX1) = -1;
	_GP(colorAreaY1) = -1;
	_GP(colorAreaX2) = -1;
	_GP(colorAreaY2) = -1;

	_GP(greyVideoMode) = false;
}

bool krn_GetGreyMode() {
	return _GP(greyVideoMode);
}

void krn_UpdateGreyArea(Buffer *greyOutThisBuffer, int32 scrnX, int32 scrnY, int32 greyX1, int32 greyY1, int32 greyX2, int32 greyY2) {
	bool        finished;
	int32       x1, y1, x2, y2;

	if ((!_GP(greyVideoMode)) || (!greyOutThisBuffer) || (!greyOutThisBuffer->data)) {
		return;
	}
	x1 = imath_max(greyX1 + scrnX, _GP(greyAreaX1));
	y1 = imath_max(greyY1 + scrnY, _GP(greyAreaY1));
	x2 = imath_min(greyX2 + scrnX, _GP(greyAreaX2));
	y2 = imath_min(greyY2 + scrnY, _GP(greyAreaY2));
	if ((x1 > x2) || (y1 > y2)) return;
	finished = false;
	if (!finished) {
		if (y1 < _GP(colorAreaY1)) {
			remap_buffer_with_luminance_map(greyOutThisBuffer,
				x1 - scrnX, y1 - scrnY, x2 - scrnX, imath_min(y2, _GP(colorAreaY1) - 1) - scrnY);
			y1 = imath_min(y2, _GP(colorAreaY1));
			if (y1 >= y2) finished = true;
		}
	}
	if (!finished) {
		if (y2 > _GP(colorAreaY2)) {
			remap_buffer_with_luminance_map(greyOutThisBuffer,
				x1 - scrnX, imath_max(y1, _GP(colorAreaY2) + 1) - scrnY, x2 - scrnX, y2 - scrnY);
			y2 = imath_max(y1, _GP(colorAreaY2));
			if (y1 >= y2) finished = true;
		}
	}
	if (!finished) {
		if (x1 < _GP(colorAreaX1)) {
			remap_buffer_with_luminance_map(greyOutThisBuffer,
				x1 - scrnX, y1 - scrnY, imath_min(x2, _GP(colorAreaX1) - 1) - scrnX, y2 - scrnY);
			x1 = imath_min(x2, _GP(colorAreaX1));
			if (x1 >= x2) finished = true;
		}
	}
	if (!finished) {
		if (x2 > _GP(colorAreaX2)) {
			remap_buffer_with_luminance_map(greyOutThisBuffer,
				imath_max(x1, _GP(colorAreaX2) + 1) - scrnX, y1 - scrnY, x2 - scrnX, y2 - scrnY);
		}
	}
}

void krn_ChangeBufferLuminance(Buffer *target, int32 percent) {
	int32 x, y, r, g, b, i;
	uint8 *inverse_palette, *tempPtr;
	frac16 fracPercent;
	RGB8 *pal;
	uint8 luminancePal[256];

	// Paremeter verification
	if ((!target) || (!target->data)) {
		return;
	}
	if ((percent < 0) || (percent == 100)) {
		return;
	}

	if (percent == 0) {
		gr_color_set(__BLACK);
		gr_buffer_rect_fill(target, 0, 0, target->w, target->h);
		return;
	}

	// Calculate the frac16 form of the percent
	fracPercent = (percent * 255) / 100;

	// Get the palette and the inverse palette
	pal = &_G(master_palette)[0];
	inverse_palette = _G(inverse_pal)->get_ptr();
	if ((!pal) || (!inverse_palette)) {
		return;
	}

	// Calculate the luminance Pal table
	for (i = 0; i < 256; i++) {
		r = ((((pal[i].r * fracPercent) >> 10) >> 1)) & 0x1f;
		g = ((((pal[i].g * fracPercent) >> 10) >> 1)) & 0x1f;
		b = ((((pal[i].b * fracPercent) >> 10) >> 1)) & 0x1f;
		luminancePal[i] = inverse_palette[(r << 10) + (g << 5) + b];
	}

	// Note: this loop should be y0 to y1, x0 to x1, not a stride*h loop.
	// Loop through every pixel replacing it with the index into the luminance table
	tempPtr = target->data;
	for (y = 0; y < target->h; y++) {
		for (x = 0; x < target->stride; x++) {
			*tempPtr = luminancePal[*tempPtr];
			tempPtr++;
			//pixel = *tempPtr;
		}
	}

	_G(inverse_pal)->release();
}


static void pal_fade_callback(frac16 myMessage) {
	_G(pal_fade_in_progress) = false;
	kernel_trigger_dispatchx((int32)myMessage);
}

void pal_fade_init(RGB8 *origPalette, int32 firstPalEntry, int32 lastPalEntry,
		int32 targetPercent, int32 numTicks, int32 triggerNum) {
	if ((!origPalette) || (firstPalEntry < 0) || (lastPalEntry > 255) || (firstPalEntry > lastPalEntry))
		return;
	if ((targetPercent < 0) || (targetPercent > 100))
		return;

	_GP(myFadeReq) = true;
	_GP(myFadeFinished) = false;
	_GP(myFadeStartTime) = timer_read_60();
	_GP(myFadeEndDelayTime) = timer_read_60();
	_GP(myFadeStartIndex) = firstPalEntry;
	_GP(myFadeEndIndex) = lastPalEntry;
	_GP(myFadeEndTime) = _GP(myFadeStartTime) + numTicks;
	_GP(myFadeTrigger) = kernel_trigger_create(triggerNum);
	_GP(myFadeStartPercentFrac) = _GP(myFadeCurrPercentFrac);
	_GP(myFadePercentFrac) = DivSF16(targetPercent << 16, 100 << 16);

	// Disable_end_user_hot_keys();
	_G(pal_fade_in_progress) = true;
}

void pal_fade_init(int32 firstPalEntry, int32 lastPalEntry, int32 targetPercent, int32 numTicks, int32 triggerNum) {
	pal_fade_init(_G(master_palette), firstPalEntry, lastPalEntry, targetPercent, numTicks, triggerNum);
}

void disable_player_commands_and_fade_init(int trigger) {
	player_set_commands_allowed(false);
	pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, trigger);
}

static void pal_fade_update(RGB8 *origPalette) {
	int32 i, currTime;
	frac16	tempFrac, tempFrac2;

	currTime = timer_read_60();

	if (currTime >= _GP(myFadeEndDelayTime)) {		// If the delay has expired, fade more
		if (currTime >= _GP(myFadeEndTime)) {
			tempFrac2 = _GP(myFadePercentFrac);
			_GP(myFadeStartPercentFrac) = _GP(myFadePercentFrac);
			_GP(myFadeFinished) = true;
		} else if (currTime <= _GP(myFadeStartTime)) {
			return;
		} else {
			tempFrac = DivSF16((currTime - _GP(myFadeStartTime)) << 16, (_GP(myFadeEndTime) - _GP(myFadeStartTime)) << 16);
			tempFrac2 = MulSF16(tempFrac, _GP(myFadePercentFrac) - _GP(myFadeStartPercentFrac)) + _GP(myFadeStartPercentFrac);
		}

		_GP(myFadeCurrPercentFrac) = tempFrac2;

		for (i = _GP(myFadeStartIndex); i <= _GP(myFadeEndIndex); i++) {
			_GP(myFXPalette)[i].r = (Byte)(MulSF16(origPalette[i].r << 16, tempFrac2) >> 16);
			_GP(myFXPalette)[i].g = (Byte)(MulSF16(origPalette[i].g << 16, tempFrac2) >> 16);
			_GP(myFXPalette)[i].b = (Byte)(MulSF16(origPalette[i].b << 16, tempFrac2) >> 16);
		}

		// Recalculate the end delay time again
		_GP(myFadeEndDelayTime) = currTime + _GP(myFadeDelayTicks);		// Recalculate the end delay time again

		// Must reresh the DAC
		_GP(myFadeDACrefresh) = true;
	}
}

void clear_DAC() {
	RGB8 color;

	color.r = color.b = color.g = 0;
	for (int i = 0; i < 256; i++)
		gr_pal_set_entry(i, &color);
}

void pal_fade_set_start(RGB8 *origPalette, int32 percent) {
	pal_fade_init(origPalette, _G(kernel).first_fade, 255, percent, 0, (uint)-1);
	pal_fade_update(origPalette);
	pal_fx_update();
}

void pal_fade_set_start(int32 percent) {
	pal_fade_set_start(_G(master_palette), percent);
}

static void pal_cycle_callback(frac16 myMessage) {
	kernel_trigger_dispatchx((uint32)myMessage);
}

void pal_cycle_init(int32 firstPalEntry, int32 lastPalEntry,
		int32 delayTicks, int32 totalTicks, int32 triggerNum) {
	// Validation
	if ((firstPalEntry < 0) || (lastPalEntry > 255) || (firstPalEntry > lastPalEntry))
		return;
	if (delayTicks <= 0)
		return;

	_GP(myCycleReq) = true;
	_GP(myCycleFinished) = false;
	_GP(myCycleDelayTicks) = delayTicks;
	_GP(myCycleStartTime) = timer_read_60();
	_GP(myCycleEndDelayTime) = timer_read_60();
	_GP(myCycleStartIndex) = firstPalEntry;
	_GP(myCycleEndIndex) = lastPalEntry;
	_GP(myCycleTrigger) = kernel_trigger_create(triggerNum);		// Returned when myCycleEndTime is reached

	if (totalTicks > 0) {											// If totalTicks > 0, calculate end time
		_GP(myCycleEndTime) = _GP(myCycleStartTime) + totalTicks;
		_GP(myCycleNeverStopCycling) = false;

	} else if (totalTicks < 0) {		  							// If totalTicks < 0, never stop the cycling
		_GP(myCycleNeverStopCycling) = true;

	} else {		  												// If totalTicks is 0, stop cycling now
		_GP(myCycleReq) = false;
		_GP(myCycleFinished) = true;
	}
}

bool pal_cycle_active() {
	return _GP(myCycleReq);
}

void pal_cycle_stop() {
	_GP(myCycleReq) = false;
}

void pal_cycle_resume() {
	_GP(myCycleReq) = true;
}

static void pal_cycle_update() {
	int32 i, currTime;
	RGB8 firstColour;

	currTime = timer_read_60();		// Get current time

	if (_GP(myCycleNeverStopCycling) == false) { 			// If there is an end time to get to...  

		if (currTime >= _GP(myCycleEndTime)) {		  		// See if we have reached it
			_GP(myCycleFinished) = true;					// Mark cycling as finished
			return;											// Return 
		}
	} else {
		// See if we should colour cycle right now
		if (currTime >= _GP(myCycleEndDelayTime)) {			// If the delay has expired, colour cycle
			// Cycle the master palette 
			firstColour.r = _G(master_palette)[_GP(myCycleStartIndex)].r;		// Remember first colour
			firstColour.g = _G(master_palette)[_GP(myCycleStartIndex)].g;
			firstColour.b = _G(master_palette)[_GP(myCycleStartIndex)].b;
			for (i = _GP(myCycleStartIndex); i < _GP(myCycleEndIndex); ++i) {	// Shift colours down one in palette
				_G(master_palette)[i].r = _G(master_palette)[i + 1].r;
				_G(master_palette)[i].g = _G(master_palette)[i + 1].g;
				_G(master_palette)[i].b = _G(master_palette)[i + 1].b;
			}
			_G(master_palette)[_GP(myCycleEndIndex)].r = firstColour.r; 		// Set last colour to the first colour
			_G(master_palette)[_GP(myCycleEndIndex)].g = firstColour.g;
			_G(master_palette)[_GP(myCycleEndIndex)].b = firstColour.b;

			// Then cycle the FX palette 
			firstColour.r = _GP(myFXPalette)[_GP(myCycleStartIndex)].r;			// Remember first colour
			firstColour.g = _GP(myFXPalette)[_GP(myCycleStartIndex)].g;
			firstColour.b = _GP(myFXPalette)[_GP(myCycleStartIndex)].b;
			for (i = _GP(myCycleStartIndex); i < _GP(myCycleEndIndex); ++i) {	// Shift colours down one in palette
				_GP(myFXPalette)[i].r = _GP(myFXPalette)[i + 1].r;
				_GP(myFXPalette)[i].g = _GP(myFXPalette)[i + 1].g;
				_GP(myFXPalette)[i].b = _GP(myFXPalette)[i + 1].b;
			}
			_GP(myFXPalette)[_GP(myCycleEndIndex)].r = firstColour.r; 			// Set last colour to the first colour
			_GP(myFXPalette)[_GP(myCycleEndIndex)].g = firstColour.g;
			_GP(myFXPalette)[_GP(myCycleEndIndex)].b = firstColour.b;


			// Recalculate the end delay time again
			_GP(myCycleEndDelayTime) = currTime + _GP(myCycleDelayTicks);		// Recalculate the end delay time again

			// must reresh the DAC
			_GP(myCycleDACrefresh) = true;
		}
	}
}

void pal_fx_update() {
	int32 startA = 0, endA = 0, startB = 0, endB = 0, startDAC = 0, endDAC = 0;

	if (!_GP(myCycleReq) && !_GP(myFadeReq))
		// Crap out quickly if no effects required
		return;

	// Perform any effect required and track index ranges
	if (_GP(myCycleReq)) {
		pal_cycle_update();						// Do the cycling	(cycles master_palette and _GP(myFXPalette))
		if (_GP(myCycleDACrefresh)) {	  		// If it needs the DAC to be refreshed, 
			startA = _GP(myCycleStartIndex);	// remember the range
			endA = _GP(myCycleEndIndex);
			_GP(myCycleDACrefresh) = false;
		}
	}

	if (_GP(myFadeReq)) {
		pal_fade_update(&_G(master_palette)[0]);	// Do the fading (sets myFXPalette to faded master_palette)
		if (_GP(myFadeDACrefresh)) {	  			// If it needs the DAC to be refreshed, 
			startB = _GP(myFadeStartIndex);			// remember the range
			endB = _GP(myFadeEndIndex);
			_GP(myFadeDACrefresh) = false;
		}
	}

	// Check ranges to perform minimum calls of gr_pal_set_range().
	// This was originally done to minimize snow on monitor due to repeated OUT instructions
	if (endA < startB || endB < startA) {
		// if A and B ranges don't overlap
		if (!(startA == 0 && endA == 0))    // if this is not the degenerate case	(just the transparent color)
			gr_pal_set_range(&_GP(myFXPalette)[0], startA, endA - startA + 1);	// set A range of the DAC

		if (!(startB == 0 && endB == 0)) // if this is not the degenerate case	(just the transparent color)
			gr_pal_set_range(&_GP(myFXPalette)[0], startB, endB - startB + 1);	// set B range of the DAC

	} else {
		// They overlap, so find the extent of the overlap
		(startA < startB) ? (startDAC = startA) : (startDAC = startB);	 	//  which start is less
		(endA > endB) ? (endDAC = endA) : (endDAC = endB);  	 	//  which end is more

		if (!(startDAC == 0 && endDAC == 0)) // if this is not the degenerate case	(just the transparent color)
			gr_pal_set_range(&_GP(myFXPalette)[0], startDAC, endDAC - startDAC + 1);	// set the whole range of the DAC
	}

	// Turn off flags and call callbacks if effects are finished
	if (_GP(myFadeReq) && _GP(myFadeFinished)) {
		_GP(myFadeReq) = false;
		pal_fade_callback(_GP(myFadeTrigger));
	}

	if (_GP(myCycleReq) && _GP(myCycleFinished)) {
		_GP(myCycleReq) = false;
		pal_cycle_callback(_GP(myCycleTrigger));
	}
}

void DAC_tint_range(const RGB8 *tintColor, int32 percent, int32 firstPalEntry, int32 lastPalEntry, bool transparent) {
	int32 i;
	int32 r, g, b, dr, dg, db;
	RGB8 color, targetColor;
	int32 percent_r, percent_g, percent_b;

	if ((firstPalEntry < 0) || (lastPalEntry > 255) || (firstPalEntry > lastPalEntry)) {
		// This should generate an error
		term_message("*** palette index error");
		return;
	}

	term_message("Colour tint DAC to: %d %d %d, %d percent, range (%d - %d)",
		tintColor->r, tintColor->g, tintColor->b, percent, firstPalEntry, lastPalEntry);
	percent = DivSF16(percent << 16, 100 << 16); // convert percent to frac16 format

	targetColor.r = tintColor->r;
	targetColor.g = tintColor->g;
	targetColor.b = tintColor->b;

	term_message("Doing palette.....");

	if (!transparent) {
		for (i = firstPalEntry; i <= lastPalEntry; ++i) {

			// Calculate deltas for RGB's  and put them in frac16 format
			dr = (targetColor.r - _G(master_palette)[i].r) << 16;
			dg = (targetColor.g - _G(master_palette)[i].g) << 16;
			db = (targetColor.b - _G(master_palette)[i].b) << 16;

			// New = orig + (delta * percent)
			r = _G(master_palette)[i].r + (MulSF16(percent, dr) >> 16);
			g = _G(master_palette)[i].g + (MulSF16(percent, dg) >> 16);
			b = _G(master_palette)[i].b + (MulSF16(percent, db) >> 16);

			// Check for under/overflow
			if (r > 255) r = 255;
			if (r < 0) r = 0;
			if (g > 255) g = 255;
			if (g < 0) g = 0;
			if (b > 255) b = 255;
			if (b < 0) b = 0;

			color.r = (byte)r;
			color.g = (byte)g;
			color.b = (byte)b;

			gr_pal_set_entry(i, &color);	// Set the new colour to the DAC
		}

	} else {
		// This is for filtering colors.  For example, a completely red filter
		// (255, 0, 0) will block out the blue and green parts of the palette.
		// 50% of the same filter will block out only 50% of the blue and
		// green, but leaving all of the rest blue.
		for (i = firstPalEntry; i <= lastPalEntry; ++i) {
			// Converting rgb to a frac16 ( << 16) dividing by 256 ( >> 8) 
			// (the range of the palette values)
			percent_r = (targetColor.r) << 8;
			percent_g = (targetColor.g) << 8;
			percent_b = (targetColor.b) << 8;

			// This is the difference between the color and the full effect
			// of the filter at 100%, as a frac16.
			dr = (_G(master_palette)[i].r << 16) - (MulSF16(percent_r, _G(master_palette)[i].r << 16));
			dg = (_G(master_palette)[i].g << 16) - (MulSF16(percent_g, _G(master_palette)[i].g << 16));
			db = (_G(master_palette)[i].b << 16) - (MulSF16(percent_b, _G(master_palette)[i].b << 16));

			// Scaling the effect to the right percentage.  This is a frac16.
			dr = MulSF16(dr, percent);
			dg = MulSF16(dg, percent);
			db = MulSF16(db, percent);

			// Subtract the result to palette.
			r = (_G(master_palette)[i].r - (dr >> 16));
			g = (_G(master_palette)[i].g - (dg >> 16));
			b = (_G(master_palette)[i].b - (db >> 16));

			// Check for under/overflow
			if (r > 255) r = 255;
			if (r < 0) r = 0;
			if (g > 255) g = 255;
			if (g < 0) g = 0;
			if (b > 255) b = 255;
			if (b < 0) b = 0;

			color.r = (byte)r;
			color.g = (byte)g;
			color.b = (byte)b;

			gr_pal_set_entry(i, &color); // Set new colors to DAC.
		}
	}
}

void DAC_restore() {
	term_message("DAC restored");
	gr_pal_set_range(&_G(master_palette)[0], 0, 256);
}

} // namespace M4
