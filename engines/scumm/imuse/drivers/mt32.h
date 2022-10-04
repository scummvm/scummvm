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

#ifndef SCUMM_IMUSE_DRV_MT32_H
#define SCUMM_IMUSE_DRV_MT32_H

#include "audio/mididrv.h"

namespace Scumm {

class IMuseChannel_MT32;
class MT32RealChan;
class MT32ControlChan;

class IMuseDriver_MT32 : public MidiDriver {
	friend class IMuseChannel_MT32;
public:
	IMuseDriver_MT32(MidiDriver::DeviceHandle dev, bool newSystem);
	~IMuseDriver_MT32() override;

	int open() override;
	void close() override;

	// Just pass these through...
	bool isOpen() const override { return _drv ? _drv->isOpen() : false; }
	uint32 property(int prop, uint32 param) override { return _drv ? _drv->property(prop, param) : 0; }
	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) override { if (_drv) _drv->setTimerCallback(timerParam, timerProc); }
	uint32 getBaseTempo() override { return _drv ? _drv->getBaseTempo() : 0; }
	void send(uint32 b) override { if (_drv) _drv->send(b); };
	void sysEx(const byte *msg, uint16 length) override { if (_drv) _drv->sysEx(msg, length); } 

	// Channel allocation functions
	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override;

private:
	void initDevice();
	void createChannels();
	void releaseChannels();

	IMuseChannel_MT32 *getPart(int number) const;

	// Convenience function that allows to send the sysex message with the exact same arguments as they are used in the original drivers.
	void sendMT32Sysex(uint32 addr, const byte *data, uint32 dataSize);

	void setNoteFlag(byte chan, byte note) { if (_notesPlaying && chan < 16 && note < 128) _notesPlaying[note] |= (1 << chan); }
	void clearNoteFlag(byte chan, byte note) { if (_notesPlaying && chan < 16 && note < 128) _notesPlaying[note] &= ~(1 << chan); }
	bool queryNoteFlag(byte chan, byte note) const { return (_notesPlaying && chan < 16 && note < 128) ? _notesPlaying[note] & (1 << chan) : false; }
	void setSustainFlag(byte chan, byte note) { if (_notesSustained && chan < 16 && note < 128) _notesSustained[note] |= (1 << chan); }
	void clearSustainFlag(byte chan, byte note) { if (_notesSustained && chan < 16 && note < 128) _notesSustained[note] &= ~(1 << chan); }
	bool querySustainFlag(byte chan, byte note) const { return (_notesSustained && chan < 16 && note < 128) ? _notesSustained[note] & (1 << chan) : false; }

	MidiDriver *_drv;
	const bool _newSystem;
	const byte _numChannels;

	IMuseChannel_MT32 **_imsParts;
	MT32RealChan **_hwOutputChan;
	MT32ControlChan **_controlChan;

	MT32ControlChan *_idleChain;
	MT32RealChan *_hwChain;
	MT32ControlChan *_activeChain;

	const byte *_programsMapping;

	uint16 *_notesPlaying;
	uint16 *_notesSustained;
};

} // End of namespace Scumm

#endif
