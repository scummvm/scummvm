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

#include "common/serializer.h"

#include "video/bink_decoder.h"

namespace Nancy {
namespace Action {

PlaySecondaryMovie::~PlaySecondaryMovie() {
	delete _decoder;

	if (NancySceneState.getActiveMovie() == this) {
		NancySceneState.setActiveMovie(nullptr);
	}

	if (_playerCursorAllowed == kNoPlayerCursorAllowed) {
		g_nancy->setMouseEnabled(true);
	}
}

void PlaySecondaryMovie::readData(Common::SeekableReadStream &stream) {
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	readFilename(ser, _videoName);
	readFilename(ser, _paletteName, kGameTypeVampire, kGameTypeVampire);
	readFilename(ser, _bitmapOverlayName);

	ser.syncAsUint16LE(_videoType, kGameTypeNancy7);
	ser.skip(2); // videoPlaySource
	ser.syncAsUint16LE(_videoFormat);
	ser.skip(4, kGameTypeVampire, kGameTypeVampire); // paletteStart, paletteSize
	ser.skip(2); // hasBitmapOverlaySurface
	ser.skip(2); // VIDEO_STOP_RENDERING, VIDEO_CONTINUE_RENDERING

	ser.syncAsUint16LE(_videoSceneChange);
	ser.syncAsUint16LE(_playerCursorAllowed);
	ser.syncAsUint16LE(_playDirection);
	ser.syncAsUint16LE(_firstFrame);
	ser.syncAsUint16LE(_lastFrame);

	if (ser.getVersion() >= kGameTypeNancy1) {
		_frameFlags.resize(15);
		for (uint i = 0; i < 15; ++i) {
			ser.syncAsSint16LE(_frameFlags[i].frameID);
			ser.syncAsSint16LE(_frameFlags[i].flagDesc.label);
			ser.syncAsUint16LE(_frameFlags[i].flagDesc.flag);
		}
	}

	_triggerFlags.readData(stream);
	_sound.readNormal(stream);
	_sceneChange.readData(stream, ser.getVersion() == kGameTypeVampire);

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
	if (!_decoder) {
		if (_videoType == kVideoPlaytypeAVF) {
			_decoder = new AVFDecoder();
		} else {
			_decoder = new Video::BinkDecoder();
		}
	}

	if (!_decoder->isVideoLoaded()) {
		if (!_decoder->loadFile(_videoName.append(_videoType == kVideoPlaytypeAVF ? ".avf" : ".bik"))) {
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
	_decoder->pauseVideo(pause);
	RenderActionRecord::onPause(pause);
}

void PlaySecondaryMovie::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_sound);
		g_nancy->_sound->playSound(_sound);

		if (_sound.name != "NO SOUND" && g_nancy->getGameType() <= kGameTypeNancy5) {
			// Sync audio and video. This is mostly relevant for some nancy2 scenes, as the
			// devs stopped using the built-in movie sound around nancy4. The 12 ms
			// difference is roughly how long it takes for a single execution of the main game loop
			((AVFDecoder *)_decoder)->addFrameTime(12);
		}

		if (_playerCursorAllowed == kNoPlayerCursorAllowed) {
			g_nancy->setMouseEnabled(false);
		}

		NancySceneState.setActiveMovie(this);

		_state = kRun;

		if (Common::Rect(_decoder->getWidth(), _decoder->getHeight()) == NancySceneState.getViewport().getBounds()) {
			g_nancy->_graphics->suppressNextDraw();
			break;
		}

		// fall through
	case kRun: {
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
			} else {
				setVisible(false);
			}
		}

		// We update the decoder here instead of in updateGraphics() to avoid an
		// edge case in nancy4 (scene 3180) where the very last frame has a frameFlag that should trigger
		// another action record, but doesn't do so, because updateGraphics() gets called after all
		// action record execution. Instead, the movie's own scene change (which is inexplicably enabled)
		// gets triggered, and teleports the player to the wrong place instead of making them lose the game
		if (!_decoder->isPlaying() && _isVisible && !_isFinished) {
			_decoder->start();

			if (_playDirection == kPlayMovieReverse) {
				_decoder->setRate(-_decoder->getRate());
				_decoder->seekToFrame(_lastFrame);
			} else {
				_decoder->seekToFrame(_firstFrame);
			}
		}

		if (_decoder->needsUpdate()) {
			uint descID = 0;

			for (uint i = 0; i < _videoDescs.size(); ++i) {
				if (_videoDescs[i].frameID == _curViewportFrame) {
					descID = i;
				}
			}

			GraphicsManager::copyToManaged(*_decoder->decodeNextFrame(), _fullFrame, g_nancy->getGameType() == kGameTypeVampire, _videoFormat == kSmallVideoFormat);
			_drawSurface.create(_fullFrame, _videoDescs[descID].srcRect);
			moveTo(_videoDescs[descID].destRect);

			_needsRedraw = true;

			for (auto &f : _frameFlags) {
				if (_decoder->getCurFrame() == f.frameID) {
					NancySceneState.setEventFlag(f.flagDesc);
				}
			}
		}

		if ((_decoder->getCurFrame() == _lastFrame && _playDirection == kPlayMovieForward) ||
			(_decoder->getCurFrame() == _firstFrame && _playDirection == kPlayMovieReverse) ||
			_decoder->endOfVideo()) {

			// Stop the video and block it from starting again, but also wait for
			// sound to end before changing state
			_decoder->pauseVideo(true);
			_isFinished = true;

			if (!g_nancy->_sound->isSoundPlaying(_sound)) {
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
		} else {
			// Not changing the scene so enable the mouse now
			if (_playerCursorAllowed == kNoPlayerCursorAllowed) {
				g_nancy->setMouseEnabled(true);
			}
		}

		NancySceneState.setActiveMovie(nullptr);
		finishExecution();

		// Allow looping
		if (!_isDone) {
			_isFinished = false;
			_decoder->seek(0);
			_decoder->pauseVideo(false);
		}

		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
