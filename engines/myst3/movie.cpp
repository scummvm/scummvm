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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/movie.h"
#include "engines/myst3/ambient.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/sound.h"
#include "engines/myst3/state.h"
#include "engines/myst3/subtitles.h"

#include "common/config-manager.h"

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
		_force2d(false),
		_forceOpaque(false),
		_subtitles(0),
		_volume(0),
		_additiveBlending(false),
		_transparency(100) {

	ResourceDescription binkDesc = _vm->getFileDescription("", id, 0, Archive::kMultitrackMovie);

	if (!binkDesc.isValid())
		binkDesc = _vm->getFileDescription("", id, 0, Archive::kDialogMovie);

	if (!binkDesc.isValid())
		binkDesc = _vm->getFileDescription("", id, 0, Archive::kStillMovie);

	if (!binkDesc.isValid())
		binkDesc = _vm->getFileDescription("", id, 0, Archive::kMovie);

	// Check whether the video is optional
	bool optional = false;
	if (_vm->_state->hasVarMovieOptional()) {
		optional = _vm->_state->getMovieOptional();
		_vm->_state->setMovieOptional(0);
	}

	if (!binkDesc.isValid()) {
		if (!optional)
			error("Movie %d does not exist", id);
		else
			return;
	}

	loadPosition(binkDesc.getVideoData());

	Common::SeekableReadStream *binkStream = binkDesc.getData();
	_bink.setDefaultHighColorFormat(Texture::getRGBAPixelFormat());
	_bink.setSoundType(Audio::Mixer::kSFXSoundType);
	_bink.loadStream(binkStream);

	if (binkDesc.getType() == Archive::kMultitrackMovie
			|| binkDesc.getType() == Archive::kDialogMovie) {
		uint language = ConfMan.getInt("audio_language");
		_bink.setAudioTrack(language);
	}

	if (ConfMan.getBool("subtitles"))
		_subtitles = Subtitles::create(_vm, id);

	// Clear the subtitles override anyway, so that it does not end up
	// being used by the another movie at some point.
	_vm->_state->setMovieOverrideSubtitles(0);
}

void Movie::loadPosition(const ResourceDescription::VideoData &videoData) {
	static const float scale = 50.0f;

	_is3D = _vm->_state->getViewType() == kCube;

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

	Common::Rect textureRect = Common::Rect(_bink.getWidth(), _bink.getHeight());

	if (_forceOpaque)
		_vm->_gfx->drawTexturedRect2D(screenRect, textureRect, _texture);
	else
		_vm->_gfx->drawTexturedRect2D(screenRect, textureRect, _texture, (float) _transparency / 100, _additiveBlending);
}

void Movie::draw3d() {
	_vm->_gfx->drawTexturedRect3D(_pTopLeft, _pBottomLeft, _pTopRight, _pBottomRight, _texture);
}

void Movie::draw() {
	if (_force2d)
		return;

	if (_is3D) {
		draw3d();
	} else {
		draw2d();
	}
}

void Movie::drawOverlay() {
	if (_force2d)
		draw2d();

	if (_subtitles) {
		_subtitles->setFrame(adjustFrameForRate(_bink.getCurFrame(), false));
		_vm->_gfx->renderWindowOverlay(_subtitles);
	}
}

void Movie::drawNextFrameToTexture() {
	const Graphics::Surface *frame = _bink.decodeNextFrame();

	if (frame) {
		if (_texture)
			_texture->update(frame);
		else
			_texture = _vm->_gfx->createTexture(frame);
	}
}

int32 Movie::adjustFrameForRate(int32 frame, bool dataToBink) {
	// The scripts give frame numbers for a framerate of 15 im/s
	// adjust the frame number according to the actual framerate
	if (_bink.getFrameRate().toInt() != 15) {
		Common::Rational rational;
		if (dataToBink) {
			rational = _bink.getFrameRate() * frame / 15;
		} else {
			rational = 15 * frame / _bink.getFrameRate();
		}
		frame = rational.toInt();
	}
	return frame;
}

void Movie::setStartFrame(int32 v) {
	_startFrame = adjustFrameForRate(v, true);
}

void Movie::setEndFrame(int32 v) {
	_endFrame = adjustFrameForRate(v, true);
}

void Movie::pause(bool p) {
	_bink.pauseVideo(p);
}

Movie::~Movie() {
	if (_texture)
		_vm->_gfx->freeTexture(_texture);

	delete _subtitles;
}

void Movie::setForce2d(bool b) {
	_force2d = b;
	if (_force2d) {
		_is3D = false;
	}
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
		_isLastFrame(false),
		_soundHeading(0),
		_soundAttenuation(0),
		_volumeVar(0),
		_loop(false),
		_transparencyVar(0) {
	_bink.start();
}

void ScriptedMovie::draw() {
	if (!_enabled)
		return;

	Movie::draw();
}

void ScriptedMovie::drawOverlay() {
	if (!_enabled)
		return;

	Movie::drawOverlay();
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

	if (_transparencyVar) {
		_transparency = _vm->_state->getVar(_transparencyVar);
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
					|| _bink.getCurFrame() >= _endFrame
					|| _bink.endOfVideo()) {
				_bink.seekToFrame(_startFrame);
				_isLastFrame = false;
			}

			if (!_scriptDriven)
				_bink.pauseVideo(false);

			drawNextFrameToTexture();

		} else {
			// Make sure not to pause the video twice. VideoDecoder handles pause levels.
			// The video may have already been paused if _disableWhenComplete is set.
			if (!_bink.isPaused()) {
				_bink.pauseVideo(true);
			}
		}
	}

	if (_enabled) {
		updateVolume();

		if (_nextFrameReadVar) {
			int32 nextFrame = _vm->_state->getVar(_nextFrameReadVar);
			if (nextFrame > 0 && nextFrame <= (int32)_bink.getFrameCount()) {
				// Are we changing frame?
				if (_bink.getCurFrame() != nextFrame - 1) {
					// Don't seek if we just want to display the next frame
					if (_bink.getCurFrame() + 1 != nextFrame - 1) {
						_bink.seekToFrame(nextFrame - 1);
					}
					drawNextFrameToTexture();
				}

				_vm->_state->setVar(_nextFrameReadVar, 0);
				_isLastFrame = false;
			}
		}

		if (!_scriptDriven && (_bink.needsUpdate() || _isLastFrame)) {
			bool complete = false;

			if (_isLastFrame) {
				_isLastFrame = false;

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

void ScriptedMovie::updateVolume() {
	int32 volume;
	if (_volumeVar) {
		volume = _vm->_state->getVar(_volumeVar);
	} else {
		volume = _volume;
	}

	int32 mixerVolume, balance;
	_vm->_sound->computeVolumeBalance(volume, _soundHeading, _soundAttenuation, &mixerVolume, &balance);
	_bink.setVolume(mixerVolume);
	_bink.setBalance(balance);
}

ScriptedMovie::~ScriptedMovie() {
}

SimpleMovie::SimpleMovie(Myst3Engine *vm, uint16 id) :
		Movie(vm, id),
		_synchronized(false) {
	_startFrame = 1;
	_endFrame = _bink.getFrameCount();
	_startEngineTick = _vm->_state->getTickCount();
}

void SimpleMovie::play() {
	playStartupSound();

	_bink.setEndFrame(_endFrame - 1);
	_bink.setVolume(_volume * Audio::Mixer::kMaxChannelVolume / 100);

	if (_bink.getCurFrame() < _startFrame - 1) {
		_bink.seekToFrame(_startFrame - 1);
	}

	_bink.start();
}

void SimpleMovie::update() {
	uint16 scriptStartFrame = _vm->_state->getMovieScriptStartFrame();
	if (scriptStartFrame && _bink.getCurFrame() > scriptStartFrame) {
		uint16 script = _vm->_state->getMovieScript();

		// The control variables are reset before running the script because
		// some scripts set up another movie triggered script
		_vm->_state->setMovieScriptStartFrame(0);
		_vm->_state->setMovieScript(0);

		_vm->runScriptsFromNode(script);
	}

	uint16 ambiantStartFrame = _vm->_state->getMovieAmbiantScriptStartFrame();
	if (ambiantStartFrame && _bink.getCurFrame() > ambiantStartFrame) {
		_vm->runAmbientScripts(_vm->_state->getMovieAmbiantScript());
		_vm->_state->setMovieAmbiantScriptStartFrame(0);
		_vm->_state->setMovieAmbiantScript(0);
	}

	if (!_synchronized) {
		// Play the movie according to the bink file framerate
		if (_bink.needsUpdate()) {
			drawNextFrameToTexture();
		}
	} else {
		// Draw a movie frame each two engine frames
		int targetFrame = (_vm->_state->getTickCount() - _startEngineTick) >> 1;
		if (_bink.getCurFrame() < targetFrame) {
			drawNextFrameToTexture();
		}
	}
}

bool SimpleMovie::endOfVideo() {
	if (!_synchronized) {
		return _bink.getTime() >= (uint)_bink.getEndTime().msecs();
	} else {
		int tickBasedEndFrame = (_vm->_state->getTickCount() - _startEngineTick) >> 1;
		return tickBasedEndFrame >= _endFrame;
	}
}

void SimpleMovie::playStartupSound() {
	int32 soundId = _vm->_state->getMovieStartSoundId();
	if (soundId) {
		uint32 volume = _vm->_state->getMovieStartSoundVolume();
		uint32 heading = _vm->_state->getMovieStartSoundHeading();
		uint32 attenuation = _vm->_state->getMovieStartSoundAttenuation();

		_vm->_sound->playEffect(soundId, volume, heading, attenuation);

		_vm->_state->setMovieStartSoundId(0);
	}
}

void SimpleMovie::refreshAmbientSounds() {
	uint32 engineFrames = _bink.getFrameCount() * 2;
	_vm->_ambient->playCurrentNode(100, engineFrames);
}

SimpleMovie::~SimpleMovie() {
}

ProjectorMovie::ProjectorMovie(Myst3Engine *vm, uint16 id, Graphics::Surface *background) :
		ScriptedMovie(vm, id),
		_background(background),
	_frame(0) {
	_enabled = true;

	for (uint i = 0; i < kBlurIterations; i++) {
		_blurTableX[i] = (uint8)(sin(2 * (float)M_PI * i / (float)kBlurIterations) * 256.0);
		_blurTableY[i] = (uint8)(cos(2 * (float)M_PI * i / (float)kBlurIterations) * 256.0);
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
		byte *dst = (byte *)_frame->getBasePtr(0, i);
		for (uint j = 0; j < _frame->w; j++) {
			uint8 depth;
			uint16 r = 0, g = 0, b = 0;
			uint32 srcX = (uint32)(backgroundX + j * delta);
			uint32 srcY = (uint32)(backgroundY + i * delta);
			byte *src = (byte *)_background->getBasePtr(srcX, srcY);

			// Get the depth from the background
			depth = *(src + 3);

			// Compute the blur level from the focus point and the depth of the current point
			uint8 blurLevel = abs(focus - depth) + 1;
			
			// No need to compute the effect for transparent pixels
			byte a = *(dst + 3);
			if (a != 0) {
				// The blur effect is done by mixing the color components from the pixel at (srcX, srcY)
				// and other pixels on the same row / column
				uint cnt = 0;
				for (uint k = 0; k < kBlurIterations; k++) {
					uint32 blurX = srcX + ((uint32) (blurLevel * _blurTableX[k] * delta) >> 12); // >> 12 = / 256 / 16
					uint32 blurY = srcY + ((uint32) (blurLevel * _blurTableY[k] * delta) >> 12);

					if (blurX < 1024 && blurY < 1024) {
						byte *blur = (byte *)_background->getBasePtr(blurX, blurY);

						r += *blur++;
						g += *blur++;
						b += *blur;
						cnt++;
					}
				}

				// Divide the components by the number of pixels used in the blur effect
				r /= cnt;
				g /= cnt;
				b /= cnt;
			}

			// Draw the new pixel
			*dst++ = r;
			*dst++ = g;
			*dst++ = b;
			dst++; // Keep the alpha channel from the previous frame
		}
	}

	if (_texture)
		_texture->update(_frame);
	else
		_texture = _vm->_gfx->createTexture(_frame);
}

} // End of namespace Myst3
