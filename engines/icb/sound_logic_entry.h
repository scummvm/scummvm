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

#ifndef ICB_SOUND_LOGIC_ENTRY_H_INCLUDED
#define ICB_SOUND_LOGIC_ENTRY_H_INCLUDED

#include "engines/icb/p4.h"
#include "engines/icb/debug.h"
#include "engines/icb/common/px_clu_api.h"

namespace ICB {

// This defines the maximum number of sounds any one mega can be registered for.
#define SL_MAX_SOUND_REGISTRATIONS 10

// This defines a range on the hearing sensitivity a mega can have ( MAX = normal hearing, MIN = deaf ).
#define SL_MIN_HEARING_SENSITIVITY 0
#define SL_MAX_HEARING_SENSITIVITY 9

// This is the volume range over which the real sound engine works over (0 to this figure).
#define SL_MAX_VOLUME 127

// This defines the default.
#define SL_DEFAULT_HEARING_SENSITIVITY 5

// Holds the current sounds that a single mega character is interested in in the sound logic engine.
class _sound_logic_entry {
public:
	// Default constructor.
	inline _sound_logic_entry();

	// This sets up a new entry for a mega.
	void Initialise(uint32 nID, bool8 bFull = TRUE8);

	// Gets and sets.
	uint32 GetMegaID() const { return (m_nMegaID); }
	inline bool8 HeardSound();
	void SetHearingSensitivity(uint32 nSensitivity);
	bool8 HeardThis(const char *pcSoundID);
	void SetSuspendedFlag(bool8 bSuspend) {
		m_bSuspended = bSuspend;
		m_bHeardSomething = FALSE8;
	}

	// This allows the heard-something event flag to be cleared.
	void ClearHeardFlag() { m_bHeardSomething = FALSE8; }

	// This adds interest in a certain sound for a mega character.
	bool8 AddSoundRegistration(const char *pcSoundID);

	// Removes one.
	void RemoveSoundRegistration(const char *pcSoundID);

	// Call this when a sound has reached a mega.
	void SoundReachedMega(uint32 nHashedSoundID, uint32 nVolume);

private:
	uint32 m_nMegaID;                                     // ID of the mega character.
	uint32 m_nHashedSoundIDs[SL_MAX_SOUND_REGISTRATIONS]; // Sounds this mega is interested in.
	uint32 m_nLastHashedSoundHeard;                       // Hash of last sound to be heard.
	bool8 m_bHeardSomething;                              // Gets set to true when there is a sound event outstanding.
	uint8 m_nHearingThreshold;                            // The hearing sensitivity of the mega.
	bool8 m_bSuspended;                                   // Set true when mega is suspended from this service.
	uint8 m_nPad2;

	// Block use of default '='.
	_sound_logic_entry(const _sound_logic_entry &) { ; }
	void operator=(const _sound_logic_entry &) { ; }
};

inline _sound_logic_entry::_sound_logic_entry() { memset(m_nHashedSoundIDs, 0, SL_MAX_SOUND_REGISTRATIONS * sizeof(uint32)); m_nPad2 = 0; }

inline bool8 _sound_logic_entry::HeardSound() { return (m_bHeardSomething); }

} // End of namespace ICB

#endif // #if !defined(SOUND_LOGIC_ENTRY_H_INCLUDED)
