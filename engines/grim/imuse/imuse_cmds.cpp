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

#include "engines/grim/imuse/imuse_engine.h"

namespace Grim {

// We have some uintptr arguments as commands 17, 18, 19 and 29 actually require pointer arguments
// Unfortunately this makes function calls for other command a little less pretty...
int IMuse::imSoundCall(int cmd, uint8 *ptr, int a, int b, int c, int d, int e,
	int f, int g, int h, int i, int j, int k, int l, int m, int n) {

	char *marker = (char *)ptr;

	if (!_initialized)
		return -1;

	switch (cmd) {
	case IMUSE_C_INIT:
		return imInitialize();
	case IMUSE_C_TERMINATE:
		return imTerminate();
	case 2:
		return cmd;
	case IMUSE_C_PAUSE:
		imPause();
		return cmd;
	case IMUSE_C_RESUME:
		imResume();
		return cmd;
	case IMUSE_C_SET_GRP_VOL:
		return imSetGroupVol(a, b);
	case IMUSE_C_START_SND:
		return imStartSound(a, b);
	case IMUSE_C_STOP_SND:
		return imStopSound(a);
	case IMUSE_C_STOP_ALL_SNDS:
		return imStopAllSounds();
	case IMUSE_C_GET_NEXT_SND:
		return imGetNextSound(a);
	case IMUSE_C_SET_PARAM:
		return imSetParam(a, b, c);
	case IMUSE_C_GET_PARAM:
		return imGetParam(a, b);
	case IMUSE_C_FADE_PARAM:
		return imFadeParam(a, b, c, d);
	case IMUSE_C_SET_HOOK:
		return imSetHook(a, b);
	case IMUSE_C_GET_HOOK:
		return imGetHook(a);
	case IMUSE_C_SET_TRIGGER:
		return imSetTrigger(a, marker, c, d, e, f, g, h, i, j, k, l, m, n);
	case IMUSE_C_CHECK_TRIGGER:
		return imCheckTrigger(a, marker, c);
	case IMUSE_C_CLEAR_TRIGGER:
		return imClearTrigger(a, marker, c);
	case IMUSE_C_DEFER_CMD:
		return imDeferCommand(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
	case IMUSE_C_START_STREAM:
		return imStartStream(a, b, c);
	case IMUSE_C_SWITCH_STREAM:
		return imSwitchStream(a, b, c, d, e);
	case IMUSE_C_PROCESS_STREAMS:
		return imProcessStreams();
	case IMUSE_C_FEED_STREAM:
		return imFeedStream(a, ptr, c, d);
	default:
		return -1;
	}
}

} // End of namespace Grim
