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

#include "agds/animation.h"
#include "agds/agds.h"
#include "agds/object.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "graphics/transparent_surface.h"
#include "video/flic_decoder.h"

namespace AGDS {

Animation::Animation(AGDSEngine *engine, const Common::String &name) :
	_engine(engine), _name(name), _flic(), _frame(),
	_frames(0), _loop(false), _cycles(1), _phaseVarControlled(false),
	_phase(0), _paused(false), _speed(100), _z(0),
	_delay(0), _random(0), _scale(1) {
}

Animation::~Animation() {
	freeFrame();
	delete _flic;
}

void Animation::freeFrame() {
	if (_frame) {
		_frame->free();
		delete _frame;
		_frame = nullptr;
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
	delete _flic;
	_flic = nullptr;

	if (fname.hasSuffixIgnoreCase(".bmp")) {
		_frame = _engine->loadPicture(fname);
		_frames = 1;
		return true;
	}

	Video::FlicDecoder *flic = new Video::FlicDecoder;
	if (flic->loadStream(stream)) {
		_frames = flic->getFrameCount();
		_flic = flic;
		return true;
	} else {
		_frames = 0;
		return false;
	}
}

void Animation::decodeNextFrame() {
	auto frame = _flic->decodeNextFrame();
	if (!frame) {
		debug("frame of %s couldn't be decoded, process: %s, phase var: %s, at end: %d", _name.c_str(), _process.c_str(), _phaseVar.c_str(), _flic->endOfVideo());
		warning("frame of %s couldn't be decoded, process: %s, phase var: %s, at end: %d", _name.c_str(), _process.c_str(), _phaseVar.c_str(), _flic->endOfVideo());
		return;
	}

	freeFrame();
	_delay = _flic->getCurFrameDelay() * _speed / 4000; //40 == 1000 / 25, 25 fps
	_frame = _engine->convertToTransparent(frame->convertTo(_engine->pixelFormat(), _flic->getPalette()));

	if (_scale != 1) {
		auto f = _frame->scale(_frame->w * _scale, _frame->h * _scale, true);
		if (f) {
			freeFrame();
			_frame = f;
		}
	}
	++_phase;
}

void Animation::rewind() {
	_phase = 0;
	_flic->rewind();
}

bool Animation::tick() {
	if (!_flic && _frame) { //static frame
		return true;
	}

	if (_paused || (_phaseVarControlled && !_frame)) {
		return true;
	}

	if (_phaseVarControlled && _engine->getGlobal(_phaseVar) == -2) {
		debug("phase var %s signalled deleting of animation", _phaseVar.c_str());
		return false;
	}

	if (_delay > 0) {
		--_delay;
		return true;
	}

	bool eov = _phase >= _frames;
	if (_phaseVarControlled && eov) {
		freeFrame();
		_engine->setGlobal(_phaseVar, -1);
		return true;
	}

	if (!eov)
		decodeNextFrame();

	if (!_process.empty()) {
		if (!_phaseVar.empty())
			_engine->setGlobal(_phaseVar, _phase - 1);
		if (/*_phase || */_phaseVarControlled) {
			if (eov && _random) {
				rewind();
				_delay = _engine->getRandomNumber(_random);
				return true;
			}
			return true;
		}
	}

	if (eov && !_loop && --_cycles <= 0) {
		if (!_phaseVar.empty())
			_engine->setGlobal(_phaseVar, _phase - 1);
		else
			_engine->reactivate(_process, true);
		return false;
	}

	if (eov)
		rewind();
	return true;
}

void Animation::paint(Graphics::Surface &backbuffer, Common::Point dst) const {
	dst += _position;
	if (_frame) {
		Common::Rect srcRect = _frame->getRect();
		if (Common::Rect::getBlitRect(dst, srcRect, backbuffer.getRect()))
			_frame->blit(backbuffer, dst.x, dst.y, Graphics::FLIP_NONE, &srcRect);
	}
}

int Animation::width() const {
	return _flic ? _flic->getWidth() * _scale: 0;
}
int Animation::height() const {
	return _flic ? _flic->getHeight() * _scale: 0;
}

} // namespace AGDS
