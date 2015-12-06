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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "audio/mixer.h"

#include "lab/labfun.h"
#include "lab/music.h"
#include "lab/lab.h"

namespace Lab {

#define MUSICBUFSIZE   (2 * 65536L)
#define SAMPLESPEED    15000L

#define CLOWNROOM           123
#define DIMROOM              80

Music::Music(LabEngine *vm) : _vm(vm) {
	_file = 0;
	_tFile = 0;
	_musicPaused = false;

	_tMusicOn = false;
	_tLeftInFile = 0;

	_leftinfile = 0;

	_musicOn = false;
	_winmusic = false;
	_loopSoundEffect = false;
	_queuingAudioStream = NULL;
	_doNotFilestopSoundEffect = false;
	_lastMusicRoom = 1;
	_doReset = true;
	_waitTillFinished = false;
}

/*****************************************************************************/
/* Figures out which buffer is currently playing based on messages sent to   */
/* it from the Audio device.                                                 */
/*****************************************************************************/
void Music::updateMusic() {
	_vm->_event->processInput();
	_vm->_event->updateMouse();

	if (_musicOn && getPlayingBufferCount() < MAXBUFFERS) {
		// NOTE: We need to use malloc(), cause this will be freed with free()
		// by the music code
		byte *musicBuffer = (byte *)malloc(MUSICBUFSIZE);
		fillbuffer(musicBuffer);

		// Queue a music block, and start the music, if needed
		bool startMusic = false;

		if (!_queuingAudioStream) {
			_queuingAudioStream = Audio::makeQueuingAudioStream(SAMPLESPEED, false);
			startMusic = true;
		}

		byte soundFlags = Audio::FLAG_LITTLE_ENDIAN;
		if (_vm->getPlatform() == Common::kPlatformWindows)
			soundFlags |= Audio::FLAG_16BITS;
		else
			soundFlags |= Audio::FLAG_UNSIGNED;

		_queuingAudioStream->queueBuffer(musicBuffer, MUSICBUFSIZE, DisposeAfterUse::YES, soundFlags);

		if (startMusic)
			_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _queuingAudioStream);
	}
}

uint16 Music::getPlayingBufferCount() {
	return (_queuingAudioStream) ? _queuingAudioStream->numQueuedStreams() : 0;
}

void Music::playSoundEffect(uint16 SampleSpeed, uint32 Length, void *Data) {
	pauseBackMusic();
	stopSoundEffect();

	if (SampleSpeed < 4000)
		SampleSpeed = 4000;

	byte soundFlags = Audio::FLAG_LITTLE_ENDIAN;
	if (_vm->getPlatform() == Common::kPlatformWindows)
		soundFlags |= Audio::FLAG_16BITS;
	else
		soundFlags |= Audio::FLAG_UNSIGNED;

	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream((const byte *)Data, Length, SampleSpeed, soundFlags, DisposeAfterUse::NO);
	uint loops = (_loopSoundEffect) ? 0 : 1;
	Audio::LoopingAudioStream *loopingAudioStream = new Audio::LoopingAudioStream(audioStream, loops);
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, loopingAudioStream);
}

void Music::stopSoundEffect() {
	if (isSoundEffectActive())
		_vm->_mixer->stopHandle(_sfxHandle);
}

bool Music::isSoundEffectActive() const {
	return _vm->_mixer->isSoundHandleActive(_sfxHandle);
}

void Music::fillbuffer(byte *musicBuffer) {
	if (MUSICBUFSIZE < _leftinfile) {
		_file->read(musicBuffer, MUSICBUFSIZE);
		_leftinfile -= MUSICBUFSIZE;
	} else {
		_file->read(musicBuffer, _leftinfile);

		memset((char *)musicBuffer + _leftinfile, 0, MUSICBUFSIZE - _leftinfile);

		_file->seek(0);
		_leftinfile = _file->size();
	}
}

/*****************************************************************************/
/* Starts up the music initially.                                            */
/*****************************************************************************/
void Music::startMusic(bool startatbegin) {
	if (!_musicOn)
		return;

	stopSoundEffect();

	if (startatbegin) {
		_file->seek(0);
		_leftinfile  = _file->size();
	}

	_musicOn = true;
	updateMusic();
}

/*****************************************************************************/
/* Initializes the music buffers.                                            */
/*****************************************************************************/
bool Music::initMusic() {
	_musicOn = true;
	_musicPaused = false;

	const char *filename;

	if (_winmusic)
		filename = "Music:WinGame";
	else
		filename = "Music:BackGrou";

	_file = g_lab->_resource->openDataFile(filename);
	startMusic(true);
	return true;
}

/*****************************************************************************/
/* Frees up the music buffers and closes the file.                           */
/*****************************************************************************/
void Music::freeMusic() {
	_musicOn = false;

	_vm->_mixer->stopHandle(_musicHandle);
	_queuingAudioStream = NULL;

	_vm->_mixer->stopHandle(_sfxHandle);

	delete _file;
	_file = NULL;
}

/*****************************************************************************/
/* Pauses the background music.                                              */
/*****************************************************************************/
void Music::pauseBackMusic() {
	if (!_musicPaused && _musicOn) {
		updateMusic();
		_musicOn = false;
		stopSoundEffect();

		_vm->_mixer->pauseHandle(_musicHandle, true);

		_musicPaused = true;
	}
}

/*****************************************************************************/
/* Resumes the paused background music.                                      */
/*****************************************************************************/
void Music::resumeBackMusic() {
	if (_musicPaused) {
		stopSoundEffect();
		_musicOn = true;

		_vm->_mixer->pauseHandle(_musicHandle, false);

		updateMusic();
		_musicPaused = false;
	}
}

/*****************************************************************************/
/* Turns the music on and off.                                               */
/*****************************************************************************/
void Music::setMusic(bool on) {
	stopSoundEffect();

	if (on && !_musicOn) {
		_musicOn = true;
		startMusic(true);
	} else if (!on && _musicOn) {
		_musicOn = false;
		updateMusic();
	} else
		_musicOn = on;
}

/******************************************************************************/
/* Checks the music that should be playing in a particular room.              */
/******************************************************************************/
void Music::checkRoomMusic() {
	if ((_lastMusicRoom == _vm->_roomNum) || !_musicOn)
		return;

	if (_vm->_roomNum == CLOWNROOM)
		changeMusic("Music:Laugh");
	else if (_vm->_roomNum == DIMROOM)
		changeMusic("Music:Rm81");
	else if (_doReset)
		resetMusic();

	_lastMusicRoom = _vm->_roomNum;
}

/*****************************************************************************/
/* Changes the background music to something else.                           */
/*****************************************************************************/
void Music::changeMusic(const char *newmusic) {
	if (!_tFile) {
		_tFile = _file;
		_tMusicOn = _musicOn;
		_tLeftInFile = _leftinfile + 65536L;

		if (_tLeftInFile > (uint32)_tFile->size())
			_tLeftInFile = _leftinfile;
	}

	_file = g_lab->_resource->openDataFile(newmusic);
	_musicOn = true;   /* turn it off */
	setMusic(false);
	_musicOn = false;  /* turn it back on */
	setMusic(true);
}

/*****************************************************************************/
/* Changes the background music to the original piece playing.               */
/*****************************************************************************/
void Music::resetMusic() {
	if (!_tFile)
		return;

	if (_file->isOpen())
		_file->close();

	_file      = _tFile;
	_leftinfile = _tLeftInFile;

	_file->seek(_file->size() - _leftinfile);

	_musicOn = true;
	setMusic(false);
	updateMusic();

	if (!_tMusicOn) {
		_tFile = 0;
		return;
	}

	_musicOn = _tMusicOn;
	startMusic(false);

	_tFile = 0;
}

/*****************************************************************************/
/* Reads in a music file.  Ignores any graphics.                             */
/*****************************************************************************/
bool Music::readMusic(const char *filename, bool waitTillFinished) {
	Common::File *file = _vm->_resource->openDataFile(filename, MKTAG('D', 'I', 'F', 'F'));
	updateMusic();
	if (!_doNotFilestopSoundEffect)
		stopSoundEffect();

	if (!file)
		return false;

	_vm->_anim->_doBlack = false;
	_vm->_anim->readSound(waitTillFinished, file);

	return true;
}

} // End of namespace Lab
