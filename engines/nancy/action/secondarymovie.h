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
#include "engines/nancy/movieplayer.h"

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

	// Name of the next sequence to chain to once the current one finishes,
	// plus its selection weight in the weighted random pick.
	struct NextSequenceRef {
		Common::Path name;
		uint16 weight = 0;
	};

	// `name` is both the sequence id and the movie filename.
	struct RandomSequence {
		Common::Path name;
		uint16 startFrame = 0;
		uint16 lastFrame = 0;
		int32 minPauseMs = 0;
		int32 maxPauseMs = 0;
		// Weight assigned to "stay on this sequence" in the weighted random
		// pick. A roll inside [0, stayWeight) means "don't transition";
		// instead pause for [minPauseMs, maxPauseMs] and re-roll.
		uint16 stayWeight = 0;
		Common::Array<NextSequenceRef> nextSequences;
	};

	PlaySecondaryMovie(bool isRandom = false);
	virtual ~PlaySecondaryMovie();

	void init() override;
	void onPause(bool pause) override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	bool getIsFinished() const { return _isFinished; }

	Common::Path _videoName;
	Common::Path _paletteName;
	Common::Path _bitmapOverlayName;

	uint16 _videoFormat = kLargeVideoFormat;
	uint16 _videoSceneChange = kMovieNoSceneChange;
	byte _playerCursorAllowed = kPlayerCursorAllowed;
	byte _playDirection = kPlayMovieForward;
	uint16 _firstFrame = 0;
	uint16 _lastFrame = 0;
	// Nancy14-only: when non-zero, hide the movie once it reaches its last frame.
	uint16 _hideOnFinish = 0;
	// Nancy15 AR 44: a "play style" selector (1 or 3). Read but currently
	// unused by playback.
	uint16 _playStyle = 1;

	// AR 47 "InteractiveVideo" (a PlaySecondaryMovie subclass): after the
	// normal AR-44-style movie data it carries a name, a flag byte, and a
	// list of named {value, flag} entries. Read but not yet acted on.
	struct InteractiveEntry {
		Common::Path name;
		uint32 value = 0;
		byte flag = 0;
	};
	Common::Path _interactiveName;
	bool _interactiveFlag = false;
	Common::Array<InteractiveEntry> _interactiveEntries;
	Common::Array<FlagAtFrame> _frameFlags;
	MultiEventFlagDescription _triggerFlags;
	FlagDescription _videoStartFlag;

	SoundDescription _sound;

	SceneChangeDescription _sceneChange;
	Common::Array<SecondaryVideoDescription> _videoDescs;

	MoviePlayer _decoder;

	// Random-movie state (only populated when _isRandom).
	bool _isRandom = false;
	// "RandomMovie" picks any sequence; otherwise it names the starting one.
	Common::String _startingSequenceName;
	uint16 _randomPlayerCursorAllowed = kPlayerCursorAllowed;
	Common::Array<RandomSequence> _sequences;

	// Nancy13+ carries one extra "secondary" movie (a recognition animation)
	// after the sequence list. Stored for future playback; reading it is
	// required so the trailing hotspot list stays aligned.
	RandomSequence _secondaryMovie;

	// Nancy13 talkable characters: the scene to open when the character is
	// clicked (its conversation). kNoScene means the character isn't clickable.
	uint16 _talkSceneID = kNoScene;
	// Hover cursor for the character (a raw Nancy13 cursor id from the chunk).
	uint16 _talkCursorType = 0;

	// Chain state. After a sequence's movie finishes the engine rolls a
	// weighted pick: "stay" -> enter pause for a random duration and
	// re-roll; valid next-sequence -> swap to that sequence's movie.
	enum RandomChainState { kRandomPlaying, kRandomPaused };
	int _activeSequenceIndex = -1;
	RandomChainState _randomChainState = kRandomPlaying;
	uint32 _randomPauseEndTime = 0;
	bool _randomStopRequested = false;

	// Talkable-character hover state: whether the mouse is over the character,
	// and whether the recognition (secondary) movie is currently playing.
	bool _isHovered = false;
	bool _playingSecondary = false;

	// Called by PlayRandomMovieControl::execute() to wind down the AR.
	void stopRandom() { _randomStopRequested = true; }

	// Pick & start a fresh random sequence. No-op when not a random AR.
	void playRandomSequence();

	bool isViewportRelative() const override { return true; }

	bool isPersistentAcrossScenes() const override;

	// Nancy13 talkable characters expose the character's on-screen box as a
	// clickable hotspot with a talk cursor; clicking opens _talkSceneID, and
	// hovering plays the recognition ("turn around") movie.
	void handleInput(NancyInput &input) override;
	CursorManager::CursorType getHoverCursor() const override;
	bool cursorSetFromScript() const override { return _isRandom && _talkSceneID != kNoScene; }

	Common::String getRecordExtraInfo() const override { return Common::String::format("Scene %d", _sceneChange.sceneID); }

protected:
	Common::String getRecordTypeName() const override {
		return _isRandom ? "PlayRandomMovie" : "PlaySecondaryMovie";
	}

	// `ser` and `stream` must wrap the same input; `stream` is only
	// needed for SecondaryVideoDescription::readData.
	void readRandomMovieData(Common::Serializer &ser, Common::SeekableReadStream &stream);
	// Nancy14 reworked the random-movie layout (confirmed identical in Nancy15):
	// a larger header (shared with the non-random AR) and a tail of two
	// blt-descriptor lists separated by the recognition movie's name, in place
	// of Nancy13's secondaryMovie record + hotspot list.
	void readRandomMovieDataNancy14(Common::Serializer &ser, Common::SeekableReadStream &stream);
	void readRandomSequence(Common::Serializer &ser, RandomSequence &seq);
	void readSecondaryRandomMovie(Common::Serializer &ser, RandomSequence &seq);

	// Shared tail of the random-movie readers: pick the starting sequence
	// (random or by name) and seed the flat playback fields from it.
	void applyStartingRandomSequence();

	void readDataNancy14(Common::Serializer &ser, Common::SeekableReadStream &stream);

	// Apply a RandomSequence's playback config to the PSM flat fields
	// and reload the decoder. Returns true on success.
	bool activateRandomSequence(int index);

	// Load & start the recognition (secondary) movie in place of the idle loop.
	bool activateSecondaryMovie();

	// A Nancy13 talkable character: has a conversation scene and a recognition
	// movie to swap to on hover.
	bool isTalkable() const { return _isRandom && _talkSceneID != kNoScene && !_secondaryMovie.name.empty(); }

	// Pick the next sequence (or "stay") per the weighted random rules.
	// Returns -1 if "stay" was picked (and sets up the pause state),
	// or the chosen sequence index otherwise.
	int rollNextSequence();

	// Enter the paused chain state for a random duration in the sequence's
	// [minPauseMs, maxPauseMs] range. Always returns -1.
	int beginRandomPause(const RandomSequence &seq);

	// Find a sequence by name, warning and returning -1 if it isn't present.
	int lookupSequence(const Common::Path &name) const;

	// Resolve the -1/-2 "whole movie" sentinels in _firstFrame/_lastFrame
	// against the loaded decoder's frame count. Random sequences only.
	void resolveSentinelFrames();

	Graphics::ManagedSurface _fullFrame;
	int _curViewportFrame = -1;
	bool _isFinished = false;
};

// Companion AR for the random-movie variant of PlaySecondaryMovie. When
// executed it stops the currently-active random PlaySecondaryMovie and
// optionally performs a scene change / event-flag set.
class PlayRandomMovieControl : public ActionRecord {
public:
	PlayRandomMovieControl() {}

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	enum RandomMovieControlMode : byte {
		kStopNow = 0,
		kStopAfterSequence = 1,
		kResume = 2
	};

protected:
	Common::String getRecordTypeName() const override { return "PlayRandomMovieControl"; }

	byte _mode = kStopNow;
	SceneChangeWithFlag _sceneChange;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SECONDARYMOVIE_H
