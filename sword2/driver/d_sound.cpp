/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

// FIXME: One feature still missing is the original's DipMusic() function
// which, as far as I can understand, softened the music volume when someone
// was speaking, but only if the music was playing loudly at the time.
//
// I'm not sure if we can implement this in any sensible fashion - I don't
// think we have that fine-grained control over the mixer - or if we really
// want it anyway.
//
// Simply adjusting the volume paramters to flow() is not enough. If you
// only adjust them a little you won't hear the difference anyway, and if you
// adjust them a lot it will sound really bad.
//
// Does anyone who can run the original interpreter have any
// opinions on this?

#include "common/stdafx.h"
#include "common/file.h"
#include "sword2/sword2.h"
#include "sound/rate.h"

namespace Sword2 {

static File fpMus;

// Decompression macros
#define GetCompressedShift(n)      ((n) >> 4)
#define GetCompressedSign(n)       (((n) >> 3) & 1)
#define GetCompressedAmplitude(n)  ((n) & 7)

static void premix_proc(void *param, int16 *data, uint len) {
	((Sound *) param)->fxServer(data, len);
}

Sound::Sound(Sword2Engine *vm) {
	_vm = vm;
	_mutex = _vm->_system->create_mutex();

	memset(_fx, 0, sizeof(_fx));

	_soundOn = true;

	_speechStatus = false;
	_speechPaused = false;
	_speechMuted = false;
	_speechVol = 14;

	_fxPaused = false;
	_fxMuted = false;
	_fxVol = 14;

	_musicVol = 16;
	_musicMuted = false;

	_converter = makeRateConverter(_music[0].getRate(), _vm->_mixer->getOutputRate(), _music[0].isStereo(), false);

	_vm->_mixer->setupPremix(premix_proc, this);
}

Sound::~Sound() {
	_vm->_mixer->setupPremix(0, 0);
	delete _converter;
	for (int i = 0; i < MAXFX; i++)
		stopFxHandle(i);
	if (_mutex)
		_vm->_system->delete_mutex(_mutex);
}

void Sound::fxServer(int16 *data, uint len) {
	Common::StackLock lock(_mutex);

	if (!_soundOn)
		return;

	updateCompSampleStreaming(data, len);

	if (!_music[0]._streaming && !_music[1]._streaming && fpMus.isOpen())
		fpMus.close();
}

/**
 * This function creates the pan table.
 */

// FIXME: Could we use the FLAG_REVERSE_STEREO mixer flag instead?

void Sound::buildPanTable(bool reverse) {
	int i;

	for (i = 0; i < 16; i++) {
		_panTable[i] = -127 + i * 8;
		_panTable[i + 17] = (i + 1) * 8 - 1;
	}

	_panTable[16] = 0;

	if (reverse) {
		for (i = 0; i < 33; i++)
			_panTable[i] = -_panTable[i];
	}
}

// ----------------------------------------------------------------------------
// MUSIC
// ----------------------------------------------------------------------------

// All music is encoded at 22050 Hz so this means fading takes 3 seconds.
#define FADE_SAMPLES 66150

int32 Sound::_musicVolTable[17] = {
	  0,  15,  31,  47,  63,  79,  95, 111, 127,
	143, 159, 175, 191, 207, 223, 239, 255
};

void MusicHandle::fadeDown(void) {
	if (_fading < 0)
		_fading = -_fading;
	else if (_fading == 0)
		_fading = FADE_SAMPLES;
}

void MusicHandle::fadeUp(void) {
	if (_fading > 0)
		_fading = -_fading;
	else if (_fading == 0)
		_fading = -1;
}

int32 MusicHandle::play(const char *filename, uint32 musicId, bool looping) {
	// The assumption here is that we are never playing music from two
	// different files at the same time.

	if (!fpMus.isOpen())
		fpMus.open(filename);

	if (!fpMus.isOpen())
		return RDERR_INVALIDFILENAME;

	fpMus.seek((musicId + 1) * 8, SEEK_SET);
	_fileStart = fpMus.readUint32LE();

	uint32 len = fpMus.readUint32LE();

	if (!_fileStart || !len)
		return RDERR_INVALIDID;

	_fileEnd = _fileStart + len;
	_filePos = _fileStart;
	_streaming = true;
	_firstTime = true;
	_looping = looping;
	fadeUp();
	return RD_OK;
}

void MusicHandle::stop(void) {
	_streaming = false;
	_looping = false;
	_fading = 0;
}

int MusicHandle::readBuffer(int16 *buffer, const int numSamples) {
	assert(numSamples > 0);
	int samples;

	// Assume the file handle has been correctly positioned already.

	for (samples = 0; samples < numSamples && !endOfData(); samples++) {
		int16 out;
		if (_firstTime) {
			_lastSample = fpMus.readUint16LE();
			_filePos += 2;
			_firstTime = false;
			out = _lastSample;
		} else {
			uint8 in = fpMus.readByte();
			uint16 delta = GetCompressedAmplitude(in) << GetCompressedShift(in);
		
			if (GetCompressedSign(in))
				out = _lastSample - delta;
			else
				out = _lastSample + delta;
		
			_filePos++;
			_lastSample = out;
		
			if (_looping) {
				if (_filePos >= _fileEnd) {
					_firstTime = true;
					_filePos = _fileStart;
					fpMus.seek(_filePos, SEEK_SET);
				}
			} else {
				// Fade out at the end of the music, unless it already is.
				if (_fileEnd - _filePos <= FADE_SAMPLES && _fading <= 0)
					fadeDown();
			}
		
			if (_fading > 0) {
				if (--_fading == 0) {
					_streaming = false;
					_looping = false;
				}
				out = (out * _fading) / FADE_SAMPLES;
			} else if (_fading < 0) {
				_fading--;
				out = -(out * _fading) / FADE_SAMPLES;
				if (_fading == -FADE_SAMPLES)
					_fading = 0;
			}
		}

		*buffer++ = out;
	}

	return samples;
}

bool MusicHandle::endOfData(void) const {
	return (!_streaming || _filePos >= _fileEnd);
}

/**
 * Mutes/Unmutes the music.
 * @param mute If mute is false, restore the volume to the last set master
 * level. Otherwise the music is muted (volume 0).
 */

void Sound::muteMusic(bool mute) {
	_musicMuted = mute;
}

/**
 * @return the music's mute state, true if mute, false if not mute
 */

bool Sound::isMusicMute(void) {
	return _musicMuted;
}

/**
 * Set the volume of any future as well as playing music.
 * @param volume volume, from 0 (silent) to 16 (max)
 */

void Sound::setMusicVolume(uint8 volume) {
	if (volume > 16)
		volume = 16;

	_musicVol = volume;
}

/**
 * @return the volume setting for music
 */

uint8 Sound::getMusicVolume(void) {
	return _musicVol;
}

/**
 * Stops the music dead in its tracks. Any music that is currently being
 * streamed is paused.
 */

void Sound::pauseMusic(void) {
	Common::StackLock lock(_mutex);

	if (_soundOn) {
		for (int i = 0; i < MAXMUS; i++)
			_music[i]._paused = true;
	}
}

/**
 * Restarts the music from where it was stopped.
 */

void Sound::unpauseMusic(void) {
	Common::StackLock lock(_mutex);

	if (_soundOn) {
		for (int i = 0; i < MAXMUS; i++)
			_music[i]._paused = false;
	}
}

/**
 * Fades out and stops the music.
 */

void Sound::stopMusic(void) {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < MAXMUS; i++)
		_music[i].fadeDown();
}

/**
 * Save/Restore information about current music so that we can restore it
 * after the credits.
 */

void Sound::saveMusicState(void) {
	Common::StackLock lock(_mutex);

	int saveStream;

	if (_music[0]._streaming && _music[0]._fading <= 0) {
		saveStream = 0;
	} else if (_music[1]._streaming && _music[0]._fading <= 0) {
		saveStream = 1;
	} else {
		_music[2]._streaming = false;
		savedMusicFilename = NULL;
		return;
	}

	_music[2]._streaming = true;
	_music[2]._fading = 0;
	_music[2]._looping = _music[saveStream]._looping;
	_music[2]._fileStart = _music[saveStream]._fileStart;
	_music[2]._filePos = _music[saveStream]._filePos;
	_music[2]._fileEnd = _music[saveStream]._fileEnd;
	_music[2]._lastSample = _music[saveStream]._lastSample;

	if (fpMus.isOpen())
		savedMusicFilename = strdup(fpMus.name());
	else
		savedMusicFilename = NULL;
}

void Sound::restoreMusicState(void) {
	Common::StackLock lock(_mutex);

	// Fade out any music that happens to be playing

	for (int i = 0; i < MAXMUS; i++) {
		if (_music[i]._streaming)
			_music[i].fadeDown();
	}

	if (!_music[2]._streaming)
		return;

	int restoreStream;

	if (!_music[0]._streaming)
		restoreStream = 0;
	else
		restoreStream = 1;

	_music[restoreStream]._streaming = true;
	_music[restoreStream]._fading = 0;
	_music[restoreStream]._looping = _music[2]._looping;
	_music[restoreStream]._fileStart = _music[2]._fileStart;
	_music[restoreStream]._filePos = _music[2]._filePos;
	_music[restoreStream]._fileEnd = _music[2]._fileEnd;
	_music[restoreStream]._lastSample = _music[2]._lastSample;
	_music[restoreStream].fadeUp();

	if (savedMusicFilename) {
		if (fpMus.isOpen())
			fpMus.close();

		fpMus.open(savedMusicFilename);
		free(savedMusicFilename);
		savedMusicFilename = NULL;
	}
}

void Sound::playLeadOut(uint8 *leadOut) {
	int i;

	if (!leadOut)
		return;

	playFx(0, leadOut, 0, 0, RDSE_FXLEADOUT);

	i = getFxIndex(-1);

	if (i == MAXFX) {
		warning("playLeadOut: Can't find lead-out sound handle");
		return;
	}

	while (_fx[i]._handle.isActive()) {
		_vm->_graphics->updateDisplay();
		_vm->_system->delay_msecs(30);
	}
}

/**
 * Streams music from a cluster file.
 * @param filename the file name of the music cluster file
 * @param musicId the id of the music to stream
 * @param looping true if the music is to loop back to the start
 * @return RD_OK or an error code
 */

int32 Sound::streamCompMusic(const char *filename, uint32 musicId, bool looping) {
	Common::StackLock lock(_mutex);

	int32 primaryStream = -1;
	int32 secondaryStream = -1;

	// If both music streams are playing, one of them will have to go.

	if (_music[0]._streaming && _music[1]._streaming) {
		if (_music[0]._fading == 0 && _music[1]._fading == 0) {
			// None of them are fading. This shouldn't happen, so
			// just pick one and be done with it.
			primaryStream = 0;
		} else if (_music[0]._fading != 0 && _music[1]._fading == 0) {
			// Stream 0 is fading, so pick that one.
			primaryStream = 0;
		} else if (_music[0]._fading == 0 && _music[1]._fading > 0) {
			// Stream 1 is fading, so pick that one.
			primaryStream = 1;
		} else {
			// Both streams are fading. Pick the one that is
			// closest to silent.
			if (ABS(_music[0]._fading) < ABS(_music[1]._fading))
				primaryStream = 0;
			else
				primaryStream = 1;
		}

		_music[primaryStream].stop();
	}

	// Pick the available music stream. If no music is playing it doesn't
	// matter which we use, so pick the first one.

	if (_music[0]._streaming || _music[1]._streaming) {
		if (_music[0]._streaming) {
			primaryStream = 1;
			secondaryStream = 0;
		} else {
			primaryStream = 0;
			secondaryStream = 1;
		}
	} else
		primaryStream = 0;

	// Don't start streaming if the volume is off.
	if (isMusicMute())
		return RD_OK;

	// Start other music stream fading out
	if (secondaryStream != -1)
		_music[secondaryStream].fadeDown();

	return _music[primaryStream].play(filename, musicId, looping);
}

void Sound::updateCompSampleStreaming(int16 *data, uint len) {
	for (int i = 0; i < MAXMUS; i++) {
		if (!_music[i]._streaming || _music[i]._paused)
			continue;

		st_volume_t volume = _musicMuted ? 0 : _musicVolTable[_musicVol];

		fpMus.seek(_music[i]._filePos, SEEK_SET);
		_converter->flow(_music[i], data, len, volume, volume);
	}

	// DipMusic();
}

int32 Sound::dipMusic(void) {
	// disable this func for now
	return RD_OK;

/*
	int32				 len;
	int32 				 readCursor, writeCursor;
	int32				 dwBytes1, dwBytes2;
	int16				*sample;
	int32				 total = 0;
	int32				 i;
	int32				 status;
	LPVOID 				 lpv1, lpv2;
	HRESULT				 hr = DS_OK;
	LPDIRECTSOUNDBUFFER  dsbMusic = NULL;

	int32				 currentMusicVol = musicVolTable[musicVol];
	int32				 minMusicVol;

	// Find which music buffer is currently playing
	for (i = 0; i<MAXMUS && !dsbMusic; i++)
	{
		if (musStreaming[i] && musFading[i] == 0)
			dsbMusic = lpDsbMus[i];
	}

	if ((!_musicMuted) && dsbMusic && (!_speechMuted) && (musicVol>2))
	{
		minMusicVol = musicVolTable[musicVol - 3];

		if (_speechStatus)
		{
			IDirectSoundBuffer_GetStatus(dsbSpeech, &status);
			if ((hr = IDirectSoundBuffer_GetCurrentPosition(dsbMusic, &readCursor, &writeCursor)) != DS_OK)
				return hr;

			len = 44100 / 12 ;//  12th of a second

			if ((hr = IDirectSoundBuffer_Lock(dsbMusic, readCursor, len, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0)) != DS_OK)
				return hr;

			for (i = 0, sample = (int16*)lpv1; sample<(int16*)((int8*)lpv1+dwBytes1); sample+= 30, i++)  // 60 samples
			{
				if (*sample>0)
					total += *sample;
				else
					total -= *sample;
			}

			total /= i;

			total = minMusicVol + ( ( (currentMusicVol - minMusicVol) * total ) / 8000);

			if (total > currentMusicVol)
				total = currentMusicVol;

			IDirectSoundBuffer_SetVolume(dsbMusic, total);

			IDirectSoundBuffer_Unlock(dsbMusic,lpv1,dwBytes1,lpv2,dwBytes2);
		}
		else
		{
			IDirectSoundBuffer_GetVolume(dsbMusic, &total);
			total += 50;
			if (total > currentMusicVol)
				total = currentMusicVol;

			IDirectSoundBuffer_SetVolume(dsbMusic, total);
		}
	}
	
	return hr;
*/
}

/**
 * @return the time left for the current music, in seconds.
 */

int32 Sound::musicTimeRemaining(void) {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < MAXMUS; i++) {
		if (_music[i]._streaming && _music[i]._fading <= 0)
			return (_music[i]._fileEnd - _music[i]._filePos) / _music[i].getRate();
	}

	return 0;
}

// ----------------------------------------------------------------------------
// SPEECH
// ----------------------------------------------------------------------------

/**
 * Mutes/Unmutes the speech.
 * @param mute If mute is false, restore the volume to the last set master
 * level. Otherwise the speech is muted (volume 0).
 */

void Sound::muteSpeech(bool mute) {
	_speechMuted = mute;

	if (getSpeechStatus() == RDSE_SAMPLEPLAYING) {
		byte volume = mute ? 0 : 16 * _speechVol;

		_vm->_mixer->setChannelVolume(_soundHandleSpeech, volume);
	}
}

/**
 * @return the speech's mute state, true if mute, false if not mute
 */

bool Sound::isSpeechMute(void) {
	return _speechMuted;
}

/**
 * Set the volume of any future as well as playing speech samples.
 * @param volume volume, from 0 (silent) to 14 (max)
 */

void Sound::setSpeechVolume(uint8 volume) {
	if (volume > 14)
		volume = 14;

	_speechVol = volume;

	if (_soundHandleSpeech.isActive() && !_speechMuted && getSpeechStatus() == RDSE_SAMPLEPLAYING) {
		_vm->_mixer->setChannelVolume(_soundHandleSpeech, 16 * _speechVol);
	}
}

/**
 * @return the volume setting for speech
 */

uint8 Sound::getSpeechVolume(void) {
	return _speechVol;
}

/**
 * Stops the speech dead in its tracks.
 */

void Sound::pauseSpeech(void) {
	if (getSpeechStatus() == RDSE_SAMPLEPLAYING) {
		_speechPaused = true;
		_vm->_mixer->pauseHandle(_soundHandleSpeech, true);
	}
}

/**
 * Restarts the speech from where it was stopped.
 */

void Sound::unpauseSpeech(void) {
	if (_speechPaused) {
		_speechPaused = false;
		_vm->_mixer->pauseHandle(_soundHandleSpeech, false);
	}
}

/**
 * Stops the speech from playing.
 */

int32 Sound::stopSpeech(void) {
	if (!_soundOn)
		return RD_OK;
  
	if (_speechStatus) {
		_vm->_mixer->stopHandle(_soundHandleSpeech);
		_speechStatus = false;
		return RD_OK;
	}
	return RDERR_SPEECHNOTPLAYING;
}

/**
 * @return Either RDSE_SAMPLEPLAYING or RDSE_SAMPLEFINISHED
 */

int32 Sound::getSpeechStatus(void) {
	if (!_soundOn || !_speechStatus)
		return RDSE_SAMPLEFINISHED;

	if (_speechPaused)
		return RDSE_SAMPLEPLAYING;

	if (!_soundHandleSpeech.isActive()) {
		_speechStatus = false;
		return RDSE_SAMPLEFINISHED;
	}
	return RDSE_SAMPLEPLAYING;
}

/**
 * Returns either RDSE_QUIET or RDSE_SPEAKING
 */

int32 Sound::amISpeaking(void) {
	if (!_speechMuted && !_speechPaused && _soundHandleSpeech.isActive())
		return RDSE_SPEAKING;

	return RDSE_QUIET;
}

/**
 * This function loads and decompresses a list of speech from a cluster, but
 * does not play it. This is primarily used by playCompSpeech(), but also to
 * store the voice-overs for the animated cutscenes until they are played.
 * @param filename the file name of the speech cluster file
 * @param speechid the text line id used to reference the speech
 * @param buf a pointer to the buffer that will be allocated for the sound
 */ 

uint32 Sound::preFetchCompSpeech(const char *filename, uint32 speechid, uint16 **buf) {
	uint32 i;
	uint8 *data8;
	uint32 speechPos, speechLength;
	File fp;
	uint32 bufferSize;

	*buf = NULL;

	// Open the speech cluster and find the data offset & size
	if (!fp.open(filename))
		return 0;

	fp.seek((speechid + 1) * 8, SEEK_SET);

	speechPos = fp.readUint32LE();
	speechLength = fp.readUint32LE();

	if (!speechPos || !speechLength) {
		fp.close();
		return 0;
	}

	// Create a temporary buffer for compressed speech
	data8 = (uint8 *) malloc(speechLength);
	if (!data8) {
		fp.close();
		return 0;
	}

	fp.seek(speechPos, SEEK_SET);

	if (fp.read(data8, speechLength) != speechLength) {
		fp.close();
		free(data8);
		return 0;
	}

	fp.close();

	// Decompress data into speech buffer.

	bufferSize = (speechLength - 1) * 2;

	*buf = (uint16 *) malloc(bufferSize);
	if (!*buf) {
		free(data8);
		return 0;
	}

	uint16 *data16 = *buf;

	// Starting Value
	data16[0] = READ_LE_UINT16(data8);

	for (i = 1; i < speechLength - 1; i++) {
		if (GetCompressedSign(data8[i + 1]))
			data16[i] = data16[i - 1] - (GetCompressedAmplitude(data8[i + 1]) << GetCompressedShift(data8[i + 1]));
		else
			data16[i] = data16[i - 1] + (GetCompressedAmplitude(data8[i + 1]) << GetCompressedShift(data8[i + 1]));
	}

	free(data8);
	return bufferSize;
}

/**
 * This function loads, decompresses and plays a line of speech. An error
 * occurs if speech is already playing.
 * @param filename the name of the speech cluster file
 * @param speechid the text line id used to reference the speech
 * @param vol volume, 0 (minimum) to 16 (maximum)
 * @param pan panning, -16 (full left) to 16 (full right)
 */

int32 Sound::playCompSpeech(const char *filename, uint32 speechid, uint8 vol, int8 pan) {
	if (_speechMuted)
		return RD_OK;

	uint16 *data16;
	uint32 bufferSize;
	
	if (getSpeechStatus() == RDERR_SPEECHPLAYING)
		return RDERR_SPEECHPLAYING;

	bufferSize = preFetchCompSpeech(filename, speechid, &data16);

	// We don't know exactly what went wrong here.
	if (bufferSize == 0)
		return RDERR_OUTOFMEMORY;

	// Modify the volume according to the master volume

	byte volume = _speechMuted ? 0 : vol * _speechVol;
	int8 p = _panTable[pan + 16];

	// Start the speech playing

	_speechPaused = true;
			
	uint32 flags = SoundMixer::FLAG_16BITS | SoundMixer::FLAG_AUTOFREE;

#ifndef SCUMM_BIG_ENDIAN
	flags |= SoundMixer::FLAG_LITTLE_ENDIAN;
#endif

	_vm->_mixer->playRaw(&_soundHandleSpeech, data16, bufferSize, 22050, flags, -1, volume, p);

	_speechStatus = true;

	// DipMusic();

	return RD_OK;
}

// ----------------------------------------------------------------------------
// SOUND EFFECTS
// ----------------------------------------------------------------------------

/**
 * @return the index of the sound effect with the ID passed in.
 */

int32 Sound::getFxIndex(int32 id) {
	for (int i = 0; i < MAXFX; i++) {
		if (_fx[i]._id == id)
			return i;
	}

	return MAXFX;
}

/**
 * Mutes/Unmutes the sound effects.
 * @param mute If mute is false, restore the volume to the last set master
 * level. Otherwise the sound effects are muted (volume 0).
 */

void Sound::muteFx(bool mute) {
	_fxMuted = mute;

	// Now update the volume of any fxs playing
	for (int i = 0; i < MAXFX; i++) {
		if (_fx[i]._id) {
			byte volume = mute ? 0 : _fx[i]._volume * _fxVol;

			_vm->_mixer->setChannelVolume(_fx[i]._handle, volume);
		}
	}
}

/**
 * @return the sound effects's mute state, true if mute, false if not mute
 */

bool Sound::isFxMute(void) {
	return _fxMuted;
}

/**
 * @return the master volume setting for sound effects
 */

uint8 Sound::getFxVolume(void) {
	return _fxVol;
}

/**
 * Set the master volume of all sound effects. The effects still have their
 * own volume setting as well as the master volume.
 * @param volume volume, from 0 (silent) to 14 (max)
 */

void Sound::setFxVolume(uint8 volume) {
	if (volume > 14)
		volume = 14;

	_fxVol = volume;

	if (_fxMuted)
		return;

	// Now update the volume of any fxs playing
	for (int i = 0; i < MAXFX; i++)
		if (_fx[i]._id)
			_vm->_mixer->setChannelVolume(_fx[i]._handle, _fx[i]._volume * _fxVol);
}

/**
 * Sets the volume and pan of the sample which is currently playing
 * @param id the id of the sample
 * @param vol volume
 * @param pan panning
 */

int32 Sound::setFxIdVolumePan(int32 id, uint8 vol, int8 pan) {
	int32 i = getFxIndex(id);

	if (i == MAXFX)
		return RDERR_FXNOTOPEN;

	if (vol > 14)
		vol = 14;

	_fx[i]._volume = vol;

	if (!_fxMuted) {
		_vm->_mixer->setChannelVolume(_fx[i]._handle, _fx[i]._volume * _fxVol);
		_vm->_mixer->setChannelPan(_fx[i]._handle, _panTable[pan + 16]);
	}

	return RD_OK;
}

int32 Sound::setFxIdVolume(int32 id, uint8 vol) {
	int32 i = getFxIndex(id);

	if (i == MAXFX)
		return RDERR_FXNOTOPEN;

	_fx[i]._volume = vol;

	if (!_fxMuted)
		_vm->_mixer->setChannelVolume(_fx[i]._handle, vol * _fxVol);

	return RD_OK;
}


void Sound::pauseFx(void) {
	if (_fxPaused)
		return;

	for (int i = 0; i < MAXFX; i++) {
		if (_fx[i]._id) {
			_vm->_mixer->pauseHandle(_fx[i]._handle, true);
			_fx[i]._paused = true;
		} else
			_fx[i]._paused = false;
	}

	_fxPaused = true;
}

void Sound::pauseFxForSequence(void) {
	if (_fxPaused)
		return;

	for (int i = 0; i < MAXFX; i++) {
		if (_fx[i]._id && _fx[i]._id != -2) {
			_vm->_mixer->pauseHandle(_fx[i]._handle, true);
			_fx[i]._paused = true;
		} else
			_fx[i]._paused = false;
	}

	_fxPaused = true;
}

void Sound::unpauseFx(void) {
	if (!_fxPaused)
		return;

	for (int i = 0; i < MAXFX; i++)
		if (_fx[i]._paused && _fx[i]._id)
			_vm->_mixer->pauseHandle(_fx[i]._handle, false);

	_fxPaused = false;
}

bool Sound::isFxPlaying(int32 id) {
	int i;

	i = getFxIndex(id);
	if (i == MAXFX)
		return false;

	return _fx[i]._handle.isActive();
}

/**
 * This function opens a sound effect ready for playing. A unique id should be
 * passed in so that each effect can be referenced individually.
 * @param id the unique sound id
 * @param data the WAV data
 * @warning Zero is not a valid id
 */

int32 Sound::openFx(int32 id, uint8 *data) {
	if (!_soundOn)
		return RD_OK;

	if (id == 0)
		return RDERR_INVALIDID;

	if (getFxIndex(id) != MAXFX)
		return RDERR_FXALREADYOPEN;

	// Find a free slot
	int32 fxi = getFxIndex(0);

	if (fxi == MAXFX) {
		warning("openFx: Running out of sound slots");

		// There isn't any free sound handle available. This usually
		// shouldn't happen, but if it does we expire the first sound
		// effect that isn't currently playing.

		for (fxi = 0; fxi < MAXFX; fxi++)
			if (!_fx[fxi]._handle.isActive())
				break;

		// Still no dice? I give up!

		if (fxi == MAXFX) {
			warning("openFx: No free sound slots");
			return RDERR_NOFREEBUFFERS;
		}
	}

	if (READ_UINT32(data) != MKID('RIFF') || READ_UINT32(data + 8) != MKID('WAVE') || READ_UINT32(data + 12) != MKID('fmt ')) {
		warning("openFx: Not a valid WAV file");
		return RDERR_INVALIDWAV;
	}

	_fx[fxi]._id = id;
	_fx[fxi]._flags = SoundMixer::FLAG_16BITS | SoundMixer::FLAG_LITTLE_ENDIAN;

	if (READ_LE_UINT16(data + 22) == 2)
		_fx[fxi]._flags |= SoundMixer::FLAG_STEREO;

	_fx[fxi]._rate = READ_LE_UINT16(data + 24);

	data += READ_LE_UINT32(data + 16) + 20;

	if (READ_UINT32(data) != MKID('data')) {
		warning("openFx: WAV file has no 'data' chunk");
		return RDERR_INVALIDWAV;
	}

	_fx[fxi]._bufSize = READ_LE_UINT32(data + 4);

	// Fill the speech buffer with data
	free(_fx[fxi]._buf);
	_fx[fxi]._buf = (uint16 *) malloc(_fx[fxi]._bufSize);
	memcpy(_fx[fxi]._buf, data + 8, _fx[fxi]._bufSize);

	return RD_OK;
}

/**
 * This function closes a sound effect which has been previously opened for
 * playing. Sound effects must be closed when they are finished with, otherwise
 * you will run out of sound effect buffers.
 * @param id the id of the sound to close
 */

int32 Sound::closeFx(int32 id) {
	int i;

	if (!_soundOn)
		return RD_OK;

	i = getFxIndex(id);

	if (i == MAXFX)
		return RDERR_FXNOTOPEN;

	stopFxHandle(i);
	return RD_OK;
}

/**
 * This function plays a sound effect. If the effect has already been opened
 * then 'data' should be NULL, and the sound effect will simply be obtained
 * from the id passed in. If the effect has not been opened, then the WAV data
 * should be passed in 'data'. The sound effect will be closed when it has
 * finished playing.
 * @param id the sound id
 * @param data either NULL or the WAV data
 * @param vol volume, 0 (minimum) to 16 (maximum)
 * @param pan panning, -16 (full left) to 16 (full right)
 * @param type either RDSE_FXSPOT or RDSE_FXLOOP
 * @warning Zero is not a valid id
 */

int32 Sound::playFx(int32 id, uint8 *data, uint8 vol, int8 pan, uint8 type) {
	if (!_soundOn)
		return RD_OK;

	byte volume = _fxMuted ? 0 : vol * _fxVol;
	int8 p = _panTable[pan + 16];
	int32 i, hr;

	if (data) {
		// All lead-ins and lead-outs I've heard are music, so we use
		// the music volume setting for them.

		if (type == RDSE_FXLEADIN || type == RDSE_FXLEADOUT) {
			id = (type == RDSE_FXLEADIN) ? -2 : -1;
			volume = _musicMuted ? 0 : _musicVolTable[_musicVol];
		}

		hr = openFx(id, data);
		if (hr != RD_OK)
			return hr;
	}

	i = getFxIndex(id);

	if (i == MAXFX) {
		if (data) {
			warning("playFx(%d, %d, %d, %d) - Not found", id, vol, pan, type);
			return RDERR_FXFUCKED;
		} else {
			warning("playFx(%d, %d, %d, %d) - Not open", id, vol, pan, type);
			return RDERR_FXNOTOPEN;
		}
	}

	if (type == RDSE_FXLOOP)
		_fx[i]._flags |= SoundMixer::FLAG_LOOP;
	else 
		_fx[i]._flags &= ~SoundMixer::FLAG_LOOP;

	_fx[i]._volume = vol;

	_vm->_mixer->playRaw(&_fx[i]._handle, _fx[i]._buf, _fx[i]._bufSize, _fx[i]._rate, _fx[i]._flags, -1, volume, p);

	return RD_OK;
}

void Sound::stopFxHandle(int i) {
	if (_fx[i]._id) {
		_vm->_mixer->stopHandle(_fx[i]._handle);
		free(_fx[i]._buf);
		_fx[i]._id = 0;
		_fx[i]._paused = false;
		_fx[i]._flags = 0;
		_fx[i]._bufSize = 0;
		_fx[i]._buf = NULL;
	}
}

/**
 * This function clears all of the sound effects which are currently open or
 * playing, irrespective of type.
 */

void Sound::clearAllFx(void) {
	if (!_soundOn)
		return;

	for (int i = 0; i < MAXFX; i++)
		if (_fx[i]._id && _fx[i]._id != -1 && _fx[i]._id != -2)
			stopFxHandle(i);
}

} // End of namespace Sword2
