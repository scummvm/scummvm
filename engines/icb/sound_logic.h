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

#ifndef ICB_SOUND_LOGIC_H_INCLUDED
#define ICB_SOUND_LOGIC_H_INCLUDED

#include "engines/icb/p4.h"
#include "engines/icb/debug.h"
#include "engines/icb/session.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/sound_logic_entry.h"
#include "engines/icb/common/px_sfx_description.h"
#include "engines/icb/mission.h"

namespace ICB {

// This is the maximum registrations of megas interested in sounds we can have.
#define SL_MAX_SUBSCRIBERS MAX_voxel_list

// This defines the maximum number of sounds any one mega can be registered for.
#define SL_MAX_SOUND_REGISTRATIONS 10

// The sound logic engine maintains the x,z position of this many sounds, which is the maximum number
// that will be audible at any one time.
#define SL_MAX_CONCURRENT_SOUNDS 24

// The boundary around a floor (cm) which is used when deciding if a sound is on a floor.
#define SL_SAME_FLOOR_BOUNDARY 40

// Some default values for the size of a sound (used in development only).
#define SL_MIN_SOUND 1000  // 10 metres
#define SL_MAX_SOUND 15000 // 150 metres

// This defines how much the sound drops when travelling across unlinked floor rectangles.
#define SL_FLOOR_RECT_DROP_FACTOR 2

// These define where the SFX subtitle appears.
#define SL_SFX_SUBTITLE_X (SCREEN_WIDTH / 2)
#define SL_SFX_SUBTITLE_Y (SCREEN_DEPTH - 20)
#define SL_SUBTITLE_WIDTH 300

#define SL_SUBTITLE_R 255
#define SL_SUBTITLE_G 255
#define SL_SUBTITLE_B 255

// This value gets returned if a coordinate cannot be found for a requested sound.
#define SL_UNDEFINED_COORDINATE (1000000)

// Maximum number of floors that can be linked for sound events.
#define SL_MAX_FLOOR_LINKS 10

// struct _sound_position
// This holds the x,z position of happening sounds (not half, pop pickers).
struct _sound_position {
	uint32 nSoundHash;
	int32 nX, nZ;
	uint32 nTimer;
};

// This class is used to define a single global object, which makes the decisions about which megas
// heard what sounds and posts events in the event manager when required.
class _sound_logic {
public:
	// Default constructor and destructor.
	inline _sound_logic();
	~_sound_logic() { ; }

	// Call this to initialise the sound logic engine at the start of each session.
	void Initialise();

	// This function is called every logic cycle to clean out expired sound events.
	void Cycle();

	// This returns true if there is a sound event pending for the supplied ID, false if not.
	bool8 SoundEventPendingForID(uint32 nID);

	// This clears any outstanding sound event for an object.
	void ClearHeardFlag(uint32 nID);

	// This function is called by the real sound engine once for each sound currently happening.
	inline uint32 ProcessSound(int32 fX, int32 fY, int32 fZ, const CSfx *pSFX) const;

	// This gets called whenever a new SFX is started.
	void NewSound(uint32 nObjectID, int32 nX, int32 nY, int32 nZ, const CSfx *pSFX, uint32 nSoundHash);

	// These are used by mission.cpp to tell when to draw a SFX subtitle.
	bool8 SubtitleActive() const { return ((m_nSFXSubtitleTimer == 0) ? FALSE8 : TRUE8); }
	void DrawSubtitle() const;

	// This function suspends/unsuspends sound event processing for a mega.
	void SetSuspendedFlag(uint32 nID, bool8 bSuspend);

	// These functions have direct script counterparts.
	void AddSubscription(uint32 nID, const char *pcSoundID);
	void RemoveSubscription(uint32 nID, const char *pcSoundID);
	void RemoveAllSubscriptions(uint32 nID);
	void SetHearingSensitivity(uint32 nID, uint32 nSensitivity);
	bool8 MegaHeardSomething(uint32 nID);
	bool8 MegaHeardThis(uint32 nID, const char *pcSoundID);
	int32 GetSoundX(uint32 nSoundHash) const;
	int32 GetSoundZ(uint32 nSoundHash) const;
	void LinkFloorsForSoundEvents(const char *pcFloor1, const char *pcFloor2);

private:
	enum SoundVolumeMode { ACTUAL_VOLUME = 0, LOGIC_VOLUME };

	uint32 m_nNumSubscribers;                               // Number of current subscriptions.
	uint32 m_nSFXSubtitleTimer;                             // Timer for displaying SFX subtitles.
	const char *m_pcSFXSubtitleText;                        // Pointer to SFX subtitle text.
	_sound_logic_entry m_pSubscribers[SL_MAX_SUBSCRIBERS];  // Current registrations of interest in certain sounds.
	_sound_position m_pPositions[SL_MAX_CONCURRENT_SOUNDS]; // A list of audible sounds and their positions.
	uint32 m_pnLinkedFloors[SL_MAX_FLOOR_LINKS][2];         // A list of floors linked for sound events.
	uint32 m_nNumLinkedFloors;                              // Number of items in previous array.

	_sound_logic(const _sound_logic &) { ; }
	void operator=(const _sound_logic &) { ; }

	uint32 FindMegaInList(uint32 nID) const;

	uint8 CalculateEffectiveVolume(SoundVolumeMode eMode, uint32 nMegaID, int32 nSoundX, int32 nSoundY, int32 nSoundZ, const CSfx *pSFX) const;

	bool8 SoundAndEarOnSameOrLinkedFloors(uint32 nEarID, PXreal fSoundX, PXreal fSoundY, PXreal fSoundZ) const;

	bool8 FloorsLinkedForSounds(uint32 nFloor1, uint32 nFloor2) const;
};

inline _sound_logic::_sound_logic() { Initialise(); }

inline uint32 _sound_logic::ProcessSound(int32 nX, int32 nY, int32 nZ, const CSfx *pSFX) const {
	// Calculate volume at player's hearing position for the sound engine.
	int watch = MS->player.Fetch_player_id();

	if (g_mission->camera_follow_id_overide) {
		watch = g_mission->camera_follow_id_overide;
	}

	return (CalculateEffectiveVolume(ACTUAL_VOLUME, watch, nX, nY, nZ, pSFX));
}

extern _sound_logic *g_oSoundLogicEngine;

} // End of namespace ICB

#endif // #if !defined( SOUND_LOGIC_H_INCLUDED )
