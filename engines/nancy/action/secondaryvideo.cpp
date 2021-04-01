/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/input.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/secondaryvideo.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void PlaySecondaryVideo::init() {
	if (_decoder.isVideoLoaded()) {
		_decoder.close();
	}

	if (!_decoder.loadFile(_filename + ".avf")) {
		error("Couldn't load video file %s", _filename.c_str());
	}

	// Every secondary video frame (in nancy1) plays exactly 12ms slower than what its metadata says.
	// I'm still not sure how/why that happens so for now I'm using this hack to fix the timings
	_decoder.addFrameTime(12);
	_drawSurface.create(_decoder.getWidth(), _decoder.getHeight(), g_nancy->_graphicsManager->getInputPixelFormat());

	if (_paletteFilename.size()) {
		GraphicsManager::loadSurfacePalette(_drawSurface, _paletteFilename);
	}

	setVisible(false);
	setTransparent(true);

	RenderObject::init();
}

void PlaySecondaryVideo::updateGraphics() {
	if (!_decoder.isVideoLoaded()) {
		return;
	}

	if (_isPlaying) {
		if (!_decoder.isPlaying()) {
			_decoder.start();
		}

		switch (_hoverState) {
		case kNoHover:
			if (_isHovered) {
				_decoder.seekToFrame(_onHoverFirstFrame);

				_hoverState = kHover;
			} else {
				if (_decoder.getCurFrame() == _loopLastFrame) {
					// loop back to beginning
					_decoder.seekToFrame(_loopFirstFrame);
				}

				break;
			}
			// fall through
		case kHover:
			if (!_isHovered) {
				// Stopped hovering, reverse playback
				_decoder.seekToFrame(_onHoverEndLastFrame);
				_decoder.setRate(-_decoder.getRate());
				if (!_decoder.isPlaying()) {
					_decoder.start();
				}

				_hoverState = kEndHover;
			} else {
				break;
			}
			// fall through
		case kEndHover:
			if (_decoder.getCurFrame() == _onHoverEndFirstFrame) {
				// reversed playback has ended, go back to no hover _state
				_decoder.seekToFrame(_loopFirstFrame);
				_decoder.setRate(-_decoder.getRate());
				_hoverState = kNoHover;
			}

			break;
		}

		if (_decoder.needsUpdate() && !_screenPosition.isEmpty()) {
			for (uint i = 0; i < _videoDescs.size(); ++i) {
				if ((uint16)_videoDescs[i].frameID == _currentViewportFrame) {
					// This ignores the srcRects for every frame
					GraphicsManager::copyToManaged(*_decoder.decodeNextFrame(), _drawSurface, _paletteFilename.size() > 0);
					break;
				}
			}

			_needsRedraw = true;
		}
	} else {
		_decoder.seekToFrame(0);
	}

	RenderObject::updateGraphics();
}

void PlaySecondaryVideo::onPause(bool pause) {
	_decoder.pauseVideo(pause);

	if (pause) {
		registerGraphics();
	}
}

void PlaySecondaryVideo::handleInput(NancyInput &input) {
	if (_hasHotspot && NancySceneState.getViewport().convertViewportToScreen(_hotspot).contains(input.mousePos)) {
		_isHovered = true;
	} else {
		_isHovered = false;
	}
}

void PlaySecondaryVideo::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _filename);
	readFilename(stream, _paletteFilename);
	stream.skip(10);

	if (_paletteFilename.size()) {
		stream.skip(14); // unknown data
	}

	_loopFirstFrame = stream.readUint16LE();
	_loopLastFrame = stream.readUint16LE();
	_onHoverFirstFrame = stream.readUint16LE();
	_onHoverLastFrame = stream.readUint16LE();
	_onHoverEndFirstFrame = stream.readUint16LE();
	_onHoverEndLastFrame = stream.readUint16LE();

	_sceneChange.readData(stream);

	if (_paletteFilename.size()) {
		stream.skip(3);
	} else {
		stream.skip(1);
	}

	uint16 numVideoDescs = stream.readUint16LE();
	_videoDescs.reserve(numVideoDescs);
	for (uint i = 0; i < numVideoDescs; ++i) {
		_videoDescs.push_back(SecondaryVideoDescription());
		_videoDescs[i].readData(stream);
	}
}

void PlaySecondaryVideo::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		// Set correct position according to viewport frame
		if (_currentViewportFrame != NancySceneState.getSceneInfo().frameID) {
			_currentViewportFrame = NancySceneState.getSceneInfo().frameID;

			int activeFrame = -1;

			for (uint i = 0; i < _videoDescs.size(); ++i) {
				if ((uint16)_videoDescs[i].frameID == _currentViewportFrame) {
					activeFrame = i;
				}
			}

			if (activeFrame != -1) {
				// Make the drawing destination rectangle valid
				_screenPosition = _videoDescs[activeFrame].destRect;

				// Activate the hotspot
				_hotspot = _videoDescs[activeFrame].destRect;
				_hasHotspot = true;
				_isPlaying = true;
				setVisible(true);
			} else {
				setVisible(false);
				_hasHotspot = false;
				_isPlaying = false;
			}
		}

		break;
	}
	case kActionTrigger:
		NancySceneState.pushScene();
		NancySceneState.changeScene(_sceneChange);
		finishExecution();
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
