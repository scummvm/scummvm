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
	int32 _myCycleStartIndex;
	int32 _myCycleEndIndex;
	uint32 _myCycleTrigger = 0;
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
	uint32 _myFadeTrigger = 0;
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

extern void pal_fade_set_start(RGB8 *origPalette, int32 percent);
extern void pal_fade_init(RGB8 *origPalette, int32 firstPalEntry, int32 lastPalEntry, int32 targetPercent, int32 numTicks, int32 triggerNum);

extern void pal_cycle_init(int32 firstPalEntry, int32 lastPalEntry, int32 delayTicks, int32 totalTicks, int32 triggerNum);
extern bool pal_cycle_active();        // Returns true if color cycling is on
extern void pal_cycle_stop();          // Stops color cycling
extern void pal_cycle_resume();        // Starts color cycling
extern void pal_fx_update();			// Handles fading and cycling  MattP

extern void kernel_examine_inventory_object(char *picName, RGB8 *pal, int steps, int delay,
	int32 x, int32 y, int32 triggerNum, char *digi_name, int32 digi_trigger);
extern void kernel_unexamine_inventory_object(RGB8 *pal, int steps, int delay);

extern void remap_buffer_with_luminance_map(Buffer *src, int32 x1, int32 y1, int32 x2, int32 y2);
extern void krn_SetGreyVideoMode(int32 grey_x1, int32 grey_y1, int32 grey_x2, int32 grey_y2, int32 color_x1, int32 color_y1, int32 color_x2, int32 color_y2);
extern void krn_UnsetGreyVideoMode(void);
extern bool krn_GetGreyMode(void);
extern void krn_UpdateGreyArea(Buffer *greyOutThisBuffer, int32 scrnX, int32 scrnY,
	int32 greyX1, int32 greyY1, int32 greyX2, int32 greyY2);
extern void krn_ChangeBufferLuminance(Buffer *target, int32 percent);

extern void krn_pal_game_task();

} // namespace M4

#endif
