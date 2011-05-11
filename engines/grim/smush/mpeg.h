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

#ifndef GRIM_MPEG_PLAYER_H
#define GRIM_MPEG_PLAYER_H
#include "common/scummsys.h"
#include "graphics/pixelformat.h"
#ifdef __SYMBIAN32__
#include <zlib\zlib.h>
#else
#include <zlib.h>
#endif

#include "common/file.h"

#include "engines/grim/smush/video.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "video/mpeg_player.h"

namespace Grim {
	

class MPEG_Player : public VideoPlayer{
private:
	int32 _nbframes;
	Common::File _f;

	Video::BaseAnimationState* bas;
	Common::String _fname;
	bool _isPlaying;
	
public:
	MPEG_Player();
	~MPEG_Player();

	virtual bool play(const char *filename, bool looping, int x, int y);
	virtual void stop();
	virtual bool isPlaying() { return !_isPlaying; }
	virtual void saveState(SaveGame *state);
	virtual void restoreState(SaveGame *state);
	void deliverFrameFromDecode(int width, int height, uint16 *dat);
private:
	static void timerCallback(void *ptr);
	void parseNextFrame();
	void handleDeltaPalette(byte *src, int32 size);
	void handleFramesHeader();
	void handleFrameDemo();
	virtual void handleFrame();
	void handleWave(const byte *src, uint32 size);
	void init();
	void deinit();
	bool setupAnim(const char *file, bool looping, int x, int y);
	bool setupAnimDemo(const char *file);
};

} // end of namespace Grim


#endif
