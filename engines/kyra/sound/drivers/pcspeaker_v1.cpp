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

#ifdef ENABLE_EOB

#include "kyra/sound/drivers/pc_base.h"
#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"

namespace Kyra {

class PCSpeakerDriver : public PCSoundDriver {
public:
	PCSpeakerDriver(Audio::Mixer *mixer);
	virtual ~PCSpeakerDriver();

	virtual void initDriver() override;
	virtual void setSoundData(uint8 *data, uint32 size) override;
	virtual void queueTrack(int track, int volume) override;
	virtual bool isChannelPlaying(int channel) const override;
	virtual void stopAllChannels() override;
	virtual int getSoundTrigger() const override { return _soundTrigger; }
	virtual void resetSoundTrigger() override { _soundTrigger = 0; }

	virtual void callback() override;

	virtual void setMusicVolume(uint8 volume) override;
	virtual void setSfxVolume(uint8 volume) override;

private:
	int _soundTrigger;
};

PCSpeakerDriver::PCSpeakerDriver(Audio::Mixer *mixer) : PCSoundDriver() {

}

PCSpeakerDriver::~PCSpeakerDriver() {

}

void PCSpeakerDriver::initDriver() {

}

void PCSpeakerDriver::setSoundData(uint8 *data, uint32 size) {

}

void PCSpeakerDriver::queueTrack(int track, int volume) {

}

bool PCSpeakerDriver::isChannelPlaying(int channel) const {
	return true;
}

void PCSpeakerDriver::stopAllChannels() {

}

void PCSpeakerDriver::callback() {

}

void PCSpeakerDriver::setMusicVolume(uint8 volume) {

}

void PCSpeakerDriver::setSfxVolume(uint8 volume) {

}

PCSoundDriver *PCSoundDriver::createPCSpk(Audio::Mixer *mixer) {
	return new PCSpeakerDriver(mixer);
}

} // End of namespace Kyra

#endif
