/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/engine/state.h"
#include "sci/sfx/player.h"
#include "sci/engine/kernel.h"
#include "sci/engine/vm.h"		// for Object

namespace Sci {

#define _K_SCI0_SOUND_INIT_HANDLE 0
#define _K_SCI0_SOUND_PLAY_HANDLE 1
#define _K_SCI0_SOUND_NOP 2
#define _K_SCI0_SOUND_DISPOSE_HANDLE 3
#define _K_SCI0_SOUND_MUTE_SOUND 4
#define _K_SCI0_SOUND_STOP_HANDLE 5
#define _K_SCI0_SOUND_SUSPEND_HANDLE 6
#define _K_SCI0_SOUND_RESUME_HANDLE 7
#define _K_SCI0_SOUND_VOLUME 8
#define _K_SCI0_SOUND_UPDATE_VOL_PRI 9
#define _K_SCI0_SOUND_FADE_HANDLE 10
#define _K_SCI0_SOUND_GET_POLYPHONY 11
#define _K_SCI0_SOUND_PLAY_NEXT 12

#define _K_SCI01_SOUND_MASTER_VOLME 0 /* Set/Get */
#define _K_SCI01_SOUND_MUTE_SOUND 1
#define _K_SCI01_SOUND_UNUSED 2
#define _K_SCI01_SOUND_GET_POLYPHONY 3
#define _K_SCI01_SOUND_UPDATE_HANDLE 4
#define _K_SCI01_SOUND_INIT_HANDLE 5
#define _K_SCI01_SOUND_DISPOSE_HANDLE 6
#define _K_SCI01_SOUND_PLAY_HANDLE 7
#define _K_SCI01_SOUND_STOP_HANDLE 8
#define _K_SCI01_SOUND_SUSPEND_HANDLE 9 /* or resume */
#define _K_SCI01_SOUND_FADE_HANDLE 10
#define _K_SCI01_SOUND_UPDATE_CUES 11
#define _K_SCI01_SOUND_MIDI_SEND 12
#define _K_SCI01_SOUND_REVERB 13 /* Get/Set */
#define _K_SCI01_SOUND_HOLD 14

#define _K_SCI1_SOUND_MASTER_VOLME 0 /* Set/Get */
#define _K_SCI1_SOUND_MUTE_SOUND 1
#define _K_SCI1_SOUND_UNUSED1 2
#define _K_SCI1_SOUND_GET_POLYPHONY 3
#define _K_SCI1_SOUND_GET_AUDIO_CAPABILITY 4
#define _K_SCI1_SOUND_SUSPEND_SOUND 5
#define _K_SCI1_SOUND_INIT_HANDLE 6
#define _K_SCI1_SOUND_DISPOSE_HANDLE 7
#define _K_SCI1_SOUND_PLAY_HANDLE 8
#define _K_SCI1_SOUND_STOP_HANDLE 9
#define _K_SCI1_SOUND_SUSPEND_HANDLE 10 /* or resume */
#define _K_SCI1_SOUND_FADE_HANDLE 11
#define _K_SCI1_SOUND_HOLD_HANDLE 12
#define _K_SCI1_SOUND_UNUSED2 13
#define _K_SCI1_SOUND_SET_HANDLE_VOLUME 14
#define _K_SCI1_SOUND_SET_HANDLE_PRIORITY 15
#define _K_SCI1_SOUND_SET_HANDLE_LOOP 16
#define _K_SCI1_SOUND_UPDATE_CUES 17
#define _K_SCI1_SOUND_MIDI_SEND 18
#define _K_SCI1_SOUND_REVERB 19 /* Get/Set */
#define _K_SCI1_SOUND_UPDATE_VOL_PRI 20

enum AudioCommands {
	// TODO: find the difference between kSci1AudioWPlay and kSci1AudioPlay
	kSci1AudioWPlay = 1, /* Plays an audio stream */
	kSci1AudioPlay = 2, /* Plays an audio stream */
	kSci1AudioStop = 3, /* Stops an audio stream */
	kSci1AudioPause = 4, /* Pauses an audio stream */
	kSci1AudioResume = 5, /* Resumes an audio stream */
	kSci1AudioPosition = 6, /* Return current position in audio stream */
	kSci1AudioRate = 7, /* Return audio rate */
	kSci1AudioVolume = 8, /* Return audio volume */
	kSci1AudioLanguage = 9 /* Return audio language */
};

#define SCI1_SOUND_FLAG_MAY_PAUSE        1 /* Only here for completeness; The interpreter doesn't touch this bit */
#define SCI1_SOUND_FLAG_SCRIPTED_PRI     2 /* but does touch this */

#define FROBNICATE_HANDLE(reg) ((reg).segment << 16 | (reg).offset)
#define DEFROBNICATE_HANDLE(handle) (make_reg((handle >> 16) & 0xffff, handle & 0xffff))
#define SCRIPT_ASSERT_ZERO(fun) if (fun) script_debug_flag = script_error_flag = 1;


static void script_set_priority(EngineState *s, reg_t obj, int priority) {
	int song_nr = GET_SEL32V(obj, number);
	Resource *song = s->resmgr->findResource(kResourceTypeSound, song_nr, 0);
	int flags = GET_SEL32V(obj, flags);

	if (priority == -1) {
		if (song->data[0] == 0xf0)
			priority = song->data[1];
		else
			warning("Attempt to unset song priority when there is no built-in value");

		flags &= ~SCI1_SOUND_FLAG_SCRIPTED_PRI;
	} else flags |= SCI1_SOUND_FLAG_SCRIPTED_PRI;

	sfx_song_renice(&s->sound, FROBNICATE_HANDLE(obj), priority);
	PUT_SEL32V(obj, flags, flags);
}

SongIterator *build_iterator(EngineState *s, int song_nr, int type, songit_id_t id) {
	Resource *song = s->resmgr->findResource(kResourceTypeSound, song_nr, 0);

	if (!song)
		return NULL;

	return songit_new(song->data, song->size, type, id);
}


void process_sound_events(EngineState *s) { /* Get all sound events, apply their changes to the heap */
	int result;
	song_handle_t handle;
	int cue;

	if (s->version >= SCI_VERSION_FTU_DOSOUND_VARIANT_1)
		return;
	/* SCI01 and later explicitly poll for everything */

	while ((result = sfx_poll(&s->sound, &handle, &cue))) {
		reg_t obj = DEFROBNICATE_HANDLE(handle);
		if (!is_object(s, obj)) {
			warning("Non-object "PREG" received sound signal (%d/%d)", PRINT_REG(obj), result, cue);
			return;
		}

		switch (result) {

		case SI_LOOP:
			SCIkdebug(SCIkSOUND, "[process-sound] Song "PREG" looped (to %d)\n",
			          PRINT_REG(obj), cue);
			/*			PUT_SEL32V(obj, loops, GET_SEL32V(obj, loop) - 1);*/
			PUT_SEL32V(obj, signal, -1);
			break;

		case SI_RELATIVE_CUE:
			SCIkdebug(SCIkSOUND, "[process-sound] Song "PREG" received relative cue %d\n",
			          PRINT_REG(obj), cue);
			PUT_SEL32V(obj, signal, cue + 0x7f);
			break;

		case SI_ABSOLUTE_CUE:
			SCIkdebug(SCIkSOUND, "[process-sound] Song "PREG" received absolute cue %d\n",
			          PRINT_REG(obj), cue);
			PUT_SEL32V(obj, signal, cue);
			break;

		case SI_FINISHED:
			SCIkdebug(SCIkSOUND, "[process-sound] Song "PREG" finished\n",
			          PRINT_REG(obj));
			PUT_SEL32V(obj, signal, -1);
			PUT_SEL32V(obj, state, _K_SOUND_STATUS_STOPPED);
			break;

		default:
			sciprintf("Unexpected result from sfx_poll: %d\n", result);
			break;
		}
	}
}


reg_t kDoSound_SCI0(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t obj = KP_ALT(1, NULL_REG);
	uint16 command = UKPV(0);
	song_handle_t handle = FROBNICATE_HANDLE(obj);
	int number = obj.segment ?
	             GET_SEL32V(obj, number) :
	             -1; /* We were not going to use it anyway */

	if (s->debug_mode & (1 << SCIkSOUNDCHK_NR)) {
		int i;

		SCIkdebug(SCIkSOUND, "Command 0x%x", command);
		switch (command) {
		case 0:
			sciprintf("[InitObj]");
			break;
		case 1:
			sciprintf("[Play]");
			break;
		case 2:
			sciprintf("[NOP]");
			break;
		case 3:
			sciprintf("[DisposeHandle]");
			break;
		case 4:
			sciprintf("[SetSoundOn(?)]");
			break;
		case 5:
			sciprintf("[Stop]");
			break;
		case 6:
			sciprintf("[Suspend]");
			break;
		case 7:
			sciprintf("[Resume]");
			break;
		case 8:
			sciprintf("[Get(Set?)Volume]");
			break;
		case 9:
			sciprintf("[Signal: Obj changed]");
			break;
		case 10:
			sciprintf("[Fade(?)]");
			break;
		case 11:
			sciprintf("[ChkDriver]");
			break;
		case 12:
			sciprintf("[PlayNextSong (formerly StopAll)]");
			break;
		default:
			sciprintf("[unknown]");
			break;
		}

		sciprintf("(");
		for (i = 1; i < argc; i++) {
			sciprintf(PREG, PRINT_REG(argv[i]));
			if (i + 1 < argc)
				sciprintf(", ");
		}
		sciprintf(")\n");
	}


	switch (command) {
	case _K_SCI0_SOUND_INIT_HANDLE:
		if (obj.segment) {
			sciprintf("Initializing song number %d\n", GET_SEL32V(obj, number));
			SCRIPT_ASSERT_ZERO(sfx_add_song(&s->sound,
			                                build_iterator(s, number,
			                                               SCI_SONG_ITERATOR_TYPE_SCI0,
			                                               handle),
			                                0, handle, number));
			PUT_SEL32V(obj, state, _K_SOUND_STATUS_INITIALIZED);
			PUT_SEL32(obj, handle, obj); /* ``sound handle'': we use the object address */
		}
		break;

	case _K_SCI0_SOUND_PLAY_HANDLE:
		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_PLAYING);
			sfx_song_set_loops(&s->sound,
			                   handle, GET_SEL32V(obj, loop));
			PUT_SEL32V(obj, state, _K_SOUND_STATUS_PLAYING);
		}
		break;

	case _K_SCI0_SOUND_NOP:
		break;

	case _K_SCI0_SOUND_DISPOSE_HANDLE:
		if (obj.segment) {
			sfx_remove_song(&s->sound, handle);
		}
		PUT_SEL32V(obj, handle, 0x0000);
		break;

	case _K_SCI0_SOUND_STOP_HANDLE:
		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_STOPPED);
			PUT_SEL32V(obj, state, SOUND_STATUS_STOPPED);
		}
		break;

	case _K_SCI0_SOUND_SUSPEND_HANDLE:
		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_SUSPENDED);
			PUT_SEL32V(obj, state, SOUND_STATUS_SUSPENDED);
		}
		break;

	case _K_SCI0_SOUND_RESUME_HANDLE:
		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_PLAYING);
			PUT_SEL32V(obj, state, SOUND_STATUS_PLAYING);
		}
		break;

	case _K_SCI0_SOUND_MUTE_SOUND: {
		/* if there's a parameter, we're setting it.  Otherwise,
		   we're querying it. */
		/*int param = UPARAM_OR_ALT(1,-1);

		if (param != -1)
		s->acc = s->sound_server->command(s, SOUND_COMMAND_SET_MUTE, 0, param);
		else
		s->acc = s->sound_server->command(s, SOUND_COMMAND_GET_MUTE, 0, 0);*/

	}
	break;

	case _K_SCI0_SOUND_VOLUME: {
		/* range from 0x0 to 0xf */
		/* parameter optional. If present, set.*/
		int vol = SKPV_OR_ALT(1, -1);

		if (vol != -1)
			sfx_set_volume(&s->sound, vol << 0xf);
		else
			s->r_acc = make_reg(0, sfx_get_volume(&s->sound) >> 0xf);
	}
	break;

	case _K_SCI0_SOUND_UPDATE_VOL_PRI:
		if (obj.segment) {
			sfx_song_set_loops(&s->sound,
			                   handle, GET_SEL32V(obj, loop));
			script_set_priority(s, obj, GET_SEL32V(obj, pri));
		}
		break;

	case _K_SCI0_SOUND_FADE_HANDLE:
		/*s->sound_server->command(s, SOUND_COMMAND_FADE_HANDLE, obj, 120);*/ /* Fade out in 2 secs */
		/* FIXME: The next couple of lines actually STOP the handle, rather
		** than fading it! */
		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_STOPPED);
			PUT_SEL32V(obj, state, SOUND_STATUS_STOPPED);
			PUT_SEL32V(obj, signal, -1);
		}
		break;

	case _K_SCI0_SOUND_GET_POLYPHONY:
		s->r_acc = make_reg(0, sfx_get_player_polyphony());
		break;

	case _K_SCI0_SOUND_PLAY_NEXT:
		/* sfx_all_stop(&s->sound);*/
		break;

	default:
		warning("Unhandled DoSound command: %x", command);

	}
	//	process_sound_events(s); /* Take care of incoming events */

	return s->r_acc;
}


reg_t kDoSound_SCI01(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	uint16 command = UKPV(0);
	reg_t obj = KP_ALT(1, NULL_REG);
	song_handle_t handle = FROBNICATE_HANDLE(obj);
	int number = obj.segment ?
	             GET_SEL32V(obj, number) :
	             -1; /* We were not going to use it anyway */

	if ((s->debug_mode & (1 << SCIkSOUNDCHK_NR))
	        && command != _K_SCI01_SOUND_UPDATE_CUES) {
		int i;

		SCIkdebug(SCIkSOUND, "Command 0x%x", command);
		switch (command) {
		case 0:
			sciprintf("[MasterVolume]");
			break;
		case 1:
			sciprintf("[Mute]");
			break;
		case 2:
			sciprintf("[NOP(2)]");
			break;
		case 3:
			sciprintf("[GetPolyphony]");
			break;
		case 4:
			sciprintf("[Update]");
			break;
		case 5:
			sciprintf("[Init]");
			break;
		case 6:
			sciprintf("[Dispose]");
			break;
		case 7:
			sciprintf("[Play]");
			break;
		case 8:
			sciprintf("[Stop]");
			break;
		case 9:
			sciprintf("[Suspend]");
			break;
		case 10:
			sciprintf("[Fade]");
			break;
		case 11:
			sciprintf("[UpdateCues]");
			break;
		case 12:
			sciprintf("[MidiSend]");
			break;
		case 13:
			sciprintf("[Reverb]");
			break;
		case 14:
			sciprintf("[Hold]");
			break;
		default:
			sciprintf("[unknown]");
			break;
		}

		sciprintf("(");
		for (i = 1; i < argc; i++) {
			sciprintf(PREG, PRINT_REG(argv[i]));
			if (i + 1 < argc)
				sciprintf(", ");
		}
		sciprintf(")\n");
	}

	switch (command) {
	case _K_SCI01_SOUND_MASTER_VOLME : {
		int vol = SKPV_OR_ALT(1, -1);

		if (vol != -1)
			sfx_set_volume(&s->sound, vol << 0xf);
		else
			s->r_acc = make_reg(0, sfx_get_volume(&s->sound) >> 0xf);
		break;
	}
	case _K_SCI01_SOUND_MUTE_SOUND : {
		/* if there's a parameter, we're setting it.  Otherwise,
		   we're querying it. */
		/*int param = UPARAM_OR_ALT(1,-1);

		if (param != -1)
			s->acc = s->sound_server->command(s, SOUND_COMMAND_SET_MUTE, 0, param);
		else
		s->acc = s->sound_server->command(s, SOUND_COMMAND_GET_MUTE, 0, 0);*/

		break;
	}
	case _K_SCI01_SOUND_UNUSED : {
		break;
	}
	case _K_SCI01_SOUND_GET_POLYPHONY : {
		s->r_acc = make_reg(0, sfx_get_player_polyphony());
		break;
	}
	case _K_SCI01_SOUND_PLAY_HANDLE : {
		int looping = GET_SEL32V(obj, loop);
		//int vol = GET_SEL32V(obj, vol);
		int pri = GET_SEL32V(obj, pri);
		RESTORE_BEHAVIOR rb = (RESTORE_BEHAVIOR) UKPV(2);		/* Too lazy to look up a default value for this */

		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_PLAYING);
			sfx_song_set_loops(&s->sound,
			                   handle, looping);
			sfx_song_renice(&s->sound,
			                handle, pri);
			song_lib_set_restore_behavior(s->sound.songlib, handle, rb);
		}

		break;
	}
	case _K_SCI01_SOUND_INIT_HANDLE : {
		//int looping = GET_SEL32V(obj, loop);
		//int vol = GET_SEL32V(obj, vol);
		//int pri = GET_SEL32V(obj, pri);

		if (obj.segment && (s->resmgr->testResource(kResourceTypeSound, number))) {
			sciprintf("Initializing song number %d\n", number);
			SCRIPT_ASSERT_ZERO(sfx_add_song(&s->sound,
			                                build_iterator(s, number,
			                                               SCI_SONG_ITERATOR_TYPE_SCI1,
			                                               handle),
			                                0, handle, number));
			PUT_SEL32(obj, nodePtr, obj);
			PUT_SEL32(obj, handle, obj);
		}
		break;
	}
	case _K_SCI01_SOUND_DISPOSE_HANDLE : {
		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_STOPPED);
			sfx_remove_song(&s->sound, handle);
		}
		break;
	}
	case _K_SCI01_SOUND_UPDATE_HANDLE : {
		/* FIXME: Get these from the sound server */
		int signal = 0;
		int min = 0;
		int sec = 0;
		int frame = 0;

		/* FIXME: Update the sound server state with 'vol' */
		int looping = GET_SEL32V(obj, loop);
		//int vol = GET_SEL32V(obj, vol);
		int pri = GET_SEL32V(obj, pri);

		sfx_song_set_loops(&s->sound,
		                   handle, looping);
		sfx_song_renice(&s->sound, handle, pri);

		SCIkdebug(SCIkSOUND, "[sound01-update-handle] -- CUE "PREG);

		PUT_SEL32V(obj, signal, signal);
		PUT_SEL32V(obj, min, min);
		PUT_SEL32V(obj, sec, sec);
		PUT_SEL32V(obj, frame, frame);

		break;
	}
	case _K_SCI01_SOUND_STOP_HANDLE : {
		PUT_SEL32V(obj, signal, -1);
		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_STOPPED);
		}
		break;
	}
	case _K_SCI01_SOUND_SUSPEND_HANDLE : {
		int state = UKPV(2);
		int setstate = (state) ?
		               SOUND_STATUS_SUSPENDED : SOUND_STATUS_PLAYING;

		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, setstate);
		}
		break;
	}
	case _K_SCI01_SOUND_FADE_HANDLE : {
		/* There are four parameters that control the fade here.
		 * TODO: Figure out the exact semantics */

		/* FIXME: The next couple of lines actually STOP the song right away */
		PUT_SEL32V(obj, signal, -1);
		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_STOPPED);
		}
		break;
	}
	case _K_SCI01_SOUND_UPDATE_CUES : {
		int signal = 0;
		int min = 0;
		int sec = 0;
		int frame = 0;
		int result = SI_LOOP; /* small hack */
		int cue = 0;

		while (result == SI_LOOP)
			result = sfx_poll_specific(&s->sound, handle, &cue);

		switch (result) {

		case SI_ABSOLUTE_CUE:
			signal = cue;
			SCIkdebug(SCIkSOUND, "---    [CUE] "PREG" Absolute Cue: %d\n",
			          PRINT_REG(obj), signal);

			PUT_SEL32V(obj, signal, signal);
			break;

		case SI_RELATIVE_CUE:
			signal = cue;
			SCIkdebug(SCIkSOUND, "---    [CUE] "PREG" Relative Cue: %d\n",
			          PRINT_REG(obj), cue);

			/* FIXME to match commented-out semantics
			 * below, with proper storage of dataInc and
			 * signal in the iterator code. */
			PUT_SEL32V(obj, dataInc, signal);
			PUT_SEL32V(obj, signal, signal);
			break;

		case SI_FINISHED:
			SCIkdebug(SCIkSOUND, "---    [FINISHED] "PREG"\n", PRINT_REG(obj));
			PUT_SEL32V(obj, signal, 0xffff);
			break;

		case SI_LOOP:
			break; /* Doesn't happen */
		}

		/*		switch (signal) */
		/*		{ */
		/*		case 0x00: */
		/*			if (dataInc!=GET_SEL32V(obj, dataInc)) */
		/*			{ */
		/*				PUT_SEL32V(obj, dataInc, dataInc); */
		/*				PUT_SEL32V(obj, signal, dataInc+0x7f); */
		/*			} else */
		/*			{ */
		/*				PUT_SEL32V(obj, signal, signal); */
		/*			} */
		/*			break; */
		/*		case 0xFF: /\* May be unnecessary *\/ */
		/*			sfx_song_set_status(&s->sound, */
		/*					    handle, SOUND_STATUS_STOPPED); */
		/*			break; */
		/*		default : */
		/*			if (dataInc!=GET_SEL32V(obj, dataInc)) */
		/*			{ */
		/*				PUT_SEL32V(obj, dataInc, dataInc); */
		/*				PUT_SEL32V(obj, signal, dataInc+0x7f); */
		/*			} else */
		/*			{ */
		/*				PUT_SEL32V(obj, signal, signal); */
		/*			} */
		/*			break; */
		/*		} */

		PUT_SEL32V(obj, min, min);
		PUT_SEL32V(obj, sec, sec);
		PUT_SEL32V(obj, frame, frame);
		break;
	}
	case _K_SCI01_SOUND_MIDI_SEND : {
		int channel = SKPV(2);
		int midiCmd = UKPV(3) == 0xff ?
		              0xe0 : /* Pitch wheel */
		              0xb0; /* UKPV(3) is actually a controller number */
		int controller = UKPV(3);
		int param = UKPV(4);

		sfx_send_midi(&s->sound, handle,
		              channel, midiCmd, controller, param);
		break;
	}
	case _K_SCI01_SOUND_REVERB : {
		break;
	}
	case _K_SCI01_SOUND_HOLD : {
		//int flag = SKPV(2);
		break;
	}
	}

	return s->r_acc;
}

reg_t kDoSound_SCI1(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	uint16 command = UKPV(0);
	reg_t obj = KP_ALT(1, NULL_REG);
	song_handle_t handle = FROBNICATE_HANDLE(obj);
	int number = obj.segment ?
	             GET_SEL32V(obj, number) :
	             -1; /* We were not going to use it anyway */

	CHECK_THIS_KERNEL_FUNCTION;

	if ((s->debug_mode & (1 << SCIkSOUNDCHK_NR))
	        && command != _K_SCI1_SOUND_UPDATE_CUES) {
		int i;

		SCIkdebug(SCIkSOUND, "Command 0x%x", command);
		switch (command) {
		case 0:
			sciprintf("[MasterVolume]");
			break;
		case 1:
			sciprintf("[Mute]");
			break;
		case 2:
			sciprintf("[NOP(2)]");
			break;
		case 3:
			sciprintf("[GetPolyphony]");
			break;
		case 4:
			sciprintf("[GetAudioCapability]");
			break;
		case 5:
			sciprintf("[GlobalSuspend]");
			break;
		case 6:
			sciprintf("[Init]");
			break;
		case 7:
			sciprintf("[Dispose]");
			break;
		case 8:
			sciprintf("[Play]");
			break;
		case 9:
			sciprintf("[Stop]");
			break;
		case 10:
			sciprintf("[SuspendHandle]");
			break;
		case 11:
			sciprintf("[Fade]");
			break;
		case 12:
			sciprintf("[Hold]");
			break;
		case 13:
			sciprintf("[Unused(13)]");
			break;
		case 14:
			sciprintf("[SetVolume]");
			break;
		case 15:
			sciprintf("[SetPriority]");
			break;
		case 16:
			sciprintf("[SetLoop]");
			break;
		case 17:
			sciprintf("[UpdateCues]");
			break;
		case 18:
			sciprintf("[MidiSend]");
			break;
		case 19:
			sciprintf("[Reverb]");
			break;
		case 20:
			sciprintf("[UpdateVolPri]");
			break;
		default:
			sciprintf("[unknown]");
			break;
		}

		sciprintf("(");
		for (i = 1; i < argc; i++) {
			sciprintf(PREG, PRINT_REG(argv[i]));
			if (i + 1 < argc)
				sciprintf(", ");
		}
		sciprintf(")\n");
	}

	switch (command) {
	case _K_SCI1_SOUND_MASTER_VOLME : {
		/*int vol = UPARAM_OR_ALT (1, -1);

		 if (vol != -1)
		         s->acc = s->sound_server->command(s, SOUND_COMMAND_SET_VOLUME, 0, vol);
		 else
		         s->acc = s->sound_server->command(s, SOUND_COMMAND_GET_VOLUME, 0, 0);
			break;*/
	}
	case _K_SCI1_SOUND_MUTE_SOUND : {
		/* if there's a parameter, we're setting it.  Otherwise,
		   we're querying it. */
		/*int param = UPARAM_OR_ALT(1,-1);

		if (param != -1)
			s->acc = s->sound_server->command(s, SOUND_COMMAND_SET_MUTE, 0, param);
		else
			s->acc = s->sound_server->command(s, SOUND_COMMAND_GET_MUTE, 0, 0);
			break;*/
	}
	case _K_SCI1_SOUND_UNUSED1 : {
		break;
	}
	case _K_SCI1_SOUND_GET_POLYPHONY : {
		/*s->acc = s->sound_server->command(s, SOUND_COMMAND_TEST, 0, 0);*/
		break;
	}
	case _K_SCI1_SOUND_GET_AUDIO_CAPABILITY : {
		return NULL_REG;
	}
	case _K_SCI1_SOUND_PLAY_HANDLE : {
		int looping = GET_SEL32V(obj, loop);
		//int vol = GET_SEL32V(obj, vol);
		int pri = GET_SEL32V(obj, pri);
		song_t *song = song_lib_find(s->sound.songlib, handle);

		if (GET_SEL32V(obj, nodePtr) && (song && number != song->resource_num)) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_STOPPED);
			sfx_remove_song(&s->sound, handle);
			PUT_SEL32(obj, nodePtr, NULL_REG);
		}

		if (!GET_SEL32V(obj, nodePtr) && obj.segment) {
			if (!s->resmgr->testResource(kResourceTypeSound, number)) {
				sciprintf("Could not open song number %d\n", number);
				return NULL_REG;
			}

			sciprintf("Initializing song number %d\n", number);
			SCRIPT_ASSERT_ZERO(sfx_add_song(&s->sound,
			                                build_iterator(s, number,
			                                               SCI_SONG_ITERATOR_TYPE_SCI1,
			                                               handle),
			                                0, handle, number));
			PUT_SEL32(obj, nodePtr, obj);
			PUT_SEL32(obj, handle, obj);
		}

		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_PLAYING);
			sfx_song_set_loops(&s->sound,
			                   handle, looping);
			sfx_song_renice(&s->sound,
			                handle, pri);
		}

		break;
	}
	case _K_SCI1_SOUND_INIT_HANDLE : {
		//int looping = GET_SEL32V(obj, loop);
		//int vol = GET_SEL32V(obj, vol);
		//int pri = GET_SEL32V(obj, pri);

		if (GET_SEL32V(obj, nodePtr)) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_STOPPED);
			sfx_remove_song(&s->sound, handle);
		}

		if (obj.segment && (s->resmgr->testResource(kResourceTypeSound, number))) {
			sciprintf("Initializing song number %d\n", number);
			SCRIPT_ASSERT_ZERO(sfx_add_song(&s->sound,
			                                build_iterator(s, number,
			                                               SCI_SONG_ITERATOR_TYPE_SCI1,
			                                               handle),
			                                0, handle, number));
			PUT_SEL32(obj, nodePtr, obj);
			PUT_SEL32(obj, handle, obj);
		}
		break;
	}
	case _K_SCI1_SOUND_DISPOSE_HANDLE : {
		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_STOPPED);
			sfx_remove_song(&s->sound, handle);
		}
		break;
	}
	case _K_SCI1_SOUND_STOP_HANDLE : {
		PUT_SEL32V(obj, signal, -1);
		if (obj.segment) {
			sfx_song_set_status(&s->sound,
			                    handle, SOUND_STATUS_STOPPED);
		}
		break;
	}
	case _K_SCI1_SOUND_SUSPEND_HANDLE : {
		break;
	}
	case _K_SCI1_SOUND_FADE_HANDLE : {
		fade_params_t fade;
		if (obj.segment) {
			fade.final_volume = UKPV(2);
			fade.ticks_per_step = UKPV(3);
			fade.step_size = UKPV(4);
			fade.action = UKPV(5) ?
			              FADE_ACTION_FADE_AND_STOP :
			              FADE_ACTION_FADE_AND_CONT;

			sfx_song_set_fade(&s->sound,
			                  handle,
			                  &fade);

			/* FIXME: The next couple of lines actually STOP the handle, rather
			** than fading it! */
			if (UKPV(5)) {
				PUT_SEL32V(obj, signal, -1);
				PUT_SEL32V(obj, nodePtr, 0);
				PUT_SEL32V(obj, handle, 0);
				sfx_song_set_status(&s->sound,
				                    handle, SOUND_STATUS_STOPPED);
			} else {
				// FIXME: Support fade-and-continue. For now, send signal right away.
				PUT_SEL32V(obj, signal, -1);
			}
		}
		break;
	}
	case _K_SCI1_SOUND_HOLD_HANDLE : {
		int value = SKPV(2);

		sfx_song_set_hold(&s->sound,
		                  handle, value);
		break;
	}
	case _K_SCI1_SOUND_UNUSED2 : {
		break;
	}
	case _K_SCI1_SOUND_SET_HANDLE_VOLUME : {
		break;
	}
	case _K_SCI1_SOUND_SET_HANDLE_PRIORITY : {
		int value = SKPV(2);

		script_set_priority(s, obj, value);
		break;
	}
	case _K_SCI1_SOUND_SET_HANDLE_LOOP : {
		break;
	}
	case _K_SCI1_SOUND_UPDATE_CUES : {
		int signal = 0;
		//int min = 0;
		//int sec = 0;
		//int frame = 0;
		int result = SI_LOOP; /* small hack */
		int cue = 0;

		while (result == SI_LOOP)
			result = sfx_poll_specific(&s->sound, handle, &cue);

		switch (result) {

		case SI_ABSOLUTE_CUE:
			signal = cue;
			fprintf(stderr, "[CUE] "PREG" Absolute Cue: %d\n",
			        PRINT_REG(obj), signal);

			PUT_SEL32V(obj, signal, signal);
			break;

		case SI_RELATIVE_CUE:
			fprintf(stderr, "[CUE] "PREG" Relative Cue: %d\n",
			        PRINT_REG(obj), cue);

			PUT_SEL32V(obj, dataInc, cue);
			PUT_SEL32V(obj, signal, cue + 127);
			break;

		case SI_FINISHED:
			PUT_SEL32V(obj, signal, 0xffff);
			break;

		case SI_LOOP:
			break; /* Doesn't happen */
		}
		break;
	}
	case _K_SCI1_SOUND_MIDI_SEND : {
		sfx_send_midi(&s->sound, handle,
		              UKPV(2), UKPV(3), UKPV(4), UKPV(5));
		break;
	}
	case _K_SCI1_SOUND_REVERB : {
		break;
	}
	case _K_SCI1_SOUND_UPDATE_VOL_PRI : {
		break;
	}
	}
	return s->r_acc;
}

reg_t kDoSound(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	if (s->version >= SCI_VERSION_FTU_DOSOUND_VARIANT_2)
		return kDoSound_SCI1(s, funct_nr, argc, argv);
	else if (s->version >= SCI_VERSION_FTU_DOSOUND_VARIANT_1)
		return kDoSound_SCI01(s, funct_nr, argc, argv);
	else
		return kDoSound_SCI0(s, funct_nr, argc, argv);
}

reg_t kDoAudio(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	switch (UKPV(0)) {
	case kSci1AudioWPlay:
		break;
	case kSci1AudioPlay:
		break;
	case kSci1AudioStop:
		break;
	case kSci1AudioPause:
		break;
	case kSci1AudioResume:
		break;
	case kSci1AudioPosition :
		return make_reg(0, -1); /* Finish immediately */
	case kSci1AudioRate:
		break;
	case kSci1AudioVolume:
		break;
	case kSci1AudioLanguage:
		break;
	}

	return s->r_acc;
}

reg_t kDoSync(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	switch (UKPV(0)) {
	case 0:	// start sync
		//printf("kDoSync: start sync\n");
		if (s->sound.soundSync) {
			s->resmgr->unlockResource(s->sound.soundSync, s->sound.soundSync->number, kResourceTypeSync);
		}

		// Load sound sync resource and lock it
		s->sound.soundSync = (ResourceSync *)s->resmgr->findResource(kResourceTypeSync, UKPV(2), 1);

		if (s->sound.soundSync) {
			s->sound.soundSync->startSync(s, argv[1]);
		} else {
			// Notify the scripts to stop sound sync
			PUT_SEL32V(argv[1], syncCue, -1);
		}
		break;
	case 1:	// next sync
		//printf("kDoSync: next sync\n");
		if (s->sound.soundSync) {
			s->sound.soundSync->nextSync(s, argv[1]);
		}
		break;
	case 2:	// stop sync
		//printf("kDoSync: stop sync\n");
		if (s->sound.soundSync) {
			s->sound.soundSync->stopSync();
			s->resmgr->unlockResource(s->sound.soundSync, s->sound.soundSync->number, kResourceTypeSync);
			s->sound.soundSync = NULL;
		}
		break;
	}

	return s->r_acc;
}

} // End of namespace Sci
