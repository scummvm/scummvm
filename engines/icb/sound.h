/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_SOUND_H
#define ICB_SOUND_H

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_clu_api.h"
#include "engines/icb/common/px_sfx_description.h"
#include "engines/icb/sound_lowlevel.h"

namespace ICB {

#define SPECIAL_SOUND (0xffffff)

#define SPEECH_CHANNEL 1
#define MUSIC_CHANNEL 0

// default sounds

#define GUNSHOT_SFX_VAR 0
#define RICOCHET_SFX_VAR 1
#define TINKLE_SFX_VAR 2

#define OPEN_SFX_VAR 0
#define CLOSE_SFX_VAR 1

extern const char *gunSfxVar;
extern const char *defaultGunSfx;
extern const char *gunDesc;

extern const char *ricochetSfxVar;
extern const char *defaultRicochetSfx;
extern const char *ricochetDesc;

extern const char *openSfxVar;
extern const char *defaultOpenSfx;
extern const char *openDesc;

extern const char *closeSfxVar;
extern const char *defaultCloseSfx;
extern const char *closeDesc;

extern const char *addingMediSfxVar;
extern const char *defaultAddingMediSfx;
extern const char *defaultUsingMediSfx;
extern const char *addingMediDesc;

extern const char *addingClipSfxVar;
extern const char *defaultAddingClipSfx;
extern const char *addingClipDesc;

extern const char *activateRemoraSfxVar;
extern const char *activateRemoraSfx;
extern const char *activateRemoraDesc;

extern const char *deactivateRemoraSfxVar;
extern const char *deactivateRemoraSfx;
extern const char *deactivateRemoraDesc;

extern const char *emailSfxVar;
extern const char *defaultEmailSfx;
extern const char *emailDesc;

extern const char *menuUpDownSfx;
extern const char *menuSelectSfx;
extern const char *menuCancelSfx;

extern const char *tinkleSfxVar;
extern const char *defaultTinkleSfx;
extern const char *tinkleDesc;

#define NO_REGISTERED_SOUND 0xffffffff
#define MAX_REGISTERED_SOUNDS 128

class CRegisteredSound {

public:
	uint32 m_objID;   // id of object calling us
	uint32 m_sndHash; // hash of sound id
	int32 m_channel;  // -1 for no channel        needed for the turn everything off hack...

	PXreal m_x;
	PXreal m_y;
	PXreal m_z;

	int32 m_restart_time;
	int32 m_volume;

private:
	int32 m_sfxNumber; // hash value of sfx

	int32 m_velocity;
	int32 m_position; // position*128

	int32 m_current_pitch;
	int32 m_sample_pitch;
	int32 m_rand_pitch_value;
	int32 m_next_random_pos;
	int32 m_pan;

	PXreal m_xoffset;
	PXreal m_yoffset;
	PXreal m_zoffset;

	int8 m_objMoving;
	int8 m_volume_offset; // offset of volume, 127 is normal full volume effect, anything less scales down

public:
	void Wipe();

	CRegisteredSound() : m_objID(NO_REGISTERED_SOUND) { Wipe(); }

	~CRegisteredSound() {}

	inline uint32 GetObjectID() { return m_objID; }

	bool8 IsThisSound(uint32 obj, uint32 sndHash) {

		if ((obj == m_objID) && (sndHash == m_sndHash))
			return TRUE8;
		else
			return FALSE8;
	}

	inline bool8 IsFree() { return (bool8)(m_objID == NO_REGISTERED_SOUND); }
	inline bool8 IsUsed() { return (bool8)(m_objID != NO_REGISTERED_SOUND); }

	int32 GetChannel() { return m_channel; }

	bool8 SetHearable();
	void SetUnhearable();

	void Update10Hz(); // update 10hz (updates position etc)

	void GetPosition();

	void TurnOff();

	// update every game cycle (starts samples if required, updates vol, pitch, pan, stops if end reached, etc...)
	void UpdateGameCycle(int32 newVol, int32 newPan);

	void GetRandom(CSfx *sfx); // update random value
	void Register(const char *sndName, const char *sfxName, uint32 sfxHash, int8 volume);

	void RegisterFromObject(const uint32 objID, const char *sndName, const char *sfxName, uint32 sfxHash, PXreal xo, PXreal yo, PXreal zo, int8 volume);
	void RegisterFromAbsolute(const uint32 objID, const char *sndName, const char *sfxName, uint32 sfxHash, PXreal x, PXreal y, PXreal z, int8 volume);

	void Remove();

	CSfx *GetSfx(); // should be reasonably fast now...

	// volume and pan together (faster than separate)
	void GetVolumeAndPan(int32 &vol, int32 &pan);

private:
	bool8 m_inSession; // if TRUE sfx is in session cluster, otherwise sfx is in mission cluster
	bool8 m_turnOff;   // turning off
	bool8 m_remove;    // if true then when finihsed turning off remove the sound
	uint8 padding1;
};

extern CRegisteredSound *g_registeredSounds[MAX_REGISTERED_SOUNDS];

bool8 SfxExists(uint32 sfxHash);
bool8 SfxExists(const char *sfx);

// full volume, central pan
#define menuSfxUpDown() RegisterMenuSound(menuUpDownSfx, 127, 0)
#define menuSfxSelect() RegisterMenuSound(menuSelectSfx, 127, 0)
#define menuSfxCancel() RegisterMenuSound(menuCancelSfx, 127, 0)

int32 GetSpeechVolume();
int32 GetSfxVolume();
int32 GetMusicVolume();
void SetSpeechVolume(int32 v);
void SetSfxVolume(int32 v);
void SetMusicVolume(int32 v);

// get current sound level in location (from camera)
// is defined as the square of the effects totaled as a percentage of 128*128*2
int32 GetCurrentSoundLevel();

// update
void UpdateRemoraScannerSound();

// called every game cycle
void UpdateHearableSounds();

// called every 10hz
void UpdateSounds10Hz();

// pause / unpause sound update...
void PauseSounds();
void UnpauseSounds();

// turn system on or off

void TurnOffSound();
void TurnOnSound();

// start / stop

int32 GetAssignedSounds();

// these functions have both sfxHash and volume_offset which most calls will not use.... they will use the inlines below...

void RegisterSoundOffset(uint32 obj, const char *offsetName, const char *sfxName, uint32 sfxHash, const char *sndID, PXreal xo, PXreal yo, PXreal zo, int32 isNico, int32 time,
						 int8 volume_offset);
void RegisterSound(uint32 obj, const char *sfxName, uint32 sfxHash, const char *sndID, int8 volume_offset);
void RegisterSoundAbsolute(uint32 obj, const char *sfxName, uint32 sfxHash, const char *sndID, PXreal x, PXreal y, PXreal z, int8 volume_offset);
void RegisterSoundTime(uint32 obj, const char *sfxName, uint32 sfxHash, const char *sndID, int32 time, int8 volume_offset);
void RegisterSoundSpecial(const char *sfxName, uint32 sfxHash, const char *sndID, int32 volume, int32 pan, int8 volume_offset);

// A special function, this plays the sound centrally on the speakers and at the given volume 0-127
void RegisterMenuSound(const char *sfxName, uint32 sfxHash, int32 volume, int32 pan, int8 volume_offset);

// copies of above with no hash.....!

inline void RegisterSoundOffset(uint32 obj, const char *offsetName, const char *sfxName, const char *sndID, PXreal xo, PXreal yo, PXreal zo, int32 isNico, int32 time) {
	RegisterSoundOffset(obj, offsetName, sfxName, NULL_HASH, sndID, xo, yo, zo, isNico, time, (int8)127);
}

inline void RegisterSound(uint32 obj, const char *sfxName, const char *sndID) { RegisterSound(obj, sfxName, NULL_HASH, sndID, (int8)127); }

inline void RegisterSoundAbsolute(uint32 obj, const char *sfxName, const char *sndID, PXreal x, PXreal y, PXreal z) {
	RegisterSoundAbsolute(obj, sfxName, NULL_HASH, sndID, x, y, z, (int8)127);
}

inline void RegisterSoundTime(uint32 obj, const char *sfxName, const char *sndID, int32 time) { RegisterSoundTime(obj, sfxName, NULL_HASH, sndID, time, (int8)127); }

inline void RegisterSoundSpecial(const char *sfxName, const char *sndID, int32 volume, int32 pan) { RegisterSoundSpecial(sfxName, NULL_HASH, sndID, volume, pan, (int8)127); }

inline void RegisterMenuSound(const char *sfxName, int32 volume, int32 pan) { RegisterMenuSound(sfxName, NULL_HASH, volume, pan, (int8)127); }

// end of no hash copies

void RemoveRegisteredSound(uint32 obj, const char *sndID);

void RemoveAllSoundsWithID(uint32 obj);

void StopAllSoundsNow();

} // End of namespace ICB

#endif
