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

#ifndef M4_GRAPHICS_KRN_PAL_H
#define M4_GRAPHICS_KRN_PAL_H

#include "m4/m4_types.h"
#include "m4/adv_r/adv_hotspot.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {

#define TO_GREY            (int32)0
#define TO_COLOR           (int32)1
#define TO_BLACK           (int32)2

struct KernelPal_Globals {
	RGB8 _myFXPalette[256];

	bool _myCycleReq = false;
	bool _myCycleFinished = true;
	bool _myCycleDACrefresh = false;
	int32 _myCycleDelayTicks = 6; // 10 times a second
	int32 _myCycleStartTime = 0;
	int32 _myCycleEndTime = 0;
	int32 _myCycleEndDelayTime = 0;
	int32 _myCycleStartIndex = 0;
	int32 _myCycleEndIndex = 0;
	int32 _myCycleTrigger = 0;
	int32 _myCycleNeverStopCycling = false;

	bool _myFadeReq = false;
	bool _myFadeFinished = true;
	bool _myFadeDACrefresh = false;
	int32 _myFadeDelayTicks = 3; // 20 times a second
	int32 _myFadeStartTime = 0;
	int32 _myFadeEndTime = 0;
	int32 _myFadeEndDelayTime = 0;
	int32 _myFadeStartIndex = 0;
	int32 _myFadeEndIndex = 0;
	int32 _myFadeTrigger = 0;
	frac16 _myFadeStartPercentFrac = 0x10000;
	frac16 _myFadeCurrPercentFrac = 0x10000;
	frac16 _myFadePercentFrac = 0;

	HotSpotRec *_exam_saved_hotspots = nullptr;

	RGB8 *_fadeToMe = nullptr;
	RGB8 *_trick = nullptr;
	RGB8 *_picPal = nullptr;
	int32 _seriesHash = 0;
	machine *_seriesAnim8 = nullptr;
	uint8 _translation[32];		// Only 32 greys in ramp

	int32 _colorAreaX1 = -1;
	int32 _colorAreaY1 = -1;
	int32 _colorAreaX2 = -1;
	int32 _colorAreaY2 = -1;

	int32 _greyAreaX1 = -1;
	int32 _greyAreaY1 = -1;
	int32 _greyAreaX2 = -1;
	int32 _greyAreaY2 = -1;

	bool _greyVideoMode = false;
};

void pal_fade_set_start(RGB8 *origPalette, int32 percent);
void pal_fade_set_start(int32 percent);
void pal_fade_init(RGB8 *origPalette, int32 firstPalEntry, int32 lastPalEntry, int32 targetPercent, int32 numTicks, int32 triggerNum);
void pal_fade_init(int32 firstPalEntry, int32 lastPalEntry, int32 targetPercent, int32 numTicks, int32 triggerNum);

void disable_player_commands_and_fade_init(int trigger);
void pal_cycle_init(int32 firstPalEntry, int32 lastPalEntry, int32 delayTicks,
	int32 totalTicks = -1, int32 triggerNum = -1);

/**
 * Returns true if color cycling is on
 */
bool pal_cycle_active();

/**
 * Stops color cycling
 */
void pal_cycle_stop();

/**
 * Starts color cycling
 */
void pal_cycle_resume();

/**
 * Handles fading and cycling
 */
void pal_fx_update();

/**
 * This is used to effect the screen colours (not the master palette) temporarily
 * until something else updates the DAC e.g. refresh_DAC()
 */
void DAC_tint_range(const RGB8 *tintColor, int32 percent, int32 firstPalEntry, int32 lastPalEntry, bool transparent);

void kernel_examine_inventory_object(const char *picName, RGB8 *pal,
	int steps, int delay, int32 x, int32 y, int32 trigger,
	const char *digiName, int32 digiTrigger);
void kernel_examine_inventory_object(const char *picName, int steps, int delay,
	int32 x, int32 y, int32 triggerNum, const char *digiName = nullptr, int32 digiTrigger = -1);
void kernel_unexamine_inventory_object(RGB8 *pal, int steps, int delay);

void remap_buffer_with_luminance_map(Buffer *src, int32 x1, int32 y1, int32 x2, int32 y2);
void krn_SetGreyVideoMode(int32 grey_x1, int32 grey_y1, int32 grey_x2, int32 grey_y2, int32 color_x1, int32 color_y1, int32 color_x2, int32 color_y2);
void krn_UnsetGreyVideoMode(void);
bool krn_GetGreyMode(void);
void krn_UpdateGreyArea(Buffer *greyOutThisBuffer, int32 scrnX, int32 scrnY,
	int32 greyX1, int32 greyY1, int32 greyX2, int32 greyY2);
void krn_ChangeBufferLuminance(Buffer *target, int32 percent);

void krn_pal_game_task();
void krn_fade_from_grey(RGB8 *pal, int32 steps, int32 delay, int32 fadeType);
void krn_fade_to_grey(RGB8 *pal, int32 steps, int32 delay);

} // namespace M4

#endif
