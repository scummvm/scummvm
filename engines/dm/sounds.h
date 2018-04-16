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

#ifndef DM_SOUND_H
#define DM_SOUND_H

#include "dm/dm.h"

namespace DM {

enum SoundMode {
	kDMSoundModeDoNotPlaySound = -1, // @ CM1_MODE_DO_NOT_PLAY_SOUND
	kDMSoundModePlayImmediately = 0, // @ C00_MODE_PLAY_IMMEDIATELY
	kDMSoundModePlayIfPrioritized = 1, // @ C01_MODE_PLAY_IF_PRIORITIZED
	kDMSoundModePlayOneTickLater = 2 // @ C02_MODE_PLAY_ONE_TICK_LATER
};

enum SoundIndex {
	kDMSoundIndexMetallicThud = 0, // @ C00_SOUND_METALLIC_THUD
	kDMSoundIndexSwitch = 1, // @ C01_SOUND_SWITCH
	kDMSoundIndexDoorRattle = 2, // @ C02_SOUND_DOOR_RATTLE
	kDMSoundIndexAttackPainRatHellHoundRedDragon = 3, // @ C03_SOUND_ATTACK_PAIN_RAT_HELLHOUND_RED_DRAGON
	kDMSoundIndexWoodenThudAttackTrolinAntmanStoneGolem = 4, // @ C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM
	kDMSoundIndexStrongExplosion = 5, // @ C05_SOUND_STRONG_EXPLOSION
	kDMSoundIndexScream = 6, // @ C06_SOUND_SCREAM
	kDMSoundIndexAttackMummyGhostRive = 7, // @ C07_SOUND_ATTACK_MUMMY_GHOST_RIVE
	kDMSoundIndexSwallow = 8, // @ C08_SOUND_SWALLOW
	kDMSoundIndexChampion0Damaged = 9, // @ C09_SOUND_CHAMPION_0_DAMAGED
	kDMSoundIndexChampion1Damaged = 10, // @ C10_SOUND_CHAMPION_1_DAMAGED
	kDMSoundIndexChampion2Damaged = 11, // @ C11_SOUND_CHAMPION_2_DAMAGED
	kDMSoundIndexChampion3Damaged = 12, // @ C12_SOUND_CHAMPION_3_DAMAGED
	kDMSoundIndexSpell = 13, // @ C13_SOUND_SPELL
	kDMSoundIndexAttackScreamerOitu = 14, // @ C14_SOUND_ATTACK_SCREAMER_OITU
	kDMSoundIndexAttackGiantScorpion = 15, // @ C15_SOUND_ATTACK_GIANT_SCORPION_SCORPION
	kDMSoundIndexAttackSkelettonAnimatedArmorDethKnight = 16, // @ C16_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT
	kDMSoundIndexBuzz = 17, // @ C17_SOUND_BUZZ
	kDMSoundIndexPartyDamaged = 18, // @ C18_SOUND_PARTY_DAMAGED
	kDMSoundIndexAttackMagentaWorm = 19, // @ C19_SOUND_ATTACK_MAGENTA_WORM_WORM
	kDMSoundIndexWeakExplosion = 20, // @ C20_SOUND_WEAK_EXPLOSION
	kDMSoundIndexAttackGiggler = 21, // @ C21_SOUND_ATTACK_GIGGLER
	kDMSoundIndexMoveAnimatedArmorDethKnight = 22, // @ C22_SOUND_MOVE_ANIMATED_ARMOUR_DETH_KNIGHT
	kDMSoundIndexMoveCouatlGiantWaspMuncher = 23, // @ C23_SOUND_MOVE_COUATL_GIANT_WASP_MUNCHER
	kDMSoundIndexMoveMummyTrolinAntmanStoneGolemGiggleVexirkDemon = 24, // @ C24_SOUND_MOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON
	kDMSoundIndexBlowHorn = 25, // @ C25_SOUND_BLOW_HORN
	kDMSoundIndexMoveScreamerRocksWormPainRatHellHoundRusterScorpionsOitu = 26, // @ C26_SOUND_MOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU
	kDMSoundIndexMoveSlimesDevilWaterElemental = 27, // @ C27_SOUND_MOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL
	kDMSoundIndexWarCry = 28, // @ C28_SOUND_WAR_CRY
	kDMSoundIndexAttackRocks = 29, // @ C29_SOUND_ATTACK_ROCK_ROCKPILE
	kDMSoundIndexAttackWaterElemental = 30, // @ C30_SOUND_ATTACK_WATER_ELEMENTAL
	kDMSoundIndexAttackCouatl = 31, // @ C31_SOUND_ATTACK_COUATL
	kDMSoundIndexMoveRedDragon = 32, // @ C32_SOUND_MOVE_RED_DRAGON
	kDMSoundIndexMoveSkeletton = 33 // @ C33_SOUND_MOVE_SKELETON
};

#define kDMSoundCount 34 // @ D13_SOUND_COUNT

class SoundData {
public:
	uint32 _byteCount;
	byte *_firstSample;
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
	Sound() : _graphicIndex(0), _period(0), _priority(0), _loudDistance(0), _softDistance(0) {}
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
	SoundMan(DMEngine *vm);
public:
	virtual ~SoundMan();

	static SoundMan *getSoundMan(DMEngine *vm, const DMADGameDescription *gameVersion);

	SoundData _soundData[kDMSoundCount]; // @ K0024_as_SoundData
	Common::Queue<PendingSound> _pendingSounds;

	virtual void loadSounds(); // @ F0503_SOUND_LoadAll
	virtual void requestPlay(uint16 soundIndex, int16 mapX, int16 mapY, SoundMode soundMode); // @ F0064_SOUND_RequestPlay_CPSD
	virtual void play(uint16 soundIndex, uint16 period, uint8 leftVolume, uint8 rightVolume); // @ F0060_SOUND_Play
	void playPendingSound(); // @ F0065_SOUND_PlayPendingSound_CPSD
	bool soundGetVolume(int16 mapX, int16 mapY, uint8 *leftVolume, uint8 *rightVolume); // @ F0505_SOUND_GetVolume

	Sound _sounds[kDMSoundCount];
	void initConstants();
};

class SoundMan_Atari: public SoundMan {
	friend class SoundMan;

	SoundMan_Atari(DMEngine *vm): SoundMan(vm) {};
public:
	void loadSounds() override {} // @ F0503_SOUND_LoadAll
	void requestPlay(uint16 soundIndex, int16 mapX, int16 mapY, SoundMode soundMode) override {} // @ F0064_SOUND_RequestPlay_CPSD
	void play(uint16 soundIndex, uint16 period, uint8 leftVolume, uint8 rightVolume) override {} // @ F0060_SOUND_Play
};

}

#endif
