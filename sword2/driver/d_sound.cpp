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
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		03-Dec-96	PRP		The sound buffer can be created, with the
//								format defined by the game engine, and speech
//								can be played.
//
//	1.1		05-Dec-96	PRP		Sound effects now done.
//
//	1.2		19-Dec-96	PRP		Added volume and pan to speech and sound
//								effects.  Also, added type to sound effects
//								so that they can be looped.  Implemented
//								a CloseAllFx function which will clear out
//								all sound effects.
//
//	1.3		20-Dec-96	PRP		Fixed a bug in the function which clears
//								spot effects when they have finished playing.
//
//	1.4		02-Jan-97	PRP		Fixed a bug in ClearAllFx which was trying
//								to close the speech.
//
//	1.5		08-Apr-97	PRP		Added ... to the
//								InitialiseSound function.
//
//	1.6		09-Apr-97	PRP		Added functions to steam music from CD.
//
//	1.7		29-May-97	PSJ		Added functions to save and restore the state
//								of the sound drivers.
//
//	1.8		04-Jun-97	PRP		Added bodge to PlayFx routine which registers
//								a sound effect to remove itself from the list
//								if it is the tune to leave a sequence.
//
//	1.9		06-Jun-97	PSJ		Expanded volTable from 17 to 241 entries.
//								Added an fx and a speech master volume level.
//								Added SetFxVolume and GetFxVolume for fx master
//								volume. Added SetSpeechVolume and GetSpeechVolume
//								for speech master volume.
//
//	1.10	09-Jun-97	PSJ		Added SetMusicVolume and GetMusicVolume.
//
//	1.11	09-Jun-97	PSJ		Fixed bug in SetSpeechVolume.
//
//	1.12	10-Jun-97	PSJ		Added MuteMusic, MuteSpeech, MuteFx, IsMusicMute,
//								IsFxMute and IsSpeechMute.
//
//	1.13	12-Jun-97	PSJ		Added PlayCompSpeech to play compressed speech
//								from a speech cluster.
//
//	1.14	19-Jun-97	PSJ		Added StreamCompMusic and UpdateCompSampleStreaming
//								to play compressed music from a music cluster.
//								Added StopMusic to fade out any music playing.
//
//	1.15	24-Jun-97	PSJ		Changed PlayCompSpeech to physically check for
//								playing samples rather than using the assuming the
//								speechStatus flag is correct.
//
//	1.16	24-Jun-97	PSJ		Fixed bug it SetSpeechVolume.
//
//	1.17	26-Jun-97	PSJ		Added AmISpeaking() for lip syncing.
//
//	1.18	26-Jun-97	PSJ		Tweaked the nose of the dread, killer AmISpeaking
//								function.
//
//	1.19	26-Jun-97	PSJ		Added PauseSpeech and UnpauseSpeech.
//
//	1.20	26-Jun-97	PSJ		Fixed a bug in the muteSpeech routine.
//
//	1.21	26-Jun-97	PSJ		Fixed a bug in the AmISpeaking routine.
//
//	1.22	26-Jun-97	PSJ		PlayCompSpeech loads and pauses the speech
//								ready to be played by UnpauseSpeech.
//
//	1.23	01-Jul-97	PSJ		Fixed GetSpeechStatus to work when speech is paused
//
//	1.24	03-Jul-97	PSJ		Stopped PlayCompSpeech clicking at the end of samples.
//
//	1.25	10-Jul-97	PSJ		Reduced music volume by 1/4 when playing speech
//
//	1.26	10-Jul-97	PSJ		GetMusicVolume return safeMusicVol if it is set.
//
//	1.27	15-Jul-97	PRP		Added functions to pause and unpause the sound effects.
//
//	1.28	15-Jul-97	PRP		Fixed PauseFx
//
//	1.29	16-Jul-97	PSJ		Added GetCompSpeechSize and PreFetchCompSpeech
//
//	1.30	16-Jul-97	PRP		Fixed setting of sound fx volume.
//
//	1.31	18-Jul-97	PRP		Added speech expansion to get samples to sound the same.
//
//	1.32	18-Jul-97	PRP		Hopefully fixed expansion algorithm.
//
//	1.33	18-Jul-97	JEL		Fixed UnpauseFx()
//
//	1.34	18-Jul-97	JEL		Fixed PlayCompSpeech()
//
//	1.35	18-Jul-97	JEL		Removed speech volume enhancing (now to be done in speech compressor)
//
//	1.36	21-Jul-97	PRP		Added new type of sound effect which is the music lead in.
//								Also, added function to pause the sound effects
//								just for sequences.
//
//	1.37	21-Jul-97	PRP		Modified ClearAllFx so that it doesn't kick out
//								lead in and lead out music for smacker sequences.
//
//	1.38	21-Jul-97	PRP		Tried to fix the bug where the second lead in
//								music will not play due to a duplicate id.
//
//	1.39	21-Jul-97	PRP		Finally fixed the bug to kick out lead in music
//								fx when they have finished.
//
//	1.40	25-Jul-97	JEL		Fixed crashing when music paused & unpaused repeatedly
//
//	1.41	28-Jul-97	PRP		Checked to see if fx are looping as well as playing!
//
//	1.42	30-Jul-97	PSJ		Added Music dipping.
//
//	1.43	30-Jul-97	PSJ		Added MusicTimeRemaining.
//
//	1.44	31-Jul-97	PSJ		Adjusted MusicTimeRemaining to include music left in buffer.
//
//	1.45	06-Aug-97	PSJ		Updated Get and Set scroll SoundStatus.
//
//	1.46	12-Aug-97	PSJ		Added ReverseStereo(void)
//
//	1.47	13-Aug-97	PSJ		Updated DipMusic so it fades up after speech has finished.
//
//	1.48	13-Aug-97	PRP		Added IsFxOpen().
//
//	1.49	15-Aug-97	PRP		Added SetFxVolumePan().
//
//	1.50	15-Aug-97	PRP		Added SetFxIdVolume()
//
//	1.51	15-Aug-97	PSJ		Fixed bug in PlayCompMusic();
//
//	1.52	19-Aug-97	JEL		Fixed bug in MusicTimeRemaining()
//
//	WE'VE SCREWED UP THE NUMBERING!
//
//	1.59	19-Aug-97	JEL		Fixed bug in MusicTimeRemaining(),  ;)
//
//	1.60	19-Aug-97	PSJ		Updated DipMusic so it fades music a bit more.
//
//	1.61	21-Aug-97	PSJ		Updated StreamCompMusic so if both streams are in use,
//								the fading stream is stopped and the new tune started.
//
//	1.62	21-Aug-97	PSJ		Updated StreamCompMusic so if the music is unmuted,
//								the last tune is restarted if it was looping.
//
//	1.63	22-Aug-97	PSJ		Update PlayFx to handle smacker leadouts.
//
//	1.64	27-Aug-97	PSJ		Update PlayFx to record an fx's local volume,
//								So SetFxVolume can update playing fx's with the
//								correct volume.
//
//	1.65	27-Aug-97	PSJ		Stopped CloseFX from closing invalid fx's.
//
//	1.66	01-Sep-97	PRP		Cleared the fxPaused flag when closing fx.
//
//	1.67	01-Sep-97	PRP		Fixed the fact that SetFxVolume was still
//								being done even if the fx were muted.
//
//	1.68	01-Sep-97	PRP		Set zero sound to -10000
//
//	Functions
//	---------
//
//	--------------------------------------------------------------------------
//
//	int32 InitialiseSound(uint16 freq, uint16 channels, uint16 bitDepth)
//
//	This function initialises DirectSound by specifying the parameters of the
//	primary buffer.
//
//	Freq is the sample rate - 44100, 22050 or 11025
//	Channels should be 1 for mono, 2 for stereo
//	BitDepth should be either 8 or 16 bits per sample.	
//
//	--------------------------------------------------------------------------
//
//	int32 PlaySpeech(uint8 *data, uint8 vol, int8 pan)
//
//	This function plays the wav file passed into it as speech.  An error occurs
//	if speech is already playing, or directSound comes accross problems.  The
//	volume is 0 for zero volume and 16 for maximum volume.  The pan position
//	is -16 for full left, 0 for central and 16 for full right.
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
//	int32 StreamMusic(uint8 *filename, int32 loopFlag)
//
//	Streams music from the file defined by filename.  The loopFlag should
//	be set to RDSE_FXLOOP if the music is to loop back to the start.
//	Otherwise, it should be RDSE_FXSPOT.
//	The return value must be checked for any problems.
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
//  StreamCompMusic should not be used inconjunction with StreamMusic.
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




#define WIN32_LEAN_AND_MEAN

//#include <windows.h>
//#include <windowsx.h>
#include <stdio.h>

#include "stdafx.h"
#include "driver96.h"
#include "rdwin.h"			// for hwnd.
#include "d_sound.h"
#include "../sword2.h"

// Decompression macros
#define MakeCompressedByte(shift,sign,amplitude) (((shift)<<4) + ((sign)<<3) + (amplitude))
#define GetCompressedShift(byte)                 ((byte)>>4)
#define GetCompressedSign(byte)                  (((byte)>>3) & 1)
#define GetCompressedAmplitude(byte)             ((byte) & 7)
#define GetdAPower(dA,power)                     for (power = 15;power>0 && !((dA) & (1<<power)); power--)

/*
LPDIRECTSOUND				lpDS;
LPDIRECTSOUNDBUFFER			dsbPrimary;
LPDIRECTSOUNDBUFFER			dsbSpeech;
LPDIRECTSOUNDBUFFER			dsbFx[MAXFX];
*/

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

int32 musicVolTable[17] = {
  -10000, 
  -5000, -3000, -2500, -2250, 
  -2000, -1750, -1500, -1250, 
  -1000, -750, -500, -350, 
  -200, -100, -50, 0
};



Sword2Sound::Sword2Sound(SoundMixer *mixer) {

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

	_mixer = mixer;
}

/*  not used seemingly - khalek

#define SPEECH_EXPANSION

#ifdef SPEECH_EXPANSION

int16 ExpandSpeech(int16 sample)
//	This code is executed to expand the speech samples to make them sound
//	louder, without losing the quality of the sample
{
	double x, xsquared, result;
	double expansionFactor = 2.5;

	x = (double) sample;
	xsquared = sample * sample;

	if (x < 0.0)
	{
		result = expansionFactor * x + (expansionFactor - 1.0) * xsquared / 32768.0;
		if (result < -32767.0)
			result = -32767.0;
	}
	else
	{
		result = expansionFactor * x + (1.0 - expansionFactor) * xsquared / 32768.0;
		if (result > 32767.0)
			result = 32767.0;
	}
	
	return (int16) result;

}
#endif
*/

//	--------------------------------------------------------------------------
//	This function reverse the pan table, thus reversing the stereo.
//	--------------------------------------------------------------------------
int32 Sword2Sound::ReverseStereo(void)
{
	int32 i,j;

	for (i = 0; i<16; i++)
	{
		j = panTable[i];
		panTable[i] = panTable[32-i];
		panTable[32-i] = j;
	}

	return (RD_OK);
}



//	--------------------------------------------------------------------------
//	This function returns the index of the sound effect with the ID passed in.
//	--------------------------------------------------------------------------
int32 Sword2Sound::GetFxIndex(int32 id)

{

	int32 i = 0;

	while (i < MAXFX)
	{
		if (fxId[i] == id)
			break;
		i++;
	}

	return(i);

}


int32 Sword2Sound::IsFxOpen(int32 id)
{

	int32 i = 0;

	while (i < MAXFX)
	{
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
void Sword2Sound::FxServer(void)

{
	warning("stub FxServer");
/*
	int32 i;
	int32 status;


	if (!soundOn)
		return;

	if (musicPaused[0] + musicPaused[1] == 0)
	{
		if (compressedMusic == 1)
			UpdateCompSampleStreaming();
		else if (compressedMusic == 2)
			UpdateSampleStreaming();
	}

	 if (fxPaused)
	 {
		for (i=0; i<MAXFX; i++)
		{
			if ((fxId[i] == 0xfffffffe) || (fxId[i] == 0xffffffff))
			{
				IDirectSoundBuffer_GetStatus(dsbFx[i], &status);
				if (!(status & (DSBSTATUS_PLAYING + DSBSTATUS_LOOPING)))
				{
					if (fxCached[i] == RDSE_FXTOCLEAR)
					{
						IDirectSoundBuffer_Release(dsbFx[i]);
						fxId[i] = 0;
					}
				}
			}
		}
		return;
	 }


	for (i=0; i<MAXFX; i++)
	{
		if (fxId[i])
		{
			IDirectSoundBuffer_GetStatus(dsbFx[i], &status);
			if (!(status & (DSBSTATUS_PLAYING + DSBSTATUS_LOOPING)))
			{
				if (fxCached[i] == RDSE_FXTOCLEAR)
				{
					IDirectSoundBuffer_Release(dsbFx[i]);
					fxId[i] = 0;
				}
			}
		}
	}
*/
}





int32 Sword2Sound::InitialiseSound(uint16 freq, uint16 channels, uint16 bitDepth)

{
	warning("stub InitaliseSound( %d, %d, %d )", freq, channels, bitDepth);

	memset(fxId,		0, sizeof(fxId));
	memset(fxCached,	0, sizeof(fxCached));
	memset(fxiPaused,	0, sizeof(fxiPaused));
	memset(fxLooped, 	0, sizeof(fxLooped));

 	memset(musStreaming,	0, sizeof(musStreaming));
	memset(musicPaused,	0, sizeof(musicPaused));
	memset(musCounter, 	0, sizeof(musCounter));
	memset(musFading, 	0, sizeof(musFading));

	memset(musLooping,	0, sizeof(musLooping));

	memset(streamCursor,	0, sizeof(streamCursor));
	memset(musFilePos,	0, sizeof(musFilePos));
	memset(musEnd,		0, sizeof(musEnd));
	memset(musLastSample,	0, sizeof(musLastSample));
	memset(musId,		0, sizeof(musId));

/*
	int32			i;
	HRESULT			hrz;
	DSBUFFERDESC	dsbd;
	WAVEFORMATEX	pf;


	hrz = DirectSoundCreate(NULL, &lpDS, NULL);
	if (hrz != DS_OK)
		return(RDERR_DSOUNDCREATE);

	hrz = IDirectSound_SetCooperativeLevel(lpDS, hwnd, DSSCL_EXCLUSIVE);
	if (hrz != DS_OK)
	{
		IDirectSound_Release(lpDS);
		return(RDERR_DSOUNDCOOPERATE);
	}


	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbd.lpwfxFormat = NULL;
	hrz = IDirectSound_CreateSoundBuffer(lpDS, &dsbd, &dsbPrimary, NULL);
	if (hrz != DS_OK)
	{
		IDirectSound_Release(lpDS);
		return(RDERR_DSOUNDPBUFFER);
	}

	memset(&pf, 0, sizeof(WAVEFORMATEX));
	
	pf.wFormatTag = WAVE_FORMAT_PCM;

	pf.nSamplesPerSec = freq;
	
	pf.nChannels = channels;
	
	pf.wBitsPerSample = bitDepth;
	pf.nBlockAlign = pf.wBitsPerSample * pf.nChannels >> 3;
	pf.nAvgBytesPerSec = pf.nBlockAlign * pf.nSamplesPerSec;
	pf.cbSize = 0;
	hrz = IDirectSoundBuffer_SetFormat(dsbPrimary, (LPWAVEFORMATEX) &pf);
	if (hrz != DS_OK)
	{
		// We have not been able to set the primary format to the format requested!!!
		//	But carry on anyway, the mixer will just have to work harder :)
	}

	// Clear the fx id's
	for (i=0; i<MAXFX; i++)
		fxId[i] = 0;
	*/
	
	soundOn = 1;
	
	/*
	//----------------------------------
	// New initialisers (James19aug97)

	memset (fxId,			0, MAXFX*sizeof(int32));
	memset (fxCached,		0, MAXFX*sizeof(uint8));
	memset (fxiPaused,		0, MAXFX*sizeof(uint8));
	memset (fxLooped, 		0, MAXFX*sizeof(uint8));

 	memset (musStreaming,	0, MAXFX*sizeof(int16));
	memset (musicPaused,	0, MAXFX*sizeof(int16));
	memset (musCounter, 	0, MAXFX*sizeof(int16));
	memset (musFading, 		0, MAXFX*sizeof(int16));

	memset (musLooping,		0, MAXFX*sizeof(int16));
	memset (fpMus,			0, MAXFX*sizeof(FILE*));

	memset (streamCursor,	0, MAXFX*sizeof(int32));
	memset (musFilePos,		0, MAXFX*sizeof(int32));
	memset (musEnd,			0, MAXFX*sizeof(int32));
	memset (musLastSample,	0, MAXFX*sizeof(int16));
	memset (musId,			0, MAXFX*sizeof(uint32));
*/
	return(RD_OK);

}


int32 Sword2Sound::PlaySpeech(uint8 *data, uint8 vol, int8 pan)

{
	warning("stub PlaySpeech");
/*
	uint32			dwBytes1, dwBytes2;
  	int32 			i;
	uint32			*data32;
	void 			*lpv1, *lpv2;
	_wavHeader		*wav;
	HRESULT 		hr;
	DSBUFFERDESC	dsbd;
	PCMWAVEFORMAT	wf;


	wav = (_wavHeader *) data;

	if (soundOn)
	{
		if (speechStatus)
			return(RDERR_SPEECHPLAYING);

		memset(&wf, 0, sizeof(PCMWAVEFORMAT));
		wf.wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.wf.nChannels = wav->channels;
		wf.wf.nSamplesPerSec = wav->samplesPerSec;
		wf.wBitsPerSample = 8 * wav->blockAlign / (wav->samplesPerSec * wav->channels);
		wf.wf.nBlockAlign = wf.wf.nChannels * wf.wBitsPerSample / 8;
		wf.wf.nAvgBytesPerSec = wf.wf.nSamplesPerSec * wf.wf.nBlockAlign;
   
		memset(&dsbd, 0, sizeof(DSBUFFERDESC));
		dsbd.dwSize = sizeof(DSBUFFERDESC);
	//	dsbd.dwFlags = DSBCAPS_CTRLDEFAULT;
		dsbd.lpwfxFormat = (LPWAVEFORMATEX) &wf;
	
	    //  Set the sample size - search for the size of the data.
		i = 0;
		while (i<100)
		{
			if (*data == 'd')
			{
				data32 = (int32 *) data;
				if (*data32 == 'atad')
					break;
			}
			i += 1;
			data++;
		}
		if (i == 100)
			return(RDERR_INVALIDWAV);

		dsbd.dwBufferBytes = *(data32 + 1);

		//	Create the speech sample buffer
		hr = IDirectSound_CreateSoundBuffer(lpDS, &dsbd, &dsbSpeech, NULL);
		if (hr != DS_OK)
			return(RDERR_CREATESOUNDBUFFER);

		//	Lock the speech buffer, ready to fill it with data
		hr = IDirectSoundBuffer_Lock(dsbSpeech, 0, dsbd.dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
		if (hr == DSERR_BUFFERLOST)
		{
			IDirectSoundBuffer_Restore(dsbSpeech);
			hr = IDirectSoundBuffer_Lock(dsbSpeech, 0, dsbd.dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
		}

		if (hr == DS_OK)
		{
			//	Fill the speech buffer with data
			memcpy((uint8 *) lpv1, (uint8 *) (data32 + 2), dwBytes1);

			if (dwBytes1 != dsbd.dwBufferBytes)
			{
				memcpy((uint8 *) lpv1 + dwBytes1, (uint8 *) (data32 + 2) + dwBytes1, dwBytes2);
			}

			//	Unlock the buffer now that we've filled it
			IDirectSoundBuffer_Unlock(dsbSpeech, lpv1, dwBytes1, lpv2, dwBytes2);

			//  Modify the volume according to the master volume
			if (speechMuted)
				IDirectSoundBuffer_SetVolume(dsbSpeech, volTable[0]);
			else
				IDirectSoundBuffer_SetVolume(dsbSpeech, volTable[vol*speechVol]);

			IDirectSoundBuffer_SetPan(dsbSpeech, panTable[pan+16]);

			//	Start the speech playing
			IDirectSoundBuffer_Play(dsbSpeech, 0, 0, 0);
			speechStatus = 1;

		}
		else
		{
			IDirectSoundBuffer_Release(dsbSpeech);
			return(RDERR_LOCKSPEECHBUFFER);
		}
	}
*/
	return(RD_OK);

}


int32 Sword2Sound::AmISpeaking()
{
	warning("stub AmISpeaking");
/*
	int32	len;
//	int32 status;
	int32 	readCursor, writeCursor;
	int32	dwBytes1, dwBytes2;
	int16  *sample;
	int32   count = 0;
	LPVOID 	lpv1, lpv2;
	HRESULT	hr;

#define POSITIVE_THRESHOLD 350
#define NEGATIVE_THRESHOLD -350
	if ((!speechMuted) && (!speechPaused) && (dsbSpeech))
	{
		if (IDirectSoundBuffer_GetCurrentPosition(dsbSpeech, &readCursor, &writeCursor) != DS_OK)
		{
			return (RDSE_SPEAKING);
		}

		len = 44100 / 12;

		hr = IDirectSoundBuffer_Lock(dsbSpeech, readCursor, len, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
		if (hr == DS_OK)
		{
			for (sample = (int16*)lpv1; sample<(int16*)((int8*)lpv1+dwBytes1); sample+= 90)  // 20 samples
				if (*sample>POSITIVE_THRESHOLD || *sample<NEGATIVE_THRESHOLD)
					count++;

			IDirectSoundBuffer_Unlock(dsbSpeech,lpv1,dwBytes1,lpv2,dwBytes2);

			if (count>5)		// 25% of the samples
				return (RDSE_SPEAKING);
		}
		return (RDSE_QUIET);
	}
	return (RDSE_SPEAKING);
*/
	return RDSE_QUIET;
}


int32 Sword2Sound::GetCompSpeechSize(const char *filename, uint32 speechid)
{
  	int32 			i;
	uint32			speechIndex[2];
	FILE		   *fp;
	
    //  Open the speech cluster and find the data offset & size
	fp = fopen(filename, "rb");
	if (fp == NULL)
		return(0);

	if (fseek(fp, (++speechid)*8, SEEK_SET))
	{
		fclose(fp);
		return (0);
	}

	if (fread(speechIndex, sizeof(uint32), 2, fp) != 2)
	{
		fclose(fp);
		return (0);
	}

	if (!speechIndex[0] || !speechIndex[1])
	{
		fclose(fp);
		return (0);
	}

	fclose(fp);

	i = (speechIndex[1]-1)*2 + sizeof(_wavHeader) + 8;
	
	return(i);
}


int32 Sword2Sound::PreFetchCompSpeech(const char *filename, uint32 speechid, uint8 *waveMem)
{
  	uint32 			i;
	uint16			*data16;
	uint8			*data8;
	uint32			speechIndex[2];
	_wavHeader		*pwf = (_wavHeader *) waveMem;
	FILE		   *fp;

	//  Open the speech cluster and find the data offset & size
	fp = fopen(filename, "rb");
	if (fp == NULL)
		return(RDERR_INVALIDFILENAME);

	if (fseek(fp, (++speechid)*8, SEEK_SET))
	{
		fclose(fp);
		return (RDERR_READERROR);
	}

	if (fread(speechIndex, sizeof(uint32), 2, fp) != 2)
	{
		fclose(fp);
		return (RDERR_READERROR);
	}

	if (!speechIndex[0] || !speechIndex[1])
	{
		fclose(fp);
		return (RDERR_INVALIDID);
	}

	data16 = (uint16*)(waveMem + sizeof(_wavHeader));

	memset(pwf, 0, sizeof(_wavHeader));

	*((uint32*)pwf->riff) = 'FFIR';
	*((uint32*)pwf->wavID) = 'EVAW';
	*((uint32*)pwf->format) = ' tmf';

	pwf->formatLen		= 0x00000010;
	pwf->formatTag		= 0x0001;
	pwf->channels		= 0x0001;
	pwf->samplesPerSec	= 0x5622;
	pwf->avgBytesPerSec = 0x0000;
	pwf->blockAlign		= 0xAC44;
	pwf->unknown1		= 0x0000;
	pwf->unknown2		= 0x0002;
	pwf->bitsPerSample	= 0x0010;

	*((uint32*)data16) = 'atad';

	data16 += 2;

	*((uint32*)data16) = (speechIndex[1]-1)*2;

	data16 += 2;

	pwf->fileLength = (speechIndex[1]-1)*2 + sizeof(_wavHeader) + 8;


	// Calculate position in buffer to load compressed sound into
	data8 = (uint8*)data16 + (speechIndex[1]-1);
	
	if (fseek(fp, speechIndex[0], SEEK_SET))
	{
		fclose(fp);
		return (RDERR_INVALIDID);
	}

	if (fread(data8, sizeof(uint8), speechIndex[1], fp) != speechIndex[1])
	{
		fclose(fp);
		return (RDERR_INVALIDID);
	}

	fclose(fp);

	data16[0] = *((int16*)data8);	// Starting Value
	i=1;

	while (i<(speechIndex[1]-1))
	{
		if (GetCompressedSign(data8[i+1]))
			data16[i] = data16[i-1] - (GetCompressedAmplitude(data8[i+1])<<GetCompressedShift(data8[i+1]));
		else
			data16[i] = data16[i-1] + (GetCompressedAmplitude(data8[i+1])<<GetCompressedShift(data8[i+1]));
		i++;
	}

	return(RD_OK);
}


int32 Sword2Sound::PlayCompSpeech(const char *filename, uint32 speechid, uint8 vol, int8 pan)
{

	uint32			dwBytes1, dwBytes2;
  	uint32 			i;
	uint16			*data16;
	uint8			*data8;
	uint32			speechIndex[2];
	void 			*lpv1;
	File fp;
	uint32	bufferSize;
	
	if (!speechMuted)
	{
		if (GetSpeechStatus() == RDERR_SPEECHPLAYING)
			return RDERR_SPEECHPLAYING;

	    //  Open the speech cluster and find the data offset & size
		if (fp.open(filename, g_sword2->getGameDataPath()) == false)
			return(RDERR_INVALIDFILENAME);
	/*	FIXME ? our fseek returns void not int
		if (fp.seek((++speechid) * 8, SEEK_SET))
		{
			fp.close();
			return (RDERR_READERROR);
		}
	*/
		fp.seek((++speechid) * 8, SEEK_SET);
		
		if (fp.read(speechIndex, sizeof(uint32) * 2) != (2 * sizeof(uint32)))
		{
			fp.close();
			return (RDERR_READERROR);
		}

		if (speechIndex[0] == 0 || speechIndex[1] == 0)
		{
			fp.close();
			return (RDERR_INVALIDID);
		}

		bufferSize = (speechIndex[1] - 1) * 2;

		// Create tempory buffer for compressed speech
		if ((data8 = (uint8 *)malloc(speechIndex[1])) == NULL)
		{
			fp.close();
			return(RDERR_OUTOFMEMORY);
		}
	/* FIXME ? see above
		if (fp.seek(speechIndex[0], SEEK_SET))
		{
			fp.close();
			free(data8);
			return (RDERR_INVALIDID);
		}
	*/
		fp.seek(speechIndex[0], SEEK_SET);

		if (fp.read(data8, sizeof(uint8) * speechIndex[1]) != (speechIndex[1] * sizeof(uint8)))
		{
			fp.close();
			free(data8);
			return (RDERR_INVALIDID);
		}

		fp.close();

		//	Create the speech sample buffer
		/*
		hr = IDirectSound_CreateSoundBuffer(lpDS, &dsbd, &dsbSpeech, NULL);
		if (hr != DS_OK)
		{
			free(data8);
			return(RDERR_CREATESOUNDBUFFER);
		}

		//	Lock the speech buffer, ready to fill it with data
		hr = IDirectSoundBuffer_Lock(dsbSpeech, 0, dsbd.dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
		if (hr == DSERR_BUFFERLOST)
		{
			IDirectSoundBuffer_Restore(dsbSpeech);
			hr = IDirectSoundBuffer_Lock(dsbSpeech, 0, dsbd.dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
		}
		*/

		lpv1 = malloc(bufferSize);
		dwBytes1 = dwBytes2 = bufferSize;
		
		if (1 /*hr == DS_OK*/)
		{
			// decompress data into speech buffer.
			data16 = (uint16*)lpv1;
			
			data16[0] = *((int16*)data8);	// Starting Value
			i=1;

			while (i<dwBytes1/2)
			{
				if (GetCompressedSign(data8[i+1]))
					data16[i] = data16[i-1] - (GetCompressedAmplitude(data8[i+1])<<GetCompressedShift(data8[i+1]));
				else
					data16[i] = data16[i-1] + (GetCompressedAmplitude(data8[i+1])<<GetCompressedShift(data8[i+1]));


				i++;
			}

			if (dwBytes1 != bufferSize)
			{
				while (i<(dwBytes1+dwBytes2)/2)
				{
					if (GetCompressedSign(data8[i+1]))
						data16[i] = data16[i-1] - (GetCompressedAmplitude(data8[i+1])<<GetCompressedShift(data8[i+1]));
					else
						data16[i] = data16[i-1] + (GetCompressedAmplitude(data8[i+1])<<GetCompressedShift(data8[i+1]));


					i++;
				}
			}

			free(data8);

			//	Unlock the buffer now that we've filled it
			//IDirectSoundBuffer_Unlock(dsbSpeech, lpv1, dwBytes1, lpv2, dwBytes2);

			//  Modify the volume according to the master volume
			/*
			if (speechMuted)
				IDirectSoundBuffer_SetVolume(dsbSpeech, volTable[0]);
			else
				IDirectSoundBuffer_SetVolume(dsbSpeech, volTable[vol*speechVol]);

			IDirectSoundBuffer_SetPan(dsbSpeech, panTable[pan+16]);
			*/

			//TODO: Implement volume change + panning
			
			//	Start the speech playing
			speechPaused = 1;
//			IDirectSoundBuffer_Play(dsbSpeech, 0, 0, 0);
			
			uint32 flags = SoundMixer::FLAG_16BITS;
			flags |= SoundMixer::FLAG_AUTOFREE;

			//Until the mixer supports LE samples natively, we need to convert our LE ones to BE
			for (uint j = 0; j < (bufferSize / 2); j++)
				data16[j] = TO_BE_16(data16[j]);

			PlayingSoundHandle speechHandle = 0;
			_mixer->playRaw(&speechHandle, data16, bufferSize, 22050, flags);
			
			speechStatus = 1;
		}
		else
		{
			//IDirectSoundBuffer_Release(dsbSpeech);
			free(data8);
			return(RDERR_LOCKSPEECHBUFFER);
		}
	}

	DipMusic();

	// return read error for now so we get subtitles
	return (RDERR_READERROR);
}


int32 Sword2Sound::StopSpeechSword2(void)

{
	warning("stub StopSpeechSword2");
/*
	int32 status;


	if (!soundOn)
		return(RD_OK);
  
	if (speechStatus)
	{
		IDirectSoundBuffer_GetStatus(dsbSpeech, &status);
		if (status & DSBSTATUS_PLAYING)
		{
			IDirectSoundBuffer_Stop(dsbSpeech);
//			SetMusicVolume(GetMusicVolume());
		}
		
		IDirectSoundBuffer_Release(dsbSpeech);
		dsbSpeech = 0;
		speechStatus = 0;
		return(RD_OK);
	}
*/	
	return(RDERR_SPEECHNOTPLAYING);

}



int32 Sword2Sound::GetSpeechStatus(void)
{
	warning("stub GetSpeechStatus");
/*
	int32 status;


	if ((!soundOn) || (!speechStatus))
		return(RDSE_SAMPLEFINISHED);

	if (speechPaused)
		return(RDSE_SAMPLEPLAYING);

	IDirectSoundBuffer_GetStatus(dsbSpeech, &status);
	if (!(status & DSBSTATUS_PLAYING))
	{
		speechStatus = 0;
		IDirectSoundBuffer_Release(dsbSpeech);
		dsbSpeech = 0;
//		SetMusicVolume(GetMusicVolume());
		return(RDSE_SAMPLEFINISHED);
	}
	return(RDSE_SAMPLEPLAYING);
*/
	return RDSE_SAMPLEFINISHED;

}


void Sword2Sound::SetSpeechVolume(uint8 volume)
{
	warning("stub SetSpeechVolume");
/*
	speechVol = volume;
	if (dsbSpeech && !speechMuted && GetSpeechStatus() == RDSE_SAMPLEPLAYING)
		IDirectSoundBuffer_SetVolume(dsbSpeech, volTable[16*speechVol]);
*/
}


uint8 Sword2Sound::GetSpeechVolume()
{
	return speechVol;
}


void Sword2Sound::MuteSpeech(uint8 mute)
{
	warning("stub MuteSpeech( %d )", mute);
/*
	speechMuted = mute;

	if (GetSpeechStatus() == RDSE_SAMPLEPLAYING)
	{
		if (mute)
			IDirectSoundBuffer_SetVolume(dsbSpeech, volTable[0]);
		else
			IDirectSoundBuffer_SetVolume(dsbSpeech, volTable[16*speechVol]);
	}
*/
}


uint8 Sword2Sound::IsSpeechMute(void)
{
	return (speechMuted);
}


int32 Sword2Sound::PauseSpeech(void)
{
	warning("PauseSpeech");
/*
	if (GetSpeechStatus() == RDSE_SAMPLEPLAYING)
	{
		speechPaused = 1;
		return (IDirectSoundBuffer_Stop(dsbSpeech));
	}
*/
	return(RD_OK);
}

int32 Sword2Sound::UnpauseSpeech(void)
{
	warning("UnpauseSpeech");
/*
	if (speechPaused)
	{
		speechPaused = 0;
		return (IDirectSoundBuffer_Play(dsbSpeech, 0, 0, 0));
	}
*/
	return(RD_OK);
}


int32 Sword2Sound::OpenFx(int32 id, uint8 *data)

{
	warning("stub OpenFx( %d )", id);
/*
	uint32			dwBytes1, dwBytes2;
  	int32 			i, fxi;
	uint32			*data32;
	void 			*lpv1, *lpv2;
	_wavHeader		*wav;
	HRESULT 		hr;
	DSBUFFERDESC	dsbd;
	PCMWAVEFORMAT	wf;


	wav = (_wavHeader *) data;

	if (soundOn)
	{

		// Check for a valid id.
		if (id == 0)
			return(RDERR_INVALIDID);

		// Check that the fx is not already open
		for (i=0; i<MAXFX; i++)
			if (fxId[i] == id)
				return(RDERR_FXALREADYOPEN);

		// Now choose a free slot for the fx
		fxi = 0;
		while (fxi<MAXFX)
		{
			if (fxId[fxi] == 0)
				break;
			fxi++;
		}

		if (fxi == MAXFX)
			return(RDERR_NOFREEBUFFERS);

		memset(&wf, 0, sizeof(PCMWAVEFORMAT));
		wf.wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.wf.nChannels = wav->channels;
		wf.wf.nSamplesPerSec = wav->samplesPerSec;
		wf.wBitsPerSample = 8 * wav->blockAlign / (wav->samplesPerSec * wav->channels);
		wf.wf.nBlockAlign = wf.wf.nChannels * wf.wBitsPerSample / 8;
		wf.wf.nAvgBytesPerSec = wf.wf.nSamplesPerSec * wf.wf.nBlockAlign;
   
		memset(&dsbd, 0, sizeof(DSBUFFERDESC));
		dsbd.dwSize = sizeof(DSBUFFERDESC);
//		dsbd.dwFlags = DSBCAPS_CTRLDEFAULT;
		dsbd.lpwfxFormat = (LPWAVEFORMATEX) &wf;
	
	    //  Set the sample size - search for the size of the data.
		i = 0;
		while (i<100)
		{
			if (*data == 'd')
			{
				data32 = (int32 *) data;
				if (*data32 == 'atad')
					break;
			}
			i += 1;
			data++;
		}
		if (i == 100)
			return(RDERR_INVALIDWAV);

		dsbd.dwBufferBytes = *(data32 + 1);

		//	Create the speech sample buffer
		hr = IDirectSound_CreateSoundBuffer(lpDS, &dsbd, &dsbFx[fxi], NULL);
		if (hr != DS_OK)
			return(RDERR_CREATESOUNDBUFFER);

		//	Lock the speech buffer, ready to fill it with data
		hr = IDirectSoundBuffer_Lock(dsbFx[fxi], 0, dsbd.dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
		if (hr == DSERR_BUFFERLOST)
		{
			IDirectSoundBuffer_Restore(dsbFx[fxi]);
			hr = IDirectSoundBuffer_Lock(dsbFx[fxi], 0, dsbd.dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
		}

		if (hr == DS_OK)
		{
			//	Fill the speech buffer with data
			memcpy((uint8 *) lpv1, (uint8 *) (data32 + 2), dwBytes1);

			if (dwBytes1 != dsbd.dwBufferBytes)
			{
				memcpy((uint8 *) lpv1 + dwBytes1, (uint8 *) (data32 + 2) + dwBytes1, dwBytes2);
			}

			//	Unlock the buffer now that we've filled it
			IDirectSoundBuffer_Unlock(dsbFx[fxi], lpv1, dwBytes1, lpv2, dwBytes2);

		}
		else
		{
			IDirectSoundBuffer_Release(dsbFx[fxi]);
			return(RDERR_LOCKSPEECHBUFFER);
		}
		
		fxId[fxi] = id;
		fxCached[fxi] = RDSE_FXCACHED;

	}
*/
	return(RD_OK);

}


int32 Sword2Sound::PlayFx(int32 id, uint8 *data, uint8 vol, int8 pan, uint8 type)

{
	warning("stub PlayFx( %d, %d, %d, %d )", id, vol, pan, type);
/*
	int32 i, loop;
	HRESULT hr;

	if (type == RDSE_FXLOOP)
		loop = DSBPLAY_LOOPING;
	else
		loop = 0;

	if (soundOn)
	{
		if (data == NULL)
		{
			if (type == RDSE_FXLEADOUT)
			{
				id = 0xffffffff;
				i = GetFxIndex(id);
				if (i == MAXFX)
					return(RDERR_FXNOTOPEN);

				fxLooped[i] = 0;

				//	Start the sound effect playing
				if (musicMuted)
					IDirectSoundBuffer_SetVolume(dsbFx[i], volTable[0]);
				else
					IDirectSoundBuffer_SetVolume(dsbFx[i], musicVolTable[volMusic[0]]);
				IDirectSoundBuffer_SetPan(dsbFx[i], 0);
				IDirectSoundBuffer_Play(dsbFx[i], 0, 0, 0);

				fxCached[i] = RDSE_FXTOCLEAR;
			}
			else
			{
				i = GetFxIndex(id);
				if (i == MAXFX)
					return(RDERR_FXNOTOPEN);

				fxLooped[i] = loop;
				fxVolume[i] = vol;

				//	Start the sound effect playing
				if (fxMuted)
					IDirectSoundBuffer_SetVolume(dsbFx[i], volTable[0]);
				else
					IDirectSoundBuffer_SetVolume(dsbFx[i], volTable[vol*fxVol]);
				IDirectSoundBuffer_SetPan(dsbFx[i], panTable[pan+16]);

				IDirectSoundBuffer_Play(dsbFx[i], 0, 0, loop);
				if (id == 0xffffffff)
					fxCached[i] = RDSE_FXTOCLEAR;
			}
		}
		else
		{
			if (type == RDSE_FXLEADIN)
			{
				id = 0xfffffffe;
				hr = OpenFx(id, data);
				if (hr != RD_OK)
					return hr;
				i = GetFxIndex(id);
				if (i == MAXFX)
					return RDERR_FXFUCKED;
				fxCached[i] = RDSE_FXTOCLEAR;
				if (musicMuted)
					IDirectSoundBuffer_SetVolume(dsbFx[i], volTable[0]);
				else
					IDirectSoundBuffer_SetVolume(dsbFx[i], musicVolTable[volMusic[0]]);
				IDirectSoundBuffer_SetPan(dsbFx[i], 0);
				IDirectSoundBuffer_Play(dsbFx[i], 0, 0, 0);
			}
			else
			{
				hr = OpenFx(id, data);
				if (hr != RD_OK)
					return(hr);

				i = GetFxIndex(id);
				if (i == MAXFX)
					return(RDERR_FXFUCKED);

				fxCached[i] = RDSE_FXTOCLEAR;
				fxLooped[i] = loop;
				fxVolume[i] = vol;

				//	Start the sound effect playing
				if (fxMuted)
					IDirectSoundBuffer_SetVolume(dsbFx[i], volTable[0]);
				else
					IDirectSoundBuffer_SetVolume(dsbFx[i], volTable[vol*fxVol]);
				IDirectSoundBuffer_SetPan(dsbFx[i], panTable[pan+16]);
				IDirectSoundBuffer_Play(dsbFx[i], 0, 0, loop);
			}
		}
	}
*/
	return(RD_OK);

}


int32 Sword2Sound::SetFxVolumePan(int32 id, uint8 vol, int8 pan)
{
	warning("stub SetFxVolumePan( %d, %d, %d )", id, vol, pan);
/*
	int32 i = GetFxIndex(id);
	if (i == MAXFX)
		return RDERR_FXNOTOPEN;

	fxVolume[i] = vol;
	if (!fxMuted)
		IDirectSoundBuffer_SetVolume(dsbFx[i], volTable[vol*fxVol]);
	IDirectSoundBuffer_SetPan(dsbFx[i], panTable[pan+16]);
*/
	return RD_OK;
}

int32 Sword2Sound::SetFxIdVolume(int32 id, uint8 vol)
{
	warning("stub SetFxIdVolume( %d, %d )", id, vol);
/*
	int32 i = GetFxIndex(id);
	if (i == MAXFX)
		return RDERR_FXNOTOPEN;

	fxVolume[i] = vol;
	if (!fxMuted)
		IDirectSoundBuffer_SetVolume(dsbFx[i], volTable[vol*fxVol]);
*/
	return RD_OK;
}



int32 Sword2Sound::ClearAllFx(void)

{
	warning("stub ClearAllFx");
/*
	int32 status;
	int32 i;


	if (!soundOn)
		return(RD_OK);

	i = 0;
	while (i < MAXFX)
	{
		if ((fxId[i]) && (fxId[i] != 0xfffffffe) && (fxId[i] != 0xffffffff))
		{
			IDirectSoundBuffer_GetStatus(dsbFx[i], &status);
			if (status & (DSBSTATUS_PLAYING + DSBSTATUS_LOOPING))
			{
				IDirectSoundBuffer_Stop(dsbFx[i]);
			}
			IDirectSoundBuffer_Release(dsbFx[i]);
			fxId[i] = 0;
			fxiPaused[i] = 0;
		}
		i++;
	}

*/	
	return(RD_OK);

}


int32 Sword2Sound::CloseFx(int32 id)

{
	warning("stub CloseFx( %d )", id);
/*
	int32 i;
	int32 status;


	if (!soundOn)
		return(RD_OK);

	i = GetFxIndex(id);
	if (i<MAXFX)
	{
		IDirectSoundBuffer_GetStatus(dsbFx[i], &status);
		if (status & (DSBSTATUS_PLAYING + DSBSTATUS_LOOPING))
		{
			IDirectSoundBuffer_Stop(dsbFx[i]);
		}

		IDirectSoundBuffer_Release(dsbFx[i]);
		fxId[i] = 0;
		fxiPaused[i] = 0;
	}
*/
	return(RD_OK);

}


int32 Sword2Sound::PauseFx(void)

{
	warning("stub PauseFx");
/*
	int32 i;
	int32 status;

	if (!fxPaused)
	{
		for (i=0; i<MAXFX; i++)
		{
			if (fxId[i])
			{
				IDirectSoundBuffer_GetStatus(dsbFx[i], &status);
				if (status & (DSBSTATUS_PLAYING + DSBSTATUS_LOOPING))
				{
					fxiPaused[i] = 1;
					if (IDirectSoundBuffer_Stop(dsbFx[i]) != RD_OK)
						return(RDERR_FXFUCKED);
				}
			}
			else
			{
				fxiPaused[i] = 0;
			}
		}
		fxPaused = 1;
	}
*/	
	return (RD_OK);

}


int32 Sword2Sound::PauseFxForSequence(void)

{
	warning("stub PauseFxForSequence");
/*
	int32 i;
	int32 status;

	if (!fxPaused)
	{
		for (i=0; i<MAXFX; i++)
		{
			if ((fxId[i]) && (fxId[i] != 0xfffffffe))
			{
				IDirectSoundBuffer_GetStatus(dsbFx[i], &status);
				if (status & (DSBSTATUS_PLAYING + DSBSTATUS_LOOPING))
				{
					fxiPaused[i] = 1;
					IDirectSoundBuffer_Stop(dsbFx[i]);
				}
			}
			else
			{
				fxiPaused[i] = 0;
			}
		}
		fxPaused = 1;
	}
*/	
	return (RD_OK);

}



int32 Sword2Sound::UnpauseFx(void)

{
	warning("stub UnpauseFx");
/*
	int32 i;

	if (fxPaused)
	{
		for (i=0; i<MAXFX; i++)
		{
			if (fxiPaused[i] && fxId[i])
			{
				if (IDirectSoundBuffer_Play(dsbFx[i], 0, 0, fxLooped[i]) != RD_OK)
					return(RDERR_FXFUCKED);
			}
		}
		fxPaused = 0;
	}
*/
	return (RD_OK);
}



uint8 Sword2Sound::GetFxVolume()
{
	return fxVol;
}


void Sword2Sound::SetFxVolume(uint8 volume)
{
	warning("stub SetFxVolume( %d )", volume);
/*
	int32 fxi;
	fxVol = volume;

	// Now update the volume of any fxs playing
	for (fxi = 0; fxi<MAXFX; fxi++)
	{
		if (fxId[fxi] && !fxMuted)
			IDirectSoundBuffer_SetVolume(dsbFx[fxi], volTable[fxVolume[fxi]*fxVol]);
	}
*/
}


void Sword2Sound::MuteFx(uint8 mute)
{
	warning("stub MuteFx( %d )");
/*
	int32 fxi;

	fxMuted = mute;

	// Now update the volume of any fxs playing
	for (fxi = 0; fxi<MAXFX; fxi++)
	{
		if (fxId[fxi])
		{
			if (mute)
				IDirectSoundBuffer_SetVolume(dsbFx[fxi], volTable[0]);
			else
				IDirectSoundBuffer_SetVolume(dsbFx[fxi], volTable[fxVolume[fxi]*fxVol]);
		}
	}
*/
}

uint8 Sword2Sound::IsFxMute(void)
{
	return (fxMuted);
}




void Sword2Sound::StartMusicFadeDown(int i)

{

//	IDirectSoundBuffer_Stop(lpDsbMus[i]);
//	IDirectSoundBuffer_Release(lpDsbMus[i]);
	musFading[i] = -16;
//	musStreaming[i] = 0;
	fpMus[i].close();

}


int32 Sword2Sound::StreamMusic(uint8 *filename, int32 looping)

{
	warning("stub StreamMusic( %s, %d )", filename, looping);
/*

	HRESULT		hr;
	LPVOID		lpv1, lpv2;
	DWORD			dwBytes1, dwBytes2;
	int32			i;
	int32 		v0, v1;
	int32			bytes;
	_wavHeader	head;

	// Do not allow compressed and uncompressed music to be streamed at the same time.
	if (compressedMusic == 1)
		return (RDERR_FXFUCKED);

	compressedMusic = 2;


	if (musStreaming[0] + musStreaming[1] == 0)
	{

		i = 0;

		fpMus[i] = fopen(filename, "rb");
		if (fpMus[i] == NULL)
			return(RDERR_INVALIDFILENAME);

		fread(&head, sizeof(_wavHeader), 1, fpMus[i]);
		streamCursor[i] = 0;
		musLooping[i] = looping;
		

  	memset(&wfMus[i], 0, sizeof(PCMWAVEFORMAT));
    wfMus[i].wf.wFormatTag = WAVE_FORMAT_PCM;
    wfMus[i].wf.nChannels = head.channels;
    wfMus[i].wf.nSamplesPerSec = head.samplesPerSec;
	wfMus[i].wBitsPerSample = 8 * head.blockAlign / (head.samplesPerSec * head.channels);
	wfMus[i].wf.nBlockAlign = wfMus[i].wf.nChannels * wfMus[i].wBitsPerSample / 8;
    wfMus[i].wf.nAvgBytesPerSec = wfMus[i].wf.nSamplesPerSec * wfMus[i].wf.nBlockAlign;


    //  Reset the sample format and size
    memset(&dsbdMus[i], 0, sizeof(DSBUFFERDESC));
    dsbdMus[i].dwSize = sizeof(DSBUFFERDESC);
//    dsbdMus[i].dwFlags = DSBCAPS_CTRLDEFAULT;
	dsbdMus[i].dwBufferBytes = 3 * wfMus[i].wf.nAvgBytesPerSec; 		// 3 seconds
    dsbdMus[i].lpwfxFormat = (LPWAVEFORMATEX) &wfMus[i];

		//	Create the sound effect sample buffer
    hr = IDirectSound_CreateSoundBuffer(lpDS, &dsbdMus[i], &lpDsbMus[i], NULL);
    if (hr == DS_OK)
		{
			hr = IDirectSoundBuffer_Lock(lpDsbMus[i], 0, dsbdMus[i].dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);

			if (hr == DSERR_BUFFERLOST)
			{
				IDirectSoundBuffer_Restore(lpDsbMus[i]);
				hr = IDirectSoundBuffer_Lock(lpDsbMus[i], 0, dsbdMus[i].dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
			}

			if (hr == DS_OK)
			{

				//	Fill the speech buffer with data
				bytes = fread(lpv1, 1, dwBytes1, fpMus[i]);
//				memcpy((uint8 *) lpv1, (uint8 *) wavData + sizeof(wavHeader), dwBytes1);

				//	Unlock the buffer now that we've filled it
				IDirectSoundBuffer_Unlock(lpDsbMus[i], lpv1, dwBytes1, lpv2, dwBytes2);

    			//  Modify the volume according to the master volume and music mute state
				if (musicMuted)
					v0 = v1 = 0;
				else
				{
    				v0 = volMusic[0];
    				v1 = volMusic[1];
				}

				if (v0 > v1)
				{
					IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v0]);
					IDirectSoundBuffer_SetPan(lpDsbMus[i], musicVolTable[v1*16/v0]);
				}
				else
				{
					if (v1 > v0)
					{
				  	    IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
						IDirectSoundBuffer_SetPan(lpDsbMus[i], -musicVolTable[v0*16/v1]);
					}
					else
					{
				        IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
						IDirectSoundBuffer_SetPan(lpDsbMus[i], 0);
					}
				}


				//	Start the sound effect playing
				IDirectSoundBuffer_Play(lpDsbMus[i], 0, 0, DSBPLAY_LOOPING);

				musStreaming[i] = 1;
				musCounter[i] = 250;
				strcpy(musFilename[i], filename);

    	  //  and exit the function.
			}
			else
			{
//				Pdebug("Failed to lock sound buffer upon creation - (%d)", hr & 0x0000ffff);
//				DirectSoundDebug("Error - ", hr);
				fclose(fpMus[i]);
				return(RDERR_LOCKFAILED);
			}
    }
		else
		{
//			Pdebug("Failed to create sound buffer - (%d)", hr & 0x0000ffff);
//			Pdebug("Error - ", hr);
			fclose(fpMus[i]);
			return(RDERR_CREATESOUNDBUFFER);
		}
	}
	else if (musStreaming[0] + musStreaming[1] == 1)
	{

		i = musStreaming[0];
		musLooping[i] = looping;

		if (!musFading[1-i])
			StartMusicFadeDown(1 - i);

		fpMus[i] = fopen(filename, "rb");
		if (fpMus[i] == NULL)
			return(RDERR_INVALIDFILENAME);

		fread(&head, sizeof(_wavHeader), 1, fpMus[i]);
		streamCursor[i] = 0;
		

  	memset(&wfMus[i], 0, sizeof(PCMWAVEFORMAT));
    wfMus[i].wf.wFormatTag = WAVE_FORMAT_PCM;
    wfMus[i].wf.nChannels = head.channels;
    wfMus[i].wf.nSamplesPerSec = head.samplesPerSec;
	wfMus[i].wBitsPerSample = 8 * head.blockAlign / (head.samplesPerSec * head.channels);
	wfMus[i].wf.nBlockAlign = wfMus[i].wf.nChannels * wfMus[i].wBitsPerSample / 8;
    wfMus[i].wf.nAvgBytesPerSec = wfMus[i].wf.nSamplesPerSec * wfMus[i].wf.nBlockAlign;


    //  Reset the sample format and size
    memset(&dsbdMus[i], 0, sizeof(DSBUFFERDESC));
    dsbdMus[i].dwSize = sizeof(DSBUFFERDESC);
//    dsbdMus[i].dwFlags = DSBCAPS_CTRLDEFAULT;
	dsbdMus[i].dwBufferBytes = 6 * wfMus[i].wf.nAvgBytesPerSec; 		// 3 seconds
    dsbdMus[i].lpwfxFormat = (LPWAVEFORMATEX) &wfMus[i];

		//	Create the sound effect sample buffer
    hr = IDirectSound_CreateSoundBuffer(lpDS, &dsbdMus[i], &lpDsbMus[i], NULL);
    if (hr == DS_OK)
		{
			hr = IDirectSoundBuffer_Lock(lpDsbMus[i], 0, dsbdMus[i].dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);

			if (hr == DSERR_BUFFERLOST)
			{
				IDirectSoundBuffer_Restore(lpDsbMus[i]);
				hr = IDirectSoundBuffer_Lock(lpDsbMus[i], 0, dsbdMus[i].dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
			}

			if (hr == DS_OK)
			{

				//	Fill the speech buffer with data
				bytes = fread(lpv1, 1, dwBytes1, fpMus[i]);
//				Pdebug("Read %d bytes\n", bytes);
//				memcpy((uint8 *) lpv1, (uint8 *) wavData + sizeof(_wavHeader), dwBytes1);

				//	Unlock the buffer now that we've filled it
				IDirectSoundBuffer_Unlock(lpDsbMus[i], lpv1, dwBytes1, lpv2, dwBytes2);

    			//  Modify the volume according to the master volume and music mute state
				if (musicMuted)
					v0 = v1 = 0;
				else
				{
    				v0 = volMusic[0];
    				v1 = volMusic[1];
				}


				if (v0 > v1)
				{
					IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v0]);
					IDirectSoundBuffer_SetPan(lpDsbMus[i], musicVolTable[v1*16/v0]);
				}
				else
				{
					if (v1 > v0)
					{
		  				IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
						IDirectSoundBuffer_SetPan(lpDsbMus[i], -musicVolTable[v0*16/v1]);
					}
					else
					{
						IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
						IDirectSoundBuffer_SetPan(lpDsbMus[i], 0);
					}
				}


				//	Start the sound effect playing
				IDirectSoundBuffer_Play(lpDsbMus[i], 0, 0, DSBPLAY_LOOPING);

				musStreaming[i] = 1;
				musCounter[i] = 250;
				strcpy(musFilename[i], filename);

			}
			else
			{
//				Pdebug("Failed to lock sound buffer upon creation - (%d)", hr & 0x0000ffff);
//				DirectSoundDebug("Error - ", hr);
				fclose(fpMus[i]);
				return(RDERR_LOCKFAILED);
			}
    }
		else
		{
//			Pdebug("Failed to create sound buffer - (%d)", hr & 0x0000ffff);
//			Pdebug("Error - ", hr);
			fclose(fpMus[i]);
			return(RDERR_CREATESOUNDBUFFER);
		}

	}
*/
	return(RD_OK);
}


void Sword2Sound::UpdateSampleStreaming(void)

{
	warning("stub UpdateSampleStreaming");
/*

	int32 	i;
	int32 	v0, v1;
	int32		readLen;
	int32		len;
	int32 	readCursor, writeCursor;
	int32		dwBytes1, dwBytes2;
	LPVOID 	lpv1, lpv2;
	HRESULT	hr;


	for (i=0; i<MAXMUS; i++)
	{
		if (musStreaming[i])
		{
			if (musFading[i])
			{
				if (musFading[i] < 0)
				{
					if (++musFading[i] == 0)
					{
						IDirectSoundBuffer_Stop(lpDsbMus[i]);
						IDirectSoundBuffer_Release(lpDsbMus[i]);
						musStreaming[i] = 0;
					}
					else
					{
		    			//  Modify the volume according to the master volume and music mute state
						if (musicMuted)
							v0 = v1 = 0;
						else
						{
							v0 = (volMusic[0] * (0 - musFading[i]) / 16);
					    	v1 = (volMusic[1] * (0 - musFading[i]) / 16);
						}

						if (v0 > v1)
						{
							IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v0]);
							IDirectSoundBuffer_SetPan(lpDsbMus[i], musicVolTable[v1*16/v0]);
						}
						else
						{
							if (v1 > v0)
							{
				  				IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
								IDirectSoundBuffer_SetPan(lpDsbMus[i], -musicVolTable[v0*16/v1]);
							}
							else
							{
								IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
								IDirectSoundBuffer_SetPan(lpDsbMus[i], 0);
							}
						}
					}
				}
			}
			else
			{

					if (IDirectSoundBuffer_GetCurrentPosition(lpDsbMus[i], &readCursor, &writeCursor) != DS_OK)
					{
//						Pdebug ("Stopping sample %d cos cant get position", i);
						IDirectSoundBuffer_Stop(lpDsbMus[i]);
					}


					len = readCursor - streamCursor[i];
					if (len < 0)
					{
						len += dsbdMus[i].dwBufferBytes;
					}
					if (len > 0)
					{
						hr = IDirectSoundBuffer_Lock(lpDsbMus[i], streamCursor[i], len, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
						if (hr == DSERR_BUFFERLOST)
						{
							IDirectSoundBuffer_Restore(lpDsbMus[i]);
							hr = IDirectSoundBuffer_Lock(lpDsbMus[i], streamCursor[i], len, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
						}

						if (hr == DS_OK)
						{
							streamCursor[i] += len;
							if (streamCursor[i] >= (int32) dsbdMus[i].dwBufferBytes)
								streamCursor[i] -= dsbdMus[i].dwBufferBytes;

							if (len > dwBytes1)
							{
								readLen = fread(lpv1, 1, dwBytes1, fpMus[i]);
								if (readLen == dwBytes1)
								{
									readLen = fread(lpv2, 1, dwBytes2, fpMus[i]);
									if (readLen != dwBytes2)
									{
										IDirectSoundBuffer_Unlock(lpDsbMus[i], lpv1, dwBytes1, lpv2, dwBytes2);
										StartMusicFadeDown(i);
										if (musLooping[i])
										{
											StreamMusic(musFilename[i], musLooping[i]);
										}
									}
									else
									{
										IDirectSoundBuffer_Unlock(lpDsbMus[i], lpv1, dwBytes1, lpv2, dwBytes2);
									}
								}
								else
								{
									IDirectSoundBuffer_Unlock(lpDsbMus[i], lpv1, dwBytes1, lpv2, dwBytes2);
									StartMusicFadeDown(i);
									if (musLooping[i])
									{
										StreamMusic(musFilename[i], musLooping[i]);
									}
								}
							}
							else
							{
								readLen = fread(lpv1, 1, len, fpMus[i]);
								if (readLen != len)
								{
									IDirectSoundBuffer_Unlock(lpDsbMus[i], lpv1, dwBytes1, lpv2, dwBytes2);
									StartMusicFadeDown(i);
									if (musLooping[i])
									{
										StreamMusic(musFilename[i], musLooping[i]);
									}
								}
								else
								{
									IDirectSoundBuffer_Unlock(lpDsbMus[i], lpv1, dwBytes1, lpv2, dwBytes2);
								}
							}
						}
//						else
//						{
//							DirectSoundDebug("Failed to lock sound buffer to write bytes", hr);
//							Pdebug("Stream cursor %d", streamCursor[i]);
//							Pdebug("len %d", len);
//						}
					}
				//}
			}
		}
	}
*/
}



int32 Sword2Sound::StreamCompMusic(const char *filename, const char *directory, uint32 musicId, int32 looping) {
	// FIXME: Find a good buffer size. The original code mentions three
	// seconds.
	uint32	buffer_size = 32768;
	uint32	i, j;
	int32	v0, v1;
	uint16	*data16;
	uint8	*data8;

	warning("stub StreamCompMusic( %s, %d, %d )", filename, musicId, looping);

	// Do not allow compressed and uncompressed music to be streamed at
	// the same time.
	if (compressedMusic == 2)
		return RDERR_FXFUCKED;

	compressedMusic = 1;

	if (musStreaming[0] + musStreaming[1] == 2) {
		// Both streams in use, try to find a fading stream
		if (musFading[0])
			i = 0;
		else
			i = 1;
			
		musFading[i] = 0;
		_mixer->stop(musicChannels[i]);
		musStreaming[i] = 0;
	}

	if (musStreaming[0] + musStreaming[1] == 1) {
		// Set i to the free channel
		i = musStreaming[0];
	} else {
		// No music streaming at present
		i = 0;
	}

	musLooping[i] = looping;		// Save looping info
	strcpy(musFilename[i], filename);	// And tune id's
	musId[i] = musicId;

	// Don't start streaming if the volume is off.
	if (IsMusicMute())
		return RD_OK;

	// Always use fpMus[0] (all music in one cluster)
	// musFilePos[i] for different pieces of music.
	if (!fpMus[0].isOpen()) {
		if (!fpMus[0].open(filename, directory))
			return RDERR_INVALIDFILENAME;
	}

	if (musStreaming[0] + musStreaming[1] == 1) {
		// Start other music stream fading out
		if (!musFading[i - 1])
			musFading[i - 1] = -16;

		// Restart the streaming cursor for this sample
		streamCursor[i] = 0;
	}

	// Seek to music index
	fpMus[0].seek((musicId + 1) * 8);

	musFilePos[i] = fpMus[0].readUint32LE();
	musEnd[i] = fpMus[0].readUint32LE();

	// Check that music is valid (has length & offset)
	if (!musEnd[i] || !musFilePos[i]) {
		fpMus[0].close();
		return RDERR_INVALIDID;
	}

	// Calculate the file position of the end of the music
	musEnd[i] += musFilePos[i];

	// Reset streaming cursor and store looping flag
	streamCursor[i] = 0;

	// Allocate a temporary buffer for compressed data
	data8 = (uint8 *) malloc(buffer_size / 2);
	if (!data8) {
		fpMus[0].close();
		return RDERR_OUTOFMEMORY;
	}

	// Allocate a buffer for the decoded sound
	data16 = (uint16 *) malloc(buffer_size);
	if (!data16) {
		fpMus[0].close();
		free(data8);
		return RDERR_OUTOFMEMORY;
	}

	lpDsbMus[i] = data16;

	// Seek to start of the compressed music
	fpMus[0].seek(musFilePos[i]);

	// Read the compressed data in to the buffer
	if (fpMus[0].read(data8, buffer_size / 2) != buffer_size / 2) {
		fpMus[0].close();
		free(data8);
		free(data16);
		return RDERR_INVALIDID;
	}

	// Store the current position in the file for future streaming
	musFilePos[i] = fpMus[0].pos();

	// Decompress the sound into the buffer

	// First sample value
	data16[0] = READ_LE_UINT16(data8);
	j = 1;

	while (j < (buffer_size / 2) - 1) {
		if (GetCompressedSign(data8[j + 1]))
			data16[j] = data16[j - 1] - (GetCompressedAmplitude(data8[j + 1]) << GetCompressedShift(data8[j + 1]));
		else
			data16[j] = data16[j - 1] + (GetCompressedAmplitude(data8[j + 1]) << GetCompressedShift(data8[j + 1]));
		j++;
	}

	// Store the value of the last sample ready for next batch of
	// decompression
	musLastSample[i] = data16[j - 1];

	// Free the decompression buffer
	free(data8);

	// Modify the volume according to the master volume and music
	// mute state
	if (musicMuted)
		v0 = v1 = 0;
	else {
		v0 = volMusic[0];
		v1 = volMusic[1];
	}

#if 0
	if (v0 > v1) {
		IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v0]);
		IDirectSoundBuffer_SetPan(lpDsbMus[i], musicVolTable[v1*16/v0]);
	} else if (v1 > v0) {
		IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
		IDirectSoundBuffer_SetPan(lpDsbMus[i], -musicVolTable[v0*16/v1]);
	} else {
	        IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
		IDirectSoundBuffer_SetPan(lpDsbMus[i], 0);
	}
#else
	warning("FIXME: Implement volume and panning");
#endif

	// Start the sound effect playing
#if 0
	// FIXME: This does not work. It sounds like white noise to me.
	musicChannels[i] = _mixer->playRaw(&musicHandle[i], lpDsbMus[i], buffer_size, 22050, SoundMixer::FLAG_16BITS | SoundMixer::FLAG_AUTOFREE);
#else
	warning("FIXME: Play the sound");
	free(lpDsbMus[i]);
#endif

	// Record the last variables for streaming and looping
	musStreaming[i] = 1;
	musCounter[i] = 250;

/*
	HRESULT		hr;
	LPVOID		lpv1, lpv2;
	DWORD		dwBytes1, dwBytes2;
	uint32		i,j;
	int32 		v0, v1;
	uint16	   *data16;
	uint8	   *data8;

	// Do not allow compressed and uncompressed music to be streamed at the same time.
	if (compressedMusic == 2)
		return (RDERR_FXFUCKED);

	compressedMusic = 1;

	if (musStreaming[0] + musStreaming[1] == 0) // No music streaming at present.
	{
		i = 0;

		musLooping[i] = looping;			// Save looping info
		strcpy(musFilename[i], filename);	// And tune id's
		musId[i] = musicId;

		if (IsMusicMute())		// Don't start streaming if the volume is off.
			return (RD_OK);

		if (!fpMus[0])
			fpMus[0] = fopen(filename, "rb");	// Always use fpMus[0] (all music in one cluster)  musFilePos[i] for different pieces of music.
		if (fpMus[0] == NULL)
			return(RDERR_INVALIDFILENAME);

		if (fseek(fpMus[0], (musicId+1)*8, SEEK_SET))	// Seek to music index
		{
			fclose(fpMus[0]);
			fpMus[0] = 0;
			return (RDERR_READERROR);
		}

		if (fread(&musFilePos[i], sizeof(uint32), 1, fpMus[0]) != 1)  // Read music index
		{
			fclose(fpMus[0]);
			fpMus[0] = 0;
			return (RDERR_READERROR);
		}

		if (fread(&musEnd[i], sizeof(uint32), 1, fpMus[0]) != 1)  // Read music length
		{
			fclose(fpMus[0]);
			fpMus[0] = 0;
			return (RDERR_READERROR);
		}

		if (!musEnd[i] || !musFilePos[i])	// Check that music is valid (has length & offset)
		{
			fclose(fpMus[0]);
			fpMus[0] = 0;
			return (RDERR_INVALIDID);
		}

		musEnd[i] += musFilePos[i];		// Calculate the file position of the end of the music

		streamCursor[i] = 0;			// Reset streaming cursor and store looping flag
	
  		memset(&wfMus[i], 0, sizeof(PCMWAVEFORMAT));	// Set up wave format (no headers in cluster)
		wfMus[i].wf.wFormatTag = WAVE_FORMAT_PCM;
		wfMus[i].wf.nChannels = 1;
		wfMus[i].wf.nSamplesPerSec = 22050;
		wfMus[i].wBitsPerSample = 16;
		wfMus[i].wf.nBlockAlign = 2;
		wfMus[i].wf.nAvgBytesPerSec = 44100;

		//  Reset the sample format and size
   		memset(&dsbdMus[i], 0, sizeof(DSBUFFERDESC));
		dsbdMus[i].dwSize = sizeof(DSBUFFERDESC);
//		dsbdMus[i].dwFlags = DSBCAPS_CTRLDEFAULT;
		dsbdMus[i].dwBufferBytes = 3 * wfMus[i].wf.nAvgBytesPerSec; 		// 3 seconds
		dsbdMus[i].lpwfxFormat = (LPWAVEFORMATEX) &wfMus[i];

		// Create a temporary buffer
		if ((data8 = malloc(dsbdMus[i].dwBufferBytes/2)) == NULL)	// Allocate a compressed data buffer
		{
			fclose(fpMus[0]);
			fpMus[0] = 0;
			return(RDERR_OUTOFMEMORY);
		}

		// Seek to start of the compressed music
		if (fseek(fpMus[0], musFilePos[i], SEEK_SET))
		{
			fclose(fpMus[0]);
			fpMus[0] = 0;
			free(data8);
			return (RDERR_INVALIDID);
		}

		// Read the compressed data in to the buffer
		if (fread(data8, sizeof(uint8), dsbdMus[i].dwBufferBytes/2, fpMus[0]) != dsbdMus[i].dwBufferBytes/2)
		{
			fclose(fpMus[0]);
			fpMus[0] = 0;
			free(data8);
			return (RDERR_INVALIDID);
		}

		// Store the current position in the file for future streaming
		musFilePos[i] = ftell(fpMus[0]);

		//	Create the music buffer
		hr = IDirectSound_CreateSoundBuffer(lpDS, &dsbdMus[i], &lpDsbMus[i], NULL);
		if (hr == DS_OK)
		{
			hr = IDirectSoundBuffer_Lock(lpDsbMus[i], 0, dsbdMus[i].dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);

			if (hr == DSERR_BUFFERLOST)
			{
				IDirectSoundBuffer_Restore(lpDsbMus[i]);
				hr = IDirectSoundBuffer_Lock(lpDsbMus[i], 0, dsbdMus[i].dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
			}

			if (hr == DS_OK)
			{
				// decompress the music into the music buffer.
				data16 = (uint16*)lpv1;
				
				data16[0] = *((int16*)data8);	// First sample value
				j=1;

				while (j<(dwBytes1/2)-1)
				{
					if (GetCompressedSign(data8[j+1]))
						data16[j] = data16[j-1] - (GetCompressedAmplitude(data8[j+1])<<GetCompressedShift(data8[j+1]));
					else
						data16[j] = data16[j-1] + (GetCompressedAmplitude(data8[j+1])<<GetCompressedShift(data8[j+1]));
					j++;
				}

				// Never need to fill lpv2 because we started at the begining of the sound buffer

				// Store the value of the last sample ready for next batch of decompression
				musLastSample[i] = data16[j-1];

				//	Free the decompression buffer and unlock the buffer now that we've filled it
				free(data8);
				IDirectSoundBuffer_Unlock(lpDsbMus[i], lpv1, dwBytes1, lpv2, dwBytes2);

    			//  Modify the volume according to the master volume and music mute state
				if (musicMuted)
					v0 = v1 = 0;
				else
				{
    				v0 = volMusic[0];
    				v1 = volMusic[1];
				}

				if (v0 > v1)
				{
					IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v0]);
					IDirectSoundBuffer_SetPan(lpDsbMus[i], musicVolTable[v1*16/v0]);
				}
				else
				{
					if (v1 > v0)
					{
				  	    IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
						IDirectSoundBuffer_SetPan(lpDsbMus[i], -musicVolTable[v0*16/v1]);
					}
					else
					{
				        IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
						IDirectSoundBuffer_SetPan(lpDsbMus[i], 0);
					}
				}


				//	Start the sound effect playing
				IDirectSoundBuffer_Play(lpDsbMus[i], 0, 0, DSBPLAY_LOOPING);

				// Recorder some last variables
				musStreaming[i] = 1;
				musCounter[i] = 250;

    		//  and exit the function.
			}
			else
			{
				// Opps Failed to lock the sound buffer
				fclose(fpMus[0]);
				fpMus[0] = 0;
				return(RDERR_LOCKFAILED);
			}
		}
		else
		{
			// Opps Failed to create the sound buffer
			fclose(fpMus[0]);
			fpMus[0] = 0;
			return(RDERR_CREATESOUNDBUFFER);
		}
	}
	else
	{
		if (musStreaming[0] + musStreaming[1] == 2)	// Both streams in use, try to find a fading stream
		{
			if (musFading[0])
				i = 0;
			else
				i = 1;
			
			musFading[i] = 0;
			IDirectSoundBuffer_Stop(lpDsbMus[i]);
			IDirectSoundBuffer_Release(lpDsbMus[i]);
			musStreaming[i] = 0;
		}

		if (musStreaming[0] + musStreaming[1] == 1)	// Some music is already streaming
		{
			i = musStreaming[0];			// Set i to the free channel

			musLooping[i] = looping;			// Save looping info
			strcpy(musFilename[i], filename);	// And tune id's
			musId[i] = musicId;

			if (IsMusicMute())		// Don't start streaming if the volume is off.
				return (RD_OK);

			if (!fpMus[0])
				fpMus[0] = fopen(filename, "rb");			// Always use fpMus[0] (all music in one cluster)  musFilePos[i] for different pieces of music.
			if (fpMus[0] == NULL)
				return(RDERR_INVALIDFILENAME);


			if (!musFading[1-i])			// Start other music stream fading out
				musFading[1 - i] = -16;

			streamCursor[i] = 0;			// Reset the streaming cursor for this sample

			if (fseek(fpMus[0], (musicId+1)*8, SEEK_SET))	// Seek to music index
			{
				fclose(fpMus[0]);
				fpMus[0] = 0;
				return (RDERR_READERROR);
			}

			if (fread(&musFilePos[i], sizeof(uint32), 1, fpMus[0]) != 1)  // Read music index
			{
				fclose(fpMus[0]);
				fpMus[0] = 0;
				return (RDERR_READERROR);
			}

			if (fread(&musEnd[i], sizeof(uint32), 1, fpMus[0]) != 1)  // Read music length
			{
				fclose(fpMus[0]);
				fpMus[0] = 0;
				return (RDERR_READERROR);
			}

			if (!musEnd[i] || !musFilePos[i])	// Check that music is valid (has length & offset)
			{
				fclose(fpMus[0]);
				fpMus[0] = 0;
				return (RDERR_INVALIDID);
			}

			musEnd[i] += musFilePos[i];		// Calculate the file position of the end of the music

	  		memset(&wfMus[i], 0, sizeof(PCMWAVEFORMAT));	// Set up the music format info
			wfMus[i].wf.wFormatTag = WAVE_FORMAT_PCM;
			wfMus[i].wf.nChannels = 1;
			wfMus[i].wf.nSamplesPerSec = 22050;
			wfMus[i].wBitsPerSample = 16;
			wfMus[i].wf.nBlockAlign = 2;
			wfMus[i].wf.nAvgBytesPerSec = 44100;

			//  Reset the sample format and size
			memset(&dsbdMus[i], 0, sizeof(DSBUFFERDESC));
			dsbdMus[i].dwSize = sizeof(DSBUFFERDESC);
//			dsbdMus[i].dwFlags = DSBCAPS_CTRLDEFAULT;
			dsbdMus[i].dwBufferBytes = 3 * wfMus[i].wf.nAvgBytesPerSec; 		// 3 seconds
			dsbdMus[i].lpwfxFormat = (LPWAVEFORMATEX) &wfMus[i];

			// Allocate a compressed data buffer
			if ((data8 = malloc(dsbdMus[i].dwBufferBytes/2)) == NULL)
			{
				fclose(fpMus[0]);
				fpMus[0] = 0;
				return(RDERR_OUTOFMEMORY);
			}

			// Seek to start of the compressed music
			if (fseek(fpMus[0], musFilePos[i], SEEK_SET))
			{
				fclose(fpMus[0]);
				fpMus[0] = 0;
				free(data8);
				return (RDERR_INVALIDID);
			}

			// Read the compressed data in to the buffer
			if (fread(data8, sizeof(uint8), dsbdMus[i].dwBufferBytes/2, fpMus[0]) != dsbdMus[i].dwBufferBytes/2)
			{
				fclose(fpMus[0]);
				fpMus[0] = 0;
				free(data8);
				return (RDERR_INVALIDID);
			}
			
			// Store the current position in the file for future streaming
			musFilePos[i] = ftell(fpMus[0]);

			//	Create the sound effect sample buffer
			hr = IDirectSound_CreateSoundBuffer(lpDS, &dsbdMus[i], &lpDsbMus[i], NULL);
			if (hr == DS_OK)
			{
				hr = IDirectSoundBuffer_Lock(lpDsbMus[i], 0, dsbdMus[i].dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);

				if (hr == DSERR_BUFFERLOST)
				{
					IDirectSoundBuffer_Restore(lpDsbMus[i]);
					hr = IDirectSoundBuffer_Lock(lpDsbMus[i], 0, dsbdMus[i].dwBufferBytes, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
				}

				if (hr == DS_OK)
				{

					// decompress the music into the music buffer.
					data16 = (uint16*)lpv1;
					
					data16[0] = *((int16*)data8);	// First sample value
					j=1;

					while (j<(dwBytes1/2)-1)
					{
						if (GetCompressedSign(data8[j+1]))
							data16[j] = data16[j-1] - (GetCompressedAmplitude(data8[j+1])<<GetCompressedShift(data8[j+1]));
						else
							data16[j] = data16[j-1] + (GetCompressedAmplitude(data8[j+1])<<GetCompressedShift(data8[j+1]));
						j++;
					}

					// Never need to fill lpv2 because we started at the begining of the sound buffer

					// Store the value of the last sample ready for next batch of decompression
					musLastSample[i] = data16[j-1];

					// Free the compressiong buffer and unlock the buffer now that we've filled it
					free(data8);
					IDirectSoundBuffer_Unlock(lpDsbMus[i], lpv1, dwBytes1, lpv2, dwBytes2);

    				//  Modify the volume according to the master volume and music mute state
					if (musicMuted)
						v0 = v1 = 0;
					else
					{
    					v0 = volMusic[0];
    					v1 = volMusic[1];
					}


					if (v0 > v1)
					{
						IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v0]);
						IDirectSoundBuffer_SetPan(lpDsbMus[i], musicVolTable[v1*16/v0]);
					}
					else
					{
						if (v1 > v0)
						{
		  					IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
							IDirectSoundBuffer_SetPan(lpDsbMus[i], -musicVolTable[v0*16/v1]);
						}
						else
						{
							IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
							IDirectSoundBuffer_SetPan(lpDsbMus[i], 0);
						}
					}


					//	Start the sound effect playing
					IDirectSoundBuffer_Play(lpDsbMus[i], 0, 0, DSBPLAY_LOOPING);

					// Record the last variables for streaming and looping
					musStreaming[i] = 1;
					musCounter[i] = 250;
				}
				else
				{
					// Opps failed to lock the sound buffer
					fclose(fpMus[0]);
					fpMus[0] = 0;
					return(RDERR_LOCKFAILED);
				}
			}
			else
			{
				// Opps failed to create the sound buffer
				fclose(fpMus[0]);
				fpMus[0] = 0;
				return(RDERR_CREATESOUNDBUFFER);
			}
		}
	}
*/
	return(RD_OK);
}


void Sword2Sound::UpdateCompSampleStreaming(void)
{
	warning("stub UpdateCompSampleStreaming");
/*

	uint32 	i,j,k;
	int32 	v0, v1;
	int32	len;
	int32 	readCursor, writeCursor;
	int32	dwBytes1, dwBytes2;
	LPVOID 	lpv1, lpv2;
	HRESULT	hr;
	uint16 *data16;
	uint8  *data8;
	int     fade;


	for (i=0; i<MAXMUS; i++)
	{
		if (musStreaming[i])
		{
			if (musFading[i])
			{
				if (musFading[i] < 0)
				{
					if (++musFading[i] == 0)
					{
						IDirectSoundBuffer_Stop(lpDsbMus[i]);
						IDirectSoundBuffer_Release(lpDsbMus[i]);
						musStreaming[i] = 0;
						musLooping[i] = 0;
					}
					else
					{
		    			//  Modify the volume according to the master volume and music mute state
						if (musicMuted)
							v0 = v1 = 0;
						else
						{
							v0 = (volMusic[0] * (0 - musFading[i]) / 16);
					    	v1 = (volMusic[1] * (0 - musFading[i]) / 16);
						}

						if (v0 > v1)
						{
							IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v0]);
							IDirectSoundBuffer_SetPan(lpDsbMus[i], musicVolTable[v1*16/v0]);
						}
						else
						{
							if (v1 > v0)
							{
				  				IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
								IDirectSoundBuffer_SetPan(lpDsbMus[i], -musicVolTable[v0*16/v1]);
							}
							else
							{
								IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[v1]);
								IDirectSoundBuffer_SetPan(lpDsbMus[i], 0);
							}
						}
					}
				}
			}
			else
			{
				if (IDirectSoundBuffer_GetCurrentPosition(lpDsbMus[i], &readCursor, &writeCursor) != DS_OK)
				{
					// Failed to get read and write positions
					IDirectSoundBuffer_Stop(lpDsbMus[i]);
				}


				// Caluculate the amount of data to load into the sound buffer
				len = readCursor - streamCursor[i];
				if (len < 0)
				{
					len += dsbdMus[i].dwBufferBytes;	// Wrap around !
				}

				// Reduce length if it requires reading past the end of the music
				if (musFilePos[i]+len >= musEnd[i])
				{
					len = musEnd[i] - musFilePos[i];
					fade = 1;		// End of music reaced so we'll need to fade and repeat
				}
				else
					fade = 0;

				if (len > 0)
				{
					hr = IDirectSoundBuffer_Lock(lpDsbMus[i], streamCursor[i], len, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
					if (hr == DSERR_BUFFERLOST)
					{
						IDirectSoundBuffer_Restore(lpDsbMus[i]);
						hr = IDirectSoundBuffer_Lock(lpDsbMus[i], streamCursor[i], len, &lpv1, &dwBytes1, &lpv2, &dwBytes2, 0);
					}

					if (hr == DS_OK)
					{
						streamCursor[i] += len;
						if (streamCursor[i] >= (int32) dsbdMus[i].dwBufferBytes)
							streamCursor[i] -= dsbdMus[i].dwBufferBytes;

						// Allocate a compressed data buffer
						if ((data8 = malloc(len/2)) == NULL)
						{
							fclose(fpMus[0]);
							fpMus[0] = 0;
							musFading[i] = -16;
						}

						// Seek to update position of compressed music when neccassary (probably never occurs)
						if (ftell(fpMus[0]) != musFilePos[i])
							fseek(fpMus[0], musFilePos[i], SEEK_SET);

						// Read the compressed data in to the buffer
						if (fread(data8, sizeof(uint8), len/2, fpMus[0]) != (size_t)len/2)
						{
							fclose(fpMus[0]);
							fpMus[0] = 0;
							free(data8);
							musFading[i] = -16;
							return;
						}

						// Update the current position in the file for future streaming
						musFilePos[i] = ftell(fpMus[0]);

						// decompress the music into the music buffer.
						data16 = (uint16*)lpv1;

						// Decompress the first byte using the last decompressed sample
						if (GetCompressedSign(data8[0]))
							data16[0] = musLastSample[i] - (GetCompressedAmplitude(data8[0])<<GetCompressedShift(data8[0]));
						else
							data16[0] = musLastSample[i] + (GetCompressedAmplitude(data8[0])<<GetCompressedShift(data8[0]));

						j = 1;

						// Decompress the rest of lpv1
						while (j<(uint32)dwBytes1/2)
						{
							if (GetCompressedSign(data8[j]))
								data16[j] = data16[j-1] - (GetCompressedAmplitude(data8[j])<<GetCompressedShift(data8[j]));
							else
								data16[j] = data16[j-1] + (GetCompressedAmplitude(data8[j])<<GetCompressedShift(data8[j]));
							j++;
						}

						// Store the value of the last sample ready for next batch of decompression
						musLastSample[i] = data16[j-1];

						if (dwBytes1 < len) // The buffer has wrapped so we need to decompress to lpv2 as well
						{
							data16 = (uint16*)lpv2;

							// Decompress first sample int lpv2 from lastsample in lpv1
							if (GetCompressedSign(data8[j]))
								data16[0] = musLastSample[i] - (GetCompressedAmplitude(data8[j])<<GetCompressedShift(data8[j]));
							else
								data16[0] = musLastSample[i] + (GetCompressedAmplitude(data8[j])<<GetCompressedShift(data8[j]));

							j++;
							k = 1;

							// Decompress the rest of lpv2
							while (k<(uint32)dwBytes2/2)
							{
								if (GetCompressedSign(data8[j]))
									data16[k] = data16[k-1] - (GetCompressedAmplitude(data8[j])<<GetCompressedShift(data8[j]));
								else
									data16[k] = data16[k-1] + (GetCompressedAmplitude(data8[j])<<GetCompressedShift(data8[j]));
								j++;
								k++;
							}
							
							// Store the value of the last sample ready for next batch of decompression
							musLastSample[i] = data16[k-1];
						}

						// Free the compressed data buffer and unlock the sound buffer.
						free(data8);
						IDirectSoundBuffer_Unlock(lpDsbMus[i], lpv1, dwBytes1, lpv2, dwBytes2);

						// End of the music so we need to start fading and start the music again
						if (fade)
						{
							musFading[i] = -16;		// Fade the old music

							// Close the music cluster if it's open
							if (fpMus[0])
							{
								fclose(fpMus[0]);
								fpMus[0] = 0;
							}

							// Loop if neccassary
							if (musLooping[i])
								StreamCompMusic(musFilename[i], musId[i], musLooping[i]);
						}
					}
				}
			}
		}
	}
	DipMusic();
*/
}

int32 Sword2Sound::DipMusic()
{
	warning("stub DipMusic");
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
	return RD_OK;
}

int32 Sword2Sound::MusicTimeRemaining()
{
	warning("stub MusicTimeRemaaining");
/*
	int32 writeCursor;
	int32 i;
	int32 readCursor;

	for (i=0; i<MAXMUS && !musStreaming[i]; i++)
	{
		// this is meant to be empty! (James19aug97)
	}

	if (i == MAXMUS)
		return 0;


	if ((IDirectSoundBuffer_GetCurrentPosition(lpDsbMus[i], &readCursor, &writeCursor)) != DS_OK)
		return 0;
	
	return (((132300-readCursor)/2 + (musEnd[i] - musFilePos[i])) / 22050);
*/
	return 0;
}



void Sword2Sound::StopMusic(void)
{
	int32 i;

	switch (compressedMusic)
	{
	case 1:	// compressed music streaming
		for (i = 0; i<MAXMUS; i++)
		{
			if (musStreaming[i])
				musFading[i] = -16;
			else
				// If the music is muted, make sure the tune doesn't restart.
				musLooping[i] = 0;
		}

		if (fpMus[0].isOpen())
			fpMus[0].close();
		break;
	case 2:
		for (i = 0; i<MAXMUS; i++)
		{
			if (musStreaming[i])
				StartMusicFadeDown(i);
		}
		break;
	default:
		break;
	}
}


int32 Sword2Sound::PauseMusic(void)
{
	warning("stub PauseMusic");
/*	
	int32 i;

	if (soundOn)
	{
		for (i=0; i<2; i++)
		{
			if (musStreaming[i])
			{
				musicPaused[i] = TRUE;

				if (IDirectSoundBuffer_Stop(lpDsbMus[i]) != RD_OK)
					return(RDERR_FXFUCKED);
			}
			else
			{
				musicPaused[i] = FALSE;
			}
		}
	}
*/
	return(RD_OK);
}

int32 Sword2Sound::UnpauseMusic(void)
{
	warning("stub UnpauseMusic");
/*
	
	int32 i;

	if (soundOn)
	{
		for (i=0; i<2; i++)
		{
			if (musicPaused[i])
			{
				if (IDirectSoundBuffer_Play(lpDsbMus[i], 0, 0, DSBPLAY_LOOPING) != RD_OK)
					return(RDERR_FXFUCKED);

				musicPaused[i] = FALSE;
			}
		}
	}
*/
	return(RD_OK);
}


void Sword2Sound::SetMusicVolume(uint8 volume)
{
	warning("stub SetMusicVolume( %d )", volume);
/*
	int32 i;
	for (i = 0; i<MAXMUS; i++)
	{
		volMusic[i] = volume;
		if (musStreaming[i] && !musFading[i] && !musicMuted)
			IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[volume]);
	}
*/
}


uint8 Sword2Sound::GetMusicVolume()
{
		return (uint8) volMusic[0];
}


void Sword2Sound::MuteMusic(uint8 mute)
{
	warning("stub MuteMusic( %d )", mute);
/*
	int32 i;

	musicMuted = mute;

	for (i = 0; i<MAXMUS; i++)
	{
		if (!mute)
		{
			if (!musStreaming[i] && musLooping[i])
				StreamCompMusic(musFilename[i], musId[i], musLooping[i]);
		}

		if (musStreaming[i] && !musFading[i])
		{
			if (mute)
				IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[0]);
			else
				IDirectSoundBuffer_SetVolume(lpDsbMus[i], musicVolTable[volMusic[i]]);
		}
	}
*/
}


uint8 Sword2Sound::IsMusicMute(void)
{
	return (musicMuted);
}



void Sword2Sound::GetSoundStatus(_drvSoundStatus *s)
{
	int i;

//	s->hwnd				= hwnd;
//	s->lpDS				= lpDS;
//	s->dsbPrimary		= dsbPrimary;
//	s->dsbSpeech		= dsbSpeech;
	s->soundOn			= soundOn;
	s->speechStatus		= speechStatus;
	s->fxPaused			= fxPaused;
	s->speechPaused		= speechPaused;
	s->speechVol		= speechVol;
	s->fxVol			= fxVol;
	s->speechMuted		= speechMuted;
	s->fxMuted			= fxMuted;
	s->compressedMusic	= compressedMusic;
	s->musicMuted		= musicMuted;

	memcpy(s->fxId,			fxId,			sizeof(int32) * MAXFX);
	memcpy(s->fxCached,		fxCached,		sizeof(uint8) * MAXFX);
//	memcpy(s->dsbFx,		dsbFx,			sizeof(LPDIRECTSOUNDBUFFER) * MAXFX);
	memcpy(s->fxiPaused,	fxiPaused,		sizeof(uint8) * MAXFX);
	memcpy(s->fxLooped,		fxLooped,		sizeof(uint8) * MAXFX);
	memcpy(s->musStreaming, musStreaming,	sizeof(int16) * MAXMUS);
	memcpy(s->musicPaused,	musicPaused,	sizeof(int16) * MAXMUS);
	memcpy(s->musCounter,	musCounter,		sizeof(int16) * MAXMUS);
	memcpy(s->musFading,	musFading,		sizeof(int16) * MAXMUS);
	memcpy(s->musLooping,	musLooping,		sizeof(int16) * MAXMUS);
	memcpy(s->musLastSample,musLastSample,	sizeof(int16) * MAXMUS);
	memcpy(s->streamCursor, streamCursor,	sizeof(int32) * MAXMUS);
	memcpy(s->musFilePos,	musFilePos,		sizeof(int32) * MAXMUS);
	memcpy(s->musEnd,		musEnd,			sizeof(int32) * MAXMUS);
	memcpy(s->musId,		musId,			sizeof(uint32) * MAXMUS);
	memcpy(s->volMusic,		volMusic,		sizeof(uint32) * 2);
//	memcpy(s->dsbdMus,		dsbdMus,		sizeof(DSBUFFERDESC) * MAXMUS);
//	memcpy(s->lpDsbMus,		lpDsbMus,		sizeof(LPDIRECTSOUNDBUFFER) * MAXMUS);
	memcpy(s->fpMus,		fpMus,			sizeof(FILE*) * MAXMUS);
//	memcpy(s->wfMus,		wfMus,			sizeof(PCMWAVEFORMAT) * MAXMUS);

	for (i = 0; i<MAXMUS; i++)
		memcpy(s->musFilename[i], musFilename[i], sizeof(char) * 256);
}


void Sword2Sound::SetSoundStatus(_drvSoundStatus *s)
{
	int i;

//	hwnd			= s->hwnd;
//	lpDS			= s->lpDS;
//	dsbPrimary		= s->dsbPrimary;
//	dsbSpeech		= s->dsbSpeech;
	soundOn			= s->soundOn;
	speechStatus	= s->speechStatus;
	fxPaused		= s->fxPaused;
	speechPaused	= s->speechPaused;
	speechVol		= s->speechVol;
	fxVol			= s->fxVol;
	speechMuted		= s->speechMuted;
	fxMuted			= s->fxMuted;
	compressedMusic	= s->compressedMusic;
	musicMuted		= s->musicMuted;

	memcpy(fxId,		 s->fxId,			sizeof(int32) * MAXFX);
	memcpy(fxCached,	 s->fxCached,		sizeof(uint8) * MAXFX);
//	memcpy(dsbFx,		 s->dsbFx,			sizeof(LPDIRECTSOUNDBUFFER) * MAXFX);
	memcpy(fxiPaused,	 s->fxiPaused,		sizeof(uint8) * MAXFX);
	memcpy(fxLooped,	 s->fxLooped,		sizeof(uint8) * MAXFX);
	memcpy(musStreaming, s->musStreaming,	sizeof(int16) * MAXMUS);
	memcpy(musicPaused,	 s->musicPaused,	sizeof(int16) * MAXMUS);
	memcpy(musCounter,	 s->musCounter,		sizeof(int16) * MAXMUS);
	memcpy(musFading,	 s->musFading,		sizeof(int16) * MAXMUS);
	memcpy(musLooping,	 s->musLooping,		sizeof(int16) * MAXMUS);
	memcpy(musLastSample,s->musLastSample,	sizeof(int16) * MAXMUS);
	memcpy(streamCursor, s->streamCursor,	sizeof(int32) * MAXMUS);
	memcpy(musFilePos,	 s->musFilePos,		sizeof(int32) * MAXMUS);
	memcpy(musEnd,		 s->musEnd,			sizeof(int32) * MAXMUS);
	memcpy(musId,		 s->musId,			sizeof(uint32) * MAXMUS);
	memcpy(volMusic,	 s->volMusic,		sizeof(uint32) * 2);
//	memcpy(dsbdMus,		 s->dsbdMus,		sizeof(DSBUFFERDESC) * MAXMUS);
//	memcpy(lpDsbMus,	 s->lpDsbMus,		sizeof(LPDIRECTSOUNDBUFFER) * MAXMUS);
//	memcpy(fpMus,		 s->fpMus,			sizeof(FILE*) * MAXMUS);
//	memcpy(wfMus,		 s->wfMus,			sizeof(PCMWAVEFORMAT) * MAXMUS);

	for (i = 0; i<MAXMUS; i++)
		memcpy(musFilename[i], s->musFilename[i], sizeof(char) * 256);
}

