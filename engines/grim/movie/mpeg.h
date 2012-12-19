/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_MPEG_PLAYER_H
#define GRIM_MPEG_PLAYER_H

#include "common/scummsys.h"
#include "common/file.h"

#include "graphics/pixelformat.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

#include "video/mpeg_player.h"

#include "engines/grim/movie/movie.h"

#ifdef USE_MPEG2

namespace Grim {

class MpegPlayer : public MoviePlayer {
private:
	Video::BaseAnimationState *_videoBase;
	Audio::SoundHandle _soundHandle;
	Audio::QueuingAudioStream *_stream;
	int _speed;	// <- Quickfix to fix compile, verify when fixing the decoder properly.
public:
	MpegPlayer();

	void deliverFrameFromDecode(int width, int height, uint16 *dat);
private:
	void handleFrame();
	void init();
	void deinit();
	bool loadFile(Common::String filename);
};

} // end of namespace Grim

#endif // USE_MPEG2

#endif
