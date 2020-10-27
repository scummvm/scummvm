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

#ifndef IMUSE_DRV_AMIGA_H
#define IMUSE_DRV_AMIGA_H

#include "audio/mididrv.h"
#include "audio/mods/paula.h"
#include "audio/mixer.h"

namespace Scumm {

class IMusePart_Amiga;
class SoundChannel_Amiga;
struct Instrument_Amiga;

class IMuseDriver_Amiga : public MidiDriver, public Audio::Paula {
friend class SoundChannel_Amiga;
public:
	IMuseDriver_Amiga(Audio::Mixer *mixer);
	~IMuseDriver_Amiga() override;

	int open() override;
	bool isOpen() const override { return _isOpen; }
	void close() override;

	void send(uint32 b) override;

	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) override;

	uint32 getBaseTempo() override;
	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override;

	void interrupt() override;

private:
	void updateParser();
	void updateSounds();

	void loadInstrument(int program);
	void unloadInstruments();

	IMusePart_Amiga **_parts;
	SoundChannel_Amiga **_chan;

	Common::TimerManager::TimerProc _timerProc;
	void *_timerProcPara;

	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;

	int32 _ticker;
	bool _isOpen;

	Instrument_Amiga *_instruments;
	uint16 _missingFiles;

	const int32 _baseTempo;
	const int32 _internalTempo;
	const uint8 _numParts;
};

}

#endif
