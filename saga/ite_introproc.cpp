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
/*
 Description:   
 
    Intro sequence scene procedures

 Notes: 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yslib.h"

#include "reinherit.h"

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "animation_mod.h"
#include "cvar_mod.h"
#include "events_mod.h"
#include "font_mod.h"
#include "game_mod.h"
#include "rscfile_mod.h"
#include "scene_mod.h"
#include "sndres_mod.h"
#include "text_mod.h"
#include "palanim_mod.h"

/*
 * Begin module:
\*--------------------------------------------------------------------------*/
#include "scene.h"
#include "ite_introproc.h"

namespace Saga {

static R_INTRO_DIALOGUE IntroDiag[] = {

	{
		    CAVE_VOICE_0, "intro1a",
	    "We see the sky, we see the land, we see the water, "
		    "and we wonder: Are we the only ones?"},
	{
		    CAVE_VOICE_1, "intro2a",
	    "Long before we came to exist, the humans ruled "
		    "the Earth."},
	{
		    CAVE_VOICE_2, "intro3a",
	    "They made marvelous things, and moved whole " "mountains."},
	{
		    CAVE_VOICE_3, "intro4a",
	    "They knew the Secret of Flight, the Secret of "
		    "Happiness, and other secrets beyond our imagining."},
	{
		    CAVE_VOICE_4, "intro1b",
	    "The humans also knew the Secret of Life, "
		    "and used it to give us the Four Great Gifts:"},
	{
		    CAVE_VOICE_5, "intro2b",
	    "Thinking minds, feeling hearts, speaking "
		    "mouths, and reaching hands."},
	{
		    CAVE_VOICE_6, "intro3b",
	    "We are their children."},
	{
		    CAVE_VOICE_7, "intro1c",
	    "They taught us how to use our hands, and how " "to speak."},
	{
		    CAVE_VOICE_8, "intro2c",
	    "They showed us the joy of using our minds."},
	{
		    CAVE_VOICE_9, "intro3c",
	    "They loved us, and when we were ready, they "
		    "surely would have given us the Secret of Happiness."},

	{
		    CAVE_VOICE_10, "intro1d",
	    "And now we see the sky, the land, and the water "
		    "that we are heirs to, and we wonder: why did "
		    "they leave?"},

	{
		    CAVE_VOICE_11, "intro2d",
	    "Do they live still, in the stars? In the oceans "
		    "depths? In the wind?"},
	{
		    CAVE_VOICE_12, "intro3d",
	    "We wonder, was their fate good or evil?"},

	{
		    CAVE_VOICE_13, "intro4d",
	    "And will we also share the same fate one day?"},
};

R_SCENE_QUEUE ITE_IntroList[] = {
	{ITE_INTRO_ANIM_SCENE, NULL, BY_RESOURCE, ITE_IntroAnimProc, 0},
	{ITE_CAVE_SCENE_1, NULL, BY_RESOURCE, ITE_IntroCave1Proc, 1},
	{ITE_CAVE_SCENE_2, NULL, BY_RESOURCE, ITE_IntroCave2Proc, 0},
	{ITE_CAVE_SCENE_3, NULL, BY_RESOURCE, ITE_IntroCave3Proc, 0},
	{ITE_CAVE_SCENE_4, NULL, BY_RESOURCE, ITE_IntroCave4Proc, 0},
	{ITE_VALLEY_SCENE, NULL, BY_RESOURCE, ITE_IntroValleyProc, 0},
	{ITE_TREEHOUSE_SCENE, NULL, BY_RESOURCE, ITE_IntroTreeHouseProc, 0},
	{ITE_FAIREPATH_SCENE, NULL, BY_RESOURCE, ITE_IntroFairePathProc, 0},
	{ITE_FAIRETENT_SCENE, NULL, BY_RESOURCE, ITE_IntroFaireTentProc, 0}
};

int ITE_StartProc(void)
{
	size_t n_introscenes;
	size_t i;

	R_SCENE_QUEUE first_scene;
	R_GAME_SCENEDESC gs_desc;

	n_introscenes = YS_NELEMS(ITE_IntroList);

	for (i = 0; i < n_introscenes; i++) {

		SCENE_Queue(&ITE_IntroList[i]);
	}

	GAME_GetSceneInfo(&gs_desc);

	first_scene.load_flag = BY_SCENE;
	first_scene.scene_n = gs_desc.first_scene;
	first_scene.scene_skiptarget = 1;
	first_scene.scene_proc = InitialSceneProc;

	SCENE_Queue(&first_scene);

	return R_SUCCESS;
}

int ITE_IntroRegisterLang(void)
{

	size_t i;

	for (i = 0; i < YS_NELEMS(IntroDiag); i++) {

		if (CVAR_Register_S(IntroDiag[i].i_str,
			IntroDiag[i].i_cvar_name,
			NULL, R_CVAR_CFG, R_INTRO_STRMAX) != R_SUCCESS) {

			R_printf(R_STDERR,
			    "Error registering intro text cvars.");

			return R_FAILURE;
		}
	}

	return R_SUCCESS;
}

int ITE_IntroAnimProc(int param, R_SCENE_INFO * scene_info)
/*--------------------------------------------------------------------------*\
 * Handles the introductory Dreamer's Guild / NWC logo animation scene.
\*--------------------------------------------------------------------------*/
{
	R_EVENT event;

	YS_IGNORE_PARAM(scene_info);

	switch (param) {

	case SCENE_BEGIN:

		/* Background for intro scene is the first frame of the
		 * intro animation; display it and set the palette
		 \*-----------------------------------------------------*/
		event.type = R_ONESHOT_EVENT;
		event.code = R_BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = SET_PALETTE;
		event.time = 0;

		EVENT_Queue(&event);

		R_printf(R_STDOUT, "Intro animation procedure started.\n");
		R_printf(R_STDOUT, "Linking animation resources...\n");

		ANIM_SetFrameTime(0, ITE_INTRO_FRAMETIME);

		/* Link this scene's animation resources for continuous
		 * playback
		 \*-----------------------------------------------------*/
		ANIM_Link(0, 1);
		ANIM_Link(1, 2);
		ANIM_Link(2, 3);
		ANIM_Link(3, 4);
		ANIM_Link(4, 5);
		ANIM_Link(5, 6);

		/* Scene should end on display of last animation frame
		 * \*----------------------------------------------------- */
		ANIM_SetFlag(6, ANIM_ENDSCENE);

		R_printf(R_STDOUT, "Beginning animation playback.\n");

		ANIM_Play(0, 0);

		/* Queue intro music playback
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_MUSIC_EVENT;
		event.param = MUSIC_1;
		event.param2 = R_MUSIC_LOOP;
		event.op = EVENT_PLAY;
		event.time = 0;

		EVENT_Queue(&event);
		break;

	case SCENE_END:
		break;

	default:
		R_printf(R_STDERR, "Illegal scene procedure parameter.\n");
		break;

	}

	return 0;
}

int ITE_IntroCave1Proc(int param, R_SCENE_INFO * scene_info)
/*--------------------------------------------------------------------------*\
 * Handles first introductory cave painting scene
\*--------------------------------------------------------------------------*/
{
	R_EVENT event;
	R_EVENT *q_event;

	int event_time = 0;
	int voice_len;
	int voice_pad = 50;

	R_TEXTLIST_ENTRY text_entry;
	R_TEXTLIST_ENTRY *entry_p;

	PALENTRY *pal;

	static PALENTRY current_pal[R_PAL_ENTRIES];

	int i;

	int font_flags = FONT_OUTLINE | FONT_CENTERED;

	switch (param) {

	case SCENE_BEGIN:

		/* Fade to black out of the intro DG/NWC logo animation
		 * \*----------------------------------------------------- */
		SYSGFX_GetCurrentPal(current_pal);

		event.type = R_CONTINUOUS_EVENT;
		event.code = R_PAL_EVENT;
		event.op = EVENT_PALTOBLACK;
		event.time = 0;
		event.duration = PALETTE_FADE_DURATION;
		event.data = current_pal;

		q_event = EVENT_Queue(&event);

		/* Display scene background, but stay with black palette
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = NO_SET_PALETTE;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		/* Fade in from black to the scene background palette
		 * \*----------------------------------------------------- */
		SCENE_GetBGPal(&pal);

		event.type = R_CONTINUOUS_EVENT;
		event.code = R_PAL_EVENT;
		event.op = EVENT_BLACKTOPAL;
		event.time = 0;
		event.duration = PALETTE_FADE_DURATION;
		event.data = pal;

		q_event = EVENT_Chain(q_event, &event);

		/* Begin palette cycling animation for candles
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		/* Queue narrator dialogue list
		 * \*----------------------------------------------------- */
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.text_x = 320 / 2;
		text_entry.text_y = INTRO_CAPTION_Y;
		text_entry.font_id = MEDIUM_FONT_ID;
		text_entry.flags = font_flags;

		for (i = INTRO_CAVE1_START; i < INTRO_CAVE1_END; i++) {

			text_entry.string = IntroDiag[i].i_str;

			entry_p = TEXT_AddEntry(scene_info->text_list,
			    &text_entry);

			/* Display text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_time;

			q_event = EVENT_Chain(q_event, &event);

			/* Play voice */
			event.type = R_ONESHOT_EVENT;
			event.code = R_VOICE_EVENT;
			event.op = EVENT_PLAY;
			event.param = IntroDiag[i].i_voice_rn;
			event.time = event_time;

			q_event = EVENT_Chain(q_event, &event);

			voice_len =
			    SND_GetVoiceLength(IntroDiag[i].i_voice_rn);
			if (voice_len < 0) {
				voice_len = strlen(IntroDiag[i].i_str) *
				    VOICE_LETTERLEN;
			}

			/* Remove text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = voice_len;

			q_event = EVENT_Chain(q_event, &event);

			event_time = voice_pad;

		}

		/* End scene after last dialogue over
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		break;

	case SCENE_END:
		break;

	default:
		R_printf(R_STDERR, "Illegal scene procedure paramater.\n");
		break;

	}

	return 0;
}

int ITE_IntroCave2Proc(int param, R_SCENE_INFO * scene_info)
/*--------------------------------------------------------------------------*\
 * Handles second introductory cave painting scene
\*--------------------------------------------------------------------------*/
{

	R_EVENT event;
	R_EVENT *q_event;

	int event_time = 0;
	int voice_len;
	int voice_pad = 50;

	R_TEXTLIST_ENTRY text_entry;
	R_TEXTLIST_ENTRY *entry_p;

	int i;

	int font_flags = FONT_OUTLINE | FONT_CENTERED;

	switch (param) {

	case SCENE_BEGIN:

		/* Start 'dissolve' transition to new scene background
		 * \*----------------------------------------------------- */
		event.type = R_CONTINUOUS_EVENT;
		event.code = R_TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = EVENT_Queue(&event);

		/* Begin palette cycling animation for candles
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		/* Queue narrator dialogue list
		 * \*----------------------------------------------------- */
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.text_x = 320 / 2;
		text_entry.text_y = INTRO_CAPTION_Y;
		text_entry.font_id = MEDIUM_FONT_ID;
		text_entry.flags = font_flags;

		for (i = INTRO_CAVE2_START; i < INTRO_CAVE2_END; i++) {

			text_entry.string = IntroDiag[i].i_str;

			entry_p = TEXT_AddEntry(scene_info->text_list,
			    &text_entry);

			/* Display text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_time;

			q_event = EVENT_Chain(q_event, &event);

			/* Play voice */
			event.type = R_ONESHOT_EVENT;
			event.code = R_VOICE_EVENT;
			event.op = EVENT_PLAY;
			event.param = IntroDiag[i].i_voice_rn;
			event.time = event_time;

			q_event = EVENT_Chain(q_event, &event);

			voice_len =
			    SND_GetVoiceLength(IntroDiag[i].i_voice_rn);
			if (voice_len < 0) {
				voice_len = strlen(IntroDiag[i].i_str) *
				    VOICE_LETTERLEN;
			}

			/* Remove text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = voice_len;

			q_event = EVENT_Chain(q_event, &event);

			event_time = voice_pad;
		}

		/* End scene after last dialogue over
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_SCENE_EVENT;
		event.op = EVENT_END;
		event.time = event_time;

		q_event = EVENT_Chain(q_event, &event);

		break;

	case SCENE_END:
		break;

	default:
		R_printf(R_STDERR, "Illegal scene procedure paramater.\n");
		break;

	}

	return 0;
}

int ITE_IntroCave3Proc(int param, R_SCENE_INFO * scene_info)
/*--------------------------------------------------------------------------*\
 * Handles third introductory cave painting scene
\*--------------------------------------------------------------------------*/
{
	R_EVENT event;
	R_EVENT *q_event;

	int event_time = 0;
	int voice_len;
	int voice_pad = 50;

	R_TEXTLIST_ENTRY text_entry;
	R_TEXTLIST_ENTRY *entry_p;

	int i;

	int font_flags = FONT_OUTLINE | FONT_CENTERED;

	switch (param) {

	case SCENE_BEGIN:

		/* Start 'dissolve' transition to new scene background
		 * \*----------------------------------------------------- */
		event.type = R_CONTINUOUS_EVENT;
		event.code = R_TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = EVENT_Queue(&event);

		/* Begin palette cycling animation for candles
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		/* Queue narrator dialogue list
		 * \*----------------------------------------------------- */
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.text_x = 320 / 2;
		text_entry.text_y = INTRO_CAPTION_Y;
		text_entry.font_id = MEDIUM_FONT_ID;
		text_entry.flags = font_flags;

		for (i = INTRO_CAVE3_START; i < INTRO_CAVE3_END; i++) {

			text_entry.string = IntroDiag[i].i_str;

			entry_p = TEXT_AddEntry(scene_info->text_list,
			    &text_entry);

			/* Display text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_time;

			q_event = EVENT_Chain(q_event, &event);

			/* Play voice */
			event.type = R_ONESHOT_EVENT;
			event.code = R_VOICE_EVENT;
			event.op = EVENT_PLAY;
			event.param = IntroDiag[i].i_voice_rn;
			event.time = event_time;

			q_event = EVENT_Chain(q_event, &event);

			voice_len =
			    SND_GetVoiceLength(IntroDiag[i].i_voice_rn);
			if (voice_len < 0) {
				voice_len = strlen(IntroDiag[i].i_str) *
				    VOICE_LETTERLEN;
			}

			/* Remove text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = voice_len;

			q_event = EVENT_Chain(q_event, &event);

			event_time = voice_pad;
		}

		/* End scene after last dialogue over
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_SCENE_EVENT;
		event.op = EVENT_END;
		event.time = event_time;

		q_event = EVENT_Chain(q_event, &event);

		break;

	case SCENE_END:
		break;

	default:
		R_printf(R_STDERR, "Illegal scene procedure paramater.\n");
		break;

	}

	return 0;
}

int ITE_IntroCave4Proc(int param, R_SCENE_INFO * scene_info)
/*--------------------------------------------------------------------------*\
 * Handles fourth introductory cave painting scene
\*--------------------------------------------------------------------------*/
{
	R_EVENT event;
	R_EVENT *q_event;

	int event_time = 0;
	int voice_len;
	int voice_pad = 50;

	R_TEXTLIST_ENTRY text_entry;
	R_TEXTLIST_ENTRY *entry_p;

	int i;

	int font_flags = FONT_OUTLINE | FONT_CENTERED;

	switch (param) {

	case SCENE_BEGIN:

		/* Start 'dissolve' transition to new scene background
		 * \*----------------------------------------------------- */
		event.type = R_CONTINUOUS_EVENT;
		event.code = R_TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = EVENT_Queue(&event);

		/* Begin palette cycling animation for candles
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		/* Queue narrator dialogue list
		 * \*----------------------------------------------------- */
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.text_x = 320 / 2;
		text_entry.text_y = INTRO_CAPTION_Y;
		text_entry.font_id = MEDIUM_FONT_ID;
		text_entry.flags = font_flags;

		for (i = INTRO_CAVE4_START; i < INTRO_CAVE4_END; i++) {

			text_entry.string = IntroDiag[i].i_str;
			entry_p = TEXT_AddEntry(scene_info->text_list,
			    &text_entry);

			/* Display text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_time;

			q_event = EVENT_Chain(q_event, &event);

			/* Play voice */
			event.type = R_ONESHOT_EVENT;
			event.code = R_VOICE_EVENT;
			event.op = EVENT_PLAY;
			event.param = IntroDiag[i].i_voice_rn;
			event.time = event_time;

			q_event = EVENT_Chain(q_event, &event);

			voice_len =
			    SND_GetVoiceLength(IntroDiag[i].i_voice_rn);
			if (voice_len < 0) {
				voice_len = strlen(IntroDiag[i].i_str) *
				    VOICE_LETTERLEN;
			}

			/* Remove text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = voice_len;

			q_event = EVENT_Chain(q_event, &event);

			event_time = voice_pad;

		}

		/* End scene after last dialogue over
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_SCENE_EVENT;
		event.op = EVENT_END;
		event.time = event_time;

		q_event = EVENT_Chain(q_event, &event);

		break;

	case SCENE_END:
		break;

	default:
		R_printf(R_STDERR, "Illegal scene procedure paramater.\n");
		break;

	}

	return 0;

}

int ITE_IntroValleyProc(int param, R_SCENE_INFO * scene_info)
/*--------------------------------------------------------------------------*\
 * Handles intro title scene (valley overlook)
\*--------------------------------------------------------------------------*/
{
	R_TEXTLIST_ENTRY text_entry;
	R_TEXTLIST_ENTRY *entry_p;

	R_EVENT event;
	R_EVENT *q_event;

	int i;

	const INTRO_CREDIT credits[] = {
		{160, 44, 9000, CREDIT_DURATION1,
		    "Producer", SMALL_FONT_ID},
		{160, 56, 0, CREDIT_DURATION1,
		    "Walter Hochbrueckner", MEDIUM_FONT_ID},
		{160, 88, 0, CREDIT_DURATION1,
		    "Executive Producer", SMALL_FONT_ID},
		{160, 100, 0, CREDIT_DURATION1,
		    "Robert McNally", MEDIUM_FONT_ID},
		{160, 132, 0, CREDIT_DURATION1,
		    "Publisher", SMALL_FONT_ID},
		{160, 144, 0, CREDIT_DURATION1,
		    "Jon Van Caneghem", MEDIUM_FONT_ID}
	};

	int n_credits = sizeof credits / sizeof credits[0];

	int event_delay = 0;

	switch (param) {

	case SCENE_BEGIN:

		/* Display ITE title screen background
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = SET_PALETTE;
		event.time = 0;

		q_event = EVENT_Queue(&event);

		R_printf(R_STDOUT, "Beginning animation playback.\n");

		/* Begin title screen background animation 
		 * \*----------------------------------------------------- */
		ANIM_SetFlag(0, ANIM_LOOP);
		ANIM_Play(0, 0);

		/* Begin ITE title theme music
		 * \*----------------------------------------------------- */
		SYSMUSIC_Stop();

		event.type = R_ONESHOT_EVENT;
		event.code = R_MUSIC_EVENT;
		event.param = MUSIC_2;
		event.op = EVENT_PLAY;
		event.time = 0;

		q_event = EVENT_Chain(q_event, &event);

		/* Queue game credits list
		 * \*----------------------------------------------------- */
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.flags = FONT_OUTLINE | FONT_CENTERED;

		for (i = 0; i < n_credits; i++) {

			text_entry.string = credits[i].string;
			text_entry.font_id = credits[i].font_id;
			text_entry.text_x = credits[i].text_x;
			text_entry.text_y = credits[i].text_y;

			entry_p = TEXT_AddEntry(scene_info->text_list,
			    &text_entry);

			/* Display text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_delay += credits[i].delta_time;

			q_event = EVENT_Queue(&event);

			/* Remove text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = credits[i].duration;

			q_event = EVENT_Chain(q_event, &event);
		}

		/* End scene after credit display
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 1000;

		q_event = EVENT_Chain(q_event, &event);
		break;

	case SCENE_END:
		break;

	default:
		R_printf(R_STDERR, "Illegal scene procedure parameter.\n");
		break;

	}

	return 0;
}

int ITE_IntroTreeHouseProc(int param, R_SCENE_INFO * scene_info)
/*--------------------------------------------------------------------------*\
 * Handles second intro credit screen (treehouse view)
\*--------------------------------------------------------------------------*/
{
	R_TEXTLIST_ENTRY text_entry;
	R_TEXTLIST_ENTRY *entry_p;

	R_EVENT event;
	R_EVENT *q_event;

	int i;

	const INTRO_CREDIT credits[] = {
		{160, 58, 2000, CREDIT_DURATION1,
		    "Game Design", SMALL_FONT_ID},
		{160, 70, 0, CREDIT_DURATION1,
		    "Talin, Joe Pearce, Robert McNally", MEDIUM_FONT_ID},
		{160, 80, 0, CREDIT_DURATION1,
		    "and Carolly Hauksdottir", MEDIUM_FONT_ID},
		{160, 119, 0, CREDIT_DURATION1,
		    "Screenplay and Dialog", SMALL_FONT_ID},
		{160, 131, 0, CREDIT_DURATION1,
		    "Robert Leh, Len Wein, and Bill Rotsler",
			    MEDIUM_FONT_ID},
		{160, 54, 5000, CREDIT_DURATION1,
		    "Art", SMALL_FONT_ID},
		{160, 66, 0, CREDIT_DURATION1,
		    "Edward Lacabanne, Glenn Price, April Lee,",
			    MEDIUM_FONT_ID},
		{160, 76, 0, CREDIT_DURATION1,
		    "Lisa Iennaco, Brian Dowrick, Reed", MEDIUM_FONT_ID},
		{160, 86, 0, CREDIT_DURATION1,
		    "Waller, Allison Hershey and Talin", MEDIUM_FONT_ID},
		{160, 123, 0, CREDIT_DURATION1,
		    "Art Direction", SMALL_FONT_ID},
		{160, 135, 0, CREDIT_DURATION1,
		    "Allison Hershey", MEDIUM_FONT_ID}
	};

	int n_credits = YS_NELEMS(credits);

	int event_delay = 0;

	switch (param) {

	case SCENE_BEGIN:

		/* Start 'dissolve' transition to new scene background
		 * \*----------------------------------------------------- */
		event.type = R_CONTINUOUS_EVENT;
		event.code = R_TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = EVENT_Queue(&event);

		event_delay = DISSOLVE_DURATION;

		/* Begin title screen background animation 
		 * \*----------------------------------------------------- */
		ANIM_SetFrameTime(0, 100);
		ANIM_Play(0, event_delay);

		/* Queue game credits list
		 * \*----------------------------------------------------- */
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.flags = FONT_OUTLINE | FONT_CENTERED;

		for (i = 0; i < n_credits; i++) {

			text_entry.string = credits[i].string;
			text_entry.font_id = credits[i].font_id;
			text_entry.text_x = credits[i].text_x;
			text_entry.text_y = credits[i].text_y;

			entry_p = TEXT_AddEntry(scene_info->text_list,
			    &text_entry);

			/* Display text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_delay += credits[i].delta_time;

			q_event = EVENT_Queue(&event);

			/* Remove text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = credits[i].duration;

			q_event = EVENT_Chain(q_event, &event);
		}

		/* End scene after credit display
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 1000;

		q_event = EVENT_Chain(q_event, &event);

		break;

	case SCENE_END:

		break;

	default:
		R_printf(R_STDERR, "Illegal scene procedure parameter.\n");
		break;

	}

	return 0;

}

int ITE_IntroFairePathProc(int param, R_SCENE_INFO * scene_info)
/*--------------------------------------------------------------------------*\
 * Handles third intro credit screen (path to puzzle tent)
\*--------------------------------------------------------------------------*/
{
	R_TEXTLIST_ENTRY text_entry;
	R_TEXTLIST_ENTRY *entry_p;

	R_EVENT event;
	R_EVENT *q_event;

	long event_delay = 0;

	int i;

	INTRO_CREDIT credits[] = {
		{160, 58, 2000, CREDIT_DURATION1,
		    "Original Game Engine Programming", SMALL_FONT_ID},

		{160, 70, 0, CREDIT_DURATION1,
		    "Talin, Walter Hochbrueckner,", MEDIUM_FONT_ID},

		{160, 80, 0, CREDIT_DURATION1,
		    "Joe Burks and Robert Wiggins", MEDIUM_FONT_ID},

		{160, 119, 0, CREDIT_DURATION1,
		    "Music and Sound", SMALL_FONT_ID},

		{160, 131, 0, CREDIT_DURATION1,
		    "Matt Nathan", MEDIUM_FONT_ID},

		{160, 58, 5000, CREDIT_DURATION1,
		    "Directed by", SMALL_FONT_ID},

		{160, 70, 0, CREDIT_DURATION1,
		    "Talin", MEDIUM_FONT_ID},

		{160, 119, 0, CREDIT_DURATION1,
		    "Game Engine Reconstruction", SMALL_FONT_ID},

		{160, 131, 0, CREDIT_DURATION1,
		    "Alpha software - Use at your own risk.",
			    MEDIUM_FONT_ID}
	};

	int n_credits = sizeof credits / sizeof credits[0];

	switch (param) {

	case SCENE_BEGIN:

		/* Start 'dissolve' transition to new scene background
		 * \*----------------------------------------------------- */
		event.type = R_CONTINUOUS_EVENT;
		event.code = R_TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = EVENT_Queue(&event);

		event_delay = DISSOLVE_DURATION;

		/* Begin title screen background animation 
		 * \*----------------------------------------------------- */
		ANIM_SetFlag(0, ANIM_LOOP);
		ANIM_Play(0, event_delay);

		/* Queue game credits list
		 * \*----------------------------------------------------- */
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.flags = FONT_OUTLINE | FONT_CENTERED;

		for (i = 0; i < n_credits; i++) {

			text_entry.string = credits[i].string;
			text_entry.font_id = credits[i].font_id;
			text_entry.text_x = credits[i].text_x;
			text_entry.text_y = credits[i].text_y;

			entry_p = TEXT_AddEntry(scene_info->text_list,
			    &text_entry);

			/* Display text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_delay += credits[i].delta_time;

			q_event = EVENT_Queue(&event);

			/* Remove text */
			event.type = R_ONESHOT_EVENT;
			event.code = R_TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = credits[i].duration;

			q_event = EVENT_Chain(q_event, &event);
		}

		/* End scene after credit display
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 1000;

		q_event = EVENT_Chain(q_event, &event);

		break;

	case SCENE_END:
		break;

	default:
		R_printf(R_STDERR, "Illegal scene procedure parameter.\n");
		break;

	}

	return 0;
}

int ITE_IntroFaireTentProc(int param, R_SCENE_INFO * scene_info)
/*--------------------------------------------------------------------------*\
 * Handles fourth intro credit screen (treehouse view)
\*--------------------------------------------------------------------------*/
{
	R_EVENT event;
	R_EVENT *q_event;
	R_EVENT *q_event_start;

	YS_IGNORE_PARAM(scene_info);

	switch (param) {

	case SCENE_BEGIN:

		/* Start 'dissolve' transition to new scene background
		 * \*----------------------------------------------------- */
		event.type = R_CONTINUOUS_EVENT;
		event.code = R_TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event_start = EVENT_Queue(&event);

		/* End scene after momentary pause
		 * \*----------------------------------------------------- */
		event.type = R_ONESHOT_EVENT;
		event.code = R_SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 5000;

		q_event = EVENT_Chain(q_event_start, &event);

		break;

	case SCENE_END:

		break;

	default:
		R_printf(R_STDERR, "Illegal scene procedure parameter.\n");
		break;

	}

	return 0;
}

} // End of namespace Saga
