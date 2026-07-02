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

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"
#include "engines/nancy/video.h"

#include "engines/nancy/action/secondarymovie.h"
#include "engines/nancy/state/scene.h"

#include "common/random.h"
#include "common/serializer.h"
#include "common/system.h"

#include "video/bink_decoder.h"

namespace Nancy {
namespace Action {

PlaySecondaryMovie::PlaySecondaryMovie(bool isRandom)
		: RenderActionRecord(8), _isRandom(isRandom) {
	if (_isRandom) {
		NancySceneState.notifyRandomMovieARLoaded();
	}
}


PlaySecondaryMovie::~PlaySecondaryMovie() {
	if (NancySceneState.getActiveMovie() == this) {
		NancySceneState.setActiveMovie(nullptr);
	}

	if (_playerCursorAllowed == kNoPlayerCursorAllowed) {
		g_nancy->setMouseEnabled(true);
	}
}

void PlaySecondaryMovie::readRandomSequence(Common::Serializer &ser, RandomSequence &seq) {
	readFilename(ser, seq.name);
	ser.syncAsUint16LE(seq.startFrame);
	ser.syncAsUint16LE(seq.lastFrame);
	ser.syncAsSint32LE(seq.minPauseMs);
	ser.syncAsSint32LE(seq.maxPauseMs);
	ser.syncAsUint16LE(seq.stayWeight);

	uint16 nextCount = 0;
	ser.syncAsUint16LE(nextCount);

	seq.nextSequences.resize(nextCount);
	for (uint i = 0; i < nextCount; ++i) {
		readFilename(ser, seq.nextSequences[i].name);
		ser.syncAsUint16LE(seq.nextSequences[i].weight);
	}
}

void PlaySecondaryMovie::readRandomMovieData(Common::Serializer &ser, Common::SeekableReadStream &stream) {
	readFilename(ser, _startingSequenceName);
	ser.syncAsUint16LE(_randomPlayerCursorAllowed);

	uint16 sequenceCount = 0, hotspotCount = 0;
	ser.syncAsUint16LE(sequenceCount);
	ser.syncAsUint16LE(hotspotCount);

	_sequences.resize(sequenceCount);
	for (uint i = 0; i < sequenceCount; ++i) {
		readRandomSequence(ser, _sequences[i]);
	}

	_videoDescs.resize(hotspotCount);
	for (uint i = 0; i < hotspotCount; ++i) {
		_videoDescs[i].readData(stream);
	}

	// "RandomMovie" picks any sequence; otherwise look up by name.
	// Only the first sequence is played; chained playback is TODO.
	if (!_sequences.empty()) {
		int startIdx = -1;
		if (_startingSequenceName == "RandomMovie") {
			startIdx = g_nancy->_randomSource->getRandomNumber(_sequences.size() - 1);
		} else {
			for (uint i = 0; i < _sequences.size(); ++i) {
				if (_sequences[i].name.toString() == _startingSequenceName) {
					startIdx = (int)i;
					break;
				}
			}
			if (startIdx < 0) {
				warning("PlayRandomMovie: starting sequence \"%s\" is not part of this AR",
					_startingSequenceName.c_str());
			}
		}

		if (startIdx >= 0) {
			const RandomSequence &src = _sequences[startIdx];
			_activeSequenceIndex = startIdx;
			_videoName = src.name;
			_firstFrame = src.startFrame;
			_lastFrame = src.lastFrame;
			_videoFormat = kLargeVideoFormat;
			_videoSceneChange = kMovieNoSceneChange;
			_playerCursorAllowed = (byte)_randomPlayerCursorAllowed;
			_playDirection = kPlayMovieForward;
		}
	}

	_sound.name = "NO SOUND";
}

bool PlaySecondaryMovie::activateRandomSequence(int index) {
	if (index < 0 || index >= (int)_sequences.size()) {
		return false;
	}

	const RandomSequence &src = _sequences[index];
	_activeSequenceIndex = index;
	_videoName = src.name;
	_firstFrame = src.startFrame;
	_lastFrame = src.lastFrame;

	if (!_decoder.loadFile(_videoName)) {
		warning("PlayRandomMovie: couldn't load %s", _videoName.toString().c_str());
		return false;
	}

	_isFinished = false;
	_curViewportFrame = -1;	// force visibility re-evaluation next tick
	return true;
}

void PlaySecondaryMovie::playRandomSequence() {
	if (!_isRandom || _sequences.empty()) {
		return;
	}
	int picked = g_nancy->_randomSource->getRandomNumber(_sequences.size() - 1);
	_randomChainState = kRandomPlaying;
	_randomStopRequested = false;
	activateRandomSequence(picked);
}

int PlaySecondaryMovie::rollNextSequence() {
	if (_activeSequenceIndex < 0 || _activeSequenceIndex >= (int)_sequences.size()) {
		return -1;
	}

	const RandomSequence &seq = _sequences[_activeSequenceIndex];

	uint32 totalWeight = seq.stayWeight;
	for (const NextSequenceRef &ns : seq.nextSequences) {
		totalWeight += ns.weight;
	}

	if (totalWeight == 0) {
		// No weights at all: stay indefinitely without a pause.
		_randomChainState = kRandomPaused;
		_randomPauseEndTime = g_system->getMillis() + 1000;	// re-check in 1s
		return -1;
	}

	uint32 roll = g_nancy->_randomSource->getRandomNumber(totalWeight - 1);

	if (roll < seq.stayWeight) {
		int32 pauseMs = seq.minPauseMs;
		if (seq.maxPauseMs > seq.minPauseMs) {
			pauseMs += g_nancy->_randomSource->getRandomNumber(seq.maxPauseMs - seq.minPauseMs - 1);
		}
		_randomPauseEndTime = g_system->getMillis() + (uint32)MAX<int32>(0, pauseMs);
		_randomChainState = kRandomPaused;
		setVisible(false);
		_decoder.pauseVideo(true);
		return -1;
	}

	uint32 cumulative = seq.stayWeight;
	for (uint i = 0; i < seq.nextSequences.size(); ++i) {
		cumulative += seq.nextSequences[i].weight;
		if (roll < cumulative) {
			// Look up the named sequence in _sequences[].
			for (uint j = 0; j < _sequences.size(); ++j) {
				if (_sequences[j].name == seq.nextSequences[i].name) {
					return (int)j;
				}
			}
			warning("PlayRandomMovie: next-sequence \"%s\" not part of this AR",
				seq.nextSequences[i].name.toString().c_str());
			return -1;
		}
	}

	return -1;
}

void PlaySecondaryMovie::readData(Common::SeekableReadStream &stream) {
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	if (_isRandom) {
		readRandomMovieData(ser, stream);
		return;
	}

	readFilename(ser, _videoName);
	readFilename(ser, _paletteName, kGameTypeVampire, kGameTypeVampire);
	readFilename(ser, _bitmapOverlayName, kGameTypeVampire, kGameTypeNancy9);

	ser.skip(2, kGameTypeNancy7);	// videoType
	ser.skip(2, kGameTypeVampire, kGameTypeNancy9); // videoPlaySource
	ser.syncAsUint16LE(_videoFormat);
	if (g_nancy->getGameType() >= kGameTypeNancy10)
		_videoFormat = kLargeVideoFormat;
	ser.skip(4, kGameTypeVampire, kGameTypeVampire); // paletteStart, paletteSize
	ser.skip(2, kGameTypeVampire, kGameTypeNancy9);  // hasBitmapOverlaySurface
	ser.skip(2, kGameTypeVampire, kGameTypeNancy9);  // VIDEO_STOP_RENDERING, VIDEO_CONTINUE_RENDERING

	ser.syncAsUint16LE(_videoSceneChange);
	ser.syncAsUint16LE(_playerCursorAllowed);
	ser.syncAsUint16LE(_playDirection);
	ser.syncAsUint16LE(_firstFrame);
	ser.syncAsUint16LE(_lastFrame);

	ser.syncAsSint16LE(_sceneChange.sceneID, kGameTypeNancy10);
	ser.skip(3 * 2, kGameTypeNancy10);	// TODO

	if (g_nancy->getGameType() >= kGameTypeNancy10) {
		ser.syncAsSint16LE(_videoStartFlag.label);
		ser.syncAsUint16LE(_videoStartFlag.flag);
	}

	if (ser.getVersion() >= kGameTypeNancy1) {
		uint size = 15;
		
		if (ser.getVersion() >= kGameTypeNancy10)
			ser.syncAsUint16LE(size);

		_frameFlags.resize(size);
		for (uint i = 0; i < size; ++i) {
			ser.syncAsSint16LE(_frameFlags[i].frameID);
			ser.syncAsSint16LE(_frameFlags[i].flagDesc.label);
			ser.syncAsUint16LE(_frameFlags[i].flagDesc.flag);
		}
	}

	if (ser.getVersion() <= kGameTypeNancy9) {
		_triggerFlags.readData(stream);
		_sound.readNormal(stream);
		_sceneChange.readData(stream, ser.getVersion() == kGameTypeVampire);
	}

	uint16 numVideoDescs = 0;
	ser.syncAsUint16LE(numVideoDescs);
	_videoDescs.resize(numVideoDescs);
	for (uint i = 0; i < numVideoDescs; ++i) {
		_videoDescs[i].readData(stream);
	}

	if (ser.getVersion() >= kGameTypeNancy6) {
		// Movie sound was deliberately disabled in nancy6
		_sound.name = "NO SOUND";
	}
}

void PlaySecondaryMovie::init() {
	if (!_decoder.isVideoLoaded()) {
		if (!_decoder.loadFile(_videoName)) {
			error("Couldn't load video file %s", _videoName.toString().c_str());
		}

		if (!_paletteName.empty()) {
			GraphicsManager::loadSurfacePalette(_fullFrame, _paletteName);
			GraphicsManager::loadSurfacePalette(_drawSurface, _paletteName);
		}

		if (g_nancy->getGameType() == kGameTypeVampire) {
			setTransparent(true);
			_fullFrame.setTransparentColor(_drawSurface.getTransparentColor());

			// TVD uses empty video files during the endgame ceremony
			// This makes sure the screen doesn't go black while the sound is playing
			_drawSurface.clear(_drawSurface.getTransparentColor());
		}
	}

	_screenPosition = _drawSurface.getBounds();

	RenderObject::init();
}

void PlaySecondaryMovie::onPause(bool pause) {
	_decoder.pauseVideo(pause);
	RenderActionRecord::onPause(pause);
}

void PlaySecondaryMovie::execute() {
	switch (_state) {
	case kBegin:
		// HACK: In Nancy 10, scene 2987, there are two PlaySecondaryMovie records that play
		// the same video, but have no dependencies. The first video leads to the losing scene,
		// while the second one leads to the winning scene. Since none of the two records has a
		// dependency, the first one will always be executed. It seems like there should be
		// a check to prevent the first record from being executed, but it wasn't possible to
		// find it. Don't start the first record for now, so that the second one can be
		// executed and the player can proceed.
		// TODO: Find out what the original engine does in this case, and implement it properly.
		if (g_nancy->getGameType() == kGameTypeNancy10 && _videoSceneChange == kMovieSceneChange && _sceneChange.sceneID == 2989)
			return;

		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_sound);
		g_nancy->_sound->playSound(_sound);

		if (_sound.name != "NO SOUND" && g_nancy->getGameType() <= kGameTypeNancy5) {
			// Sync audio and video. This is mostly relevant for some nancy2 scenes, as the
			// devs stopped using the built-in movie sound around nancy4. The 12 ms
			// difference is roughly how long it takes for a single execution of the main game loop
			_decoder.addFrameTime(12);
		}

		if (_playerCursorAllowed == kNoPlayerCursorAllowed) {
			g_nancy->setMouseEnabled(false);
		}

		NancySceneState.setActiveMovie(this);

		if (g_nancy->getGameType() >= kGameTypeNancy10)
			NancySceneState.setEventFlag(_videoStartFlag);

		_state = kRun;

		if (Common::Rect(_decoder.getWidth(), _decoder.getHeight()) == NancySceneState.getViewport().getBounds()) {
			g_nancy->_graphics->suppressNextDraw();
			break;
		}

		// fall through
	case kRun: {
		// Random-movie chain: while paused, wait for the pause to expire
		// then re-roll. The roll itself may set up another pause, swap to
		// the next sequence, or finish the AR if stop was requested.
		if (_isRandom && _randomChainState == kRandomPaused) {
			if (_randomStopRequested) {
				_state = kActionTrigger;
				break;
			}
			if (g_system->getMillis() < _randomPauseEndTime) {
				break;
			}
			_randomChainState = kRandomPlaying;
			int picked = rollNextSequence();
			if (picked >= 0) {
				activateRandomSequence(picked);
			}
			// If picked < 0, rollNextSequence() set up another pause.
			break;
		}

		int newFrame = NancySceneState.getSceneInfo().frameID;

		if (newFrame != _curViewportFrame) {
			_curViewportFrame = newFrame;
			int activeFrame = -1;
			for (uint i = 0; i < _videoDescs.size(); ++i) {
				if (newFrame == _videoDescs[i].frameID) {
					activeFrame = i;
					break;
				}
			}

			if (activeFrame != -1) {
				_screenPosition = _videoDescs[activeFrame].destRect;
				setVisible(true);
			} else if (_isRandom) {
				// Random movies aren't gated on hotspot/viewport-frame
				// matches the way regular PSMs are: play full viewport.
				_screenPosition = NancySceneState.getViewport().getBounds();
				setVisible(true);
			} else {
				setVisible(false);
			}
		}

		// We update the decoder here instead of in updateGraphics() to avoid an
		// edge case in nancy4 (scene 3180) where the very last frame has a frameFlag that should trigger
		// another action record, but doesn't do so, because updateGraphics() gets called after all
		// action record execution. Instead, the movie's own scene change (which is inexplicably enabled)
		// gets triggered, and teleports the player to the wrong place instead of making them lose the game
		if (!_decoder.isPlaying() && _isVisible && !_isFinished) {
			_decoder.start();

			if (_playDirection == kPlayMovieReverse) {
				_decoder.setRate(-_decoder.getRate());
				_decoder.seekToFrame(_lastFrame);
			} else {
				_decoder.seekToFrame(_firstFrame);
			}
		}

		if (_decoder.needsUpdate()) {
			uint descID = 0;

			for (uint i = 0; i < _videoDescs.size(); ++i) {
				if (_videoDescs[i].frameID == _curViewportFrame) {
					descID = i;
				}
			}

			GraphicsManager::copyToManaged(*_decoder.decodeNextFrame(), _fullFrame, g_nancy->getGameType() == kGameTypeVampire, _videoFormat == kSmallVideoFormat);
			_drawSurface.create(_fullFrame, _videoDescs[descID].srcRect);
			moveTo(_videoDescs[descID].destRect);

			_needsRedraw = true;

			for (auto &f : _frameFlags) {
				if (_decoder.getCurFrame() == f.frameID) {
					NancySceneState.setEventFlag(f.flagDesc);
				}
			}
		}

		if ((_decoder.getCurFrame() == _lastFrame && _playDirection == kPlayMovieForward) ||
			(_decoder.getCurFrame() == _firstFrame && _playDirection == kPlayMovieReverse) ||
			_decoder.endOfVideo()) {

			_decoder.pauseVideo(true);
			_isFinished = true;

			if (_isRandom) {
				// Sequence finished: roll for next. If stop was requested
				// by a PlayRandomMovieControl, wind the AR down normally.
				if (_randomStopRequested) {
					_state = kActionTrigger;
				} else {
					int picked = rollNextSequence();
					if (picked >= 0) {
						activateRandomSequence(picked);
					}
					// Otherwise the chain entered the paused state; no
					// state-trigger transition.
				}
			} else if (!g_nancy->_sound->isSoundPlaying(_sound)) {
				// Stop the video and block it from starting again, but also wait for
				// sound to end before changing state
				g_nancy->_sound->stopSound(_sound);
				_state = kActionTrigger;
			}
		}

		break;
	}
	case kActionTrigger:
		_triggerFlags.execute();
		if (_videoSceneChange == kMovieSceneChange) {
			NancySceneState.changeScene(_sceneChange);
		}

		NancySceneState.setActiveMovie(nullptr);
		finishExecution();

		// Allow looping
		if (!_isDone) {
			_isFinished = false;
			_decoder.seek(0);
			_decoder.pauseVideo(false);
		} else if (_playerCursorAllowed == kNoPlayerCursorAllowed) {
			// The movie finished and isn't looping, so restore the cursor now.
			// WORKAROUND: Don't restore the cursor for Nancy 8, scenes 5420 - 5422
			// (confrontation with the culprit). For some reason, the original engine
			// doesn't restore the cursor in this scene - restoring it allows the user
			// to examine items and break the scene itself.
			// Refer to bug #16728 for more details
			const uint16 sceneId = NancySceneState.getSceneInfo().sceneID;
			if (!(g_nancy->getGameType() == kGameTypeNancy8 && (sceneId >= 5420 && sceneId <= 5422)))
				g_nancy->setMouseEnabled(true);
		}

		break;
	}
}

// --- PlayRandomMovieControl --------------------------------------------

void PlayRandomMovieControl::readData(Common::SeekableReadStream &stream) {
	_mode = stream.readByte();
	_sceneChange.readData(stream, true, true);
}

void PlayRandomMovieControl::execute() {
	PlaySecondaryMovie *target = NancySceneState.getActiveMovie();
	if (target && target->_isRandom) {
		target->stopRandom();
	}

	_sceneChange.execute();
	finishExecution();
}

} // End of namespace Action
} // End of namespace Nancy
