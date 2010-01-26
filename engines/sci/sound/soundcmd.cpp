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

#include "sci/sci.h"	// for USE_OLD_MUSIC_FUNCTIONS

#ifdef USE_OLD_MUSIC_FUNCTIONS
#include "sci/sound/iterator/iterator.h"	// for SongIteratorStatus
#endif

#include "common/config-manager.h"
#include "sci/sound/music.h"
#include "sci/sound/soundcmd.h"

namespace Sci {

#define SCI1_SOUND_FLAG_MAY_PAUSE        1 /* Only here for completeness; The interpreter doesn't touch this bit */
#define SCI1_SOUND_FLAG_SCRIPTED_PRI     2 /* but does touch this */

#ifdef USE_OLD_MUSIC_FUNCTIONS
#define FROBNICATE_HANDLE(reg) ((reg).segment << 16 | (reg).offset)
#define DEFROBNICATE_HANDLE(handle) (make_reg((handle >> 16) & 0xffff, handle & 0xffff))
#endif

#define SOUNDCOMMAND(x) _soundCommands.push_back(new MusicEntryCommand(#x, &SoundCommandParser::x))

#ifdef USE_OLD_MUSIC_FUNCTIONS
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
			PUT_SEL32V(segMan, obj, state, kSoundStopped);
			break;

		default:
			warning("Unexpected result from sfx_poll: %d", result);
			break;
		}
	}
}

#endif
SoundCommandParser::SoundCommandParser(ResourceManager *resMan, SegManager *segMan, Kernel *kernel, AudioPlayer *audio, SciVersion soundVersion) :
	_resMan(resMan), _segMan(segMan), _kernel(kernel), _audio(audio), _soundVersion(soundVersion) {

#ifdef USE_OLD_MUSIC_FUNCTIONS
	// The following hack is needed to ease the change from old to new sound code (because the new sound code does not use SfxState)
	_state = &((SciEngine *)g_engine)->getEngineState()->_sound;	// HACK
#endif

	#ifndef USE_OLD_MUSIC_FUNCTIONS
		_music = new SciMusic(_soundVersion);
		_music->init();
	#endif

	switch (_soundVersion) {
	case SCI_VERSION_0_EARLY:
	case SCI_VERSION_0_LATE:
		SOUNDCOMMAND(cmdInitSound);
		SOUNDCOMMAND(cmdPlaySound);
		SOUNDCOMMAND(cmdDummy);
		SOUNDCOMMAND(cmdDisposeSound);
		SOUNDCOMMAND(cmdMuteSound);
		SOUNDCOMMAND(cmdStopSound);
		SOUNDCOMMAND(cmdPauseSound);
		SOUNDCOMMAND(cmdResumeSound);
		SOUNDCOMMAND(cmdMasterVolume);
		SOUNDCOMMAND(cmdUpdateSound);
		SOUNDCOMMAND(cmdFadeSound);
		SOUNDCOMMAND(cmdGetPolyphony);
		SOUNDCOMMAND(cmdStopAllSounds);
		_cmdUpdateCuesIndex = -1;
		break;
	case SCI_VERSION_1_EARLY:
		SOUNDCOMMAND(cmdMasterVolume);
		SOUNDCOMMAND(cmdMuteSound);
		SOUNDCOMMAND(cmdDummy);
		SOUNDCOMMAND(cmdGetPolyphony);
		SOUNDCOMMAND(cmdUpdateSound);
		SOUNDCOMMAND(cmdInitSound);
		SOUNDCOMMAND(cmdDisposeSound);
		SOUNDCOMMAND(cmdPlaySound);
		SOUNDCOMMAND(cmdStopSound);
		SOUNDCOMMAND(cmdPauseSound);
		SOUNDCOMMAND(cmdFadeSound);
		SOUNDCOMMAND(cmdUpdateCues);
		SOUNDCOMMAND(cmdSendMidi);
		SOUNDCOMMAND(cmdReverb);
		SOUNDCOMMAND(cmdSetSoundHold);
		_cmdUpdateCuesIndex = 11;
		break;
	case SCI_VERSION_1_LATE:
		SOUNDCOMMAND(cmdMasterVolume);
		SOUNDCOMMAND(cmdMuteSound);
		SOUNDCOMMAND(cmdDummy);
		SOUNDCOMMAND(cmdGetPolyphony);
		SOUNDCOMMAND(cmdGetAudioCapability);
		SOUNDCOMMAND(cmdSuspendSound);
		SOUNDCOMMAND(cmdInitSound);
		SOUNDCOMMAND(cmdDisposeSound);
		SOUNDCOMMAND(cmdPlaySound);
		SOUNDCOMMAND(cmdStopSound);
		SOUNDCOMMAND(cmdPauseSound);
		SOUNDCOMMAND(cmdFadeSound);
		SOUNDCOMMAND(cmdSetSoundHold);
		SOUNDCOMMAND(cmdDummy);
		SOUNDCOMMAND(cmdSetSoundVolume);
		SOUNDCOMMAND(cmdSetSoundPriority);
		SOUNDCOMMAND(cmdSetSoundLoop);
		SOUNDCOMMAND(cmdUpdateCues);
		SOUNDCOMMAND(cmdSendMidi);
		SOUNDCOMMAND(cmdReverb);
		SOUNDCOMMAND(cmdUpdateSound);
		_cmdUpdateCuesIndex = 17;
		break;
	default:
		warning("Sound command parser: unknown sound version %d", _soundVersion);
		break;
	}
}

SoundCommandParser::~SoundCommandParser() {
	for (SoundCommandContainer::iterator i = _soundCommands.begin(); i != _soundCommands.end(); ++i)
		delete *i;

#ifndef USE_OLD_MUSIC_FUNCTIONS
	delete _music;
#endif
}

reg_t SoundCommandParser::parseCommand(int argc, reg_t *argv, reg_t acc) {
	uint16 command = argv[0].toUint16();
	reg_t obj = (argc > 1) ? argv[1] : NULL_REG;
	int16 value = (argc > 2) ? argv[2].toSint16() : 0;
	_acc = acc;
	_argc = argc;
	_argv = argv;

	if (argc == 6) {	// cmdSendMidi
		byte channel = argv[2].toUint16() & 0xf;
		byte midiCmd = argv[3].toUint16() & 0xff;

		uint16 controller = argv[4].toUint16();
		uint16 param = argv[5].toUint16();

		_midiCommand = (channel | midiCmd) | ((uint32)controller << 8) | ((uint32)param << 16);
	}

	if (command < _soundCommands.size()) {
		if (command != _cmdUpdateCuesIndex) {
			//printf("%s, object %04x:%04x\n", _soundCommands[command]->desc, PRINT_REG(obj));	// debug
			debugC(2, kDebugLevelSound, "%s, object %04x:%04x", _soundCommands[command]->desc, PRINT_REG(obj));
		}

		(this->*(_soundCommands[command]->sndCmd))(obj, value);
	} else {
		warning("Invalid sound command requested (%d), valid range is 0-%d", command, _soundCommands.size() - 1);
	}

	return _acc;
}

void SoundCommandParser::cmdInitSound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

	int number = GET_SEL32V(_segMan, obj, number);

#ifdef USE_OLD_MUSIC_FUNCTIONS

	SongHandle handle = FROBNICATE_HANDLE(obj);

	if (_soundVersion != SCI_VERSION_1_LATE) {
		if (!obj.segment)
			return;
	}

	SongIteratorType type = (_soundVersion <= SCI_VERSION_0_LATE) ? SCI_SONG_ITERATOR_TYPE_SCI0 : SCI_SONG_ITERATOR_TYPE_SCI1;

	if (_soundVersion <= SCI_VERSION_0_LATE) {
		if (GET_SEL32V(_segMan, obj, nodePtr)) {
			_state->sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
			_state->sfx_remove_song(handle);
		}
	}

	if (!obj.segment || !_resMan->testResource(ResourceId(kResourceTypeSound, number)))
		return;

	_state->sfx_add_song(build_iterator(_resMan, number, type, handle), 0, handle, number);


	// Notify the engine
	if (_soundVersion <= SCI_VERSION_0_LATE)
		PUT_SEL32V(_segMan, obj, state, kSoundInitialized);
	else
		PUT_SEL32(_segMan, obj, nodePtr, obj);

	PUT_SEL32(_segMan, obj, handle, obj);

#else

	MusicEntry *newSound = new MusicEntry();
	newSound->resnum = number;
	if (number && _resMan->testResource(ResourceId(kResourceTypeSound, number)))
		newSound->soundRes = new SoundResource(number, _resMan, _soundVersion);
	else
		newSound->soundRes = 0;

	newSound->soundObj = obj;
	newSound->loop = GET_SEL32V(_segMan, obj, loop);
	newSound->prio = GET_SEL32V(_segMan, obj, pri) & 0xFF;
	if (_soundVersion >= SCI_VERSION_1_LATE)
		newSound->volume = CLIP<int>(GET_SEL32V(_segMan, obj, vol), 0, MUSIC_VOLUME_MAX);

	// Check if a track with the same sound object is already playing
	MusicEntry *oldSound = _music->getSlot(obj);
	if (oldSound)
		cmdDisposeSound(obj, value);

	// In SCI1.1 games, sound effects are started from here. If we can find
	// a relevant audio resource, play it, otherwise switch to synthesized
	// effects. If the resource exists, play it using map 65535 (sound
	// effects map)

	if (getSciVersion() >= SCI_VERSION_1_1 && _resMan->testResource(ResourceId(kResourceTypeAudio, number))) {
		// Found a relevant audio resource, play it
		int sampleLen;
		newSound->pStreamAud = _audio->getAudioStream(number, 65535, &sampleLen);
		newSound->soundType = Audio::Mixer::kSpeechSoundType;
	} else {
		if (newSound->soundRes)
			_music->soundInitSnd(newSound);
	}

	_music->pushBackSlot(newSound);

	if (newSound->soundRes || newSound->pStreamAud) {
		// Notify the engine
		if (_soundVersion <= SCI_VERSION_0_LATE)
			PUT_SEL32V(_segMan, obj, state, kSoundInitialized);
		else
			PUT_SEL32(_segMan, obj, nodePtr, obj);

		PUT_SEL32(_segMan, obj, handle, obj);
	}
#endif

}

void SoundCommandParser::cmdPlaySound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	SongHandle handle = FROBNICATE_HANDLE(obj);

	if (_soundVersion <= SCI_VERSION_0_LATE) {
		_state->sfx_song_set_status(handle, SOUND_STATUS_PLAYING);
		_state->sfx_song_set_loops(handle, GET_SEL32V(_segMan, obj, loop));
		PUT_SEL32V(_segMan, obj, state, kSoundPlaying);
	} else if (_soundVersion == SCI_VERSION_1_EARLY) {
		_state->sfx_song_set_status(handle, SOUND_STATUS_PLAYING);
		_state->sfx_song_set_loops(handle, GET_SEL32V(_segMan, obj, loop));
		_state->sfx_song_renice(handle, GET_SEL32V(_segMan, obj, pri));
		RESTORE_BEHAVIOR rb = (RESTORE_BEHAVIOR) value;		/* Too lazy to look up a default value for this */
		_state->_songlib.setSongRestoreBehavior(handle, rb);
		PUT_SEL32V(_segMan, obj, signal, 0);
	} else if (_soundVersion == SCI_VERSION_1_LATE) {
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

#else

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdPlaySound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	int number = obj.segment ? GET_SEL32V(_segMan, obj, number) : -1;

	if (musicSlot->resnum != number) { // another sound loaded into struct
		cmdDisposeSound(obj, value);
		cmdInitSound(obj, value);
		// Find slot again :)
		musicSlot = _music->getSlot(obj);
	}
	int16 loop = GET_SEL32V(_segMan, obj, loop);
	debugC(2, kDebugLevelSound, "cmdPlaySound: resource number %d, loop %d", number, loop);

	PUT_SEL32(_segMan, obj, handle, obj);

	if (_soundVersion >= SCI_VERSION_1_EARLY) {
		PUT_SEL32(_segMan, obj, nodePtr, obj);
		PUT_SEL32V(_segMan, obj, min, 0);
		PUT_SEL32V(_segMan, obj, sec, 0);
		PUT_SEL32V(_segMan, obj, frame, 0);
		PUT_SEL32V(_segMan, obj, signal, 0);
	} else {
		PUT_SEL32V(_segMan, obj, state, kSoundPlaying);
	}

	musicSlot->loop = GET_SEL32V(_segMan, obj, loop);
	musicSlot->prio = GET_SEL32V(_segMan, obj, priority);
	if (_soundVersion >= SCI_VERSION_1_LATE)
		musicSlot->volume = GET_SEL32V(_segMan, obj, vol);
	_music->soundPlay(musicSlot);

#endif

}

void SoundCommandParser::cmdDummy(reg_t obj, int16 value) {
	warning("cmdDummy invoked");	// not supposed to occur
}

#ifdef USE_OLD_MUSIC_FUNCTIONS
void SoundCommandParser::changeSoundStatus(reg_t obj, int newStatus) {
	SongHandle handle = FROBNICATE_HANDLE(obj);
	if (obj.segment) {
		_state->sfx_song_set_status(handle, newStatus);
		if (_soundVersion <= SCI_VERSION_0_LATE)
			PUT_SEL32V(_segMan, obj, state, newStatus);
	}
}
#endif

void SoundCommandParser::cmdDisposeSound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	SongHandle handle = FROBNICATE_HANDLE(obj);
	changeSoundStatus(obj, SOUND_STATUS_STOPPED);

	if (obj.segment) {
		_state->sfx_remove_song(handle);

		if (_soundVersion <= SCI_VERSION_0_LATE)
			PUT_SEL32V(_segMan, obj, handle, 0x0000);
	}

#else

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdDisposeSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	cmdStopSound(obj, value);

	_music->soundKill(musicSlot);
	if (_soundVersion >= SCI_VERSION_1_EARLY)
		PUT_SEL32(_segMan, obj, nodePtr, NULL_REG);
	else
		PUT_SEL32V(_segMan, obj, state, kSoundStopped);
#endif
}

void SoundCommandParser::cmdStopSound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	changeSoundStatus(obj, SOUND_STATUS_STOPPED);

	if (_soundVersion >= SCI_VERSION_1_EARLY)
		PUT_SEL32V(_segMan, obj, signal, SIGNAL_OFFSET);
#else
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdStopSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	PUT_SEL32V(_segMan, obj, handle, 0);
	if (_soundVersion <= SCI_VERSION_0_LATE)
		PUT_SEL32V(_segMan, obj, state, kSoundStopped);
	else
		PUT_SEL32V(_segMan, obj, signal, SIGNAL_OFFSET);

	musicSlot->dataInc = 0;
	musicSlot->signal = 0;
	_music->soundStop(musicSlot);
#endif
}

void SoundCommandParser::cmdPauseSound(reg_t obj, int16 value) {
#ifdef USE_OLD_MUSIC_FUNCTIONS
	if (!obj.segment)
		return;

	if (_soundVersion <= SCI_VERSION_0_LATE)
		changeSoundStatus(obj, SOUND_STATUS_SUSPENDED);
	else
		changeSoundStatus(obj, value ? SOUND_STATUS_SUSPENDED : SOUND_STATUS_PLAYING);
#else

	if (!obj.segment) {		// pause the whole playlist
		// Pausing/Resuming the whole playlist was introduced
		// in the SCI1 late sound scheme
		if (_soundVersion <= SCI_VERSION_1_EARLY)
			return;

		_music->pauseAll(value);
	} else {	// pause a playlist slot
		Common::StackLock lock(_music->_mutex);
		MusicEntry *musicSlot = _music->getSlot(obj);
		if (!musicSlot) {
			warning("cmdPauseSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
			return;
		}

		if (_soundVersion <= SCI_VERSION_0_LATE) {
			// Always pause the sound in SCI0 games. It's resumed in cmdResumeSound()
			PUT_SEL32V(_segMan, musicSlot->soundObj, state, kSoundPaused);
			_music->soundPause(musicSlot);
		} else {
			_music->soundToggle(musicSlot, value);
		}
	}

#endif
}

void SoundCommandParser::cmdResumeSound(reg_t obj, int16 value) {
	// SCI0 only command

	if (!obj.segment)
		return;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	changeSoundStatus(obj, SOUND_STATUS_PLAYING);
#else
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdResumeSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	PUT_SEL32V(_segMan, musicSlot->soundObj, state, kSoundPlaying);
	_music->soundResume(musicSlot);
#endif
}

void SoundCommandParser::cmdMuteSound(reg_t obj, int16 value) {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	if (_argc > 1)	// the first parameter is the sound command
		_music->soundSetSoundOn(obj.toUint16());
	_acc = make_reg(0, _music->soundGetSoundOn());
#endif
}

void SoundCommandParser::cmdMasterVolume(reg_t obj, int16 value) {
#ifdef USE_OLD_MUSIC_FUNCTIONS
	_acc = make_reg(0, _state->sfx_getVolume());

	if (obj != SIGNAL_REG)
 		_state->sfx_setVolume(obj.toSint16());
#else
	debugC(2, kDebugLevelSound, "cmdMasterVolume: %d", value);
	_acc = make_reg(0, _music->soundGetMasterVolume());

	if (_argc > 1)	{ // the first parameter is the sound command
		int vol = CLIP<int16>(obj.toSint16(), 0, kMaxSciVolume);
		vol = vol * Audio::Mixer::kMaxMixerVolume / kMaxSciVolume;
		ConfMan.setInt("music_volume", vol);
		ConfMan.setInt("sfx_volume", vol);
		g_engine->syncSoundSettings();
	}
#endif
}

void SoundCommandParser::cmdFadeSound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	SongHandle handle = FROBNICATE_HANDLE(obj);
	if (_soundVersion != SCI_VERSION_1_LATE) {
		/*s->sound_server->command(s, SOUND_COMMAND_FADE_HANDLE, obj, 120);*/ /* Fade out in 2 secs */
		/* FIXME: The next couple of lines actually STOP the handle, rather
		** than fading it! */
		_state->sfx_song_set_status(handle, SOUND_STATUS_STOPPED);
		if (_soundVersion <= SCI_VERSION_0_LATE)
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
#else
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdFadeSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	int volume = musicSlot->volume;

	switch (_argc) {
	case 2: // SCI0
		// SCI0 fades out all the time and when fadeout is done it will also stop the music from playing
		musicSlot->fadeTo = 0;
		musicSlot->fadeStep = -5;
		musicSlot->fadeTickerStep = 10 * 16667 / _music->soundGetTempo();
		musicSlot->fadeTicker = 0;
		break;

	case 5: // SCI01+
	case 6: // SCI1+ (SCI1 late sound scheme), with fade and continue
		musicSlot->fadeTo = CLIP<uint16>(_argv[2].toUint16(), 0, MUSIC_VOLUME_MAX);
		musicSlot->fadeStep = volume > _argv[2].toUint16() ? -_argv[4].toUint16() : _argv[4].toUint16();
		musicSlot->fadeTickerStep = _argv[3].toUint16() * 16667 / _music->soundGetTempo();
		musicSlot->fadeTicker = 0;
		musicSlot->stopAfterFading = (_argc == 6) ? (_argv[5].toUint16() != 0) : false;
		break;

	default:
		error("cmdFadeSound: unsupported argc %d", _argc);
	}

	debugC(2, kDebugLevelSound, "cmdFadeSound: to %d, step %d, ticker %d", musicSlot->fadeTo, musicSlot->fadeStep, musicSlot->fadeTickerStep);
#endif
}

void SoundCommandParser::cmdGetPolyphony(reg_t obj, int16 value) {
#ifdef USE_OLD_MUSIC_FUNCTIONS
	_acc = make_reg(0, _state->sfx_get_player_polyphony());
#else
	_acc = make_reg(0, _music->soundGetVoices());	// Get the number of voices
#endif
}

void SoundCommandParser::cmdUpdateSound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	SongHandle handle = FROBNICATE_HANDLE(obj);
	if (_soundVersion <= SCI_VERSION_0_LATE && obj.segment) {
 		_state->sfx_song_set_loops(handle, GET_SEL32V(_segMan, obj, loop));
 		script_set_priority(_resMan, _segMan, _state, obj, GET_SEL32V(_segMan, obj, pri));
 	}
#else
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdUpdateSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	musicSlot->loop = GET_SEL32V(_segMan, obj, loop);
	int16 objVol = CLIP<int>(GET_SEL32V(_segMan, obj, vol), 0, 255);
	if (objVol != musicSlot->volume)
		_music->soundSetVolume(musicSlot, objVol);
	uint32 objPrio = GET_SEL32V(_segMan, obj, pri);
	if (objPrio != musicSlot->prio)
		_music->soundSetPriority(musicSlot, objPrio);

#endif
}

void SoundCommandParser::cmdUpdateCues(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	int signal = 0;
	int min = 0;
	int sec = 0;
	int frame = 0;
	int result = SI_LOOP; // small hack
	SongHandle handle = FROBNICATE_HANDLE(obj);

	while (result == SI_LOOP)
		result = _state->sfx_poll_specific(handle, &signal);

	switch (result) {
	case SI_ABSOLUTE_CUE:
		debugC(2, kDebugLevelSound, "---    [CUE] %04x:%04x Absolute Cue: %d\n",
		          PRINT_REG(obj), signal);
		debugC(2, kDebugLevelSound, "abs-signal %04X\n", signal);
		PUT_SEL32V(_segMan, obj, signal, signal);
		break;

	case SI_RELATIVE_CUE:
		debugC(2, kDebugLevelSound, "---    [CUE] %04x:%04x Relative Cue: %d\n",
		          PRINT_REG(obj), signal);

		/* FIXME to match commented-out semantics
		 * below, with proper storage of dataInc and
		 * signal in the iterator code. */
		PUT_SEL32V(_segMan, obj, dataInc, signal);
		debugC(2, kDebugLevelSound, "rel-signal %04X\n", signal);
		if (_soundVersion == SCI_VERSION_1_EARLY)
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

	if (_soundVersion == SCI_VERSION_1_EARLY) {
		PUT_SEL32V(_segMan, obj, min, min);
		PUT_SEL32V(_segMan, obj, sec, sec);
		PUT_SEL32V(_segMan, obj, frame, frame);
	}
#else
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdUpdateCues: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	if (musicSlot->pStreamAud) {
		// Update digital sound effect slots
		uint currentLoopCounter = 0;

		if (musicSlot->pLoopStream)
			currentLoopCounter = musicSlot->pLoopStream->getCompleteIterations();

		if (currentLoopCounter != musicSlot->sampleLoopCounter) {
			// during last time we looped at least one time, update loop accordingly
			musicSlot->loop -= currentLoopCounter - musicSlot->sampleLoopCounter;
			musicSlot->sampleLoopCounter = currentLoopCounter;
		}
		if (!_music->soundIsActive(musicSlot)) {
			cmdStopSound(obj, 0);
		} else {
			_music->updateAudioStreamTicker(musicSlot);
		}
		// We get a flag from MusicEntry::doFade() here to set volume for the stream
		if (musicSlot->fadeSetVolume) {
			_music->soundSetVolume(musicSlot, musicSlot->volume);
			musicSlot->fadeSetVolume = false;
		}
	} else {
		// Update MIDI slots
		if (musicSlot->signal == 0) {
			if (musicSlot->dataInc != GET_SEL32V(_segMan, obj, dataInc)) {
				if (_kernel->_selectorCache.dataInc > -1)
					PUT_SEL32V(_segMan, obj, dataInc, musicSlot->dataInc);
				PUT_SEL32V(_segMan, obj, signal, musicSlot->dataInc + 127);
			}
		} else {
			// Sync the signal of the sound object
			PUT_SEL32V(_segMan, obj, signal, musicSlot->signal);
			// We need to do this especially because state selector needs to get updated
			if (musicSlot->signal == SIGNAL_OFFSET)
				cmdStopSound(obj, 0);
		}
	}

	if (musicSlot->fadeCompleted) {
		musicSlot->fadeCompleted = false;
		if (_soundVersion <= SCI_VERSION_0_LATE) {
			cmdStopSound(obj, 0);
		} else {
			PUT_SEL32V(_segMan, obj, signal, SIGNAL_OFFSET);
			if (musicSlot->stopAfterFading)
				cmdStopSound(obj, 0);
		}
	}

	// Sync loop selector for SCI0
	if (_soundVersion <= SCI_VERSION_0_LATE)
		PUT_SEL32V(_segMan, obj, loop, musicSlot->loop);

	musicSlot->signal = 0;

	if (_soundVersion >= SCI_VERSION_1_EARLY) {
		PUT_SEL32V(_segMan, obj, min, musicSlot->ticker / 3600);
		PUT_SEL32V(_segMan, obj, sec, musicSlot->ticker % 3600 / 60);
		PUT_SEL32V(_segMan, obj, frame, musicSlot->ticker);
	}

#endif
}

void SoundCommandParser::cmdSendMidi(reg_t obj, int16 value) {
#ifdef USE_OLD_MUSIC_FUNCTIONS
	//SongHandle handle = FROBNICATE_HANDLE(obj);
	//_state->sfx_send_midi(handle, value, _midiCmd, _controller, _param);
#else
	_music->sendMidiCommand(_midiCommand);
#endif
}

void SoundCommandParser::cmdReverb(reg_t obj, int16 value) {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	_music->setReverb(obj.toUint16() & 0xF);
#endif
}

void SoundCommandParser::cmdSetSoundHold(reg_t obj, int16 value) {
#ifdef USE_OLD_MUSIC_FUNCTIONS
	SongHandle handle = FROBNICATE_HANDLE(obj);
	_state->sfx_song_set_hold(handle, value);
#else
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdSetSoundHold: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	// Set the special hold marker ID where the song should be looped at.
	musicSlot->hold = value;
#endif
}

void SoundCommandParser::cmdGetAudioCapability(reg_t obj, int16 value) {
	// Tests for digital audio support
	_acc = make_reg(0, 1);
}

void SoundCommandParser::cmdStopAllSounds(reg_t obj, int16 value) {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	Common::StackLock(_music->_mutex);

	const MusicList::iterator end = _music->getPlayListEnd();
	for (MusicList::iterator i = _music->getPlayListStart(); i != end; ++i) {
		if (_soundVersion <= SCI_VERSION_0_LATE)
			PUT_SEL32V(_segMan, (*i)->soundObj, state, kSoundStopped);
		else
			PUT_SEL32V(_segMan, (*i)->soundObj, signal, SIGNAL_OFFSET);

		(*i)->dataInc = 0;
		_music->soundStop(*i);
	}
#endif
}

void SoundCommandParser::cmdSetSoundVolume(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

#ifndef USE_OLD_MUSIC_FUNCTIONS
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		// Do not throw a warning if the sound can't be found, as in some games
		// this is called before the actual sound is loaded (e.g. SQ4CD, with the
		// drum sounds of the energizer bunny at the beginning), so this is normal
		// behavior
		//warning("cmdSetSoundVolume: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	debugC(2, kDebugLevelSound, "cmdSetSoundVolume: %d", value);

	value = CLIP<int>(value, 0, MUSIC_VOLUME_MAX);

	if (musicSlot->volume != value) {
		musicSlot->volume = value;
		_music->soundSetVolume(musicSlot, value);
		PUT_SEL32V(_segMan, obj, vol, value);
	}
#endif
}

void SoundCommandParser::cmdSetSoundPriority(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	script_set_priority(_resMan, _segMan, _state, obj, value);
#else
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdSetSoundPriority: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	if (value == -1) {
		// Set priority from the song data
		Resource *song = _resMan->findResource(ResourceId(kResourceTypeSound, musicSlot->resnum), 0);
		if (song->data[0] == 0xf0)
			_music->soundSetPriority(musicSlot, song->data[1]);
		else
			warning("cmdSetSoundPriority: Attempt to unset song priority when there is no built-in value");

		//pSnd->prio=0;field_15B=0
		PUT_SEL32V(_segMan, obj, flags, GET_SEL32V(_segMan, obj, flags) & 0xFD);
	} else {
		// Scripted priority

		//pSnd->field_15B=1;
		PUT_SEL32V(_segMan, obj, flags, GET_SEL32V(_segMan, obj, flags) | 2);
		//DoSOund(0xF,hobj,w)
	}
#endif
}

void SoundCommandParser::cmdSetSoundLoop(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

#ifdef USE_OLD_MUSIC_FUNCTIONS
	if (!GET_SEL32(_segMan, obj, nodePtr).isNull()) {
		SongHandle handle = FROBNICATE_HANDLE(obj);
		_state->sfx_song_set_loops(handle, value);
	}
#else
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		// Apparently, it's perfectly normal for a game to call cmdSetSoundLoop
		// before actually initializing the sound and adding it to the playlist
		// with cmdInitSound. Usually, it doesn't matter if the game doesn't
		// request to loop the sound, so in this case, don't throw any warning,
		// otherwise do, because the sound won't be looped
		if (value == -1) {
			warning("cmdSetSoundLoop: Slot not found (%04x:%04x) and the song was requested to be looped", PRINT_REG(obj));
		} else {
			// Doesn't really matter
		}
		return;
	}
	if (value == -1) {
		musicSlot->loop = 0xFFFF;
	} else {
		musicSlot->loop = 1; // actually plays the music once
	}

	PUT_SEL32V(_segMan, obj, loop, musicSlot->loop);
#endif
}

void SoundCommandParser::cmdSuspendSound(reg_t obj, int16 value) {
	// TODO
	warning("STUB: cmdSuspendSound");
}

#ifndef USE_OLD_MUSIC_FUNCTIONS

void SoundCommandParser::updateSci0Cues() {
	Common::StackLock(_music->_mutex);

	const MusicList::iterator end = _music->getPlayListEnd();
	for (MusicList::iterator i = _music->getPlayListStart(); i != end; ++i) {
		// Is the sound stopped, and the sound object updated too? If yes, skip
		// this sound, as SCI0 only allows one active song
		if ((*i)->signal == 0 && (*i)->status != kSoundPlaying)
			continue;

		cmdUpdateCues((*i)->soundObj, 0);
	}
}

#endif

void SoundCommandParser::clearPlayList() {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	_music->clearPlayList();
#endif
}

void SoundCommandParser::syncPlayList(Common::Serializer &s) {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	_music->saveLoadWithSerializer(s);
#endif
}

void SoundCommandParser::reconstructPlayList(int savegame_version) {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	Common::StackLock lock(_music->_mutex);

	const MusicList::iterator end = _music->getPlayListEnd();
	for (MusicList::iterator i = _music->getPlayListStart(); i != end; ++i) {
		if (savegame_version < 14) {
			(*i)->dataInc = GET_SEL32V(_segMan, (*i)->soundObj, dataInc);
			(*i)->signal = GET_SEL32V(_segMan, (*i)->soundObj, signal);

			if (_soundVersion >= SCI_VERSION_1_LATE)
				(*i)->volume = GET_SEL32V(_segMan, (*i)->soundObj, vol);
		}

		if ((*i)->resnum && _resMan->testResource(ResourceId(kResourceTypeSound, (*i)->resnum))) {
			(*i)->soundRes = new SoundResource((*i)->resnum, _resMan, _soundVersion);
			_music->soundInitSnd(*i);
		} else {
			(*i)->soundRes = 0;
		}
		if ((*i)->status == kSoundPlaying)
			cmdPlaySound((*i)->soundObj, 0);
	}

#endif
}

void SoundCommandParser::printPlayList(Console *con) {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	_music->printPlayList(con);
#endif
}

void SoundCommandParser::printSongInfo(reg_t obj, Console *con) {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	_music->printSongInfo(obj, con);
#endif
}

void SoundCommandParser::stopAllSounds() {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	_music->stopAll();
#endif
}

void SoundCommandParser::startNewSound(int number) {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	Common::StackLock lock(_music->_mutex);

	// Overwrite the first sound in the playlist
	MusicEntry *song = *_music->getPlayListStart();
	reg_t soundObj = song->soundObj;
	cmdDisposeSound(soundObj, 0);
	PUT_SEL32V(_segMan, soundObj, number, number);
	cmdInitSound(soundObj, 0);
	cmdPlaySound(soundObj, 0);
#endif
}

void SoundCommandParser::setMasterVolume(int vol) {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	_music->soundSetMasterVolume(vol);
#endif
}

void SoundCommandParser::pauseAll(bool pause) {
#ifndef USE_OLD_MUSIC_FUNCTIONS
	_music->pauseAll(pause);
#endif
}

} // End of namespace Sci
