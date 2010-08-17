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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

/* sound.c - sound effects and music support */

#include "common/system.h"

#include "sound/decoders/raw.h"
#include "sound/audiostream.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/file.h"
#include "hugo/sound.h"

namespace Hugo {

uint16 SeqID;                                       // Device id of (MIDI) sequencer
uint16 SeqVolID;                                    // Low level id to set midi volume
uint16 WavID = 0;                                   // Device id of waveaudio

//HWAVEOUT hwav;                                    // Handle of waveaudio
//LPWAVEHDR lphdr;                                  // WaveOut structure ptr

SoundHandler::SoundHandler(HugoEngine &vm) : _vm(vm) {
}

void SoundHandler::setMusicVolume() {
	/* Set the FM music volume from config.mvolume (0..100%) */
	warning("STUB: setMusicVolume()");

	// uint32 dwVolume;
	//
	// if (config.music) {
	//  dwVolume = config.mvolume * 0xffffL / 100;  // Convert % to 0..0xffff
	//  dwVolume |= dwVolume << 16;                 // Set volume in both stereo words
	//  midiOutSetVolume(SeqVolID, dwVolume);
	// }
}

void SoundHandler::stopSound() {
	/* Stop any sound that might be playing */
	warning("STUB: stopSound()");

	// waveOutReset(hwav);
	// waveOutUnprepareHeader(hwav, lphdr, sizeof(WAVEHDR));
}

void SoundHandler::stopMusic() {
	/* Stop any tune that might be playing */
	warning("STUB: stopMusic()");
	//mciSendCommand(SeqID, MCI_CLOSE, MCI_WAIT, 0);
}

void SoundHandler::toggleMusic() {
// Turn music on and off
	if (_config.musicFl)
		stopMusic();
	_config.musicFl = !_config.musicFl;
	initSound(RESET);
}

void SoundHandler::toggleSound() {
// Turn digitized sound on and off
	_config.soundFl = !_config.soundFl;
	initSound(RESET);
}

void SoundHandler::playMIDI(sound_pt seq_p, uint16 size) {
// Write supplied midi data to a temp file for MCI interface
// If seq_p is NULL, delete temp file

	warning("STUB: playMIDI()");
}


void SoundHandler::playMusic(int16 tune) {
	/* Read a tune sequence from the sound database and start playing it */
	sound_pt seqPtr;                                // Sequence data from file
	uint16 size;                                    // Size of sequence data

	if (_config.musicFl) {
		_vm.getGameStatus().song = tune;
		seqPtr = _vm.file().getSound(tune, &size);
		playMIDI(seqPtr, size);
	}
}


void SoundHandler::playSound(int16 sound, stereo_t channel, byte priority) {
	/* Produce various sound effects on supplied stereo channel(s) */
	/* Override currently playing sound only if lower or same priority */

	// uint32 dwVolume;                             // Left, right volume of sound
	sound_pt sound_p;                               // Sound data
	uint16 size;                                    // Size of data
	static byte curPriority = 0;                    // Priority of currently playing sound
	//
	/* Sound disabled */
	if (!_config.soundFl || !_vm._mixer->isReady())
		return;
	//
	// // See if last wave still playing - if so, check priority
	// if (waveOutUnprepareHeader(hwav, lphdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
	//  if (priority < curPriority)                 // Don't override unless priority >= current
	//      return;
	//  else
	//      Stop_sound();
	curPriority = priority;
	//
	/* Get sound data */
	if ((sound_p = _vm.file().getSound(sound, &size)) == NULL)
		return;

	Audio::AudioStream *stream = Audio::makeRawStream(sound_p, size, 11025, Audio::FLAG_UNSIGNED);
	_vm._mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, stream);

}

void SoundHandler::initSound(inst_t action) {
	/* Initialize for MCI sound and midi */

	warning("STUB: initSound()");
}

void SoundHandler::pauseSound(bool activeFl, int hTask) {
// Pause and restore music, sound on losing activity to hTask
// Don't stop music if we are parent of new task, i.e. WinHelp()
// or config.music_bkg is TRUE.

//TODO: Is 'hTask' still useful ?

	static bool firstFl = true;
	static bool musicFl, soundFl;

	if (firstFl) {
		firstFl = false;
		musicFl = _config.musicFl;
		soundFl = _config.soundFl;
	}

	// Kill or restore music, sound
	if (activeFl) { // Remember states, reset WinHelp flag
		_config.musicFl = musicFl;
		_config.soundFl = soundFl;
		_vm.getGameStatus().helpFl = false;
	} else {    // Store states and disable
		musicFl = _config.musicFl;
		soundFl = _config.soundFl;

		// Don't disable music during WinHelp() or config.music_bkg
		if (!_vm.getGameStatus().helpFl && !_config.backgroundMusicFl) {
			_config.musicFl = false;
			_config.soundFl = false;
		}
	}
	initSound(RESET);
}

} // end of namespace Hugo
