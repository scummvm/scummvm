/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

enum {
	kCHeader,
	kCText
};

enum {
	kITEPC          = (1 << 0),
	kITEPCCD        = (1 << 1),
	kITEMac         = (1 << 2),
	kITEWyrmKeep    = (1 << 3),
	kITEAny         = 0xffff,
	kITENotWyrmKeep = kITEAny & ~kITEWyrmKeep
};

#define INV(n) (kITEAny & ~(n))

// Queue a page of credits text. The original interpreter did word-wrapping
// automatically. We currently don't.

EVENT *Scene::ITEQueueCredits(SCENE_INFO *scene_info, int delta_time, int duration, int n_credits, const INTRO_CREDIT credits[]) {
	int game;

	// The assumption here is that all WyrmKeep versions have the same
	// credits, regardless of which operating system they're for.

	if (_vm->_features & GF_WYRMKEEP) {
		game = kITEWyrmKeep;
	} else if (_vm->_features & GF_MAC_RESOURCES) {
		game = kITEMac;
	} else if (_vm->_gameId == GID_ITE_CD_G) {
		game = kITEPCCD;
	} else {
		game = kITEPC;
	}

	int line_spacing = 0;
	int paragraph_spacing;
	int font = 0;
	int i;

	int n_paragraphs = 0;
	int credits_height = 0;

	for (i = 0; i < n_credits; i++) {
		if (!(credits[i].game & game)) {
			continue;
		}

		switch (credits[i].type) {
		case kCHeader:
			font = SMALL_FONT_ID;
			line_spacing = 4;
			n_paragraphs++;
			break;
		case kCText:
			font = MEDIUM_FONT_ID;
			line_spacing = 2;
			break;
		default:
			error("Unknown credit type");
		}

		credits_height += (_vm->_font->getHeight(font) + line_spacing);
	}

	paragraph_spacing = (200 - credits_height) / (n_paragraphs + 3);
	credits_height += (n_paragraphs * paragraph_spacing);

	int y = paragraph_spacing;

	TEXTLIST_ENTRY text_entry;
	TEXTLIST_ENTRY *entry_p;
	EVENT event;
	EVENT *q_event = NULL;

	text_entry.color = 255;
	text_entry.effect_color = 0;
	text_entry.flags = FONT_OUTLINE | FONT_CENTERED;
	text_entry.text_x = 160;

	for (i = 0; i < n_credits; i++) {
		if (!(credits[i].game & game)) {
			continue;
		}

		switch (credits[i].type) {
		case kCHeader:
			font = SMALL_FONT_ID;
			line_spacing = 4;
			y += paragraph_spacing;
			break;
		case kCText:
			font = MEDIUM_FONT_ID;
			line_spacing = 2;
			break;
		default:
			break;
		}

		text_entry.string = credits[i].string;
		text_entry.font_id = font;
		text_entry.text_y = y;

		entry_p = _vm->textAddEntry(scene_info->text_list, &text_entry);

		// Display text
		event.type = ONESHOT_EVENT;
		event.code = TEXT_EVENT;
		event.op = EVENT_DISPLAY;
		event.data = entry_p;
		event.time = delta_time;

		q_event = _vm->_events->queue(&event);

		// Remove text
		event.type = ONESHOT_EVENT;
		event.code = TEXT_EVENT;
		event.op = EVENT_REMOVE;
		event.data = entry_p;
		event.time = duration;

		q_event = _vm->_events->chain(q_event, &event);

		y += (_vm->_font->getHeight(font) + line_spacing);
	}

	return q_event;
}

int Scene::SC_ITEIntroAnimProc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->ITEIntroAnimProc(param, scene_info);
}

// Handles the introductory Dreamer's Guild / NWC logo animation scene.
int Scene::ITEIntroAnimProc(int param, SCENE_INFO *scene_info) {
	EVENT event;
	EVENT *q_event;

	switch (param) {
	case SCENE_BEGIN:

		// Background for intro scene is the first frame of the
		// intro animation; display it and set the palette
		event.type = ONESHOT_EVENT;
		event.code = BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = SET_PALETTE;
		event.time = 0;

		q_event = _vm->_events->queue(&event);

		debug(0, "Intro animation procedure started.");
		debug(0, "Linking animation resources...");

		_vm->_anim->setFrameTime(0, ITE_INTRO_FRAMETIME);

		// Link this scene's animation resources for continuous
		// playback
		int lastAnim;

		if (_vm->_features & GF_WYRMKEEP) {
			if (_vm->_features & GF_MAC_RESOURCES) {
				lastAnim = 3;
			} else {
				lastAnim = 2;
			}
		} else {
			if (_vm->_features & GF_MAC_RESOURCES) {
				lastAnim = 4;
			} else {
				lastAnim = 5;
			}
		}

		for (int i = 0; i < lastAnim; i++)
			_vm->_anim->link(i, i+1);

		_vm->_anim->setFlag(lastAnim, ANIM_ENDSCENE);

		debug(0, "Beginning animation playback.");

		// Begin the animation
		event.type = ONESHOT_EVENT;
		event.code = ANIM_EVENT;
		event.op = EVENT_PLAY;
		event.param = 0;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Queue intro music playback
		event.type = ONESHOT_EVENT;
		event.code = MUSIC_EVENT;
		event.param = MUSIC_1;
		event.param2 = MUSIC_LOOP;
		event.op = EVENT_PLAY;
		event.time = 0;

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
	EVENT event;
	EVENT *q_event;

	const INTRO_CREDIT credits[] = {
		{kITEAny, kCHeader, "Producer"},
		{kITEAny, kCText, "Walter Hochbrueckner"},
		{kITEAny, kCHeader, "Executive Producer"},
		{kITEAny, kCText, "Robert McNally"},
		{kITEWyrmKeep, kCHeader, "2nd Executive Producer"},
		{kITENotWyrmKeep, kCHeader, "Publisher"},
		{kITEAny, kCText, "Jon Van Caneghem"}
	};

	int n_credits = ARRAYSIZE(credits);

	switch (param) {
	case SCENE_BEGIN:
		// Begin title screen background animation 
		_vm->_anim->setCycles(0, -1);

		event.type = ONESHOT_EVENT;
		event.code = ANIM_EVENT;
		event.op = EVENT_PLAY;
		event.param = 0;
		event.time = 0;

		q_event = _vm->_events->queue(&event);

		// Begin ITE title theme music
		_vm->_music->stop();

		event.type = ONESHOT_EVENT;
		event.code = MUSIC_EVENT;
		event.param = MUSIC_2;
		event.param2 = 0;
		event.op = EVENT_PLAY;
		event.time = 0;
		
		q_event = _vm->_events->chain(q_event, &event);

		// Pause animation before logo
		event.type = ONESHOT_EVENT;
		event.code = ANIM_EVENT;
		event.op = EVENT_STOP;
		event.param = 0;
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
		event.op = EVENT_PLAY;
		event.time = 0;
		event.param = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Queue game credits list
		q_event = ITEQueueCredits(scene_info, 12000, CREDIT_DURATION1, n_credits, credits);

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
	EVENT event;
	EVENT *q_event;

	const INTRO_CREDIT credits1[] = {
		{kITEAny, kCHeader, "Game Design"},
		{kITEAny, kCText, "Talin, Joe Pearce, Robert McNally"},
		{kITEAny, kCText, "and Carolly Hauksdottir"},
		{kITEAny, kCHeader, "Screenplay and Dialog"},
		{kITEAny, kCText, "Robert Leh, Len Wein, and Bill Rotsler"}
	};

	int n_credits1 = ARRAYSIZE(credits1);

	const INTRO_CREDIT credits2[] = {
		{kITEWyrmKeep, kCHeader, "Art Direction"},
		{kITEWyrmKeep, kCText, "Allison Hershey"},
		{kITEAny, kCHeader, "Art"},
		{kITEAny, kCText, "Edward Lacabanne, Glenn Price, April Lee,"},
		{kITEWyrmKeep, kCText, "Lisa Sample, Brian Dowrick, Reed Waller,"},
		{kITEWyrmKeep, kCText, "Allison Hershey and Talin"},
		{kITENotWyrmKeep, kCText, "Lisa Iennaco, Brian Dowrick, Reed"},
		{kITENotWyrmKeep, kCText, "Waller, Allison Hershey and Talin"},
		{kITENotWyrmKeep, kCHeader, "Art Direction"},
		{kITENotWyrmKeep, kCText, "Allison Hershey"}
	};

	int n_credits2 = ARRAYSIZE(credits2);

	switch (param) {
	case SCENE_BEGIN:
		// Start 'dissolve' transition to new scene background
		event.type = CONTINUOUS_EVENT;
		event.code = TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = _vm->_events->queue(&event);

		// Begin title screen background animation 
		_vm->_anim->setFrameTime(0, 100);

		event.type = ONESHOT_EVENT;
		event.code = ANIM_EVENT;
		event.op = EVENT_PLAY;
		event.param = 0;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Queue game credits list
		q_event = ITEQueueCredits(scene_info, DISSOLVE_DURATION + 2000, CREDIT_DURATION1, n_credits1, credits1);
		q_event = ITEQueueCredits(scene_info, DISSOLVE_DURATION + 7000, CREDIT_DURATION1, n_credits2, credits2);

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
	EVENT event;
	EVENT *q_event;

	const INTRO_CREDIT credits1[] = {
		{kITEAny, kCHeader, "Programming"},
		{kITEAny, kCText, "Talin, Walter Hochbrueckner,"},
		{kITEAny, kCText, "Joe Burks and Robert Wiggins"},
		{kITEPCCD | kITEWyrmKeep, kCHeader, "Additional Programming"},
		{kITEPCCD | kITEWyrmKeep, kCText, "John Bolton"},
		{kITEMac, kCHeader, "Macintosh Version"},
		{kITEMac, kCText, "Michael McNally and Robert McNally"},
		{kITEAny, kCHeader, "Music and Sound"},
		{kITEAny, kCText, "Matt Nathan"}
	};

	int n_credits1 = ARRAYSIZE(credits1);

	const INTRO_CREDIT credits2[] = {
		{kITEAny, kCHeader, "Directed by"},
		{kITEAny, kCText, "Talin"}
	};

	int n_credits2 = ARRAYSIZE(credits2);

	switch (param) {
	case SCENE_BEGIN:
		// Start 'dissolve' transition to new scene background
		event.type = CONTINUOUS_EVENT;
		event.code = TRANSITION_EVENT;
		event.op = EVENT_DISSOLVE;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;

		q_event = _vm->_events->queue(&event);

		// Begin title screen background animation 
		_vm->_anim->setCycles(0, -1);

		event.type = ONESHOT_EVENT;
		event.code = ANIM_EVENT;
		event.op = EVENT_PLAY;
		event.param = 0;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Queue game credits list
		q_event = ITEQueueCredits(scene_info, DISSOLVE_DURATION + 2000, CREDIT_DURATION1, n_credits1, credits1);
		q_event = ITEQueueCredits(scene_info, DISSOLVE_DURATION + 7000, CREDIT_DURATION1, n_credits2, credits2);

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
