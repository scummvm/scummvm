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

#ifndef SCUMM_IMUSE_DRV_MAC_H
#define SCUMM_IMUSE_DRV_MAC_H

#include "audio/mididrv.h"
#include "scumm/players/mac_sound_lowlevel.h"

namespace Audio {
class Mixer;
}

namespace IMSMacintosh {
class IMuseChannel_Macintosh;
class IMSMacSoundSystem;
struct ChanControlNode;
} // End of namespace IMSMacintosh

namespace Scumm {

class IMuseDriver_Macintosh final : public MidiDriver {
	friend class IMSMacintosh::IMuseChannel_Macintosh;
public:
	IMuseDriver_Macintosh(ScummEngine *vm, Audio::Mixer *mixer, byte gameID);
	virtual ~IMuseDriver_Macintosh() override;

	int open() override;
	void close() override;

	bool isOpen() const override { return _isOpen; }
	uint32 property(int prop, uint32 param) override;
	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) override;
	uint32 getBaseTempo() override { return _baseTempo; }
	void send(uint32 b) override { error("%s():: Not implemented", __FUNCTION__); }

	// Channel allocation functions
	MidiChannel *allocateChannel() override;
	MidiChannel *getPercussionChannel() override;

private:
	void createChannels();
	void releaseChannels();

	bool _isOpen;
	uint32 _quality;
	uint32 _musicVolume;
	uint32 _sfxVolume;

	IMSMacintosh::IMSMacSoundSystem *_device;
	IMSMacintosh::IMuseChannel_Macintosh **_imsParts;
	IMSMacintosh::ChanControlNode **_channels;

	byte _numChannels;
	byte _numParts;
	uint32 _baseTempo;
	int8 _version;
};

} // End of namespace Scumm

#endif
