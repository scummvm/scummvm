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

#ifndef SCUMM_IMUSE_DRV_MIDI_H
#define SCUMM_IMUSE_DRV_MIDI_H

#include "audio/mididrv.h"

namespace IMSMidi {
class IMuseChannel_Midi;
class IMuseChannel_MT32;
struct ChannelNode;
} // End of namespace IMSMidi

namespace Scumm {

class IMuseDriver_GMidi : public MidiDriver {
	friend class IMSMidi::IMuseChannel_Midi;
public:
	IMuseDriver_GMidi(MidiDriver::DeviceHandle dev, bool rolandGSMode, bool newSystem);
	virtual ~IMuseDriver_GMidi() override;

	int open() override;
	void close() override;

	// Just pass these through...
	bool isOpen() const override { return _drv ? _drv->isOpen() : false; }
	uint32 property(int prop, uint32 param) override { return _drv ? _drv->property(prop, param) : 0; }
	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) override { if (_drv) _drv->setTimerCallback(timerParam, timerProc); }
	uint32 getBaseTempo() override { return _drv ? _drv->getBaseTempo() : 0; }
	void send(uint32 b) override { if (_drv) _drv->send(b); };
	void sysEx(const byte *msg, uint16 length) override { if (_drv) _drv->sysEx(msg, length); }
	virtual void setPitchBendRange(byte channel, uint range) override { if (_drv) _drv->setPitchBendRange(channel, range); }

	// Channel allocation functions
	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override;

protected:
	IMSMidi::IMuseChannel_Midi *getPart(int number);
	virtual void createChannels();
	virtual void createParts();
	virtual void releaseChannels();

	MidiDriver *_drv;
	const bool _newSystem;
	byte _numChannels;
	byte _numVoices;
	IMSMidi::IMuseChannel_Midi **_imsParts;

private:
	virtual void initDevice();
	void initRolandGSMode();
	virtual void deinitDevice();

	void setNoteFlag(byte chan, byte note) { if (_notesPlaying && chan < 16 && note < 128) _notesPlaying[note] |= (1 << chan); }
	void clearNoteFlag(byte chan, byte note) { if (_notesPlaying && chan < 16 && note < 128) _notesPlaying[note] &= ~(1 << chan); }
	bool queryNoteFlag(byte chan, byte note) const { return (_notesPlaying && chan < 16 && note < 128) ? _notesPlaying[note] & (1 << chan) : false; }
	void setSustainFlag(byte chan, byte note) { if (_notesSustained && chan < 16 && note < 128) _notesSustained[note] |= (1 << chan); }
	void clearSustainFlag(byte chan, byte note) { if (_notesSustained && chan < 16 && note < 128) _notesSustained[note] &= ~(1 << chan); }
	bool querySustainFlag(byte chan, byte note) const { return (_notesSustained && chan < 16 && note < 128) ? _notesSustained[note] & (1 << chan) : false; }

	const bool _gsMode;

	IMSMidi::ChannelNode *_idleChain;
	IMSMidi::ChannelNode *_activeChain;

	uint16 *_notesPlaying;
	uint16 *_notesSustained;
};

class IMuseDriver_MT32 final : public IMuseDriver_GMidi {
	friend class IMSMidi::IMuseChannel_MT32;
public:
	IMuseDriver_MT32(MidiDriver::DeviceHandle dev, bool newSystem);
	~IMuseDriver_MT32() override {}

private:
	void initDevice() override;
	void deinitDevice() override;
	void createChannels() override;
	void createParts() override;
	void releaseChannels() override;

	// Convenience function that allows to send the sysex message with the exact same arguments as they are used in the original drivers.
	void sendMT32Sysex(uint32 addr, const byte *data, uint32 dataSize);

	IMSMidi::ChannelNode *_hwRealChain;

	const byte *_programsMapping;
};

} // End of namespace Scumm

#endif
