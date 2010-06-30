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

#include "common/config-manager.h"
#include "sci/sound/audio.h"
#include "sci/sound/music.h"
#include "sci/sound/soundcmd.h"

#include "sci/engine/kernel.h"
#include "sci/engine/selector.h"

namespace Sci {

#define SCI1_SOUND_FLAG_MAY_PAUSE        1 /* Only here for completeness; The interpreter doesn't touch this bit */
#define SCI1_SOUND_FLAG_SCRIPTED_PRI     2 /* but does touch this */

#define SOUNDCOMMAND(x) _soundCommands.push_back(new MusicEntryCommand(#x, &SoundCommandParser::x))

SoundCommandParser::SoundCommandParser(ResourceManager *resMan, SegManager *segMan, Kernel *kernel, AudioPlayer *audio, SciVersion soundVersion) :
	_resMan(resMan), _segMan(segMan), _kernel(kernel), _audio(audio), _soundVersion(soundVersion) {

	_music = new SciMusic(_soundVersion);
	_music->init();

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
		SOUNDCOMMAND(cmdDummy);	// Longbow demo
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
		error("Sound command parser: unknown sound version %d", _soundVersion);
		break;
	}
}

SoundCommandParser::~SoundCommandParser() {
	for (SoundCommandContainer::iterator i = _soundCommands.begin(); i != _soundCommands.end(); ++i)
		delete *i;

	delete _music;
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

		if (channel)
			channel--; // channel is given 1-based, we are using 0-based

		_midiCommand = (channel | midiCmd) | ((uint32)controller << 8) | ((uint32)param << 16);
	}

	if (command < _soundCommands.size()) {
		if (command != _cmdUpdateCuesIndex) {
			//printf("%s, object %04x:%04x\n", _soundCommands[command]->desc, PRINT_REG(obj));	// debug
			debugC(2, kDebugLevelSound, "%s, object %04x:%04x", _soundCommands[command]->desc, PRINT_REG(obj));
		}

		(this->*(_soundCommands[command]->sndCmd))(obj, value);
	} else {
		error("Invalid sound command requested (%d), valid range is 0-%d", command, _soundCommands.size() - 1);
	}

	return _acc;
}

void SoundCommandParser::cmdInitSound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

	int resourceId = readSelectorValue(_segMan, obj, SELECTOR(number));

	// Check if a track with the same sound object is already playing
	MusicEntry *oldSound = _music->getSlot(obj);
	if (oldSound)
		cmdDisposeSound(obj, value);

	MusicEntry *newSound = new MusicEntry();
	newSound->resourceId = resourceId;
	if (resourceId && _resMan->testResource(ResourceId(kResourceTypeSound, resourceId)))
		newSound->soundRes = new SoundResource(resourceId, _resMan, _soundVersion);
	else
		newSound->soundRes = 0;

	newSound->soundObj = obj;
	newSound->loop = readSelectorValue(_segMan, obj, SELECTOR(loop));
	newSound->priority = readSelectorValue(_segMan, obj, SELECTOR(pri)) & 0xFF;
	if (_soundVersion >= SCI_VERSION_1_EARLY)
		newSound->volume = CLIP<int>(readSelectorValue(_segMan, obj, SELECTOR(vol)), 0, MUSIC_VOLUME_MAX);

	debugC(2, kDebugLevelSound, "cmdInitSound, number %d, loop %d, prio %d, vol %d", resourceId, 
			newSound->loop, newSound->priority, newSound->volume);

	// In SCI1.1 games, sound effects are started from here. If we can find
	// a relevant audio resource, play it, otherwise switch to synthesized
	// effects. If the resource exists, play it using map 65535 (sound
	// effects map)

	if (getSciVersion() >= SCI_VERSION_1_1 && _resMan->testResource(ResourceId(kResourceTypeAudio, resourceId))) {
		// Found a relevant audio resource, play it
		int sampleLen;
		newSound->pStreamAud = _audio->getAudioStream(resourceId, 65535, &sampleLen);
		newSound->soundType = Audio::Mixer::kSpeechSoundType;
	} else {
		if (newSound->soundRes)
			_music->soundInitSnd(newSound);
	}

	_music->pushBackSlot(newSound);

	if (newSound->soundRes || newSound->pStreamAud) {
		// Notify the engine
		if (_soundVersion <= SCI_VERSION_0_LATE)
			writeSelectorValue(_segMan, obj, SELECTOR(state), kSoundInitialized);
		else
			writeSelector(_segMan, obj, SELECTOR(nodePtr), obj);

		writeSelector(_segMan, obj, SELECTOR(handle), obj);
	}
}

void SoundCommandParser::cmdPlaySound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdPlaySound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	int resourceId = obj.segment ? readSelectorValue(_segMan, obj, SELECTOR(number)) : -1;

	if (musicSlot->resourceId != resourceId) { // another sound loaded into struct
		cmdDisposeSound(obj, value);
		cmdInitSound(obj, value);
		// Find slot again :)
		musicSlot = _music->getSlot(obj);
	}

	writeSelector(_segMan, obj, SELECTOR(handle), obj);

	if (_soundVersion >= SCI_VERSION_1_EARLY) {
		writeSelector(_segMan, obj, SELECTOR(nodePtr), obj);
		writeSelectorValue(_segMan, obj, SELECTOR(min), 0);
		writeSelectorValue(_segMan, obj, SELECTOR(sec), 0);
		writeSelectorValue(_segMan, obj, SELECTOR(frame), 0);
		writeSelectorValue(_segMan, obj, SELECTOR(signal), 0);
	} else {
		writeSelectorValue(_segMan, obj, SELECTOR(state), kSoundPlaying);
	}

	musicSlot->loop = readSelectorValue(_segMan, obj, SELECTOR(loop));
	musicSlot->priority = readSelectorValue(_segMan, obj, SELECTOR(priority));
	if (_soundVersion >= SCI_VERSION_1_EARLY)
		musicSlot->volume = readSelectorValue(_segMan, obj, SELECTOR(vol));

	debugC(2, kDebugLevelSound, "cmdPlaySound, number %d, loop %d, prio %d, vol %d", resourceId, 
			musicSlot->loop, musicSlot->priority, musicSlot->volume);

	_music->soundPlay(musicSlot);
}

void SoundCommandParser::cmdDummy(reg_t obj, int16 value) {
	warning("cmdDummy invoked");	// not supposed to occur
}

void SoundCommandParser::cmdDisposeSound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdDisposeSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	cmdStopSound(obj, value);

	_music->soundKill(musicSlot);
	writeSelectorValue(_segMan, obj, SELECTOR(handle), 0);
	if (_soundVersion >= SCI_VERSION_1_EARLY)
		writeSelector(_segMan, obj, SELECTOR(nodePtr), NULL_REG);
	else
		writeSelectorValue(_segMan, obj, SELECTOR(state), kSoundStopped);
}

void SoundCommandParser::cmdStopSound(reg_t obj, int16 value) {
	processStopSound(obj, value, false);
}

void SoundCommandParser::processStopSound(reg_t obj, int16 value, bool sampleFinishedPlaying) {
	if (!obj.segment)
		return;

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdStopSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	if (_soundVersion <= SCI_VERSION_0_LATE) {
		writeSelectorValue(_segMan, obj, SELECTOR(state), kSoundStopped);
	} else {
		writeSelectorValue(_segMan, obj, SELECTOR(handle), 0);
	}

	// Set signal selector in sound SCI0 games only, when the sample has
	// finished playing. If we don't set it at all, we get a problem when using
	// vaporizer on the 2 guys. If we set it all the time, we get no music in
	// sq3new and kq1.
	// FIXME: This *may* be wrong, it's impossible to find out in Sierra DOS
	//        SCI, because SCI0 under DOS didn't have sfx drivers included.
	// We need to set signal in sound SCI1+ games all the time.
	if ((_soundVersion > SCI_VERSION_0_LATE) || sampleFinishedPlaying)
		writeSelectorValue(_segMan, obj, SELECTOR(signal), SIGNAL_OFFSET);

	musicSlot->dataInc = 0;
	musicSlot->signal = 0;
	_music->soundStop(musicSlot);
}

void SoundCommandParser::cmdPauseSound(reg_t obj, int16 value) {
	if (!obj.segment) {		// pause the whole playlist
		// Pausing/Resuming the whole playlist was introduced in the SCI1 late
		// sound scheme.
		if (_soundVersion <= SCI_VERSION_1_EARLY)
			return;

		_music->pauseAll(value);
	} else {	// pause a playlist slot
		MusicEntry *musicSlot = _music->getSlot(obj);
		if (!musicSlot) {
			warning("cmdPauseSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
			return;
		}

		if (_soundVersion <= SCI_VERSION_0_LATE) {
			// Always pause the sound in SCI0 games. It's resumed in cmdResumeSound()
			writeSelectorValue(_segMan, musicSlot->soundObj, SELECTOR(state), kSoundPaused);
			_music->soundPause(musicSlot);
		} else {
			_music->soundToggle(musicSlot, value);
		}
	}
}

void SoundCommandParser::cmdResumeSound(reg_t obj, int16 value) {
	// SCI0 only command

	if (!obj.segment)
		return;

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdResumeSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	writeSelectorValue(_segMan, musicSlot->soundObj, SELECTOR(state), kSoundPlaying);
	_music->soundResume(musicSlot);
}

void SoundCommandParser::cmdMuteSound(reg_t obj, int16 value) {
	if (_argc > 1)	// the first parameter is the sound command
		_music->soundSetSoundOn(obj.toUint16());
	_acc = make_reg(0, _music->soundGetSoundOn());
}

void SoundCommandParser::cmdMasterVolume(reg_t obj, int16 value) {
	debugC(2, kDebugLevelSound, "cmdMasterVolume: %d", value);
	_acc = make_reg(0, _music->soundGetMasterVolume());

	if (_argc > 1)	{ // the first parameter is the sound command
		int vol = CLIP<int16>(obj.toSint16(), 0, kMaxSciVolume);
		vol = vol * Audio::Mixer::kMaxMixerVolume / kMaxSciVolume;
		ConfMan.setInt("music_volume", vol);
		ConfMan.setInt("sfx_volume", vol);
		g_engine->syncSoundSettings();
	}
}

void SoundCommandParser::cmdFadeSound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdFadeSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	int volume = musicSlot->volume;

	// If sound is not playing currently, set signal directly
	if (musicSlot->status != kSoundPlaying) {
		debugC(2, kDebugLevelSound, "cmdFadeSound: fading requested, but sound is currently not playing");
		writeSelectorValue(_segMan, obj, SELECTOR(signal), SIGNAL_OFFSET);
		return;
	}

	switch (_argc) {
	case 2: // SCI0
		// SCI0 fades out all the time and when fadeout is done it will also
		// stop the music from playing
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
}

void SoundCommandParser::cmdGetPolyphony(reg_t obj, int16 value) {
	_acc = make_reg(0, _music->soundGetVoices());	// Get the number of voices
}

void SoundCommandParser::cmdUpdateSound(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdUpdateSound: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	musicSlot->loop = readSelectorValue(_segMan, obj, SELECTOR(loop));
	int16 objVol = CLIP<int>(readSelectorValue(_segMan, obj, SELECTOR(vol)), 0, 255);
	if (objVol != musicSlot->volume)
		_music->soundSetVolume(musicSlot, objVol);
	uint32 objPrio = readSelectorValue(_segMan, obj, SELECTOR(pri));
	if (objPrio != musicSlot->priority)
		_music->soundSetPriority(musicSlot, objPrio);
}

void SoundCommandParser::cmdUpdateCues(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

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
		if ((!_music->soundIsActive(musicSlot)) && (musicSlot->status != kSoundPaused)) {
			processStopSound(obj, 0, true);
		} else {
			_music->updateAudioStreamTicker(musicSlot);
		}
		// We get a flag from MusicEntry::doFade() here to set volume for the stream
		if (musicSlot->fadeSetVolume) {
			_music->soundSetVolume(musicSlot, musicSlot->volume);
			musicSlot->fadeSetVolume = false;
		}
	} else if (musicSlot->pMidiParser) {
		// Update MIDI slots
		if (musicSlot->signal == 0) {
			if (musicSlot->dataInc != readSelectorValue(_segMan, obj, SELECTOR(dataInc))) {
				if (SELECTOR(dataInc) > -1)
					writeSelectorValue(_segMan, obj, SELECTOR(dataInc), musicSlot->dataInc);
				writeSelectorValue(_segMan, obj, SELECTOR(signal), musicSlot->dataInc + 127);
			}
		} else {
			// Sync the signal of the sound object
			writeSelectorValue(_segMan, obj, SELECTOR(signal), musicSlot->signal);
			// We need to do this especially because state selector needs to get updated
			if (musicSlot->signal == SIGNAL_OFFSET)
				cmdStopSound(obj, 0);
		}
	} else {
		// Slot actually has no data (which would mean that a sound-resource w/
		// unsupported data is used.
		//  (example lsl5 - sound resource 744 - it's roland exclusive
		writeSelectorValue(_segMan, obj, SELECTOR(signal), SIGNAL_OFFSET);
		// If we don't set signal here, at least the switch to the mud wrestling
		// room in lsl5 will not work.
	}

	if (musicSlot->fadeCompleted) {
		musicSlot->fadeCompleted = false;
		// We need signal for sci0 at least in iceman as well (room 14, fireworks)
		writeSelectorValue(_segMan, obj, SELECTOR(signal), SIGNAL_OFFSET);
		if (_soundVersion <= SCI_VERSION_0_LATE) {
			cmdStopSound(obj, 0);
		} else {
			if (musicSlot->stopAfterFading)
				cmdStopSound(obj, 0);
		}
	}

	// Sync loop selector for SCI0
	if (_soundVersion <= SCI_VERSION_0_LATE)
		writeSelectorValue(_segMan, obj, SELECTOR(loop), musicSlot->loop);

	musicSlot->signal = 0;

	if (_soundVersion >= SCI_VERSION_1_EARLY) {
		writeSelectorValue(_segMan, obj, SELECTOR(min), musicSlot->ticker / 3600);
		writeSelectorValue(_segMan, obj, SELECTOR(sec), musicSlot->ticker % 3600 / 60);
		writeSelectorValue(_segMan, obj, SELECTOR(frame), musicSlot->ticker);
	}
}

void SoundCommandParser::cmdSendMidi(reg_t obj, int16 value) {
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		// TODO: maybe it's possible to call this with obj == 0:0 and send directly?!
		// if so, allow it
		//_music->sendMidiCommand(_midiCommand);
		warning("cmdSendMidi: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}
	_music->sendMidiCommand(musicSlot, _midiCommand);
}

void SoundCommandParser::cmdReverb(reg_t obj, int16 value) {
	_music->setReverb(obj.toUint16() & 0xF);
}

void SoundCommandParser::cmdSetSoundHold(reg_t obj, int16 value) {
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdSetSoundHold: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	// Set the special hold marker ID where the song should be looped at.
	musicSlot->hold = value;
}

void SoundCommandParser::cmdGetAudioCapability(reg_t obj, int16 value) {
	// Tests for digital audio support
	_acc = make_reg(0, 1);
}

void SoundCommandParser::cmdStopAllSounds(reg_t obj, int16 value) {
	Common::StackLock(_music->_mutex);

	const MusicList::iterator end = _music->getPlayListEnd();
	for (MusicList::iterator i = _music->getPlayListStart(); i != end; ++i) {
		if (_soundVersion <= SCI_VERSION_0_LATE) {
			writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(state), kSoundStopped);
		} else {
			writeSelectorValue(_segMan, obj, SELECTOR(handle), 0);
			writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(signal), SIGNAL_OFFSET);
		}

		(*i)->dataInc = 0;
		_music->soundStop(*i);
	}
}

void SoundCommandParser::cmdSetSoundVolume(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		// Do not throw a warning if the sound can't be found, as in some games
		// this is called before the actual sound is loaded (e.g. SQ4CD, with
		// the drum sounds of the energizer bunny at the beginning), so this is
		// normal behavior.
		//warning("cmdSetSoundVolume: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	debugC(2, kDebugLevelSound, "cmdSetSoundVolume: %d", value);

	value = CLIP<int>(value, 0, MUSIC_VOLUME_MAX);

	if (musicSlot->volume != value) {
		musicSlot->volume = value;
		_music->soundSetVolume(musicSlot, value);
		writeSelectorValue(_segMan, obj, SELECTOR(vol), value);
	}
}

void SoundCommandParser::cmdSetSoundPriority(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("cmdSetSoundPriority: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	if (value == -1) {
		// Set priority from the song data
		Resource *song = _resMan->findResource(ResourceId(kResourceTypeSound, musicSlot->resourceId), 0);
		if (song->data[0] == 0xf0)
			_music->soundSetPriority(musicSlot, song->data[1]);
		else
			warning("cmdSetSoundPriority: Attempt to unset song priority when there is no built-in value");

		//pSnd->prio=0;field_15B=0
		writeSelectorValue(_segMan, obj, SELECTOR(flags), readSelectorValue(_segMan, obj, SELECTOR(flags)) & 0xFD);
	} else {
		// Scripted priority

		//pSnd->field_15B=1;
		writeSelectorValue(_segMan, obj, SELECTOR(flags), readSelectorValue(_segMan, obj, SELECTOR(flags)) | 2);
		//DoSOund(0xF,hobj,w)
	}
}

void SoundCommandParser::cmdSetSoundLoop(reg_t obj, int16 value) {
	if (!obj.segment)
		return;

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		// Apparently, it's perfectly normal for a game to call cmdSetSoundLoop
		// before actually initializing the sound and adding it to the playlist
		// with cmdInitSound. Usually, it doesn't matter if the game doesn't
		// request to loop the sound, so in this case, don't throw any warning,
		// otherwise do, because the sound won't be looped.
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

	writeSelectorValue(_segMan, obj, SELECTOR(loop), musicSlot->loop);
}

void SoundCommandParser::cmdSuspendSound(reg_t obj, int16 value) {
	// TODO
	warning("STUB: cmdSuspendSound");
}

void SoundCommandParser::updateSci0Cues() {
	bool noOnePlaying = true;
	MusicEntry *pWaitingForPlay = NULL;

	_music->_mutex.lock();

	const MusicList::iterator end = _music->getPlayListEnd();
	for (MusicList::iterator i = _music->getPlayListStart(); i != end; ++i) {
		// Is the sound stopped, and the sound object updated too? If yes, skip
		// this sound, as SCI0 only allows one active song.
		if  ((*i)->isQueued) {
			pWaitingForPlay = (*i);
			// FIXME(?): In iceman 2 songs are queued when playing the door
			// sound - if we use the first song for resuming then it's the wrong
			// one. Both songs have same priority. Maybe the new sound function
			// in sci0 is somehow responsible.
			continue;
		}
		if ((*i)->signal == 0 && (*i)->status != kSoundPlaying)
			continue;

		cmdUpdateCues((*i)->soundObj, 0);
		noOnePlaying = false;
	}
	_music->_mutex.unlock();

	if (noOnePlaying && pWaitingForPlay) {
		// If there is a queued entry, play it now ffs: SciMusic::soundPlay()
		pWaitingForPlay->isQueued = false;
		_music->soundPlay(pWaitingForPlay);
	}
}

void SoundCommandParser::clearPlayList() {
	_music->clearPlayList();
}

void SoundCommandParser::printPlayList(Console *con) {
	_music->printPlayList(con);
}

void SoundCommandParser::printSongInfo(reg_t obj, Console *con) {
	_music->printSongInfo(obj, con);
}

void SoundCommandParser::stopAllSounds() {
	_music->stopAll();
}

void SoundCommandParser::startNewSound(int number) {
	Common::StackLock lock(_music->_mutex);

	// Overwrite the first sound in the playlist
	MusicEntry *song = *_music->getPlayListStart();
	reg_t soundObj = song->soundObj;
	cmdDisposeSound(soundObj, 0);
	writeSelectorValue(_segMan, soundObj, SELECTOR(number), number);
	cmdInitSound(soundObj, 0);
	cmdPlaySound(soundObj, 0);
}

void SoundCommandParser::setMasterVolume(int vol) {
	_music->soundSetMasterVolume(vol);
}

void SoundCommandParser::pauseAll(bool pause) {
	_music->pauseAll(pause);
}

} // End of namespace Sci
