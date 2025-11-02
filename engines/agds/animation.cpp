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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "agds/animation.h"
#include "agds/agds.h"
#include "agds/object.h"
#include "agds/resourceManager.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "graphics/managed_surface.h"
#include "video/flic_decoder.h"

namespace AGDS {

Animation::Animation(AGDSEngine *engine, const Common::String &name) : _engine(engine), _name(name), _flic(), _frame(), _scaledFrame(),
																	   _frames(0), _loop(false), _cycles(1), _phaseVarControlled(false),
																	   _phase(0), _paused(false), _speed(100), _z(0), _rotation(0),
																	   _delay(0), _random(0), _scale(1), _onScreen(true),
																	   _visibleHeight(0), _visibleCenter(0) {
}

Animation::~Animation() {
	freeFrame();
}

void Animation::freeScaledFrame() {
	if (_scaledFrame) {
		_scaledFrame->free();
		_scaledFrame.reset();
	}
}

void Animation::freeFrame() {
	freeScaledFrame();
	if (_frame) {
		_frame->free();
		_frame.reset();
	}
}

bool Animation::load(Common::SeekableReadStream *stream, const Common::String &fname) {
	if (_phaseVarControlled) {
		if (_phaseVar.empty()) {
			warning("phase var controlled animation with no phase var");
			_phaseVarControlled = false;
			return false;
		}
		if (_loop) {
			warning("phase var controller animation with loop, resetting");
			_loop = false;
		}
		if (_cycles > 1) {
			warning("phase var controller animation with cycles, resetting");
			_cycles = 1;
		}
		if (_random) {
			warning("phase var controller animation with random, resetting");
			_random = false;
		}
		if (_delay > 0) {
			warning("phase var controller animation with delay, resetting");
			_delay = 0;
		}
	}
	_flic.reset();
	auto is_bmp = ResourceManager::IsBMP(*stream);

	if (fname.hasSuffixIgnoreCase(".bmp") || is_bmp) {
		_frame.reset(_engine->loadPicture(fname));
		rescaleCurrentFrame();
		_frames = 1;
		return true;
	}

	Video::FlicDecoder *flic = new Video::FlicDecoder;
	if (flic->loadStream(stream)) {
		_frames = flic->getFrameCount();
		_flic.reset(flic);
		decodeNextFrame();
		return true;
	} else {
		_frames = 0;
		return false;
	}
}

void Animation::scale(float scale) {
	if (scale != _scale) {
		debug("changing scale to %g", scale);
		_scale = scale;
		rescaleCurrentFrame();
	}
}

void Animation::rescaleCurrentFrame() {
	if (!_frame)
		return;

	freeScaledFrame();
	if (_rotation != 0) {
		Graphics::TransformStruct transform(_scale * 100, _scale * 100, 90 * _rotation, _frame->w / 2, _frame->h / 2, Graphics::TSpriteBlendMode::BLEND_NORMAL, Graphics::kDefaultRgbaMod);
		_scaledFrame.reset(_frame->rotoscale(transform));
	} else if (_scale != 1) {
		_scaledFrame.reset(_frame->scale(_frame->w * _scale, _frame->h * _scale, true));
	}
	auto *frame = _scaledFrame ? _scaledFrame.get() : _frame.get();
	if (frame) {
		uint h = frame->h, w = frame->w;
		_visibleHeight = 0;
		uint minX = w, maxX = 0;
		for (uint i = 0; i != h; ++i) {
			uint y = h - 1 - i;
			auto *ptr = static_cast<uint32 *>(frame->getBasePtr(0, y));
			for (uint x = 0; x != w; ++x) {
				uint8 a, r, g, b;
				frame->format.colorToARGB(*ptr++, a, r, g, b);
				if (a != 0) {
					if (h - i > _visibleHeight)
						_visibleHeight = h - i;
					if (x > maxX)
						maxX = x;
					else if (x < minX)
						minX = x;
				}
			}
		}
		_visibleCenter = minX < maxX ? (maxX + minX) / 2 : 0;
	}
}

void Animation::decodeNextFrame() {
	if (ended()) {
		_phase = 0;
		_flic->rewind();
	}

	auto frame = _flic->decodeNextFrame();
	if (!frame) {
		debug("frame of %s couldn't be decoded, process: %s, phase var: %s, at end: %d", _name.c_str(), _process.c_str(), _phaseVar.c_str(), _flic->endOfVideo());
		warning("frame of %s couldn't be decoded, process: %s, phase var: %s, at end: %d", _name.c_str(), _process.c_str(), _phaseVar.c_str(), _flic->endOfVideo());
		return;
	}

	freeFrame();
	_delay = _flic->getCurFrameDelay() * _speed / 4000; // 40 == 1000 / 25, 25 fps
	_frame.reset(_engine->convertToTransparent(frame->convertTo(_engine->pixelFormat(), _flic->getPalette())));
	rescaleCurrentFrame();
	++_phase;
}

void Animation::rewind() {
	_phase = 0;
	_flic->rewind();
}

void Animation::onScreen(bool onScreen) {
	_onScreen = onScreen;
}

bool Animation::tick() {
	if (!_flic && _frame) { // static frame
		return true;
	}

	if (_paused || !_frame || (_phaseVarControlled && !_onScreen)) {
		return true;
	}

	if (_phaseVarControlled && _engine->getGlobal(_phaseVar) == -2) {
		debug("phase var %s signalled deleting of animation", _phaseVar.c_str());
		freeFrame();
		return false;
	}

	if (_delay > 0) {
		--_delay;
		return true;
	}

	bool eov = _phase >= _frames;
	if (!_phaseVar.empty() && eov) {
		_engine->setGlobal(_phaseVar, -1);
		onScreen(false);
		return true;
	}

	if (!eov) {
		decodeNextFrame();
		onScreen(true);
	}

	if (!_process.empty()) {
		if (!_phaseVar.empty())
			_engine->setGlobal(_phaseVar, _phase - 1);
		if (/*_phase || */ _phaseVarControlled) {
			if (eov && _random) {
				rewind();
				_delay = _engine->getRandomNumber(_random);
				return true;
			}
			return true;
		}
	}

	if (eov && !_loop && --_cycles <= 0) {
		if (!_phaseVar.empty()) {
			_engine->setGlobal(_phaseVar, _phase - 1);
		} else {
			_engine->reactivate(_process, "animation end", true);
			_engine->runPendingReactivatedProcesses();
		}

		return false;
	}

	if (eov && frames() > 1)
		rewind();
	return true;
}

void Animation::paint(Graphics::Surface &backbuffer, Common::Point dst, Graphics::ManagedSurface *mask, int maskAlpha) const {
	dst += _position;
	auto *frame = _scaledFrame ? _scaledFrame.get() : _frame.get();
	if (!frame || !_onScreen)
		return;

	Common::Rect srcRect = frame->getBounds();
	if (!Common::Rect::getBlitRect(dst, srcRect, backbuffer.getRect()))
		return;

	if (mask) {
		int invMaskAlpha = 255 - maskAlpha;
		auto subFrame = frame->getSubArea(srcRect);
		auto subMask = mask->getSubArea(srcRect);
		byte *dstPixels = static_cast<byte *>(backbuffer.getBasePtr(dst.x, dst.y));
		const byte *srcPixels = static_cast<byte *>(subFrame.getPixels());
		const byte *maskPixels = static_cast<byte *>(subMask.getPixels());
		for (int y = 0; y != srcRect.height(); ++y) {
			byte *dstRow = dstPixels;
			const byte *srcRow = srcPixels;
			const byte *maskRow = maskPixels;
			for (int x = 0; x != srcRect.width(); ++x) {
				uint8 srcA = *srcRow++;
				++dstRow;
				++maskRow;
				uint8 invSrcA = 255 - srcA;
				if (srcA != 0) {
					for (int n = 3; n--; ++dstRow) {
						*dstRow = ((*dstRow * invSrcA + *srcRow++ * srcA) * invMaskAlpha + *maskRow++ * maskAlpha * srcA) / 65025;
					}
				} else {
					srcRow += 3;
					dstRow += 3;
					maskRow += 3;
				}
			}
			dstPixels += backbuffer.pitch;
			srcPixels += subFrame.pitch;
			maskPixels += subMask.pitch;
		}
	} else
		frame->blendBlitTo(backbuffer, dst.x, dst.y, Graphics::FLIP_NONE, &srcRect);
}

uint Animation::width() const {
	return _flic ? _flic->getWidth() * _scale : 0;
}
uint Animation::height() const {
	return _flic ? _flic->getHeight() * _scale : 0;
}

} // namespace AGDS
