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
 */

#include "audio/mixer.h"
#include "common/system.h"

#include "audio/softsynth/opl/mame.h"

#include "mame/sound/fmopl.h"
#include "mame/sound/ymf262.h"

struct FM_OPL;
struct OPL3;

namespace OPL {
namespace MAME {

// emu/drivers/xtal.h
enum {
	XTAL_3_579545MHz    = 3579545,      /* NTSC color subcarrier, extremely common, used on 100's of PCBs (Keytronic custom part #48-300-010 is equivalent) */
	XTAL_14_31818MHz    = 14318181      /* Extremely common, used on 100's of PCBs (4x NTSC subcarrier) */
};

enum {
	// Dual OPL2
	kLeftChannel  = 0,
	kRightChannel = 2,
	kBothChannels = 8,

	// OPL3
	kBank0         = 0,
	kBank1         = 2,
	kBank1Selector = 0x100
};

template <Config::OplType TYPE>
OPL<TYPE>::~OPL() {
	stop();
	shutdown();
}

template <Config::OplType TYPE>
bool OPL<TYPE>::init() {
	shutdown();
	const uint rate = g_system->getMixer()->getOutputRate();
	switch (TYPE) {
	case Config::kDualOpl2:
		_opl2 = ym3812_init(&_device2, XTAL_3_579545MHz, rate);
		if (!_opl2) {
			return false;
		}
		// fall through
	case Config::kOpl2:
		_opl = ym3812_init(&_device, XTAL_3_579545MHz, rate);
		break;
	case Config::kOpl3:
		_opl = ymf262_init(&_device, XTAL_14_31818MHz, rate);
		break;
	}

	return _opl != nullptr;
}

template <Config::OplType TYPE>
void OPL<TYPE>::reset() {
	switch (TYPE) {
	case Config::kDualOpl2:
		ym3812_reset_chip(_opl2);
		// fall through
	case Config::kOpl2:
		ym3812_reset_chip(_opl);
		break;
	case Config::kOpl3:
		ymf262_reset_chip(_opl);
		break;
	}
}

template <Config::OplType TYPE>
void OPL<TYPE>::write(const int port, const int value) {
	switch (TYPE) {
	case Config::kOpl2:
		ym3812_write(_opl, port, value);
		if (isAddressPort(port)) {
			_lastReg = value;
		}
		break;
	case Config::kDualOpl2:
		if (port & kBothChannels) {
			ym3812_write(_opl, port, value);
			ym3812_write(_opl2, port, value);
			if (isAddressPort(port)) {
				_lastReg = _lastReg2 = value;
			}
		} else if (port & kRightChannel) {
			ym3812_write(_opl2, port, value);
			if (isAddressPort(port)) {
				_lastReg2 = value;
			}
		} else {
			ym3812_write(_opl, port, value);
			if (isAddressPort(port)) {
				_lastReg = value;
			}
		}
		break;
	case Config::kOpl3:
		ymf262_write(_opl, port, value);
		if (isAddressPort(port)) {
			if (port & kBank1) {
				_lastReg2 = value;
			} else {
				_lastReg = value;
			}
		}
		break;
	}
}

template <Config::OplType TYPE>
byte OPL<TYPE>::read(const int port) {
	switch (TYPE) {
	case Config::kOpl2:
		return ym3812_read(_opl, port);
	case Config::kDualOpl2:
		if (port & kRightChannel) {
			return ym3812_read(_opl2, port);
		} else {
			return ym3812_read(_opl, port);
		}
		break;
	case Config::kOpl3:
		return ymf262_read(_opl, port);
	}
}

template <Config::OplType TYPE>
void OPL<TYPE>::writeReg(int reg, const int value) {
	switch (TYPE) {
	case Config::kOpl2: {
		const int lastReg = _lastReg;
		write(0x388, reg);
		write(0x389, value);
		write(0x388, lastReg);
		break;
	}
	case Config::kDualOpl2: {
		const int lastReg = _lastReg;
		const int lastReg2 = _lastReg2;
		write(0x388, reg);
		write(0x389, value);
		write(0x220, lastReg);
		write(0x222, lastReg2);
		break;
	}
	case Config::kOpl3: {
		int port = 0x388;
		int lastReg;
		if (reg & kBank1Selector) {
			port += kBank1;
			reg &= ~kBank1Selector;
			lastReg = _lastReg2;
		} else {
			lastReg = _lastReg;
		}

		write(port, reg);
		write(port + 1, value);
		write(port, lastReg);
		break;
	}
	}
}

template <Config::OplType TYPE>
void OPL<TYPE>::generateSamples(int16 *buffer, int length) {
	switch (TYPE) {
	case Config::kOpl2:
		ym3812_update_one(_opl, buffer, length);
		break;
	case Config::kDualOpl2: {
		assert((length % 2) == 0);
		int numSamples = length / 2;
		while (numSamples--) {
			ym3812_update_one(_opl, buffer++, 1);
			ym3812_update_one(_opl2, buffer++, 1);
		}
		break;
	}
	case Config::kOpl3: {
		assert((length % 2) == 0);
		int16 ch3_4;
		int16 *buffers[] = { buffer, buffer + 1, &ch3_4, &ch3_4 };
		int numSamples = length / 2;
		while (numSamples--) {
			ymf262_update_one(_opl, buffers, 1);
			buffers[0] += 2;
			buffers[1] += 2;
		}
		break;
	}
	}
}

template <Config::OplType TYPE>
void OPL<TYPE>::shutdown() {
	if (!_opl) {
		return;
	}

	switch (TYPE) {
	case Config::kDualOpl2:
		ym3812_shutdown(_opl2);
		// fall through
	case Config::kOpl2:
		ym3812_shutdown(_opl);
		break;
	case Config::kOpl3:
		ymf262_shutdown(_opl);
		break;
	}
}

template class OPL<Config::kOpl2>;
template class OPL<Config::kDualOpl2>;
template class OPL<Config::kOpl3>;

} // End of namespace MAME
} // End of namespace OPL
