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

// "I Have No Mouth" Intro sequence scene procedures

#include "saga.h"
#include "yslib.h"

#include "reinherit.h"

#include "animation_mod.h"
#include "cvar_mod.h"
#include "events_mod.h"
#include "font_mod.h"
#include "rscfile_mod.h"
#include "scene_mod.h"
#include "text_mod.h"
#include "palanim_mod.h"

#include "scene.h"
#include "ihnm_introproc.h"

namespace Saga {

R_SCENE_RESLIST IHNM_IntroMovie1RL[] = {
	{30, SAGA_BG_IMAGE, 0, 0} ,
	{31, SAGA_ANIM_1, 0, 0}
};

R_SCENE_DESC IHNM_IntroMovie1Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie1RL,
	ARRAYSIZE(IHNM_IntroMovie1RL)
};

R_SCENE_RESLIST IHNM_IntroMovie2RL[] = {
	{32, SAGA_BG_IMAGE, 0, 0} ,
	{33, SAGA_ANIM_1, 0, 0}
};

R_SCENE_DESC IHNM_IntroMovie2Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie2RL,
	ARRAYSIZE(IHNM_IntroMovie2RL)
};

R_SCENE_RESLIST IHNM_IntroMovie3RL[] = {
	{34, SAGA_BG_IMAGE, 0, 0},
	{35, SAGA_ANIM_1, 0, 0}
};

R_SCENE_DESC IHNM_IntroMovie3Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie3RL,
	ARRAYSIZE(IHNM_IntroMovie3RL)
};

R_SCENE_RESLIST IHNM_IntroMovie4RL[] = {
	{1227, SAGA_BG_IMAGE, 0, 0},
	{1226, SAGA_ANIM_1, 0, 0}
};

R_SCENE_DESC IHNM_IntroMovie4Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie4RL,
	ARRAYSIZE(IHNM_IntroMovie4RL)
};

R_SCENE_QUEUE IHNM_IntroList[] = {
	{0, &IHNM_IntroMovie1Desc, BY_DESC, IHNM_IntroMovieProc1, 0} ,
	{0, &IHNM_IntroMovie2Desc, BY_DESC, IHNM_IntroMovieProc2, 0} ,
	{0, &IHNM_IntroMovie3Desc, BY_DESC, IHNM_IntroMovieProc3, 0} ,
	{0, &IHNM_IntroMovie4Desc, BY_DESC, IHNM_HateProc, 0}
};

int IHNM_StartProc() {
	size_t n_introscenes;
	size_t i;

	n_introscenes = ARRAYSIZE(IHNM_IntroList);

	for (i = 0; i < n_introscenes; i++) {
		SCENE_Queue(&IHNM_IntroList[i]);
	}

	return R_SUCCESS;
}

int IHNM_IntroMovieProc1(int param, R_SCENE_INFO *scene_info) {
	R_EVENT event;
	YS_IGNORE_PARAM(scene_info);

	switch (param) {
	case SCENE_BEGIN:
		// Background for intro scene is the first frame of the
		// intro animation; display it and set the palette
		event.type = R_ONESHOT_EVENT;
		event.code = R_BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = SET_PALETTE;
		event.time = 0;
		EVENT_Queue(&event);
		ANIM_SetFrameTime(0, R_IHNM_INTRO_FRAMETIME);
		ANIM_SetFlag(0, ANIM_ENDSCENE);
		ANIM_Play(0, 0);
		break;
	default:
		break;
	}

	return 0;
}

int IHNM_IntroMovieProc2(int param, R_SCENE_INFO *scene_info) {
	R_EVENT event;
	R_EVENT *q_event;
	PALENTRY *pal;

	static PALENTRY current_pal[R_PAL_ENTRIES];
	YS_IGNORE_PARAM(scene_info);
	switch (param) {

	case SCENE_BEGIN:
		// Fade to black out of the intro CyberDreams logo anim
		SYSGFX_GetCurrentPal(current_pal);

		event.type = R_CONTINUOUS_EVENT;
		event.code = R_PAL_EVENT;
		event.op = EVENT_PALTOBLACK;
		event.time = 0;
		event.duration = R_IHNM_PALFADE_TIME;
		event.data = current_pal;

		q_event = EVENT_Queue(&event);

		// Background for intro scene is the first frame of the
		// intro animation; display it but don't set palette
		event.type = R_ONESHOT_EVENT;
		event.code = R_BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = NO_SET_PALETTE;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		// Fade in from black to the scene background palette
		SCENE_GetBGPal(&pal);

		event.type = R_CONTINUOUS_EVENT;
		event.code = R_PAL_EVENT;
		event.op = EVENT_BLACKTOPAL;
		event.time = 0;
		event.duration = R_IHNM_PALFADE_TIME;
		event.data = pal;

		q_event = EVENT_Chain(q_event, &event);

		ANIM_SetFlag(0, ANIM_LOOP);
		ANIM_Play(0, R_IHNM_PALFADE_TIME * 2);

		// Queue end of scene after looping animation for a while
		event.type = R_ONESHOT_EVENT;
		event.code = R_SCENE_EVENT;
		event.op = EVENT_END;
		event.time = R_IHNM_DGLOGO_TIME;

		q_event = EVENT_Chain(q_event, &event);
		break;
	default:
		break;
	}

	return 0;
}

int IHNM_IntroMovieProc3(int param, R_SCENE_INFO *scene_info) {
	R_EVENT event;
	R_EVENT *q_event;
	PALENTRY *pal;
	static PALENTRY current_pal[R_PAL_ENTRIES];
	YS_IGNORE_PARAM(scene_info);

	switch (param) {
	case SCENE_BEGIN:
		// Fade to black out of the intro DG logo anim
		SYSGFX_GetCurrentPal(current_pal);

		event.type = R_CONTINUOUS_EVENT;
		event.code = R_PAL_EVENT;
		event.op = EVENT_PALTOBLACK;
		event.time = 0;
		event.duration = R_IHNM_PALFADE_TIME;
		event.data = current_pal;

		q_event = EVENT_Queue(&event);

		// Background for intro scene is the first frame of the
		// intro animation; display it but don't set palette
		event.type = R_ONESHOT_EVENT;
		event.code = R_BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = NO_SET_PALETTE;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		// Fade in from black to the scene background palette
		SCENE_GetBGPal(&pal);

		event.type = R_CONTINUOUS_EVENT;
		event.code = R_PAL_EVENT;
		event.op = EVENT_BLACKTOPAL;
		event.time = 0;
		event.duration = R_IHNM_PALFADE_TIME;
		event.data = pal;

		q_event = EVENT_Chain(q_event, &event);

		ANIM_Play(0, 0);

		// Queue end of scene after a while
		event.type = R_ONESHOT_EVENT;
		event.code = R_SCENE_EVENT;
		event.op = EVENT_END;
		event.time = R_IHNM_TITLE_TIME;

		q_event = EVENT_Chain(q_event, &event);
		break;
	default:
		break;
	}

	return 0;
}

int IHNM_HateProc(int param, R_SCENE_INFO *scene_info) {
	R_EVENT event;
	R_EVENT *q_event;
	YS_IGNORE_PARAM(scene_info);

	switch (param) {
	case SCENE_BEGIN:
		// Background for intro scene is the first frame of the
		// intro animation; display it and set the palette
		event.type = R_ONESHOT_EVENT;
		event.code = R_BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = SET_PALETTE;
		event.time = 0;

		q_event = EVENT_Queue(&event);

		ANIM_SetFlag(0, ANIM_LOOP);
		ANIM_Play(0, 0);
		break;
	default:
		break;
	}

	return 0;
}

} // End of namespace Saga
