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
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_SOUND_H
#define TONY_SOUND_H

#include "audio/mixer.h"
#include "common/file.h"
#include "tony/gfxcore.h"
#include "tony/loc.h"
#include "tony/utils.h"

namespace Audio {
class RewindableAudioStream;	
}

namespace Tony {

class FPSTREAM;
class FPSFX;

enum CODECS {
	FPCODEC_RAW,
	FPCODEC_ADPCM
};


/****************************************************************************\
*****************************************************************************
*       class FPSound
*       -------------
* Description: Sound driver per Falling Pumpkins
*****************************************************************************
\****************************************************************************/

class FPSOUND {

private:

	bool bSoundSupported;

	/****************************************************************************\
	*       Methods
	\****************************************************************************/

public:

	/****************************************************************************\
	*
	* Function:     FPSOUND::FPSOUND();
	*
	* Description:  Default constructor. Initializes the attributes
	*
	\****************************************************************************/

	FPSOUND();

	/****************************************************************************\
	*
	* Function:     FPSOUND::~FPSOUND();
	*
	* Description:  Deinitialize the object, free memory
	*
	\****************************************************************************/

	~FPSOUND();

	/****************************************************************************\
	*
	* Function:     bool FPSOUND::Init();
	*
	* Description:  Initializes the objects, and prepare everything required to 
	*               create streams and sound effects.
	*
	* Return:       True if everything is OK, False otherwise.
	*
	\****************************************************************************/

	bool Init();

	/****************************************************************************\
	*
	* Function:     bool CreateStream(FPSTREAM** lplpStream);
	*
	* Description:  Allocates an object of type FPSTREAM, and return its 
	*               pointer after it has been initialized.
	*
	* Input:        FPSTREAM** lplpStream   Will contain the pointer of the 
	*                                       object
	*
	* Return:       True is everything i OK, False otherwise
	*
	* Note:         The use of functions like CreateStream () and CreateSfx () 
	*               are due to the fact that the class constructors and 
	*               FPSTREAM FPSFX require that DirectSound is already initialized. 
	*               In this way, you avoid the bugs that would be created if an 
	*               object type is declared FPSTREAM FPSFX or global 
	*               (or anyway before initializing DirectSound).
	\****************************************************************************/

	bool CreateStream(FPSTREAM **lplpStream);

	/****************************************************************************\
	*
	* Function:     bool CreateSfx(FPSFX** lplpSfx);
	*
	* Description:  Allocates an object of type FPSFX and returns a pointer 
	*               pointing to it
	*
	* Input:        FPSFX** lplpSfx         Will contain the pointer of the 
	*                                       object
	*
	* Return:       True is everything i OK, False otherwise
	*
	* Note:         See notes about CreateStream()
	*
	\****************************************************************************/

	bool CreateSfx(FPSFX **lplpSfx);

	/****************************************************************************\
	*
	* Function:     void SetMasterVolume(int dwVolume);
	*
	* Description:  Set main volume
	*
	* Input:        int dwVolume          Volume to be set (0-63)
	*
	\****************************************************************************/

	void SetMasterVolume(int dwVolume);

	/****************************************************************************\
	*
	* Function:     void GetMasterVolume(LPINT lpdwVolume);
	*
	* Description:  Get main volume
	*
	* Input:        LPINT lpdwVolume        This variable will contain the 
	*                                       current volume (0-63)
	*
	\****************************************************************************/

	void GetMasterVolume(int *lpdwVolume);
};

class FPSFX {

	/****************************************************************************\
	*       Attributes
	\****************************************************************************/

private:
	bool bSoundSupported;                 // True if the sound is active
	bool bFileLoaded;                     // True is a file is opened
	bool bLoop;                           // True is sound effect should loop
	int lastVolume;

	bool bIsVoice;
	bool bPaused;

	Audio::AudioStream *_stream;
	Audio::RewindableAudioStream *_rewindableStream;
	Audio::SoundHandle _handle;

public:
	uint32 hEndOfBuffer;

private:


	/****************************************************************************\
	*       Methods
	\****************************************************************************/

public:
	/**
	 * Check process for whether sounds have finished playing
	 */
	static void soundCheckProcess(CORO_PARAM, const void *param);

	/****************************************************************************\
	*
	* Function:     FPSFX(bool bSoundOn);
	*
	* Description:  Default constructor. *DO NOT* declare the object directly,
	*               create it though FPSOUND::CreateSfx() instead
	*
	\****************************************************************************/

	FPSFX(bool bSoundOn);

	/****************************************************************************\
	*
	* Function:     ~FPSFX();
	*
	* Description:  Default destructor. It also stops the sound effect that 
	*               may be running, and free the memory used.
	*
	\****************************************************************************/

	~FPSFX();

	/****************************************************************************\
	*
	* Function:     Release();
	*
	* Description:  Releases the memory object. Must be called when the object 
	*               is no longer useful and **ONLY** when the object was created 
	*               with the FPSOUND :: CreateStream ().
	*
	* Note:         Any object pointers are no longer valid after this call.
	*
	\****************************************************************************/

	void Release();

	/****************************************************************************\
	*
	* Function:     bool LoadFile(char *lpszFileName, uint32 dwCodec=FPCODEC_RAW);
	*
	* Description:  Opens a file and load sound effect
	*
	* Input:        char *lpszFile          SFX filename
	*               uint32 dwCodec          CODEC to be used to decompress
	*                                       the sound samples
	*
	* Return:       True if everything is OK, False otherwise
	*
	\****************************************************************************/

	bool LoadFile(const char *lpszFileName, uint32 dwCodec = FPCODEC_RAW);
	bool loadWave(Common::SeekableReadStream *stream);
	bool LoadVoiceFromVDB(Common::File &vdbFP);

	/****************************************************************************\
	*
	* Function:     bool Play();
	*
	* Description:  Play the loaded FX.
	*
	* Return:       True if everything is OK, False otherwise
	*
	\****************************************************************************/

	bool Play();

	/****************************************************************************\
	*
	* Function:     bool Stop();
	*
	* Description:  Stop a FX
	*
	* Return:       True if everything is OK, False otherwise
	*
	\****************************************************************************/

	bool Stop();

	/****************************************************************************\
	*
	* Function:     void Pause(bool bPause);
	*
	* Description:  Pause a FX
	*
	\****************************************************************************/

	void Pause(bool bPause);

	/****************************************************************************\
	*
	* Function:     bool SetLoop(bool bLoop);
	*
	* Description:  Enables or disables SFX loop
	*
	* Input:        bool bLoop              True to activate the loop, else False
	*
	* Note:         The loop must be activated before the SFX starts to play, 
	*               else the effect will only be noticable next time the SFX is 
	*               played
	*
	\****************************************************************************/

	void SetLoop(bool bLoop);

	/****************************************************************************\
	*
	* Function:     void SetVolume(int dwVolume);
	*
	* Description:  Set SFX Volume
	*
	* Input:        int dwVolume            Volume to set (0-63)
	*
	\****************************************************************************/

	void SetVolume(int dwVolume);

	/****************************************************************************\
	*
	* Function:     void GetVolume(int * lpdwVolume);
	*
	* Description:  Get SFX volume
	*
	* Input:        int * lpdwVolume        Will contain the current volume
	*
	\****************************************************************************/

	void GetVolume(int *lpdwVolume);

	/**
	 * Returns true if the sound has finished playing
	 */
	bool endOfBuffer() const;
};

class FPSTREAM {

	/****************************************************************************\
	*       Attributes
	\****************************************************************************/

private:

	/*
	    HWND hwnd;
	    LPDIRECTSOUND lpDS;
	    LPDIRECTSOUNDBUFFER lpDSBuffer;    // DirectSound circular buffer
	    LPDIRECTSOUNDNOTIFY lpDSNotify;    // Notify hotspots in the buffer
	*/
	byte *lpTempBuffer;                    // Temporary buffer use for decompression

	uint32 dwBufferSize;                   // Buffer size (bytes)
	uint32 dwSize;                         // Stream size (bytes)
	uint32 dwCodec;                        // CODEC used

	HANDLE hThreadEnd;                     // Event used to close thread
	Common::File _file;                    // File handle used for the stream
	HANDLE hPlayThread;                    // Handle of the Play thread
	HANDLE hHot1, hHot2, hHot3;            // Events set by DirectSoundNotify
	HANDLE hPlayThread_PlayFast;
	HANDLE hPlayThread_PlayNormal;

	bool bSoundSupported;                  // True if the sound is active
	bool bFileLoaded;                      // True if the file is open 
	bool bLoop;                            // True if the stream should loop
	bool bDoFadeOut;                       // True if fade out is required
	bool bSyncExit;
	bool bPaused;
	int lastVolume;
	FPSTREAM *SyncToPlay;
//	DSBPOSITIONNOTIFY dspnHot[3];

	bool CreateBuffer(int nBufSize);

public:
	bool bIsPlaying;                      // True if the stream is playing

private:

	static void PlayThread(FPSTREAM *This);

	/****************************************************************************\
	*       Methods
	\****************************************************************************/

public:

	/****************************************************************************\
	*
	* Function:     FPSTREAM(bool bSoundOn);
	*
	* Description:  Default contractor. *DO NOT* declare the object directly: use
	*                                   FPSOUND::CreateStream() indtead
	*
	\****************************************************************************/

	FPSTREAM(bool bSoundOn);

	/****************************************************************************\
	*
	* Function:     ~FPSTREAM();
	*
	* Description:  Destructor by default. Stops the playing stream (if any) and
	*                                      frees the memory used by them
	*
	\****************************************************************************/

	~FPSTREAM();

	/****************************************************************************\
	*
	* Function:     Release();
	*
	* Description:  Releases memory used by object. Must be used when the object 
	*               is no longer used. *ONLY*¨for objects created by 
	*               FPSOUND::CreateStream().
	*
	* Note:         Object pointers are no longer valid after this call.
	*
	\****************************************************************************/

	void Release();

	/****************************************************************************\
	*
	* Function:     bool LoadFile(char *lpszFileName, uint32 dwCodec=FPCODEC_RAW);
	*
	* Description:  Open a file for a stream.
	*
	* Input:        char *lpszFile          Filename to be opened
	*               uint32 dwCodec          CODEC to be used to decompress
	*                                       sound samples
	*
	* Return:       True if everything is OK, False otherwise
	*
	\****************************************************************************/

	bool LoadFile(const char *lpszFileName, uint32 dwCodec = FPCODEC_RAW, int nSync = 2000);

	/****************************************************************************\
	*
	* Function:     UnloadFile();
	*
	* Description:  Close a file stream (if opened). This function must be
	*               called to free the memory used by the stream
	*
	* Return:       Just to be sure, the destructor of this class calls
	*               UnloadFile() if it has not been mentioned explicitly
	*
	\****************************************************************************/

	bool UnloadFile();

	/****************************************************************************\
	*
	* Function:     bool Play();
	*
	* Description:  Play the loaded stream.
	*
	* Return:       True if everything is OK, False otherwise
	*
	\****************************************************************************/

	bool Play();
	void PlayFast(void);
	void Prefetch(void);

	/****************************************************************************\
	*
	* Function:     bool Stop();
	*
	* Description:  Stops the play of the stream.
	*
	* Return:       True if everything is OK, False otherwise
	*
	\****************************************************************************/

	bool Stop(bool bSync = false);
	void WaitForSync(FPSTREAM *toplay);

	/****************************************************************************\
	*
	* Function:     void Pause(bool bPause);
	*
	* Description:  Pause sound effect
	*
	\****************************************************************************/

	void Pause(bool bPause);

	/****************************************************************************\
	*
	* Function:     bool SetLoop(bool bLoop);
	*
	* Description:  Enable of disable stream loop
	*
	* Input:        bool bLoop              True to enable loop, false otherwise
	*
	* Note:         The loop must be activated BEFORE you play back the stream. 
	*               Any changes made during play will not have 'effect until
	*               the stream is not stopped, and then comes back into play.
	*
	\****************************************************************************/

	void SetLoop(bool bLoop);

	/****************************************************************************\
	*
	* Function:     void SetVolume(int dwVolume);
	*
	* Description:  Change stream colume
	*
	* Input:        int dwVolume            Volume to be set (0-63)
	*
	\****************************************************************************/

	void SetVolume(int dwVolume);



	/****************************************************************************\
	*
	* Function:     void GetVolume(LPINT lpdwVolume);
	*
	* Description:  Get stream volume
	*
	* Input:        LPINT lpdwVolume        Will contain the current stream volume
	*
	\****************************************************************************/

	void GetVolume(int *lpdwVolume);
};


} // End of namespace Tony

#endif
