/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Initial and default scene procedures

#include "saga.h"
#include "reinherit.h"

#include "yslib.h"

#include "animation_mod.h"
#include "events_mod.h"
#include "scene_mod.h"
#include "palanim_mod.h"
#include "sound.h"
#include "music.h"

#include "scene.h"
#include "sceneproc.h"

namespace Saga {

int InitialSceneProc(int param, R_SCENE_INFO *scene_info) {
	R_EVENT event;
	R_EVENT *q_event;
	int delay_time = 0;
	static PALENTRY current_pal[R_PAL_ENTRIES];
	PALENTRY *pal;

	YS_IGNORE_PARAM(scene_info);

	switch (param) {
	case SCENE_BEGIN:
		_vm->_music->stop();
		_vm->_sound->stopVoice();

		// Fade palette to black from intro scene
		GFX_GetCurrentPal(current_pal);

		event.type = R_CONTINUOUS_EVENT;
		event.code = R_PAL_EVENT;
		event.op = EVENT_PALTOBLACK;
		event.time = 0;
		event.duration = PALETTE_FADE_DURATION;
		event.data = current_pal;

		delay_time += PALETTE_FADE_DURATION;

		q_event = EVENT_Queue(&event);

		// Activate user interface
		event.type = R_ONESHOT_EVENT;
		event.code = R_INTERFACE_EVENT;
		event.op = EVENT_ACTIVATE;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		// Set first scene background w/o changing palette
		event.type = R_ONESHOT_EVENT;
		event.code = R_BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = NO_SET_PALETTE;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		// Fade in to first scene background palette
		SCENE_GetBGPal(&pal);

		event.type = R_CONTINUOUS_EVENT;
		event.code = R_PAL_EVENT;
		event.op = EVENT_BLACKTOPAL;
		event.time = delay_time;
		event.duration = PALETTE_FADE_DURATION;
		event.data = pal;

		q_event = EVENT_Chain(q_event, &event);

		event.code = R_PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		ANIM_SetFlag(0, ANIM_LOOP);
		ANIM_Play(0, delay_time);
		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int DefaultSceneProc(int param, R_SCENE_INFO *scene_info) {
	R_EVENT event;

	YS_IGNORE_PARAM(scene_info);

	switch (param) {
	case SCENE_BEGIN:
		// Set scene background
		event.type = R_ONESHOT_EVENT;
		event.code = R_BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = SET_PALETTE;
		event.time = 0;

		EVENT_Queue(&event);

		// Activate user interface
		event.type = R_ONESHOT_EVENT;
		event.code = R_INTERFACE_EVENT;
		event.op = EVENT_ACTIVATE;
		event.time = 0;

		EVENT_Queue(&event);

		// Begin palette cycle animation if present
		event.type = R_ONESHOT_EVENT;
		event.code = R_PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		EVENT_Queue(&event);
		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

} // End of namespace Saga

