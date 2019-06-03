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

#include "agos/drivers/accolade/mididriver.h"

#include "audio/mididrv.h"

#include "common/mutex.h"

namespace AGOS {

class MidiDriver_Accolade_MT32 : public MidiDriver {
public:
	MidiDriver_Accolade_MT32();
	virtual ~MidiDriver_Accolade_MT32();

	// MidiDriver
	int open();
	void close();
	bool isOpen() const { return _isOpen; }

	void send(uint32 b);

	MidiChannel *allocateChannel() {
		if (_driver)
			return _driver->allocateChannel();
		return NULL;
	}
	MidiChannel *getPercussionChannel() {
		if (_driver)
			return _driver->getPercussionChannel();
		return NULL;
	}

	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
		if (_driver)
			_driver->setTimerCallback(timer_param, timer_proc);
	}

	uint32 getBaseTempo() {
		if (_driver) {
			return _driver->getBaseTempo();
		}
		return 1000000 / _baseFreq;
	}

protected:
	Common::Mutex _mutex;
	MidiDriver *_driver;
	bool _nativeMT32; // native MT32, may also be our MUNT, or MUNT over MIDI

	bool _isOpen;
	int _baseFreq;

private:
	// simple mapping between MIDI channel and MT32 channel
	byte _channelMapping[AGOS_MIDI_CHANNEL_COUNT];
	// simple mapping between MIDI instruments and MT32 instruments
	byte _instrumentMapping[AGOS_MIDI_INSTRUMENT_COUNT];

public:
	bool setupInstruments(byte *instrumentData, uint16 instrumentDataSize, bool useMusicDrvFile);
};

} // End of namespace AGOS
