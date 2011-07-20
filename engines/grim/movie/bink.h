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
 */

#ifndef GRIM_BINK_PLAYER_H
#define GRIM_BINK_PLAYER_H

#include "common/scummsys.h"
#include "common/file.h"

#include "graphics/pixelformat.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

#include "engines/grim/movie/movie.h"

namespace Video{
	class BinkDecoder;
}
namespace Graphics{
	struct Surface;
}

namespace Grim {

class BinkPlayer : public MoviePlayer {
private:
	Common::File _f;
	Video::BinkDecoder *_binkDecoder;
	Graphics::Surface *_surface;
public:
	BinkPlayer();
	~BinkPlayer();

	bool play(const char *filename, bool looping, int x, int y);
	void stop();
	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);
	void deliverFrameFromDecode(int width, int height, uint16 *dat);
private:
	static void timerCallback(void *ptr);
	virtual void handleFrame();
	void init();
	void deinit();
};

} // end of namespace Grim


#endif
