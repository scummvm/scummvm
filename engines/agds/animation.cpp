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

Animation::Animation() :
	_flic(), _frame(), _frames(0), _loop(false), _cycles(1), _phaseVarControlled(false),
	_phase(0), _paused(false), _speed(100), _z(0),
	_delay(-1), _random(0), _scale(1) {
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


bool Animation::load(Common::SeekableReadStream *stream) {
	delete _flic;
	Video::FlicDecoder *flic = new Video::FlicDecoder;
	if (flic->loadStream(stream)) {
		_frames = flic->getFrameCount();
		delete _flic;
		_flic = flic;
		return true;
	} else {
		_frames = 0;
		delete flic;
		return false;
	}
}

void Animation::decodeNextFrame(AGDSEngine &engine) {
	auto frame = _flic->decodeNextFrame();
	if (!frame) {
		rewind();
		frame = _flic->decodeNextFrame();
		if (!frame)
			error("failed decoding frame after rewind");
	}
	freeFrame();
	_frame = engine.convertToTransparent(frame->convertTo(engine.pixelFormat(), _flic->getPalette()));
	if (_scale != 1) {
		auto f = _frame->scale(_frame->w * _scale, _frame->h * _scale, true);
		if (f) {
			delete _frame;
			_frame = f;
		}
	}
}

void Animation::rewind() {
	freeFrame();
	_phase = 0;
	_flic->rewind();
}

bool Animation::tick(AGDSEngine &engine) {
	if (_paused) {
		return true;
	}

	if (!_frame && _phaseVarControlled)
		return true;

	if (_delay > 0) {
		--_delay;
		return true;
	}

	if (_phaseVarControlled && !_phaseVar.empty() && engine.getGlobal(_phaseVar) == -2) {
		debug("phase var %s signalled deleting of animation", _phaseVar.c_str());
		return false;
	}

	int frame = frameIndex();
	if (frame >= _frames && !_loop) {
		if (!_phaseVar.empty()) {
			engine.setGlobal(_phaseVar, -1);
		} else {
			engine.reactivate(_process);
		}
		if (_phaseVarControlled) {
			freeFrame();
			return true;
		}
		return false;
	}

	for(int begin = frame, end = frameIndex(1); begin < end; ++begin) {
		decodeNextFrame(engine);
	}

	if (!_process.empty()) {
		if (!_phaseVar.empty()) {
			engine.setGlobal(_phaseVar, _phase);
		}
	}
	++_phase;
	return true;
}

void Animation::paint(AGDSEngine &engine, Graphics::Surface &backbuffer, Common::Point dst) {
	dst += _position;
	if (_frame) {
		Common::Rect srcRect = _frame->getRect();
		if (Common::Rect::getBlitRect(dst, srcRect, backbuffer.getRect()))
			_frame->blit(backbuffer, dst.x, dst.y, Graphics::FLIP_NONE, &srcRect);
	}
}

int Animation::width() const {
	return _flic ? _flic->getWidth() : 0;
}
int Animation::height() const {
	return _flic ? _flic->getHeight() : 0;
}

} // namespace AGDS
