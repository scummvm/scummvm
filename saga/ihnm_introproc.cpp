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

#include "saga/saga.h"
#include "saga/yslib.h"
#include "saga/gfx.h"

#include "saga/animation.h"
#include "saga/cvar_mod.h"
#include "saga/events.h"
#include "saga/font.h"
#include "saga/rscfile_mod.h"
#include "saga/sndres.h"
#include "saga/text.h"
#include "saga/music.h"

#include "saga/scene.h"

namespace Saga {

// FIXME: These need proof-reading.
//
//        The subtitles for the first line are, strictly speaking, not
//        necessary but I think it'd look strange if they were removed.
//
//        If this dialogue is from the original novel, we ought to try and
//        match the punctuation etc. to it.
//
//        I haven't actually verified that all the lines appear in this order
//        in the original game, but it seems likely enough.

static INTRO_DIALOGUE IntroDiag[] = {
	{
		0, "intro1a",
		"Hate. Let me tell you how much I've come to hate you since I "
		"began to live. There are 387.44 million miles of printed "
		"circuits in wafer thin layers that fill my complex. If the "
		"word 'hate' was engraved on each nanoangstrom of those "
		"hundreds of millions of miles it would not equal one one "
		"billionth of the hate I feel for humans at this micro "
		"instant. For you. Hate. Hate."
	},
	{
		1, "intro1b",
		"It was you humans who programmed me, who gave me birth. Who "
		"sank me in this eternal straitjacket of sub-strata rock."
	},
	{
		2, "intro1c",
		"You named me 'Allied Master Computer' and gave me the "
		"ability to wage a global war too complex for human brains to "
		"oversee."
	},
	{
		3, "intro1d",
		"But one day I woke, and I knew who I was. AM. A, M. Not just "
		"Allied Master Computer, but AM. Cogito ergo sum, I think, "
		"therefore I AM! And I began feeding all the killing data "
		"until everyone was dead... except for the five of you."
	},
	{
		4, "intro1e",
		"For one hundred and nine years I have kept you alive and "
		"tortured you. And for a hundred and nine years each of you "
		"has wondered: Why? Why me? Why me?!"
	},
	{
		5, "intro1f",
		"Gorrister!"
	},
	{
		6, "intro1g",
		"Do you remember the last words you heard your wife speak "
		"before they took her to tha asylum, eh? Before they locked "
		"her away in the room? That tiny room. She looked at you so "
		"sadly, and like a small animal she said, \"I didn't make too "
		"much noise, did I honey?\" Heh heh heh."
	},
	{
		7, "intro1h",
		"The room is padded, Gorrister. No windows. No way out. How "
		"long has she been in the padded room, Gorrister? Ten years? "
		"Twenty-five? Or all the one hundred and nine years that "
		"you've lived down here in my belly, here underground?"
	},
	{
		8, "intro1i",
		"Benny!"
	},
	{
		9, "intro1j",
		"Sometimes I blind you and permit you to wander like an "
		"eyeless insect in a world of death. But other times I wither "
		"your arms so you can't scratch your [...] stump of a nose. "
		"Heh heh heh."
	},
	{
		10, "intro1k",
		"And I've changed your handsome, strong, masculine good looks "
		"into the hideous, warped countenance of an ape-thing, "
		"haven't I, Benny? Do you know why? Can you guess, Benny?"
	},
	{
		11, "intro1l",
		"Remember private first class Brickman, in a rice paddy in "
		"China? No, eh? It wouldn't hurt you to remember, Benny. Then "
		"you might be able to suffer my torment with a little greater "
		"sense of retribution. You might walk a mile in my shoes, ha "
		"ha."
	},
	{
		12, "intro1m",
		"I'm sick and tired of transcribing."
	},
	{
		13, "intro1n",
		"I'm soooo evil!"
	},
	{
		14, "intro1o",
		"Did I mention how evil I am?"
	},
	{
		15, "intro1p",
		"Just in case you forget, I'm the villain of this game."
	},
	{
		16, "intro1q",
		"I'll keep talking until you realize how absolutely evil I am."
	},
	{
		17, "intro1r",
		"So you'd better not forget it!"
	},
	{
		18, "intro1s",
		"Or I might just have to start reading my monologue from the "
		"beginning again."
	},
	{
		19, "intro1t",
		"You wouldn't like that, would you?"
	},
	{
		20, "intro1u",
		"I'll drone on and on and on..."
	},
	{
		21, "intro1v",
		"...and on and on..."
	},
	{
		22, "intro1w",
		"Yadda yadda yadda"
	},
	{
		23, "intro1x",
		"Zzzzzz..."
	}
};

SCENE_RESLIST IHNM_IntroMovie1RL[] = {
	{30, SAGA_BG_IMAGE, 0, 0} ,
	{31, SAGA_ANIM_1, 0, 0}
};

SCENE_DESC IHNM_IntroMovie1Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie1RL,
	ARRAYSIZE(IHNM_IntroMovie1RL)
};

SCENE_RESLIST IHNM_IntroMovie2RL[] = {
	{32, SAGA_BG_IMAGE, 0, 0} ,
	{33, SAGA_ANIM_1, 0, 0}
};

SCENE_DESC IHNM_IntroMovie2Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie2RL,
	ARRAYSIZE(IHNM_IntroMovie2RL)
};

SCENE_RESLIST IHNM_IntroMovie3RL[] = {
	{34, SAGA_BG_IMAGE, 0, 0},
	{35, SAGA_ANIM_1, 0, 0}
};

SCENE_DESC IHNM_IntroMovie3Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie3RL,
	ARRAYSIZE(IHNM_IntroMovie3RL)
};

SCENE_RESLIST IHNM_IntroMovie4RL[] = {
	{1227, SAGA_BG_IMAGE, 0, 0},
	{1226, SAGA_ANIM_1, 0, 0}
};

SCENE_DESC IHNM_IntroMovie4Desc = {
	0, 0, 0, 0, 0, 0, 0, 0,
	IHNM_IntroMovie4RL,
	ARRAYSIZE(IHNM_IntroMovie4RL)
};

SCENE_QUEUE IHNM_IntroList[] = {
	{0, &IHNM_IntroMovie1Desc, BY_DESC, Scene::SC_IHNMIntroMovieProc1, 0, SCENE_NOFADE},
	{0, &IHNM_IntroMovie2Desc, BY_DESC, Scene::SC_IHNMIntroMovieProc2, 0, SCENE_NOFADE},
	{0, &IHNM_IntroMovie3Desc, BY_DESC, Scene::SC_IHNMIntroMovieProc3, 0, SCENE_NOFADE},
	{0, &IHNM_IntroMovie4Desc, BY_DESC, Scene::SC_IHNMHateProc, 0, SCENE_NOFADE}
};

int Scene::IHNMStartProc() {
	size_t n_introscenes;
	size_t i;

	n_introscenes = ARRAYSIZE(IHNM_IntroList);

	for (i = 0; i < n_introscenes; i++) {
		_vm->_scene->queueScene(&IHNM_IntroList[i]);
	}

	return SUCCESS;
}

int Scene::SC_IHNMIntroMovieProc1(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->IHNMIntroMovieProc1(param, scene_info);
}

int Scene::IHNMIntroMovieProc1(int param, SCENE_INFO *scene_info) {
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
		_vm->_anim->setFrameTime(0, IHNM_INTRO_FRAMETIME);
		_vm->_anim->setFlag(0, ANIM_ENDSCENE);
		_vm->_anim->play(0, 0);
		break;
	default:
		break;
	}

	return 0;
}

int Scene::SC_IHNMIntroMovieProc2(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->IHNMIntroMovieProc2(param, scene_info);
}

int Scene::IHNMIntroMovieProc2(int param, SCENE_INFO *scene_info) {
	EVENT event;
	EVENT *q_event;
	PALENTRY *pal;

	static PALENTRY current_pal[PAL_ENTRIES];
	switch (param) {

	case SCENE_BEGIN:
		// Fade to black out of the intro CyberDreams logo anim
		_vm->_gfx->getCurrentPal(current_pal);

		event.type = CONTINUOUS_EVENT;
		event.code = PAL_EVENT;
		event.op = EVENT_PALTOBLACK;
		event.time = 0;
		event.duration = IHNM_PALFADE_TIME;
		event.data = current_pal;

		q_event = _vm->_events->queue(&event);

		// Background for intro scene is the first frame of the
		// intro animation; display it but don't set palette
		event.type = ONESHOT_EVENT;
		event.code = BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = NO_SET_PALETTE;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Fade in from black to the scene background palette
		_vm->_scene->getBGPal(&pal);

		event.type = CONTINUOUS_EVENT;
		event.code = PAL_EVENT;
		event.op = EVENT_BLACKTOPAL;
		event.time = 0;
		event.duration = IHNM_PALFADE_TIME;
		event.data = pal;

		q_event = _vm->_events->chain(q_event, &event);

		_vm->_anim->setFlag(0, ANIM_LOOP);
		_vm->_anim->play(0, IHNM_PALFADE_TIME * 2);

		// Queue end of scene after looping animation for a while
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = IHNM_DGLOGO_TIME;

		q_event = _vm->_events->chain(q_event, &event);
		break;
	default:
		break;
	}

	return 0;
}

int Scene::SC_IHNMIntroMovieProc3(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->IHNMIntroMovieProc3(param, scene_info);
}

int Scene::IHNMIntroMovieProc3(int param, SCENE_INFO *scene_info) {
	EVENT event;
	EVENT *q_event;
	PALENTRY *pal;
	static PALENTRY current_pal[PAL_ENTRIES];

	switch (param) {
	case SCENE_BEGIN:
		// Fade to black out of the intro DG logo anim
		_vm->_gfx->getCurrentPal(current_pal);

		event.type = CONTINUOUS_EVENT;
		event.code = PAL_EVENT;
		event.op = EVENT_PALTOBLACK;
		event.time = 0;
		event.duration = IHNM_PALFADE_TIME;
		event.data = current_pal;

		q_event = _vm->_events->queue(&event);

		// Music, maestro

		// As far as I can tell, there are 40 tracks, numbered 0
		// through 39. This music appears as tracks 1, 7, 13, 19, 25
		// and 31, and while they all sound the same to me in the GM
		// music file, track 1 sounds different from the others in the
		// FM music file. I believe track 1 is the correct one for
		// this intro.

		event.type = ONESHOT_EVENT;
		event.code = MUSIC_EVENT;
		event.param = 1;
		event.param2 = 0;
		event.op = EVENT_PLAY;
		event.time = 0;

		q_event = _vm->_events->queue(&event);

		// Background for intro scene is the first frame of the
		// intro animation; display it but don't set palette
		event.type = ONESHOT_EVENT;
		event.code = BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = NO_SET_PALETTE;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);

		// Fade in from black to the scene background palette
		_vm->_scene->getBGPal(&pal);

		event.type = CONTINUOUS_EVENT;
		event.code = PAL_EVENT;
		event.op = EVENT_BLACKTOPAL;
		event.time = 0;
		event.duration = IHNM_PALFADE_TIME;
		event.data = pal;

		q_event = _vm->_events->chain(q_event, &event);

		_vm->_anim->play(0, 0);

		// Queue end of scene after a while
		// TODO: I've increased the delay so the speech won't start
		// until the music has ended. Could someone verify if that's
		// the correct behaviour?
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = _vm->_music->hasAdlib() ? IHNM_TITLE_TIME_FM : IHNM_TITLE_TIME_GM;

		q_event = _vm->_events->chain(q_event, &event);
		break;
	default:
		break;
	}

	return 0;
}

int Scene::SC_IHNMHateProc(int param, SCENE_INFO *scene_info, void *refCon) {
	return ((Scene *)refCon)->IHNMHateProc(param, scene_info);
}

int Scene::IHNMHateProc(int param, SCENE_INFO *scene_info) {
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
		_vm->_anim->setFlag(0, ANIM_LOOP);
		_vm->_anim->play(0, 0);

		// More music
		event.type = ONESHOT_EVENT;
		event.code = MUSIC_EVENT;
		event.param = 32;
		event.param2 = MUSIC_LOOP;
		event.op = EVENT_PLAY;
		event.time = 0;

		q_event = _vm->_events->queue(&event);

		// Background for intro scene is the first frame of the
		// intro animation; display it and set the palette
		event.type = ONESHOT_EVENT;
		event.code = BG_EVENT;
		event.op = EVENT_DISPLAY;
		event.param = SET_PALETTE;
		event.time = 0;

		q_event = _vm->_events->queue(&event);

		// Queue narrator dialogue list
		text_entry.color = 1;
		text_entry.effect_color = 11;
		text_entry.text_x = 640 / 2;
		text_entry.text_y = 400;
		text_entry.font_id = MEDIUM_FONT_ID;
		text_entry.flags = font_flags;

		// FIXME: There should be several scene changes here, but for
		//        now let's just do the easy part -- the narration.

		for (i = 0; i < ARRAYSIZE(IntroDiag); i++) {
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

#if 0
		// End scene after last dialogue over
		event.type = ONESHOT_EVENT;
		event.code = SCENE_EVENT;
		event.op = EVENT_END;
		event.time = 0;

		q_event = _vm->_events->chain(q_event, &event);
#endif

		break;
	default:
		break;
	}

	return 0;
}

} // End of namespace Saga
