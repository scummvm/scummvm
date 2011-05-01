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
 * $URL:  $
 * $Id:  $
 */

#ifndef TOWNS_MIDI_H
#define TOWNS_MIDI_H

#include "audio/softsynth/fmtowns_pc98/towns_audio.h"
#include "audio/mididrv.h"


class TownsMidiOutputChannel;
class TownsMidiInputChannel;
struct ChanState;

class MidiDriver_TOWNS : public MidiDriver, public TownsAudioInterfacePluginDriver {
friend class TownsMidiInputChannel;
friend class TownsMidiOutputChannel;
public:
	MidiDriver_TOWNS(Audio::Mixer *mixer);
	~MidiDriver_TOWNS();

	int open();
	bool isOpen() const { return _open; }
	void close();
	void send(uint32 b);
	//virtual uint32 property(int prop, uint32 param) { return 0; }
	//virtual void sysEx(const byte *msg, uint16 length) { }
	//virtual void sysEx_customInstrument(byte channel, uint32 type, const byte *instr) { }
	//virtual void metaEvent(byte type, byte *data, uint16 length) { }
	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc);
	uint32 getBaseTempo();
	MidiChannel *allocateChannel();
	MidiChannel *getPercussionChannel();

	void timerCallback(int timerId);

	TownsAudioInterface *intf() { return _intf; }
	
private:
	TownsMidiOutputChannel *allocateOutputChannel(int pri);

	TownsMidiInputChannel **_channels;
	TownsMidiOutputChannel **_out;	

	ChanState *_chanState;

	Common::TimerManager::TimerProc _timerBproc;
	void *_timerBpara;

	TownsAudioInterface *_intf;

	uint32 _tickCounter;
	uint8 _curChan;
	
	bool _open;

	uint8 *_chanOutputLevel;
};

#endif

