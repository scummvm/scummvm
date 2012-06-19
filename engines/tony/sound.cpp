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


/****************************************************************************\
*****************************************************************************
*       class CODECRAW
*       --------------
* Description: CODEC to play hard from pure samples
*****************************************************************************
\****************************************************************************/

class CODECRAW : public CODEC {
public:
	CODECRAW(bool _bLoop = true);

	virtual ~CODECRAW();
	virtual uint32 decompress(Common::SeekableReadStream *stream, void *lpBuf, uint32 dwSize);
	virtual void loopReset();
};


/****************************************************************************\
*****************************************************************************
*       class CODECADPCM
*       ----------------
* Description: Play ADPCM compressed data
*****************************************************************************
\****************************************************************************/

class CODECADPCM : public CODECRAW {
protected:
	byte *lpTemp;
	static const int indexTable[16];
	static const int stepSizeTable[89];

public:
	CODECADPCM(bool _bLoop = true, byte *lpTempBuffer = NULL);
	virtual ~CODECADPCM();
	virtual uint32 decompress(Common::SeekableReadStream *stream, void *lpBuf, uint32 dwSize) = 0;
	virtual void loopReset() = 0;
};

class CODECADPCMSTEREO : public CODECADPCM {
protected:
	int valpred[2], index[2];

public:
	CODECADPCMSTEREO(bool _bLoop=true, byte *lpTempBuffer = NULL);
	virtual ~CODECADPCMSTEREO();
	virtual uint32 decompress(Common::SeekableReadStream *stream, void *lpBuf, uint32 dwSize);
	virtual void loopReset();
};

class CODECADPCMMONO : public CODECADPCM {
protected:
	int valpred, index;

public:
	CODECADPCMMONO(bool _bLoop = true, byte *lpTempBuffer = NULL);
	virtual ~CODECADPCMMONO();
	virtual uint32 decompress(Common::SeekableReadStream *stream, void *lpBuf, uint32 dwSize);
	virtual void loopReset();
};

/****************************************************************************\
*       CODEC Methods
\****************************************************************************/

/**
 * Standard cosntructor. It's possible to specify whether you want to
 * enable or disable the loop (which by default, and 'active).
 *
 * @param loop			True if you want to loop, false to disable
 */
CODEC::CODEC(bool loop) {
	_bLoop = loop;
	_bEndReached = false;
}

CODEC::~CODEC() {
}

/**
 * Tell whether we have reached the end of the stream
 *
 * @return				True if we're done, false otherwise.
 */
bool CODEC::endOfStream() {
	return _bEndReached;
}


/****************************************************************************\
*       CODECRAW Methods
\****************************************************************************/

/**
 * Standard cosntructor. Simply calls the inherited constructor
 */
CODECRAW::CODECRAW(bool loop) : CODEC(loop) {
}

CODECRAW::~CODECRAW() {
}

/**
 * Reset the stream to the beginning of the file. In the case of RAW files, does nothing
 */
void CODECRAW::loopReset() {
}

/**
 * Manage the RAW format. Simply copies the file's stream buffer
 * 
 * @return				Indicates the position of the file for the end of the loop
 */
uint32 CODECRAW::decompress(Common::SeekableReadStream *stream, void *buf, uint32 dwSize) {
	byte *lpBuf = (byte *)buf;
	uint32 dwRead;
	uint32 dwEOF;

	_bEndReached = false;
	dwEOF = 0;
	dwRead = stream->read(lpBuf, dwSize);

	if (dwRead < dwSize) {
		dwEOF = dwRead;
		_bEndReached = true;

		if (!_bLoop) {
			Common::fill(lpBuf + dwRead, lpBuf + dwRead + (dwSize - dwRead), 0);
		} else {
			stream->seek(0);
			dwRead = stream->read(lpBuf + dwRead, dwSize - dwRead);
		}
	}

	return dwEOF;
}

/****************************************************************************\
*       CODECADPCM Methods
\****************************************************************************/

const int CODECADPCM::indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

const int CODECADPCM::stepSizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

#define MAXDECODESIZE          (44100 * 2 * 2)

/**
 * Standard constructor. Initialises and allocates temporary memory tables
 */
CODECADPCM::CODECADPCM(bool loop, byte *lpTempBuffer) : CODECRAW(loop) {
	// Alloca la memoria temporanea
	if (lpTempBuffer != NULL) {
		lpTemp = lpTempBuffer;
	} else {
		lpTemp = (byte *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, MAXDECODESIZE);

 		if (lpTemp == NULL) {
			error("Insufficient memory!");
			return;
		}
	}
}


CODECADPCMMONO::CODECADPCMMONO(bool loop, byte *lpTempBuffer) : CODECADPCM(loop,lpTempBuffer) {
	// Inizializza per il playing
	loopReset();
}

CODECADPCMMONO::~CODECADPCMMONO() {
}


CODECADPCMSTEREO::CODECADPCMSTEREO(bool loop, byte *lpTempBuffer) : CODECADPCM(loop, lpTempBuffer) {
	// Initialise for playing
	loopReset();
}

CODECADPCMSTEREO::~CODECADPCMSTEREO() {
}

/**
 * Destructor. Free the buffer
 */
CODECADPCM::~CODECADPCM() {
	globalDestroy(lpTemp);
}


/**
 * Reset the player before each play or loop
 */
void CODECADPCMSTEREO::loopReset() {
	valpred[0] = 0;
	valpred[1] = 0;
	index[0] = 0;
	index[1] = 0;
}

void CODECADPCMMONO::loopReset() {
	valpred = 0;
	index = 0;
}


/**
 * Manages decompressing the ADPCM 16:4 format.
 */
uint32 CODECADPCMMONO::decompress(Common::SeekableReadStream *stream, void *buf, uint32 dwSize) {
	uint16 *lpBuf = (uint16 *)buf;
	byte *inp;
	int bufferstep;
	int cache = 0;
	int delta;
	int sign;
	 int vpdiff;
	uint32 eof, i;
	int step;

	bufferstep = 1;
	step = stepSizeTable[index];

	// Invokes the raw codec to read the stream from disk to loop. 
	eof = CODECRAW::decompress(stream, lpTemp, dwSize / 4);
	inp = lpTemp;

	eof *= 2;
	
	// If you must do an immediate loop
	if (endOfStream() && eof == 0) {
		loopReset();
		bufferstep = 1;
		step = stepSizeTable[index];
	} else if (!endOfStream())
		eof = 0;

	dwSize /= 2;
	for (i = 0; i < dwSize; i++) {
		// Check if we are at the end of the file, and are looping
		if (eof != 0 && i == eof) {
			loopReset();
			bufferstep=1;
			step = stepSizeTable[index];
		}

		// Read the delta (4 bits)
		if (bufferstep) {
			cache = *inp++;
			delta = (cache >> 4) & 0xF;
		} else
			delta = cache & 0xF;

		// Find the new index
		index += indexTable[delta];
		if (index < 0) index = 0;
		if (index > 88) index = 88;

		// Reads the sign and separates it
		sign = delta & 8;
		delta = delta & 7;

		// Find the difference from the previous value
		vpdiff = step >> 3;
		if (delta & 4) vpdiff += step;
		if (delta & 2) vpdiff += step >> 1;
		if (delta & 1) vpdiff += step >> 2;

		if (sign)
			valpred -= vpdiff;
		else
			valpred += vpdiff;

		// Check the limits of the found value
		if (valpred > 32767)
			valpred = 32767;
		else if (valpred < -32768)
			valpred = -32768;

		// Update the step
		step = stepSizeTable[index];

		// Write the value found
		*lpBuf++ = (uint16)valpred;

		bufferstep = !bufferstep;
	}

	return eof / 2;
}

uint32 CODECADPCMSTEREO::decompress(Common::SeekableReadStream *stream, void *buf, uint32 dwSize) {
	uint16 *lpBuf=(uint16 *)buf;
	byte *inp;
	int bufferstep;
	int cache = 0;
	int delta;
	int sign;
	int vpdiff;
	uint32 eof, i;
	int step[2];

	bufferstep = 1;
	step[0] = stepSizeTable[index[0]];
	step[1] = stepSizeTable[index[1]];

	// Invokes the RAW codec to read the stream from disk.
	eof = CODECRAW::decompress(stream, lpTemp, dwSize / 4);
	inp = lpTemp;

	eof *= 2;
	
	// If you must do an immediate loop 
	if (endOfStream() && eof == 0) {
		loopReset();
		bufferstep = 1;
		step[0] = stepSizeTable[index[0]];
		step[1] = stepSizeTable[index[1]];

	} else if (!endOfStream())
		eof = 0;

	dwSize /= 2;

	for (i = 0;i < dwSize; i++) {
		// If you must do an immediate loop
		if (eof != 0 && i == eof) {
			loopReset();
			bufferstep = 1;
			step[0] = stepSizeTable[index[0]];
			step[1] = stepSizeTable[index[1]];
		}

		// Reads the delta (4 bits)
		if (bufferstep) {
			cache = *inp++;
			delta = cache & 0xF;
		} else
			delta = (cache >> 4) & 0xF;

		// Find the new index
		index[bufferstep] += indexTable[delta];
		if (index[bufferstep] < 0) index[bufferstep] = 0;
		if (index[bufferstep] > 88) index[bufferstep] = 88;

		// Reads the sign and separates it
		sign = delta & 8;
		delta = delta & 7;

		// Find the difference from the previous value
		vpdiff = step[bufferstep] >> 3;
		if (delta & 4) vpdiff += step[bufferstep];
		if (delta & 2) vpdiff += step[bufferstep] >> 1;
		if (delta & 1) vpdiff += step[bufferstep] >> 2;

		if (sign)
			valpred[bufferstep] -= vpdiff;
		else
			valpred[bufferstep] += vpdiff;

		// Check the limits of the value
		if (valpred[bufferstep] > 32767)
			valpred[bufferstep] = 32767;
		else if (valpred[bufferstep] < -32768)
			valpred[bufferstep] =- 32768;

		// Update the step
		step[bufferstep] = stepSizeTable[index[bufferstep]];

		// Write the found value
		*lpBuf++ = (uint16)valpred[bufferstep];

		bufferstep = !bufferstep;
	}

	return eof / 2;
}

/****************************************************************************\
*       FPSOUND Methods
\****************************************************************************/

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
 * @param _bLoop         True to enable the loop, False to disable
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
	hwnd=hWnd;
	lpDS = LPDS;
	_lpDSBuffer = NULL;
	_lpDSNotify = NULL;
#endif
	_bSoundSupported = bSoundOn;
	_bFileLoaded = false;
	_bIsPlaying = false;
	_bPaused = false;
	_bLoop = false;
	_bDoFadeOut = false;
	_bSyncExit = false;
	_hHot1 = _hHot2 = _hHot3 = CORO_INVALID_PID_VALUE;
	_hPlayThread = _hPlayThreadPlayFast = _hPlayThreadPlayNormal = CORO_INVALID_PID_VALUE;
	_hThreadEnd = CORO_INVALID_PID_VALUE;
	_dwBufferSize = _dwSize = 0;
	_dwCodec = 0;
	_lastVolume = 0;
	_lpTempBuffer = NULL;
	_syncToPlay = NULL;
	_codec = NULL;
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
	if (!_bSoundSupported)
		return;

	if (_bIsPlaying)
		stop();

	if (_bFileLoaded)
		unloadFile();

	if (_hHot1) {
		CoroScheduler.closeEvent(_hHot1);
		_hHot1 = CORO_INVALID_PID_VALUE;
	}
	if (_hHot2) {
		CoroScheduler.closeEvent(_hHot2);
		_hHot2 = CORO_INVALID_PID_VALUE;
	}
	if (_hHot3) {
		CoroScheduler.closeEvent(_hHot3);
		_hHot3 = CORO_INVALID_PID_VALUE;
	}
	if (_hPlayThreadPlayFast != CORO_INVALID_PID_VALUE) {
		CoroScheduler.closeEvent(_hPlayThreadPlayFast);
		_hPlayThreadPlayFast = CORO_INVALID_PID_VALUE;
	}
	if (_hPlayThreadPlayNormal != CORO_INVALID_PID_VALUE) {
		CoroScheduler.closeEvent(_hPlayThreadPlayNormal);
		_hPlayThreadPlayNormal = CORO_INVALID_PID_VALUE;
	}

	_syncToPlay = NULL;
#ifdef REFACTOR_ME
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
 * @param fileName      Filename to be opened
 * @param dwCodec       CODEC to be used to uncompress samples
 *
 * @returns             True is everything is OK, False otherwise
 */
bool FPStream::loadFile(const Common::String &fileName, uint32 dwCodType, int nBufSize) {
	if (!_bSoundSupported)
		return true;

	// Save the codec type
	_dwCodec = dwCodType;

	// Create the buffer
	if (!createBuffer(nBufSize))
		return true;

	// Open the file stream for reading
	if (!_file.open(fileName))
		return false;

	// Save the size of the stream
	_dwSize = _file.size();

	// All done
	_bFileLoaded = true;
	_bIsPlaying = false;
	_bPaused = false;

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
	if (!_bSoundSupported || !_bFileLoaded)
		return true;

	/* Closes the file handle stream */
	_file.close();
#ifdef REFACTOR_ME
	RELEASE(lpDSNotify);
	RELEASE(lpDSBuffer);
#endif

	// Flag that the file is no longer in memory
	_bFileLoaded = false;

	return true;
}

void FPStream::prefetch() {
	void *lpBuf = NULL;

	if (!_bSoundSupported || !_bFileLoaded)
		return;

	// Allocates a temporary buffer
	_lpTempBuffer = (byte *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, _dwBufferSize / 2);
	if (_lpTempBuffer == NULL)
		return;

	if (_dwCodec == FPCODEC_RAW) {
		_codec = new CODECRAW(_bLoop);
	} else if (_dwCodec == FPCODEC_ADPCM) {
		_codec = new CODECADPCMSTEREO(_bLoop);
	} else {
		return;
	}

	// reset the file position
	_file.seek(0);

#ifdef REFACTOR_ME
	// Fills the buffer for the data already ready
	if ((err = lpDSBuffer->Lock(0, dwBufferSize / 2, &lpBuf, (uint32 *)&dwHi, NULL, NULL, 0)) != DS_OK) {
		_vm->GUIError("Cannot lock stream buffer!", "soundLoadStream()");
		return;
	}
#else
	// Return, since lpBuf hasn't been set
	return;
#endif

	// Uncompress the data from the stream directly into the locked buffer
	_codec->decompress(_file.readStream(_file.size()), lpBuf, _dwBufferSize / 2);

	// Unlock the buffer
#ifdef REFACTOR_ME
	lpDSBuffer->unlock(lpBuf, _dwBufferSize / 2, NULL, NULL);
#endif

	// Create a thread to play the stream
	_hThreadEnd = CoroScheduler.createEvent(false, false);
	_hPlayThread = CoroScheduler.createProcess(playThread, this, sizeof(FPStream *));
	
	// Start to play the buffer
#ifdef REFACTOR_ME
	lpDSBuffer->setCurrentPosition(0);
#endif
	_bIsPlaying = true;

#ifdef REFACTOR_ME
	_dspnHot[0].dwOffset = 32;
	_dspnHot[0].hEventNotify = _hHot1;

	_dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
	_dspnHot[1].hEventNotify = _hHot2;

	_dspnHot[2].dwOffset = dwBufferSize - 32;   //DSBPN_OFFSETSTOP;
	_dspnHot[2].hEventNotify = _hHot3;

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
		_codec = new CODECRAW(_bLoop);
		break;

	case FPCODEC_ADPCM:
		_codec = new CODECADPCMSTEREO(_bLoop);
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
	_codec->Decompress(hFile, lpBuf, dwBufferSize / 2);

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
		delete _codec;

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
	delete _codec;
#endif
	_bIsPlaying = false;
}

/**
 * Thread playing the stream
 *
 */

void FPStream::playThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		byte *lpLockBuf;
		uint32 dwResult;
		byte *lpLockBuf2;
		uint32 dwResult2;
		bool cicla;
		uint32 countEnd;
		bool bPrecache;
		char buf[1024];
		uint32 hList[5];
	CORO_END_CONTEXT(_ctx);

//	FPStream *This = *(FPStream **)param;

	CORO_BEGIN_CODE(_ctx);
#ifdef REFACTOR_ME
	// Events that signal when you need to do something
	_ctx->hList[0] = This->_hThreadEnd;
	_ctx->hList[1] = This->_hHot1;
	_ctx->hList[2] = This->_hHot2;
	_ctx->hList[3] = This->_hHot3;
	_ctx->hList[4] = This->_hPlayThreadPlayFast;

	_ctx->cicla = true;
	_ctx->bPrecache = true;
	_ctx->countEnd = 0;
	while (_ctx->cicla) {
		if (This->_codec->endOfStream() && This->_codec->_bLoop == false) {
			_ctx->countEnd++;
			if (_ctx->countEnd == 3)
				break;
		}

		// Uncompresses the data being written into the temporary buffer
		if (This->_lastVolume == 0)
			ZeroMemory(This->_lpTempBuffer, This->_dwBufferSize / 2);
		else if (_ctx->bPrecache)
			This->_codec->decompress(This->_file.readStream(This->_file.size()), This->_lpTempBuffer, This->_dwBufferSize / 2);

		_ctx->bPrecache = false;

		// Waits for an event. Since they are all in automatic reset, there is no need to reset it after

		uint32 dwBufStatus;

		CORO_INVOKE_4(CoroScheduler.waitForMultipleObjects, 5, _ctx->hList, false, CORO_INFINITE, &_ctx->dwResult);

		// Check to determine which event has been set
		if (CoroScheduler.getEvent(This->_hThreadEnd)->signalled) {
			/* Must leave the thread */
			_ctx->cicla = false;
			
		} else if (CoroScheduler.getEvent(This->_hHot1)->signalled) {
			// Must fill the second half of the buffer
			This->lpDSBuffer->Lock(This->_dwBufferSize / 2, This->_dwBufferSize / 2, (void **)&_ctx->lpLockBuf, &_ctx->dwResult, (void **)&_ctx->lpLockBuf2, &_ctx->dwResult2, 0);

			copyMemory(_ctx->lpLockBuf, This->_lpTempBuffer, This->_dwBufferSize / 2);
			This->lpDSBuffer->Unlock(_ctx->lpLockBuf, This->_dwBufferSize / 2, _ctx->lpLockBuf2, 0);

			_ctx->bPrecache = true;

		} else if (CoroScheduler.getEvent(This->_hHot2)->signalled) {
			This->lpDSBuffer->Lock(0, This->_dwBufferSize / 2, (void **)&_ctx->lpLockBuf, &_ctx->dwResult, NULL, NULL, 0);

			copyMemory(_ctx->lpLockBuf, This->_lpTempBuffer, This->_dwBufferSize / 2);
			This->lpDSBuffer->Unlock(_ctx->lpLockBuf, This->_dwBufferSize / 2, NULL, NULL);

			_ctx->bPrecache = true;

		} else if (CoroScheduler.getEvent(This->_hHot3)->signalled) {
		
			if (This->_bSyncExit) {
				CoroScheduler.setEvent(This->_syncToPlay->_hPlayThreadPlayFast);

				// Transfer immediatly control to the other threads
				CORO_SLEEP(1);

				This->_bSyncExit = false;
				_ctx->cicla = false;
				break;
			}
		} else if (CoroScheduler.getEvent(This->_hPlayThreadPlayFast)->signalled) {
			This->playFast();
		}
	}

	// Close the DirectSound buffer
	This->lpDSBuffer->Stop();
#endif

	CORO_END_CODE;
}


/**
 * Unables or disables stream loop.
 *
 * @param _bLoop         True enable loop, False disables it
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
	if (_bFileLoaded) {
		if (bPause && _bIsPlaying) {
#ifdef REFACTOR_ME
			_lpDSBuffer->Stop();
#endif
			_bIsPlaying = false;
			_bPaused = true;
		} else if (!bPause && _bPaused) {
#ifdef REFACTOR_ME
			_dspnHot[0].dwOffset = 32;
			_dspnHot[0].hEventNotify = hHot1;

			_dspnHot[1].dwOffset = dwBufferSize / 2 + 32;
			_dspnHot[1].hEventNotify = hHot2;

			_dspnHot[2].dwOffset = dwBufferSize - 32; //DSBPN_OFFSETSTOP;
			_dspnHot[2].hEventNotify = hHot3;

			if (FAILED(lpDSNotify->SetNotificationPositions(3, dspnHot))) {
				int a = 1;
			}

			lpDSBuffer->Play(0, 0, _bLoop);
#endif
			_bIsPlaying = true;
			_bPaused = false;

			// Trick to reset the volume after a possible new sound configuration
			setVolume(_lastVolume);
		}
	}
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
