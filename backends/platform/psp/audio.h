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

#ifndef PSP_AUDIO_H
#define PSP_AUDIO_H

class PspAudio {
public:
	enum {
		NUM_BUFFERS = 2,
		FREQUENCY = 44100	/* only frequency we allow */
	};
	typedef void (* callbackFunc)(void *userData, byte *samples, int len);
	PspAudio() : _pspChannel(0), 
			_numOfChannels(0), _numOfSamples(0), _callback(0), 
			_bufferToPlay(0), _bufferToFill(0), _emptyBuffers(NUM_BUFFERS), 
			_init(false), _paused(true), _stoppedPlayingOnceFlag(true) {
		for (int i=0; i<NUM_BUFFERS; i++)
			_buffers[i] = 0;
	}
	~PspAudio() { close(); }
	bool playBuffer();
	void nextBuffer(int &bufferIdx);
	static int thread(SceSize, void *);
	void audioThread();
	bool open(uint32 freq, uint32 numOfChannels, uint32 numOfSamples, callbackFunc callback, void *userData);
	bool createThread();
	void close();
	uint32 getFrequency() { return FREQUENCY; }
	void pause() { _paused = true; }
	void unpause() { _paused = false; }
	
private:
	int _pspChannel;				// chosen hardware output channel
	uint32 _numOfChannels;			// 1 for mono; 2 for stereo
	uint32 _numOfSamples;
	callbackFunc _callback;			// the callback to call between outputting audio
	void *_userData;				// userData to send with callback
	byte *_buffers[NUM_BUFFERS];
	int _bufferToPlay;				// the next buffer to output
	int _bufferToFill;
	int _bufferSize;
	int _emptyBuffers;
	bool _init;						// flag for initialization
	bool _paused;
	bool _stoppedPlayingOnceFlag;		// used to make sure we know when the playing stopped
};

#endif /* PSP_AUDIO_H */
