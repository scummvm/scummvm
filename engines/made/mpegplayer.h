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

#ifndef MADE_MPEGPLAYER_H
#define MADE_MPEGPLAYER_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Made {

class MadeEngine;

class MadeMpegDecoder;

class MpegPlayer {
public:
	explicit MpegPlayer(MadeEngine *vm);
	~MpegPlayer();

	/**
	 * Play mode the script asks for, its second argument to PLAYMPEGMOVIE. The
	 * original interpreter tells the card to pause on completion for anything
	 * but mode 2, which loops; mode 3 additionally sits in a loop of its own
	 * until the movie has finished rather than letting the script carry on.
	 */
	enum PlayMode {
		kPlayOnce = 1,		// runs through once and its last picture stays up
		kPlayLoop = 2,		// runs until something else replaces it
		kPlayBlocking = 3	// as kPlayOnce, but the script waits here for the end
	};

	/**
	 * Starts a movie and returns at once. The game keeps drawing and stays
	 * responsive while it runs, which is how the ReelMagic driver behaved: the
	 * script fires a movie off and then polls whether it is still going.
	 */
	bool start(const char *filename, int16 playMode = kPlayOnce);

	/**
	 * True while a movie still has its first pass to finish. A looping overlay
	 * reports false once it has been through once, so that a script waiting for
	 * it is answered while the animation carries on behind the scene.
	 */
	bool isPlaying() const { return _decoder != nullptr && !_passComplete; }

	/**
	 * Counts the movies started so far. A caller waiting for one to end can
	 * watch this to notice that something else has taken the player over in the
	 * meantime, rather than sit there waiting for a movie that is long gone.
	 */
	uint32 playToken() const { return _playToken; }

	/**
	 * The play state as the card reported it, which is what the script polls:
	 * 2 when a movie has been left paused on its current picture, 1 while one
	 * is running, 0 once the handle has been closed. The
	 * script tells a movie that finished from one that was cut short by exactly
	 * this, so 0 and 2 are not interchangeable.
	 */
	enum { kStateClosed = 0, kStatePlaying = 1, kStatePaused = 2 };
	int16 playState() const;

	/** Decodes and shows whatever is due. Cheap when nothing is. */
	void update();

	/** Ends playback and lets go of the decoder. */
	void stop();

	/** Pauses playback on the current picture. */
	void pause();

	/** Closes the emulated media handle, making its state 0. */
	void close();

private:
	void drawFrame(const Graphics::Surface *frame);

	/** Builds a decoder for _filename. Used to start, and to go round again. */
	bool openDecoder();

	/** Builds the decoder itself, over an already prepared stream. */
	bool openDecoderFrom(Common::SeekableReadStream *stream);

	/** Reloads the stream so a looping overlay carries on. */
	bool restartForLoop();

	MadeEngine *_vm;

	MadeMpegDecoder *_decoder;
	bool _looping;
	bool _passComplete;
	bool _paused;

	// The decoded picture, scaled to the size of the game screen. Screen mixes
	// it in underneath the graphics layer.
	Graphics::Surface *_videoLayer;

	uint32 _startTime;
	uint32 _framesDrawn;
	uint32 _loopFrames;	// _framesDrawn at the last time round
	uint32 _loops;
	uint32 _playToken;

	// A looping movie is read and patched once and then kept, avoiding another
	// disk read and both repair passes at every loop boundary.
	Common::Array<byte> _loopData;
	uint32 _frameDurNum, _frameDurDen;
	Common::String _filename;
};

} // End of namespace Made

#endif // MADE_MPEGPLAYER_H
