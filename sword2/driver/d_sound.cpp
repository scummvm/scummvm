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
#include "rdwin.h"			// for hwnd.
#include "d_sound.h"
#include "../sword2.h"
#include "common/timer.h"
#include "sound/mixer.h"

// Decompression macros
#define MakeCompressedByte(shift, sign, amplitude) (((shift) << 4) + ((sign) << 3) + (amplitude))
#define GetCompressedShift(byte)                   ((byte) >> 4)
#define GetCompressedSign(byte)                    (((byte) >> 3) & 1)
#define GetCompressedAmplitude(byte)               ((byte) & 7)
#define GetdAPower(dA, power)                      for (power = 15; power > 0 && !((dA) & (1 << power)); power--)

int32 panTable[33] = {
	-127, -119, -111, -103, -95, -87, -79, -71, -63, -55, -47, -39, -31, -23, -15, -7,
	0,
	7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119, 127
};
/*
int32 panTable[33] = {
	-10000,
	-1500, -1400, -1300, -1200,
	-1100, -1000, -900, -800,
	-700, -600, -500, -400,
	-300, -200, -100, 0,
	100, 200, 300, 400,
	500, 600, 700, 800,
	900, 1000, 1100, 1200,
	1300, 1400, 1500, 10000
};

int32 volTable[241] = {
-10000,	-3925,	-3852,	-3781,	-3710,	-3642,	-3574,	-3508,	-3443,	-3379,	-3316,	-3255,	-3194,	-3135,	-3077,	-3020,	-2964,	-2909,	-2855,	-2802,	-2750,	-2699,	-2649,	-2600,	-2551,	-2504,	-2458,	-2412,	-2367,	-2323,	-2280,	-2238,	-2197,	-2156,	-2116,	-2077,	-2038,	-2000,	-1963,	-1927,	-1891,	-1856,	-1821,	-1788,	-1755,	-1722,	-1690,	-1659,	-1628,	-1598,	-1568,	-1539,	-1510,	-1482,	-1455,	-1428,	-1401,	-1375,	-1350,	-1325,	
-1300,	-1290,	-1279,	-1269,	-1259,	-1249,	-1239,	-1229,	-1219,	-1209,	-1199,	-1190,	-1180,	-1171,	-1161,	-1152,	-1142,	-1133,	-1124,	-1115,	-1106,	-1097,	-1088,	-1080,	-1071,	-1062,	-1054,	-1045,	-1037,	-1028,	-1020,	-1012,	-1004,	-996,	-988,	-980,	-972,	-964,	-956,	-949,	-941,	-933,	-926,	-918,	-911,	-904,	-896,	-889,	-882,	-875,	-868,	-861,	-854,	-847,	-840,	-833,	-827,	-820,	-813,	-807,	
-800,	-791,	-782,	-773,	-764,	-755,	-747,	-738,	-730,	-721,	-713,	-705,	-697,	-689,	-681,	-673,	-665,	-658,	-650,	-643,	-635,	-628,	-621,	-613,	-606,	-599,	-593,	-586,	-579,	-572,	-566,	-559,	-553,	-546,	-540,	-534,	-528,	-522,	-516,	-510,	-504,	-498,	-492,	-487,	-481,	-476,	-470,	-465,	-459,	-454,	-449,	-444,	-439,	-434,	-429,	-424,	-419,	-414,	-409,	-404,	
-400,	-362,	-328,	-297,	-269,	-244,	-221,	-200,	-181,	-164,	-148,	-134,	-122,	-110,	-100,	-90,	-82,	-74,	-67,	-61,	-55,	-50,	-45,	-41,	-37,	-33,	-30,	-27,	-25,	-22,	-20,	-18,	-16,	-15,	-13,	-12,	-11,	-10,	-9,	-8,	-7,	-6,	-6,	-5,	-5,	-4,	-4,	-3,	-3,	-3,	-2,	-2,	-2,	-2,	-1,	-1,	-1,	-1,	-1,	0	
};	
*/
int32 musicVolTable[17] = {
	0, 15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255
};
/*
int32 musicVolTable[17] = {
  -10000, 
  -5000, -3000, -2500, -2250, 
  -2000, -1750, -1500, -1250, 
  -1000, -750, -500, -350, 
  -200, -100, -50, 0
};
*/
void sword2_sound_handler(void *refCon) {
	Sword2Sound *sound = (Sword2Sound *)refCon;
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

	volMusic[0] = 16;
	volMusic[1] = 16;
	musicMuted = 0;
	bufferSizeMusic = 4410;
	_mixer = mixer;
	
	memset(fxId,		0, sizeof(fxId));
	memset(fxiPaused,	0, sizeof(fxiPaused));
	memset(fxRate,		0, sizeof(fxRate));

 	memset(musStreaming,	0, sizeof(musStreaming));
	memset(musicPaused,	0, sizeof(musicPaused));
	memset(musFading, 	0, sizeof(musFading));
	memset(musLooping,	0, sizeof(musLooping));
	memset(musFilePos,	0, sizeof(musFilePos));
	memset(musEnd,		0, sizeof(musEnd));
	memset(musLastSample,	0, sizeof(musLastSample));
	memset(musId,		0, sizeof(musId));
	memset(soundHandleMusic, 0, sizeof(soundHandleMusic));
	memset(soundHandleFx, 0, sizeof(soundHandleFx));
	soundHandleSpeech = 0;
	memset(bufferFx, 0, sizeof(bufferFx));
	memset(flagsFx, 0, sizeof(flagsFx));
	memset(bufferSizeFx, 0, sizeof(bufferSizeFx));

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

int32 Sword2Sound::ReverseStereo(void) {
	int i,j;

	for (i = 0; i < 16; i++) {
		j = panTable[i];
		panTable[i] = panTable[32 - i];
		panTable[32 - i] = j;
	}

	return (RD_OK);
}

//	--------------------------------------------------------------------------
//	This function returns the index of the sound effect with the ID passed in.
//	--------------------------------------------------------------------------
int32 Sword2Sound::GetFxIndex(int32 id) {
	int32 i = 0;

	while (i < MAXFX) {
		if (fxId[i] == id)
			break;
		i++;
	}

	return(i);
}

int32 Sword2Sound::IsFxOpen(int32 id) {
	int32 i = 0;

	while (i < MAXFX) {
		if (fxId[i] == id)
			break;
		i++;
	}

	if (i == MAXFX)
		return 1;
	else
		return 0;
}

//	--------------------------------------------------------------------------
//	This function checks the status of all current sound effects, and clears
//	out the ones which are no longer required in a buffer.  It is called on
//	a slow timer from rdwin.c
//	--------------------------------------------------------------------------
void Sword2Sound::FxServer(void) {
	StackLock lock(_mutex);

	if (!soundOn)
		return;

	if (musicPaused[0] + musicPaused[1] == 0) {
		if (compressedMusic == 1)
			UpdateCompSampleStreaming();
	}

	if (!musStreaming[0] && !musStreaming[1] && fpMus.isOpen())
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
			if ((fxId[i] == (int32) 0xfffffffe) || (fxId[i] == (int32) 0xffffffff)) {
				if (!soundHandleFx[i]) {
					fxId[i] = 0;
					if (bufferFx[i] != NULL) {
						free(bufferFx[i]);
						bufferFx[i] = NULL;
					}
					bufferSizeFx[i] = 0;
					flagsFx[i] = 0;
				}
			}
		}
		return;
	}

	for (i = 0; i < MAXFX; i++) {
		if (fxId[i]) {
			if (!soundHandleFx[i]) {
				fxId[i] = 0;
				if (bufferFx[i] != NULL) {
					free(bufferFx[i]);
					bufferFx[i] = NULL;
				}
				bufferSizeFx[i] = 0;
				flagsFx[i] = 0;
			}
		}
	}
#endif
}

int32 Sword2Sound::AmISpeaking() {
	if ((!speechMuted) && (!speechPaused) && (soundHandleSpeech != 0)) {
		return (RDSE_SPEAKING);
	}
	return (RDSE_QUIET);
}

int32 Sword2Sound::GetCompSpeechSize(const char *filename, uint32 speechid) {
 	int32 			i;
	uint32			speechIndex[2];
	File		    fp;
	
  //  Open the speech cluster and find the data offset & size
	fp.open(filename, g_engine->getGameDataPath());
	if (fp.isOpen() == false)
		return(0);

	fp.seek((++speechid) * 8, SEEK_SET);

	if (fp.read(speechIndex, sizeof(uint32) * 2) != (sizeof(uint32) * 2)) {
		fp.close();
		return (0);
	}

	if (!FROM_LE_32(speechIndex[0]) || !FROM_LE_32(speechIndex[1])) {
		fp.close();
		return (0);
	}

	fp.close();

	i = (FROM_LE_32(speechIndex[1]) - 1) * 2 + sizeof(_wavHeader) + 8;
	
	return(i);
}

int32 Sword2Sound::PreFetchCompSpeech(const char *filename, uint32 speechid, uint8 *waveMem) {
 	uint32 			i;
	uint16			*data16;
	uint8			*data8;
	uint32			speechIndex[2];
	_wavHeader		*pwf = (_wavHeader *) waveMem;
	File		   fp;

	//  Open the speech cluster and find the data offset & size
	fp.open(filename, g_engine->getGameDataPath());
	if (fp.isOpen() == false)
		return(RDERR_INVALIDFILENAME);

	fp.seek((++speechid) * 8, SEEK_SET);

	if (fp.read(speechIndex, sizeof(uint32) * 2) != (sizeof(uint32) * 2)) {
		fp.close();
		return (RDERR_READERROR);
	}

	if (!speechIndex[0] || !speechIndex[1]) {
		fp.close();
		return (RDERR_INVALIDID);
	}

	data16 = (uint16*)(waveMem + sizeof(_wavHeader));

	memset(pwf, 0, sizeof(_wavHeader));

	pwf->riff = MKID('RIFF');
	pwf->wavID = MKID('WAVE');
	pwf->format = MKID('fmt ');

	pwf->formatLen		= TO_LE_32(0x00000010);
	pwf->formatTag		= TO_LE_16(0x0001);
	pwf->channels		= TO_LE_16(0x0001);
	pwf->samplesPerSec	= TO_LE_16(0x5622);
	pwf->avgBytesPerSec = TO_LE_16(0x0000);
	pwf->blockAlign		= TO_LE_16(0xAC44);
	pwf->unknown1		= TO_LE_16(0x0000);
	pwf->unknown2		= TO_LE_16(0x0002);
	pwf->bitsPerSample	= TO_LE_16(0x0010);

	*((uint32*)data16) = MKID('data');

	data16 += 2;

	*((uint32*)data16) = TO_LE_32((speechIndex[1] - 1) * 2);

	data16 += 2;

	pwf->fileLength = (speechIndex[1] - 1) * 2 + sizeof(_wavHeader) + 8;

	// Calculate position in buffer to load compressed sound into
	data8 = (uint8*)data16 + (speechIndex[1]-1);
	
	fp.seek(speechIndex[0], SEEK_SET);

	if (fp.read(data8, speechIndex[1]) != speechIndex[1]) {
		fp.close();
		return (RDERR_INVALIDID);
	}

	fp.close();

	data16[0] = READ_LE_UINT16(data8);	// Starting Value

	for (i = 1; i < (speechIndex[1] - 1); i++) {
		if (GetCompressedSign(data8[i + 1]))
			data16[i] = data16[i - 1] - (GetCompressedAmplitude(data8[i + 1]) << GetCompressedShift(data8[i + 1]));
		else
			data16[i] = data16[i - 1] + (GetCompressedAmplitude(data8[i + 1]) << GetCompressedShift(data8[i + 1]));
	}

	return(RD_OK);
}

int32 Sword2Sound::PlayCompSpeech(const char *filename, uint32 speechid, uint8 vol, int8 pan) {
 	uint32	i;
	uint16	*data16;
	uint8	*data8;
	uint32	speechIndex[2];
	File	fp;
	uint32	bufferSize;
	
	if (!speechMuted) {
		if (GetSpeechStatus() == RDERR_SPEECHPLAYING)
			return RDERR_SPEECHPLAYING;

		//  Open the speech cluster and find the data offset & size
		fp.open(filename, g_engine->getGameDataPath());
		if (fp.isOpen() == false) 
			return(RDERR_INVALIDFILENAME);
		
		fp.seek((++speechid) * 8, SEEK_SET);

		if (fp.read(speechIndex, sizeof(uint32) * 2) != (sizeof(uint32) * 2)) {
			fp.close();
			return (RDERR_READERROR);
		}

		if (speechIndex[0] == 0 || speechIndex[1] == 0) {
			fp.close();
			return (RDERR_INVALIDID);
		}

		bufferSize = (speechIndex[1] - 1) * 2;

		// Create tempory buffer for compressed speech
		if ((data8 = (uint8 *)malloc(speechIndex[1])) == NULL) {
			fp.close();
			return(RDERR_OUTOFMEMORY);
		}

		fp.seek(speechIndex[0], SEEK_SET);

		if (fp.read(data8, speechIndex[1]) != speechIndex[1]) {
			fp.close();
			free(data8);
			return (RDERR_INVALIDID);
		}

		fp.close();

		// Decompress data into speech buffer.
		data16 = (uint16*)malloc(bufferSize);

		data16[0] = READ_LE_UINT16(data8);	// Starting Value

		for (i = 1; i < bufferSize / 2; i++) {
			if (GetCompressedSign(data8[i + 1]))
				data16[i] = data16[i - 1] - (GetCompressedAmplitude(data8[i + 1]) << GetCompressedShift(data8[i + 1]));
			else
				data16[i] = data16[i - 1] + (GetCompressedAmplitude(data8[i + 1]) << GetCompressedShift(data8[i + 1]));
		}

		free(data8);

		// Modify the volume according to the master volume
		byte volume;
		int8 p;
		if (speechMuted) {
			volume = 0;
		} else {
			volume = vol * speechVol;
		}
		p = panTable[pan + 16];

		// Start the speech playing
		speechPaused = 1;
			
		uint32 flags = SoundMixer::FLAG_16BITS;
		flags |= SoundMixer::FLAG_AUTOFREE;

#ifndef SCUMM_BIG_ENDIAN
		// Until the mixer supports LE samples natively, we need to convert our LE ones to BE
		for (uint j = 0; j < (bufferSize / 2); j++)
			data16[j] = SWAP_BYTES_16(data16[j]);
#endif

		_mixer->playRaw(&soundHandleSpeech, data16, bufferSize, 22050, flags, -1, volume, pan);

		speechStatus = 1;
	}

	DipMusic();
	return (RD_OK);
}

int32 Sword2Sound::StopSpeechSword2(void) {
	if (!soundOn)
		return(RD_OK);
  
	if (speechStatus) {
		g_engine->_mixer->stopHandle(soundHandleSpeech);
		speechStatus = 0;
		return(RD_OK);
	}
	return(RDERR_SPEECHNOTPLAYING);
}

int32 Sword2Sound::GetSpeechStatus(void) {
	if ((!soundOn) || (!speechStatus))
		return(RDSE_SAMPLEFINISHED);

	if (speechPaused)
		return(RDSE_SAMPLEPLAYING);

	if (!soundHandleSpeech) {
		speechStatus = 0;
		return(RDSE_SAMPLEFINISHED);
	}
	return(RDSE_SAMPLEPLAYING);
}

void Sword2Sound::SetSpeechVolume(uint8 volume) {
	speechVol = volume;
	if ((soundHandleSpeech != 0) && !speechMuted && GetSpeechStatus() == RDSE_SAMPLEPLAYING) {
		g_engine->_mixer->setChannelVolume(soundHandleSpeech, 16 * speechVol);
	}
}

uint8 Sword2Sound::GetSpeechVolume() {
	return speechVol;
}

void Sword2Sound::MuteSpeech(uint8 mute) {
	speechMuted = mute;

	if (GetSpeechStatus() == RDSE_SAMPLEPLAYING) {
		if (mute) {
			g_engine->_mixer->setChannelVolume(soundHandleSpeech, 0);
		} else {
			g_engine->_mixer->setChannelVolume(soundHandleSpeech, 16 * speechVol);
		}
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
	return(RD_OK);
}

int32 Sword2Sound::UnpauseSpeech(void) {
	if (speechPaused) {
		speechPaused = 0;
		g_engine->_mixer->pauseHandle(soundHandleSpeech, false);
	}
	return(RD_OK);
}

int32 Sword2Sound::OpenFx(int32 id, uint8 *data) {
 	int32 	i, fxi;
	uint32	*data32 = NULL;
	_wavHeader	*wav;

	wav = (_wavHeader *) data;

	if (soundOn) {
		// Check for a valid id.
		if (id == 0)
			return(RDERR_INVALIDID);

		// Check that the fx is not already open
		for (i = 0; i < MAXFX; i++) {
			if (fxId[i] == id)
				return(RDERR_FXALREADYOPEN);
		}

		// Now choose a free slot for the fx
		fxi = 0;
		while (fxi < MAXFX) {
			if (fxId[fxi] == 0)
				break;
			fxi++;
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

			fxi = 0;
			while (fxi < MAXFX) {
				if (!soundHandleFx[fxi])
					break;
				fxi++;
			}

			// Still no dice? I give up!

			if (fxi == MAXFX)
				return(RDERR_NOFREEBUFFERS);
		}

    //  Set the sample size - search for the size of the data.
		i = 0;
		while (i < 100) {
			if (*data == 'd') {
				data32 = (uint32*)data;
				if (READ_UINT32(data32) == MKID('data'))
					break;
			}
			i += 1;
			data++;
		}
		if (!data32)
			return(RDERR_INVALIDWAV);

		bufferSizeFx[fxi] = READ_LE_UINT32(data32 + 1);

		//	Fill the speech buffer with data
		if (bufferFx[fxi] != NULL)
			free(bufferFx[fxi]);
		bufferFx[fxi] = (uint16*)malloc(bufferSizeFx[fxi]);
		memcpy(bufferFx[fxi], (uint8 *)(data32 + 2), bufferSizeFx[fxi]);
		flagsFx[fxi] = SoundMixer::FLAG_16BITS;
		if (FROM_LE_16(wav->channels) == 2)
			flagsFx[fxi] |= SoundMixer::FLAG_STEREO;

		fxRate[fxi] = FROM_LE_16(wav->samplesPerSec);

		// Until the mixer supports LE samples natively, we need to convert our LE ones to BE
		for (int32 j = 0; j < (bufferSizeFx[fxi] / 2); j++)
			bufferFx[fxi][j] = SWAP_BYTES_16(bufferFx[fxi][j]);

		fxId[fxi] = id;
	}
	return(RD_OK);
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
					return(RDERR_FXNOTOPEN);
				}
				flagsFx[i] &= ~SoundMixer::FLAG_LOOP;

				byte volume;
				//	Start the sound effect playing
				if (musicMuted) {
					volume = 0;
				} else {
					volume = musicVolTable[volMusic[0]];
				}
				g_engine->_mixer->playRaw(&soundHandleFx[i], bufferFx[i], bufferSizeFx[i], fxRate[i], flagsFx[i], -1, volume, 0);
			} else {
				i = GetFxIndex(id);
				if (i == MAXFX) {
					warning("PlayFx(%d, %d, %d, %d) - Not open", id, vol, pan, type);
					return(RDERR_FXNOTOPEN);
				}
				if (loop == 1)
					flagsFx[i] |= SoundMixer::FLAG_LOOP;
				else 
					flagsFx[i] &= ~SoundMixer::FLAG_LOOP;
				 
				fxVolume[i] = vol;

				byte volume;
				int8 p;
				//	Start the sound effect playing
				if (fxMuted) {
					volume = 0;
				} else {
					volume = vol * fxVol;
				}
				p = panTable[pan + 16];

				g_engine->_mixer->playRaw(&soundHandleFx[i], bufferFx[i], bufferSizeFx[i], fxRate[i], flagsFx[i], -1, volume, p);
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
				flagsFx[i] &= ~SoundMixer::FLAG_LOOP;
				
				byte volume;
				if (musicMuted) {
					volume = 0;
				} else {
					volume = musicVolTable[volMusic[0]];
				}
				g_engine->_mixer->playRaw(&soundHandleFx[i], bufferFx[i], bufferSizeFx[i], fxRate[i], flagsFx[i], -1, volume, 0);
			} else {
				hr = OpenFx(id, data);
				if (hr != RD_OK) {
					return(hr);
				}

				i = GetFxIndex(id);
				if (i == MAXFX) {
					warning("PlayFx(%d, %d, %d, %d) - Not found", id, vol, pan, type);
					return(RDERR_FXFUCKED);
				}
				if (loop == 1)
					flagsFx[i] |= SoundMixer::FLAG_LOOP;
				else 
					flagsFx[i] &= ~SoundMixer::FLAG_LOOP;
				fxVolume[i] = vol;

				byte volume;
				int8 p;
				//	Start the sound effect playing
				if (fxMuted) {
					volume = 0;
				} else {
					volume = vol * fxVol;
				}
				p = panTable[pan + 16];
				g_engine->_mixer->playRaw(&soundHandleFx[i], bufferFx[i], bufferSizeFx[i], fxRate[i], flagsFx[i], -1, volume, p);
			}
		}
	}
	return(RD_OK);
}

int32 Sword2Sound::SetFxVolumePan(int32 id, uint8 vol, int8 pan) {
	int32 i = GetFxIndex(id);
	if (i == MAXFX)
		return RDERR_FXNOTOPEN;

	fxVolume[i] = vol;
	if (!fxMuted) {
		g_engine->_mixer->setChannelVolume(soundHandleFx[i], vol * fxVol);
		g_engine->_mixer->setChannelPan(soundHandleFx[i], panTable[pan + 16]);
	}
	return RD_OK;
}

int32 Sword2Sound::SetFxIdVolume(int32 id, uint8 vol) {
	int32 i = GetFxIndex(id);
	if (i == MAXFX)
		return RDERR_FXNOTOPEN;

	fxVolume[i] = vol;
	if (!fxMuted) {
		g_engine->_mixer->setChannelVolume(soundHandleFx[i], vol * fxVol);
	}
	return RD_OK;
}

int32 Sword2Sound::ClearAllFx(void) {
	int i;

	if (!soundOn)
		return(RD_OK);

	i = 0;
	while (i < MAXFX) {
		if ((fxId[i]) && (fxId[i] != (int32) 0xfffffffe) && (fxId[i] != (int32) 0xffffffff)) {
			g_engine->_mixer->stopHandle(soundHandleFx[i]);
			fxId[i] = 0;
			fxiPaused[i] = 0;
			if (bufferFx[i] != NULL) {
				free(bufferFx[i]);
				bufferFx[i] = NULL;
			}
			bufferSizeFx[i] = 0;
			flagsFx[i] = 0;
		}
		i++;
	}
	return(RD_OK);
}

int32 Sword2Sound::CloseFx(int32 id) {
	int i;

	if (!soundOn)
		return(RD_OK);

	i = GetFxIndex(id);
	if (i < MAXFX) {
		g_engine->_mixer->stopHandle(soundHandleFx[i]);
		fxId[i] = 0;
		fxiPaused[i] = 0;
		if (bufferFx[i] != NULL) {
			free(bufferFx[i]);
			bufferFx[i] = NULL;
		}
		bufferSizeFx[i] = 0;
		flagsFx[i] = 0;
	}
	return(RD_OK);
}

int32 Sword2Sound::PauseFx(void) {
	int i;

	if (!fxPaused) {
		for (i = 0; i < MAXFX; i++) {
			if (fxId[i]) {
				g_engine->_mixer->pauseHandle(soundHandleFx[i], true);
				fxiPaused[i] = 1;
			} else {
				fxiPaused[i] = 0;
			}
		}
		fxPaused = 1;
	}
	return (RD_OK);
}

int32 Sword2Sound::PauseFxForSequence(void) {
	int i;

	if (!fxPaused) {
		for (i = 0; i<MAXFX; i++) {
			if ((fxId[i]) && (fxId[i] != (int32) 0xfffffffe)) {
				g_engine->_mixer->pauseHandle(soundHandleFx[i], true);
				fxiPaused[i] = 1;
			} else {
				fxiPaused[i] = 0;
			}
		}
		fxPaused = 1;
	}
	return (RD_OK);
}

int32 Sword2Sound::UnpauseFx(void) {
	int i;

	if (fxPaused) {
		for (i = 0; i < MAXFX; i++) {
			if (fxiPaused[i] && fxId[i]) {
				g_engine->_mixer->pauseHandle(soundHandleFx[i], false);
			}
		}
		fxPaused = 0;
	}
	return (RD_OK);
}

uint8 Sword2Sound::GetFxVolume() {
	return fxVol;
}

void Sword2Sound::SetFxVolume(uint8 volume) {
	int32 fxi;
	fxVol = volume;

	// Now update the volume of any fxs playing
	for (fxi = 0; fxi < MAXFX; fxi++) {
		if (fxId[fxi] && !fxMuted) {
			g_engine->_mixer->setChannelVolume(soundHandleFx[fxi], fxVolume[fxi] * fxVol);
		}
	}
}

void Sword2Sound::MuteFx(uint8 mute) {
	int32 fxi;

	fxMuted = mute;

	// Now update the volume of any fxs playing
	for (fxi = 0; fxi < MAXFX; fxi++) {
		if (fxId[fxi]) {
			if (mute) {
				g_engine->_mixer->setChannelVolume(soundHandleFx[fxi], 0);
			} else {
				g_engine->_mixer->setChannelVolume(soundHandleFx[fxi], fxVolume[fxi] * fxVol);
			}
		}
	}
}

uint8 Sword2Sound::IsFxMute(void) {
	return (fxMuted);
}

int32 Sword2Sound::StreamCompMusic(const char *filename, uint32 musicId, int32 looping) {
	StackLock lock(_mutex);
	return StreamCompMusicFromLock(filename, musicId, looping);
}

int32 Sword2Sound::StreamCompMusicFromLock(const char *filename, uint32 musicId, int32 looping) {
	int32 primaryStream = -1;
	int32 secondaryStream = -1;
	int32 i;
	int32 v0, v1;
	uint16 *data16;
	uint8 *data8;

	compressedMusic = 1;

	// If both music streams are playing, that should mean one of them is
	// fading out. Pick that one.

	if (musStreaming[0] && musStreaming[1]) {
		if (musFading[0])
			primaryStream = 0;
		else
			primaryStream = 1;

		musFading[primaryStream] = 0;
		g_engine->_mixer->stopHandle(soundHandleMusic[primaryStream]);
		musStreaming[primaryStream] = 0;
	}

	// Pick the available music stream. If no music is playing it doesn't
	// matter which we use, so pick the first one.

	if (musStreaming[0] || musStreaming[1]) {
		if (musStreaming[0]) {
			primaryStream = 1;
			secondaryStream = 0;
		} else {
			primaryStream = 0;
			secondaryStream = 1;
		}
	} else
		primaryStream = 0;

	strcpy(musFilename[primaryStream], filename);

	// Save looping info and tune id
	musLooping[primaryStream] = looping;
	musId[primaryStream] = musicId;

	// Don't start streaming if the volume is off.
	if (IsMusicMute())
		return RD_OK;

	// Always use fpMus[0] (all music in one cluster)
	// musFilePos[primaryStream] for different pieces of music.
	if (!fpMus.isOpen())
		fpMus.open(filename, g_engine->getGameDataPath());

	if (!fpMus.isOpen())
		return RDERR_INVALIDFILENAME;

	// Start other music stream fading out
	if (secondaryStream != -1 && !musFading[secondaryStream])
		musFading[secondaryStream] = -16;

	fpMus.seek((musicId + 1) * 8, SEEK_SET);
	musFilePos[primaryStream] = fpMus.readUint32LE();
	musEnd[primaryStream] = fpMus.readUint32LE();

	if (!musEnd[primaryStream] || !musFilePos[primaryStream]) {
		return RDERR_INVALIDID;
	}

	// Calculate the file position of the end of the music
	musEnd[primaryStream] += musFilePos[primaryStream];

	// Create a temporary buffer
	data8 = (uint8*) malloc(bufferSizeMusic / 2);
	if (!data8) {
		return RDERR_OUTOFMEMORY;
	}

	// Seek to start of the compressed music
	fpMus.seek(musFilePos[primaryStream], SEEK_SET);

	// Read the compressed data in to the buffer
	if ((int32) fpMus.read(data8, bufferSizeMusic / 2) != bufferSizeMusic / 2) {
		free(data8);
		return RDERR_INVALIDID;
	}

	// Store the current position in the file for future streaming
	musFilePos[primaryStream] = fpMus.pos();

	// FIXME: We used this decompression function in several places, so
	// it really should be a separate function.

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
	musLastSample[primaryStream] = data16[i - 1];

	// Free the compressed sound buffer
	free(data8);

	// Modify the volume according to the master volume and music mute
	// state
	if (musicMuted)
		v0 = v1 = 0;
	else {
		v0 = volMusic[0];
		v1 = volMusic[1];
	}

	byte volume;
	int8 pan;

	if (v0 > v1) {
		volume = musicVolTable[v0];
		pan = (musicVolTable[v1 * 16 / v0] / 2) - 127;
	} else if (v1 > v0) {
		volume = musicVolTable[v1];
		pan = (musicVolTable[v0 * 16 / v1] / 2) + 127;
	} else {
		volume = musicVolTable[v1];
		pan = 0;
	}

#ifndef SCUMM_BIG_ENDIAN
	// FIXME: Until the mixer supports LE samples natively, we need to
	// convert our LE ones to BE
	for (i = 0; i < (bufferSizeMusic / 2); i++) {
		data16[i] = SWAP_BYTES_16(data16[i]);
	}
#endif

	g_engine->_mixer->newStream(&soundHandleMusic[primaryStream], data16,
		bufferSizeMusic, 22050, SoundMixer::FLAG_16BITS, 100000, volume, pan);
		
	free(data16);

	// Recorder some last variables
	musStreaming[primaryStream] = 1;
	return RD_OK;
}

void Sword2Sound::UpdateCompSampleStreaming(void) {
	uint32 	i,j;
	int32 	v0, v1;
	int32	len;
	uint16 *data16;
	uint8  *data8;
	int     fade;

	for (i = 0; i < MAXMUS; i++) {
		if (!musStreaming[i])
			continue;

		// If the music is fading, adjust the volume for it.

		if (musFading[i] < 0) {
			if (++musFading[i] == 0) {
				g_engine->_mixer->stopHandle(soundHandleMusic[i]);
				musStreaming[i] = 0;
				musLooping[i] = 0;
				continue;
			}

    			// Modify the volume according to the master volume
			// and music mute state
			if (musicMuted)
				v0 = v1 = 0;
			else {
				v0 = (volMusic[0] * (0 - musFading[i]) / 16);
	    			v1 = (volMusic[1] * (0 - musFading[i]) / 16);
			}

			byte volume;
			int8 pan;

			if (v0 > v1) {
				volume = musicVolTable[v0];
				pan = (musicVolTable[v1 * 16 / v0] / 2) - 127;
			}
			if (v1 > v0) {
				volume = musicVolTable[v1];
				pan = (musicVolTable[v0 * 16 / v1] / 2) + 127;
			} else {
				volume = musicVolTable[v1];
				pan = 0;
			}
			g_engine->_mixer->setChannelVolume(soundHandleMusic[i], volume);
			g_engine->_mixer->setChannelPan(soundHandleMusic[i], pan);
		}

		// Re-fill the audio buffer.

		len = bufferSizeMusic;

		// Reduce length if it requires reading past the end of the
		// music

		if (musFilePos[i] + len >= musEnd[i]) {
			// End of music reached so we'll need to fade and
			// repeat
			len = musEnd[i] - musFilePos[i];
			fade = 1;
		} else
			fade = 0;

		if (len > 0) {
			data8 = (uint8*) malloc(len / 2);
			// Allocate a compressed data buffer
			if (data8 == NULL) {
				g_engine->_mixer->stopHandle(soundHandleMusic[i]);
				musStreaming[i] = 0;
				musLooping[i] = 0;
				continue;
			}

			// Seek to update position of compressed music when
			// neccassary (probably never occurs)
			if ((int32) fpMus.pos() != musFilePos[i])
				fpMus.seek(musFilePos[i], SEEK_SET);

			// Read the compressed data in to the buffer
			if ((int32) fpMus.read(data8, len / 2) != (len / 2)) {
				g_engine->_mixer->stopHandle(soundHandleMusic[i]);
				free(data8);
				musStreaming[i] = 0;
				musLooping[i] = 0;
				continue;
			}

			// Update the current position in the file for future
			// streaming

			musFilePos[i] = fpMus.pos();

			// decompress the music into the music buffer.
			data16 = (uint16*) malloc(len);

			// Decompress the first byte using the last
			// decompressed sample
			if (GetCompressedSign(data8[0]))
				data16[0] = musLastSample[i] - (GetCompressedAmplitude(data8[0]) << GetCompressedShift(data8[0]));
			else
				data16[0] = musLastSample[i] + (GetCompressedAmplitude(data8[0]) << GetCompressedShift(data8[0]));

			j = 1;

			while (j < (uint32)len / 2) {
				if (GetCompressedSign(data8[j]))
					data16[j] = data16[j - 1] - (GetCompressedAmplitude(data8[j]) << GetCompressedShift(data8[j]));
				else
					data16[j] = data16[j - 1] + (GetCompressedAmplitude(data8[j]) << GetCompressedShift(data8[j]));
				j++;
			}

			musLastSample[i] = data16[j - 1];

#ifndef SCUMM_BIG_ENDIAN
			// Until the mixer supports LE samples natively, we
			// need to convert our LE ones to BE
			for (int32 y = 0; y < (len / 2); y++)
				data16[y] = SWAP_BYTES_16(data16[y]);
#endif

			// Paranoid check that seems to be necessary.
			if (len & 1)
				len--;

			g_engine->_mixer->appendStream(soundHandleMusic[i], data16, len);
					
			free(data16);
			free(data8);
		}

		// End of the music so we need to start fading and start the
		// music again
		if (fade) {
			g_engine->_mixer->stopHandle(soundHandleMusic[i]);

			// FIXME: The original code faded the music here, but
			// to do that we need to start before we reach the end
			// of the file.

			// Fade the old music
			// musFading[i] = -16;

			// Loop if neccassary
			if (musLooping[i])
				StreamCompMusicFromLock(musFilename[i], musId[i], musLooping[i]);
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

	int i;

	for (i = 0; i < MAXMUS && !musStreaming[i]; i++) {
		// this is meant to be empty! (James19aug97)
	}

	if (i == MAXMUS)
		return 0;

	return (musEnd[i] - musFilePos[i]) / 22050;
}

void Sword2Sound::StopMusic(void) {
	StackLock lock(_mutex);

	int i;

	for (i = 0; i < MAXMUS; i++) {
		if (musStreaming[i])
			musFading[i] = -16;
		else
			musLooping[i] = 0;
	}
}

int32 Sword2Sound::PauseMusic(void) {
	StackLock lock(_mutex);

	int i;

	if (soundOn) {
		for (i = 0; i < 2; i++) {
			if (musStreaming[i]) {
				musicPaused[i] = TRUE;
				g_engine->_mixer->pauseHandle(soundHandleMusic[i], true);
			} else {
				musicPaused[i] = FALSE;
			}
		}
	}
	return(RD_OK);
}

int32 Sword2Sound::UnpauseMusic(void) {
	StackLock lock(_mutex);

	int i;

	if (soundOn) {
		for (i = 0; i < 2; i++) {
			if (musicPaused[i]) {
				g_engine->_mixer->pauseHandle(soundHandleMusic[i], false);
				musicPaused[i] = FALSE;
			}
		}
	}
	return(RD_OK);
}

void Sword2Sound::SetMusicVolume(uint8 volume) {
	StackLock lock(_mutex);

	int i;

	for (i = 0; i < MAXMUS; i++) {
		volMusic[i] = volume;
		if (musStreaming[i] && !musFading[i] && !musicMuted) {
			g_engine->_mixer->setChannelVolume(soundHandleMusic[i], musicVolTable[volume]);
		}
	}
}

uint8 Sword2Sound::GetMusicVolume() {
	return (uint8) volMusic[0];
}

void Sword2Sound::MuteMusic(uint8 mute) {
	StackLock lock(_mutex);

	int i;

	musicMuted = mute;

	for (i = 0; i < MAXMUS; i++) {
		if (!mute) {
			if (!musStreaming[i] && musLooping[i])
				StreamCompMusicFromLock(musFilename[i], musId[i], musLooping[i]);
		}

		if (musStreaming[i] && !musFading[i]) {
			if (mute) {
				g_engine->_mixer->setChannelVolume(soundHandleMusic[i], musicVolTable[0]);
			} else {
				g_engine->_mixer->setChannelVolume(soundHandleMusic[i], musicVolTable[volMusic[i]]);
			}
		}
	}
}

uint8 Sword2Sound::IsMusicMute(void) {
	return (musicMuted);
}
