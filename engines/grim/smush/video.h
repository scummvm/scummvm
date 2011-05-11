/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_VIDEO_PLAYER_H
#define GRIM_VIDEO_PLAYER_H

#ifdef __SYMBIAN32__
#include <zlib\zlib.h>
#else
#include <zlib.h>
#endif

#include "common/file.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Grim {

class SaveGame;

struct SavePos {
	uint32 filePos;
	z_stream streamBuf;
	byte *tmpBuf;
};

class VideoPlayer {
protected:
	Common::File _f;
	Common::String _fname;

	int32 _frame;
	bool _updateNeeded;
	int32 _speed;
	int32 _movieTime;
	int _channels;
	int _freq;
	bool _videoFinished;
	bool _videoPause;
	bool _videoLooping;
	struct SavePos *_startPos;
	int _x, _y;
	int _width, _height;
	byte *_internalBuffer, *_externalBuffer;

public:
	VideoPlayer(){
		_internalBuffer = NULL;
		_externalBuffer = NULL;
		_width = 0;
		_height = 0;
		_speed = 0;
		_channels = -1;
		_freq = 22050;
		_videoFinished = false;
		_videoLooping = false;
		_videoPause = true;
		_updateNeeded = false;
		_startPos = NULL;
		//	_stream = NULL;
		_movieTime = 0;
		_frame = 0;
	};
	virtual ~VideoPlayer(){
	}
	virtual bool play(const char *filename, bool looping, int x, int y) = 0;
	virtual void stop() = 0;
	virtual void pause(bool p) { _videoPause = p; }
	virtual bool isPlaying() { return !_videoFinished; }
	virtual bool isUpdateNeeded() { return _updateNeeded; }
	virtual byte *getDstPtr() { return _externalBuffer; }
	virtual int getX() { return _x; }
	virtual int getY() { return _y; }
	virtual int getWidth() {return _width; }
	virtual int getHeight() { return _height; }
	virtual int getFrame() { return _frame; }
	virtual void clearUpdateNeeded() { _updateNeeded = false; }
	virtual int32 getMovieTime() { return _movieTime; }

	virtual void saveState(SaveGame *state) = 0;
	virtual void restoreState(SaveGame *state) = 0;
	
protected:
	static void timerCallback(void *ptr);
	virtual void handleFrame() = 0;
	virtual void init() = 0;
	virtual void deinit() = 0;
};

// Factory-like functions:
	
VideoPlayer* CreateMPEGPlayer();	
VideoPlayer* CreateSMUSHPlayer();
VideoPlayer* CreateBinkPlayer();
extern VideoPlayer *g_video;

} // end of namespace Grim

#endif
