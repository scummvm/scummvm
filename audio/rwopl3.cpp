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

// Allow forbidden symbols for included library headers
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "rwopl3.h"

#include <RetroWaveLib/Board/OPL3.h>
#include <RetroWaveLib/Platform/Linux_SPI.h>
#include <RetroWaveLib/Platform/POSIX_SerialPort.h>
#include <RetroWaveLib/Platform/Win32_SerialPort.h>

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/tokenizer.h"

namespace OPL {
namespace RetroWaveOPL3 {

OPL::OPL(Config::OplType type) : _type(type), _activeReg(0), _initialized(false), _connType(RWCONNTYPE_POSIX_SERIAL), _useBuffer(true) {
	_rwMutex = new Common::Mutex();
	_retrowaveGlobalContext = { nullptr, nullptr, nullptr, 0, 0, 0 };
}

OPL::~OPL() {
	_rwMutex->lock();

	if (_initialized) {
		reset();
		retrowave_deinit(&_retrowaveGlobalContext);

		switch (_connType) {
		case RWCONNTYPE_POSIX_SERIAL:
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
			retrowave_deinit_posix_serialport(&_retrowaveGlobalContext);
#endif
			break;
		case RWCONNTYPE_WIN32_SERIAL:
#ifdef WIN32
			retrowave_deinit_win32_serialport(&_retrowaveGlobalContext);
#endif
			break;
		case RWCONNTYPE_LINUX_SPI:
#if defined(__linux__)
			retrowave_deinit_linux_spi(&_retrowaveGlobalContext);
#endif
			break;
		}

		_initialized = false;
	}

	_rwMutex->unlock();
}

bool OPL::init() {
	Common::String bus = ConfMan.get("retrowaveopl3_bus");
	Common::String port = ConfMan.get("retrowaveopl3_port");
	Common::String spiCs = ConfMan.get("retrowaveopl3_spi_cs");
	_useBuffer = ConfMan.hasKey("retrowaveopl3_disable_buffer") ? !ConfMan.getBool("retrowaveopl3_disable_buffer") : true;

	int rc = -1;

	_rwMutex->lock();

	if (bus == "serial") {
		if (port.empty()) {
			warning("RWOPL3: Missing port specification.");
		} else {
#if defined(__linux__) || defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
			char buf[128];
			snprintf(buf, sizeof(buf) - 1, "/dev/%s", port.c_str());

			rc = retrowave_init_posix_serialport(&_retrowaveGlobalContext, buf);
			_connType = RWCONNTYPE_POSIX_SERIAL;
#endif

#ifdef WIN32
			rc = retrowave_init_win32_serialport(&_retrowaveGlobalContext, port.c_str());
			_connType = RWCONNTYPE_WIN32_SERIAL;
#endif
		}
	} else if (bus == "spi") {
#if defined(__linux__)
		Common::StringTokenizer *st = new Common::StringTokenizer(spiCs, ",");
		Common::String gpiochip = st->nextToken();
		Common::String line = st->nextToken();
		int scg[2] = {0};

		if (gpiochip.empty() || line.empty()) {
			warning("RWOPL3: Bad GPIO specification. Please use the 'gpiochip,line' format.");
		} else {
			scg[0] = strtol(gpiochip.c_str(), nullptr, 10);
			scg[1] = strtol(line.c_str(), nullptr, 10);

			debug("RWOPL3: SPI CS: chip=%d, line=%d\n", scg[0], scg[1]);

			rc = retrowave_init_linux_spi(&_retrowaveGlobalContext, port.c_str(), scg[0], scg[1]);
			_connType = RWCONNTYPE_LINUX_SPI;
		}
#else
		warning("RWOPL3: SPI is not supported on your platform.");
#endif
	} else {
		warning("RWOPL3: Bad bus specification. Valid values are \"serial\" and \"spi\".");
	}

	if (rc < 0) {
		warning("RWOPL3: Failed to init board - init returned %d.", rc);
	} else {
		retrowave_io_init(&_retrowaveGlobalContext);
		_initialized = true;
	}

	_rwMutex->unlock();

	return rc >= 0;
}

void OPL::reset() {
	_rwMutex->lock();

	if (_initialized) {
		retrowave_opl3_reset(&_retrowaveGlobalContext);

		writeReg(0x01, 0, true);
		writeReg(0x02, 0, true);
		writeReg(0x03, 0, true);
		writeReg(0x04, 0x60, true);
		writeReg(0x04, 0x80, true);
		writeReg(0x104, 0, true);
		writeReg(0x105, 0, true);
		writeReg(0x08, 0, true);

		for (int offset = 0; offset <= 0x100; offset += 0x100) {
			for (int reg = 0x20; reg <= 0xF5; reg++) {
				writeReg(offset + reg, 0, true);
			}
		}
	}

	_rwMutex->unlock();
}

void OPL::write(int portAddress, int value) {
	if (portAddress & 1) {
		writeReg(_activeReg, value);
	} else {
		if (_type == Config::kOpl2) {
			_activeReg = value & 0xFF;
		} else {
			_activeReg = ((portAddress & 2) << 7) | value;
		}
	}
}

byte OPL::read(int portAddress) {
	// Reads are not supported by the RetroWave OPL3.
	return 0;
}

void OPL::writeReg(int reg, int value) {
	writeReg(reg, value, false);
}

void OPL::writeReg(int reg, int value, bool forcePort) {
	int portReg = reg & 0xFF;
	value &= 0xFF;

	_rwMutex->lock();

	if (_initialized) {
		if (reg & 0x100 && (forcePort || _type != Config::kOpl2)) {
			// Write to the second register set.
			if (_useBuffer) {
				retrowave_opl3_queue_port1(&_retrowaveGlobalContext, portReg, value);
			} else {
				retrowave_opl3_emit_port1(&_retrowaveGlobalContext, portReg, value);
			}
		} else {
			// Write to the first register set.
			if (_useBuffer) {
				retrowave_opl3_queue_port0(&_retrowaveGlobalContext, portReg, value);
			} else {
				retrowave_opl3_emit_port0(&_retrowaveGlobalContext, portReg, value);
			}
		}
	}

	_rwMutex->unlock();
}

void OPL::onTimer() {
	if (_useBuffer) {
		_rwMutex->lock();

		if (_initialized)
			retrowave_flush(&_retrowaveGlobalContext);

		_rwMutex->unlock();
	}

	RealOPL::onTimer();
}

OPL *create(Config::OplType type) {
	return new OPL(type);
}

} // End of namespace RetroWaveOPL3
} // End of namespace OPL

