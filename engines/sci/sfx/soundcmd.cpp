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

#include "sci/sfx/iterator.h"	// for SongIteratorStatus
#include "sci/sfx/music.h"
#include "sci/sfx/soundcmd.h"

namespace Sci {

#define USE_OLD_MUSIC_FUNCTIONS

#define SCI1_SOUND_FLAG_MAY_PAUSE        1 /* Only here for completeness; The interpreter doesn't touch this bit */
#define SCI1_SOUND_FLAG_SCRIPTED_PRI     2 /* but does touch this */

#define FROBNICATE_HANDLE(reg) ((reg).segment << 16 | (reg).offset)
#define DEFROBNICATE_HANDLE(handle) (make_reg((handle >> 16) & 0xffff, handle & 0xffff))

/* Sound status */
enum {
	_K_SOUND_STATUS_STOPPED = 0,
	_K_SOUND_STATUS_INITIALIZED = 1,
	_K_SOUND_STATUS_PAUSED = 2,
	_K_SOUND_STATUS_PLAYING = 3
};

static void script_set_priority(ResourceManager *resMan, SegManager *segMan, SfxState *state, reg_t obj, int priority) {
	int song_nr = GET_SEL32V(segMan, obj, number);
	Resource *song = resMan->findResource(ResourceId(kResourceTypeSound, song_nr), 0);
	int flags = GET_SEL32V(segMan, obj, flags);

	if (priority == -1) {
		if (song->data[0] == 0xf0)
			priority = song->data[1];
		else
			warning("Attempt to unset song priority when there is no built-in value");

		flags &= ~SCI1_SOUND_FLAG_SCRIPTED_PRI;
	} else flags |= SCI1_SOUND_FLAG_SCRIPTED_PRI;

	state->sfx_song_renice(FROBNICATE_HANDLE(obj), priority);
	PUT_SEL32V(segMan, obj, flags, flags);
}

SongIterator *build_iterator(ResourceManager *resMan, int song_nr, SongIteratorType type, songit_id_t id) {
	Resource *song = resMan->findResource(ResourceId(kResourceTypeSound, song_nr), 0);

	if (!song)
		return NULL;

	return songit_new(song->data, song->size, type, id);
}

void process_sound_events(EngineState *s) { /* Get all sound events, apply their changes to the heap */
	int result;
	SongHandle handle;
	int cue;
	SegManager *segMan = s->_segMan;

	if (getSciVersion() > SCI_VERSION_01)
		return;
	// SCI1 and later explicitly poll for everything

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
			/*			PUT_SEL32V(segMan, obj, loops, GET_SEL32V(segMan, obj, loop) - 1);*/
			PUT_SEL32V(segMan, obj, signal, SIGNAL_OFFSET);
			break;

		case SI_RELATIVE_CUE:
			debugC(2, kDebugLevelSound, "[process-sound] Song %04x:%04x received relative cue %d\n",
			          PRINT_REG(obj), cue);
			PUT_SEL32V(segMan, obj, signal, cue + 0x7f);
			break;

		case SI_ABSOLUTE_CUE:
			debugC(2, kDebugLevelSound, "[process-sound] Song %04x:%04x received absolute cue %d\n",
			          PRINT_REG(obj), cue);
			PUT_SEL32V(segMan, obj, signal, cue);
			break;

		case SI_FINISHED:
			debugC(2, kDebugLevelSound, "[process-sound] Song %04x:%04x finished\n",
			          PRINT_REG(obj));
			PUT_SEL32V(segMan, obj, signal, SIGNAL_OFFSET);
			PUT_SEL32V(segMan, obj, state, _K_SOUND_STATUS_STOPPED);
			break;

		default:
			warning("Unexpected result from sfx_poll: %d", result);
			break;
		}
	}
}

#define SOUNDCOMMAND(x) _soundCommands.push_back(new SciSoundCommand(#x, &SoundCommandParser::x))

SoundCommandParser::SoundCommandParser(ResourceManager *resMan, SegManager *segMan, SfxState *state, AudioPlayer *audio, SciVersion doSoundVersion) : 
	_resMan(resMan), _segMan(segMan), _state(state), _audio(audio), _doSoundVersion(doSoundVersion) {

	_hasNodePtr = (((SciEngine*)g_engine)->getKernel()->_selectorCache.nodePtr != -1);

	_music = new SciMusic();
	_music->init();

	switch (doSoundVersion) {
	case SCI_VERSION_0_EARLY:
		SOUNDCOMMAND(cmdInitHandle);
		SOUNDCOMMAND(cmdPlayHandle);
		SOUNDCOMMAND(cmdDummy);
		SOUNDCOMMAND(cmdDisposeHandle);
		SOUNDCOMMAND(cmdMuteSound);
		SOUNDCOMMAND(cmdStopHandle);
		SOUNDCOMMAND(cmdSuspendHandle);
		SOUNDCOMMAND(cmdResumeHandle);
		SOUNDCOMMAND(cmdVolume);
		SOUNDCOMMAND(cmdUpdateVolumePriority);
		SOUNDCOMMAND(cmdFadeHandle);
		SOUNDCOMMAND(cmdGetPolyphony);
		SOUNDCOMMAND(cmdGetPlayNext);
		break;
	case SCI_VERSION_1_EARLY:
		SOUNDCOMMAND(cmdVolume);
		SOUNDCOMMAND(cmdMuteSound);
		SOUNDCOMMAND(cmdDummy);
		SOUNDCOMMAND(cmdGetPolyphony);
		SOUNDCOMMAND(cmdUpdateHandle);
		SOUNDCOMMAND(cmdInitHandle);
		SOUNDCOMMAND(cmdDisposeHandle);
		SOUNDCOMMAND(cmdPlayHandle);
		SOUNDCOMMAND(cmdStopHandle);
		SOUNDCOMMAND(cmdSuspendHandle);
		SOUNDCOMMAND(cmdFadeHandle);
		SOUNDCOMMAND(cmdUpdateCues);
		SOUNDCOMMAND(cmdSendMidi);
		SOUNDCOMMAND(cmdReverb);
		SOUNDCOMMAND(cmdHoldHandle);
		break;
	case SCI_VERSION_1_LATE:
		SOUNDCOMMAND(cmdVolume);
		SOUNDCOMMAND(cmdMuteSound);
		SOUNDCOMMAND(cmdDummy);
		SOUNDCOMMAND(cmdGetPolyphony);
		SOUNDCOMMAND(cmdGetAudioCapability);
		SOUNDCOMMAND(cmdSuspendSound);
		SOUNDCOMMAND(cmdInitHandle);
		SOUNDCOMMAND(cmdDisposeHandle);
		SOUNDCOMMAND(cmdPlayHandle);
		SOUNDCOMMAND(cmdStopHandle);
		SOUNDCOMMAND(cmdSuspendHandle);
		SOUNDCOMMAND(cmdFadeHandle);
		SOUNDCOMMAND(cmdHoldHandle);
		SOUNDCOMMAND(cmdDummy);
		SOUNDCOMMAND(cmdSetHandleVolume);
		SOUNDCOMMAND(cmdSetHandlePriority);
		SOUNDCOMMAND(cmdSetHandleLoop);
		SOUNDCOMMAND(cmdUpdateCues);
		SOUNDCOMMAND(cmdSendMidi);
		SOUNDCOMMAND(cmdReverb);
		SOUNDCOMMAND(cmdUpdateVolumePriority);
		break;
	default:
		warning("Sound command parser: unknown DoSound type %d", doSoundVersion);
		break;
	}
}

SoundCommandParser::~SoundCommandParser() {
}

reg_t SoundCommandParser::parseCommand(int argc, reg_t *argv, reg_t acc) {
	uint16 command = argv[0].toUint16();
	reg_t obj = (argc > 1) ? argv[1] : NULL_REG;
	SongHandle handle = FROBNICATE_HANDLE(obj);
	int value = (argc > 2) ? argv[2].toSint16() : 0;
	_acc = acc;
	_argc = argc;
	_argv = argv;

	if (argc > 5) {	// for cmdSendMidi
		_midiCmd = argv[3].toUint16() == 0xff ?
					  0xe0 : /* Pitch wheel */
					  0xb0; /* argv[3].toUint16() is actually a controller number */
		_controller = argv[3].toUint16();
		_param = argv[4].toUint16();
	}

	if (command < _soundCommands.size()) {
		// printf("%s\n", _soundCommands[command]->desc);	// debug
		debugC(2, kDebugLevelSound, "%s", _soundCommands[command]->desc);
		(this->*(_soundCommands[command]->sndCmd))(obj, handle, value);
	} else {
		warning("Invalid sound command requested (%d), valid range is 0-%d", command, _soundCommands.size() - 1);
	}

	return _acc;
}

void SoundCommandParser::cmdInitHandle(reg_t obj, SongHandle handle, int value) {
#ifdef USE_OLD_MUSIC_FUNCTIONS

	if (_doSoundVersion != SCI_VERSION_1_LATE) {
		if (!obj.segment)
			return;
	}

	SongIteratorType type = !_hasNodePtr ? SCI_SONG_ITERATOR_TYPE_SCI0 : SCI_SONG_ITERATOR_TYPE_SCI1;
	int number = obj.segment ? GET_SEL32V(_segMan, obj, number) : -1;

	if (_hasNodePtr) {
		if (GET_SEL32V(_segMan, obj, nodePtr)) {
			_state->sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
			_state->sfx_remove_song(handle);
		}
	}

	if (!obj.segment || !_resMan->testResource(ResourceId(kResourceTypeSound, number)))
		return;

	_state->sfx_add_song(build_iterator(_resMan, number, type, handle), 0, handle, number);

	if (!_hasNodePtr)
		PUT_SEL32V(_segMan, obj, state, _K_SOUND_STATUS_INITIALIZED);
	else
		PUT_SEL32(_segMan, obj, nodePtr, obj);

	PUT_SEL32(_segMan, obj, handle, obj);
#else

	uint16 resnum = GET_SEL32V(_segMan, obj, number);
	Resource *res = resnum ? _resMan->findResource(ResourceId(kResourceTypeSound, resnum), true) : NULL;

	if (!GET_SEL32V(_segMan, obj, nodePtr)) {
		PUT_SEL32(_segMan, obj, nodePtr, obj);
		_soundList.push_back(obj);
	}

	sciSound *pSnd = new sciSound();
	pSnd->resnum = resnum;
	pSnd->loop = (GET_SEL32V(_segMan, obj, loop) == 0xFFFF ? 1 : 0);
	pSnd->prio = GET_SEL32V(_segMan, obj, pri) & 0xFF; // priority
	pSnd->volume = GET_SEL32V(_segMan, obj, vol) & 0xFF; // volume
	pSnd->signal = pSnd->dataInc = 0;
	pSnd->pMidiParser = 0;
	pSnd->pStreamAud = 0;

	_music->soundKill(pSnd);
	if (res) {
		SoundRes *sndRes = (SoundRes *)res;
		_music->soundInitSnd(sndRes, pSnd);
	}
#endif
}

void SoundCommandParser::cmdPlayHandle(reg_t obj, SongHandle handle, int value) {
	if (!obj.segment)
		return;

	if (!_hasNodePtr) {
		_state->sfx_song_set_status(handle, SOUND_STATUS_PLAYING);
		_state->sfx_song_set_loops(handle, GET_SEL32V(_segMan, obj, loop));
		PUT_SEL32V(_segMan, obj, state, _K_SOUND_STATUS_PLAYING);
	} else if (_doSoundVersion == SCI_VERSION_1_EARLY) {
		_state->sfx_song_set_status(handle, SOUND_STATUS_PLAYING);
		_state->sfx_song_set_loops(handle, GET_SEL32V(_segMan, obj, loop));
		_state->sfx_song_renice(handle, GET_SEL32V(_segMan, obj, pri));
		RESTORE_BEHAVIOR rb = (RESTORE_BEHAVIOR) value;		/* Too lazy to look up a default value for this */
		_state->_songlib.setSongRestoreBehavior(handle, rb);
		PUT_SEL32V(_segMan, obj, signal, 0);
	} else if (_doSoundVersion == SCI_VERSION_1_LATE) {
		int looping = GET_SEL32V(_segMan, obj, loop);
		//int vol = GET_SEL32V(_segMan, obj, vol);
		int pri = GET_SEL32V(_segMan, obj, pri);
		int sampleLen = 0;
		Song *song = _state->_songlib.findSong(handle);
		int songNumber = GET_SEL32V(_segMan, obj, number);

		if (GET_SEL32V(_segMan, obj, nodePtr) && (song && songNumber != song->_resourceNum)) {
			_state->sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
			_state->sfx_remove_song(handle);
			PUT_SEL32(_segMan, obj, nodePtr, NULL_REG);
		}

		if (!GET_SEL32V(_segMan, obj, nodePtr) && obj.segment) {
			// In SCI1.1 games, sound effects are started from here. If we can find
			// a relevant audio resource, play it, otherwise switch to synthesized
			// effects. If the resource exists, play it using map 65535 (sound
			// effects map)
			if (_resMan->testResource(ResourceId(kResourceTypeAudio, songNumber)) &&
				getSciVersion() >= SCI_VERSION_1_1) {
				// Found a relevant audio resource, play it
				_audio->stopAudio();
				warning("Initializing audio resource instead of requested sound resource %d", songNumber);
				sampleLen = _audio->startAudio(65535, songNumber);
				// Also create iterator, that will fire SI_FINISHED event, when the sound is done playing
				_state->sfx_add_song(new_timer_iterator(sampleLen), 0, handle, songNumber);
			} else {
				if (!_resMan->testResource(ResourceId(kResourceTypeSound, songNumber))) {
					warning("Could not open song number %d", songNumber);
					// Send a "stop handle" event so that the engine won't wait forever here
					_state->sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
					PUT_SEL32V(_segMan, obj, signal, SIGNAL_OFFSET);
					return;
				}
				debugC(2, kDebugLevelSound, "Initializing song number %d\n", songNumber);
				_state->sfx_add_song(build_iterator(_resMan, songNumber, SCI_SONG_ITERATOR_TYPE_SCI1,
				                          handle), 0, handle, songNumber);
			}

			PUT_SEL32(_segMan, obj, nodePtr, obj);
			PUT_SEL32(_segMan, obj, handle, obj);
		}

		if (obj.segment) {
			_state->sfx_song_set_status(handle, SOUND_STATUS_PLAYING);
			_state->sfx_song_set_loops(handle, looping);
			_state->sfx_song_renice(handle, pri);
			PUT_SEL32V(_segMan, obj, signal, 0);
		}
	}

#if 0
	if (hobj == 0)
		return;
	Object obj(hobj);
	HEAPHANDLE hptr = obj.getProperty(44);
	if (hptr) {
		sciSound *pSnd = (sciSound *)heap2Ptr(hptr);
		if (pSnd->resnum != obj.getProperty(43)) { // another sound loaded into struct
			KillSnd(hobj);
			InitSnd(hobj);
		}
		ResMgr.ResLock(SCI_RES_SOUND, pSnd->resnum);
		obj.setProperty(93, 0x1234/*res->getHandle()*/); // handle
		obj.setProperty(17, 0);	// signal
		obj.setProperty(94, 0);	// min
		obj.setProperty(95, 0);	// sec
		obj.setProperty(96, 0);	// frame
		pSnd->loop = (obj.getProperty(6) == 0xFFFF ? 1 : 0);	// loop
		pSnd->prio = obj.getProperty(63);	// priority
		pSnd->volume = obj.getProperty(97);	// vol
		//ChangeSndState(hobj);
		_audio->soundPlay(pSnd);
	}
#endif
}

void SoundCommandParser::cmdDummy(reg_t obj, SongHandle handle, int value) {
	warning("cmdDummy invoked");	// not supposed to occur
}

void SoundCommandParser::changeHandleStatus(reg_t obj, SongHandle handle, int newStatus) {
	if (obj.segment) {
		_state->sfx_song_set_status(handle, newStatus);
		if (!_hasNodePtr)
			PUT_SEL32V(_segMan, obj, state, newStatus);
	}
}

void SoundCommandParser::cmdDisposeHandle(reg_t obj, SongHandle handle, int value) {
	changeHandleStatus(obj, handle, SOUND_STATUS_STOPPED);

	if (obj.segment) {
		_state->sfx_remove_song(handle);

		if (!_hasNodePtr)
			PUT_SEL32V(_segMan, obj, handle, 0x0000);
	}

#if 0
	if (hobj == 0)
		return;
	Object obj(hobj);
	StopSnd(hobj);
	HEAPHANDLE hptr = obj.getProperty(44);	// nodePtr
	if (hptr) {
		sciSound *pSnd = (sciSound *)heap2Ptr(hptr);
		_audio->soundKill(pSnd);
		ResMgr.ResUnload(SCI_RES_SOUND, pSnd->resnum);
		_soundList->DeleteNode(hptr);
		heapDisposePtr(hptr);
	}
	obj.setProperty(44, 0);	// nodePtr
#endif
}

void SoundCommandParser::cmdStopHandle(reg_t obj, SongHandle handle, int value) {
	changeHandleStatus(obj, handle, SOUND_STATUS_STOPPED);

	if (_hasNodePtr)
		PUT_SEL32V(_segMan, obj, signal, SIGNAL_OFFSET);

#if 0
	if (hobj == 0)
		return;
	Object obj(hobj);
	obj.setProperty(93, 0);	// handle
	obj.setProperty(17, 0xFFFF);	// signal
	sciSound *pSnd = (sciSound *)heap2Ptr(obj.getProperty(44));
	if (pSnd) {
		pSnd->dataInc = 0;
		pSnd->signal = 0xFFFF;
		_audio->soundStop(pSnd);
	}
#endif
}

void SoundCommandParser::cmdSuspendHandle(reg_t obj, SongHandle handle, int value) {
	if (!_hasNodePtr)
		changeHandleStatus(obj, handle, SOUND_STATUS_SUSPENDED);
	else
		changeHandleStatus(obj, handle, value ? SOUND_STATUS_SUSPENDED : SOUND_STATUS_PLAYING);

#if 0
	// PauseSnd(argv[2], argv[3]); - PauseSnd(HEAPHANDLE hobj, uint16 w)
	if (hobj == 0)
		return;
	Object obj(hobj);
	HEAPHANDLE hnode = obj.getProperty(44);	// nodePtr
	if (!hnode)
		return;
	sciSound *pSnd = (sciSound *)heap2Ptr(hnode);
	if (w)
		_audio->soundPause(pSnd);
	else
		_audio->soundPlay(pSnd);
#endif
}

void SoundCommandParser::cmdResumeHandle(reg_t obj, SongHandle handle, int value) {
	changeHandleStatus(obj, handle, SOUND_STATUS_PLAYING);
}

void SoundCommandParser::cmdMuteSound(reg_t obj, SongHandle handle, int value) {
	//_acc = _music->SoundOn(argc > 1 ? argv[2] : 0xFF);

	// TODO

	/* if there's a parameter, we're setting it.  Otherwise, we're querying it. */
	/*int param = UPARAM_OR_ALT(1,-1);

	if (param != -1)
	s->acc = s->sound_server->command(s, SOUND_COMMAND_SET_MUTE, 0, param);
	else
	s->acc = s->sound_server->command(s, SOUND_COMMAND_GET_MUTE, 0, 0);*/
}

void SoundCommandParser::cmdVolume(reg_t obj, SongHandle handle, int value) {
	if (_argc > 1)
		_music->soundSetMasterVolume(obj.toSint16());
	_acc = make_reg(0, _music->soundGetMasterVolume());
}

void SoundCommandParser::cmdFadeHandle(reg_t obj, SongHandle handle, int value) {
	if (!obj.segment)
		return;

	if (_doSoundVersion != SCI_VERSION_1_LATE) {
		/*s->sound_server->command(s, SOUND_COMMAND_FADE_HANDLE, obj, 120);*/ /* Fade out in 2 secs */
		/* FIXME: The next couple of lines actually STOP the handle, rather
		** than fading it! */
		_state->sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
		if (!_hasNodePtr)
			PUT_SEL32V(_segMan, obj, state, SOUND_STATUS_STOPPED);
		PUT_SEL32V(_segMan, obj, signal, SIGNAL_OFFSET);
	} else {
		fade_params_t fade;
		fade.final_volume = _argv[2].toUint16();
		fade.ticks_per_step = _argv[3].toUint16();
		fade.step_size = _argv[4].toUint16();
		fade.action = _argv[5].toUint16() ?
		              FADE_ACTION_FADE_AND_STOP :
		              FADE_ACTION_FADE_AND_CONT;

		_state->sfx_song_set_fade(handle,  &fade);

		/* FIXME: The next couple of lines actually STOP the handle, rather
		** than fading it! */
		if (_argv[5].toUint16()) {
			PUT_SEL32V(_segMan, obj, signal, SIGNAL_OFFSET);
			_state->sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
		} else {
			// FIXME: Support fade-and-continue. For now, send signal right away.
			PUT_SEL32V(_segMan, obj, signal, SIGNAL_OFFSET);
		}
	}

#if 0
	// FadeSnd(argv[2], argv[3], argv[4], argv[5]);

//void SciEngine::FadeSnd(HEAPHANDLE hobj, uint16 tVolume, uint16 tickerstep, int16 fadestep) {
	if (hobj == 0)
		return;
	Object obj(hobj);
	HEAPHANDLE hnode = obj.getProperty(44);	// nodePtr
	if (!hnode)
		return;
	sciSound *pSnd = (sciSound *)heap2Ptr(hnode);
	pSnd->FadeTo = tVolume;
	pSnd->FadeStep = pSnd->volume > tVolume ? -fadestep : fadestep;
	pSnd->FadeTickerStep = tickerstep * 16667 / _audio->soundGetTempo();
	pSnd->FadeTicker = 0;
//}
#endif
}

void SoundCommandParser::cmdGetPolyphony(reg_t obj, SongHandle handle, int value) {
	_acc = make_reg(0, _state->sfx_get_player_polyphony());
	// TODO: Amiga music
	//_acc = make_reg(0, _music->soundGetVoices());
}

void SoundCommandParser::cmdUpdateHandle(reg_t obj, SongHandle handle, int value) {
	if (!GET_SEL32(_segMan, obj, nodePtr).isNull()) {
		/*int16 loop = GET_SEL32V(_segMan, obj, loop);
		int16 vol = GET_SEL32V(_segMan, obj, vol);
		int16 priority = GET_SEL32V(_segMan, obj, priority);*/

		// TODO: pSnd
		//pSnd->loop = (loop == 0xFFFF ? 1 : 0);
		//_music->soundSetVolume(pSnd, vol);
		//_music->soundSetPriority(pSnd, prio);
	}
}

void SoundCommandParser::cmdUpdateCues(reg_t obj, SongHandle handle, int value) {
	int signal = 0;
	int min = 0;
	int sec = 0;
	int frame = 0;
	int result = SI_LOOP; // small hack

	while (result == SI_LOOP)
		result = _state->sfx_poll_specific(handle, &signal);

	switch (result) {
	case SI_ABSOLUTE_CUE:
		debugC(2, kDebugLevelSound, "---    [CUE] %04x:%04x Absolute Cue: %d\n",
		          PRINT_REG(obj), signal);

		PUT_SEL32V(_segMan, obj, signal, signal);
		break;

	case SI_RELATIVE_CUE:
		debugC(2, kDebugLevelSound, "---    [CUE] %04x:%04x Relative Cue: %d\n",
		          PRINT_REG(obj), signal);

		/* FIXME to match commented-out semantics
		 * below, with proper storage of dataInc and
		 * signal in the iterator code. */
		PUT_SEL32V(_segMan, obj, dataInc, signal);
		if (_doSoundVersion == SCI_VERSION_1_EARLY)
			PUT_SEL32V(_segMan, obj, signal, signal);
		else
			PUT_SEL32V(_segMan, obj, signal, signal + 127);
		break;

	case SI_FINISHED:
		debugC(2, kDebugLevelSound, "---    [FINISHED] %04x:%04x\n", PRINT_REG(obj));
		PUT_SEL32V(_segMan, obj, signal, SIGNAL_OFFSET);
		break;

	case SI_LOOP:
		break; // Doesn't happen
	}

	//switch (signal) {
	//case 0x00:
	//	if (dataInc!=GET_SEL32V(segMan, obj, dataInc)) {
	//		PUT_SEL32V(segMan, obj, dataInc, dataInc);
	//		PUT_SEL32V(segMan, obj, signal, dataInc+0x7f);
	//	} else {
	//		PUT_SEL32V(segMan, obj, signal, signal);
	//	}
	//	break;
	//case 0xFF: // May be unnecessary
	//	s->_sound.sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
	//	break;
	//default :
	//	if (dataInc!=GET_SEL32V(segMan, obj, dataInc)) {
	//		PUT_SEL32V(segMan, obj, dataInc, dataInc);
	//		PUT_SEL32V(segMan, obj, signal, dataInc + 0x7f);
	//	} else {
	//		PUT_SEL32V(segMan, obj, signal, signal);
	//	}
	//	break;
	//}

	if (_doSoundVersion == SCI_VERSION_1_EARLY) {
		PUT_SEL32V(_segMan, obj, min, min);
		PUT_SEL32V(_segMan, obj, sec, sec);
		PUT_SEL32V(_segMan, obj, frame, frame);
	}

#if 0
	if (hobj == 0)
		return;
	Object obj(hobj);
	HEAPHANDLE hnode = obj.getProperty(44);	// nodePtr
	if (hnode) {
		sciSound *pSnd = (sciSound *)heap2Ptr(hnode);
		switch (pSnd->signal) {
		case 0:
			if (pSnd->dataInc != obj.getProperty(92)) { // dataInc
				obj.setProperty(92, pSnd->dataInc);	// dataInc
				obj.setProperty(17, pSnd->dataInc + 127); // signal
			}
			break;
		case 0xFFFF:
			StopSnd(hobj);
			break;
		default:
			obj.setProperty(17, pSnd->signal);	// signal
		}
		//D13E
		pSnd->signal = 0;
		obj.setProperty(94, pSnd->ticker / 3600); // .min
		obj.setProperty(95, pSnd->ticker % 3600 / 60); // .sec
		obj.setProperty(96, pSnd->ticker); // .frame
		obj.setProperty(97, pSnd->volume); // volume
	}
#endif
}

void SoundCommandParser::cmdSendMidi(reg_t obj, SongHandle handle, int value) {
	int channel = value;

	_state->sfx_send_midi(handle, channel, _midiCmd, _controller, _param);
}

void SoundCommandParser::cmdReverb(reg_t obj, SongHandle handle, int value) {
	// TODO
}
void SoundCommandParser::cmdHoldHandle(reg_t obj, SongHandle handle, int value) {
	_state->sfx_song_set_hold(handle, value);
	// TODO
}

void SoundCommandParser::cmdGetAudioCapability(reg_t obj, SongHandle handle, int value) {
	// Tests for digital audio support
	_acc = make_reg(0, 1);
}

void SoundCommandParser::cmdGetPlayNext(reg_t obj, SongHandle handle, int value) {
}

void SoundCommandParser::cmdSetHandleVolume(reg_t obj, SongHandle handle, int value) {
	// TODO
#if 0
	if (hobj == 0)
		return;
	Object obj(hobj);
	sciSound *pSnd;
	HEAPHANDLE hnode = obj.getProperty(44);	// nodePtr
	if (hnode && (pSnd = (sciSound *)heap2Ptr(hnode))) {
		if (pSnd->volume != w) {
			pSnd->volume = w;
			_audio->soundSetVolume(pSnd, w);
			obj.setProperty(97, w);	// volume
		}
	}
#endif
}

void SoundCommandParser::cmdSetHandlePriority(reg_t obj, SongHandle handle, int value) {
	script_set_priority(_resMan, _segMan, _state, obj, value);

#if 0
	if (hobj == 0)
		return;
	Object obj(hobj);
	sciSound *pSnd;
	HEAPHANDLE hnode = obj.getProperty(44);	// nodePtr
	if (hnode && (pSnd = (sciSound *)heap2Ptr(hnode))) {
		if (w == 0xFFFF) {
			//pSnd->prio=0;field_15B=0
			obj.setProperty(102, obj.getProperty(102) & 0xFD);//flags
		} else {
			//pSnd->field_15B=1;
			obj.setProperty(102, obj.getProperty(102) | 2);//flags
			//DoSOund(0xF,hobj,w)
		}
	}
#endif
}

void SoundCommandParser::cmdSetHandleLoop(reg_t obj, SongHandle handle, int value) {
	if (!GET_SEL32(_segMan, obj, nodePtr).isNull()) {
		uint16 looping = (value == -1) ? 1 : 0xFFFF;
		_state->sfx_song_set_loops(handle, looping);
		PUT_SEL32V(_segMan, obj, loop, (value == -1) ? 0xFFFF : 1);
	}
}

void SoundCommandParser::cmdSuspendSound(reg_t obj, SongHandle handle, int value) {
	// TODO
}

void SoundCommandParser::cmdUpdateVolumePriority(reg_t obj, SongHandle handle, int value) {
	if (!_hasNodePtr && obj.segment) {
 		_state->sfx_song_set_loops(handle, GET_SEL32V(_segMan, obj, loop));
 		script_set_priority(_resMan, _segMan, _state, obj, GET_SEL32V(_segMan, obj, pri));
 	}

#if 0
	if (!GET_SEL32(_segMan, obj, nodePtr).isNull()) {
		int16 loop = GET_SEL32V(_segMan, obj, loop);
		int16 vol = GET_SEL32V(_segMan, obj, vol);
		int16 priority = GET_SEL32V(_segMan, obj, priority);

		// TODO: pSnd
		//pSnd->loop = (loop == 0xFFFF ? 1 : 0);
		//_music->soundSetVolume(pSnd, vol);
		//_music->soundSetPriority(pSnd, prio);
	}
#endif
}

} // End of namespace Sci
