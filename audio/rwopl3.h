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

/*
 * Based on RetroWave OPL3 integration code of DOSBox-X
 */

#ifndef AUDIO_SOFTSYNTH_OPL_RWOPL3_H
#define AUDIO_SOFTSYNTH_OPL_RWOPL3_H

#include "audio/fmopl.h"
#include "common/mutex.h"

#if defined(__clang__) && defined(__has_warning)
#  if __has_warning("-Wnewline-eof")
#    define COMPILER_HAS_NEWLINE_EOF_WARNING
#  endif
#endif

#ifdef COMPILER_HAS_NEWLINE_EOF_WARNING
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnewline-eof"
#endif

#include <RetroWaveLib/RetroWave.h>

#ifdef COMPILER_HAS_NEWLINE_EOF_WARNING
#pragma GCC diagnostic pop
#endif

namespace OPL {
namespace RetroWaveOPL3 {

class OPL : public ::OPL::OPL, public Audio::RealChip {
private:
	enum RWConnType {
		RWCONNTYPE_POSIX_SERIAL,
		RWCONNTYPE_WIN32_SERIAL,
		RWCONNTYPE_LINUX_SPI,
		RWCONNTYPE_WEB_SERIAL
	};

	Config::OplType _type;
	int _activeReg;
	bool _initialized;
	RWConnType _connType;
	bool _useBuffer;

	RetroWaveContext _retrowaveGlobalContext;

	Common::Mutex *_rwMutex;

public:
	explicit OPL(Config::OplType type);
	~OPL();

	bool init() override;
	void reset() override;

	void write(int portAddress, int value) override;

	void writeReg(int reg, int value) override;

protected:
	void writeReg(int reg, int value, bool forcePort);
	void onTimer() override;
};

} // End of namespace RetroWaveOPL3
} // End of namespace OPL

#endif
