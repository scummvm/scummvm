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

#include "engines/grim/imuse/dimuse_engine.h"

namespace Grim {

// We have some uintptr arguments as commands 17, 18, 19 and 29 actually require pointer arguments
// Unfortunately this makes function calls for other command a little less pretty...
int IMuseDigital::cmdsHandleCmd(int cmd, uint8 *ptr, int a, int b, int c, int d, int e,
	int f, int g, int h, int i, int j, int k, int l, int m, int n) {

	char *marker = (char *)ptr;

	if (!_initialized)
		return -1;

	switch (cmd) {
	case DIMUSE_C_INIT:
		return diMUSEInitialize();
	case DIMUSE_C_TERMINATE:
		return diMUSETerminate();
	case 2:
		return cmd;
	case DIMUSE_C_PAUSE:
		diMUSEPause();
		return cmd;
	case DIMUSE_C_RESUME:
		diMUSEResume();
		return cmd;
	case DIMUSE_C_SET_GRP_VOL:
		return diMUSESetGroupVol(a, b);
	case DIMUSE_C_START_SND:
		return diMUSEStartSound(a, b);
	case DIMUSE_C_STOP_SND:
		return diMUSEStopSound(a);
	case DIMUSE_C_STOP_ALL_SNDS:
		return diMUSEStopAllSounds();
	case DIMUSE_C_GET_NEXT_SND:
		return diMUSEGetNextSound(a);
	case DIMUSE_C_SET_PARAM:
		return diMUSESetParam(a, b, c);
	case DIMUSE_C_GET_PARAM:
		return diMUSEGetParam(a, b);
	case DIMUSE_C_FADE_PARAM:
		return diMUSEFadeParam(a, b, c, d);
	case DIMUSE_C_SET_HOOK:
		return diMUSESetHook(a, b);
	case DIMUSE_C_GET_HOOK:
		return diMUSEGetHook(a);
	case DIMUSE_C_SET_TRIGGER:
		return diMUSESetTrigger(a, marker, c, d, e, f, g, h, i, j, k, l, m, n);
	case DIMUSE_C_CHECK_TRIGGER:
		return diMUSECheckTrigger(a, marker, c);
	case DIMUSE_C_CLEAR_TRIGGER:
		return diMUSEClearTrigger(a, marker, c);
	case DIMUSE_C_DEFER_CMD:
		return diMUSEDeferCommand(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
	case DIMUSE_C_START_STREAM:
		return diMUSEStartStream(a, b, c);
	case DIMUSE_C_SWITCH_STREAM:
		return diMUSESwitchStream(a, b, c, d, e);
	case DIMUSE_C_PROCESS_STREAMS:
		return diMUSEProcessStreams();
	case DIMUSE_C_FEED_STREAM:
		return diMUSEFeedStream(a, ptr, c, d);
	default:
		return -1;
	}
}

} // End of namespace Grim
