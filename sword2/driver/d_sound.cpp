/* Copyright (C) 1994-2003 Revolution Software Ltd
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

#include "stdafx.h"
#include "driver96.h"
#include "d_sound.h"
#include "../sword2.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/rate.h"

// Fade-out takes half a second. This may need some tuning.
#define FADE_SAMPLES 11025

static File fpMus;

// Decompression macros
#define GetCompressedShift(n)      ((n) >> 4)
#define GetCompressedSign(n)       (((n) >> 3) & 1)
#define GetCompressedAmplitude(n)  ((n) & 7)

int32 panTable[33] = {
	-127, -119, -111, -103,  -95,  -87,  -79,  -71,
	 -63,  -55,  -47,  -39,  -31,  -23,  -15,   -7,
	   0,
	   7,   15,   23,   31,   39,   47,   55,   63,
          71,   79,   87,   95,  103,  111,  119,  127
};

int32 musicVolTable[17] = {
	  0,  15,  31,  47,  63,  79,  95, 111, 127,
	143, 159, 175, 191, 207, 223, 239, 255
};

int16 MusicHandle::read() {
	uint8 in;
	uint16 delta;
	int16 out;

	if (!_streaming)
		return 0;

	if (_firstTime) {
		_lastSample = fpMus.readUint16LE();
		_filePos += 2;
		_firstTime = false;
		return _lastSample;
	}

	// Assume the file handle has been correctly positioned already.

	in = fpMus.readByte();
	delta = GetCompressedAmplitude(in) << GetCompressedShift(in);

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
		}
	} else {
		// Fade out at the end of the music. Is this really desirable
		// behaviour?

		if (_fileEnd - _filePos <= FADE_SAMPLES)
			_fading = _fileEnd - _filePos;
	}

	if (_fading > 0) {
		if (--_fading == 0) {
			_streaming = false;
			_looping = false;
		}
		out = (out * _fading) / FADE_SAMPLES;
	}

	return out;
}

bool MusicHandle::eos() const {
	if (!_streaming || _filePos >= _fileEnd)
		return true;
	return false;
}

static void premix_proc(void *param, int16 *data, uint len) {
	((Sword2Sound *) param)->FxServer(data, len);
}

Sword2Sound::Sword2Sound(SoundMixer *mixer) {
	_mutex = g_system->create_mutex();

	soundOn = 0;
	speechStatus = 0;
	fxPaused = 0;
	speechPaused = 0;
	speechVol = 14;
	fxVol = 14;
	speechMuted = 0;
	fxMuted = 0;
	musicVol = 16;

	musicMuted = 0;
	_mixer = mixer;

	memset(fx, 0, sizeof(fx));

	soundHandleSpeech = 0;
	soundOn = 1;

	_converter = makeRateConverter(music[0].getRate(), _mixer->getOutputRate(), music[0].isStereo(), false);

	_mixer->setupPremix(premix_proc, this);
}

Sword2Sound::~Sword2Sound() {
	if (_mutex)
		g_system->delete_mutex(_mutex);
}

// --------------------------------------------------------------------------
// This function reverse the pan table, thus reversing the stereo.
// --------------------------------------------------------------------------

// FIXME: We could probably use the FLAG_REVERSE_STEREO mixer flag here.

/**
 * This function reverses the pan table, thus reversing the stereo.
 */

int32 Sword2Sound::ReverseStereo(void) {
	int i, j;

	for (i = 0; i < 16; i++) {
		j = panTable[i];
		panTable[i] = panTable[32 - i];
		panTable[32 - i] = j;
	}

	return RD_OK;
}

// Save/Restore information about current music so that we can restore it
// after the credits.

void Sword2Sound::saveMusicState() {
	StackLock lock(_mutex);

	int saveStream;

	if (music[0]._streaming && !music[0]._fading) {
		saveStream = 0;
	} else if (music[1]._streaming && !music[0]._fading) {
		saveStream = 1;
	} else {
		music[2]._streaming = false;
		return;
	}

	music[2]._streaming = true;
	music[2]._fading = 0;
	music[2]._looping = music[saveStream]._looping;
	music[2]._fileStart = music[saveStream]._fileStart;
	music[2]._filePos = music[saveStream]._filePos;
	music[2]._fileEnd = music[saveStream]._fileEnd;
	music[2]._lastSample = music[saveStream]._lastSample;
}

void Sword2Sound::restoreMusicState() {
	StackLock lock(_mutex);

	int restoreStream;

	if (!music[2]._streaming)
		return;

	// Fade out any music that happens to be playing

	for (int i = 0; i < MAXMUS; i++) {
		if (music[i]._streaming && !music[i]._fading)
			music[i]._fading = FADE_SAMPLES;
	}

	if (!music[0]._streaming && !music[0]._fading) {
		restoreStream = 0;
	} else {
		restoreStream = 1;
	}

	music[restoreStream]._streaming = true;
	music[restoreStream]._fading = 0;
	music[restoreStream]._looping = music[2]._looping;
	music[restoreStream]._fileStart = music[2]._fileStart;
	music[restoreStream]._filePos = music[2]._filePos;
	music[restoreStream]._fileEnd = music[2]._fileEnd;
	music[restoreStream]._lastSample = music[2]._lastSample;
}

void Sword2Sound::playLeadOut(uint8 *leadOut) {
	int i;

	if (!leadOut)
		return;

	PlayFx(0, leadOut, 0, 0, RDSE_FXLEADOUT);

	i = GetFxIndex(-1);

	if (i == MAXFX) {
		warning("playLeadOut: Can't find lead-out sound handle");
		return;
	}

	while (fx[i]._handle) {
		ServiceWindows();
		g_system->delay_msecs(30);
	}

	CloseFx(-2);
}

// --------------------------------------------------------------------------
// This function returns the index of the sound effect with the ID passed in.
// --------------------------------------------------------------------------

int32 Sword2Sound::GetFxIndex(int32 id) {
	for (int i = 0; i < MAXFX; i++) {
		if (fx[i]._id == id)
			return i;
	}

	return MAXFX;
}

int32 Sword2Sound::IsFxOpen(int32 id) {
	// FIXME: This seems backwards to me, but changing it breaks sound.
	return GetFxIndex(id) == MAXFX;
}

// --------------------------------------------------------------------------
// This function checks the status of all current sound effects, and clears
// out the ones which are no longer required in a buffer.  It is called by
// a separate thread.
// --------------------------------------------------------------------------

void Sword2Sound::FxServer(int16 *data, uint len) {
	StackLock lock(_mutex);

	if (!soundOn)
		return;

	UpdateCompSampleStreaming(data, len);

	if (!music[0]._streaming && !music[1]._streaming && fpMus.isOpen())
		fpMus.close();

	// FIXME: Doing this sort of things from a separate thread seems like
	// just asking for trouble. But removing it outright causes regressions
	// which need to be investigated.
	//
	// I've fixed one such regression, and as far as I can tell it's
	// working now.

#if 0
	int i;

	if (fxPaused) {
		for (i = 0; i < MAXFX; i++) {
			if ((fx[i]._id == -1) || (fx[i]._id == -2)) {
				if (!fx[i]._handle) {
					fx[i]._id = 0;
					if (fx[i]._buf != NULL) {
						free(fx[i]._buf);
						fx[i]._buf = NULL;
					}
					fx[i]._bufSize = 0;
					fx[i]._flags = 0;
				}
			}
		}
		return;
	}

	for (i = 0; i < MAXFX; i++) {
		if (fx[i]._id) {
			if (!fx[i]._handle) {
				fx[i]._id = 0;
				if (fx[i]._buf != NULL) {
					free(fx[i]._buf);
					fx[i]._buf = NULL;
				}
				fx[i]._bufSize = 0;
				fx[i]._flags = 0;
			}
		}
	}
#endif
}

/**
 * Returns either RDSE_QUIET or RDSE_SPEAKING
 */

int32 Sword2Sound::AmISpeaking() {
	if (!speechMuted && !speechPaused && soundHandleSpeech != 0)
		return RDSE_SPEAKING;

	return RDSE_QUIET;
}

/**
 * This function loads and decompresses a list of speech from a cluster, but
 * does not play it. This is primarily used by PlayCompSpeech(), but also to
 * store the voice-overs for the animated cutscenes until they are played.
 * @param filename the file name of the speech cluster file
 * @param speechid the text line id used to reference the speech
 * @param buf a pointer to the buffer that will be allocated for the sound
 */ 

uint32 Sword2Sound::PreFetchCompSpeech(const char *filename, uint32 speechid, uint16 **buf) {
	uint32 i;
	uint8 *data8;
	uint32 speechIndex[2];
	File fp;
	uint32 bufferSize;

	*buf = NULL;

	// Open the speech cluster and find the data offset & size
	fp.open(filename);
	if (!fp.isOpen())
		return 0;

	fp.seek((speechid + 1) * 8, SEEK_SET);

	if (fp.read(speechIndex, sizeof(uint32) * 2) != (sizeof(uint32) * 2)) {
		fp.close();
		return 0;
	}

#ifdef SCUMM_BIG_ENDIAN
	speechIndex[0] = SWAP_BYTES_32(speechIndex[0]);
	speechIndex[1] = SWAP_BYTES_32(speechIndex[1]);
#endif

	if (!speechIndex[0] || !speechIndex[1]) {
		fp.close();
		return 0;
	}

	// Create a temporary buffer for compressed speech
	if ((data8 = (uint8 *) malloc(speechIndex[1])) == NULL) {
		fp.close();
		return 0;
	}

	fp.seek(speechIndex[0], SEEK_SET);

	if (fp.read(data8, speechIndex[1]) != speechIndex[1]) {
		fp.close();
		free(data8);
		return 0;
	}

	fp.close();

	// Decompress data into speech buffer.

	bufferSize = (speechIndex[1] - 1) * 2;

	*buf = (uint16 *) malloc(bufferSize);
	if (!*buf) {
		free(data8);
		return 0;
	}

	uint16 *data16 = *buf;

	// Starting Value
	data16[0] = READ_LE_UINT16(data8);

	for (i = 1; i < speechIndex[1] - 1; i++) {
		if (GetCompressedSign(data8[i + 1]))
			data16[i] = data16[i - 1] - (GetCompressedAmplitude(data8[i + 1]) << GetCompressedShift(data8[i + 1]));
		else
			data16[i] = data16[i - 1] + (GetCompressedAmplitude(data8[i + 1]) << GetCompressedShift(data8[i + 1]));
	}

	free(data8);

#ifndef SCUMM_BIG_ENDIAN
	// Until the mixer supports LE samples natively, we need to convert
	// our LE ones to BE
	for (uint j = 0; j < bufferSize / 2; j++)
		data16[j] = SWAP_BYTES_16(data16[j]);
#endif

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

int32 Sword2Sound::PlayCompSpeech(const char *filename, uint32 speechid, uint8 vol, int8 pan) {
	uint16 *data16;
	uint32 bufferSize;
	
	if (!speechMuted) {
		if (GetSpeechStatus() == RDERR_SPEECHPLAYING)
			return RDERR_SPEECHPLAYING;

		bufferSize = PreFetchCompSpeech(filename, speechid, &data16);

		// We don't know exactly what went wrong here.
		if (bufferSize == 0)
			return RDERR_OUTOFMEMORY;

		// Modify the volume according to the master volume

		byte volume = speechMuted ? 0 : vol * speechVol;
		int8 p = panTable[pan + 16];

		// Start the speech playing

		speechPaused = 1;
			
		uint32 flags = SoundMixer::FLAG_16BITS | SoundMixer::FLAG_AUTOFREE;

		_mixer->playRaw(&soundHandleSpeech, data16, bufferSize, 22050, flags, -1, volume, p);

		speechStatus = 1;
	}

	// DipMusic();

	return RD_OK;
}

/**
 * Stops the speech from playing.
 */

int32 Sword2Sound::StopSpeechSword2(void) {
	if (!soundOn)
		return RD_OK;
  
	if (speechStatus) {
		g_engine->_mixer->stopHandle(soundHandleSpeech);
		speechStatus = 0;
		return RD_OK;
	}
	return RDERR_SPEECHNOTPLAYING;
}

/**
 * @return Either RDSE_SAMPLEPLAYING or RDSE_SAMPLEFINISHED
 */

int32 Sword2Sound::GetSpeechStatus(void) {
	if (!soundOn || !speechStatus)
		return RDSE_SAMPLEFINISHED;

	if (speechPaused)
		return RDSE_SAMPLEPLAYING;

	if (!soundHandleSpeech) {
		speechStatus = 0;
		return RDSE_SAMPLEFINISHED;
	}
	return RDSE_SAMPLEPLAYING;
}

/**
 * Set the volume of any future as well as playing speech samples.
 * @param volume volume, from 0 (silent) to 14 (max)
 */

void Sword2Sound::SetSpeechVolume(uint8 volume) {
	speechVol = volume;
	if (soundHandleSpeech != 0 && !speechMuted && GetSpeechStatus() == RDSE_SAMPLEPLAYING) {
		g_engine->_mixer->setChannelVolume(soundHandleSpeech, 16 * speechVol);
	}
}

/**
 * @return the volume setting for speech
 */

uint8 Sword2Sound::GetSpeechVolume() {
	return speechVol;
}

/**
 * Mutes/Unmutes the speech.
 * @param mute If mute is 0, restore the volume to the last set master level.
 * Otherwise the speech is muted (volume 0).
 */

void Sword2Sound::MuteSpeech(uint8 mute) {
	speechMuted = mute;

	if (GetSpeechStatus() == RDSE_SAMPLEPLAYING) {
		byte volume = mute ? 0 : 16 * speechVol;

		g_engine->_mixer->setChannelVolume(soundHandleSpeech, volume);
	}
}

/**
 * @return the speech's mute state, 1 if mute, 0 if not mute
 */

uint8 Sword2Sound::IsSpeechMute(void) {
	return speechMuted;
}

/**
 * Stops the speech dead in its tracks.
 */

int32 Sword2Sound::PauseSpeech(void) {
	if (GetSpeechStatus() == RDSE_SAMPLEPLAYING) {
		speechPaused = 1;
		g_engine->_mixer->pauseHandle(soundHandleSpeech, true);
	}
	return RD_OK;
}

/**
 * Restarts the speech from where it was stopped.
 */

int32 Sword2Sound::UnpauseSpeech(void) {
	if (speechPaused) {
		speechPaused = 0;
		g_engine->_mixer->pauseHandle(soundHandleSpeech, false);
	}
	return RD_OK;
}

/**
 * This function opens a sound effect ready for playing. A unique id should be
 * passed in so that each effect can be referenced individually.
 * @param id the unique sound id
 * @data the WAV data
 * @warning Zero is not a valid id
 */

int32 Sword2Sound::OpenFx(int32 id, uint8 *data) {
 	int32 i, fxi;
	uint32 *data32 = NULL;
	_wavHeader *wav;

	wav = (_wavHeader *) data;

	if (soundOn) {
		// Check for a valid id.
		if (id == 0)
			return RDERR_INVALIDID;

		// Check that the fx is not already open
		for (i = 0; i < MAXFX; i++) {
			if (fx[i]._id == id)
				return RDERR_FXALREADYOPEN;
		}

		// Now choose a free slot for the fx
		for (fxi = 0; fxi < MAXFX; fxi++) {
			if (fx[fxi]._id == 0)
				break;
		}

		if (fxi == MAXFX) {
			// Expire the first sound effect that isn't currently
			// playing.

			// FIXME. This may need a bit of work. I still haven't
			// grasped all the intricacies of the sound effects
			// handling.
			//
			// Anyway, it'd be nicer - in theory - to expire the
			// least recently used slot.
			//
			// This used to be done by the "garbage collector" in
			// FxServer().

			for (fxi = 0; fxi < MAXFX; fxi++) {
				if (!fx[fxi]._handle)
					break;
			}

			// Still no dice? I give up!

			if (fxi == MAXFX) {
				warning("OpenFx: No free sound slots");
				return RDERR_NOFREEBUFFERS;
			}
		}

		// Set the sample size - search for the size of the data.
		i = 0;
		while (i < 100) {
			if (*data == 'd') {
				data32 = (uint32 *) data;
				if (READ_UINT32(data32) == MKID('data'))
					break;
			}
			i++;
			data++;
		}

		if (!data32)
			return RDERR_INVALIDWAV;

		fx[fxi]._bufSize = READ_LE_UINT32(data32 + 1);

		// Fill the speech buffer with data
		if (fx[fxi]._buf != NULL)
			free(fx[fxi]._buf);
		fx[fxi]._buf = (uint16 *) malloc(fx[fxi]._bufSize);
		memcpy(fx[fxi]._buf, (uint8 *) (data32 + 2), fx[fxi]._bufSize);
		fx[fxi]._flags = SoundMixer::FLAG_16BITS;
		if (FROM_LE_16(wav->channels) == 2)
			fx[fxi]._flags |= SoundMixer::FLAG_STEREO;

		fx[fxi]._rate = FROM_LE_16(wav->samplesPerSec);

		// Until the mixer supports LE samples natively, we need to
		// convert our LE ones to BE
		for (int32 j = 0; j < fx[fxi]._bufSize / 2; j++)
			fx[fxi]._buf[j] = SWAP_BYTES_16(fx[fxi]._buf[j]);

		fx[fxi]._id = id;
	}
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

int32 Sword2Sound::PlayFx(int32 id, uint8 *data, uint8 vol, int8 pan, uint8 type) {
	int32 i, loop;
	uint32 hr;

	if (type == RDSE_FXLOOP)
		loop = 1;
	else
		loop = 0;

	if (soundOn) {
		if (data == NULL) {
			i = GetFxIndex(id);
			if (i == MAXFX) {
				warning("PlayFx(%d, %d, %d, %d) - Not open", id, vol, pan, type);
				return RDERR_FXNOTOPEN;
			}
			if (loop == 1)
				fx[i]._flags |= SoundMixer::FLAG_LOOP;
			else 
				fx[i]._flags &= ~SoundMixer::FLAG_LOOP;
				 
			fx[i]._volume = vol;

			// Start the sound effect playing

			byte volume = fxMuted ? 0 : vol * fxVol;
			int8 p = panTable[pan + 16];

			g_engine->_mixer->playRaw(&fx[i]._handle, fx[i]._buf, fx[i]._bufSize, fx[i]._rate, fx[i]._flags, -1, volume, p);
		} else {
			if (type == RDSE_FXLEADIN || type == RDSE_FXLEADOUT) {
				if (type == RDSE_FXLEADIN)
					id = -2;
				else
					id = -1;

				hr = OpenFx(id, data);
				if (hr != RD_OK)
					return hr;

				i = GetFxIndex(id);
				if (i == MAXFX) {
					warning("PlayFx(%d, %d, %d, %d) - Not found", id, vol, pan, type);
					return RDERR_FXFUCKED;
				}
				fx[i]._flags &= ~SoundMixer::FLAG_LOOP;
				
				byte volume = musicMuted ? 0 : musicVolTable[musicVol];

				g_engine->_mixer->playRaw(&fx[i]._handle, fx[i]._buf, fx[i]._bufSize, fx[i]._rate, fx[i]._flags, -1, volume, 0);
			} else {
				hr = OpenFx(id, data);
				if (hr != RD_OK) {
					return hr;
				}

				i = GetFxIndex(id);
				if (i == MAXFX) {
					warning("PlayFx(%d, %d, %d, %d) - Not found", id, vol, pan, type);
					return RDERR_FXFUCKED;
				}
				if (loop == 1)
					fx[i]._flags |= SoundMixer::FLAG_LOOP;
				else 
					fx[i]._flags &= ~SoundMixer::FLAG_LOOP;
				fx[i]._volume = vol;

				// Start the sound effect playing

				byte volume = fxMuted ? 0 : vol * fxVol;
				int8 p = panTable[pan + 16];

				g_engine->_mixer->playRaw(&fx[i]._handle, fx[i]._buf, fx[i]._bufSize, fx[i]._rate, fx[i]._flags, -1, volume, p);
			}
		}
	}
	return RD_OK;
}

/**
 * Sets the volume and pan of the sample which is currently playing
 * @param id the id of the sample
 * @param vol volume
 * @param pan panning
 */

int32 Sword2Sound::SetFxVolumePan(int32 id, uint8 vol, int8 pan) {
	int32 i = GetFxIndex(id);
	if (i == MAXFX)
		return RDERR_FXNOTOPEN;

	fx[i]._volume = vol;
	if (!fxMuted) {
		g_engine->_mixer->setChannelVolume(fx[i]._handle, vol * fxVol);
		g_engine->_mixer->setChannelPan(fx[i]._handle, panTable[pan + 16]);
	}
	return RD_OK;
}

int32 Sword2Sound::SetFxIdVolume(int32 id, uint8 vol) {
	int32 i = GetFxIndex(id);

	if (i == MAXFX)
		return RDERR_FXNOTOPEN;

	fx[i]._volume = vol;
	if (!fxMuted)
		g_engine->_mixer->setChannelVolume(fx[i]._handle, vol * fxVol);

	return RD_OK;
}

/**
 * This function clears all of the sound effects which are currently open or
 * playing, irrespective of type.
 */

int32 Sword2Sound::ClearAllFx(void) {
	if (!soundOn)
		return(RD_OK);

	for (int i = 0; i < MAXFX; i++) {
		if (fx[i]._id && fx[i]._id != -1 && fx[i]._id != -2) {
			g_engine->_mixer->stopHandle(fx[i]._handle);
			fx[i]._id = 0;
			fx[i]._paused = false;
			if (fx[i]._buf != NULL) {
				free(fx[i]._buf);
				fx[i]._buf = NULL;
			}
			fx[i]._bufSize = 0;
			fx[i]._flags = 0;
		}
	}

	return RD_OK;
}

/**
 * This function closes a sound effect which has been previously opened for
 * playing. Sound effects must be closed when they are finished with, otherwise
 * you will run out of sound effect buffers.
 * @param id the id of the sound to close
 */

int32 Sword2Sound::CloseFx(int32 id) {
	int i;

	if (!soundOn)
		return RD_OK;

	i = GetFxIndex(id);
	if (i < MAXFX) {
		g_engine->_mixer->stopHandle(fx[i]._handle);
		fx[i]._id = 0;
		fx[i]._paused = false;
		if (fx[i]._buf != NULL) {
			free(fx[i]._buf);
			fx[i]._buf = NULL;
		}
		fx[i]._bufSize = 0;
		fx[i]._flags = 0;
	}

	return RD_OK;
}

int32 Sword2Sound::PauseFx(void) {
	if (!fxPaused) {
		for (int i = 0; i < MAXFX; i++) {
			if (fx[i]._id) {
				g_engine->_mixer->pauseHandle(fx[i]._handle, true);
				fx[i]._paused = true;
			} else
				fx[i]._paused = false;
		}
		fxPaused = 1;
	}

	return RD_OK;
}

int32 Sword2Sound::PauseFxForSequence(void) {
	if (!fxPaused) {
		for (int i = 0; i < MAXFX; i++) {
			if (fx[i]._id && fx[i]._id != -2) {
				g_engine->_mixer->pauseHandle(fx[i]._handle, true);
				fx[i]._paused = true;
			} else {
				fx[i]._paused = false;
			}
		}
		fxPaused = 1;
	}

	return RD_OK;
}

int32 Sword2Sound::UnpauseFx(void) {
	if (fxPaused) {
		for (int i = 0; i < MAXFX; i++) {
			if (fx[i]._paused && fx[i]._id) {
				g_engine->_mixer->pauseHandle(fx[i]._handle, false);
			}
		}
		fxPaused = 0;
	}

	return RD_OK;
}

/**
 * @return the master volume setting for sound effects
 */

uint8 Sword2Sound::GetFxVolume() {
	return fxVol;
}

/**
 * Set the master volume of all sound effects. The effects still have their
 * own volume setting as well as the master volume.
 * @param volume volume, from 0 (silent) to 14 (max)
 */

void Sword2Sound::SetFxVolume(uint8 volume) {
	fxVol = volume;

	// Now update the volume of any fxs playing
	for (int i = 0; i < MAXFX; i++) {
		if (fx[i]._id && !fxMuted)
			g_engine->_mixer->setChannelVolume(fx[i]._handle, fx[i]._volume * fxVol);
	}
}

/**
 * Mutes/Unmutes the sound effects.
 * @param mute If mute is 0, restore the volume to the last set master level.
 * Otherwise the sound effects are muted (volume 0).
 */

void Sword2Sound::MuteFx(uint8 mute) {
	fxMuted = mute;

	// Now update the volume of any fxs playing
	for (int i = 0; i < MAXFX; i++) {
		if (fx[i]._id) {
			byte volume = mute ? 0 : fx[i]._volume * fxVol;

			g_engine->_mixer->setChannelVolume(fx[i]._handle, volume);
		}
	}
}

/**
 * @return the sound effects's mute state, 1 if mute, 0 if not mute
 */

uint8 Sword2Sound::IsFxMute(void) {
	return fxMuted;
}

/**
 * Streams music from a cluster file.
 * @param filename the file name of the music cluster file
 * @param musicId the id of the music to stream
 * @param looping true if the music is to loop back to the start
 * @return RD_OK or an error code
 */

int32 Sword2Sound::StreamCompMusic(const char *filename, uint32 musicId, bool looping) {
	StackLock lock(_mutex);
	return StreamCompMusicFromLock(filename, musicId, looping);
}

int32 Sword2Sound::StreamCompMusicFromLock(const char *filename, uint32 musicId, bool looping) {
	uint32 len;
	int32 primaryStream = -1;
	int32 secondaryStream = -1;

	// If both music streams are playing, that should mean one of them is
	// fading out. Pick that one.

	if (music[0]._streaming && music[1]._streaming) {
		if (music[0]._fading)
			primaryStream = 0;
		else
			primaryStream = 1;

		music[primaryStream]._fading = 0;
		music[primaryStream]._streaming = false;
	}

	// Pick the available music stream. If no music is playing it doesn't
	// matter which we use, so pick the first one.

	if (music[0]._streaming || music[1]._streaming) {
		if (music[0]._streaming) {
			primaryStream = 1;
			secondaryStream = 0;
		} else {
			primaryStream = 0;
			secondaryStream = 1;
		}
	} else
		primaryStream = 0;

	// Save looping info and tune id
	music[primaryStream]._looping = looping;
	music[primaryStream]._id = musicId;

	// Don't start streaming if the volume is off.
	if (IsMusicMute())
		return RD_OK;

	// The assumption here is that we are never playing music from two
	// different files at the same time.

	if (!fpMus.isOpen())
		fpMus.open(filename);

	if (!fpMus.isOpen())
		return RDERR_INVALIDFILENAME;

	// Start other music stream fading out
	if (secondaryStream != -1 && !music[secondaryStream]._fading)
		music[secondaryStream]._fading = FADE_SAMPLES;

	fpMus.seek((musicId + 1) * 8, SEEK_SET);
	music[primaryStream]._fileStart = fpMus.readUint32LE();
	len = fpMus.readUint32LE();

	if (!music[primaryStream]._fileStart || !len)
		return RDERR_INVALIDID;

	music[primaryStream]._fileEnd = music[primaryStream]._fileStart + len;
	music[primaryStream]._filePos = music[primaryStream]._fileStart;
	music[primaryStream]._streaming = true;
	music[primaryStream]._firstTime = true;

	return RD_OK;
}

void Sword2Sound::UpdateCompSampleStreaming(int16 *data, uint len) {
	for (int i = 0; i < MAXMUS; i++) {
		if (!music[i]._streaming || music[i]._paused)
			continue;

		st_sample_t volume = musicMuted ? 0 : musicVolTable[musicVol];

		fpMus.seek(music[i]._filePos, SEEK_SET);
		_converter->flow(music[i], data, len, volume, volume);
	}

	// DipMusic();
}

int32 Sword2Sound::DipMusic() {
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

	int32				 currentMusicVol = musicVolTable[volMusic[0]];
	int32				 minMusicVol;

	// Find which music buffer is currently playing
	for (i = 0; i<MAXMUS && !dsbMusic; i++)
	{
		if (musStreaming[i] && musFading[i] == 0)
			dsbMusic = lpDsbMus[i];
	}

	if ((!musicMuted) && dsbMusic && (!speechMuted) && (volMusic[0]>2))
	{
		minMusicVol = musicVolTable[volMusic[0] - 3];

		if (speechStatus)
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
	
	return (hr);
*/
}

/**
 * @return the time left for the current music, in seconds.
 */

int32 Sword2Sound::MusicTimeRemaining() {
	StackLock lock(_mutex);

	for (int i = 0; i < MAXMUS; i++) {
		if (music[i]._streaming && !music[i]._fading)
			return (music[i]._fileEnd - music[i]._filePos) / 22050;
	}

	return 0;
}

/**
 * Fades out and stops the music.
 */

void Sword2Sound::StopMusic(void) {
	StackLock lock(_mutex);

	for (int i = 0; i < MAXMUS; i++) {
		if (music[i]._streaming)
			music[i]._fading = FADE_SAMPLES;
		else
			music[i]._looping = false;
	}
}

/**
 * Stops the music dead in its tracks.
 */

int32 Sword2Sound::PauseMusic(void) {
	StackLock lock(_mutex);

	if (soundOn) {
		for (int i = 0; i < MAXMUS; i++) {
			if (music[i]._streaming) {
				music[i]._paused = true;
			} else {
				music[i]._paused = false;
			}
		}
	}
	return RD_OK;
}

/**
 * Restarts the music from where it was stopped.
 */

int32 Sword2Sound::UnpauseMusic(void) {
	StackLock lock(_mutex);

	if (soundOn) {
		for (int i = 0; i < MAXMUS; i++)
			music[i]._paused = false;
	}
	return RD_OK;
}

/**
 * Set the volume of any future as well as playing music.
 * @param volume volume, from 0 (silent) to 16 (max)
 */

void Sword2Sound::SetMusicVolume(uint8 volume) {
	musicVol = volume;
}

/**
 * @return the volume setting for music
 */

uint8 Sword2Sound::GetMusicVolume() {
	return musicVol;
}

/**
 * Mutes/Unmutes the music.
 * @param mute If mute is 0, restore the volume to the last set master level.
 * Otherwise the music is muted (volume 0).
 */

void Sword2Sound::MuteMusic(uint8 mute) {
	musicMuted = mute;
}

/**
 * @return the music's mute state, 1 if mute, 0 if not mute
 */

uint8 Sword2Sound::IsMusicMute(void) {
	return musicMuted;
}
