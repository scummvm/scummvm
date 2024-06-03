/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA8_GFX_AVIPLAYER_H
#define ULTIMA8_GFX_AVIPLAYER_H

#include "ultima/ultima8/gfx/movie_player.h"
#include "graphics/managed_surface.h"

namespace Video {
class AVIDecoder;
}

namespace Ultima {
namespace Ultima8 {

class AVIPlayer : public MoviePlayer {
public:
	//!
	//! Create a new player for the given stream at the given size.  Playback is
	//! automatically scaled up using the line-skip style from Crusader, unless
	//! noScale flag is set.
	//! If overridePal is set, use that as the palette instead of the AVI's one.
	//!
	AVIPlayer(Common::SeekableReadStream *rs, int width, int height, const byte *overridePal, bool noScale);
	~AVIPlayer();

	void run() override;
	void paint(RenderSurface *surf, int lerp) override;

	void start() override;
	void stop() override;
	bool isPlaying() const override {
		return _playing;
	}

	int getFrameNo() const;

	// Adjust the offsets by the given values
	void setOffset(int xoff, int yoff) override;

private:

	bool _playing;
	Video::AVIDecoder *_decoder;
	Graphics::ManagedSurface _currentFrame;
	// Width and height of the area we've been given to play back in
	uint32 _width;
	uint32 _height;
	// Xoff and Yoff into that playback area
	uint32 _xoff;
	uint32 _yoff;
	bool _doubleSize;
	const byte *_overridePal;
	bool _pausedMusic;

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
