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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "dm.h"

namespace DM {

#define k34_D13_soundCount 34 // @ D13_SOUND_COUNT

class SoundData {
public:
	uint32 _byteCount;
	byte* _firstSample;
	uint32 _sampleCount;
	SoundData() : _byteCount(0), _firstSample(nullptr), _sampleCount(0) {}
}; // @ SOUND_DATA

class Sound {
public:
	int16 _graphicIndex;
	byte _period;
	byte _priority;
	byte _loudDistance;
	byte _softDistance;
	Sound(int16 index, byte period, byte priority, byte loudDist, byte softDist) :
		_graphicIndex(index), _period(period), _priority(priority), _loudDistance(loudDist), _softDistance(softDist) {}
}; // @ Sound

class PendingSound {
public:
	uint8 _leftVolume;
	uint8 _rightVolume;
	int16 _soundIndex;
	PendingSound(uint8 leftVolume, uint8 rightVolume, int16 soundIndex) :
		_leftVolume(leftVolume), _rightVolume(rightVolume), _soundIndex(soundIndex) {}
};


class SoundMan {
	DMEngine *_vm;

protected:
	SoundMan(DMEngine* vm);
public:
	virtual ~SoundMan();

	static SoundMan *getSoundMan(DMEngine *vm, const ADGameDescription *gameVersion);

	SoundData _gK24_soundData[k34_D13_soundCount]; // @ K0024_as_SoundData
	Common::Queue<PendingSound> _pendingSounds;

	virtual void f503_loadSounds(); // @ F0503_SOUND_LoadAll
	virtual void f064_SOUND_RequestPlay_CPSD(uint16 P0088_ui_SoundIndex, int16 P0089_i_MapX, int16 P0090_i_MapY, uint16 P0091_ui_Mode); // @ F0064_SOUND_RequestPlay_CPSD
	virtual void f060_SOUND_Play(uint16 P0921_ui_SoundIndex, uint16 P0085_i_Period, uint8 leftVol, uint8 rightVol); // @ F0060_SOUND_Play
	void f65_playPendingSound(); // @ F0065_SOUND_PlayPendingSound_CPSD
	bool f505_soundGetVolume(int16 mapX, int16 mapY, uint8 *leftVolume, uint8 *rightVolume); // @ F0505_SOUND_GetVolume
};

class SoundMan_Atari: public SoundMan {
	friend class SoundMan;

	SoundMan_Atari(DMEngine* vm): SoundMan(vm) {};
public:
	void f503_loadSounds() override {} // @ F0503_SOUND_LoadAll
	void f064_SOUND_RequestPlay_CPSD(uint16 P0088_ui_SoundIndex, int16 P0089_i_MapX, int16 P0090_i_MapY, uint16 P0091_ui_Mode) override {} // @ F0064_SOUND_RequestPlay_CPSD
	void f060_SOUND_Play(uint16 P0921_ui_SoundIndex, uint16 P0085_i_Period, uint8 leftVol, uint8 rightVol) override {} // @ F0060_SOUND_Play
};

}
