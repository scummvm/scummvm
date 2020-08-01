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

#ifndef KYRA_SOUND_ADLIB_H
#define KYRA_SOUND_ADLIB_H

#include "kyra/sound/sound.h"

#include "common/mutex.h"

namespace Kyra {
class PCSoundDriver;

/**
 * AdLib/PC Speaker (early version) implementation of the
 * sound output device.
 *
 * It uses a special sound file format special to EoB I, II,
 * Dune II, Kyrandia 1 and 2 and LoL. EoB I has a slightly
 * different (oldest) file format, EoB II, Dune II and
 * Kyrandia 1 have the exact same format, Kyrandia 2  and
 * LoL have a slightly different format.
 *
 * For PC Speaker this is a little different. Only the EoB
 * games use the old driver with this data file format. The
 * newer games use a MIDI-like driver (see pcspeaker_v2.cpp).
 *
 * See AdLibDriver / PCSpeakerDriver for more information.
 * @see AdLibDriver
 */
class SoundPC_v1 : public Sound {
public:
	SoundPC_v1(KyraEngine_v1 *vm, Audio::Mixer *mixer, kType type);
	~SoundPC_v1() override;

	kType getMusicType() const override { return _type; }

	bool init() override;
	void process() override;

	void updateVolumeSettings() override;

	void initAudioResourceInfo(int set, void *info) override;
	void selectAudioResourceSet(int set) override;
	bool hasSoundFile(uint file) const override;
	void loadSoundFile(uint file) override;
	void loadSoundFile(Common::String file) override;

	void playTrack(uint8 track) override;
	void haltTrack() override;
	bool isPlaying() const override;

	void playSoundEffect(uint16 track, uint8 volume = 0xFF) override;

	void beginFadeOut() override;

	int checkTrigger() override;
	void resetTrigger() override;
private:
	void internalLoadFile(Common::String file);

	void play(uint8 track, uint8 volume);

	const SoundResourceInfo_PC *res() const {return _resInfo[_currentResourceSet]; }
	SoundResourceInfo_PC *_resInfo[3];
	int _currentResourceSet;

	PCSoundDriver *_driver;

	int _version;
	kType _type;
	uint8 _trackEntries[500];
	uint8 *_soundDataPtr;
	int _sfxPlayingSound;

	Common::String _soundFileLoaded;

	int _numSoundTriggers;
	const int *_soundTriggers;

	static const int _kyra1NumSoundTriggers;
	static const int _kyra1SoundTriggers[];
};

} // End of namespace Kyra

#endif
