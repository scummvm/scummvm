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

#ifndef NANCY_ACTION_SECONDARYMOVIE_H
#define NANCY_ACTION_SECONDARYMOVIE_H

#include "common/ptr.h"

#include "engines/nancy/action/actionrecord.h"

namespace Video {
class VideoDecoder;
}

namespace Nancy {
namespace Action {

class InteractiveVideo;

// Plays an AVF or Bink video. Optionally supports:
// - playing a sound;
// - reverse playback;
// - moving with the scene's background frame;
// - hiding of player cursor (and thus, disabling input);
// - setting event flags on a specific frame, as well as at the end of the video;
// - changing the scene after playback ends
// Mostly used for cinematics, with some occasional uses for background animations.
//
// Construct with `isRandom = true` for Nancy 11's AT_PLAY_RANDOM_MOVIE (AR 45):
// the chunk holds a list of sequences and one is picked at readData() time.
class PlaySecondaryMovie : public RenderActionRecord {
public:
	static const byte kMovieSceneChange			= 5;
	static const byte kMovieNoSceneChange		= 6;

	static const byte kPlayerCursorAllowed		= 1;
	static const byte kNoPlayerCursorAllowed	= 2;

	static const byte kPlayMovieForward			= 1;
	static const byte kPlayMovieReverse			= 2;

	struct FlagAtFrame {
		int16 frameID;
		FlagDescription flagDesc;
	};

	// Name of the next sequence to chain to once the current one finishes.
	struct NextSequenceRef {
		Common::Path name;
		uint16 unknown = 0;
	};

	// `name` is both the sequence id and the movie filename.
	struct RandomSequence {
		Common::Path name;
		uint16 startFrame = 0;
		uint16 unknown_0x23 = 0;
		int32 minPauseMs = 0;
		int32 maxPauseMs = 0;
		uint16 unknown_0x2D = 0;
		Common::Array<NextSequenceRef> nextSequences;
	};

	PlaySecondaryMovie(bool isRandom = false) : RenderActionRecord(8), _isRandom(isRandom) {}
	virtual ~PlaySecondaryMovie();

	void init() override;
	void onPause(bool pause) override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	bool getIsFinished() const { return _isFinished; }

	Common::Path _videoName;
	Common::Path _paletteName;
	Common::Path _bitmapOverlayName;

	uint16 _videoType = kVideoPlaytypeAVF;
	uint16 _videoFormat = kLargeVideoFormat;
	uint16 _videoSceneChange = kMovieNoSceneChange;
	byte _playerCursorAllowed = kPlayerCursorAllowed;
	byte _playDirection = kPlayMovieForward;
	uint16 _firstFrame = 0;
	uint16 _lastFrame = 0;
	Common::Array<FlagAtFrame> _frameFlags;
	MultiEventFlagDescription _triggerFlags;
	FlagDescription _videoStartFlag;

	SoundDescription _sound;

	SceneChangeDescription _sceneChange;
	Common::Array<SecondaryVideoDescription> _videoDescs;

	Common::ScopedPtr<Video::VideoDecoder> _decoder;

	// Random-movie state (only populated when _isRandom).
	bool _isRandom = false;
	// "RandomMovie" picks any sequence; otherwise it names the starting one.
	Common::String _startingSequenceName;
	uint16 _randomPlayerCursorAllowed = kPlayerCursorAllowed;
	Common::Array<RandomSequence> _sequences;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override {
		return _isRandom ? "PlayRandomMovie" : "PlaySecondaryMovie";
	}

	// `ser` and `stream` must wrap the same input; `stream` is only
	// needed for SecondaryVideoDescription::readData.
	void readRandomMovieData(Common::Serializer &ser, Common::SeekableReadStream &stream);
	void readRandomSequence(Common::Serializer &ser, RandomSequence &seq);

	Graphics::ManagedSurface _fullFrame;
	int _curViewportFrame = -1;
	bool _isFinished = false;
};

// Companion AR for the random-movie variant of PlaySecondaryMovie. When
// executed it locates the matching PlaySecondaryMovie(isRandom=true)
// instance (by sequence/AR name) and stops it. First seen in Nancy 11
// (AR 46, AT_PLAY_RANDOM_MOVIE_CONTROL).
//
// Chunk layout: fixed 15 bytes (3 × uint32 + uint16 + uint8). The exact
// meaning of each field is not yet reverse-engineered; the runtime debug
// string is "STOP AT_PLAY_RANDOM_MOVIE AR %d in Scene %d", so at least two
// of these fields identify the target AR.
class PlayRandomMovieControl : public ActionRecord {
public:
	PlayRandomMovieControl() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "PlayRandomMovieControl"; }

	uint32 _targetA = 0;
	uint32 _targetB = 0;
	uint32 _targetC = 0;
	uint16 _flagsOrIndex = 0;
	byte _trailing = 0;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SECONDARYMOVIE_H
