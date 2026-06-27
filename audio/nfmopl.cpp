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
    for NokturnFM2 / 3, OPL-L carts, CE OPL2 Audio board, CE OPL3 Duo!, Serdaco OPL2LPT / OPL3LPT, RetroWave OPL3 Express and more ...
    (c) 2023-26 Paweł Góralski
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#include "audio/mixer.h"
#include "audio/fmopl.h"
#include "audio/nfmopl.h"

#ifdef ATARI
#include <hwinfo.h>
#endif

#include <nfmoplshadowregs.h>

#ifndef RELEASE_BUILD
	#include "common/debug.h"
	#define NFM_ENABLE_LOGS 1
#endif

#define NFM_ENABLE_BUFFERED_OUTPUT false
#define NFM_ENABLE_CUSTOM_ALLOC 1

// TODO set for Linux / Win
//static const char gsOpl3ExpressPortName[] = "";

#if NFM_ENABLE_CUSTOM_ALLOC
	// custom allocators
	#include "backends/platform/atari/dlmalloc.h"
	#define NFM_MSPACE_SIZE 1*1024
#endif

namespace OPL {
namespace NfmOPL {

#if NFM_ENABLE_CUSTOM_ALLOC

extern "C"
{
	static mspace s_mNfmSpace = nullptr;
	static sNfUserMemoryCallbacks s_MemCallbacks;
	static void *s_nfmMemoryBase = nullptr;

	static void *nfmAlloc(size_t amount, const eNfMemoryFlag flag, void* userData, const char* functionName, char* fileName, uint32_t lineNo) {
#if NFM_ENABLE_LOGS
		debug("nfmAlloc()");
#endif
		return mspace_malloc(s_mNfmSpace, amount);
	}

	static void *nfmAlignedAlloc(size_t alignment, size_t amount, const eNfMemoryFlag flag, void* userData, const char* functionName, char* fileName, uint32_t lineNo) {
#if NFM_ENABLE_LOGS
		debug("nfmAlignedAlloc()");
#endif
		return mspace_memalign(s_mNfmSpace, alignment, amount);
	}

	static void *nfmRealloc(void* pOriginal, size_t size, void* userData) {
#if NFM_ENABLE_LOGS
		debug("nfmRealloc()");
#endif
		return mspace_realloc(s_mNfmSpace, pOriginal, size);
	}

	static void nfmFree(void* ptr, void* userData) {
#if NFM_ENABLE_LOGS
		debug("nfmFree()");
#endif
		mspace_free(s_mNfmSpace, ptr);
	}

	static void nfmOutOfMemoryCb(void* userData) {
#if NFM_ENABLE_LOGS
		debug("nfmOutOfMemoryCb() out of memory!");
#endif
	}

}
#endif

#if NFM_ENABLE_LOGS
static const char *s_DebugConfigMsgStrs[NfmOPL::dtNumDevices] = {
	"Configuring NokturnFM2 cartridge",
	"Configuring NokturnFM3 cartridge",
	"Configuring RetroWave OPL3 Express",
	"Configuring Serdaco OPL2LPT",
	"Configuring Serdaco OPL3LPT",
	"Configuring CE OPL2 Audio Board",
	"Configuring CE OPL3 Duo!",
	"Configuring ST Bus ISA / VME SoundBlaster",
	"Configuring NatFeats / NULL",
	"Configuring Nuked-OPL3",
};

static const char *s_DebugOplWriteStrs[NfmOPL::dtNumDevices] = {
	"NfmOPL NokturnFM2 writeReg",
	"NfmOPL NokturnFM3 writeReg",
	"NfmOPL OPL3 Express writeReg",
	"NfmOPL OPL2LPT writeReg",
	"NfmOPL OPL3LPT writeReg",
	"NfmOPL OPL2AudioBoard writeReg",
	"NfmOPL OPL3Duo writeReg",
	"NfmOPL ST Bus ISA / VME SoundBlaster writeReg",
	"NfmOPL NatFeats / NULL writeReg",
	"NfmOPL Nuked-OPL3 writeReg",
};
#endif
namespace RealChip {
// hardware opl
OPL::OPL(Config::OplType type, NfmOPL::OplDevice deviceType) : _type(type), _deviceType(deviceType), _activeReg(0), _initialized(false), _useBuffer(NFM_ENABLE_BUFFERED_OUTPUT), _incapableDevice(false) {
#if NFM_ENABLE_LOGS
	debug("NfmOPL::RealChip create");
	debug("Requesting hardware info");
#endif

#ifdef ATARI
	Supexec(scOsUpdateHardwareInfo);
#endif

	// defaults
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

#if NFM_ENABLE_LOGS
	if (deviceType < dtNumDevices) {
		debug(s_DebugConfigMsgStrs[deviceType]);
	}
#endif

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
	case dtRWOpl3Express: {
		_useBuffer = true;                                          // use buffered output, sending data has significant overhead
		_params.uOpl3ExpressSettings.outputPort = OPT_USB;
		_ifaceCfg.deviceType = eFmDriverType::FMD_OPL3EXPRESS;
		_ifaceCfg.soundchip = CM_OPL3;
		_ifaceCfg.setup = CC_SINGLE;
		// TODO set serial port on Win / Linux requested by user
	}
	break;
	case dtOPL2LPT: {
		_params.uParam.outputPort = OPT_LPT;

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

		_ifaceCfg.deviceType = eFmDriverType::FMD_OPL3LPT;
		_ifaceCfg.soundchip = CM_OPL3;
		_ifaceCfg.setup = CC_SINGLE;
	}
	break;
	case dtOPL2AudioBoard: {
		_params.uParam.outputPort = OPT_LPT_SPI;
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
		_params.uCeAudioBoardSettings.isOpl2AudioBoard = false;
		_ifaceCfg.deviceType = eFmDriverType::FMD_CE_OPL3DUO_LPT_SPI;
		_ifaceCfg.soundchip = CM_OPL3;
		_ifaceCfg.setup = CC_SINGLE;
	}
	break;
	case dtStBusIsaVmeSb: {
		// TODO: handle additional VME / ISA parameters if needed
		_params.uParam.outputPort = OPT_ISA;
		_params.uParam.param = 0;

		_ifaceCfg.deviceType = eFmDriverType::FMD_ISA_SB;
		_ifaceCfg.soundchip = CM_OPL3;
		_ifaceCfg.operationMode = CO_OPL2,
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
	if (_initialized == true) {
#if NFM_ENABLE_LOGS
		debug("NfmOPL destroy");
#endif
		if (_useBuffer) {
			// flush
#if NFM_ENABLE_LOGS
			debug("OPL flush");
#endif
			_oplFlush();
		}

		_oplWrite = nullptr;
		_oplEnqueWrite = nullptr;
		_oplFlush = nullptr;
		_oplReset = nullptr;

		(void)nfDestroyInterface(&_iface);

		_incapableDevice = false;
		_useBuffer = false;
		_initialized = false;
#if NFM_ENABLE_CUSTOM_ALLOC
		if (s_mNfmSpace) {
			destroy_mspace(s_mNfmSpace);
			Mfree(s_nfmMemoryBase);
			s_nfmMemoryBase = nullptr;
		}
#endif
	}
}

bool OPL::init() {
#if NFM_ENABLE_LOGS
	debug("NfmOPL::RealChip init");
#endif
	if (_incapableDevice) {
#if NFM_ENABLE_LOGS
	debug("NfmOPL::RealChip OPL2 device cannot emulate requested dual OPL2 / OPL3!");
#endif
		return false;
	}

#if NFM_ENABLE_CUSTOM_ALLOC

	s_nfmMemoryBase = (void *)Mxalloc((int32_t)NFM_MSPACE_SIZE + 256, (int16_t)3);

	if (s_nfmMemoryBase) {
		s_mNfmSpace = create_mspace_with_base(s_nfmMemoryBase, NFM_MSPACE_SIZE, 0);

		if (s_mNfmSpace == 0) {
#if NFM_ENABLE_LOGS
			debug("NfmOPL::RealChip create_mspace failed!");
#endif
			return false;
		}

		// install user memory allocator callbacks
		s_MemCallbacks.alloc = nfmAlloc;
		s_MemCallbacks.alignedAlloc = nfmAlignedAlloc;
		s_MemCallbacks.release = nfmFree;
		s_MemCallbacks.realloc = nfmRealloc;
		s_MemCallbacks.outOfMemory = nfmOutOfMemoryCb;

		nfInit(&s_MemCallbacks, NULL);

	} else {
#if NFM_ENABLE_LOGS
		debug("NfmOPL::RealChip Out of system memory!");
#endif
		return false;
	}
#else
	nfInit(NULL,NULL);
#endif
	_iface = nfCreateInterface(_ifaceCfg);

	if (_iface.setup != CC_UNDEFINED) {
		const int32_t retval = nfInitialiseInterface(&_iface, &_params);

		if (retval >= 0) {
			_oplWrite = _iface.write;
			_oplEnqueWrite = _iface.enqueWrite;
			_oplFlush = _iface.flush;
			_oplReset = _iface.reset;

			initDualOpl2OnOpl3(_type);
			_initialized = true;

#if NFM_ENABLE_LOGS
			debug("NfmOPL::RealChip init OK");
#endif
			return true;
		}
	}

#if NFM_ENABLE_LOGS
	debug("NfmOPL::RealChip init failed!");
#endif
	return false;
}

void OPL::reset() {
#if NFM_ENABLE_LOGS
	debug("NfmOPL::RealChip reset");
#endif
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
#if NFM_ENABLE_LOGS
	debug(s_DebugOplWriteStrs[_deviceType]);
#endif

	if (_type == Config::kOpl3 || _type == Config::kDualOpl2) {
		reg &= 0x1ff;
	} else {
		reg &= 0xff;
	}

	value &= 0xff;

	if (emulateDualOpl2OnOpl3(reg, value, _type)) {
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
#if NFM_ENABLE_LOGS
			// flush
			debug("NfmOPL::RealChip flush");
#endif
			_oplFlush();
		}
	}

	Audio::RealChip::onTimer();
}

OPL *create(Config::OplType type, OplDevice device) {
	return new OPL(type, device);
}
} // End of namespace RealChip

namespace EmulatedChip {
OPL::OPL(Config::OplType type, enum NfmOPL::OplDevice deviceType): _type(type), _rate(0), _deviceType(deviceType), _activeReg(0), _initialized(false), _useBuffer(NFM_ENABLE_BUFFERED_OUTPUT), _incapableDevice(false) {
#if NFM_ENABLE_LOGS
	debug("NfmOPL::EmulatedChip create");
	debug("Requesting hardware info");
#endif

#ifdef ATARI
	Supexec(scOsUpdateHardwareInfo);
#endif
	
	// defaults
	_ifaceCfg.deviceType = eFmDriverType::FMD_UNDEFINED;
	_ifaceCfg.soundchip = CM_UNDEFINED;
	_ifaceCfg.operationMode = CO_UNDEFINED;
	_ifaceCfg.setup = CC_UNDEFINED;
	_ifaceCfg.dualChipEmulationEnabled = false;

	_oplWrite = nullptr;
	_oplEnqueWrite = nullptr;
	_oplFlush = nullptr;
	_oplReset = nullptr;
	_generateAudioStream = nullptr;

	if (_type == Config::kOpl2) {
		_ifaceCfg.operationMode = CO_OPL2;
	}

	if (_type == Config::kOpl3 || _type == Config::kDualOpl2) {
		_ifaceCfg.operationMode = CO_OPL3;
	}

	_ifaceCfg.setup = CC_SINGLE;

#if NFM_ENABLE_LOGS
	if (deviceType < dtNumDevices) {
		debug(s_DebugConfigMsgStrs[deviceType]);
	}
#endif
	switch (deviceType) {
	case dtNukedOpl3: {
		_rate = g_system->getMixer()->getOutputRate();
		_params.uSoftSynthSettings.outputPort = OPT_INTERNAL;
		_params.uSoftSynthSettings.sampleRate = _rate;
		_params.uSoftSynthSettings.enableDualChipEmulation = _ifaceCfg.dualChipEmulationEnabled;

		_ifaceCfg.deviceType = FMD_NUKEDOPL3;
		_ifaceCfg.soundchip = CM_OPL3;
		_ifaceCfg.operationMode = CO_OPL3;
		_ifaceCfg.setup = CC_SINGLE;
	}
	break;

	default: {
		warning("NfmOPL::EmulatedChip Unrecognized device type or not software synthesizer!");
		_incapableDevice = true;
	}
	break;
	};
}

OPL::~OPL() {

	if (_initialized == true) {
#if NFM_ENABLE_LOGS
		debug("NfmOPL::EmulatedChip destroy");
#endif
		stop();

		if (_useBuffer) {
#if NFM_ENABLE_LOGS
			// flush
			debug("NfmOPL::EmulatedChip OPL flush");
#endif
			_oplFlush();
		}

		_oplWrite = nullptr;
		_oplEnqueWrite = nullptr;
		_oplFlush = nullptr;
		_oplReset = nullptr;
		_generateAudioStream = nullptr;

		(void)nfDestroyInterface(&_iface);

		_incapableDevice = false;
		_useBuffer = false;
		_initialized = false;

#if NFM_ENABLE_CUSTOM_ALLOC
		if (s_mNfmSpace) {
			destroy_mspace(s_mNfmSpace);
			Mfree(s_nfmMemoryBase);
			s_nfmMemoryBase = nullptr;
		}
#endif
	}
}

bool OPL::init() {
#if NFM_ENABLE_LOGS
	debug("NfmOPL::EmulatedChip init");
#endif
	if (_incapableDevice) {
#if NFM_ENABLE_LOGS
		debug("NfmOPL::EmulatedChip device isn't soft synth type!");
#endif
		return false;
	}

#if NFM_ENABLE_CUSTOM_ALLOC

	s_nfmMemoryBase = (void *)Mxalloc((int32_t)NFM_MSPACE_SIZE + 256, (int16_t)3);

	if (s_nfmMemoryBase) {
		s_mNfmSpace = create_mspace_with_base(s_nfmMemoryBase, NFM_MSPACE_SIZE, 0);

		if (s_mNfmSpace == 0) {
#if NFM_ENABLE_LOGS
			debug("NfmOPL::EmulatedChip create_mspace failed!");
#endif
			return false;
		}

		// install custom memory allocator callbacks
		s_MemCallbacks.alloc = nfmAlloc;
		s_MemCallbacks.alignedAlloc = nfmAlignedAlloc;
		s_MemCallbacks.release = nfmFree;
		s_MemCallbacks.realloc = nfmRealloc;
		s_MemCallbacks.outOfMemory = nfmOutOfMemoryCb;

		nfInit(&s_MemCallbacks,NULL);

	} else {
#if NFM_ENABLE_LOGS
		debug("NfmOPL::EmulatedChip Out of system memory!");
#endif
		return false;
	}
#else
	nfInit(NULL,NULL);
#endif
	_iface = nfCreateInterface(_ifaceCfg);

	if (_iface.setup != CC_UNDEFINED) {
		const int32_t retval = nfInitialiseInterface(&_iface, &_params);

		if (retval >= 0) {
			_oplWrite = _iface.write;
			_oplEnqueWrite = _iface.enqueWrite;
			_oplFlush = _iface.flush;
			_oplReset = _iface.reset;
			_generateAudioStream = _iface.generateAudioStream;

			_activeReg = 0;

			initDualOpl2OnOpl3(_type);

			_initialized = true;

#if NFM_ENABLE_LOGS
			debug("NfmOPL::RealChip init OK");
#endif
			return true;
		}
	}

#if NFM_ENABLE_LOGS
	debug("NfmOPL::RealChip init failed!");
#endif
	return false;
}

void OPL::reset() {

	_oplReset();

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

		warning("NfmOPL::EmulatedChip: unsupported OPL mode %d", _type);
	}
}

void OPL::writeReg(int reg, int value) {
#if NFM_ENABLE_LOGS
	debug(s_DebugOplWriteStrs[_deviceType]);
#endif

	if (_type == Config::kOpl3 || _type == Config::kDualOpl2) {
		reg &= 0x1ff;
	} else {
		reg &= 0xff;
	}

	value &= 0xff;

	if (emulateDualOpl2OnOpl3(reg, value, _type)) {
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

void OPL::generateSamples(int16 *buffer, int length) {
	assert(buffer != nullptr);
	assert(length!=0);
	_generateAudioStream(buffer, 0, (uint16_t)length / 2);
}

OPL *create(Config::OplType type, OplDevice device) {
	return new OPL(type, device);
}
} // End of namespace EmulatedChip

} // End of namespace NfmOPL
} // End of namespace OPL
