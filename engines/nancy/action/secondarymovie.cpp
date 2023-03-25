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

#include "engines/nancy/action/secondarymovie.h"

#include "engines/nancy/state/scene.h"

#include "common/serializer.h"

namespace Nancy {
namespace Action {

PlaySecondaryMovie::~PlaySecondaryMovie() {
	_decoder.close();

	if (_playerCursorAllowed == kNoPlayerCursorAllowed && _videoSceneChange == kMovieSceneChange) {
		g_nancy->setMouseEnabled(true);
	}
}

void PlaySecondaryMovie::readData(Common::SeekableReadStream &stream) {
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	readFilename(stream, _videoName);
	readFilename(stream, _paletteName);
	ser.skip(10, kGameTypeVampire, kGameTypeVampire); // skip _bitmapOverlayName for now

	ser.skip(0x2); // videoPlaySource

	ser.skip(2, kGameTypeVampire, kGameTypeVampire); // smallSize
	ser.skip(4, kGameTypeVampire, kGameTypeVampire); // paletteStart, paletteSize
	ser.skip(2, kGameTypeVampire, kGameTypeVampire); // hasBitmapOverlaySurface
	ser.skip(2, kGameTypeVampire, kGameTypeVampire); // unknown, probably related to playing a sfx

	ser.skip(6, kGameTypeNancy1);

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
	_sound.read(stream, SoundDescription::kNormal);
	_sceneChange.readData(stream, ser.getVersion() == kGameTypeVampire);

	uint16 numVideoDescs;
	ser.syncAsUint16LE(numVideoDescs);
	_videoDescs.resize(numVideoDescs);
	for (uint i = 0; i < numVideoDescs; ++i) {
		_videoDescs[i].readData(stream);
	}
}

void PlaySecondaryMovie::init() {
	if (_decoder.isVideoLoaded()) {
		_decoder.close();
	}

	if (!_decoder.loadFile(_videoName + ".avf")) {
		error("Couldn't load video file %s", _videoName.c_str());
	}

	_drawSurface.create(_decoder.getWidth(), _decoder.getHeight(), g_nancy->_graphicsManager->getInputPixelFormat());
	
	if (_paletteName.size()) {
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

	_screenPosition = _drawSurface.getBounds();

	RenderObject::init();
}

void PlaySecondaryMovie::updateGraphics() {
	if (!_decoder.isVideoLoaded()) {
		return;
	}

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

		GraphicsManager::copyToManaged(*_decoder.decodeNextFrame(), _fullFrame, _paletteName.size() > 0);
		_drawSurface.create(_fullFrame, _fullFrame.getBounds());
		moveTo(_videoDescs[descID].destRect);
		
		_needsRedraw = true;

		for (auto f : _frameFlags) {
			if (_decoder.getCurFrame() == f.frameID) {
				NancySceneState.setEventFlag(f.flagDesc);
			}
		}
	}

	if ((_decoder.getCurFrame() == _lastFrame && _playDirection == kPlayMovieForward) ||
		(_decoder.getCurFrame() == _firstFrame && _playDirection == kPlayMovieReverse) ||
		_decoder.atEnd()) {
		
		// Stop the video and block it from starting again, but also wait for
		// sound to end before changing state
		_decoder.stop();
		_isFinished = true;

		if (!g_nancy->_sound->isSoundPlaying(_sound)) {
			g_nancy->_sound->stopSound(_sound);
			_state = kActionTrigger;
		}
	}

	RenderObject::updateGraphics();
}

void PlaySecondaryMovie::onPause(bool pause) {
	_decoder.pauseVideo(pause);

	if (!pause) {
		registerGraphics();
	}
}

void PlaySecondaryMovie::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_sound);
		g_nancy->_sound->playSound(_sound);

		if (_playerCursorAllowed == kNoPlayerCursorAllowed) {
			g_nancy->setMouseEnabled(false);
		}

		_state = kRun;
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

		finishExecution();
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
