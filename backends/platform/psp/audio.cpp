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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/osys_psp.cpp $
 * $Id: osys_psp.cpp 46126 2009-11-24 14:18:46Z fingolfin $
 *
 */

#include <SDL.h> 
#include <pspthreadman.h> 
#include <pspaudio.h>
 
#include "common/scummsys.h" 
#include "backends/platform/psp/audio.h"
#include "backends/platform/psp/thread.h"

//#define __PSP_DEBUG_FUNCS__	/* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */

#include "backends/platform/psp/trace.h"

bool PspAudio::open(uint32 freq, uint32 numOfChannels, uint32 numOfSamples, callbackFunc callback, void *userData) {
	DEBUG_ENTER_FUNC();
	if (_init) {
		PSP_ERROR("audio device already initialized\n");
		return true;
	}		

	PSP_DEBUG_PRINT("freq[%d], numOfChannels[%d], numOfSamples[%d], callback[%p], userData[%x]\n", 
			freq, numOfChannels, numOfSamples, callback, (uint32)userData);
	
	numOfSamples = PSP_AUDIO_SAMPLE_ALIGN(numOfSamples);
	uint32 bufLen = numOfSamples * numOfChannels * NUM_BUFFERS * sizeof(uint16);
	
	PSP_DEBUG_PRINT("total buffer size[%d]\n", bufLen);
		
	_buffers[0] = (byte *)memalign(64, bufLen);
	if (!_buffers[0]) {
		PSP_ERROR("failed to allocate memory for audio buffers\n");
		return false;
	}
	memset(_buffers[0], 0, bufLen);	// clean the buffer
	
	// Fill in the rest of the buffer pointers
	byte *pBuffer = _buffers[0];
	for (int i = 1; i < NUM_BUFFERS; i++) {
		pBuffer += numOfSamples * numOfChannels * sizeof(uint16);
		_buffers[i] = pBuffer;
	}
	
	// Reserve a HW channel for our audio
	_pspChannel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, numOfSamples, numOfChannels == 2 ? PSP_AUDIO_FORMAT_STEREO : PSP_AUDIO_FORMAT_MONO);
	if (_pspChannel < 0) {
		PSP_ERROR("failed to reserve audio channel\n");
		return false;
	}
	
	PSP_DEBUG_PRINT("reserved channel[%d] for audio\n", _pspChannel);

	// Save our data
	_numOfChannels = numOfChannels;
	_numOfSamples = numOfSamples;
	_bufferSize = numOfSamples * numOfChannels * sizeof(uint16);	// should be the right size to send the app
	_callback = callback;
	_userData = userData;
	_emptyBuffers = NUM_BUFFERS;
	_bufferToFill = 0;
	_bufferToPlay = 0;
	
	_init = true;
	_paused = true;	// start in paused mode
	
	createThread();
	
	return true;
}

bool PspAudio::createThread() {
	DEBUG_ENTER_FUNC();
	int threadId = sceKernelCreateThread("audioThread", thread, PRIORITY_AUDIO_THREAD, STACK_AUDIO_THREAD, THREAD_ATTR_USER, 0); 
	
	if (threadId < 0) {	// error
		PSP_ERROR("failed to create audio thread. Error code %d\n", threadId);
		return false;
	}
	
	PspAudio *_this = this;	// trick to get into context when the thread starts
	
	if (sceKernelStartThread(threadId, sizeof(uint32 *), &_this) < 0) {
		PSP_ERROR("failed to start thread %d\n", threadId);
		return false;
	}
	
	PSP_DEBUG_PRINT("created audio thread[%x]\n", threadId);
	
	return true;
}

// Static function to be called upon thread startup. Will call a non-static function	
int PspAudio::thread(SceSize, void *__this) {
	DEBUG_ENTER_FUNC();
	PspAudio *_this = *(PspAudio **)__this;		// get our this for the context
	
	_this->audioThread();
	return 0;
};

// The real thread function
void PspAudio::audioThread() {
	bool isPlaying = false;
	int remainingSamples = 0;
	
	assert(_callback);
	PSP_DEBUG_PRINT_FUNC("audio thread started\n");

	while (_init) {		// Keep looping so long as we haven't been told to stop
		if (_paused)
			PSP_DEBUG_PRINT("audio thread paused\n");
		while (_paused) {	// delay until we stop pausing
			SDL_Delay(100);
		}
		if (!_paused)
			PSP_DEBUG_PRINT("audio thread unpaused\n");

		// check if the audio is playing
		remainingSamples = sceAudioGetChannelRestLen(_pspChannel);
		if (remainingSamples < 0) {
			PSP_ERROR("failed to get remaining samples\n");
			return;
		}
		isPlaying = remainingSamples ? true : false;
		
		PSP_DEBUG_PRINT("remaining samples[%d]\n", remainingSamples);
		
		while (true) {	// really only execute once. this just helps write the logic
			if (isPlaying) {
				_stoppedPlayingOnceFlag = false;
			
				// check if a buffer is empty
				if (_emptyBuffers)	{ // we have some empty buffers
					PSP_DEBUG_PRINT("sound playing & an empty buffer. filling buffer[%d]. empty buffers[%d]\n", _bufferToFill, _emptyBuffers);
					_callback(_userData, _buffers[_bufferToFill], _bufferSize); // ask mixer to fill in
					nextBuffer(_bufferToFill);
					_emptyBuffers--;
					break;
				} else { // we have no empty buffers
					// calculate how long we need to sleep. time(us) = samples * 1000000 / freq
					// since frequency is always 44100, we can do a shortcut:
					// time(us) = samples * (10000 / 441)
					uint32 sleepTime = (remainingSamples * 10000) / 441;
					if (!sleepTime)
						break;
					PSP_DEBUG_PRINT("sound playing & no empty buffers. sleeping for %d samples for %dus\n", remainingSamples, sleepTime);	
					sceKernelDelayThread(sleepTime);
					break;
				}
			} else {	// we're not playing right now
				if (_stoppedPlayingOnceFlag == false) {	// we only want to do this when we finish playing
					nextBuffer(_bufferToPlay);
					_emptyBuffers++;
					_stoppedPlayingOnceFlag = true;
				}	
			
				if (_emptyBuffers == NUM_BUFFERS) { // problem: we have only empty buffers!
					PSP_DEBUG_PRINT("no sound playing & no full buffer. filling buffer[%d]. empty buffers[%d]\n", _bufferToFill, _emptyBuffers);
					_callback(_userData, _buffers[_bufferToFill], _bufferSize);
					nextBuffer(_bufferToFill);
					_emptyBuffers--;
					break;
				} else { // we have at least one non-empty buffer
					PSP_DEBUG_PRINT("no sound playing & a full buffer. playing buffer[%d]. empty buffers[%d]\n", _bufferToPlay, _emptyBuffers);
					playBuffer();
					break;
				}
			}
		} // while true	
	} // while _init
	
	// destroy everything
	free(_buffers[0]);
	sceAudioChRelease(_pspChannel);
	PSP_DEBUG_PRINT("audio thread exiting. ****************************\n");
}

// Much faster than using %
inline void PspAudio::nextBuffer(int &bufferIdx) {
	DEBUG_ENTER_FUNC();
	bufferIdx++;
	if (bufferIdx >= NUM_BUFFERS)
		bufferIdx = 0;
}

// Don't do it with blocking
inline bool PspAudio::playBuffer() {
	DEBUG_ENTER_FUNC();
	int ret;
	if (_numOfChannels == 1)
		ret = sceAudioOutput(_pspChannel, PSP_AUDIO_VOLUME_MAX, _buffers[_bufferToPlay]);
	else
		ret = sceAudioOutputPanned(_pspChannel, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, _buffers[_bufferToPlay]);
	
	if (ret < 0) {
		PSP_ERROR("failed to output audio. Error[%d]\n", ret);
		return false;
	}
	return true;
}

void PspAudio::close() {
	PSP_DEBUG_PRINT("close had been called ***************\n");
	_init = false; 
}
