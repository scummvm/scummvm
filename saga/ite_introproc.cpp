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


// Intro sequence scene procedures

#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/animation.h"
#include "saga/events.h"
#include "saga/font.h"
#include "saga/rscfile_mod.h"
#include "saga/sndres.h"
#include "saga/text.h"
#include "saga/palanim.h"
#include "saga/music.h"

#include "saga/scene.h"

namespace Saga {

static INTRO_DIALOGUE IntroDiag[] = {
	{
		RID_CAVE_VOICE_0, "intro1a",
		"We see the sky, we see the land, we see the water, "
		"and we wonder: Are we the only ones?"
	},
	{
		RID_CAVE_VOICE_1, "intro2a",
		"Long before we came to exist, the humans ruled "
		"the Earth."
	},
	{
		RID_CAVE_VOICE_2, "intro3a",
		"They made marvelous things, and moved whole " "mountains."
	},
	{
		RID_CAVE_VOICE_3, "intro4a",
		"They knew the Secret of Flight, the Secret of "
		"Happiness, and other secrets beyond our imagining."
	},
	{
		RID_CAVE_VOICE_4, "intro1b",
		"The humans also knew the Secret of Life, "
		"and they used it to give us the Four Great Gifts:"
	},
	{
		RID_CAVE_VOICE_5, "intro2b",
		"Thinking minds, feeling hearts, speaking "
		"mouths, and reaching hands."
	},
	{
		RID_CAVE_VOICE_6, "intro3b",
		"We are their children."
	},
	{
		RID_CAVE_VOICE_7, "intro1c",
		"They taught us how to use our hands, and how " "to speak."
	},
	{
		RID_CAVE_VOICE_8, "intro2c",
		"They showed us the joy of using our minds."
	},
	{
		RID_CAVE_VOICE_9, "intro3c",
		"They loved us, and when we were ready, they "
		"surely would have given us the Secret of Happiness."
	},
	{
		RID_CAVE_VOICE_10, "intro1d",
		"And now we see the sky, the land, and the water "
		"that we are heirs to, and we wonder: why did "
		"they leave?"
	},
	{
		RID_CAVE_VOICE_11, "intro2d",
		"Do they live still, in the stars? In the oceans "
		"depths? In the wind?"
	},
	{
		RID_CAVE_VOICE_12, "intro3d",
		"We wonder, was their fate good or evil?"
	},
	{
		RID_CAVE_VOICE_13, "intro4d",
		"And will we also share the same fate one day?"
	},
};

SCENE_QUEUE ITE_IntroList[] = {
	{RID_ITE_INTRO_ANIM_SCENE, NULL, BY_RESOURCE, Scene::SC_ITEIntroAnimProc, 0, SCENE_NOFADE},
	{RID_ITE_CAVE_SCENE_1, NULL, BY_RESOURCE, Scene::SC_ITEIntroCave1Proc, 0, SCENE_FADE_NO_INTERFACE},
	{RID_ITE_CAVE_SCENE_2, NULL, BY_RESOURCE, Scene::SC_ITEIntroCave2Proc, 0, SCENE_NOFADE},
	{RID_ITE_CAVE_SCENE_3, NULL, BY_RESOURCE, Scene::SC_ITEIntroCave3Proc, 0, SCENE_NOFADE},
	{RID_ITE_CAVE_SCENE_4, NULL, BY_RESOURCE, Scene::SC_ITEIntroCave4Proc, 0, SCENE_NOFADE},
	{RID_ITE_VALLEY_SCENE, NULL, BY_RESOURCE, Scene::SC_ITEIntroValleyProc, 0, SCENE_FADE_NO_INTERFACE},
	{RID_ITE_TREEHOUSE_SCENE, NULL, BY_RESOURCE, Scene::SC_ITEIntroTreeHouseProc, 0, SCENE_NOFADE},
	{RID_ITE_FAIREPATH_SCENE, NULL, BY_RESOURCE, Scene::SC_ITEIntroFairePathProc, 0, SCENE_NOFADE},
	{RID_ITE_FAIRETENT_SCENE, NULL, BY_RESOURCE, Scene::SC_ITEIntroFaireTentProc, 0, SCENE_NOFADE}
};

int Scene::ITEStartProc() {
	size_t n_introscenes;
	size_t i;

	SCENE_QUEUE first_scene;
	SCENE_QUEUE tempScene;
	GAME_SCENEDESC gs_desc;

	n_introscenes = ARRAYSIZE(ITE_IntroList);

	for (i = 0; i < n_introscenes; i++) {
		tempScene = ITE_IntroList[i];
		tempScene.scene_n = RSC_ConvertID(tempScene.scene_n);
		_vm->_scene->queueScene(&tempScene);
	}

	_vm->getSceneInfo(&gs_desc);

	first_scene.load_flag = BY_SCENE;
	first_scene.scene_n = gs_desc.first_scene;
	first_scene.scene_skiptarget = 1;
	first_scene.scene_proc = NULL;
	first_scene.fadeType = SCENE_FADE;

	_vm->_scene->queueScene(&first_scene);

	return SUCCESS;
}

int Scene::ITEIntroRegisterLang() {
#if 0
	size_t i;

	for (i = 0; i < ARRAYSIZE(IntroDiag); i++) {
		if (CVAR_Register_S(IntroDiag[i].i_str,
			IntroDiag[i].i_cvar_name,
			NULL, CVAR_CFG, INTRO_STRMAX) != SUCCESS) {
			warning("Error registering intro text cvars");
			return FAILURE;
		}
	}
#endif

	return SUCCESS;
}

int Scene::SC_ITEIntroAnimProc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->ITEIntroAnimProc(param, scene_info);
}

// Handles the introductory Dreamer's Guild / NWC logo animation scene.
int Scene::ITEIntroAnimProc(int param, SCENE_INFO *scene_info) {
	EVENT event;

	switch (param) {
	case SCENE_BEGIN:

		// Background for intro scene is the first frame of the
		// intro animation; display it and set the palette
		event.type = ONESHOT_EVENT;
		event.code = BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = SET_PALETTE;
		event.time = 0;

		_vm->_events->queue(&event);

		debug(0, "Intro animation procedure started.");
		debug(0, "Linking animation resources...");

		_vm->_anim->setFrameTime(0, ITE_INTRO_FRAMETIME);

		// Link this scene's animation resources for continuous
		// playback
		_vm->_anim->link(0, 1);
		_vm->_anim->link(1, 2);
		_vm->_anim->link(2, 3);
		_vm->_anim->link(3, 4);
		_vm->_anim->link(4, 5);
		_vm->_anim->link(5, 6);

		// Scene should end on display of last animation frame
		_vm->_anim->setFlag(6, ANIM_ENDSCENE);

		debug(0, "Beginning animation playback.");

		_vm->_anim->play(0, 0);

		// Queue intro music playback
		event.type = ONESHOT_EVENT;
		event.code = MUSIC_EVENT;
		event.param = MUSIC_1;
		event.param2 = MUSIC_LOOP;
		event.op = EVENT_PLAY;
		event.time = 0;

		_vm->_events->queue(&event);
		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroCave1Proc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->ITEIntroCave1Proc(param, scene_info);
}

// Handles first introductory cave painting scene
int Scene::ITEIntroCave1Proc(int param, SCENE_INFO *scene_info) {
	EVENT event;
	EVENT *q_event;
	int event_time = 0;
	int voice_len;
	int voice_pad = 50;
	TEXTLIST_ENTRY text_entry;
	TEXTLIST_ENTRY *entry_p;
	int i;
	int font_flags = FONT_OUTLINE | FONT_CENTERED;

	switch (param) {
	case SCENE_BEGIN:
		// Begin palette cycling animation for candles
		event.type = ONESHOT_EVENT;
		event.code = PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		q_event = _vm->_events->queue(&event);

		// Queue narrator dialogue list
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.text_x = 320 / 2;
		text_entry.text_y = INTRO_CAPTION_Y;
		text_entry.font_id = MEDIUM_FONT_ID;
		text_entry.flags = font_flags;

		for (i = INTRO_CAVE1_START; i < INTRO_CAVE1_END; i++) {
			text_entry.string = IntroDiag[i].i_str;
			entry_p = _vm->textAddEntry(scene_info->text_list, &text_entry);

			// Display text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_time;

			q_event = _vm->_events->chain(q_event, &event);

			// Play voice
			event.type = ONESHOT_EVENT;
			event.code = VOICE_EVENT;
			event.op = EVENT_PLAY;
			event.param = IntroDiag[i].i_voice_rn;
			event.time = event_time;

			q_event = _vm->_events->chain(q_event, &event);

			voice_len = _vm->_sndRes->getVoiceLength(IntroDiag[i].i_voice_rn);
			if (voice_len < 0) {
				voice_len = strlen(IntroDiag[i].i_str) * VOICE_LETTERLEN;
			}

			// Remove text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = voice_len;

			q_event = _vm->_events->chain(q_event, &event);

			event_time = voice_pad;
		}

		// End scene after last dialogue over
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);
		break;
	case SCENE_END:
		break;

	default:
		warning("Illegal scene procedure paramater");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroCave2Proc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->ITEIntroCave2Proc(param, scene_info);
}

// Handles second introductory cave painting scene
int Scene::ITEIntroCave2Proc(int param, SCENE_INFO *scene_info) {
	EVENT event;
	EVENT *q_event;
	int event_time = 0;
	int voice_len;
	int voice_pad = 50;
	TEXTLIST_ENTRY text_entry;
	TEXTLIST_ENTRY *entry_p;
	int i;
	int font_flags = FONT_OUTLINE | FONT_CENTERED;

	switch (param) {
	case SCENE_BEGIN:
		// Start 'dissolve' transition to new scene background
		event.type = CONTINUOUS_EVENT;
		event.code = TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = _vm->_events->queue(&event);

		// Begin palette cycling animation for candles
		event.type = ONESHOT_EVENT;
		event.code = PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Queue narrator dialogue list
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.text_x = 320 / 2;
		text_entry.text_y = INTRO_CAPTION_Y;
		text_entry.font_id = MEDIUM_FONT_ID;
		text_entry.flags = font_flags;

		for (i = INTRO_CAVE2_START; i < INTRO_CAVE2_END; i++) {
			text_entry.string = IntroDiag[i].i_str;
			entry_p = _vm->textAddEntry(scene_info->text_list, &text_entry);

			// Display text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_time;

			q_event = _vm->_events->chain(q_event, &event);

			// Play voice
			event.type = ONESHOT_EVENT;
			event.code = VOICE_EVENT;
			event.op = EVENT_PLAY;
			event.param = IntroDiag[i].i_voice_rn;
			event.time = event_time;

			q_event = _vm->_events->chain(q_event, &event);

			voice_len = _vm->_sndRes->getVoiceLength(IntroDiag[i].i_voice_rn);
			if (voice_len < 0) {
				voice_len = strlen(IntroDiag[i].i_str) * VOICE_LETTERLEN;
			}

			// Remove text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = voice_len;

			q_event = _vm->_events->chain(q_event, &event);

			event_time = voice_pad;
		}

		// End scene after last dialogue over
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = event_time;

		q_event = _vm->_events->chain(q_event, &event);
		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure paramater");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroCave3Proc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->ITEIntroCave3Proc(param, scene_info);
}

// Handles third introductory cave painting scene
int Scene::ITEIntroCave3Proc(int param, SCENE_INFO *scene_info) {
	EVENT event;
	EVENT *q_event;
	int event_time = 0;
	int voice_len;
	int voice_pad = 50;
	TEXTLIST_ENTRY text_entry;
	TEXTLIST_ENTRY *entry_p;
	int i;
	int font_flags = FONT_OUTLINE | FONT_CENTERED;

	switch (param) {
	case SCENE_BEGIN:
		// Start 'dissolve' transition to new scene background
		event.type = CONTINUOUS_EVENT;
		event.code = TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = _vm->_events->queue(&event);

		// Begin palette cycling animation for candles
		event.type = ONESHOT_EVENT;
		event.code = PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Queue narrator dialogue list
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.text_x = 320 / 2;
		text_entry.text_y = INTRO_CAPTION_Y;
		text_entry.font_id = MEDIUM_FONT_ID;
		text_entry.flags = font_flags;

		for (i = INTRO_CAVE3_START; i < INTRO_CAVE3_END; i++) {
			text_entry.string = IntroDiag[i].i_str;
			entry_p = _vm->textAddEntry(scene_info->text_list, &text_entry);

			// Display text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_time;

			q_event = _vm->_events->chain(q_event, &event);

			// Play voice
			event.type = ONESHOT_EVENT;
			event.code = VOICE_EVENT;
			event.op = EVENT_PLAY;
			event.param = IntroDiag[i].i_voice_rn;
			event.time = event_time;

			q_event = _vm->_events->chain(q_event, &event);

			voice_len = _vm->_sndRes->getVoiceLength(IntroDiag[i].i_voice_rn);
			if (voice_len < 0) {
				voice_len = strlen(IntroDiag[i].i_str) * VOICE_LETTERLEN;
			}

			// Remove text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = voice_len;

			q_event = _vm->_events->chain(q_event, &event);

			event_time = voice_pad;
		}

		// End scene after last dialogue over
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = event_time;

		q_event = _vm->_events->chain(q_event, &event);

		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure paramater");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroCave4Proc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->ITEIntroCave4Proc(param, scene_info);
}

// Handles fourth introductory cave painting scene
int Scene::ITEIntroCave4Proc(int param, SCENE_INFO *scene_info) {
	EVENT event;
	EVENT *q_event;
	int event_time = 0;
	int voice_len;
	int voice_pad = 50;
	TEXTLIST_ENTRY text_entry;
	TEXTLIST_ENTRY *entry_p;
	int i;
	int font_flags = FONT_OUTLINE | FONT_CENTERED;

	switch (param) {
	case SCENE_BEGIN:
		// Start 'dissolve' transition to new scene background
		event.type = CONTINUOUS_EVENT;
		event.code = TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = _vm->_events->queue(&event);

		// Begin palette cycling animation for candles
		event.type = ONESHOT_EVENT;
		event.code = PALANIM_EVENT;
		event.op = EVENT_CYCLESTART;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Queue narrator dialogue list
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.text_x = 320 / 2;
		text_entry.text_y = INTRO_CAPTION_Y;
		text_entry.font_id = MEDIUM_FONT_ID;
		text_entry.flags = font_flags;

		for (i = INTRO_CAVE4_START; i < INTRO_CAVE4_END; i++) {
			text_entry.string = IntroDiag[i].i_str;
			entry_p = _vm->textAddEntry(scene_info->text_list, &text_entry);

			// Display text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_time;

			q_event = _vm->_events->chain(q_event, &event);

			// Play voice
			event.type = ONESHOT_EVENT;
			event.code = VOICE_EVENT;
			event.op = EVENT_PLAY;
			event.param = IntroDiag[i].i_voice_rn;
			event.time = event_time;

			q_event = _vm->_events->chain(q_event, &event);

			voice_len = _vm->_sndRes->getVoiceLength(IntroDiag[i].i_voice_rn);
			if (voice_len < 0) {
				voice_len = strlen(IntroDiag[i].i_str) * VOICE_LETTERLEN;
			}

			// Remove text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = voice_len;

			q_event = _vm->_events->chain(q_event, &event);

			event_time = voice_pad;
		}

		// End scene after last dialogue over
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = event_time;

		q_event = _vm->_events->chain(q_event, &event);

		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure paramater");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroValleyProc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->ITEIntroValleyProc(param, scene_info);
}

// Handles intro title scene (valley overlook)
int Scene::ITEIntroValleyProc(int param, SCENE_INFO *scene_info) {
	TEXTLIST_ENTRY text_entry;
	TEXTLIST_ENTRY *entry_p;
	EVENT event;
	EVENT *q_event;
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
	int event_delay = 3000;

	switch (param) {
	case SCENE_BEGIN:
		debug(0, "Beginning animation playback.");

		// Begin title screen background animation 
		_vm->_anim->setFlag(0, ANIM_LOOP);
		_vm->_anim->play(0, PALETTE_FADE_DURATION);

		// Begin ITE title theme music
		_vm->_music->stop();

		event.type = ONESHOT_EVENT;
		event.code = MUSIC_EVENT;
		event.param = MUSIC_2;
		event.param2 = 0;
		event.op = EVENT_PLAY;
		event.time = 0;

		q_event = _vm->_events->queue(&event);
		
		// Pause animation before logo
		event.type = ONESHOT_EVENT;
		event.code = ANIM_EVENT;
		event.op = EVENT_SETFLAG;
		event.param = 0;
		event.param2 = ANIM_PAUSE;
		event.time = 3000;

		q_event = _vm->_events->chain(q_event, &event);

		// Display logo
		event.type = CONTINUOUS_EVENT;
		event.code = TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE_BGMASK;
		event.time = 0;
		event.duration = LOGO_DISSOLVE_DURATION;

		q_event = _vm->_events->chain(q_event, &event);

		// Remove logo
		event.type = CONTINUOUS_EVENT;
		event.code = TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 1000;
		event.duration = LOGO_DISSOLVE_DURATION;

		q_event = _vm->_events->chain(q_event, &event);

		// Unpause animation before logo
		event.type = ONESHOT_EVENT;
		event.code = ANIM_EVENT;
		event.op = EVENT_CLEARFLAG;
		event.param = 0;
		event.param2 = ANIM_PAUSE;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		event.type = ONESHOT_EVENT;
		event.code = ANIM_EVENT;
		event.op = EVENT_FRAME;
		event.param = 0;
		event.time = LOGO_DISSOLVE_DURATION;

		q_event = _vm->_events->chain(q_event, &event);

		// Queue game credits list
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.flags = FONT_OUTLINE | FONT_CENTERED;

		for (i = 0; i < n_credits; i++) {
			text_entry.string = credits[i].string;
			text_entry.font_id = credits[i].font_id;
			text_entry.text_x = credits[i].text_x;
			text_entry.text_y = credits[i].text_y;

			entry_p = _vm->textAddEntry(scene_info->text_list, &text_entry);

			// Display text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_delay += credits[i].delta_time;

			q_event = _vm->_events->queue(&event);

			// Remove text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = credits[i].duration;

			q_event = _vm->_events->chain(q_event, &event);
		}

		// End scene after credit display
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 1000;

		q_event = _vm->_events->chain(q_event, &event);
		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroTreeHouseProc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->ITEIntroTreeHouseProc(param, scene_info);
}

// Handles second intro credit screen (treehouse view)
int Scene::ITEIntroTreeHouseProc(int param, SCENE_INFO *scene_info) {
	TEXTLIST_ENTRY text_entry;
	TEXTLIST_ENTRY *entry_p;
	EVENT event;
	EVENT *q_event;

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

	int n_credits = ARRAYSIZE(credits);
	int event_delay = 0;

	switch (param) {
	case SCENE_BEGIN:
		// Start 'dissolve' transition to new scene background
		event.type = CONTINUOUS_EVENT;
		event.code = TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = _vm->_events->queue(&event);

		event_delay = DISSOLVE_DURATION;

		// Begin title screen background animation 
		_vm->_anim->setFrameTime(0, 100);
		_vm->_anim->play(0, event_delay);

		// Queue game credits list
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.flags = FONT_OUTLINE | FONT_CENTERED;

		for (i = 0; i < n_credits; i++) {
			text_entry.string = credits[i].string;
			text_entry.font_id = credits[i].font_id;
			text_entry.text_x = credits[i].text_x;
			text_entry.text_y = credits[i].text_y;

			entry_p = _vm->textAddEntry(scene_info->text_list, &text_entry);

			// Display text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_delay += credits[i].delta_time;

			q_event = _vm->_events->queue(&event);

			// Remove text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = credits[i].duration;

			q_event = _vm->_events->chain(q_event, &event);
		}

		// End scene after credit display
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 1000;

		q_event = _vm->_events->chain(q_event, &event);
		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroFairePathProc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->ITEIntroFairePathProc(param, scene_info);
}

// Handles third intro credit screen (path to puzzle tent)
int Scene::ITEIntroFairePathProc(int param, SCENE_INFO *scene_info) {
	TEXTLIST_ENTRY text_entry;
	TEXTLIST_ENTRY *entry_p;
	EVENT event;
	EVENT *q_event;
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

		// Start 'dissolve' transition to new scene background
		event.type = CONTINUOUS_EVENT;
		event.code = TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = _vm->_events->queue(&event);

		event_delay = DISSOLVE_DURATION;

		// Begin title screen background animation 
		_vm->_anim->setFlag(0, ANIM_LOOP);
		_vm->_anim->play(0, event_delay);

		// Queue game credits list
		text_entry.color = 255;
		text_entry.effect_color = 0;
		text_entry.flags = FONT_OUTLINE | FONT_CENTERED;

		for (i = 0; i < n_credits; i++) {
			text_entry.string = credits[i].string;
			text_entry.font_id = credits[i].font_id;
			text_entry.text_x = credits[i].text_x;
			text_entry.text_y = credits[i].text_y;

			entry_p = _vm->textAddEntry(scene_info->text_list, &text_entry);

			// Display text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_DISPLAY;
			event.data = entry_p;
			event.time = event_delay += credits[i].delta_time;

			q_event = _vm->_events->queue(&event);

			// Remove text
			event.type = ONESHOT_EVENT;
			event.code = TEXT_EVENT;
			event.op = EVENT_REMOVE;
			event.data = entry_p;
			event.time = credits[i].duration;

			q_event = _vm->_events->chain(q_event, &event);
		}

		// End scene after credit display
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 1000;

		q_event = _vm->_events->chain(q_event, &event);
		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroFaireTentProc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->ITEIntroFaireTentProc(param, scene_info);
}

// Handles fourth intro credit screen (treehouse view)
int Scene::ITEIntroFaireTentProc(int param, SCENE_INFO *scene_info) {
	EVENT event;
	EVENT *q_event;
	EVENT *q_event_start;

	switch (param) {
	case SCENE_BEGIN:

		// Start 'dissolve' transition to new scene background
		event.type = CONTINUOUS_EVENT;
		event.code = TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event_start = _vm->_events->queue(&event);

		// End scene after momentary pause
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 5000;
		q_event = _vm->_events->chain(q_event_start, &event);
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
