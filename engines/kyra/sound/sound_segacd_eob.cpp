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

#include "kyra/sound/drivers/segacd.h"
#include "kyra/sound/sound_intern.h"
#include "kyra/resource/resource.h"
#include "common/config-manager.h"
#include "backends/audiocd/audiocd.h"

namespace Kyra {

SoundSegaCD_EoB::SoundSegaCD_EoB(KyraEngine_v1 *vm, Audio::Mixer *mixer) : Sound(vm, mixer),
	_vm(vm), _driver(0), _fmData(0), _lastSoundEffect(-1), _ready(false) {
	memset(_pcmOffsets, 0, sizeof(_pcmOffsets));
	memset(_fmOffsets, 0, sizeof(_fmOffsets));
}

SoundSegaCD_EoB::~SoundSegaCD_EoB() {
	delete _driver;
	delete[] _fmData;
}

Sound::kType SoundSegaCD_EoB::getMusicType() const {
	return kSegaCD;
}

bool SoundSegaCD_EoB::init() {
	_driver = new SegaAudioDriver(_mixer);
	g_system->getAudioCDManager()->open();

	loadPCMData();
	loadFMData();

	_ready = true;

	return true;
}

void SoundSegaCD_EoB::playTrack(uint8 track) {
	if (!_ready)
		return;

	if (!_musicEnabled) {
		haltTrack();
		return;
	}

	int loop = track >> 6;
	track &= 0x7F;

	g_system->getAudioCDManager()->play(track - 1, loop - 1, 0, 0);
	g_system->getAudioCDManager()->update();
}

void SoundSegaCD_EoB::haltTrack() {
	if (!_ready)
		return;
	g_system->getAudioCDManager()->stop();
}

void SoundSegaCD_EoB::playSoundEffect(uint16 track, uint8 volume) {
	if (!_sfxEnabled || !_ready)
		return;

	uint8 flags = track >> 8;
	track &= 0xFF;

	if (flags & 0x80) {
		track--;
		assert(track < ARRAYSIZE(_pcmOffsets));
		for (uint8 i = 0; i < 8; ++i)
			_driver->startPCMSound(i, _pcmOffsets[track]);

	} else {
		uint8 snd = (flags & 0x40) ? track : _fmTrackMap[track];
		if (snd == 0 || snd > 135)
			return;

		_driver->startFMSound(&_fmData[_fmOffsets[snd - 1]], volume, (SegaAudioDriver::PrioFlags)flags);
		_lastSoundEffect = track;
	}
}

bool SoundSegaCD_EoB::isPlaying() const {
	return g_system->getAudioCDManager()->isPlaying();
}

void SoundSegaCD_EoB::updateVolumeSettings() {
	if (!_driver || !_ready)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

void SoundSegaCD_EoB::loadPCMData() {
	uint32 dataSize;
	uint8 *data1 = _vm->resource()->fileData("PCM", &dataSize);
	if (!data1)
		error("SoundSegaCD_EoB::loadPCMData: File not found: '%s'", "PCM");

	uint8 *data2 = new uint8[0x100];
	memset(data2, 0xFF, 0x100);
	data2[0] = 0x80;

	for (int i = 0; i < ARRAYSIZE(_pcmOffsets); ++i)
		_pcmOffsets[i] = data1[(i << 2) + 2];

	_driver->loadPCMData(0, data1 + 64, dataSize - 64);
	_driver->loadPCMData(0xFF00, data2, 0x100);

	delete[] data1;
	delete[] data2;
}

void SoundSegaCD_EoB::loadFMData() {
	Common::SeekableReadStreamEndian *in = _vm->resource()->createEndianAwareReadStream("FMSE");
	if (!in)
		error("SoundSegaCD_EoB::loadFMData: File not found: '%s'", "FMSE");

	for (int i = 0; i < ARRAYSIZE(_fmOffsets); ++i)
		_fmOffsets[i] = (in->readUint32() - ARRAYSIZE(_fmOffsets) * 4) & 0xFFFF;

	uint32 dataSize = in->size() - in->pos();
	uint8 *data = new uint8[dataSize];
	in->read(data, dataSize);
	delete[] _fmData;
	_fmData = data;

	delete in;
}

const uint8 SoundSegaCD_EoB::_fmTrackMap[140] = {
	0x00, 0x05, 0x40, 0x01, 0x02, 0x02, 0x06, 0x07, 0x12, 0x0a,
	0x11, 0x1f, 0x1e, 0x12, 0x09, 0x0c, 0x0b, 0x17, 0x21, 0x0d,
	0x00, 0x14, 0x00, 0x16, 0x00, 0x00, 0x26, 0x0f, 0x13, 0x10,
	0x00, 0x00, 0x27, 0x00, 0x1a, 0x28, 0x39, 0x46, 0x33, 0x4a,
	0x3b, 0x48, 0x33, 0x47, 0x38, 0x4d, 0x3e, 0x45, 0x36, 0x41,
	0x3a, 0x49, 0x46, 0x38, 0x44, 0x37, 0x42, 0x34, 0x4b, 0x3c,
	0x41, 0x3b, 0x40, 0x38, 0x47, 0x39, 0x4d, 0x35, 0x4c, 0x3d,
	0x4e, 0x3d, 0x42, 0x43, 0x36, 0x32, 0x00, 0x60, 0x1f, 0x82,
	0x1c, 0x29, 0x00, 0x00, 0x00, 0x65, 0x24, 0x60, 0x62, 0x6d,
	0x00, 0x78, 0x70, 0x74, 0x7e, 0x7d, 0x66, 0x81, 0x6a, 0x67,
	0x80, 0x68, 0x64, 0x6c, 0x77, 0x77, 0x77, 0x61, 0x61, 0x61,
	0x71, 0x79, 0x7f, 0x73, 0x7a, 0x7b, 0x71, 0x7c, 0x6e, 0x0e,
	0x75, 0x76, 0x78, 0x6b, 0x30, 0x2f, 0x03, 0x04, 0x23, 0x2b,
	0x5a, 0x1a, 0x1c, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

} // End of namespace Kyra

#endif // ENABLE_EOB
