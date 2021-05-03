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

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/secondarymovie.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

PlaySecondaryMovie::~PlaySecondaryMovie() {
	_decoder.close();

	if (_hideMouse == kTrue && _unknown == 5) {
		g_nancy->setMouseEnabled(true);
	}
}

void PlaySecondaryMovie::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _videoName);

	stream.skip(0x12);
	_unknown = stream.readUint16LE();
	_hideMouse = (NancyFlag)stream.readUint16LE();
	_isReverse = (NancyFlag)stream.readUint16LE();
	_firstFrame = (NancyFlag)stream.readUint16LE();
	_lastFrame = (NancyFlag)stream.readUint16LE();

	for (uint i = 0; i < 15; ++i) {
		_frameFlags[i].frameID = stream.readSint16LE();
		_frameFlags[i].flagDesc.label = stream.readSint16LE();
		_frameFlags[i].flagDesc.flag = (NancyFlag)stream.readUint16LE();
	}

	_triggerFlags.readData(stream);
	_sound.read(stream, SoundDescription::kNormal);
	_sceneChange.readData(stream);

	uint16 numVideoDescs = stream.readUint16LE();
	_videoDescs.reserve(numVideoDescs);
	for (uint i = 0; i < numVideoDescs; ++i) {
		_videoDescs.push_back(SecondaryVideoDescription());
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
	_screenPosition = _drawSurface.getBounds();

	RenderObject::init();
}

void PlaySecondaryMovie::updateGraphics() {
	if (!_decoder.isVideoLoaded()) {
		return;
	}

	if (!_decoder.isPlaying() && _isVisible && !_isFinished) {
		_decoder.start();

		if (_isReverse == kTrue) {
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

		_drawSurface.blitFrom(*_decoder.decodeNextFrame(), _videoDescs[descID].srcRect, Common::Point());
		_needsRedraw = true;

		for (auto f : _frameFlags) {
			if (_decoder.getCurFrame() == f.frameID) {
				NancySceneState.setEventFlag(f.flagDesc);
			}
		}
	}

	if ((_decoder.getCurFrame() == _lastFrame && _isReverse == kFalse) ||
		(_decoder.getCurFrame() == _firstFrame && _isReverse == kTrue)) {
		if (!g_nancy->_sound->isSoundPlaying(_sound)) {
			g_nancy->_sound->stopSound(_sound);
			_decoder.stop();
			_isFinished = true;
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

		if (_hideMouse == kTrue) {
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
		if (_unknown == 5) {
			NancySceneState.changeScene(_sceneChange);
		} else {
			// Not changing the scene so enable the mouse now
			if (_hideMouse == kTrue) {
				g_nancy->setMouseEnabled(true);
			}
		}

		finishExecution();
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
