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
    OPL interface using nFM library(https://framagit.org/nokturnal/nfm)
    for NokturnFM2 / 3, CE OPL2 Audio board, CE OPL3 Duo!, Serdaco OPL2LPT / OPL3LPT, ST Bus ISA / VME SoundBlaster and NatFeats
    (c) 2023-26 Paweł Góralski
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#include "audio/mixer.h"
#include "audio/fmopl.h"
#include "audio/nfmopl.h"

extern "C"
{
#include <nfmcore.h>
#include <nfmutil.h>
}

#include <mint/osbind.h>

#define NFM_ENABLE_BUFFERED_OUTPUT false

namespace OPL {
namespace NfmOPL {

namespace RealChip {

class OPL : public ::OPL::OPL, public Audio::RealChip {
private:
	Config::OplType _type;
	OplDevice _deviceType;
	sFmInterface _iface;
	funcPtrOplWrite _oplWrite;
	funcPtrOplWrite _oplEnqueWrite;
	funcPtrOplFlush _oplFlush;
	funcPtrOplReset _oplReset;

	sInterfaceInitData _params;
	sOplInterfaceConfiguration _ifaceCfg;

	int _activeReg;
	bool _initialized;
	bool _useBuffer;
	bool _incapableDevice;
	bool _needsSupervisor;	// driver accesses I/O registers directly
	bool _inSupervisor;		// set while a SupervisorScope holds supervisor mode
public:
	explicit OPL(Config::OplType type, enum NfmOPL::OplDevice deviceType);
	~OPL();

	bool init() override final;
	void reset() override final;

	void write(int portAddress, int value) override final;
	void writeReg(int reg, int value) override final;

protected:

	void onTimer() override final;
};

// Drivers which access I/O registers directly (e.g. the parallel port through
// the YM2149 at $FFFF8800) cause a bus error in user mode, which is where all
// ScummVM threads run. The cartridge port and NatFeats are accessible from user
// mode. ISA access goes through an _ISA cookie driver or machine specific
// addresses whose requirements are not known, so it uses supervisor mode as a
// precaution. Nested scopes are cheap: only the outermost one enters
// supervisor mode.
class SupervisorScope {
public:
	SupervisorScope(bool needed, bool &active) : _active(active), _oldSsp(nullptr) {
		if (needed && !_active && Super(SUP_INQUIRE) == 0) {
			_oldSsp = (void *)Super(SUP_SET);
			_active = true;
		}
	}

	~SupervisorScope() {
		if (_oldSsp) {
			_active = false;
			SuperToUser(_oldSsp);
		}
	}

private:
	bool &_active;
	void *_oldSsp;
};

// hardware opl
OPL::OPL(Config::OplType type, NfmOPL::OplDevice deviceType) : _type(type), _deviceType(deviceType), _activeReg(0), _initialized(false), _useBuffer(NFM_ENABLE_BUFFERED_OUTPUT), _incapableDevice(false), _needsSupervisor(false), _inSupervisor(false) {
	// defaults
	memset(&_params, 0, sizeof(_params));
	_ifaceCfg.deviceType = eFmDriverType::FMD_UNDEFINED;
	_ifaceCfg.soundchip = CM_UNDEFINED;
	_ifaceCfg.operationMode = CO_UNDEFINED;
	_ifaceCfg.setup = CC_UNDEFINED;
	_ifaceCfg.dualChipEmulationEnabled = false;
	
	_oplWrite = nullptr;
	_oplEnqueWrite = nullptr;
	_oplFlush = nullptr;
	_oplReset = nullptr;

	if (_type == Config::kOpl2) {
		_ifaceCfg.operationMode = CO_OPL2;
	}

	if (_type == Config::kOpl3 || _type == Config::kDualOpl2) {
		_ifaceCfg.operationMode = CO_OPL3;
	}

	_ifaceCfg.setup = CC_SINGLE;

	switch (deviceType) {
	case dtNokturnFM2: {
		_params.uParam.outputPort = OPT_ST_CART;
		_ifaceCfg.deviceType = eFmDriverType::FMD_OPLCART;
		_ifaceCfg.soundchip = CM_OPL2;
		_ifaceCfg.setup = CC_SINGLE;

		if (_type == Config::kOpl3 || _type == Config::kDualOpl2) {
			_incapableDevice = true;
		}
	}
	break;
	case dtNokturnFM3: {
		_params.uParam.outputPort = OPT_ST_CART;
		_ifaceCfg.deviceType = eFmDriverType::FMD_OPLCART;
		_ifaceCfg.soundchip = CM_OPL3;
		_ifaceCfg.setup = CC_SINGLE;
	}
	break;
	case dtOPL2LPT: {
		_params.uParam.outputPort = OPT_LPT;
		_needsSupervisor = true;

		_ifaceCfg.deviceType = eFmDriverType::FMD_OPL2LPT;
		_ifaceCfg.soundchip = CM_OPL2;
		_ifaceCfg.setup = CC_SINGLE;

		if (_type == Config::kOpl3 || _type == Config::kDualOpl2) {
			_incapableDevice = true;
		}
	}
	break;
	case dtOPL3LPT: {
		// OPL2 mode is forced internally on anything below TT due to lack of signals
		_params.uParam.outputPort = OPT_LPT;
		_needsSupervisor = true;

		_ifaceCfg.deviceType = eFmDriverType::FMD_OPL3LPT;
		_ifaceCfg.soundchip = CM_OPL3;
		_ifaceCfg.setup = CC_SINGLE;
	}
	break;
	case dtOPL2AudioBoard: {
		_params.uParam.outputPort = OPT_LPT_SPI;
		_needsSupervisor = true;
		_params.uCeAudioBoardSettings.isOpl2AudioBoard = true;
		_ifaceCfg.deviceType = eFmDriverType::FMD_CE_OPL2AUDIO_LPT_SPI;
		_ifaceCfg.soundchip = CM_OPL2;
		_ifaceCfg.setup = CC_SINGLE;

		if (_type == Config::kOpl3 || _type == Config::kDualOpl2) {
			_incapableDevice = true;
		}
	}
	break;
	case dtOPL3Duo: {
		_params.uParam.outputPort = OPT_LPT_SPI;
		_needsSupervisor = true;
		_params.uCeAudioBoardSettings.isOpl2AudioBoard = false;
		_ifaceCfg.deviceType = eFmDriverType::FMD_CE_OPL3DUO_LPT_SPI;
		_ifaceCfg.soundchip = CM_OPL3;
		_ifaceCfg.setup = CC_SINGLE;
	}
	break;
	case dtStBusIsaVmeSb: {
		// TODO: handle additional VME / ISA parameters if needed
		_params.uParam.outputPort = OPT_ISA;
		_needsSupervisor = true;	// precaution, see SupervisorScope
		_params.uParam.param = 0;

		_ifaceCfg.deviceType = eFmDriverType::FMD_ISA_SB;
		_ifaceCfg.soundchip = CM_OPL3;
		_ifaceCfg.setup = CC_SINGLE;
	}
	break;
	case dtNatfeatsOpl: {
		_params.uParam.outputPort = OPT_INTERNAL;
		_ifaceCfg.deviceType = eFmDriverType::FMD_NULL;
		_ifaceCfg.soundchip = CM_OPL3;
		_ifaceCfg.setup = CC_SINGLE;
	}
	break;
	default: {
		warning("NfmOPL::RealChip Unrecognized device type or software synthesizer was requested.");
	}
	break;
	};
}

OPL::~OPL() {
	// stop the timer callbacks before the interface goes away
	stop();

	if (_initialized == true) {
		SupervisorScope supervisor(_needsSupervisor, _inSupervisor);

		if (_useBuffer) {
			// flush
			_oplFlush();
		}

		_oplWrite = nullptr;
		_oplEnqueWrite = nullptr;
		_oplFlush = nullptr;
		_oplReset = nullptr;

		(void)nfDestroyInterface(&_iface);
		(void)nfDeinit();

		_incapableDevice = false;
		_useBuffer = false;
		_initialized = false;
	}
}

bool OPL::init() {
	if (_incapableDevice) {
		return false;
	}

	nfInit(NULL, NULL);
	_iface = nfCreateInterface(_ifaceCfg);

	if (_iface.setup != CC_UNDEFINED) {
		SupervisorScope supervisor(_needsSupervisor, _inSupervisor);

		const int32_t retval = nfInitialiseInterface(&_iface, &_params);

		if (retval >= 0) {
			_oplWrite = _iface.write;
			_oplEnqueWrite = _iface.enqueWrite;
			_oplFlush = _iface.flush;
			_oplReset = _iface.reset;

			initDualOpl2OnOpl3(_type);
			_initialized = true;

			return true;
		}

		(void)nfDestroyInterface(&_iface);
	}

	(void)nfDeinit();

	return false;
}

void OPL::reset() {
	SupervisorScope supervisor(_needsSupervisor, _inSupervisor);

	for (int16_t i = 0; i < 256; i ++) {
		writeReg((int)i, 0);
	}

	if (_type == Config::kOpl3 || _type == Config::kDualOpl2) {
		for (int16_t i = 0; i < 256; i++) {
			writeReg((int)i + 256, 0);
		}
	}

	_activeReg = 0;

	initDualOpl2OnOpl3(_type);
}

void OPL::write(int portAddress, int value) {
	if (portAddress & 1) {
		writeReg(_activeReg, value);
		return;
	} else {
		if (_type == Config::kOpl2) {
			_activeReg = value & 0xff;
			return;
		} else {
			// opl3 / dual opl2
			_activeReg = (value & 0xff) | ((portAddress << 7) & 0x100);
			return;
		}

		warning("NfmOPL::RealChip: unsupported OPL mode %d", _type);
	}
}

void OPL::writeReg(int reg, int value) {
	if (_type == Config::kOpl3 || _type == Config::kDualOpl2) {
		reg &= 0x1ff;
	} else {
		reg &= 0xff;
	}

	value &= 0xff;

	if (emulateDualOpl2OnOpl3(reg, value, _type)) {
		SupervisorScope supervisor(_needsSupervisor, _inSupervisor);
		sOplRegisterWrite regWrite;

		if (reg < 0x100) {
			regWrite = {0, (uint8_t)reg, (uint8_t)value};
		} else {
			regWrite = {1, (uint8_t)(reg - 0x100), (uint8_t)value};
		}

		if (_useBuffer) {
			_oplEnqueWrite(&regWrite);
		} else {
			_oplWrite(&regWrite);
		}
	}
}

void OPL::onTimer() {
	if (_useBuffer) {
		if (_initialized) {
			SupervisorScope supervisor(_needsSupervisor, _inSupervisor);
			_oplFlush();
		}
	}

	Audio::RealChip::onTimer();
}

::OPL::OPL *create(Config::OplType type, OplDevice device) {
	return new OPL(type, device);
}
} // End of namespace RealChip

} // End of namespace NfmOPL
} // End of namespace OPL
