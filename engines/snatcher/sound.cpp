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

#include "snatcher/sound_device.h"
#include "snatcher/sound.h"

namespace Snatcher {

SoundEngine::SoundEngine(FIO *fio, Common::Platform platform, int soundOptions) : _dev(nullptr) {
	_dev = SoundDevice::create(fio, platform, soundOptions);
	assert(_dev);
}

SoundEngine::~SoundEngine() {
	delete _dev;
}

bool SoundEngine::init(Audio::Mixer *mixer) {
	return _dev->init(mixer);
}

void SoundEngine::cdaPlay(int track) {
	_dev->cdaPlay(track);
}

void SoundEngine::cdaStop() {
	_dev->cdaStop();
}

bool SoundEngine::cdaIsPlaying() const {
	return _dev->cdaIsPlaying();
}

uint32 SoundEngine::cdaGetTime() const {
	return _dev->cdaGetTime();
}

void SoundEngine::fmSendCommand(int cmd, int arg) {
	_dev->fmSendCommand(cmd, arg);
}

uint8 SoundEngine::fmGetStatus() const {
	return _dev->fmGetStatus();
}

void SoundEngine::pcmSendCommand(int cmd, int arg) {
	_dev->pcmSendCommand(cmd, arg);
}

void SoundEngine::pcmInitSound(int sndId) {
	_dev->pcmInitSound(sndId);
}

uint8 SoundEngine::pcmGetStatus() const {
	return _dev->pcmGetStatus();
}

void SoundEngine::pause(bool toggle) {
	_dev->pause(toggle);
}

void SoundEngine::update() {
	_dev->update();
}

void SoundEngine::setMusicVolume(int vol) {
	_dev->setMusicVolume(vol);
}

void SoundEngine::setSoundEffectVolume(int vol) {
	_dev->setSoundEffectVolume(vol);
}

SoundDevice *SoundDevice::create(FIO *fio, Common::Platform platform, int soundOptions) {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(soundOptions);
	MusicType musicType = MidiDriver::getMusicType(dev);

	if (musicType == MT_INVALID)
		return nullptr;

	// TODO: We still need to create the appropriate sound device, since the game
	// heavily relies on audio sync for its animations. We'll just have to handle
	// it via volume settings.
	// if (musicType == MT_NULL)

	switch (platform) {
	case Common::kPlatformSegaCD:
		return createSegaSoundDevice(fio);
	default:
		break;
	};

	return nullptr;
}

} // End of namespace Snatcher
