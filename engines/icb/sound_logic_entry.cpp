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

#include "engines/icb/sound_logic_entry.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/mission.h"

namespace ICB {

void _sound_logic_entry::Initialise(uint32 nID, bool8 bFull) {
	m_nMegaID = nID;
	m_bHeardSomething = FALSE8;
	m_nLastHashedSoundHeard = NULL_HASH;
	m_bSuspended = FALSE8;

	if (bFull)
		SetHearingSensitivity(SL_DEFAULT_HEARING_SENSITIVITY);

	memset(m_nHashedSoundIDs, 0, SL_MAX_SOUND_REGISTRATIONS * sizeof(uint32));
}

bool8 _sound_logic_entry::HeardThis(const char *pcSoundID) {
	uint32 nHashedSoundID;
	bool8 bSoundWaiting;

	// The script writer calling this function is checking to see which sound event happened, so it makes sense
	// to clear the event.  They could have been given a separate function to do this, but this seems unnecessary.
	bSoundWaiting = m_bHeardSomething;
	m_bHeardSomething = FALSE8;

	// Compute the hash of the sound we are checking for.
	nHashedSoundID = HashString(pcSoundID);

	// See if it is the one that caused the sound event flag to be set.  Note that we only clear the
	// last-heard when this function returns true, otherwise a ladder of if-elses would only catch
	// outstanding events if they were the first in the list being checked for.
	if (bSoundWaiting && (nHashedSoundID == m_nLastHashedSoundHeard)) {
		m_nLastHashedSoundHeard = NULL_HASH;
		return (TRUE8);
	} else {
		return (FALSE8);
	}
}

bool8 _sound_logic_entry::AddSoundRegistration(const char *pcSoundID) {
	uint32 i;
	uint32 nHashedSoundID;

	// Work out the hash value for the sound we want to look for and possibly add.
	nHashedSoundID = HashString(pcSoundID);

	// Look for the sound already being in the list.
	i = 0;
	while ((i < SL_MAX_SOUND_REGISTRATIONS) && (m_nHashedSoundIDs[i] != nHashedSoundID))
		++i;

	// If it is already in the list, do nothing.
	if (i < SL_MAX_SOUND_REGISTRATIONS)
		return (TRUE8);

	// New sound, so look for a blank slot.
	i = 0;
	while ((i < SL_MAX_SOUND_REGISTRATIONS) && (m_nHashedSoundIDs[i] != NULL_HASH))
		++i;

	// If we didn't find a slot, the list is full.
	if (i == SL_MAX_SOUND_REGISTRATIONS)
		return (FALSE8);

	// We found a slot, so set the hash ID for the sound and return success.
	m_nHashedSoundIDs[i] = nHashedSoundID;
	return (TRUE8);
}

void _sound_logic_entry::RemoveSoundRegistration(const char *pcSoundID) {
	uint32 i;
	uint32 nHashedSoundID;

	// Work out the hash value for the sound we want to look for and possibly add.
	nHashedSoundID = HashString(pcSoundID);

	// Look for the sound already being in the list.
	i = 0;
	while ((i < SL_MAX_SOUND_REGISTRATIONS) && (m_nHashedSoundIDs[i] != nHashedSoundID))
		++i;

	// If we found it, remove it.
	if (i < SL_MAX_SOUND_REGISTRATIONS)
		m_nHashedSoundIDs[i] = NULL_HASH;
}

void _sound_logic_entry::SetHearingSensitivity(uint32 nSensitivity) {
	uint8 nReverseSensitivity;

	// Take the opportunity here to turn the value into something that will be more useful
	// when we later come to ask the question Can the mega hear the sound?
	nReverseSensitivity = (uint8)(SL_MAX_HEARING_SENSITIVITY - nSensitivity);

	m_nHearingThreshold = (uint8)(nReverseSensitivity * (SL_MAX_VOLUME / SL_MAX_HEARING_SENSITIVITY));
}

void _sound_logic_entry::SoundReachedMega(uint32 nHashedSoundID, uint32 nVolume) {
	uint32 i;
	uint32 nGunshotHash;

	// If this mega is suspended from receiving sound events then the event does not go through.
	if (m_bSuspended)
		return;

	// If this mega is the current interact object and the sound is a gunshot then the event does not
	// go through because this was stopping the mega recating to the bullet.

	if ((MS->player.Fetch_player_interact_id() == m_nMegaID) && MS->player.Fetch_player_interact_status()) {
		nGunshotHash = HashString("gunshot");

		if (nGunshotHash == nHashedSoundID) {
			Zdebug("Ignoring gunshot sound event for mega %d because it is the current interact object", m_nMegaID);
			return;
		}
	}

	// First check if the mega has an interest in this sound.
	i = 0;
	while ((i < SL_MAX_SOUND_REGISTRATIONS) && (m_nHashedSoundIDs[i] != nHashedSoundID))
		++i;

	// If the mega is interested in this sound then we must look next at hearing sensitivity
	// to see if the mega actually hears the sound.
	if (i < SL_MAX_SOUND_REGISTRATIONS) {
		// If the volume is greater than the threshold calculated for this mega, the mega
		// hears the sound.
		if (nVolume >= m_nHearingThreshold) {
			m_nLastHashedSoundHeard = nHashedSoundID;
			m_bHeardSomething = TRUE8;
		}
	}
}

} // End of namespace ICB
