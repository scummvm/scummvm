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

#ifndef IMUSE_DRV_FMTOWNS_H
#define IMUSE_DRV_FMTOWNS_H

#include "audio/softsynth/fmtowns_pc98/towns_audio.h"
#include "audio/mididrv.h"

namespace Scumm {

class TownsMidiOutputChannel;
class TownsMidiInputChannel;
class TownsMidiChanState;

class IMuseDriver_FMTowns : public MidiDriver, public TownsAudioInterfacePluginDriver {
friend class TownsMidiInputChannel;
friend class TownsMidiOutputChannel;
public:
	IMuseDriver_FMTowns(Audio::Mixer *mixer);
	~IMuseDriver_FMTowns() override;

	int open() override;
	bool isOpen() const override { return _isOpen; }
	void close() override;

	void send(uint32 b) override;

	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) override;

	uint32 getBaseTempo() override;
	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override;

	void timerCallback(int timerId) override;

private:
	void updateParser();
	void updateOutputChannels();

	TownsMidiOutputChannel *allocateOutputChannel(uint8 pri);

	int randomValue(int para);

	TownsMidiInputChannel **_channels;
	TownsMidiOutputChannel **_out;
	TownsMidiChanState *_chanState;
	const uint8 _numParts;

	Common::TimerManager::TimerProc _timerProc;
	void *_timerProcPara;

	TownsAudioInterface *_intf;

	uint32 _tickCounter;
	uint8 _allocCurPos;
	uint8 _rand;

	bool _isOpen;

	uint8 *_operatorLevelTable;

	const uint16 _baseTempo;
};

} // end of namespace Scumm

#endif
