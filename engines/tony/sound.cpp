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
	_hEndOfBuffer = CoroScheduler.createEvent(true, false);
	_bIsVoice = false;
	_loopStream = 0;
	_rewindableStream = 0;
	_bPaused = false;

	g_vm->_activeSfx.push_back(this);
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
	g_vm->_activeSfx.remove(this);

	if (_loopStream)
		delete _loopStream; // _rewindableStream is deleted by deleting _loopStream
	else
		delete _rewindableStream;

	// Free the buffer end event
	CoroScheduler.closeEvent(_hEndOfBuffer);
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
		CoroScheduler.resetEvent(_hEndOfBuffer);

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
		for (_ctx->i = g_vm->_activeSfx.begin(); _ctx->i != g_vm->_activeSfx.end(); ++_ctx->i) {
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
	_bSoundSupported = bSoundOn;
	_bFileLoaded = false;
	_bPaused = false;
	_bLoop = false;
	_bDoFadeOut = false;
	_bSyncExit = false;
	_dwBufferSize = _dwSize = 0;
	_lastVolume = 0;
	_syncToPlay = NULL;
	_loopStream = NULL;
	_rewindableStream = NULL;
}

/**
 * Default destructor. 
 *
 * @remarks             It calls CloseFile() if needed.
 */

FPStream::~FPStream() {
	if (!_bSoundSupported)
		return;

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		stop();

	if (_bFileLoaded)
		unloadFile();

	_syncToPlay = NULL;
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

	if (_bFileLoaded)
		unloadFile();

	// Save the codec type
	_dwCodec = dwCodType;

	// Open the file stream for reading
	if (!_file.open(fileName)) {
		// Fallback: try with an extra '0' prefix
		if (!_file.open("0" + fileName))
			return false;
	}

	// Save the size of the stream
	_dwSize = _file.size();

	switch (_dwCodec) {
	case FPCODEC_RAW:
		_rewindableStream = Audio::makeRawStream(&_file, 44100, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO, DisposeAfterUse::NO);
		break;

	case FPCODEC_ADPCM:
		_rewindableStream = Audio::makeADPCMStream(&_file, DisposeAfterUse::NO, 0, Audio::kADPCMDVI, 44100, 2);
		break;

	default:
		_file.close();
		return false;
	}

	// All done
	_bFileLoaded = true;
	_bPaused = false;

	setVolume(63);

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

	assert(!g_system->getMixer()->isSoundHandleActive(_handle));

	/* Closes the file handle stream */
	delete _loopStream;
	delete _rewindableStream;
	_loopStream = NULL;
	_rewindableStream = NULL;
	_file.close();

	// Flag that the file is no longer in memory
	_bFileLoaded = false;

	return true;
}

/**
 * Play the stream.
 *
 * @returns             True is everything is OK, False otherwise
 */

bool FPStream::play() {
	if (!_bSoundSupported || !_bFileLoaded)
		return false;

	stop();

	_rewindableStream->rewind();

	Audio::AudioStream *stream = _rewindableStream;

	if (_bLoop) {
		if (!_loopStream)
			_loopStream = new Audio::LoopingAudioStream(_rewindableStream, 0, DisposeAfterUse::NO);

		stream = _loopStream;
	}

	// FIXME: Should this be kMusicSoundType or KPlainSoundType?
	g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_handle, stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	setVolume(_lastVolume);
	_bPaused = false;

	return true;
}

/**
 * Closes the stream.
 *
 * @returns             True is everything is OK, False otherwise
 * 
 */

bool FPStream::stop(bool bSync) {
	if (!_bSoundSupported)
		return true;

	if (!_bFileLoaded)
		return false;

	if (!g_system->getMixer()->isSoundHandleActive(_handle))
		return false;

	if (bSync) {
		// The caller intends to call waitForSync.
		// FIXME: Why call this in that case?! Looks like old code. Remove that parameter.
		return true;
	} else {
		g_system->getMixer()->stopHandle(_handle);

		_bPaused = false;
	}

	return true;
}

void FPStream::waitForSync(FPStream *toplay) {
	// FIXME: The idea here is that you wait for this stream to reach
	// a buffer which is a multiple of nBufSize/nSync, and then the
	// thread stops it and immediately starts the 'toplay' stream.

	stop();
	toplay->play();
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
	if (!_bFileLoaded)
		return;

	if (bPause == _bPaused)
		return;

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->pauseHandle(_handle, bPause);

	_bPaused = bPause;

	// Trick to reset the volume after a possible new sound configuration
	setVolume(_lastVolume);
}

/**
 * Change the volume of the stream
 *
 * @param dwVolume      Volume to be set (0-63)
 *
 */

void FPStream::setVolume(int dwVolume) {
	if (dwVolume > 63) dwVolume = 63;
	if (dwVolume < 0) dwVolume = 0;

	_lastVolume = dwVolume;

	if (!GLOBALS._bCfgMusic) dwVolume = 0;
	else {
		dwVolume -= (10 - GLOBALS._nCfgMusicVolume) * 2;
		if (dwVolume < 0) dwVolume = 0;
	}

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->setChannelVolume(_handle, dwVolume * Audio::Mixer::kMaxChannelVolume / 63);
}

/**
 * Gets the volume of the stream
 *
 * @param lpdwVolume    Variable that will contain the current volume
 *
 */

void FPStream::getVolume(int *lpdwVolume) {
	if (g_system->getMixer()->isSoundHandleActive(_handle))
		*lpdwVolume = g_system->getMixer()->getChannelVolume(_handle) * 63 / Audio::Mixer::kMaxChannelVolume;
	else
		*lpdwVolume = 0;
}

} // End of namespace Tony
