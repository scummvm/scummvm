/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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

// One feature still missing is the original's DipMusic() function which, as
// far as I can understand, softened the music volume when someone was
// speaking, but only (?) if the music was playing loudly at the time.
//
// All things considered, I think this is more bother than it's worth.

#include "common/stdafx.h"
#include "common/file.h"
#include "common/system.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/flac.h"
#include "sound/rate.h"
#include "sound/wave.h"
#include "sword2/sword2.h"
#include "sword2/resman.h"
#include "sword2/sound.h"
#include "sword2/driver/d_draw.h"

namespace Sword2 {

static AudioStream *makeCLUStream(File *fp, int size);

static File fpMus;

static AudioStream *getAudioStream(File *fp, const char *base, int cd, uint32 id, uint32 *numSamples) {
	struct {
		const char *ext;
		int mode;
	} file_types[] = {
#ifdef USE_MAD
		{ "cl3", kMP3Mode },
#endif
#ifdef USE_VORBIS
		{ "clg", kVorbisMode },
#endif
#ifdef USE_FLAC
		{ "clf", kFlacMode },
#endif
		{ "clu", kCLUMode }
	};

	int soundMode = -1;
	char filename[20];

	for (int i = 0; i < ARRAYSIZE(file_types); i++) {
		File f;

		sprintf(filename, "%s%d.%s", base, cd, file_types[i].ext);
		if (f.open(filename)) {
			soundMode = file_types[i].mode;
			break;
		}

		sprintf(filename, "%s.%s", base, file_types[i].ext);
		if (f.open(filename)) {
			soundMode = file_types[i].mode;
			break;
		}
	}

	if (soundMode == 0)
		return NULL;

	// The assumption here is that a sound file is closed when the sound
	// finishes, and we never play sounds from two different files at the
	// same time. Thus, if the file is already open it's the correct file,
	// and the loop above was just needed to figure out the compression.
	//
	// This is to avoid having two file handles open to the same file at
	// the same time. There was some speculation that some of our target
	// systems may have trouble with that.

	if (!fp->isOpen())
		fp->open(filename);

	if (!fp->isOpen())
		return NULL;

	fp->seek((id + 1) * ((soundMode == kCLUMode) ? 8 : 12), SEEK_SET);

	uint32 pos = fp->readUint32LE();
	uint32 len = fp->readUint32LE();
	uint32 enc_len;

	if (numSamples)
		*numSamples = len;

	if (soundMode != kCLUMode)
		enc_len = fp->readUint32LE();
	else
		enc_len = len + 1;

	if (!pos || !len) {
		fp->close();
		return NULL;
	}

	fp->seek(pos, SEEK_SET);

	switch (soundMode) {
	case kCLUMode:
		return makeCLUStream(fp, enc_len);
#ifdef USE_MAD
	case kMP3Mode:
		return makeMP3Stream(fp, enc_len);
#endif
#ifdef USE_VORBIS
	case kVorbisMode:
		return makeVorbisStream(fp, enc_len);
#endif
#ifdef USE_FLAC
	case kFlacMode:
		return makeFlacStream(fp, enc_len);
#endif
	default:
		return NULL;
	}
}

// ----------------------------------------------------------------------------
// Custom AudioStream class to handle Broken Sword II's audio compression.
// ----------------------------------------------------------------------------

#define GetCompressedShift(n)      ((n) >> 4)
#define GetCompressedSign(n)       (((n) >> 3) & 1)
#define GetCompressedAmplitude(n)  ((n) & 7)

CLUInputStream::CLUInputStream(File *file, int size)
	: _file(file), _firstTime(true), _bufferEnd(_outbuf + BUFFER_SIZE) {

	_file->incRef();

	// Determine the end position.
	_file_pos = _file->pos();
	_end_pos = _file_pos + size;

	// Read in initial data
	refill();
}

CLUInputStream::~CLUInputStream() {
	_file->decRef();
}

int CLUInputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples - samples, (int) (_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			refill();
		}
	}
	return samples;
}

void CLUInputStream::refill() {
	byte *in = _inbuf;
	int16 *out = _outbuf;

	_file->seek(_file_pos, SEEK_SET);

	uint len_left = _file->read(in, MIN((uint32) BUFFER_SIZE, _end_pos - _file->pos()));

	_file_pos = _file->pos();

	while (len_left > 0) {
		uint16 sample;

		if (_firstTime) {
			_firstTime = false;
			_prev = READ_LE_UINT16(in);
			sample = _prev;
			len_left -= 2;
			in += 2;
		} else {
			uint16 delta = GetCompressedAmplitude(*in) << GetCompressedShift(*in);
			if (GetCompressedSign(*in))
				sample = _prev - delta;
			else
				sample = _prev + delta;

			_prev = sample;
			len_left--;
			in++;
		}

		*out++ = sample;
	}

	_pos = _outbuf;
	_bufferEnd = out;
}

AudioStream *makeCLUStream(File *file, int size) {
	return new CLUInputStream(file, size);
}

// ----------------------------------------------------------------------------
// Another custom AudioStream class, to wrap around the various AudioStream
// classes used for music decompression, and to add looping, fading, etc.
// ----------------------------------------------------------------------------

// The length of a fade-in/out, in milliseconds.
#define FADE_LENGTH 3000

MusicInputStream::MusicInputStream(int cd, uint32 musicId, bool looping)
	: _cd(cd), _musicId(musicId), _bufferEnd(_buffer + BUFFER_SIZE),
	  _remove(false), _looping(looping), _fading(0) {
	_decoder = getAudioStream(&fpMus, "music", _cd, _musicId, &_numSamples);
	if (_decoder) {
		_samplesLeft = _numSamples;
		_fadeSamples = (getRate() * FADE_LENGTH) / 1000;
		fadeUp();

		// Read in initial data
		refill();
	}
}

MusicInputStream::~MusicInputStream() {
	delete _decoder;
}

int MusicInputStream::readBuffer(int16 *buffer, const int numSamples) {
	if (!_decoder)
		return 0;

	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples - samples, (int) (_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			refill();
		}
	}
	return samples;
}

void MusicInputStream::refill() {
	int16 *buf = _buffer;
	uint32 numSamples = 0;
	uint32 len_left;
	bool endFade = false;

	len_left = BUFFER_SIZE;

	if (_fading > 0 && (uint32) _fading < len_left)
		len_left = _fading;

	if (_samplesLeft < len_left)
		len_left = _samplesLeft;

	if (!_looping) {
		// None-looping music is faded out at the end. If this fade
		// out would have started somewhere within the len_left samples
		// to read, we only read up to that point. This way, we can
		// treat this fade as any other.

		if (!_fading) {
			uint32 currentlyAt = _numSamples - _samplesLeft;
			uint32 fadeOutAt = _numSamples - _fadeSamples;
			uint32 readTo = currentlyAt + len_left;

			if (fadeOutAt == currentlyAt)
				fadeDown();
			else if (fadeOutAt > currentlyAt && fadeOutAt <= readTo) {
				len_left = fadeOutAt - currentlyAt;
				endFade = true;
			}
		}
	}

	int desired = len_left - numSamples;
	int len = _decoder->readBuffer(buf, desired);

	// Shouldn't happen, but if it does it could cause an infinite loop.
	// Of course there were bugs that caused it to happen several times
	// during development. :-)

	if (len < desired) {
		warning("Expected %d samples, but got none", desired);
		if (!len)
			return;
	}

	buf += len;
	numSamples += len;
	len_left -= len;
	_samplesLeft -= len;

	int16 *ptr;

	if (_fading > 0) {
		// Fade down
		for (ptr = _buffer; ptr < buf; ptr++) {
			if (_fading > 0) {
				_fading--;
				*ptr = (*ptr * _fading) / _fadeSamples;
			}
			if (_fading == 0) {
				_looping = false;
				_remove = true;
				*ptr = 0;
			}
		}
	} else if (_fading < 0) {
		// Fade up
		for (ptr = _buffer; ptr < buf; ptr++) {
			_fading--;
			*ptr = -(*ptr * _fading) / _fadeSamples;
			if (_fading <= -_fadeSamples) {
				_fading = 0;
				break;
			}
		}
	}

	if (endFade)
		fadeDown();

	if (!_samplesLeft) {
		if (_looping) {
			delete _decoder;
			_decoder = getAudioStream(&fpMus, "music", _cd, _musicId, &_numSamples);
			_samplesLeft = _numSamples;
		} else
			_remove = true;
	}

	_pos = _buffer;
	_bufferEnd = buf;
}

void MusicInputStream::fadeUp() {
	if (_fading > 0)
		_fading = -_fading;
	else if (_fading == 0)
		_fading = -1;
}

void MusicInputStream::fadeDown() {
	if (_fading < 0)
		_fading = -_fading;
	else if (_fading == 0)
		_fading = _fadeSamples;
}

bool MusicInputStream::readyToRemove() {
	return _remove;
}

int32 MusicInputStream::getTimeRemaining() {
	// This is far from exact, but it doesn't have to be.
	return (_samplesLeft + BUFFER_SIZE) / getRate();
}

// ----------------------------------------------------------------------------
// Main sound class
// ----------------------------------------------------------------------------

// AudioStream API

int Sound::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);
	int i;

	if (_musicPaused)
		return 0;

	for (i = 0; i < MAXMUS; i++) {
		if (_music[i] && _music[i]->readyToRemove()) {
			delete _music[i];
			_music[i] = NULL;
		}
	}

	memset(buffer, 0, 2 * numSamples);

	if (!_mixBuffer || numSamples > _mixBufferLen) {
		if (_mixBuffer)
			_mixBuffer = (int16 *) realloc(_mixBuffer, 2 * numSamples);
		else
			_mixBuffer = (int16 *) malloc(2 * numSamples);

		_mixBufferLen = numSamples;
	}

	if (!_mixBuffer)
		return 0;

	for (i = 0; i < MAXMUS; i++) {
		if (!_music[i])
			continue;

		int len = _music[i]->readBuffer(_mixBuffer, numSamples);

		if (!_musicMuted) {
			for (int j = 0; j < len; j++) {
				clampedAdd(buffer[j], _mixBuffer[j]);
			}
		}
	}

	if (!_music[0] && !_music[1] && fpMus.isOpen())
		fpMus.close();

	return numSamples;
}

bool Sound::isStereo() const { return false; }
bool Sound::endOfData() const { return !fpMus.isOpen(); }
int Sound::getRate() const { return 22050; }

// ----------------------------------------------------------------------------
// MUSIC
// ----------------------------------------------------------------------------

/**
 * Stops the music dead in its tracks. Any music that is currently being
 * streamed is paused.
 */

void Sound::pauseMusic() {
	Common::StackLock lock(_mutex);

	_musicPaused = true;
}

/**
 * Restarts the music from where it was stopped.
 */

void Sound::unpauseMusic() {
	Common::StackLock lock(_mutex);

	_musicPaused = false;
}

/**
 * Fades out and stops the music.
 */

void Sound::stopMusic() {
	Common::StackLock lock(_mutex);

	_loopingMusicId = 0;

	for (int i = 0; i < MAXMUS; i++)
		if (_music[i])
			_music[i]->fadeDown();
}

/**
 * Streams music from a cluster file.
 * @param musicId the id of the music to stream
 * @param looping true if the music is to loop back to the start
 * @return RD_OK or an error code
 */
int32 Sound::streamCompMusic(uint32 musicId, bool loop) {
	Common::StackLock lock(_mutex);

	if (loop)
		_loopingMusicId = musicId;
	else
		_loopingMusicId = 0;

	int primary = -1;
	int secondary = -1;

	// If both music streams are active, one of them will have to go.

	if (_music[0] && _music[1]) {
		int32 fade0 = _music[0]->isFading();
		int32 fade1 = _music[1]->isFading();

		if (!fade0 && !fade1) {
			// Neither is fading. This shouldn't happen, so just
			// pick one and be done with it.
			primary = 0;
		} else if (fade0 && !fade1) {
			// Stream 0 is fading, so pick that one.
			primary = 0;
		} else if (!fade0 && fade1) {
			// Stream 1 is fading, so pick that one.
			primary = 1;
		} else {
			// Both streams are fading. Pick the one that is
			// closest to silent.
			if (ABS(fade0) < ABS(fade1))
				primary = 0;
			else
				primary = 1;
		}

		delete _music[primary];
		_music[primary] = NULL;
	}

	// Pick the available music stream. If no music is playing it doesn't
	// matter which we use.

	if (_music[0] || _music[1]) {
		if (_music[0]) {
			primary = 1;
			secondary = 0;
		} else {
			primary = 0;
			secondary = 1;
		}
	} else
		primary = 0;

	// Don't start streaming if the volume is off.
	if (isMusicMute())
		return RD_OK;

	if (secondary != -1)
		_music[secondary]->fadeDown();

	_music[primary] = new MusicInputStream(_vm->_resman->whichCd(), musicId, loop);

	if (!_music[primary]->isReady()) {
		delete _music[primary];
		_music[primary] = NULL;
		return RDERR_INVALIDFILENAME;
	}

	return RD_OK;
}

/**
 * @return the time left for the current music, in seconds.
 */

int32 Sound::musicTimeRemaining(void) {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < MAXMUS; i++) {
		if (_music[i] && _music[i]->isFading() <= 0)
			return _music[i]->getTimeRemaining();
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

	if (_soundHandleSpeech.isActive()) {
		uint volume = mute ? 0 : SoundMixer::kMaxChannelVolume;

		_vm->_mixer->setChannelVolume(_soundHandleSpeech, volume);
	}
}

/**
 * Stops the speech dead in its tracks.
 */

void Sound::pauseSpeech(void) {
	_speechPaused = true;
	_vm->_mixer->pauseHandle(_soundHandleSpeech, true);
}

/**
 * Restarts the speech from where it was stopped.
 */

void Sound::unpauseSpeech(void) {
	_speechPaused = false;
	_vm->_mixer->pauseHandle(_soundHandleSpeech, false);
}

/**
 * Stops the speech from playing.
 */

int32 Sound::stopSpeech() {
	if (_soundHandleSpeech.isActive()) {
		_vm->_mixer->stopHandle(_soundHandleSpeech);
		return RD_OK;
	}

	return RDERR_SPEECHNOTPLAYING;
}

/**
 * @return Either RDSE_SAMPLEPLAYING or RDSE_SAMPLEFINISHED
 */

int32 Sound::getSpeechStatus() {
	return _soundHandleSpeech.isActive() ? RDSE_SAMPLEPLAYING : RDSE_SAMPLEFINISHED;
}

/**
 * Returns either RDSE_QUIET or RDSE_SPEAKING
 */

int32 Sound::amISpeaking() {
	if (!_speechMuted && !_speechPaused && _soundHandleSpeech.isActive())
		return RDSE_SPEAKING;

	return RDSE_QUIET;
}

/**
 * This function loads and decompresses a list of speech from a cluster, but
 * does not play it. This is used for cutscene voice-overs, presumably to
 * avoid having to read from more than one file on the CD during playback.
 * @param speechId the text line id used to reference the speech
 * @param buf a pointer to the buffer that will be allocated for the sound
 */

uint32 Sound::preFetchCompSpeech(uint32 speechId, uint16 **buf) {
	File fp;
	uint32 numSamples;

	AudioStream *input = getAudioStream(&fp, "speech", _vm->_resman->whichCd(), speechId, &numSamples);

	*buf = NULL;

	// Decompress data into speech buffer.

	uint32 bufferSize = 2 * numSamples;

	*buf = (uint16 *) malloc(bufferSize);
	if (!*buf) {
		delete input;
		fp.close();
		return 0;
	}

	uint32 readSamples = input->readBuffer((int16 *) *buf, numSamples);

	fp.close();
	delete input;

	return 2 * readSamples;
}

/**
 * This function loads, decompresses and plays a line of speech. An error
 * occurs if speech is already playing.
 * @param speechId the text line id used to reference the speech
 * @param vol volume, 0 (minimum) to 16 (maximum)
 * @param pan panning, -16 (full left) to 16 (full right)
 */

int32 Sound::playCompSpeech(uint32 speechId, uint8 vol, int8 pan) {
	if (_speechMuted)
		return RD_OK;

	if (getSpeechStatus() == RDERR_SPEECHPLAYING)
		return RDERR_SPEECHPLAYING;

	File *fp = new File;
	AudioStream *input = getAudioStream(fp, "speech", _vm->_resman->whichCd(), speechId, NULL);

	// Make the AudioStream object the sole owner of the file so that it
	// will die along with the AudioStream when the speech has finished.
	fp->decRef();

	if (!input)
		return RDERR_INVALIDID;

	// Modify the volume according to the master volume

	byte volume = _speechMuted ? 0 : vol * SoundMixer::kMaxChannelVolume / 16;
	int8 p = (pan * 127) / 16;

	if (isReverseStereo())
		p = -p;

	// Start the speech playing
	_vm->_mixer->playInputStream(SoundMixer::kSpeechAudioDataType, &_soundHandleSpeech, input, -1, volume, p);
	return RD_OK;
}

// ----------------------------------------------------------------------------
// SOUND EFFECTS
// ----------------------------------------------------------------------------

/**
 * Mutes/Unmutes the sound effects.
 * @param mute If mute is false, restore the volume to the last set master
 * level. Otherwise the sound effects are muted (volume 0).
 */

void Sound::muteFx(bool mute) {
	_fxMuted = mute;

	// Now update the volume of any fxs playing
	for (int i = 0; i < FXQ_LENGTH; i++) {
		if (_fxQueue[i].resource) {
			_vm->_mixer->setChannelVolume(_fxQueue[i].handle, mute ? 0 : _fxQueue[i].volume);
		}
	}
}

/**
 * Sets the volume and pan of the sample which is currently playing
 * @param id the id of the sample
 * @param vol volume
 * @param pan panning
 */

int32 Sound::setFxIdVolumePan(int32 i, int vol, int pan) {
	if (!_fxQueue[i].resource)
		return RDERR_FXNOTOPEN;

	if (vol > 16)
		vol = 16;

	_fxQueue[i].volume = (vol * SoundMixer::kMaxChannelVolume) / 16;

	if (pan != -1)
		_fxQueue[i].pan = (pan * 127) / 16;

	if (!_fxMuted && _fxQueue[i].handle.isActive()) {
		_vm->_mixer->setChannelVolume(_fxQueue[i].handle, _fxQueue[i].volume);
		if (pan != -1)
			_vm->_mixer->setChannelBalance(_fxQueue[i].handle, _fxQueue[i].pan);
	}

	return RD_OK;
}

void Sound::pauseFx() {
	if (_fxPaused)
		return;

	for (int i = 0; i < FXQ_LENGTH; i++) {
		if (_fxQueue[i].resource)
			_vm->_mixer->pauseHandle(_fxQueue[i].handle, true);
	}

	_fxPaused = true;
}

void Sound::unpauseFx() {
	if (!_fxPaused)
		return;

	for (int i = 0; i < FXQ_LENGTH; i++)
		if (_fxQueue[i].resource)
			_vm->_mixer->pauseHandle(_fxQueue[i].handle, false);

	_fxPaused = false;
}

} // End of namespace Sword2
