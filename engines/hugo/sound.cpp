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
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// sound.c - sound effects and music support

#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/config-manager.h"

#include "audio/decoders/raw.h"
#include "audio/audiostream.h"
#include "audio/midiparser.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/file.h"
#include "hugo/sound.h"
#include "hugo/text.h"

namespace Hugo {

MidiPlayer::MidiPlayer() {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driver = MidiDriver::createMidi(dev);
	assert(_driver);
	_paused = false;


	int ret = _driver->open();
	if (ret == 0) {
		_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
}

void MidiPlayer::play(uint8 *stream, uint16 size) {
	debugC(3, kDebugMusic, "MidiPlayer::play");

	Common::StackLock lock(_mutex);

	stop();
	if (!stream)
		return;

	_midiData = (uint8 *)malloc(size);
	if (_midiData) {
		memcpy(_midiData, stream, size);

		syncVolume();	// FIXME: syncVolume calls setVolume which in turn also locks the mutex! ugh

		_parser = MidiParser::createParser_SMF();
		_parser->loadMusic(_midiData, size);
		_parser->setTrack(0);
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		_isLooping = false;
		_isPlaying = true;
	}
}

void MidiPlayer::pause(bool p) {
	_paused = p;

	for (int i = 0; i < kNumChannels; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_paused ? 0 : _channelsVolume[i] * _masterVolume / 255);
		}
	}
}

void MidiPlayer::onTimer() {
	Common::StackLock lock(_mutex);

	if (!_paused && _isPlaying && _parser) {
		_parser->onTimer();
	}
}

void MidiPlayer::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}


SoundHandler::SoundHandler(HugoEngine *vm) : _vm(vm) {
	_midiPlayer = new MidiPlayer();
	_speakerStream = new Audio::PCSpeaker(_vm->_mixer->getOutputRate());
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
						_speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	DOSSongPtr = 0;
	curPriority = 0;
	pcspkrTimer = 0;
	pcspkrOctave = 3;
	pcspkrNoteDuration = 2;
}

SoundHandler::~SoundHandler() {
	_vm->getTimerManager()->removeTimerProc(&loopPlayer);
	_vm->_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;
	delete _midiPlayer;
}

/**
 * Set the FM music volume from config.mvolume (0..100%)
 */
void SoundHandler::setMusicVolume() {
	_midiPlayer->syncVolume();
}

/**
 * Stop any sound that might be playing
 */
void SoundHandler::stopSound() {
	_vm->_mixer->stopAll();
}

/**
 * Stop any tune that might be playing
 */
void SoundHandler::stopMusic() {
	_midiPlayer->stop();
}

/**
 * Turn music on and off
 */
void SoundHandler::toggleMusic() {
	_vm->_config.musicFl = !_vm->_config.musicFl;

	_midiPlayer->pause(!_vm->_config.musicFl);
}

/**
 * Turn digitized sound on and off
 */
void SoundHandler::toggleSound() {
	_vm->_config.soundFl = !_vm->_config.soundFl;
}

void SoundHandler::playMIDI(sound_pt seq_p, uint16 size) {
	_midiPlayer->play(seq_p, size);
}

/**
 * Read a tune sequence from the sound database and start playing it
 */
void SoundHandler::playMusic(int16 tune) {
	sound_pt seqPtr;                                // Sequence data from file
	uint16 size;                                    // Size of sequence data

	if (_vm->_config.musicFl) {
		_vm->getGameStatus().song = tune;
		seqPtr = _vm->_file->getSound(tune, &size);
		playMIDI(seqPtr, size);
		free(seqPtr);
	}
}

/**
 * Produce various sound effects on supplied stereo channel(s)
 * Override currently playing sound only if lower or same priority
 */
void SoundHandler::playSound(int16 sound, const byte priority) {
	// uint32 dwVolume;                             // Left, right volume of sound
	sound_pt sound_p;                               // Sound data
	uint16 size;                                    // Size of data

	// Sound disabled
	if (!_vm->_config.soundFl || !_vm->_mixer->isReady())
		return;

	syncVolume();
	curPriority = priority;

	// Get sound data
	if ((sound_p = _vm->_file->getSound(sound, &size)) == 0)
		return;

	Audio::AudioStream *stream = Audio::makeRawStream(sound_p, size, 11025, Audio::FLAG_UNSIGNED);
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream);
}

/**
 * Initialize for MCI sound and midi
 */
void SoundHandler::initSound() {
	//_midiPlayer->open();
}

void SoundHandler::syncVolume() {
	int soundVolume;

	if (ConfMan.getBool("sfx_mute") || ConfMan.getBool("mute"))
		soundVolume = -1;
	else
		soundVolume = MIN(255, ConfMan.getInt("sfx_volume"));

	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolume);
	_midiPlayer->syncVolume();
}

/**
 * Check if music is still playing.
 * If not, select the next track in the playlist and play it
 */
void SoundHandler::checkMusic() {
	if (_midiPlayer->isPlaying())
		return;

	for (int i = 0; _vm->_defltTunes[i] != -1; i++) {
		if (_vm->_defltTunes[i] == _vm->getGameStatus().song) {
			if (_vm->_defltTunes[i + 1] != -1)
				playMusic(_vm->_defltTunes[i + 1]);
			else
				playMusic(_vm->_defltTunes[0]);
			break;
		}
	}
}

void SoundHandler::loopPlayer(void *refCon) {
	((SoundHandler*)refCon)->pcspkr_player();
}

/**
 * Decrement last note's timer and see if time to play next note yet.
 * If so, interpret next note in string and play it.  Update ptr to string
 * Timer: >0 - song still going, 0 - Stop note, -1 - Set next note
 */
void SoundHandler::pcspkr_player() {
	static const uint16 pcspkrNotes[8] =  {1352, 1205, 2274, 2026, 1805, 1704, 1518}; // The 3rd octave note counts A..G
	static const uint16 pcspkrSharps[8] = {1279, 1171, 2150, 1916, 1755, 1611, 1435}; // The sharps, A# to B#
	static const uint16 pcspkrFlats[8] =  {1435, 1279, 2342, 2150, 1916, 1755, 1611}; // The flats, Ab to Bb

	_vm->getTimerManager()->removeTimerProc(&loopPlayer);
	_vm->getTimerManager()->installTimerProc(&loopPlayer, 1000000 / 9, this);

	uint16 count;                                   // Value to set timer chip to for note
	bool   cmd_note;

	if (!_vm->_config.soundFl || !_vm->_mixer->isReady())
		return;                                     // Poo!  User doesn't want sound!

	if (!DOSSongPtr)
		return;

	if (!*DOSSongPtr)                               // Song has finished
		return;

	if (!--pcspkrTimer) {                           // timer zero, stop note
		_speakerStream->stop();
		return;
	} else if (pcspkrTimer >= 0) {                  // Note still going
		return;
	}

	// Time to play next note
	do {
		cmd_note = true;
		switch (*DOSSongPtr) {
		case 'O':                                   // Switch to new octave 1..7
			DOSSongPtr++;
			pcspkrOctave = *DOSSongPtr - '0';
			if ((pcspkrOctave < 0) || (pcspkrOctave > 7))
				error("pcspkr_player() - Bad octave");
			DOSSongPtr++;
			break;
		case 'L':                                   // Switch to new duration (in ticks)
			DOSSongPtr++;
			pcspkrNoteDuration = *DOSSongPtr - '0';
			if (pcspkrNoteDuration < 0)
				error("pcspkr_player() - Bad duration");
			pcspkrNoteDuration--;
			DOSSongPtr++;
			break;
		case '<':
		case '^':                                   // Move up an octave
			pcspkrOctave++;
			DOSSongPtr++;
			break;
		case '>':
		case 'v':                                   // Move down an octave
			pcspkrOctave--;
			DOSSongPtr++;
			break;
		default:
			cmd_note = false;
			break;
		}
	} while (cmd_note);

	switch (*DOSSongPtr) {
	case 'A':                                       // The notes.
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
		count = pcspkrNotes[*DOSSongPtr - 'A'];
		switch (DOSSongPtr[1]) {                    // Check for sharp or flat (#, -)
		case '#':
			count = pcspkrSharps[*DOSSongPtr++ - 'A'];
			break;
		case 'b':
			count = pcspkrFlats[*DOSSongPtr++ - 'A'];
			break;
		default:
			break;
		}
		if (pcspkrOctave > 3)                       // Adjust for octave
			count /= (1 << (pcspkrOctave - 3));
		else if (pcspkrOctave < 3)
			count *= (1 << (3 - pcspkrOctave));
		_speakerStream->play(Audio::PCSpeaker::kWaveFormSaw, kHugoCNT / count, (int32) ((1 + pcspkrNoteDuration) * _vm->_normalTPS) * 8);
		pcspkrTimer = pcspkrNoteDuration;
		DOSSongPtr++;
		break;
	case '.':                                       // A rest note
		_speakerStream->stop();
		pcspkrTimer = pcspkrNoteDuration;
		DOSSongPtr++;
		break;
	default:
		warning("pcspkr_player() - Unhandled note");
	}
}

void SoundHandler::loadIntroSong(Common::ReadStream &in) {
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numBuf = in.readUint16BE();
		if (varnt == _vm->_gameVariant)
			DOSIntroSong = _vm->_text->getTextData(numBuf);
	}
}

void SoundHandler::initPcspkrPlayer() {
	_vm->getTimerManager()->installTimerProc(&loopPlayer, 1000000 / 9, this);
}

} // End of namespace Hugo
