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

#include "audio/audiostream.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "common/textconsole.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {


#define RELEASE(x)             {if ((x) != NULL) { (x)->release(); x = NULL; }}

/**
 * Default constructor. Initializes the attributes.
 *
 */

FPSound::FPSound() {
	_bSoundSupported = false;
}

/**
 * Initializes the object, and prepare everything you need to create streams and sound effects.
 *
 * @returns     True is everything is OK, False otherwise
 */

bool FPSound::init() {
	_bSoundSupported = g_system->getMixer()->isReady();
	return _bSoundSupported;
}

/**
 * Destroy the object and free the memory
 *
 */

FPSound::~FPSound() {
}

/**
 * Allocates an object of type FPStream, and return its pointer
 *
 * @param lplpStream   Will contain a pointer to the object you just created.
 *
 * @returns     True is everything is OK, False otherwise
 */

bool FPSound::createStream(FPStream **lplpStream) {
	(*lplpStream) = new FPStream(_bSoundSupported);

	return (*lplpStream != NULL);
}

/**
 * Allocates an object of type FpSfx, and return its pointer
 *
 * @param lplpSfx         Will contain a pointer to the object you just created.
 *
 * @returns     True is everything is OK, False otherwise
 */

bool FPSound::createSfx(FPSfx **lplpSfx) {
	(*lplpSfx) = new FPSfx(_bSoundSupported);

	return (*lplpSfx != NULL);
}

/**
 * Set the general volume
 *
 * @param dwVolume          Volume to set (0-63)
 */

void FPSound::setMasterVolume(int dwVolume) {
	if (!_bSoundSupported)
		return;

	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, CLIP<int>(dwVolume, 0, 63) * Audio::Mixer::kMaxChannelVolume / 63);
}

/**
 * Get the general volume
 *
 * @param lpdwVolume          Variable that will contain the volume (0-63)
 */

void FPSound::getMasterVolume(int *lpdwVolume) {
	if (!_bSoundSupported)
		return;

	*lpdwVolume = g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) * 63 / Audio::Mixer::kMaxChannelVolume;
}

/**
 * Default constructor.
 *
 * @remarks                   Do *NOT* declare an object directly, but rather
 *                            create it using FPSound::CreateSfx()
 *
 */

FPSfx::FPSfx(bool bSoundOn) {
	_bSoundSupported = bSoundOn;
	_bFileLoaded = false;
	_lastVolume = 63;
	_hEndOfBuffer = CORO_INVALID_PID_VALUE;
	_bIsVoice = false;
	_loopStream = 0;
	_rewindableStream = 0;
	_bPaused = false;

	_vm->_activeSfx.push_back(this);
}

/**
 * Default Destructor.
 *
 * @remarks                   It is also stops the sound effect that may be
 *                            currently played, and free the memory it uses.
 *
 */

FPSfx::~FPSfx() {
	if (!_bSoundSupported)
		return;

	g_system->getMixer()->stopHandle(_handle);
	_vm->_activeSfx.remove(this);

	if (_loopStream)
		delete _loopStream; // _rewindableStream is deleted by deleting _loopStream
	else
		delete _rewindableStream;

	// FIXME
	//if (hEndOfBuffer != CORO_INVALID_PID_VALUE)
	//	CloseHandle(hEndOfBuffer);
}

/**
 * Releases the memory used by the object.
 *
 * @remarks                Must be called when the object is no longer used and
 *                         **ONLY** if the object was created by
 *                         FPSound::CreateStream().
 *                         Object pointers are no longer valid after this call.
 */

void FPSfx::release() {
	delete this;
}

bool FPSfx::loadWave(Common::SeekableReadStream *stream) {
	if (!stream)
		return false;

	_rewindableStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	if (!_rewindableStream)
		return false;

	_bFileLoaded = true;
	setVolume(_lastVolume);
	return true;
}

bool FPSfx::loadVoiceFromVDB(Common::File &vdbFP) {
	if (!_bSoundSupported)
		return true;

	uint32 size = vdbFP.readUint32LE();
	uint32 rate = vdbFP.readUint32LE();
	_bIsVoice = true;

	_rewindableStream = Audio::makeADPCMStream(vdbFP.readStream(size), DisposeAfterUse::YES, 0, Audio::kADPCMDVI, rate, 1);

	_bFileLoaded = true;
	setVolume(62);
	return true;
}

/**
 * Opens a file and loads a sound effect.
 *
 * @param lpszFileName     Sfx filename
 * @param dwCodec          CODEC used to uncompress the samples
 *
 * @returns                True is everything is OK, False otherwise
 */

bool FPSfx::loadFile(const char *lpszFileName, uint32 dwCodec) {
	if (!_bSoundSupported)
		return true;

	Common::File file;
	if (!file.open(lpszFileName)) {
		warning("FPSfx::LoadFile(): Cannot open sfx file!");
		return false;
	}

	if (file.readUint32BE() != MKTAG('A', 'D', 'P', 0x10)) {
		warning("FPSfx::LoadFile(): Invalid ADP header!");
		return false;
	}

	uint32 rate = file.readUint32LE();
	uint32 channels = file.readUint32LE();

	Common::SeekableReadStream *buffer = file.readStream(file.size() - file.pos());

	if (dwCodec == FPCODEC_ADPCM) {
		_rewindableStream = Audio::makeADPCMStream(buffer, DisposeAfterUse::YES, 0, Audio::kADPCMDVI, rate, channels);
	} else {
		byte flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

		if (channels == 2)
			flags |= Audio::FLAG_STEREO;

		_rewindableStream = Audio::makeRawStream(buffer, rate, flags, DisposeAfterUse::YES);
	}

	_bFileLoaded = true;
	return true;
}

/**
 * Play the Sfx in memory.
 *
 * @returns                True is everything is OK, False otherwise
 */

bool FPSfx::play() {
	stop(); // sanity check

	if (_bFileLoaded) {
		// FIXME
		//if (hEndOfBuffer != CORO_INVALID_PID_VALUE)
		//	ResetEvent(hEndOfBuffer);

		_rewindableStream->rewind();

		Audio::AudioStream *stream = _rewindableStream;

		if (_bLoop) {
			if (!_loopStream)
				_loopStream = Audio::makeLoopingAudioStream(_rewindableStream, 0);

			stream = _loopStream;
		}

		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, stream, -1,
				Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);

		setVolume(_lastVolume);

		if (_bPaused)
			g_system->getMixer()->pauseHandle(_handle, true);
	}

	return true;
}

/**
 * Stops a Sfx.
 *
 * @returns                True is everything is OK, False otherwise
 */

bool FPSfx::stop() {
	if (_bFileLoaded) {
		g_system->getMixer()->stopHandle(_handle);
		_bPaused = false;
	}

	return true;
}

/**
 * Enables or disables the Sfx loop.
 *
 * @param bLoop         True to enable the loop, False to disable
 *
 * @remarks             The loop must be activated BEFORE the sfx starts
 *                      playing. Any changes made during the play will have
 *                      no effect until the sfx is stopped then played again.
 */

void FPSfx::setLoop(bool bLop) {
	_bLoop = bLop;
}

/**
 * Pauses a Sfx.
 *
 */

void FPSfx::pause(bool bPause) {
	if (_bFileLoaded) {
		if (g_system->getMixer()->isSoundHandleActive(_handle) && (bPause ^ _bPaused))
			g_system->getMixer()->pauseHandle(_handle, bPause);

		_bPaused = bPause;
	}
}

/**
 * Change the volume of Sfx
 *
 * @param dwVolume      Volume to be set (0-63)
 *
 */

void FPSfx::setVolume(int dwVolume) {
	if (dwVolume > 63)
		dwVolume = 63;
	if (dwVolume < 0)
		dwVolume = 0;

	_lastVolume = dwVolume;

	if (_bIsVoice) {
		if (!GLOBALS._bCfgDubbing)
			dwVolume = 0;
		else {
			dwVolume -= (10 - GLOBALS._nCfgDubbingVolume) * 2;
			if (dwVolume < 0) dwVolume = 0;
		}
	} else {
		if (!GLOBALS._bCfgSFX)
			dwVolume = 0;
		else {
			dwVolume -= (10 - GLOBALS._nCfgSFXVolume) * 2;
			if (dwVolume < 0)
				dwVolume = 0;
		}
	}

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->setChannelVolume(_handle, dwVolume * Audio::Mixer::kMaxChannelVolume / 63);
}

/**
 * Gets the Sfx volume
 *
 * @param lpdwVolume    Will contain the current Sfx volume
 *
 */

void FPSfx::getVolume(int *lpdwVolume) {
	if (g_system->getMixer()->isSoundHandleActive(_handle))
		*lpdwVolume = g_system->getMixer()->getChannelVolume(_handle) * 63 / Audio::Mixer::kMaxChannelVolume;
	else
		*lpdwVolume = 0;
}

/**
 * Returns true if the underlying sound has ended
 */

bool FPSfx::endOfBuffer() const {
	return !g_system->getMixer()->isSoundHandleActive(_handle) && (!_rewindableStream || _rewindableStream->endOfData());
}

/**
 * Continually checks to see if active sounds have finished playing
 * Sets the event signalling the sound has ended
 */
void FPSfx::soundCheckProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		Common::List<FPSfx *>::iterator i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	for (;;) {
		// Check each active sound
		for (_ctx->i = _vm->_activeSfx.begin(); _ctx->i != _vm->_activeSfx.end(); ++_ctx->i) {
			FPSfx *sfx = *_ctx->i;
			if (sfx->endOfBuffer())
				CoroScheduler.setEvent(sfx->_hEndOfBuffer);
		}

		// Delay until the next check is done
		CORO_INVOKE_1(CoroScheduler.sleep, 50);
	}

	CORO_END_CODE;
}

/**
 * Default constructor. 
 *
 * @remarks             Do *NOT* declare an object directly, but rather
 *                      create it using FPSound::CreateStream()
 */

FPStream::FPStream(bool bSoundOn) {
#ifdef REFACTOR_ME
	//hwnd=hWnd;
	lpDS = LPDS;
	bSoundSupported = bSoundOn;
	bFileLoaded = false;
	bIsPlaying = false;
	bPaused = false;
	bSyncExit = false;
	lpDSBuffer = NULL;
	lpDSNotify = NULL;
	hHot1 = hHot2 = hHot3 = hPlayThread_PlayFast = hPlayThread_PlayNormal = NULL;
#endif
}

bool FPStream::createBuffer(int nBufSize) {
#ifdef REFACTOR_ME
	static PCMWAVEFORMAT pcmwf;
	static DSBUFFERDESC dsbdesc;
	static HRESULT err;
	static char errbuf[128];

	if (bSoundSupported == false)
		return true;

	/* Set the required structures for the creation of a secondary buffer for the stream containing exactly 1 second of music. 
	   Also activate the volume control, in order to lower and raise the volume of the music regardless of the general volume. 
	   Obviously it is a buffer in RAM */

	pcmwf.wBitsPerSample = 16;
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = 2;
	pcmwf.wf.nSamplesPerSec = 44100;
	pcmwf.wf.nBlockAlign = (pcmwf.wBitsPerSample / 8) * pcmwf.wf.nChannels;
	pcmwf.wf.nAvgBytesPerSec = (uint32)pcmwf.wf.nBlockAlign * (uint32)pcmwf.wf.nSamplesPerSec;

	dsbdesc.dwSize = sizeof(dsbdesc);
	dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;
	dwBufferSize = dsbdesc.dwBufferBytes = (((uint32)(pcmwf.wf.nAvgBytesPerSec * nBufSize) / 1000 + 31) / 32) * 32;
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX) & pcmwf;

	if ((err = lpDS->CreateSoundBuffer(&dsbdesc, &lpDSBuffer, NULL)) != DS_OK) {
		wsprintf(errbuf, "Error creating the secondary buffer (%lx)", err);
		MessageBox(hwnd, errbuf, "FPStream::FPStream()", MB_OK);
		bSoundSupported = false;
		return false;
	}
	SetVolume(63);


	/* Create an alert when key positions are reached in the stream. 
	   Key positions are located, respectively, immediately after 
	   the start and immediately after the middle of the buffer */
	err = lpDSBuffer->QueryInterface(IID_IDirectSoundNotify, (void **)&lpDSNotify);

	if (FAILED(err)) {
		wsprintf(errbuf, "Error creating notify object! (%lx)", err);
		MessageBox(hwnd, errbuf, "FPStream::FPStream()", MB_OK);
		bSoundSupported = false;
		return false;
	}

	hHot1 = CreateEvent(NULL, false, false, NULL);
	hHot2 = CreateEvent(NULL, false, false, NULL);
	hHot3 = CreateEvent(NULL, false, false, NULL);
	hPlayThread_PlayFast = CreateEvent(NULL, false, false, NULL);

	dspnHot[0].dwOffset = 32;
	dspnHot[0].hEventNotify = hHot1;

	dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
	dspnHot[1].hEventNotify = hHot2;

	dspnHot[2].dwOffset = dwBufferSize - 32;   //DSBPN_OFFSETSTOP;
	dspnHot[2].hEventNotify = hHot3;

	lpDSNotify->SetNotificationPositions(3, dspnHot);
#endif
	return true;
}

/**
 * Default destructor. 
 *
 * @remarks             It calls CloseFile() if needed.
 */

FPStream::~FPStream() {
#ifdef REFACTOR_ME

	if (!bSoundSupported)
		return;

	if (bIsPlaying)
		Stop();

	if (bFileLoaded)
		UnloadFile();

	if (hHot1) {
		CloseHandle(hHot1);
		hHot1 = NULL;
	}
	if (hHot2) {
		CloseHandle(hHot2);
		hHot2 = NULL;
	}
	if (hHot3) {
		CloseHandle(hHot3);
		hHot3 = NULL;
	}
	if (hPlayThread_PlayFast) {
		CloseHandle(hPlayThread_PlayFast);
		hPlayThread_PlayFast = NULL;
	}
	if (hPlayThread_PlayNormal) {
		CloseHandle(hPlayThread_PlayNormal);
		hPlayThread_PlayNormal = NULL;
	}

	SyncToPlay = NULL;

	RELEASE(lpDSNotify);
	RELEASE(lpDSBuffer);
#endif
}

/**
 * Releases the memory object. 
 *
 * @remarks             Must be called when the object is no longer used 
 *                      and **ONLY** if the object was created by
 *                      FPSound::CreateStream().
 *                      Object pointers are no longer valid after this call.
 */

void FPStream::release() {
	delete this;
}

/**
 * Opens a file stream
 *
 * @param lpszFile      Filename to be opened
 * @param dwCodec       CODEC to be used to uncompress samples
 *
 * @returns             True is everything is OK, False otherwise
 */

bool FPStream::loadFile(const char *lpszFileName, uint32 dwCodType, int nBufSize) {
#ifdef REFACTOR_ME
	HRESULT err;
	void *lpBuf;
	uint32 dwHi;

	if (!bSoundSupported)
		return true;

	/* Si salva il tipo di codec */
	dwCodec = dwCodType;

	/* Crea il buffer */
	if (!CreateBuffer(nBufSize))
		return true;

	/* Apre il file di stream in lettura */
	if (!_file.open(lpszFileName))
		//MessageBox(hwnd,"Cannot open stream file!","FPStream::LoadFile()", MB_OK);
		return false;
}

/* Si salva la lunghezza dello stream */
dwSize = _file.size();
_file.seek(0);

/* Tutto a posto, possiamo uscire */
bFileLoaded = true;
bIsPlaying = false;
bPaused = false;
#endif
return true;
}

/**
 * Closes a file stream (opened or not).
 *
 * @returns             For safety, the destructor calls unloadFile() if it has not
 *                      been mentioned explicitly.
 * 
 * @remarks             It is necessary to call this function to free the 
 *                      memory used by the stream.
 */

bool FPStream::unloadFile() {
#ifdef REFACTOR_ME

	if (!bSoundSupported || !bFileLoaded)
		return true;

	/* Closes the file handle stream */
	_file.close();

	RELEASE(lpDSNotify);
	RELEASE(lpDSBuffer);

	/* Remember no more file is loaded in memory */
	bFileLoaded = false;
#endif
	return true;
}

void FPStream::prefetch() {
#ifdef REFACTOR_ME
	uint32 dwId;
	void *lpBuf;
	uint32 dwHi;
	HRESULT err;

	if (!bSoundSupported || !bFileLoaded)
		return;

	/* Allocates a temporary buffer */
	lpTempBuffer = (byte *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwBufferSize / 2);
	if (lpTempBuffer == NULL)
		return;

	switch (dwCodec) {
	case FPCODEC_RAW:
		lpCodec = new CODECRAW(bLoop);
		break;

	case FPCODEC_ADPCM:
		lpCodec = new CODECADPCMSTEREO(bLoop);
		break;

	default:
		return;
	}

	/* reset the file position */
	_file.seek(0);

	/* Fills the buffer for the data already ready */
	if ((err = lpDSBuffer->Lock(0, dwBufferSize / 2, &lpBuf, (uint32 *)&dwHi, NULL, NULL, 0)) != DS_OK) {
		MessageBox(hwnd, "Cannot lock stream buffer!", "soundLoadStream()", MB_OK);
		return;
	}

	/* Uncompress the data from the stream directly into the locked buffer */
	lpCodec->Decompress(hFile, lpBuf, dwBufferSize / 2);

	/* Unlock the buffer */
	lpDSBuffer->Unlock(lpBuf, dwBufferSize / 2, NULL, NULL);

	/* Create a thread to play the stream */
	hThreadEnd = CreateEvent(NULL, false, false, NULL);
	hPlayThread = CreateThread(NULL, 10240, (LPTHREAD_START_ROUTINE)PlayThread, (void *)this, 0, &dwId);
	SetThreadPriority(hPlayThread, THREAD_PRIORITY_HIGHEST);

	/* Start to play the buffer */
	lpDSBuffer->SetCurrentPosition(0);
	bIsPlaying = true;

	dspnHot[0].dwOffset = 32;
	dspnHot[0].hEventNotify = hHot1;

	dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
	dspnHot[1].hEventNotify = hHot2;

	dspnHot[2].dwOffset = dwBufferSize - 32;   //DSBPN_OFFSETSTOP;
	dspnHot[2].hEventNotify = hHot3;

	if (FAILED(lpDSNotify->SetNotificationPositions(3, dspnHot))) {
		int a = 1;
	}
#endif
}

void FPStream::playFast() {
#ifdef REFACTOR_ME
	dspnHot[0].dwOffset = 32;
	dspnHot[0].hEventNotify = hHot1;

	dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
	dspnHot[1].hEventNotify = hHot2;

	dspnHot[2].dwOffset = dwBufferSize - 32; //DSBPN_OFFSETSTOP;
	dspnHot[2].hEventNotify = hHot3;

	lpDSBuffer->Stop();

	if (FAILED(lpDSNotify->SetNotificationPositions(3, dspnHot))) {
		warning("PlayFast SNP failed!");
	}

	if (FAILED(lpDSBuffer->Play(0, 0, DSBPLAY_LOOPING))) {
		warning("PlayFast failed!\n");
	}
#endif
}

/**
 * Play the stream.
 *
 * @returns             True is everything is OK, False otherwise
 */

bool FPStream::play() {
#ifdef REFACTOR_ME
	uint32 dwId;
	void *lpBuf;
	uint32 dwHi;
	HRESULT err;

	if (!bSoundSupported || !bFileLoaded)
		return false;

	/* Allocate a temporary buffer */
	lpTempBuffer = (byte *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwBufferSize / 2);
	if (lpTempBuffer == NULL)
		return false;

	switch (dwCodec) {
	case FPCODEC_RAW:
		lpCodec = new CODECRAW(bLoop);
		break;

	case FPCODEC_ADPCM:
		lpCodec = new CODECADPCMSTEREO(bLoop);
		break;

	default:
		return false;
	}

	/* Reset the file position */
	_file.seek(0);
	lpDSBuffer->Stop();
	lpDSBuffer->SetCurrentPosition(0);

	/* Fills the buffer for the data already ready */
	if ((err = lpDSBuffer->Lock(0, dwBufferSize / 2, &lpBuf, (uint32 *)&dwHi, NULL, NULL, 0)) != DS_OK) {
		error("Cannot lock stream buffer!", "soundLoadStream()");
	}

	/* Uncompress the data from the stream directly into the locked buffer */
	lpCodec->Decompress(hFile, lpBuf, dwBufferSize / 2);

	/* Unlock the buffer */
	lpDSBuffer->Unlock(lpBuf, dwBufferSize / 2, NULL, NULL);

	/* Create a thread to play the stream */
	hThreadEnd = CreateEvent(NULL, false, false, NULL);
	hPlayThread = CreateThread(NULL, 10240, (LPTHREAD_START_ROUTINE)PlayThread, (void *)this, 0, &dwId);
	SetThreadPriority(hPlayThread, THREAD_PRIORITY_HIGHEST);

	SetEvent(hPlayThread_PlayFast);

#if 0
	/* Start to play the buffer */
	lpDSBuffer->SetCurrentPosition(0);

	dspnHot[0].dwOffset = 32;
	dspnHot[0].hEventNotify = hHot1;

	dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
	dspnHot[1].hEventNotify = hHot2;

	dspnHot[2].dwOffset = dwBufferSize - 32; //DSBPN_OFFSETSTOP;
	dspnHot[2].hEventNotify = hHot3;

	if (FAILED(lpDSNotify->SetNotificationPositions(3, dspnHot))) {
		int a = 1;
	}


	lpDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
#endif

	bIsPlaying = true;
#endif
	return true;
}

/**
 * Closes the stream.
 *
 * @returns             True is everything is OK, False otherwise
 * 
 */

bool FPStream::stop(bool bSync) {
#ifdef REFACTOR_ME

	if (!bSoundSupported)
		return true;

	if (!bFileLoaded)
		return false;

	if (!bIsPlaying)
		return false;

	if (bSync) {
//	 bSyncExit = true;
//	 lpDSBuffer->Stop();
//	 lpDSBuffer->Play(0, 0, 0);
		return true;
	} else {
		/* Close the DirectSound buffer */
		lpDSBuffer->Stop();

		/* Notify the thread is should stop */
		SetEvent(hThreadEnd);
		WaitForSingleObject(hPlayThread, CORO_INFINITE);

		/* Closes the handle used by the stream and free its memory */
		CloseHandle(hPlayThread);
		CloseHandle(hThreadEnd);
		GlobalFree(lpTempBuffer);

		/* Close and free the CODEC */
		delete lpCodec;

		bIsPlaying = false;
		bPaused = false;
	}
#endif
	return true;
}

void FPStream::waitForSync(FPStream *toplay) {
#ifdef REFACTOR_ME
	if (!bSoundSupported)
		return;

	if (!bFileLoaded)
		return;

	if (!bIsPlaying)
		return;

	SyncToPlay = toplay;
	bSyncExit = true;

	char buf[1024];
	sprintf(buf, "Wait for sync: %x (SyncToPlay: [%x]=%x, SyncExit: [%x]=%d) MyThread: 0x%x\n",
	        this->lpDSBuffer, &this->SyncToPlay, SyncToPlay, &bSyncExit, bSyncExit, GetCurrentThreadId());
	warning(buf);

	WaitForSingleObject(hPlayThread, CORO_INFINITE);

	/* Closes the handle used by the stream and free its memory */
	CloseHandle(hPlayThread);
	CloseHandle(hThreadEnd);
	GlobalFree(lpTempBuffer);

	/* Close and free the CODEC */
	delete lpCodec;
#endif
	_bIsPlaying = false;
}

/**
 * Thread playing the stream
 *
 */

void FPStream::playThread(FPStream *This) {
#ifdef REFACTOR_ME
	byte *lpLockBuf;
	uint32 dwResult;
	byte *lpLockBuf2;
	uint32 dwResult2;
	bool cicla = true;
	uint32 countEnd;
	bool bPrecache;
	char buf[1024];

	/* Events that signal when you need to do something */
	HANDLE hList[5] = { This->hThreadEnd, This->hHot1, This->hHot2, This->hHot3, This->hPlayThread_PlayFast };

	bPrecache = true;
	countEnd = 0;
	while (cicla) {
		if (This->lpCodec->EndOfStream() && This->lpCodec->bLoop == false) {
			countEnd++;
			if (countEnd == 3)
				break;
		}

		/* Uncompresses the data being written into the temporary buffer */
		if (This->lastVolume == 0)
			ZeroMemory(This->lpTempBuffer, This->dwBufferSize / 2);
		else if (bPrecache)
			This->lpCodec->Decompress(This->_file, This->lpTempBuffer, This->dwBufferSize / 2);

		bPrecache = false;

		/* Waits for an event. Since they are all in automatic reset, there is no need to reset it after */

//	 uint32 dwBufStatus;
//	 This->lpDSBuffer->GetStatus(&dwBufStatus);


// sprintf(buf, "WFMO: %x (buf status: %x) MyThread: 0x%x\n", This->lpDSBuffer, dwBufStatus, GetCurrentThreadId());
// warning(buf);
		dwResult = WaitForMultipleObjects(5, hList, false, CORO_INFINITE);

		/*  uint32 dwPlay, dwWrite;
		    This->lpDSBuffer->GetCurrentPosition(&dwPlay, &dwWrite);
		    sprintf(buf, "CP Play: %u, Write: %u\n", dwPlay, dwWrite);
		    warning(buf); */

		/* Make a switch to determine which event has been set */
		switch (dwResult - WAIT_OBJECT_0) {
		case 0:
			/* Must leave the thread */
			cicla = false;
			break;

		case 1:
			/* Must fill the second half of the buffer */
//	   if (dwPlay >= This->dspnHot[0].dwOffset && dwPlay <= This->dspnHot[0].dwOffset+1024 )
		{
//		   sprintf(buf, "Prima metà buffer: %x\n", This->lpDSBuffer);
//		   warning(buf);
			This->lpDSBuffer->Lock(This->dwBufferSize / 2, This->dwBufferSize / 2, (void **)&lpLockBuf, &dwResult, (void **)&lpLockBuf2, &dwResult2, 0);
			//     sprintf(buf, "LockedBuf: dwResult=%x, dwBufferSize/2=%x, lpLockBuf2=%x, dwResult2=%x\n", dwResult, This->dwBufferSize/2, lpLockBuf2, dwResult2);
			//     warning(buf);
			copyMemory(lpLockBuf, This->lpTempBuffer, This->dwBufferSize / 2);
			This->lpDSBuffer->Unlock(lpLockBuf, This->dwBufferSize / 2, lpLockBuf2, 0);
			bPrecache = true;
		}
		break;

		case 2:
			/* Must fill the first half of the buffer */
//		 if (dwPlay >= This->dspnHot[1].dwOffset && dwPlay <= This->dspnHot[1].dwOffset+1024 )
		{
//			 sprintf(buf, "Seconda metà buffer: %x\n", This->lpDSBuffer);
//			 warning(buf);
			This->lpDSBuffer->Lock(0, This->dwBufferSize / 2, (void **)&lpLockBuf, &dwResult, NULL, NULL, 0);
			copyMemory(lpLockBuf, This->lpTempBuffer, This->dwBufferSize / 2);
			This->lpDSBuffer->Unlock(lpLockBuf, This->dwBufferSize / 2, NULL, NULL);
			bPrecache = true;
		}
		break;

		case 3: {
//		 sprintf(buf, "End of buffer %x (SyncToPlay [%x]=%x, SyncExit: [%x]=%d)\n", This->lpDSBuffer, &This->SyncToPlay, This->SyncToPlay, &This->bSyncExit, This->bSyncExit);
//		 warning(buf);
			if (This->bSyncExit) {
//			 sprintf(buf, "Go with sync (Buffer: %x) MyThread: %x!\n", This->SyncToPlay->lpDSBuffer, GetCurrentThreadId());
//			 warning(buf);
				//This->SyncToPlay->PlayFast();
				SetEvent(This->SyncToPlay->hPlayThread_PlayFast);
				// Transfer immediatly control to the other threads
				Sleep(0);
				This->bSyncExit = false;
				cicla = false;
				break;
			}
		}
		break;

		case 4:
			This->PlayFast();
			break;
		}
	}

	/* Close the DirectSound buffer */
// sprintf(buf, "Exiting thread. Buffer = %x, MyThread = 0x%x\n", This->lpDSBuffer, GetCurrentThreadId());
// warning(buf);
	This->lpDSBuffer->Stop();

	ExitThread(0);
#endif
}

/**
 * Unables or disables stream loop.
 *
 * @param bLoop         True enable loop, False disables it
 *
 * @remarks             The loop must be activated BEFORE the stream starts
 *                      playing. Any changes made during the play will have no
 *                      effect until the stream is stopped then played again.
 */

void FPStream::setLoop(bool loop) {
	_bLoop = loop;
}


/**
 * Pause sound effect
 *
 * @param bPause        True enables pause, False disables it
 */

void FPStream::pause(bool bPause) {
#ifdef REFACTOR_ME

	if (bFileLoaded) {
		if (bPause && bIsPlaying) {
			lpDSBuffer->Stop();
			bIsPlaying = false;
			bPaused = true;
		} else if (!bPause && bPaused) {
			dspnHot[0].dwOffset = 32;
			dspnHot[0].hEventNotify = hHot1;

			dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
			dspnHot[1].hEventNotify = hHot2;

			dspnHot[2].dwOffset = dwBufferSize - 32; //DSBPN_OFFSETSTOP;
			dspnHot[2].hEventNotify = hHot3;

			if (FAILED(lpDSNotify->SetNotificationPositions(3, dspnHot))) {
				int a = 1;
			}

			lpDSBuffer->Play(0, 0, bLoop);
			bIsPlaying = true;
			bPaused = false;

			// Trick to reset the volume after a possible new sound configuration
			SetVolume(lastVolume);
		}
	}
#endif
}

/**
 * Change the volume of the stream
 *
 * @param dwVolume      Volume to be set (0-63)
 *
 */

void FPStream::setVolume(int dwVolume) {
#ifdef REFACTOR_ME
	if (dwVolume > 63) dwVolume = 63;
	if (dwVolume < 0) dwVolume = 0;

	lastVolume = dwVolume;

	if (!GLOBALS.bCfgMusic) dwVolume = 0;
	else {
		dwVolume -= (10 - GLOBALS.nCfgMusicVolume) * 2;
		if (dwVolume < 0) dwVolume = 0;
	}

	if (lpDSBuffer)
		lpDSBuffer->SetVolume(dwVolume * (DSBVOLUME_MAX - DSBVOLUME_MIN) / 64 + DSBVOLUME_MIN);
#endif
}

/**
 * Gets the vgolume of the stream
 *
 * @param lpdwVolume    Variable that will contain the current volume
 *
 */

void FPStream::getVolume(int *lpdwVolume) {
#ifdef REFACTOR_ME
	if (lpDSBuffer)
		lpDSBuffer->GetVolume((uint32 *)lpdwVolume);
	*lpdwVolume -= (DSBVOLUME_MIN);
	*lpdwVolume *= 64;
	*lpdwVolume /= (DSBVOLUME_MAX - DSBVOLUME_MIN);
#endif
}

} // End of namespace Tony
