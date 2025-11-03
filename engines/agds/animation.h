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

#ifndef ANIMATION_H
#define ANIMATION_H

#include "common/ptr.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}
namespace Graphics {
struct Surface;
class ManagedSurface;
} // namespace Graphics
namespace Video {
class FlicDecoder;
}

namespace AGDS {

class AGDSEngine;
class Object;

class Animation {
	using FlicPtr = Common::ScopedPtr<Video::FlicDecoder>;
	using ManagedSurfacePtr = Common::ScopedPtr<Graphics::ManagedSurface>;

	AGDSEngine *_engine;
	Common::String _name;
	FlicPtr _flic;
	ManagedSurfacePtr _frame;
	ManagedSurfacePtr _scaledFrame;
	int _frames;
	Common::Point _position;
	Common::String _process;
	Common::String _phaseVar;
	bool _loop;
	int _cycles;
	bool _phaseVarControlled;
	int _phase;
	bool _paused;
	int _speed;
	int _z;
	int _rotation;
	int _delay;
	int _random;
	float _scale;
	bool _onScreen;
	uint _visibleHeight;
	uint _visibleCenter;

public:
	Animation(AGDSEngine *engine, const Common::String &name);
	~Animation();

	bool hasFrame() const {
		return _frame != nullptr;
	}

	int frames() const {
		return _frames;
	}
	bool ended() const {
		return _phase >= _frames;
	}

	const Common::Point &position() const {
		return _position;
	}

	void position(Common::Point position) {
		_position = position;
	}

	const Common::String &phaseVar() const {
		return _phaseVar;
	}

	void phaseVar(const Common::String &phaseVar) {
		_phaseVar = phaseVar;
	}

	const Common::String &process() const {
		return _process;
	}

	void process(const Common::String &process) {
		_process = process;
	}

	void loop(bool loop) {
		_loop = loop;
	}

	void cycles(int cycles) {
		_cycles = cycles;
	}

	void delay(int delay) {
		_delay = delay;
	}

	void setRandom(int value) { // can't declare random() because of stupid macro
		_random = value;
	}

	void phaseVarControlled(bool controlled) {
		_phaseVarControlled = controlled;
		_onScreen = !controlled;
	}

	bool paused() const {
		return _paused;
	}

	void pause() {
		_paused = true;
	}

	void resume() {
		_paused = false;
	}

	void rewind();

	void speed(int speed) {
		_speed = speed;
	}

	void z(int z) {
		_z = z;
	}

	int z() const {
		return _z;
	}

	void rotate(int rotation) {
		if (_rotation == rotation)
			return;

		_rotation = rotation;
		rescaleCurrentFrame();
	}

	void scale(float scale);
	float scale() const {
		return _scale;
	}

	int phase() const {
		return _phase;
	}

	void onScreen(bool onScreen);

	bool load(Common::SeekableReadStream *stream, const Common::String &fname);
	void paint(Graphics::Surface &backbuffer, Common::Point dst, Graphics::ManagedSurface *mask = nullptr, int maskAlpha = 0) const;
	uint width() const;
	uint height() const;
	uint visibleHeight() const {
		return _visibleHeight;
	}
	uint visibleCenter() const {
		return _visibleCenter;
	}
	bool tick();

	void decodeNextFrame();

private:
	void rescaleCurrentFrame();
	void freeFrame();
	void freeScaledFrame();
};

} // End of namespace AGDS

#endif /* AGDS_ANIMATION_H */
