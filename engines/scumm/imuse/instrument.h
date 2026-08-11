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

#ifndef SCUMM_IMUSE_INSTRUMENT_H
#define SCUMM_IMUSE_INSTRUMENT_H


#include "common/scummsys.h"
#include "common/serializer.h"

class MidiChannel;

namespace Scumm {

class Instrument;

class InstrumentInternal : public Common::Serializable {
public:
	~InstrumentInternal() override {}
	virtual void send(MidiChannel *mc) = 0;
	virtual void copy_to(Instrument *dest) = 0;
	virtual bool is_valid() = 0;
};

class Instrument : public Common::Serializable {
private:
	byte _type;
	InstrumentInternal *_instrument;

public:
	enum {
		itNone = 0,
		itProgram = 1,
		itAdLib = 2,
		itRoland = 3,
		itPcSpk = 4,
		itMacDeprecated = 5
	};

	Instrument() : _type(0), _instrument(0), _nativeMT32Device(false) { }
	~Instrument() override { delete _instrument; }
	void setNativeMT32Mode(bool isNativeMT32) { _nativeMT32Device = isNativeMT32; }
	static const byte _gmRhythmMap[35];

	void clear();
	void copy_to(Instrument *dest) {
		if (_instrument)
			_instrument->copy_to(dest);
		else
			dest->clear();
	}

	void program(byte program, byte bank, bool mt32SoundType);
	void adlib(const byte *instrument);
	void roland(const byte *instrument);
	void pcspk(const byte *instrument);

	byte getType() { return _type; }
	bool isValid() { return (_instrument ? _instrument->is_valid() : false); }
	void saveLoadWithSerializer(Common::Serializer &s) override;
	void send(MidiChannel *mc) {
		if (_instrument)
			_instrument->send(mc);
	}

	bool _nativeMT32Device;
};

} // End of namespace Scumm

#endif
