/* Copyright (C) 1994-2005 Revolution Software Ltd
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
#include "sword2/driver/d_draw.h"
#include "sword2/driver/d_sound.h"

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

#define BUFFER_SIZE 4096

// ----------------------------------------------------------------------------
// Custom AudioStream class to handle Broken Sword II's audio compression.
// ----------------------------------------------------------------------------

#define GetCompressedShift(n)      ((n) >> 4)
#define GetCompressedSign(n)       (((n) >> 3) & 1)
#define GetCompressedAmplitude(n)  ((n) & 7)

class CLUInputStream : public AudioStream {
private:
	File *_file;
	bool _firstTime;
	uint32 _file_pos;
	uint32 _end_pos;
	int16 _outbuf[BUFFER_SIZE];
	byte _inbuf[BUFFER_SIZE];
	const int16 *_bufferEnd;
	const int16 *_pos;

	uint16 _prev;

	void refill();
	inline bool eosIntern() const;
public:
	CLUInputStream(File *file, int size);
	~CLUInputStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const	{ return eosIntern(); }
	bool isStereo() const	{ return false; }
	int getRate() const	{ return 22050; }
};

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

inline bool CLUInputStream::eosIntern() const {
	return _pos >= _bufferEnd;
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

class MusicInputStream : public AudioStream {
private:
	int _cd;
	uint32 _musicId;
	AudioStream *_decoder;
	int16 _buffer[BUFFER_SIZE];
	const int16 *_bufferEnd;
	const int16 *_pos;
	bool _remove;
	uint32 _numSamples;
	uint32 _samplesLeft;
	bool _looping;
	int32 _fading;
	int32 _fadeSamples;
	bool _paused;

	void refill();
	inline bool eosIntern() const;
public:
	MusicInputStream(int cd, uint32 musicId, bool looping);
	~MusicInputStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const	{ return eosIntern(); }
	bool isStereo() const	{ return _decoder->isStereo(); }
	int getRate() const	{ return _decoder->getRate(); }

	void fadeUp();
	void fadeDown();

	bool isReady()		{ return _decoder != NULL; }
	int32 isFading()	{ return _fading; }

	bool readyToRemove();
	int32 getTimeRemaining();
};

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

inline bool MusicInputStream::eosIntern() const {
	if (_looping)
		return false;
	return _remove || _pos >= _bufferEnd;
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

Sound::Sound(Sword2Engine *vm) {
	_vm = vm;
	_mutex = _vm->_system->createMutex();

	memset(_fx, 0, sizeof(_fx));

	_soundOn = true;

	_speechPaused = false;
	_speechMuted = false;

	_fxPaused = false;
	_fxMuted = false;

	_musicPaused = false;
	_musicMuted = false;

	_mixBuffer = NULL;
	_mixBufferLen = 0;

	for (int i = 0; i < MAXMUS; i++)
		_music[i] = NULL;

	_vm->_mixer->setupPremix(this, SoundMixer::kMusicAudioDataType);
}

Sound::~Sound() {
	int i;

	_vm->_mixer->setupPremix(0);

	for (i = 0; i < MAXMUS; i++)
		delete _music[i];

	free(_mixBuffer);

	for (i = 0; i < MAXFX; i++)
		stopFxHandle(i);

	stopSpeech();

	if (_mutex)
		_vm->_system->deleteMutex(_mutex);
}

// AudioStream API

int Sound::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);
	int i;

	if (!_soundOn || _musicPaused)
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


/**
 * This function creates the pan table.
 */

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
 * Stops the music dead in its tracks. Any music that is currently being
 * streamed is paused.
 */

void Sound::pauseMusic(void) {
	Common::StackLock lock(_mutex);

	if (!_soundOn)
		return;

	_musicPaused = true;
}

/**
 * Restarts the music from where it was stopped.
 */

void Sound::unpauseMusic(void) {
	Common::StackLock lock(_mutex);

	if (!_soundOn)
		return;

	_musicPaused = false;
}

/**
 * Fades out and stops the music.
 */

void Sound::stopMusic(void) {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < MAXMUS; i++)
		if (_music[i])
			_music[i]->fadeDown();
}

void Sound::waitForLeadOut(void) {
	int i = getFxIndex(-1);

	if (i == MAXFX)
		return;

	while (_fx[i]._handle.isActive()) {
		_vm->_graphics->updateDisplay();
		_vm->_system->delayMillis(30);
	}
}

/**
 * Streams music from a cluster file.
 * @param musicId the id of the music to stream
 * @param looping true if the music is to loop back to the start
 * @return RD_OK or an error code
 */
int32 Sound::streamCompMusic(uint32 musicId, bool looping) {
	Common::StackLock lock(_mutex);

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

	_music[primary] = new MusicInputStream(_vm->_resman->whichCd(), musicId, looping);

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
 * @return the speech's mute state, true if mute, false if not mute
 */

bool Sound::isSpeechMute(void) {
	return _speechMuted;
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

int32 Sound::stopSpeech(void) {
	if (!_soundOn)
		return RD_OK;
  
	if (_soundHandleSpeech.isActive()) {
		_vm->_mixer->stopHandle(_soundHandleSpeech);
		return RD_OK;
	}
	return RDERR_SPEECHNOTPLAYING;
}

/**
 * @return Either RDSE_SAMPLEPLAYING or RDSE_SAMPLEFINISHED
 */

int32 Sound::getSpeechStatus(void) {
	return _soundHandleSpeech.isActive() ? RDSE_SAMPLEPLAYING : RDSE_SAMPLEFINISHED;
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
 * does not play it. This is used for cutscene voice-overs, presumably to
 * avoid having to read from more than one file on the CD during playback.
 * @param speechid the text line id used to reference the speech
 * @param buf a pointer to the buffer that will be allocated for the sound
 */

uint32 Sound::preFetchCompSpeech(uint32 speechid, uint16 **buf) {
	File fp;
	uint32 numSamples;

	AudioStream *input = getAudioStream(&fp, "speech", _vm->_resman->whichCd(), speechid, &numSamples);

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
 * @param speechid the text line id used to reference the speech
 * @param vol volume, 0 (minimum) to 16 (maximum)
 * @param pan panning, -16 (full left) to 16 (full right)
 */

int32 Sound::playCompSpeech(uint32 speechid, uint8 vol, int8 pan) {
	if (_speechMuted)
		return RD_OK;

	if (getSpeechStatus() == RDERR_SPEECHPLAYING)
		return RDERR_SPEECHPLAYING;

	File *fp = new File;
	AudioStream *input = getAudioStream(fp, "speech", _vm->_resman->whichCd(), speechid, NULL);

	// Make the AudioStream object the sole owner of the file so that it
	// will die along with the AudioStream when the speech has finished.
	fp->decRef();

	if (!input)
		return RDERR_INVALIDID;

	// Modify the volume according to the master volume

	byte volume = _speechMuted ? 0 : vol * SoundMixer::kMaxChannelVolume / 16;
	int8 p = _panTable[pan + 16];

	// Start the speech playing
	_vm->_mixer->playInputStream(SoundMixer::kSpeechAudioDataType, &_soundHandleSpeech, input, -1, volume, p);
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
			byte volume = mute ? 0 : _fx[i]._volume * SoundMixer::kMaxChannelVolume / 16;

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
		_vm->_mixer->setChannelVolume(_fx[i]._handle, _fx[i]._volume * SoundMixer::kMaxChannelVolume / 16);
		_vm->_mixer->setChannelBalance(_fx[i]._handle, _panTable[pan + 16]);
	}

	return RD_OK;
}

int32 Sound::setFxIdVolume(int32 id, uint8 vol) {
	int32 i = getFxIndex(id);

	if (i == MAXFX)
		return RDERR_FXNOTOPEN;

	_fx[i]._volume = vol;

	if (!_fxMuted)
		_vm->_mixer->setChannelVolume(_fx[i]._handle, vol * SoundMixer::kMaxChannelVolume / 16);

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
 * This function closes a sound effect which has been previously opened for
 * playing. Sound effects must be closed when they are finished with, otherwise
 * you will run out of sound effect buffers.
 * @param id the id of the sound to close
 */

int32 Sound::stopFx(int32 id) {
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

int32 Sound::playFx(int32 id, uint32 len, byte *data, uint8 vol, int8 pan, uint8 type) {
	if (!_soundOn)
		return RD_OK;

	byte volume = _fxMuted ? 0 : vol * SoundMixer::kMaxChannelVolume / 16;
	SoundMixer::SoundType soundType = SoundMixer::kSFXAudioDataType;

	// All lead-ins and lead-outs I've heard are music, so we use
	// the music volume setting for them.

	if (type == RDSE_FXLEADIN || type == RDSE_FXLEADOUT) {
		id = (type == RDSE_FXLEADIN) ? -2 : -1;
		volume = _musicMuted ? 0 : SoundMixer::kMaxChannelVolume;
		soundType = SoundMixer::kMusicAudioDataType;
	}

	Common::MemoryReadStream stream(data, len);
	int rate, size;
	byte flags;

	if (!loadWAVFromStream(stream, size, rate, flags)) {
		warning("playFx: Not a valid WAV file");
		return RDERR_INVALIDWAV;
	}

	int32 fxi = getFxIndex(id);

	if (fxi == MAXFX) {
		// Find a free slot
		fxi = getFxIndex(0);

		if (fxi == MAXFX) {
			warning("openFx: Running out of sound slots");

			// There aren't any free sound handles available. This
			// usually shouldn't happen, but if it does we expire
			// the first sound effect that isn't currently playing.

			for (fxi = 0; fxi < MAXFX; fxi++)
				if (!_fx[fxi]._handle.isActive())
					break;

			// Still no dice? I give up!

			if (fxi == MAXFX) {
				warning("openFx: No free sound slots");
				return RDERR_NOFREEBUFFERS;
			}
		}

		_fx[fxi]._id = id;
	}

	if (_fx[fxi]._handle.isActive())
		return RDERR_FXALREADYOPEN;

	if (type == RDSE_FXLOOP)
		flags |= SoundMixer::FLAG_LOOP;
	else 
		flags &= ~SoundMixer::FLAG_LOOP;

	_fx[fxi]._volume = vol;

	int8 p = _panTable[pan + 16];

	_vm->_mixer->playRaw(&_fx[fxi]._handle, data + stream.pos(), size, rate, flags, -1, volume, p, 0, 0, soundType);

	return RD_OK;
}

void Sound::stopFxHandle(int i) {
	if (_fx[i]._id) {
		_vm->_mixer->stopHandle(_fx[i]._handle);
		_fx[i]._id = 0;
		_fx[i]._paused = false;
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
