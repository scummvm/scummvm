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
#include "common/ptr.h"
#include "hollywood/resource.h"
#include "hollywood/runaway/graphics.h"
#include "video/video_decoder.h"

namespace Hollywood {

// Silent AVI/Bink clips are advanced by the engine clock so menu pauses also
// freeze frame callbacks and the separately stored room sound effects.
class RunawayVideo {
public:
	enum FrameStatus { kFrameError, kFramePending, kFrameReady };
	bool load(const char *name);
	void close();
	bool decodeToFrame(uint frame);
	bool seekToFrame(uint frame);
	bool isBink() const;
	void cacheFrames(uint first, uint last);
	// A zero budget checks readiness without decoding; prefetch keeps the displayed frame.
	FrameStatus prepareFrame(uint frame, uint maxFrames);
	FrameStatus prefetchFrame(uint frame, uint maxFrames);
	uint width() const { return _frame ? _frame->w : 0; }
	uint height() const { return _frame ? _frame->h : 0; }
	uint frameCount() const { return _decoder ? _decoder->getFrameCount() : 0; }
	uint frameAtTime(uint32 milliseconds) const;
	void draw(Graphics::ManagedSurface &surface, const Common::Rect &destination) const;
	void drawRegion(Graphics::ManagedSurface &surface, const Common::Rect &source, const Common::Rect &destination) const;

private:
	bool useCachedFrame(uint frame);
	bool decodeFrames(uint frame);

	struct CachedFrame {
		bool enabled = false;
		Graphics::ManagedSurface surface;
	};
	Common::Array<CachedFrame> _cachedFrames;
	uint32 _cacheBytes = 0;
	Common::Array<byte> _data;
	Common::ScopedPtr<Video::VideoDecoder> _decoder;
	const Graphics::Surface *_frame = nullptr;
	const Graphics::Surface *_decodedFrame = nullptr;
	Graphics::ManagedSurface _heldFrame;
};

// The CD stores patch/mouth pairs; GOG stores the same poses as Bink frames.
class RunawayFilmPortrait {
public:
	bool load(char chapter, uint movie);
	void close();
	RunawayVideo::FrameStatus prepare(uint portrait, uint maxFrames);
	bool draw(Graphics::ManagedSurface &surface, const Common::Rect &destination, uint portrait, uint mouth);

private:
	RunawayVideo _video;
	ChunkArchive _archive;
	RunawayAnimation _mouth;
	Graphics::ManagedSurface _portrait, _talking;
	struct Portrait {
		uint firstFrame = 0;
		Common::Rect bounds;
		Common::Array<Graphics::ManagedSurface> faces;
	};
	Common::Array<Portrait> _portraits;
	bool _loaded = false;
	int _loadedPortrait = -1;
	uint _preparedFrames = 0;
	char _chapter = 'B';
	uint _movie = 0;
};

} // namespace Hollywood

#endif
