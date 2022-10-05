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

#ifndef SCUMM_IMUSE_DRV_GMIDI_H
#define SCUMM_IMUSE_DRV_GMIDI_H

#include "audio/mididrv.h"

namespace Scumm {

class IMuseChannel_GMidi;
class GMidiControlChan;

class IMuseDriver_GMidi : public MidiDriver {
	friend class IMuseChannel_GMidi;
public:
	IMuseDriver_GMidi(MidiDriver::DeviceHandle dev, bool rolandGSMode, bool newSystem);
	~IMuseDriver_GMidi() override;

	int open() override;
	void close() override;

	// Just pass these through...
	bool isOpen() const override { return _drv ? _drv->isOpen() : false; }
	uint32 property(int prop, uint32 param) override { return _drv ? _drv->property(prop, param) : 0; }
	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) override { if (_drv) _drv->setTimerCallback(timerParam, timerProc); }
	uint32 getBaseTempo() override { return _drv ? _drv->getBaseTempo() : 0; }
	void send(uint32 b) override { if (_drv) _drv->send(b); };
	void sysEx(const byte *msg, uint16 length) override { if (_drv) _drv->sysEx(msg, length); } 
	void setPitchBendRange(byte channel, uint range) override { if (_drv) _drv->setPitchBendRange(channel, range); }

	// Channel allocation functions
	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override;

private:
	void initDevice();
	void initDeviceAsRolandGS();
	void createChannels();
	void releaseChannels();

	void setNoteFlag(byte chan, byte note) { if (_notesPlaying && chan < 16 && note < 128) _notesPlaying[note] |= (1 << chan); }
	void clearNoteFlag(byte chan, byte note) { if (_notesPlaying && chan < 16 && note < 128) _notesPlaying[note] &= ~(1 << chan); }
	bool queryNoteFlag(byte chan, byte note) const { return (_notesPlaying && chan < 16 && note < 128) ? _notesPlaying[note] & (1 << chan) : false; }
	void setSustainFlag(byte chan, byte note) { if (_notesSustained && chan < 16 && note < 128) _notesSustained[note] |= (1 << chan); }
	void clearSustainFlag(byte chan, byte note) { if (_notesSustained && chan < 16 && note < 128) _notesSustained[note] &= ~(1 << chan); }
	bool querySustainFlag(byte chan, byte note) const { return (_notesSustained && chan < 16 && note < 128) ? _notesSustained[note] & (1 << chan) : false; }

	MidiDriver *_drv;
	const bool _newSystem;
	const bool _gsMode;
	const byte _numChannels;

	IMuseChannel_GMidi **_imsParts;
	GMidiControlChan **_controlChan;

	GMidiControlChan *_idleChain;
	GMidiControlChan *_activeChain;

	uint16 *_notesPlaying;
	uint16 *_notesSustained;
};

} // End of namespace Scumm

#endif
