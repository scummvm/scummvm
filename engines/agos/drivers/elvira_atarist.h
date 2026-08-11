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

#ifndef AGOS_ELVIRA_ATARIST_H
#define AGOS_ELVIRA_ATARIST_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/stream.h"

namespace YM2149 {
class YM2149;
}

namespace AGOS {

class ElviraPrgDriver;

class ElviraAtariSTPlayer {
public:
	ElviraAtariSTPlayer(Common::SeekableReadStream *stream);
	ElviraAtariSTPlayer(Common::SeekableReadStream *stream, uint16 elvira1Tune);
	~ElviraAtariSTPlayer();

	bool isValid() const { return _isValid; }

private:
	friend class ElviraPrgDriver;

	static const uint32 kDefaultFrameHz = 25;
	static const uint32 kElvira1PrgFrameHz = 50;

	Common::ScopedPtr<Common::SeekableReadStream> _stream;
	Common::Array<uint8> _data;
	size_t _pos = 0;
	uint32 _frameHz = kDefaultFrameHz;
	bool _ended = false;
	uint32 _framesLeftInWait = 0;
	uint16 _elvira1Tune = 1;
	ElviraPrgDriver *_elviraPrgDriver = nullptr;
	bool _isValid = true;
	YM2149::YM2149 *_chip = nullptr;

	enum Mode {
		kModeElvira2PKD,
		kModeElvira1PRG
	};

	Mode _mode = kModeElvira2PKD;

	void resetPlayer();
	void parseUntilWait();
	void writeReg(int reg, uint8 data);
	void onTimer();
};

} // namespace AGOS

#endif
