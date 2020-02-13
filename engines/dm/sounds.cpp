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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "advancedDetector.h"

#include "dm/dm.h"
#include "dm/gfx.h"
#include "dm/timeline.h"
#include "dm/dungeonman.h"
#include "dm/group.h"
#include "dm/sounds.h"

namespace DM {

SoundMan *SoundMan::getSoundMan(DMEngine *vm, const DMADGameDescription *gameVersion) {
	switch (gameVersion->_desc.platform) {
	default:
		warning("Unknown platform, using default Amiga SoundMan");
		// fall through
	case Common::kPlatformAmiga:
		return new SoundMan(vm);
	case Common::kPlatformAtariST:
		return new SoundMan_Atari(vm);
	}
}

void SoundMan::initConstants() {
	Sound sounds[kDMSoundCount] = {
		Sound(533, 112,  11, 3, 6), /* k00_soundMETALLIC_THUD 0 */
		Sound(534, 112,  15, 0, 3), /* k01_soundSWITCH 1 */
		Sound(535, 112,  72, 3, 6), /* k02_soundDOOR_RATTLE 2 */
		Sound(550, 112,  60, 3, 5), /* k03_soundATTACK_PAIN_RAT_HELLHOUND_RED_DRAGON 3 */
		Sound(536, 112,  10, 3, 6), /* k04_soundWOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM 4 */
		Sound(537, 112,  99, 3, 7), /* k05_soundSTRONG_EXPLOSION 5 */
		Sound(539, 112, 110, 3, 6), /* k06_soundSCREAM 6 */
		Sound(551, 112,  55, 3, 5), /* k07_soundATTACK_MUMMY_GHOST_RIVE 7 */
		Sound(540, 112,   2, 3, 6), /* k08_soundSWALLOW 8 */
		Sound(541, 112,  80, 3, 6), /* k09_soundCHAMPION_0_DAMAGED 9 */
		Sound(542, 112,  82, 3, 6), /* k10_soundCHAMPION_1_DAMAGED 10 */
		Sound(543, 112,  84, 3, 6), /* k11_soundCHAMPION_2_DAMAGED 11 */
		Sound(544, 112,  86, 3, 6), /* k12_soundCHAMPION_3_DAMAGED 12 */
		Sound(545, 112,  95, 3, 6), /* k13_soundSPELL 13 */
		Sound(552, 112,  57, 3, 5), /* k14_soundATTACK_SCREAMER_OITU 14 */
		Sound(553, 112,  52, 3, 5), /* k15_soundATTACK_GIANT_SCORPION_SCORPION 15 */
		Sound(546, 112,  40, 2, 4), /* k16_soundCOMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT 16 */
		Sound(547, 112,  70, 1, 4), /* k17_soundBUZZ 17 */
		Sound(549, 138,  75, 3, 6), /* k18_soundPARTY_DAMAGED 18 */
		Sound(554, 112,  50, 3, 5), /* k19_soundATTACK_MAGENTA_WORM_WORM 19 */
		Sound(537, 112,  98, 0, 4), /* k20_soundWEAK_EXPLOSION 20 */
		Sound(555, 112,  96, 2, 4), /* k21_soundATTACK_GIGGLER 21 */
		Sound(563, 138,  24, 0, 4), /* k22_soundMOVE_ANIMATED_ARMOUR_DETH_KNIGHT 22 Atari ST: not present */
		Sound(564, 138,  21, 0, 4), /* k23_soundMOVE_COUATL_GIANT_WASP_MUNCHER 23 Atari ST: not present */
		Sound(565, 138,  23, 0, 4), /* k24_soundMOVE_MUMMY_TROLIN_ANTMAN_STONE_GOLEM_GIGGLER_VEXIRK_DEMON 24 Atari ST: not present */
		Sound(566, 138, 105, 0, 4), /* k25_soundBLOW_HORN 25 Atari ST: not present */
		Sound(567, 138,  27, 0, 4), /* k26_soundMOVE_SCREAMER_ROCK_ROCKPILE_MAGENTA_WORM_WORM_PAIN_RAT_HELLHOUND_RUSTER_GIANT_SCORPION_SCORPION_OITU 26 Atari ST: not present */
		Sound(568, 138,  28, 0, 4), /* k27_soundMOVE_SWAMP_SLIME_SLIME_DEVIL_WATER_ELEMENTAL 27 Atari ST: not present */
		Sound(569, 138, 106, 0, 4), /* k28_soundWAR_CRY 28 Atari ST: not present */
		Sound(570, 138,  56, 0, 4), /* k29_soundATTACK_ROCK_ROCKPILE 29 Atari ST: not present */
		Sound(571, 138,  58, 0, 4), /* k30_soundATTACK_WATER_ELEMENTAL 30 Atari ST: not present */
		Sound(572, 112,  53, 0, 4), /* k31_soundATTACK_COUATL 31 Atari ST: not present */
		Sound(573, 138,  29, 0, 4), /* k32_soundMOVE_RED_DRAGON 32 Atari ST: not present */
		Sound(574, 150,  22, 0, 4)  /* k33_soundMOVE_SKELETON 33 Atari ST: not present */
	};
	for (int i = 0; i < kDMSoundCount; i++)
		_sounds[i] = sounds[i];
}

SoundMan::SoundMan(DMEngine *vm) : _vm(vm) {
	initConstants();
}

SoundMan::~SoundMan() {
	for (uint16 i = 0; i < kDMSoundCount; ++i)
		delete[] _soundData[i]._firstSample;
}

void SoundMan::loadSounds() {
	for (uint16 soundIndex = 0; soundIndex < kDMSoundCount; ++soundIndex) {
		SoundData *soundData = _soundData + soundIndex;

		uint16 graphicIndex = _sounds[soundIndex]._graphicIndex;
		soundData->_byteCount = _vm->_displayMan->getCompressedDataSize(graphicIndex) - 2; // the header is 2 bytes long
		soundData->_firstSample = new byte[soundData->_byteCount];

		Common::MemoryReadStream stream = _vm->_displayMan->getCompressedData(graphicIndex);
		soundData->_sampleCount = stream.readUint16BE();
		stream.read(soundData->_firstSample, soundData->_byteCount);
	}
}

void SoundMan::play(uint16 soundIndex, uint16 period, uint8 leftVolume, uint8 rightVolume) {
	SoundData *sound = &_soundData[soundIndex];
	Audio::AudioStream *stream = Audio::makeRawStream(sound->_firstSample, sound->_byteCount, (72800 / period) * 8, 0, DisposeAfterUse::NO);

	signed char balance = ((int16)rightVolume - (int16)leftVolume) / 2;

	Audio::SoundHandle handle;
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, stream, -1, 127, balance);
}

void SoundMan::playPendingSound() {
	while (!_pendingSounds.empty()) {
		PendingSound pendingSound = _pendingSounds.pop();
		play(pendingSound._soundIndex, _sounds[pendingSound._soundIndex]._period, pendingSound._leftVolume, pendingSound._rightVolume);
	}
}

bool SoundMan::soundGetVolume(int16 mapX, int16 mapY, uint8 *leftVolume, uint8 *rightVolume) {
	static byte distanceToSoundVolume[25][25] = {
		{1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4,  5,  5,  5,  5,  5,  5,  5, 5, 4, 4, 4, 4, 4},
		{1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 5,  6,  6,  6,  6,  5,  5,  5, 5, 5, 5, 4, 4, 4},
		{1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 5,  6,  6,  6,  6,  6,  6,  5, 5, 5, 5, 5, 4, 4},
		{1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 5,  7,  7,  7,  7,  6,  6,  6, 6, 5, 5, 5, 5, 4},
		{1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 5,  8,  8,  7,  7,  7,  7,  6, 6, 6, 5, 5, 5, 4},
		{1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 4, 6,  9,  9,  8,  8,  8,  7,  7, 6, 6, 6, 5, 5, 5},
		{1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 6, 10, 10, 10,  9,  8,  8,  7, 7, 6, 6, 5, 5, 5},
		{1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 7, 12, 12, 11, 10,  9,  9,  8, 7, 7, 6, 6, 5, 5},
		{1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4, 7, 15, 14, 13, 12, 11,  9,  8, 8, 7, 6, 6, 5, 5},
		{1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4, 8, 20, 19, 16, 14, 12, 10,  9, 8, 7, 7, 6, 6, 5},
		{1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4, 8, 29, 26, 21, 16, 13, 11, 10, 8, 7, 7, 6, 6, 5},
		{1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4, 8, 58, 41, 26, 19, 14, 12, 10, 9, 8, 7, 6, 6, 5},
		{1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 6, 64, 58, 29, 20, 15, 12, 10, 9, 8, 7, 6, 6, 5},
		{0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 3, 6, 41, 29, 19, 13, 10,  8,  7, 6, 6, 5, 5, 4, 4},
		{0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 3, 6, 21, 19, 15, 12, 10,  8,  7, 6, 5, 5, 4, 4, 4},
		{0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 3, 6, 14, 13, 12, 10,  9,  7,  7, 6, 5, 5, 4, 4, 4},
		{0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 5, 11, 10, 10,  9,  8,  7,  6, 6, 5, 5, 4, 4, 4},
		{0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 5,  9,  8,  8,  7,  7,  6,  6, 5, 5, 4, 4, 4, 4},
		{0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 5,  7,  7,  7,  7,  6,  6,  5, 5, 5, 4, 4, 4, 4},
		{0, 1, 1, 1, 1, 1, 1, 2, 2, 1, 3, 4,  6,  6,  6,  6,  6,  5,  5, 5, 4, 4, 4, 4, 3},
		{1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4,  6,  6,  5,  5,  5,  5,  5, 4, 4, 4, 4, 3, 3},
		{1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4,  5,  5,  5,  5,  5,  4,  4, 4, 4, 4, 3, 3, 3},
		{1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3,  5,  5,  4,  4,  4,  4,  4, 4, 4, 3, 3, 3, 3},
		{1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3,  4,  4,  4,  4,  4,  4,  4, 4, 3, 3, 3, 3, 3},
		{1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3,  4,  4,  4,  4,  4,  4,  4, 3, 3, 3, 3, 3, 3}};

	int16 lineIndex = 0;
	int16 rightVolumeColumnIndex = 0;

	switch (_vm->_dungeonMan->_partyDir) {
	case kDMDirNorth:
		rightVolumeColumnIndex = mapX - _vm->_dungeonMan->_partyMapX;
		lineIndex = mapY - _vm->_dungeonMan->_partyMapY;
		break;
	case kDMDirEast:
		rightVolumeColumnIndex = mapY - _vm->_dungeonMan->_partyMapY;
		lineIndex = -(mapX - _vm->_dungeonMan->_partyMapX);
		break;
	case kDMDirSouth:
		rightVolumeColumnIndex = -(mapX - _vm->_dungeonMan->_partyMapX);
		lineIndex = -(mapY - _vm->_dungeonMan->_partyMapY);
		break;
	case kDMDirWest:
		rightVolumeColumnIndex = -(mapY - _vm->_dungeonMan->_partyMapY);
		lineIndex = mapX - _vm->_dungeonMan->_partyMapX;
		break;
	default:
		break;
	}

	if ((rightVolumeColumnIndex < -12) || (rightVolumeColumnIndex > 12)) /* Sound is not audible if source is more than 12 squares away from the party */
		return false;

	if ((lineIndex < -12) || (lineIndex > 12)) /* Sound is not audible if source is more than 12 squares away from the party */
		return false;

	int16 leftVolumeColumnIndex = -rightVolumeColumnIndex + 12;
	rightVolumeColumnIndex += 12;
	lineIndex += 12;
	*rightVolume = distanceToSoundVolume[lineIndex][rightVolumeColumnIndex];
	*leftVolume = distanceToSoundVolume[lineIndex][leftVolumeColumnIndex];
	return true;
}

void SoundMan::requestPlay(uint16 soundIndex, int16 mapX, int16 mapY, SoundMode soundMode) {
	if ((soundMode != kDMSoundModePlayImmediately) && (_vm->_dungeonMan->_currMapIndex != _vm->_dungeonMan->_partyMapIndex))
		return;

	Sound *sound = &_sounds[soundIndex];
	if (soundMode == kDMSoundModePlayOneTickLater) { /* Add an event in the timeline to play the sound (mode - 1) ticks later */
		TimelineEvent newEvent;
		newEvent._mapTime = _vm->setMapAndTime(_vm->_dungeonMan->_currMapIndex, _vm->_gameTime + soundMode - 1);
		newEvent._type = kDMEventTypePlaySound;
		newEvent._priority = sound->_priority;
		newEvent._Cu._soundIndex = soundIndex;
		newEvent._Bu._location._mapX = mapX;
		newEvent._Bu._location._mapY = mapY;
		_vm->_timeline->addEventGetEventIndex(&newEvent);
		return;
	}

	uint8 leftVolume, rightVolume;
	if (!soundGetVolume(mapX, mapY, &leftVolume, &rightVolume))
		return;

	if (soundMode == kDMSoundModePlayImmediately) { /* Play the sound immediately */
		play(soundIndex, sound->_period, leftVolume, rightVolume);
		return;
	}
	_pendingSounds.push(PendingSound(leftVolume, rightVolume, soundIndex));
}

}
