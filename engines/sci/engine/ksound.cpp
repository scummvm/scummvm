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

#include "sci/sci.h"
#include "sci/engine/state.h"
//#include "sci/sfx/player.h"
#include "sci/sfx/iterator.h"
#include "sci/sfx/misc.h"
#include "sci/engine/kernel.h"
#include "sci/engine/vm.h"		// for Object

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Sci {

/* Sound status */
enum {
	_K_SOUND_STATUS_STOPPED = 0,
	_K_SOUND_STATUS_INITIALIZED = 1,
	_K_SOUND_STATUS_PAUSED = 2,
	_K_SOUND_STATUS_PLAYING = 3
};

enum {
	_K_SCI0_SOUND_INIT_HANDLE = 0,
	_K_SCI0_SOUND_PLAY_HANDLE = 1,
	_K_SCI0_SOUND_NOP = 2,
	_K_SCI0_SOUND_DISPOSE_HANDLE = 3,
	_K_SCI0_SOUND_MUTE_SOUND = 4,
	_K_SCI0_SOUND_STOP_HANDLE = 5,
	_K_SCI0_SOUND_SUSPEND_HANDLE = 6,
	_K_SCI0_SOUND_RESUME_HANDLE = 7,
	_K_SCI0_SOUND_VOLUME = 8,
	_K_SCI0_SOUND_UPDATE_VOL_PRI = 9,
	_K_SCI0_SOUND_FADE_HANDLE = 10,
	_K_SCI0_SOUND_GET_POLYPHONY = 11,
	_K_SCI0_SOUND_PLAY_NEXT = 12
};

enum {
	_K_SCI01_SOUND_MASTER_VOLME = 0, /* Set/Get */
	_K_SCI01_SOUND_MUTE_SOUND = 1,
	_K_SCI01_SOUND_UNUSED = 2,
	_K_SCI01_SOUND_GET_POLYPHONY = 3,
	_K_SCI01_SOUND_UPDATE_HANDLE = 4,
	_K_SCI01_SOUND_INIT_HANDLE = 5,
	_K_SCI01_SOUND_DISPOSE_HANDLE = 6,
	_K_SCI01_SOUND_PLAY_HANDLE = 7,
	_K_SCI01_SOUND_STOP_HANDLE = 8,
	_K_SCI01_SOUND_SUSPEND_HANDLE = 9, /* or resume */
	_K_SCI01_SOUND_FADE_HANDLE = 10,
	_K_SCI01_SOUND_UPDATE_CUES = 11,
	_K_SCI01_SOUND_MIDI_SEND = 12,
	_K_SCI01_SOUND_REVERB = 13, /* Get/Set */
	_K_SCI01_SOUND_HOLD = 14
};

enum {
	_K_SCI1_SOUND_MASTER_VOLME = 0, /* Set/Get */
	_K_SCI1_SOUND_MUTE_SOUND = 1,
	_K_SCI1_SOUND_UNUSED1 = 2,
	_K_SCI1_SOUND_GET_POLYPHONY = 3,
	_K_SCI1_SOUND_GET_AUDIO_CAPABILITY = 4,
	_K_SCI1_SOUND_SUSPEND_SOUND = 5,
	_K_SCI1_SOUND_INIT_HANDLE = 6,
	_K_SCI1_SOUND_DISPOSE_HANDLE = 7,
	_K_SCI1_SOUND_PLAY_HANDLE = 8,
	_K_SCI1_SOUND_STOP_HANDLE = 9,
	_K_SCI1_SOUND_SUSPEND_HANDLE = 10, /* or resume */
	_K_SCI1_SOUND_FADE_HANDLE = 11,
	_K_SCI1_SOUND_HOLD_HANDLE = 12,
	_K_SCI1_SOUND_UNUSED2 = 13,
	_K_SCI1_SOUND_SET_HANDLE_VOLUME = 14,
	_K_SCI1_SOUND_SET_HANDLE_PRIORITY = 15,
	_K_SCI1_SOUND_SET_HANDLE_LOOP = 16,
	_K_SCI1_SOUND_UPDATE_CUES = 17,
	_K_SCI1_SOUND_MIDI_SEND = 18,
	_K_SCI1_SOUND_REVERB = 19, /* Get/Set */
	_K_SCI1_SOUND_UPDATE_VOL_PRI = 20
};

enum AudioCommands {
	// TODO: find the difference between kSci1AudioWPlay and kSci1AudioPlay
	kSciAudioWPlay = 1, /* Plays an audio stream */
	kSciAudioPlay = 2, /* Plays an audio stream */
	kSciAudioStop = 3, /* Stops an audio stream */
	kSciAudioPause = 4, /* Pauses an audio stream */
	kSciAudioResume = 5, /* Resumes an audio stream */
	kSciAudioPosition = 6, /* Return current position in audio stream */
	kSciAudioRate = 7, /* Return audio rate */
	kSciAudioVolume = 8, /* Return audio volume */
	kSciAudioLanguage = 9 /* Return audio language */
};

enum AudioSyncCommands {
	kSciAudioSyncStart = 0,
	kSciAudioSyncNext = 1,
	kSciAudioSyncStop = 2
};

#define SCI1_SOUND_FLAG_MAY_PAUSE        1 /* Only here for completeness; The interpreter doesn't touch this bit */
#define SCI1_SOUND_FLAG_SCRIPTED_PRI     2 /* but does touch this */
//#define DEBUG_SOUND	// enable for sound debugging

#define FROBNICATE_HANDLE(reg) ((reg).segment << 16 | (reg).offset)
#define DEFROBNICATE_HANDLE(handle) (make_reg((handle >> 16) & 0xffff, handle & 0xffff))


static void script_set_priority(EngineState *s, reg_t obj, int priority) {
	SegManager *segMan = s->_segMan;
	int song_nr = GET_SEL32V(obj, number);
	Resource *song = s->resMan->findResource(ResourceId(kResourceTypeSound, song_nr), 0);
	int flags = GET_SEL32V(obj, flags);

	if (priority == -1) {
		if (song->data[0] == 0xf0)
			priority = song->data[1];
		else
			warning("Attempt to unset song priority when there is no built-in value");

		flags &= ~SCI1_SOUND_FLAG_SCRIPTED_PRI;
	} else flags |= SCI1_SOUND_FLAG_SCRIPTED_PRI;

	s->_sound.sfx_song_renice(FROBNICATE_HANDLE(obj), priority);
	PUT_SEL32V(obj, flags, flags);
}

SongIterator *build_iterator(EngineState *s, int song_nr, SongIteratorType type, songit_id_t id) {
	Resource *song = s->resMan->findResource(ResourceId(kResourceTypeSound, song_nr), 0);

	if (!song)
		return NULL;

	return songit_new(song->data, song->size, type, id);
}

SongIterator *build_timeriterator(EngineState *s, int delta) {
	return new_timer_iterator(delta);
}

void process_sound_events(EngineState *s) { /* Get all sound events, apply their changes to the heap */
	int result;
	SongHandle handle;
	int cue;
	SegManager *segMan = s->_segMan;

	if (getSciVersion() > SCI_VERSION_01)
		return;
	/* SCI1 and later explicitly poll for everything */

	while ((result = s->_sound.sfx_poll(&handle, &cue))) {
		reg_t obj = DEFROBNICATE_HANDLE(handle);
		if (!s->_segMan->isObject(obj)) {
			warning("Non-object %04x:%04x received sound signal (%d/%d)", PRINT_REG(obj), result, cue);
			return;
		}

		switch (result) {

		case SI_LOOP:
			debugC(2, kDebugLevelSound, "[process-sound] Song %04x:%04x looped (to %d)\n",
			          PRINT_REG(obj), cue);
			/*			PUT_SEL32V(obj, loops, GET_SEL32V(obj, loop) - 1);*/
			PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
			break;

		case SI_RELATIVE_CUE:
			debugC(2, kDebugLevelSound, "[process-sound] Song %04x:%04x received relative cue %d\n",
			          PRINT_REG(obj), cue);
			PUT_SEL32V(obj, signal, cue + 0x7f);
			break;

		case SI_ABSOLUTE_CUE:
			debugC(2, kDebugLevelSound, "[process-sound] Song %04x:%04x received absolute cue %d\n",
			          PRINT_REG(obj), cue);
			PUT_SEL32V(obj, signal, cue);
			break;

		case SI_FINISHED:
			debugC(2, kDebugLevelSound, "[process-sound] Song %04x:%04x finished\n",
			          PRINT_REG(obj));
			PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
			PUT_SEL32V(obj, state, _K_SOUND_STATUS_STOPPED);
			break;

		default:
			warning("Unexpected result from sfx_poll: %d", result);
			break;
		}
	}
}


static reg_t kDoSoundSci0(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t obj = (argc > 1) ? argv[1] : NULL_REG;
	uint16 command = argv[0].toUint16();
	SongHandle handle = FROBNICATE_HANDLE(obj);
	int number = obj.segment ?
	             GET_SEL32V(obj, number) :
	             -1; /* We were not going to use it anyway */

#ifdef DEBUG_SOUND
	int i;

	debugC(2, kDebugLevelSound, "Command 0x%x", command);
	switch (command) {
	case 0:
		debugC(2, kDebugLevelSound, "[InitObj]");
		break;
	case 1:
		debugC(2, kDebugLevelSound, "[Play]");
		break;
	case 2:
		debugC(2, kDebugLevelSound, "[NOP]");
		break;
	case 3:
		debugC(2, kDebugLevelSound, "[DisposeHandle]");
		break;
	case 4:
		debugC(2, kDebugLevelSound, "[SetSoundOn(?)]");
		break;
	case 5:
		debugC(2, kDebugLevelSound, "[Stop]");
		break;
	case 6:
		debugC(2, kDebugLevelSound, "[Suspend]");
		break;
	case 7:
		debugC(2, kDebugLevelSound, "[Resume]");
		break;
	case 8:
		debugC(2, kDebugLevelSound, "[Get(Set?)Volume]");
		break;
	case 9:
		debugC(2, kDebugLevelSound, "[Signal: Obj changed]");
		break;
	case 10:
		debugC(2, kDebugLevelSound, "[Fade(?)]");
		break;
	case 11:
		debugC(2, kDebugLevelSound, "[ChkDriver]");
		break;
	case 12:
		debugC(2, kDebugLevelSound, "[PlayNextSong (formerly StopAll)]");
		break;
	default:
		debugC(2, kDebugLevelSound, "[unknown]");
		break;
	}

	debugC(2, kDebugLevelSound, "(");
	for (i = 1; i < argc; i++) {
		debugC(2, kDebugLevelSound, "%04x:%04x", PRINT_REG(argv[i]));
		if (i + 1 < argc)
			debugC(2, kDebugLevelSound, ", ");
	}
	debugC(2, kDebugLevelSound, ")\n");
#endif	// DEBUG_SOUND


	switch (command) {
	case _K_SCI0_SOUND_INIT_HANDLE:
		if (obj.segment) {
			debugC(2, kDebugLevelSound, "Initializing song number %d\n", GET_SEL32V(obj, number));
			s->_sound.sfx_add_song(build_iterator(s, number, SCI_SONG_ITERATOR_TYPE_SCI0,
			                                               handle), 0, handle, number);

			PUT_SEL32V(obj, state, _K_SOUND_STATUS_INITIALIZED);
			PUT_SEL32(obj, handle, obj); /* ``sound handle'': we use the object address */
		}
		break;

	case _K_SCI0_SOUND_PLAY_HANDLE:
		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_PLAYING);
			s->_sound.sfx_song_set_loops(handle, GET_SEL32V(obj, loop));
			PUT_SEL32V(obj, state, _K_SOUND_STATUS_PLAYING);
		}
		break;

	case _K_SCI0_SOUND_NOP:
		break;

	case _K_SCI0_SOUND_DISPOSE_HANDLE:
		if (obj.segment) {
			s->_sound.sfx_remove_song(handle);
		}
		PUT_SEL32V(obj, handle, 0x0000);
		break;

	case _K_SCI0_SOUND_STOP_HANDLE:
		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
			PUT_SEL32V(obj, state, SOUND_STATUS_STOPPED);
		}
		break;

	case _K_SCI0_SOUND_SUSPEND_HANDLE:
		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_SUSPENDED);
			PUT_SEL32V(obj, state, SOUND_STATUS_SUSPENDED);
		}
		break;

	case _K_SCI0_SOUND_RESUME_HANDLE:
		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_PLAYING);
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
		int vol = (argc > 1) ? argv[1].toSint16() : -1;

		if (vol != -1)
			s->_sound.sfx_set_volume(vol << 0xf);
		else
			s->r_acc = make_reg(0, s->_sound.sfx_get_volume() >> 0xf);
	}
	break;

	case _K_SCI0_SOUND_UPDATE_VOL_PRI:
		if (obj.segment) {
			s->_sound.sfx_song_set_loops(handle, GET_SEL32V(obj, loop));
			script_set_priority(s, obj, GET_SEL32V(obj, pri));
		}
		break;

	case _K_SCI0_SOUND_FADE_HANDLE:
		/*s->sound_server->command(s, SOUND_COMMAND_FADE_HANDLE, obj, 120);*/ /* Fade out in 2 secs */
		/* FIXME: The next couple of lines actually STOP the handle, rather
		** than fading it! */
		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
			PUT_SEL32V(obj, state, SOUND_STATUS_STOPPED);
			PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
		}
		break;

	case _K_SCI0_SOUND_GET_POLYPHONY:
		s->r_acc = make_reg(0, sfx_get_player_polyphony());
		break;

	case _K_SCI0_SOUND_PLAY_NEXT:
		/* s->_sound.sfx_all_stop();*/
		break;

	default:
		warning("Unhandled DoSound command: %x", command);

	}
	//	process_sound_events(s); /* Take care of incoming events */

	return s->r_acc;
}


static reg_t kDoSoundSci1Early(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	uint16 command = argv[0].toUint16();
	reg_t obj = (argc > 1) ? argv[1] : NULL_REG;
	SongHandle handle = FROBNICATE_HANDLE(obj);
	int number = obj.segment ?
	             GET_SEL32V(obj, number) :
	             -1; /* We were not going to use it anyway */

#ifdef DEBUG_SOUND
	if (command != _K_SCI01_SOUND_UPDATE_CUES) {
		int i;

		debugC(2, kDebugLevelSound, "Command 0x%x", command);
		switch (command) {
		case 0:
			debugC(2, kDebugLevelSound, "[MasterVolume]");
			break;
		case 1:
			debugC(2, kDebugLevelSound, "[Mute]");
			break;
		case 2:
			debugC(2, kDebugLevelSound, "[NOP(2)]");
			break;
		case 3:
			debugC(2, kDebugLevelSound, "[GetPolyphony]");
			break;
		case 4:
			debugC(2, kDebugLevelSound, "[Update]");
			break;
		case 5:
			debugC(2, kDebugLevelSound, "[Init]");
			break;
		case 6:
			debugC(2, kDebugLevelSound, "[Dispose]");
			break;
		case 7:
			debugC(2, kDebugLevelSound, "[Play]");
			break;
		case 8:
			debugC(2, kDebugLevelSound, "[Stop]");
			break;
		case 9:
			debugC(2, kDebugLevelSound, "[Suspend]");
			break;
		case 10:
			debugC(2, kDebugLevelSound, "[Fade]");
			break;
		case 11:
			debugC(2, kDebugLevelSound, "[UpdateCues]");
			break;
		case 12:
			debugC(2, kDebugLevelSound, "[MidiSend]");
			break;
		case 13:
			debugC(2, kDebugLevelSound, "[Reverb]");
			break;
		case 14:
			debugC(2, kDebugLevelSound, "[Hold]");
			break;
		default:
			debugC(2, kDebugLevelSound, "[unknown]");
			break;
		}

		debugC(2, kDebugLevelSound, "(");
		for (i = 1; i < argc; i++) {
			debugC(2, kDebugLevelSound, "%04x:%04x", PRINT_REG(argv[i]));
			if (i + 1 < argc)
				debugC(2, kDebugLevelSound, ", ");
		}
		debugC(2, kDebugLevelSound, ")\n");
	}
#endif

	switch (command) {
	case _K_SCI01_SOUND_MASTER_VOLME : {
		int vol = (argc > 1) ? argv[1].toSint16() : -1;

		if (vol != -1)
			s->_sound.sfx_set_volume(vol << 0xf);
		else
			s->r_acc = make_reg(0, s->_sound.sfx_get_volume() >> 0xf);
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
		RESTORE_BEHAVIOR rb = (RESTORE_BEHAVIOR) argv[2].toUint16();		/* Too lazy to look up a default value for this */

		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_PLAYING);
			s->_sound.sfx_song_set_loops(handle, looping);
			s->_sound.sfx_song_renice(handle, pri);
			s->_sound._songlib.setSongRestoreBehavior(handle, rb);
			PUT_SEL32V(obj, signal, 0);
		}

		break;
	}
	case _K_SCI01_SOUND_INIT_HANDLE : {
		//int looping = GET_SEL32V(obj, loop);
		//int vol = GET_SEL32V(obj, vol);
		//int pri = GET_SEL32V(obj, pri);

		if (obj.segment && (s->resMan->testResource(ResourceId(kResourceTypeSound, number)))) {
			debugC(2, kDebugLevelSound, "Initializing song number %d\n", number);
			s->_sound.sfx_add_song(build_iterator(s, number, SCI_SONG_ITERATOR_TYPE_SCI1,
			                                      handle), 0, handle, number);
			PUT_SEL32(obj, nodePtr, obj);
			PUT_SEL32(obj, handle, obj);
		}
		break;
	}
	case _K_SCI01_SOUND_DISPOSE_HANDLE : {
		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
			s->_sound.sfx_remove_song(handle);
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

		s->_sound.sfx_song_set_loops(handle, looping);
		s->_sound.sfx_song_renice(handle, pri);

		debugC(2, kDebugLevelSound, "[sound01-update-handle] -- CUE %04x:%04x", PRINT_REG(obj));

		PUT_SEL32V(obj, signal, signal);
		PUT_SEL32V(obj, min, min);
		PUT_SEL32V(obj, sec, sec);
		PUT_SEL32V(obj, frame, frame);

		break;
	}
	case _K_SCI01_SOUND_STOP_HANDLE : {
		PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
		}
		break;
	}
	case _K_SCI01_SOUND_SUSPEND_HANDLE : {
		int state = argv[2].toUint16();
		int setstate = (state) ?
		               SOUND_STATUS_SUSPENDED : SOUND_STATUS_PLAYING;

		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, setstate);
		}
		break;
	}
	case _K_SCI01_SOUND_FADE_HANDLE : {
		/* There are four parameters that control the fade here.
		 * TODO: Figure out the exact semantics */

		/* FIXME: The next couple of lines actually STOP the song right away */
		PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
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
			result = s->_sound.sfx_poll_specific(handle, &cue);

		switch (result) {

		case SI_ABSOLUTE_CUE:
			signal = cue;
			debugC(2, kDebugLevelSound, "---    [CUE] %04x:%04x Absolute Cue: %d\n",
			          PRINT_REG(obj), signal);

			PUT_SEL32V(obj, signal, signal);
			break;

		case SI_RELATIVE_CUE:
			signal = cue;
			debugC(2, kDebugLevelSound, "---    [CUE] %04x:%04x Relative Cue: %d\n",
			          PRINT_REG(obj), cue);

			/* FIXME to match commented-out semantics
			 * below, with proper storage of dataInc and
			 * signal in the iterator code. */
			PUT_SEL32V(obj, dataInc, signal);
			PUT_SEL32V(obj, signal, signal);
			break;

		case SI_FINISHED:
			debugC(2, kDebugLevelSound, "---    [FINISHED] %04x:%04x\n", PRINT_REG(obj));
			PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
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
		/*			s->_sound.sfx_song_set_status(*/
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
		int channel = argv[2].toSint16();
		int midiCmd = argv[3].toUint16() == 0xff ?
		              0xe0 : /* Pitch wheel */
		              0xb0; /* argv[3].toUint16() is actually a controller number */
		int controller = argv[3].toUint16();
		int param = argv[4].toUint16();

		s->_sound.sfx_send_midi(handle,
		              channel, midiCmd, controller, param);
		break;
	}
	case _K_SCI01_SOUND_REVERB : {
		break;
	}
	case _K_SCI01_SOUND_HOLD : {
		//int flag = argv[2].toSint16();
		break;
	}
	}

	return s->r_acc;
}

static reg_t kDoSoundSci1Late(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	uint16 command = argv[0].toUint16();
	reg_t obj = (argc > 1) ? argv[1] : NULL_REG;
	SongHandle handle = FROBNICATE_HANDLE(obj);
	int number = obj.segment ?
	             GET_SEL32V(obj, number) :
	             -1; /* We were not going to use it anyway */

#ifdef DEBUG_SOUND
	if (command != _K_SCI1_SOUND_UPDATE_CUES) {
		int i;

		debugC(2, kDebugLevelSound, "Command 0x%x", command);
		switch (command) {
		case 0:
			debugC(2, kDebugLevelSound, "[MasterVolume]");
			break;
		case 1:
			debugC(2, kDebugLevelSound, "[Mute]");
			break;
		case 2:
			debugC(2, kDebugLevelSound, "[NOP(2)]");
			break;
		case 3:
			debugC(2, kDebugLevelSound, "[GetPolyphony]");
			break;
		case 4:
			debugC(2, kDebugLevelSound, "[GetAudioCapability]");
			break;
		case 5:
			debugC(2, kDebugLevelSound, "[GlobalSuspend]");
			break;
		case 6:
			debugC(2, kDebugLevelSound, "[Init]");
			break;
		case 7:
			debugC(2, kDebugLevelSound, "[Dispose]");
			break;
		case 8:
			debugC(2, kDebugLevelSound, "[Play]");
			break;
		case 9:
			debugC(2, kDebugLevelSound, "[Stop]");
			break;
		case 10:
			debugC(2, kDebugLevelSound, "[SuspendHandle]");
			break;
		case 11:
			debugC(2, kDebugLevelSound, "[Fade]");
			break;
		case 12:
			debugC(2, kDebugLevelSound, "[Hold]");
			break;
		case 13:
			debugC(2, kDebugLevelSound, "[Unused(13)]");
			break;
		case 14:
			debugC(2, kDebugLevelSound, "[SetVolume]");
			break;
		case 15:
			debugC(2, kDebugLevelSound, "[SetPriority]");
			break;
		case 16:
			debugC(2, kDebugLevelSound, "[SetLoop]");
			break;
		case 17:
			debugC(2, kDebugLevelSound, "[UpdateCues]");
			break;
		case 18:
			debugC(2, kDebugLevelSound, "[MidiSend]");
			break;
		case 19:
			debugC(2, kDebugLevelSound, "[Reverb]");
			break;
		case 20:
			debugC(2, kDebugLevelSound, "[UpdateVolPri]");
			break;
		default:
			debugC(2, kDebugLevelSound, "[unknown]");
			break;
		}

		debugC(2, kDebugLevelSound, "(");
		for (i = 1; i < argc; i++) {
			debugC(2, kDebugLevelSound, "%04x:%04x", PRINT_REG(argv[i]));
			if (i + 1 < argc)
				debugC(2, kDebugLevelSound, ", ");
		}
		debugC(2, kDebugLevelSound, ")\n");
	}
#endif	// DEBUG_SOUND

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
		// Tests for digital audio support
		return make_reg(0, 1);
	}
	case _K_SCI1_SOUND_PLAY_HANDLE : {
		int looping = GET_SEL32V(obj, loop);
		//int vol = GET_SEL32V(obj, vol);
		int pri = GET_SEL32V(obj, pri);
		int sampleLen = 0;
		Song *song = s->_sound._songlib.findSong(handle);

		if (GET_SEL32V(obj, nodePtr) && (song && number != song->_resourceNum)) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
			s->_sound.sfx_remove_song(handle);
			PUT_SEL32(obj, nodePtr, NULL_REG);
		}

		if (!GET_SEL32V(obj, nodePtr) && obj.segment) {
			// In SCI1.1 games, sound effects are started from here. If we can find
			// a relevant audio resource, play it, otherwise switch to synthesized
			// effects. If the resource exists, play it using map 65535 (sound
			// effects map)
			if (s->resMan->testResource(ResourceId(kResourceTypeAudio, number)) &&
				getSciVersion() >= SCI_VERSION_1_1) {
				// Found a relevant audio resource, play it
				s->_sound.stopAudio();
				warning("Initializing audio resource instead of requested sound resource %d", number);
				sampleLen = s->_sound.startAudio(65535, number);
				// Also create iterator, that will fire SI_FINISHED event, when the sound is done playing
				s->_sound.sfx_add_song(build_timeriterator(s, sampleLen), 0, handle, number);
			} else {
				if (!s->resMan->testResource(ResourceId(kResourceTypeSound, number))) {
					warning("Could not open song number %d", number);
					// Send a "stop handle" event so that the engine won't wait forever here
					s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
					PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
					return s->r_acc;
				}
				debugC(2, kDebugLevelSound, "Initializing song number %d\n", number);
				s->_sound.sfx_add_song(build_iterator(s, number, SCI_SONG_ITERATOR_TYPE_SCI1,
				                          handle), 0, handle, number);
			}

			PUT_SEL32(obj, nodePtr, obj);
			PUT_SEL32(obj, handle, obj);
		}

		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_PLAYING);
			s->_sound.sfx_song_set_loops(handle, looping);
			s->_sound.sfx_song_renice(handle, pri);
			PUT_SEL32V(obj, signal, 0);
		}

		break;
	}
	case _K_SCI1_SOUND_INIT_HANDLE : {
		//int looping = GET_SEL32V(obj, loop);
		//int vol = GET_SEL32V(obj, vol);
		//int pri = GET_SEL32V(obj, pri);

		if (GET_SEL32V(obj, nodePtr)) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
			s->_sound.sfx_remove_song(handle);
		}

		if (obj.segment && (s->resMan->testResource(ResourceId(kResourceTypeSound, number)))) {
			debugC(2, kDebugLevelSound, "Initializing song number %d\n", number);
			s->_sound.sfx_add_song(build_iterator(s, number, SCI_SONG_ITERATOR_TYPE_SCI1,
			                                    handle), 0, handle, number);
			PUT_SEL32(obj, nodePtr, obj);
			PUT_SEL32(obj, handle, obj);
		}
		break;
	}
	case _K_SCI1_SOUND_DISPOSE_HANDLE : {
		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
			s->_sound.sfx_remove_song(handle);
		}
		break;
	}
	case _K_SCI1_SOUND_STOP_HANDLE : {
		PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
		if (obj.segment) {
			s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
		}
		break;
	}
	case _K_SCI1_SOUND_SUSPEND_HANDLE : {
		break;
	}
	case _K_SCI1_SOUND_FADE_HANDLE : {
		fade_params_t fade;
		if (obj.segment) {
			fade.final_volume = argv[2].toUint16();
			fade.ticks_per_step = argv[3].toUint16();
			fade.step_size = argv[4].toUint16();
			fade.action = argv[5].toUint16() ?
			              FADE_ACTION_FADE_AND_STOP :
			              FADE_ACTION_FADE_AND_CONT;

			s->_sound.sfx_song_set_fade(handle,  &fade);

			/* FIXME: The next couple of lines actually STOP the handle, rather
			** than fading it! */
			if (argv[5].toUint16()) {
				PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
				PUT_SEL32V(obj, nodePtr, 0);
				PUT_SEL32V(obj, handle, 0);
				s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
			} else {
				// FIXME: Support fade-and-continue. For now, send signal right away.
				PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
			}
		}
		break;
	}
	case _K_SCI1_SOUND_HOLD_HANDLE : {
		int value = argv[2].toSint16();

		s->_sound.sfx_song_set_hold(handle, value);
		break;
	}
	case _K_SCI1_SOUND_UNUSED2 : {
		break;
	}
	case _K_SCI1_SOUND_SET_HANDLE_VOLUME : {
		break;
	}
	case _K_SCI1_SOUND_SET_HANDLE_PRIORITY : {
		int value = argv[2].toSint16();

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
			result = s->_sound.sfx_poll_specific(handle, &cue);

		switch (result) {

		case SI_ABSOLUTE_CUE:
			signal = cue;
			debugC(2, kDebugLevelSound, "[CUE] %04x:%04x Absolute Cue: %d\n",
			        PRINT_REG(obj), signal);

			PUT_SEL32V(obj, signal, signal);
			break;

		case SI_RELATIVE_CUE:
			debugC(2, kDebugLevelSound, "[CUE] %04x:%04x Relative Cue: %d\n",
			        PRINT_REG(obj), cue);

			PUT_SEL32V(obj, dataInc, cue);
			PUT_SEL32V(obj, signal, cue + 127);
			break;

		case SI_FINISHED:
			PUT_SEL32V(obj, signal, SIGNAL_OFFSET);
			break;

		case SI_LOOP:
			break; /* Doesn't happen */
		}
		break;
	}
	case _K_SCI1_SOUND_MIDI_SEND : {
		s->_sound.sfx_send_midi(handle,
		              argv[2].toUint16(), argv[3].toUint16(), argv[4].toUint16(), argv[5].toUint16());
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

/**
 * Used for synthesized music playback
 */
reg_t kDoSound(EngineState *s, int argc, reg_t *argv) {
	switch (s->detectDoSoundType()) {
	case SCI_VERSION_0_EARLY:
		return kDoSoundSci0(s, argc, argv);
	case SCI_VERSION_1_EARLY:
		return kDoSoundSci1Early(s, argc, argv);
	case SCI_VERSION_1_LATE:
		return kDoSoundSci1Late(s, argc, argv);
	default:
		warning("Unknown DoSound type");
		return NULL_REG;
	}
}

/**
 * Used for speech playback and digital soundtracks in CD games
 */
reg_t kDoAudio(EngineState *s, int argc, reg_t *argv) {
	Audio::Mixer *mixer = g_system->getMixer();

	switch (argv[0].toUint16()) {
	case kSciAudioWPlay:
	case kSciAudioPlay: {
		uint16 module;
		uint32 number;

		s->_sound.stopAudio();

		if (argc == 2) {
			module = 65535;
			number = argv[1].toUint16();
		} else if (argc == 6) {
			module = argv[1].toUint16();
			number = ((argv[2].toUint16() & 0xff) << 24) | ((argv[3].toUint16() & 0xff) << 16) |
					 ((argv[4].toUint16() & 0xff) <<  8) | (argv[5].toUint16() & 0xff);
		} else {
			warning("kDoAudio: Play called with an unknown number of parameters (%d)", argc);
			return NULL_REG;
		}

		return make_reg(0, s->_sound.startAudio(module, number)); // return sample length in ticks
	}
	case kSciAudioStop:
		s->_sound.stopAudio();
		break;
	case kSciAudioPause:
		s->_sound.pauseAudio();
		break;
	case kSciAudioResume:
		s->_sound.resumeAudio();
		break;
	case kSciAudioPosition:
		return make_reg(0, s->_sound.getAudioPosition());
	case kSciAudioRate:
		s->_sound.setAudioRate(argv[1].toUint16());
		break;
	case kSciAudioVolume:
		mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, argv[1].toUint16());
		break;
	case kSciAudioLanguage:
		if (argc == 1) {
			// In SCI1.1: tests for digital audio support
			return make_reg(0, 1);
		} else {
			s->resMan->setAudioLanguage(argv[1].toSint16());
		}
		break;
	default:
		warning("kDoAudio: Unhandled case %d", argv[0].toUint16());
	}

	return s->r_acc;
}

reg_t kDoSync(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	switch (argv[0].toUint16()) {
	case kSciAudioSyncStart: {
		ResourceId id;

		if (s->_sound._syncResource) {
			s->resMan->unlockResource(s->_sound._syncResource);
			s->_sound._syncResource = NULL;
		}

		// Load sound sync resource and lock it
		if (argc == 3) {
			id = ResourceId(kResourceTypeSync, argv[2].toUint16());
		} else if (argc == 7) {
			id = ResourceId(kResourceTypeSync36, argv[2].toUint16(), argv[3].toUint16(), argv[4].toUint16(),
							argv[5].toUint16(), argv[6].toUint16());
		} else {
			warning("kDoSync: Start called with an unknown number of parameters (%d)", argc);
			return s->r_acc;
		}

		s->_sound._syncResource = s->resMan->findResource(id, 1);

		if (s->_sound._syncResource) {
			PUT_SEL32V(argv[1], syncCue, 0);
			s->_sound._syncOffset = 0;
		} else {
			warning("DoSync: failed to find resource %s", id.toString().c_str());
			// Notify the scripts to stop sound sync
			PUT_SEL32V(argv[1], syncCue, SIGNAL_OFFSET);
		}
		break;
	}
	case kSciAudioSyncNext: {
		Resource *res = s->_sound._syncResource;
		if (res && (s->_sound._syncOffset < res->size - 1)) {
			int16 syncCue = -1;
			int16 syncTime = (int16)READ_LE_UINT16(res->data + s->_sound._syncOffset);

			s->_sound._syncOffset += 2;

			if ((syncTime != -1) && (s->_sound._syncOffset < res->size - 1)) {
				syncCue = (int16)READ_LE_UINT16(res->data + s->_sound._syncOffset);
				s->_sound._syncOffset += 2;
			}

			PUT_SEL32V(argv[1], syncTime, syncTime);
			PUT_SEL32V(argv[1], syncCue, syncCue);
		}
		break;
	}
	case kSciAudioSyncStop:
		if (s->_sound._syncResource) {
			s->resMan->unlockResource(s->_sound._syncResource);
			s->_sound._syncResource = NULL;
		}
		break;
	default:
		warning("DoSync: Unhandled subfunction %d", argv[0].toUint16());
	}

	return s->r_acc;
}

} // End of namespace Sci
