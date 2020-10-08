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

#ifndef AGDS_ANIMATION_H
#define AGDS_ANIMATION_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace Common	{ class SeekableReadStream; }
namespace Graphics	{ struct Surface; struct TransparentSurface; }
namespace Video		{ class FlicDecoder; }

namespace AGDS {

class AGDSEngine;
class Object;

class Animation {
	Video::FlicDecoder *_flic;
	Graphics::TransparentSurface *_frame;
	int					_frames;
	Common::Point		_position;
	Common::String		_process;
	Common::String 		_phaseVar;
	bool				_loop;
	int					_cycles;
	bool				_startPaused;
	int					_frameIndex;
	bool				_paused;
	int					_speed;
	int					_z;
	int					_delay;
	int					_random;

public:
	Animation();
	~Animation();

	void freeFrame();

	const Common::Point & position() const {
		return _position;
	}

	void position(Common::Point position) {
		_position = position;
	}

	const Common::String & phaseVar() const {
		return _phaseVar;
	}

	void phaseVar(const Common::String & phaseVar) {
		_phaseVar = phaseVar;
	}
	int frameIndex() const {
		return _frameIndex;
	}

	const Common::String & process() const {
		return _process;
	}

	void process(const Common::String & process) {
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

	void setRandom(int value) { //can't declare random() because of stupid macro
		_random = value;
	}

	void startPaused(bool paused) {
		_startPaused = paused;
		_paused = paused;
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

	bool load(Common::SeekableReadStream *stream);
	void paint(AGDSEngine & engine, Graphics::Surface & backbuffer, Common::Point dst);
	int width() const;
	int height() const;
	bool tick(AGDSEngine &engine);
private:
	void decodeNextFrame(AGDSEngine &engine);
};


} // End of namespace AGDS

#endif /* AGDS_ANIMATION_H */
