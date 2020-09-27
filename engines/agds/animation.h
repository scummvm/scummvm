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
namespace Graphics	{ struct Surface; }
namespace Video		{ class FlicDecoder; }

namespace AGDS {

class AGDSEngine;
class Object;

class Animation {
	Video::FlicDecoder *_flic;
	int					_frames;
	Common::Point		_position;
	Common::String		_process;
	Common::String 		_phaseVar;
	bool				_loop;
	int					_cycles;
	int					_phase;
	bool				_paused;
	int					_speed;
	int					_z;

public:
	Animation();
	~Animation();

	void position(Common::Point position) {
		_position = position;
	}

	const Common::String & phaseVar() const {
		return _phaseVar;
	}

	void phaseVar(const Common::String & phaseVar) {
		_phaseVar = phaseVar;
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

	int phase() const {
		return _phase;
	}

	void play() {
		_paused = false;
		_phase = 0;
	}

	void pause() {
		_paused = true;
	}

	void stop() {
		_phase = -1;
		_paused = true;
	}

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
	void updatePhaseVar(AGDSEngine & engine);
	void paint(AGDSEngine & engine, Graphics::Surface & backbuffer, Common::Point dst);
	int width() const;
	int height() const;
};


} // End of namespace AGDS

#endif /* AGDS_ANIMATION_H */
