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

//=============================================================================
//
//	Filename	:	d_sound.c
//	Created		:	3rd December 1996
//	By			:	P.R.Porter
//
//	Summary		:	This module holds the driver interface to direct sound.
//
//	Functions
//	---------
//
//	--------------------------------------------------------------------------
//
//	int32 PlayCompSpeech(const char *filename, uint32 id, uint8 vol, int8 pan)
//
//	This function loads, decompresses and plays the wav 'id' from the cluster
// 'filename'.  An error occurs if speech is already playing, or directSound
//  comes accross problems. 'volume' can be from 0 to 16. 'pan' can be from
//  -16 (full left) to 16 (full right).
//  id is the text line id used to reference the speech within the speech
//  cluster.
//
//	--------------------------------------------------------------------------
//
//	int32 StopSpeechSword2(void)
//
//	Stops the speech from playing.
//
//	--------------------------------------------------------------------------
//
//	int32 GetSpeechStatus(void)
//
//	Returns either RDSE_SAMPLEPLAYING or RDSE_SAMPLEFINISHED
//
//	--------------------------------------------------------------------------
//
//	int32 AmISpeaking(void)
//
//	Returns either RDSE_QUIET or RDSE_SPEAKING
//
//	--------------------------------------------------------------------------
//
//	int32 PauseSpeech(void)
//
//	Stops the speech dead in it's tracks.
//
//	--------------------------------------------------------------------------
//
//	int32 UnpauseSpeech(void)
//
//	Re-starts the speech from where it was stopped.
//
//	--------------------------------------------------------------------------
//
//	int32 OpenFx(int32 id, uint8 *data)
//
//	This function opens a sound effect ready for playing.  A unique id should
//	be passed in so that each effect can be referenced individually.
//
//	WARNING: Zero is not a valid ID.
//
//	--------------------------------------------------------------------------
//
//	int32 PlayFx(int32 id, uint8 *data, uint8 vol, int8 pan, uint8 type)
//
//	This function plays a sound effect.  If the effect has already been opened
//	then *data should be NULL, and the sound effect will simply be obtained 
//	from the id passed in.  If the effect has not been opened, then the wav
//	data should be passed in data.  The sound effect will be closed when it
//	has finished playing.
//
//	The volume can be between 0 (minimum) and 16 (maximum).  The pan defines
//	the left/right balance of the sample.  -16 is full left, and 16 is full
//	right with 0 in the middle.  The sample type can be either RDSE_FXSPOT, or
//	RDSE_FXLOOP.
//
//	WARNING: Zero is not a valid ID
//
//	--------------------------------------------------------------------------
//
//	int32 CloseFx(int32 id)
//
//	This function closes a sound effect which has been previously opened for
//	playing.  Sound effects must be closed when they are finished with,
//	otherwise you will run out of sound effect buffers.
//
//	--------------------------------------------------------------------------
//
//	int32 ClearAllFx(void)
//
//	This function clears all of the sound effects which are currently open or
//	playing, irrespective of type.
//
//	--------------------------------------------------------------------------
//
//	int32 StreamCompMusic(uint8 *filename, uint32 id, int32 loopFlag)
//
//	Streams music 'id' from the cluster file 'filename'.  The loopFlag should
//	be set to RDSE_FXLOOP if the music is to loop back to the start.
//	Otherwise, it should be RDSE_FXSPOT.
//	The return value must be checked for any problems.
//
//	--------------------------------------------------------------------------
//
//	void StopMusic(void)
//
//	Fades out and stops the music.
//
//	--------------------------------------------------------------------------
//
//	int32 PauseMusic(void)
//
//	Stops the music dead in it's tracks.
//
//	--------------------------------------------------------------------------
//
//	int32 UnpauseMusic(void)
//
//	Re-starts the music from where it was stopped.
//
//	---------------------------------------------------------------------------
//
//	int32 MusicTimeRemaining(void)
//
//  Returns the time left for the current tune.
//
//	----------------------------------------------------------------------------
//
//	int32 ReverseStereo(void)
//
//	This function reverse the pan table, thus reversing the stereo.
//
//=============================================================================

#include "stdafx.h"
#include "driver96.h"
#include "d_sound.h"
#include "../sword2.h"
#include "common/timer.h"
#include "sound/mixer.h"

// FIXME: Remove code duplication

// Decompression macros
#define GetCompressedShift(byte)      ((byte) >> 4)
#define GetCompressedSign(byte)       (((byte) >> 3) & 1)
#define GetCompressedAmplitude(byte)  ((byte) & 7)

int32 panTable[33] = {
	-127, -119, -111, -103, -95, -87, -79, -71, -63, -55, -47, -39, -31, -23, -15, -7,
	0,
	7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119, 127
};

int32 musicVolTable[17] = {
	0, 15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255
};

// FIXME: Is this really a good way? The music occasionally pops, and I have
// a feeling it could be that the stream is emptied faster than we are filling
// it. Maybe we should set up a premix function instead, but then we'd have to
// do rate conversion and mixing here.
//
// Maybe it'd be useful if audio streams could have their own callbacks to fill
// them with data?

void sword2_sound_handler(void *refCon) {
	Sword2Sound *sound = (Sword2Sound *) refCon;
	sound->FxServer();
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
	compressedMusic = 0;

	musicVol = 16;

	musicMuted = 0;
	bufferSizeMusic = 4410;
	_mixer = mixer;

	memset(fx, 0, sizeof(fx));
	memset(music, 0, sizeof(music));

	soundHandleSpeech = 0;

	soundOn = 1;
	g_engine->_timer->installProcedure(sword2_sound_handler, 100000, this);
}

Sword2Sound::~Sword2Sound() {
	g_engine->_timer->releaseProcedure(sword2_sound_handler);
	if (_mutex)
		g_system->delete_mutex(_mutex);
}

// --------------------------------------------------------------------------
// This function reverse the pan table, thus reversing the stereo.
// --------------------------------------------------------------------------

// FIXME: We could probably use the FLAG_REVERSE_STEREO mixer flag here.

int32 Sword2Sound::ReverseStereo(void) {
	int i, j;

	for (i = 0; i < 16; i++) {
		j = panTable[i];
		panTable[i] = panTable[32 - i];
		panTable[32 - i] = j;
	}

	return RD_OK;
}

// --------------------------------------------------------------------------
// This function returns the index of the sound effect with the ID passed in.
// --------------------------------------------------------------------------

int32 Sword2Sound::GetFxIndex(int32 id) {
	int32 i;

	for (i = 0; i < MAXFX; i++) {
		if (fx[i]._id == id)
			break;
	}

	return i;
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

void Sword2Sound::FxServer(void) {
	StackLock lock(_mutex);

	if (!soundOn)
		return;

	if (!music[0]._paused && !music[1]._paused) {
		if (compressedMusic == 1)
			UpdateCompSampleStreaming();
	}

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
			if ((fx[i]._id == (int32) 0xfffffffe) || (fx[i]._id == (int32) 0xffffffff)) {
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

int32 Sword2Sound::AmISpeaking() {
	if (!speechMuted && !speechPaused && soundHandleSpeech != 0)
		return RDSE_SPEAKING;

	return RDSE_QUIET;
}

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

	// FIXME: See comment in UpdateCompSampleStreaming()
	// DipMusic();

	return RD_OK;
}

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

void Sword2Sound::SetSpeechVolume(uint8 volume) {
	speechVol = volume;
	if (soundHandleSpeech != 0 && !speechMuted && GetSpeechStatus() == RDSE_SAMPLEPLAYING) {
		g_engine->_mixer->setChannelVolume(soundHandleSpeech, 16 * speechVol);
	}
}

uint8 Sword2Sound::GetSpeechVolume() {
	return speechVol;
}

void Sword2Sound::MuteSpeech(uint8 mute) {
	speechMuted = mute;

	if (GetSpeechStatus() == RDSE_SAMPLEPLAYING) {
		byte volume = mute ? 0 : 16 * speechVol;

		g_engine->_mixer->setChannelVolume(soundHandleSpeech, volume);
	}
}

uint8 Sword2Sound::IsSpeechMute(void) {
	return speechMuted;
}

int32 Sword2Sound::PauseSpeech(void) {
	if (GetSpeechStatus() == RDSE_SAMPLEPLAYING) {
		speechPaused = 1;
		g_engine->_mixer->pauseHandle(soundHandleSpeech, true);
	}
	return RD_OK;
}

int32 Sword2Sound::UnpauseSpeech(void) {
	if (speechPaused) {
		speechPaused = 0;
		g_engine->_mixer->pauseHandle(soundHandleSpeech, false);
	}
	return RD_OK;
}

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

int32 Sword2Sound::PlayFx(int32 id, uint8 *data, uint8 vol, int8 pan, uint8 type) {
	int32 i, loop;
	uint32 hr;

	if (type == RDSE_FXLOOP)
		loop = 1;
	else
		loop = 0;

	if (soundOn) {
		if (data == NULL) {
			if (type == RDSE_FXLEADOUT) {
				id = (int32) 0xffffffff;
				i = GetFxIndex(id);
				if (i == MAXFX) {
					warning("PlayFx(%d, %d, %d, %d) - Not open", id, vol, pan, type);
					return RDERR_FXNOTOPEN;
				}
				fx[i]._flags &= ~SoundMixer::FLAG_LOOP;

				// Start the sound effect playing

				byte volume = musicMuted ? 0 : musicVolTable[musicVol];

				g_engine->_mixer->playRaw(&fx[i]._handle, fx[i]._buf, fx[i]._bufSize, fx[i]._rate, fx[i]._flags, -1, volume, 0);
			} else {
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
			}
		} else {
			if (type == RDSE_FXLEADIN) {
				id = (int32) 0xfffffffe;
				hr = OpenFx(id, data);
				if (hr != RD_OK) {
					return hr;
				}
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

int32 Sword2Sound::ClearAllFx(void) {
	if (!soundOn)
		return(RD_OK);

	for (int i = 0; i < MAXFX; i++) {
		if (fx[i]._id && fx[i]._id != (int32) 0xfffffffe && fx[i]._id != (int32) 0xffffffff) {
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
			if (fx[i]._id && fx[i]._id != (int32) 0xfffffffe) {
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

uint8 Sword2Sound::GetFxVolume() {
	return fxVol;
}

void Sword2Sound::SetFxVolume(uint8 volume) {
	fxVol = volume;

	// Now update the volume of any fxs playing
	for (int i = 0; i < MAXFX; i++) {
		if (fx[i]._id && !fxMuted)
			g_engine->_mixer->setChannelVolume(fx[i]._handle, fx[i]._volume * fxVol);
	}
}

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

uint8 Sword2Sound::IsFxMute(void) {
	return fxMuted;
}

int32 Sword2Sound::StreamCompMusic(const char *filename, uint32 musicId, bool looping) {
	StackLock lock(_mutex);
	return StreamCompMusicFromLock(filename, musicId, looping);
}

int32 Sword2Sound::StreamCompMusicFromLock(const char *filename, uint32 musicId, bool looping) {
	int32 primaryStream = -1;
	int32 secondaryStream = -1;
	int32 i;
	uint16 *data16;
	uint8 *data8;

	compressedMusic = 1;

	// If both music streams are playing, that should mean one of them is
	// fading out. Pick that one.

	if (music[0]._streaming && music[1]._streaming) {
		if (music[0]._fading)
			primaryStream = 0;
		else
			primaryStream = 1;

		music[primaryStream]._fading = false;
		g_engine->_mixer->stopHandle(music[primaryStream]._handle);
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

	strcpy(music[primaryStream]._fileName, filename);

	// Save looping info and tune id
	music[primaryStream]._looping = looping;
	music[primaryStream]._id = musicId;

	// Don't start streaming if the volume is off.
	if (IsMusicMute())
		return RD_OK;

	// Always use fpMus[0] (all music in one cluster)
	// musFilePos[primaryStream] for different pieces of music.
	if (!fpMus.isOpen())
		fpMus.open(filename);

	if (!fpMus.isOpen())
		return RDERR_INVALIDFILENAME;

	// Start other music stream fading out
	if (secondaryStream != -1 && !music[secondaryStream]._fading)
		music[secondaryStream]._fading = -16;

	fpMus.seek((musicId + 1) * 8, SEEK_SET);
	music[primaryStream]._filePos = fpMus.readUint32LE();
	music[primaryStream]._fileEnd = fpMus.readUint32LE();

	if (!music[primaryStream]._filePos || !music[primaryStream]._fileEnd)
		return RDERR_INVALIDID;

	// Calculate the file position of the end of the music
	music[primaryStream]._fileEnd += music[primaryStream]._filePos;

	// Create a temporary buffer
	data8 = (uint8 *) malloc(bufferSizeMusic / 2);
	if (!data8)
		return RDERR_OUTOFMEMORY;

	// Seek to start of the compressed music
	fpMus.seek(music[primaryStream]._filePos, SEEK_SET);

	// Read the compressed data in to the buffer
	if ((int32) fpMus.read(data8, bufferSizeMusic / 2) != bufferSizeMusic / 2) {
		free(data8);
		return RDERR_INVALIDID;
	}

	// Store the current position in the file for future streaming
	music[primaryStream]._filePos = fpMus.pos();

	// decompress the music into the music buffer.
	data16 = (uint16 *) malloc(bufferSizeMusic);
	if (!data16)
		return RDERR_OUTOFMEMORY;

	data16[0] = READ_LE_UINT16(data8);	// First sample value

	for (i = 1; i < (bufferSizeMusic / 2) - 1; i++) {
		if (GetCompressedSign(data8[i + 1]))
			data16[i] = data16[i - 1] - (GetCompressedAmplitude(data8[i + 1]) << GetCompressedShift(data8[i + 1]));
		else
			data16[i] = data16[i - 1] + (GetCompressedAmplitude(data8[i + 1]) << GetCompressedShift(data8[i + 1]));
	}

	// Store the value of the last sample ready for next batch of
	// decompression
	music[primaryStream]._lastSample = data16[i - 1];

	// Free the compressed sound buffer
	free(data8);

	// Modify the volume according to the master volume and music mute
	// state

	byte volume = musicMuted ? 0 : musicVolTable[musicVol];

#ifndef SCUMM_BIG_ENDIAN
	// FIXME: Until the mixer supports LE samples natively, we need to
	// convert our LE ones to BE
	for (i = 0; i < bufferSizeMusic / 2; i++)
		data16[i] = SWAP_BYTES_16(data16[i]);
#endif

	g_engine->_mixer->newStream(&music[primaryStream]._handle, data16,
		bufferSizeMusic, 22050, SoundMixer::FLAG_16BITS, 100000, volume, 0);
		
	free(data16);

	// Recorder some last variables
	music[primaryStream]._streaming = true;
	return RD_OK;
}

void Sword2Sound::UpdateCompSampleStreaming(void) {
	uint32 i,j;
	int32 len;
	uint16 *data16;
	uint8 *data8;
	int fade;

	for (i = 0; i < MAXMUS; i++) {
		if (!music[i]._streaming)
			continue;

		// If the music is fading, adjust the volume for it.

		if (music[i]._fading < 0) {
			if (++music[i]._fading == 0) {
				g_engine->_mixer->stopHandle(music[i]._handle);
				music[i]._streaming = false;
				music[i]._looping = false;
				continue;
			}

    			// Modify the volume according to the master volume
			// and music mute state

			byte volume = musicMuted ? 0 : musicVolTable[musicVol * (0 - music[i]._fading ) / 16];

			g_engine->_mixer->setChannelVolume(music[i]._handle, volume);
		}

		// Re-fill the audio buffer.

		len = bufferSizeMusic;

		// Reduce length if it requires reading past the end of the
		// music

		if (music[i]._filePos + len >= music[i]._fileEnd) {
			// End of music reached so we'll need to fade and
			// repeat
			len = music[i]._fileEnd - music[i]._filePos;
			fade = 1;
		} else
			fade = 0;

		if (len > 0) {
			data8 = (uint8 *) malloc(len / 2);
			// Allocate a compressed data buffer
			if (data8 == NULL) {
				g_engine->_mixer->stopHandle(music[i]._handle);
				music[i]._streaming = false;
				music[i]._looping = false;
				continue;
			}

			// Seek to update position of compressed music when
			// neccassary (probably never occurs)
			if ((int32) fpMus.pos() != music[i]._filePos)
				fpMus.seek(music[i]._filePos, SEEK_SET);

			// Read the compressed data in to the buffer
			if ((int32) fpMus.read(data8, len / 2) != len / 2) {
				g_engine->_mixer->stopHandle(music[i]._handle);
				free(data8);
				music[i]._streaming = false;
				music[i]._looping = false;
				continue;
			}

			// Update the current position in the file for future
			// streaming

			music[i]._filePos = fpMus.pos();

			// decompress the music into the music buffer.
			data16 = (uint16 *) malloc(len);

			// Decompress the first byte using the last
			// decompressed sample
			if (GetCompressedSign(data8[0]))
				data16[0] = music[i]._lastSample - (GetCompressedAmplitude(data8[0]) << GetCompressedShift(data8[0]));
			else
				data16[0] = music[i]._lastSample + (GetCompressedAmplitude(data8[0]) << GetCompressedShift(data8[0]));

			for (j = 1; j < (uint32) len / 2; j++) {
				if (GetCompressedSign(data8[j]))
					data16[j] = data16[j - 1] - (GetCompressedAmplitude(data8[j]) << GetCompressedShift(data8[j]));
				else
					data16[j] = data16[j - 1] + (GetCompressedAmplitude(data8[j]) << GetCompressedShift(data8[j]));
			}

			music[i]._lastSample = data16[j - 1];

#ifndef SCUMM_BIG_ENDIAN
			// Until the mixer supports LE samples natively, we
			// need to convert our LE ones to BE
			for (int32 y = 0; y < len / 2; y++)
				data16[y] = SWAP_BYTES_16(data16[y]);
#endif

			// Paranoid check that seems to be necessary.
			if (len & 1)
				len--;

			g_engine->_mixer->appendStream(music[i]._handle, data16, len);
					
			free(data16);
			free(data8);
		}

		// End of the music so we need to start fading and start the
		// music again

		if (fade) {
			g_engine->_mixer->stopHandle(music[i]._handle);

			// FIXME: The original code faded the music here, but
			// to do that we need to start before we reach the end
			// of the file.
			//
			// On the other hand, do we want to fade out the end
			// of the music?

			// Fade the old music
			// musFading[i] = -16;

			// Loop if neccassary
			if (music[i]._looping)
				StreamCompMusicFromLock(music[i]._fileName, music[i]._id, music[i]._looping);
		}
	}

	// FIXME: We need to implement DipMusic()'s functionality, but since
	// our sound buffer is much shorter than the original's it should be
	// enough to simply modify the channel volume in this function instead
	// of using a separate function to modify part of the sound buffer.

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

int32 Sword2Sound::MusicTimeRemaining() {
	StackLock lock(_mutex);

	for (int i = 0; i < MAXMUS; i++) {
		if (music[i]._streaming && !music[i]._fading)
			return (music[i]._fileEnd - music[i]._filePos) / 22050;
	}

	return 0;
}

void Sword2Sound::StopMusic(void) {
	StackLock lock(_mutex);

	for (int i = 0; i < MAXMUS; i++) {
		if (music[i]._streaming)
			music[i]._fading = -16;
		else
			music[i]._looping = false;
	}
}

int32 Sword2Sound::PauseMusic(void) {
	StackLock lock(_mutex);

	if (soundOn) {
		for (int i = 0; i < MAXMUS; i++) {
			if (music[i]._streaming) {
				music[i]._paused = true;
				g_engine->_mixer->pauseHandle(music[i]._handle, true);
			} else {
				music[i]._paused = false;
			}
		}
	}
	return RD_OK;
}

int32 Sword2Sound::UnpauseMusic(void) {
	StackLock lock(_mutex);

	if (soundOn) {
		for (int i = 0; i < MAXMUS; i++) {
			if (music[i]._paused) {
				g_engine->_mixer->pauseHandle(music[i]._handle, false);
				music[i]._paused = false;
			}
		}
	}
	return RD_OK;
}

void Sword2Sound::SetMusicVolume(uint8 volume) {
	StackLock lock(_mutex);

	musicVol = volume;

	for (int i = 0; i < MAXMUS; i++) {
		if (music[i]._streaming && !music[i]._fading && !musicMuted)
			g_engine->_mixer->setChannelVolume(music[i]._handle, musicVolTable[volume]);
	}
}

uint8 Sword2Sound::GetMusicVolume() {
	return musicVol;
}

void Sword2Sound::MuteMusic(uint8 mute) {
	StackLock lock(_mutex);

	musicMuted = mute;

	for (int i = 0; i < MAXMUS; i++) {
		if (!mute) {
			if (!music[i]._streaming && music[i]._looping)
				StreamCompMusicFromLock(music[i]._fileName, music[i]._id, music[i]._looping);
		}

		if (music[i]._streaming && !music[i]._fading) {
			byte volume = mute ? 0 : musicVolTable[musicVol];

			g_engine->_mixer->setChannelVolume(music[i]._handle, volume);
		}
	}
}

uint8 Sword2Sound::IsMusicMute(void) {
	return musicMuted;
}
