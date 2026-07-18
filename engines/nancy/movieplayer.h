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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_MOVIEPLAYER_H
#define NANCY_MOVIEPLAYER_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/path.h"
#include "common/rational.h"
#include "common/rect.h"

#include "audio/timestamp.h"

#include "graphics/managed_surface.h"

#include "engines/nancy/commontypes.h"

namespace Graphics {
struct Surface;
}

namespace Video {
class VideoDecoder;
}

namespace Nancy {

// The single low-level interface to the AVF/Bink video decoders. Every consumer
// (PlaySecondaryMovie, PlaySecondaryVideo, Conversation, Viewport, Map,
// BoardGamePuzzle, ...) owns one of these instead of a raw Video::VideoDecoder,
// so the AVF-vs-Bink specifics (decoder creation, .avf/.bik file selection, and
// the AVF-only behaviour of decodeFrame/addFrameTime/endOfVideo) live in one
// place. All other methods forward 1:1 to the underlying decoder, so playback
// behaviour is unchanged; higher-level playback/timing logic stays per consumer.
class MoviePlayer {
public:
	MoviePlayer();
	~MoviePlayer();

	// Load <name> + ".avf"/".bik", auto-detecting the format from which file
	// exists (AVF preferred if both do) and creating the matching decoder.
	// bidirectionalCache enables fast bidirectional scrubbing; pass it only for
	// scrubbed panorama scenes.
	bool loadFile(const Common::Path &name, bool bidirectionalCache = false);
	bool isVideoLoaded() const;
	void close();
	Video::VideoDecoder *getDecoder() { return _decoder.get(); }

	// Playback control - forwarded to the decoder.
	void start();
	void stop();
	void pauseVideo(bool pause);
	bool isPlaying() const;
	bool needsUpdate() const;
	bool endOfVideo() const;	// AVF uses AVFDecoder::atEnd()

	void seekToFrame(uint frame);
	void seek(const Audio::Timestamp &time);
	void rewind();
	void setRate(const Common::Rational &rate);
	Common::Rational getRate() const;
	void setReverse(bool reverse);

	int getCurFrame() const;
	int getFrameCount() const;
	Audio::Timestamp getDuration() const;
	uint16 getWidth() const;
	uint16 getHeight() const;
	void addFrameTime(uint16 timeToAdd);	// AVF only, no-op otherwise

	// Decode a frame: frameNr < 0 returns the next frame; otherwise that
	// specific frame via the format-appropriate cached path (AVF decodeFrame;
	// Bink decodes forward when possible and caches frames, see _frameCache).
	const Graphics::Surface *decodeNextFrame(int frameNr = -1);

	// --- Optional simple frame-range player, built on the above. Handy for
	// consumers (e.g. BoardGamePuzzle) that just want to show a still frame or
	// play [first, last] over time. Uses its own, no-skip timing so it starts
	// immediately and stays smooth, and decodes into an internal surface.
	void goToFrame(int frameNr);				// show a single still frame
	void playRange(int first, int last);		// begin timed playback (reverse if last < first)
	bool update();								// advance; true if the shown frame changed
	bool isRangePlaying() const { return _rangePlaying; }
	int getCurrentFrame() const { return getCurFrame(); }
	void drawFrame(Graphics::ManagedSurface &dst, const Common::Point &pos) const;

private:
	void storeCurrentFrame();
	void freeFrameCache();

	Common::ScopedPtr<Video::VideoDecoder> _decoder;
	byte _videoType = kVideoPlaytypeAVF;

	// Decoded-frame cache for the Bink path (AVF caches internally). Bink seeking
	// re-decodes from the previous keyframe, so caching keeps panorama scrubbing
	// fast. Enabled only when loadFile() is asked for a bidirectional cache.
	bool _useFrameCache = false;
	Common::Array<Graphics::Surface> _frameCache;

	// Simple frame-range player state. _currentSurface points at the decoder's
	// last-decoded frame (owned by it, valid until the next decode).
	const Graphics::Surface *_currentSurface = nullptr;
	int _rangeLast = 0;
	int _step = 1;
	bool _rangePlaying = false;
	uint32 _lastFrameTime = 0;
	uint32 _frameDelayMs = 66;
};

} // End of namespace Nancy

#endif // NANCY_MOVIEPLAYER_H
