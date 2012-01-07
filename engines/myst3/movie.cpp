/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/movie.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/variables.h"

namespace Myst3 {

Movie::Movie(Myst3Engine *vm, uint16 id) :
	_vm(vm),
	_startFrame(0),
	_endFrame(0) {

	const DirectorySubEntry *binkDesc = _vm->getFileDescription(0, id, 0, DirectorySubEntry::kMovie);

	if (!binkDesc)
		binkDesc = _vm->getFileDescription(0, id, 0, DirectorySubEntry::kStillMovie);

		if (!binkDesc)
		binkDesc = _vm->getFileDescription(0, id, 0, DirectorySubEntry::kImagerMovie);

	if (!binkDesc)
		error("Movie %d does not exist", id);

	loadPosition(binkDesc->getVideoData());
	initTexture();

	Common::MemoryReadStream *binkStream = binkDesc->getData();
	_bink.loadStream(binkStream, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));
}

void Movie::loadPosition(const VideoData &videoData) {
	static const float scale = 50.0f;

	Math::Vector3d planeDirection = videoData.v1;
	planeDirection.normalize();

	Math::Vector3d u;
	u.set(planeDirection.z(), 0.0f, -planeDirection.x());
	u.normalize();

	Math::Vector3d v = Math::Vector3d::crossProduct(planeDirection, u);
	v.normalize();

	Math::Vector3d planeOrigin = planeDirection * scale;

	float left = (videoData.u - 320) * 0.003125f;
	float right = (videoData.u + videoData.width - 320) * 0.003125f;
	float top = (320 - videoData.v) * 0.003125f;
	float bottom = (320 - videoData.v - videoData.height) * 0.003125f;

	Math::Vector3d vLeft = scale * left * u;
	Math::Vector3d vRight = scale * right * u;
	Math::Vector3d vTop = scale * top * v;
	Math::Vector3d vBottom = scale * bottom * v;

	_pTopLeft = planeOrigin + vTop + vLeft;
	_pBottomLeft = planeOrigin + vBottom + vLeft;
	_pBottomRight = planeOrigin + vBottom + vRight;
	_pTopRight = planeOrigin + vTop + vRight;
}

void Movie::initTexture() {
	glGenTextures(1, &_texture);

	glBindTexture(GL_TEXTURE_2D, _texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _movieTextureSize, _movieTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Movie::draw() {
	const float w = _bink.getWidth() / (float)(_movieTextureSize);
	const float h = _bink.getHeight() / (float)(_movieTextureSize);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, _texture);

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0, 0);
		glVertex3f(-_pTopLeft.x(), _pTopLeft.y(), _pTopLeft.z());

		glTexCoord2f(0, h);
		glVertex3f(-_pBottomLeft.x(), _pBottomLeft.y(), _pBottomLeft.z());

		glTexCoord2f(w, 0);
		glVertex3f(-_pTopRight.x(), _pTopRight.y(), _pTopRight.z());

		glTexCoord2f(w, h);
		glVertex3f(-_pBottomRight.x(), _pBottomRight.y(), _pBottomRight.z());
	glEnd();

	glDisable(GL_BLEND);
}

void Movie::drawNextFrameToTexture() {
	const Graphics::Surface *frame = _bink.decodeNextFrame();

	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->w, frame->h, GL_RGBA, GL_UNSIGNED_BYTE, frame->pixels);
}

Movie::~Movie() {
	glDeleteTextures(1, &_texture);
}

ScriptedMovie::ScriptedMovie(Myst3Engine *vm, uint16 id) :
	Movie(vm, id),
	_condition(0),
	_conditionBit(0),
	_startFrameVar(0),
	_endFrameVar(0),
	_posU(0),
	_posUVar(0),
	_posV(0),
	_posVVar(0),
	_nextFrameReadVar(0),
	_nextFrameWriteVar(0),
	_playingVar(0),
	_enabled(false),
	_disableWhenComplete(false),
	_scriptDriven(false),
	_isLastFrame(false) {

}

void ScriptedMovie::draw() {
	if (!_enabled)
		return;

	Movie::draw();
}

void ScriptedMovie::update() {
	if (_startFrameVar) {
		_startFrame = _vm->_vars->get(_startFrameVar);
	}

	if (_endFrameVar) {
		_endFrame = _vm->_vars->get(_endFrameVar);
	}

	if (!_endFrame) {
		_endFrame = _bink.getFrameCount();
	}

	if (_posUVar) {
		_posU = _vm->_vars->get(_posUVar);
	}

	if (_posVVar) {
		_posV = _vm->_vars->get(_posVVar);
	}

	bool newEnabled;
	if (_conditionBit) {
		newEnabled = (_vm->_vars->get(_condition) & (1 << (_conditionBit - 1))) != 0;
	} else {
		newEnabled = _vm->_vars->evaluate(_condition);
	}

	if (newEnabled != _enabled) {
		_enabled = newEnabled;

		if (newEnabled) {
			if (_disableWhenComplete
					|| _bink.getCurFrame() < _startFrame
					|| _bink.endOfVideo()) {
				_bink.seekToFrame(_startFrame);
			}

			if (!_scriptDriven)
				_bink.pauseVideo(false);

			drawNextFrameToTexture();

		} else {
			_bink.pauseVideo(true);
		}
	}

	if (_enabled) {
		if (_nextFrameReadVar) {
			int32 nextFrame = _vm->_vars->get(_nextFrameReadVar);
			if (nextFrame > 0) {
				if (_bink.getCurFrame() != nextFrame) {
					_bink.seekToFrame(nextFrame - 1);
					drawNextFrameToTexture();
				}
				_vm->_vars->set(_nextFrameReadVar, 0);
				_isLastFrame = false;
			}
		}

		if (!_scriptDriven && (_bink.needsUpdate() || _isLastFrame)) {

			bool complete = false;

			if (_isLastFrame) {
				_isLastFrame = 0;

				if (_loop) {
					_bink.seekToFrame(_startFrame);
					drawNextFrameToTexture();
				} else {
					complete = true;
				}
			} else {
				drawNextFrameToTexture();
				_isLastFrame = _bink.getCurFrame() == (_endFrame - 1);
			}

			if (_nextFrameWriteVar) {
				_vm->_vars->set(_nextFrameWriteVar, _bink.getCurFrame() + 1);
			}

			if (_disableWhenComplete && complete) {
				_bink.pauseVideo(true);

				if (_playingVar) {
					_vm->_vars->set(_playingVar, 0);
				} else {
					_enabled = 0;
					_vm->_vars->set(_condition & 0x7FF, 0);
				}
			}

		}
	}
}

ScriptedMovie::~ScriptedMovie() {
}

SimpleMovie::SimpleMovie(Myst3Engine *vm, uint16 id) :
	Movie(vm, id),
	_synchronized(false) {
	_startFrame = 1;
	_endFrame = _bink.getFrameCount();
}

bool SimpleMovie::update() {
	if (_bink.getCurFrame() < (_startFrame - 1)) {
		_bink.seekToFrame(_startFrame - 1);
	}

	uint startEngineFrame = _vm->getFrameCount();

	if (!_synchronized) {
		// Play the movie according to the bink file framerate
		if (_bink.needsUpdate()) {
			drawNextFrameToTexture();
		}
	} else {
		// Draw a movie frame each two engine frames
		int targetFrame = (_vm->getFrameCount() - startEngineFrame) >> 2;
		if (_bink.getCurFrame() < targetFrame) {
			drawNextFrameToTexture();
		}
	}

	return !_bink.endOfVideo() && _bink.getCurFrame() < _endFrame;
}

SimpleMovie::~SimpleMovie() {
}

} /* namespace Myst3 */
