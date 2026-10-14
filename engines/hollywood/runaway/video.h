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

#ifndef HOLLYWOOD_RUNAWAY_VIDEO_H
#define HOLLYWOOD_RUNAWAY_VIDEO_H

#include "common/array.h"
#include "graphics/managed_surface.h"
#include "video/avi_decoder.h"

namespace Hollywood {

// Silent AVI clips are advanced by the engine clock so menu pauses also
// freeze frame callbacks and the separately stored room sound effects.
class RunawayVideo {
public:
	bool load(const char *name);
	void close();
	bool decodeToFrame(uint frame);
	uint frameCount() const { return _decoder.getFrameCount(); }
	void draw(Graphics::ManagedSurface &surface, const Common::Rect &destination) const;

private:
	Common::Array<byte> _data;
	Video::AVIDecoder _decoder;
	const Graphics::Surface *_frame = nullptr;
};

} // namespace Hollywood

#endif
