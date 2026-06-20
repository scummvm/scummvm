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

#ifndef AUDIO_SYNTH_NFM_OPL_H
#define AUDIO_SYNTH_NFM_OPL_H

#include "audio/fmopl.h"

#ifndef restrict
	#define restrict __restrict__
#endif

extern "C"
{
#include <nfmcore.h>
#include <nfmutil.h>
}

namespace OPL {
namespace NfmOPL {
enum OplDevice : int16_t {
	dtNokturnFM2 = 0,
	dtNokturnFM3,
	dtRWOpl3Express,
	dtOPL2LPT,
	dtOPL3LPT,
	dtOPL2AudioBoard,
	dtOPL3Duo,
	dtStBusIsaVmeSb,
	dtNatfeatsOpl,
	dtNukedOpl3,
	dtNumDevices
};

namespace RealChip {
class OPL : public ::OPL::OPL, public Audio::RealChip {
private:
	Config::OplType _type;
	OplDevice _deviceType;
	sOplInterface _iface;
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
public:
	explicit OPL(Config::OplType type, enum NfmOPL::OplDevice deviceType);
	~OPL();

	bool init() override final;
	void reset() override final;

	void write(int portAddress, int value) override final;
	void writeReg(int reg, int value) override final;

protected:

	virtual void onTimer() override final;
};
};  // End of namespace RealChip

namespace EmulatedChip {
class OPL : public ::OPL::OPL, public Audio::EmulatedChip {
private:
	Config::OplType _type;
	OplDevice _deviceType;
	sOplInterface _iface;
	funcPtrOplWrite _oplWrite;
	funcPtrOplWrite _oplEnqueWrite;
	funcPtrOplFlush _oplFlush;
	funcPtrOplReset _oplReset;
	funcGenerateAudioStream _generateAudioStream;
	sInterfaceInitData _params;
	sOplInterfaceConfiguration _ifaceCfg;

	int _activeReg;
	uint _rate;
	bool _initialized;
	bool _useBuffer;
	bool _incapableDevice;
public:
	explicit OPL(Config::OplType type, enum NfmOPL::OplDevice deviceType);
	~OPL();

	bool isStereo() const override {
		return true;
	}

	bool init() override final;
	void reset() override final;

	void write(int portAddress, int value) override final;
	void writeReg(int reg, int value) override final;

protected:
	void generateSamples(int16 *buffer, int numSamples) override final;
};
}; // End of namespace EmulatedChip

} // End of namespace NfmOPL
} // End of namespace OPL

#endif
