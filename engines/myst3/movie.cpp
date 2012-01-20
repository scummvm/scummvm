/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
#include "engines/myst3/state.h"

#include "graphics/colormasks.h"

namespace Myst3 {

Movie::Movie(Myst3Engine *vm, uint16 id) :
	_vm(vm),
	_id(id),
	_posU(0),
	_posV(0),
	_startFrame(0),
	_endFrame(0),
	_texture(0),
	_force2d(false) {

	const DirectorySubEntry *binkDesc = _vm->getFileDescription(0, id, 0, DirectorySubEntry::kMovie);

	if (!binkDesc)
		binkDesc = _vm->getFileDescription(0, id, 0, DirectorySubEntry::kStillMovie);

	if (!binkDesc)
		binkDesc = _vm->getFileDescription(0, id, 0, DirectorySubEntry::kMultitrackMovie);

	if (!binkDesc)
		binkDesc = _vm->getFileDescription(0, id, 0, DirectorySubEntry::kDialogMovie);

	if (!binkDesc)
		error("Movie %d does not exist", id);

	loadPosition(binkDesc->getVideoData());

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

	_posU = videoData.u;
	_posV = videoData.v;
}

void Movie::draw2d() {
	Common::Rect screenRect = Common::Rect(_bink.getWidth(), _bink.getHeight());
	screenRect.translate(_posU, _posV);

	if (_vm->_state->getViewType() != kMenu)
		screenRect.translate(0, Scene::kTopBorderHeight);

	Common::Rect textureRect = Common::Rect(_bink.getWidth(), _bink.getHeight());
	_vm->_gfx->drawTexturedRect2D(screenRect, textureRect, _texture, 0.99f);
}

void Movie::draw3d() {
	_vm->_gfx->drawTexturedRect3D(_pTopLeft, _pBottomLeft, _pTopRight, _pBottomRight, _texture);
}

void Movie::draw() {
	if (_force2d)
		return;

	if (_vm->_state->getViewType() != kCube) {
		draw2d();
	} else {
		draw3d();
	}
}

void Movie::drawForce2d() {
	if (_force2d)
		draw2d();
}

void Movie::drawNextFrameToTexture() {
	const Graphics::Surface *frame = _bink.decodeNextFrame();

	if (_texture)
		_texture->update(frame);
	else
		_texture = _vm->_gfx->createTexture(frame);
}

Movie::~Movie() {
	if (_texture)
		_vm->_gfx->freeTexture(_texture);
}

ScriptedMovie::ScriptedMovie(Myst3Engine *vm, uint16 id) :
	Movie(vm, id),
	_condition(0),
	_conditionBit(0),
	_startFrameVar(0),
	_endFrameVar(0),
	_posUVar(0),
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
		_startFrame = _vm->_state->getVar(_startFrameVar);
	}

	if (_endFrameVar) {
		_endFrame = _vm->_state->getVar(_endFrameVar);
	}

	if (!_endFrame) {
		_endFrame = _bink.getFrameCount();
	}

	if (_posUVar) {
		_posU = _vm->_state->getVar(_posUVar);
	}

	if (_posVVar) {
		_posV = _vm->_state->getVar(_posVVar);
	}

	bool newEnabled;
	if (_conditionBit) {
		newEnabled = (_vm->_state->getVar(_condition) & (1 << (_conditionBit - 1))) != 0;
	} else {
		newEnabled = _vm->_state->evaluate(_condition);
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
			int32 nextFrame = _vm->_state->getVar(_nextFrameReadVar);
			if (nextFrame > 0) {
				if (_bink.getCurFrame() != nextFrame - 1) {
					_bink.seekToFrame(nextFrame - 1);
					drawNextFrameToTexture();
				}
				_vm->_state->setVar(_nextFrameReadVar, 0);
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
				_vm->_state->setVar(_nextFrameWriteVar, _bink.getCurFrame() + 1);
			}

			if (_disableWhenComplete && complete) {
				_bink.pauseVideo(true);

				if (_playingVar) {
					_vm->_state->setVar(_playingVar, 0);
				} else {
					_enabled = 0;
					_vm->_state->setVar(_condition & 0x7FF, 0);
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

	uint startEngineFrame = _vm->_state->getFrameCount();

	if (!_synchronized) {
		// Play the movie according to the bink file framerate
		if (_bink.needsUpdate()) {
			drawNextFrameToTexture();
		}
	} else {
		// Draw a movie frame each two engine frames
		int targetFrame = (_vm->_state->getFrameCount() - startEngineFrame) >> 2;
		if (_bink.getCurFrame() < targetFrame) {
			drawNextFrameToTexture();
		}
	}

	return !_bink.endOfVideo() && _bink.getCurFrame() < _endFrame;
}

SimpleMovie::~SimpleMovie() {
}

ProjectorMovie::ProjectorMovie(Myst3Engine *vm, uint16 id, Graphics::Surface *background) :
	ScriptedMovie(vm, id),
	_background(background),
	_frame(0) {
	_enabled = true;

	for (uint i = 0; i < kBlurIterations; i++) {
		_blurTableX[i] = sin(2 * M_PI * i / (float) kBlurIterations) * 256.0;
		_blurTableY[i] = cos(2 * M_PI * i / (float) kBlurIterations) * 256.0;
	}
}

ProjectorMovie::~ProjectorMovie() {
	if (_frame) {
		_frame->free();
		delete _frame;
	}

	if (_background) {
		_background->free();
		delete _background;
	}
}

void ProjectorMovie::update() {
	if (!_frame) {
		// First call, get the alpha channel from the bink file
		const Graphics::Surface *frame = _bink.decodeNextFrame();
		_frame = new Graphics::Surface();
		_frame->copyFrom(*frame);
	}

	uint16 focus = _vm->_state->getProjectorBlur() / 10;
	uint16 zoom = _vm->_state->getProjectorZoom();
	uint16 backgroundX = (_vm->_state->getProjectorX() - zoom / 2) / 10;
	uint16 backgroundY = (_vm->_state->getProjectorY() - zoom / 2) / 10;
	float delta = zoom / 10.0 / _frame->w;

	// For each pixel in the target image
	for (uint i = 0; i < _frame->h; i++) {
		uint32 *dst = (uint32 *)_frame->getBasePtr(0, i);
		for (uint j = 0; j < _frame->w; j++) {
			uint8 a, depth;
			uint16 r = 0, g = 0, b = 0;
			uint32 srcX = backgroundX + j * delta;
			uint32 srcY = backgroundY + i * delta;
			uint32 *src = (uint32 *)_background->getBasePtr(srcX, srcY);

			// Keep the alpha channel from the previous frame
			a = *dst >> 24;

			// Get the depth from the background
			depth = *src >> 24;

			// Compute the blur level from the focus point and the depth of the current point
			uint8 blurLevel = abs(focus - depth) + 1;
			
			// No need to compute the effect for transparent pixels
			if (a != 0) {
				// The blur effect is done by mixing the color components from the pixel at (srcX, srcY)
				// and other pixels on the same row / column
				uint cnt = 0;
				for (uint k = 0; k < kBlurIterations; k++) {
					uint8 blurR, blurG, blurB;
					uint32 blurX = srcX + ((uint32) (blurLevel * _blurTableX[k] * delta) >> 12); // >> 12 = / 256 / 16
					uint32 blurY = srcY + ((uint32) (blurLevel * _blurTableY[k] * delta) >> 12);

					if (blurX < 1024 && blurY < 1024) {
						uint32 *blur = (uint32 *)_background->getBasePtr(blurX, blurY);

						Graphics::colorToRGB< Graphics::ColorMasks<8888> >(*blur, blurR, blurG, blurB);
						r += blurR;
						g += blurG;
						b += blurB;
						cnt++;
					}
				}

				// Divide the components by the number of pixels used in the blur effect
				r /= cnt;
				g /= cnt;
				b /= cnt;
			}

			// Draw the new frame
			*dst++ = Graphics::ARGBToColor< Graphics::ColorMasks<8888> >(a, r, g, b);
		}
	}

	if (_texture)
		_texture->update(_frame);
	else
		_texture = _vm->_gfx->createTexture(_frame);
}

} /* namespace Myst3 */
