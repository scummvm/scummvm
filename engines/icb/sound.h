/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_SOUND_H
#define ICB_SOUND_H

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_clu_api.h"
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

void RegisterSoundOffset(uint32 obj, const cstr offsetName, const cstr sfxName, uint32 sfxHash, const cstr sndID, PXreal xo, PXreal yo, PXreal zo, int isNico, int time,
                         int8 volume_offset);
void RegisterSound(uint32 obj, const cstr sfxName, uint32 sfxHash, const cstr sndID, int8 volume_offset);
void RegisterSoundAbsolute(uint32 obj, const cstr sfxName, uint32 sfxHash, const cstr sndID, PXreal x, PXreal y, PXreal z, int8 volume_offset);
void RegisterSoundTime(uint32 obj, const cstr sfxName, uint32 sfxHash, const cstr sndID, int32 time, int8 volume_offset);
void RegisterSoundSpecial(const cstr sfxName, uint32 sfxHash, const cstr sndID, int32 volume, int32 pan, int8 volume_offset);

// A special function, this plays the sound centrally on the speakers and at the given volume 0-127
void RegisterMenuSound(const cstr sfxName, uint32 sfxHash, int32 volume, int32 pan, int8 volume_offset);

// copies of above with no hash.....!

inline void RegisterSoundOffset(uint32 obj, const cstr offsetName, const cstr sfxName, const cstr sndID, PXreal xo, PXreal yo, PXreal zo, int isNico, int time) {
	RegisterSoundOffset(obj, offsetName, sfxName, NULL_HASH, sndID, xo, yo, zo, isNico, time, (int8)127);
}

inline void RegisterSound(uint32 obj, const cstr sfxName, const cstr sndID) { RegisterSound(obj, sfxName, NULL_HASH, sndID, (int8)127); }

inline void RegisterSoundAbsolute(uint32 obj, const cstr sfxName, const cstr sndID, PXreal x, PXreal y, PXreal z) {
	RegisterSoundAbsolute(obj, sfxName, NULL_HASH, sndID, x, y, z, (int8)127);
}

inline void RegisterSoundTime(uint32 obj, const cstr sfxName, const cstr sndID, int32 time) { RegisterSoundTime(obj, sfxName, NULL_HASH, sndID, time, (int8)127); }

inline void RegisterSoundSpecial(const cstr sfxName, const cstr sndID, int32 volume, int32 pan) { RegisterSoundSpecial(sfxName, NULL_HASH, sndID, volume, pan, (int8)127); }

inline void RegisterMenuSound(const cstr sfxName, int32 volume, int32 pan) { RegisterMenuSound(sfxName, NULL_HASH, volume, pan, (int8)127); }

// end of no hash copies

void RemoveRegisteredSound(uint32 obj, const cstr sndID);

void RemoveAllSoundsWithID(uint32 obj);

void StopAllSoundsNow();

} // End of namespace ICB

#endif
