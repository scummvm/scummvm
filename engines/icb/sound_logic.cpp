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

#include "engines/icb/sound_logic.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/sound.h"
#include "engines/icb/floors.h"

#include "common/util.h"

namespace ICB {

void _sound_logic::Initialise() {
	uint32 i;

	m_nNumSubscribers = 0;
	m_nSFXSubtitleTimer = 0;
	m_pcSFXSubtitleText = NULL;
	m_nNumLinkedFloors = 0;

	for (i = 0; i < SL_MAX_CONCURRENT_SOUNDS; ++i) {
		m_pPositions[i].nSoundHash = NULL_HASH;
		m_pPositions[i].nTimer = 0xffffffff;
	}
}

void _sound_logic::DrawSubtitle() const {
	// Safety net check to make sure the string has something in it.
	if (strlen(m_pcSFXSubtitleText) > 0) {
		// Yes we need to display speech text.
		SetTextColour(SL_SUBTITLE_R, SL_SUBTITLE_G, SL_SUBTITLE_B);

		MS->Create_remora_text(SL_SFX_SUBTITLE_X, SL_SFX_SUBTITLE_Y, m_pcSFXSubtitleText, 0, PIN_AT_CENTRE, 0, 0, SL_SUBTITLE_WIDTH);

		MS->Render_speech(MS->text_bloc);
		MS->Kill_remora_text();
	}
}

void _sound_logic::SetSuspendedFlag(uint32 nID, bool8 bSuspend) {
	uint32 nIndex;

	// Look for the mega in the list of subscribers.
	nIndex = FindMegaInList(nID);

	// Running off the list is not an error, since this is called from the main event system, which has
	// no knowledge of whether a mega has registered for a sound.
	if (nIndex < m_nNumSubscribers)
		m_pSubscribers[nIndex].SetSuspendedFlag(bSuspend);
}

void _sound_logic::ClearHeardFlag(uint32 nID) {
	uint32 nIndex;

	// Look for the mega in the list of subscribers.
	nIndex = FindMegaInList(nID);

	// Running off the list is not an error, since this is called from the main event system, which has
	// no knowledge of whether a mega has registered for a sound.
	if (nIndex < m_nNumSubscribers)
		m_pSubscribers[nIndex].ClearHeardFlag();
}

void _sound_logic::Cycle() {
	uint32 i;

	// If a subtitle is being displayed, work the timer for it.
	if (m_nSFXSubtitleTimer >= 1)
		--m_nSFXSubtitleTimer;

	// Update the age timers for the sounds we are keeping the positions of.  These will wrap after about 10
	// years, but Windows itself can't stand up that int32, so we won't worry about it.
	for (i = 0; i < SL_MAX_CONCURRENT_SOUNDS; ++i)
		m_pPositions[i].nTimer++;
}

void _sound_logic::AddSubscription(uint32 nID, const char *pcSoundID) {
	uint32 nIndex;
	bool8 bSuccess;

	// Look for the mega already in the list.
	nIndex = FindMegaInList(nID);

	// If we ran off the list, we are extending the size of the list.
	if (nIndex == m_nNumSubscribers) {
		// Increase list size.
		m_pSubscribers[nIndex].Initialise(nID);
		++m_nNumSubscribers;
	}

	// Safe to add the subscription.
	bSuccess = m_pSubscribers[nIndex].AddSoundRegistration(pcSoundID);

	// Error if the list was already full.
	if (!bSuccess)
		Fatal_error("Unable to register mega %d to listen for [%s] - mega already registered for maximum %d sounds", nID, pcSoundID, SL_MAX_SOUND_REGISTRATIONS);
}

void _sound_logic::RemoveSubscription(uint32 nID, const char *pcSoundID) {
	uint32 nIndex;

	// Look for the mega in the list of subscribers.
	nIndex = FindMegaInList(nID);

	// If we ran off that list, the mega is not currently subscribed to be listening for anything, which
	// makes it an error to be trying to unsubscribe them to listen for a sound.
	if (nIndex == m_nNumSubscribers)
		Fatal_error("Unable to unsubscribe object %d for sound [%s] because object is not listed in the sound logic engine", nID, pcSoundID);

	// We found the object okay, so unsubscribe them for this sound.
	m_pSubscribers[nIndex].RemoveSoundRegistration(pcSoundID);
}

void _sound_logic::RemoveAllSubscriptions(uint32 nID) {
	uint32 nIndex;

	// Look for the mega in the list of subscribers.
	nIndex = FindMegaInList(nID);

	// If we ran off that list, the mega is not currently subscribed to be listening for anything, which
	// makes it an error to be trying to unsubscribe them to listen for a sound.
	if (nIndex == m_nNumSubscribers)
		Fatal_error("Unable to unsubscribe object %d for all sounds because it is not listed in the sound logic engine", nID);

	// We found the object okay, so unsubscribe them for this sound.
	m_pSubscribers[nIndex].Initialise(nID, FALSE8);
}

void _sound_logic::SetHearingSensitivity(uint32 nID, uint32 nSensitivity) {
	uint32 nIndex;

	if (nSensitivity > SL_MAX_HEARING_SENSITIVITY)
		Fatal_error("Attempt to set mega hearing sensitivity at %d out of range %d-%d.", nSensitivity, SL_MIN_HEARING_SENSITIVITY, SL_MAX_HEARING_SENSITIVITY);

	// Look for the mega in the list of subscribers.
	nIndex = FindMegaInList(nID);

	// If we ran off the list, we are extending the size of the list.
	if (nIndex == m_nNumSubscribers) {
		// Increase list size.
		m_pSubscribers[nIndex].Initialise(nID);
		++m_nNumSubscribers;
	}

	// Safe to set hearing sensitivity.
	m_pSubscribers[nIndex].SetHearingSensitivity((uint8)nSensitivity);
}

bool8 _sound_logic::MegaHeardSomething(uint32 nID) {
	uint32 nIndex;

	// Look for the mega in the list of subscribers.
	nIndex = FindMegaInList(nID);

	// If the mega is in the list, return the flag for sound events, otherwise just return false.
	if (nIndex < m_nNumSubscribers)
		return (m_pSubscribers[nIndex].HeardSound());
	else
		return (FALSE8);
}

bool8 _sound_logic::MegaHeardThis(uint32 nID, const char *pcSoundID) {
	uint32 nIndex;

	// Look for the mega in the list of subscribers.
	nIndex = FindMegaInList(nID);

	if (nIndex < m_nNumSubscribers)
		return (m_pSubscribers[nIndex].HeardThis(pcSoundID));
	else
		return (FALSE8);
}

void _sound_logic::NewSound(uint32 nObjectID, int32 nX, int32 nY, int32 nZ, const CSfx *pSFX, uint32 nSoundHash) {
	uint32 i;
	uint32 nMegaID;
	uint32 nVolume;
	uint32 nOldest, nPosition;

	// First we jot down the location of the sound in case script asks for it.  First, look for the
	// sound already being in the list.
	nPosition = 0;
	while ((nPosition < SL_MAX_CONCURRENT_SOUNDS) && (m_pPositions[nPosition].nSoundHash != nSoundHash))
		++nPosition;

	// If it wasn't in the list, look for the oldest entry to age out.
	if (nPosition == SL_MAX_CONCURRENT_SOUNDS) {
		i = 0;
		nOldest = 0;
		nPosition = 0;

		while ((i < SL_MAX_CONCURRENT_SOUNDS)) {
			if (m_pPositions[i].nTimer > nOldest) {
				nOldest = m_pPositions[i].nTimer;
				nPosition = i;
			}

			++i;
		}
	}

	// Don't put up a new SFX subtitle if one is being displayed already.
	if (m_nSFXSubtitleTimer == 0) {
		// Here, we put up a SFX subtitle if one is listed for the sound.
		m_pcSFXSubtitleText = (const char *)global_text->Try_fetch_item_by_hash(nSoundHash);

		if (m_pcSFXSubtitleText)
			m_nSFXSubtitleTimer = Get_reading_time(m_pcSFXSubtitleText);
		else
			m_nSFXSubtitleTimer = 0;
	}

	// We now have the position of where to write the entry.
	m_pPositions[nPosition].nTimer = 0;
	m_pPositions[nPosition].nX = nX;
	m_pPositions[nPosition].nZ = nZ;
	m_pPositions[nPosition].nSoundHash = nSoundHash; // Do this every time 'cos it's faster than checking to see if it needs doing.

	// Loop for each mega we are running sound logic for.
	for (i = 0; i < m_nNumSubscribers; ++i) {
		// Get the mega's ID and logic structure.
		nMegaID = m_pSubscribers[i].GetMegaID();

		// Megas don't need to post sound events to themselves.
		if (nMegaID != nObjectID) {
			// Work out the volume at the mega character's position.
			nVolume = CalculateEffectiveVolume(LOGIC_VOLUME, nMegaID, nX, nY, nZ, pSFX);

			// Mega can't possibly hear sound if volume is zero.
			if (nVolume > 0) {
				// Supply the volume to the mega's entry so it can calculate whether or not the mega
				// actually hears the sound (depends on hearing sensitivity).
				m_pSubscribers[i].SoundReachedMega(nSoundHash, nVolume);
			}
		}
	}
}

int32 _sound_logic::GetSoundX(uint32 nSoundHash) const {
	uint32 nPosition;

	// Look for the sound in the list.
	nPosition = 0;
	while ((nPosition < SL_MAX_CONCURRENT_SOUNDS) && (m_pPositions[nPosition].nSoundHash != nSoundHash))
		++nPosition;

	// Return a very big number if it wasn't found, otherwise return the real number.
	if (nPosition == SL_MAX_CONCURRENT_SOUNDS)
		return (SL_UNDEFINED_COORDINATE);
	else
		return (m_pPositions[nPosition].nX);
}

int32 _sound_logic::GetSoundZ(uint32 nSoundHash) const {
	uint32 nPosition;

	// Look for the sound in the list.
	nPosition = 0;
	while ((nPosition < SL_MAX_CONCURRENT_SOUNDS) && (m_pPositions[nPosition].nSoundHash != nSoundHash))
		++nPosition;

	// Return a very big number if it wasn't found, otherwise return the real number.
	if (nPosition == SL_MAX_CONCURRENT_SOUNDS)
		return (SL_UNDEFINED_COORDINATE);
	else
		return (m_pPositions[nPosition].nZ);
}

bool8 _sound_logic::SoundEventPendingForID(uint32 nID) {
	uint32 i;

	// Loop for each mega we are running sound logic for.
	for (i = 0; i < m_nNumSubscribers; ++i) {
		if (m_pSubscribers[i].GetMegaID() == nID)
			return (m_pSubscribers[i].HeardSound());
	}

	// If we fell off the list, we are not currently running event processing for the object, so
	/// just return false.
	return (FALSE8);
}

void _sound_logic::LinkFloorsForSoundEvents(const char *pcFloor1, const char *pcFloor2) {
	uint32 nFloor1Index, nFloor2Index;

	// Check we have space to make a new link.
	if (m_nNumLinkedFloors == SL_MAX_FLOOR_LINKS) {
		return;
	}

	// Look for first floor.
	nFloor1Index = MS->floor_def->Fetch_floor_number_by_name(pcFloor1);

	if (nFloor1Index == PX_LINKED_DATA_FILE_ERROR) {
		return;
	}

	// Look for second floor.
	nFloor2Index = MS->floor_def->Fetch_floor_number_by_name(pcFloor2);

	if (nFloor2Index == PX_LINKED_DATA_FILE_ERROR) {
		return;
	}

	// Both floors found so link them.
	m_pnLinkedFloors[m_nNumLinkedFloors][0] = nFloor1Index;
	m_pnLinkedFloors[m_nNumLinkedFloors][1] = nFloor2Index;

	// Keep track of how many we've linked.
	++m_nNumLinkedFloors;
}

uint32 _sound_logic::FindMegaInList(uint32 nID) const {
	uint32 i = 0;

	while ((i < m_nNumSubscribers) && (m_pSubscribers[i].GetMegaID() != nID))
		++i;

	return (i);
}

uint8 _sound_logic::CalculateEffectiveVolume(SoundVolumeMode eMode, uint32 nMegaID, int32 nSoundX, int32 nSoundY, int32 nSoundZ, const CSfx *pSFX) const {
	int32 nRawVolume;
	int32 nMegaX, nMegaY, nMegaZ;
	int32 nDeltaX, nDeltaY, nDeltaZ;
	int32 nSqrMicDist, nSqrMinDist, nSqrMaxDist;
	int32 nMaxDelta;
	_logic *pMega;

	// If the Remora is making the sound then it is automatically full volume.
	if (nMegaID == SPECIAL_SOUND)
		return (SL_MAX_VOLUME);

	// If we are working out the actual volume to be used by the speakers, the sound gets cut dead
	// unless it is on the same or a linked floor.
	if ((eMode == ACTUAL_VOLUME) && !SoundAndEarOnSameOrLinkedFloors(nMegaID, (PXreal)nSoundX, (PXreal)nSoundY, (PXreal)nSoundZ)) {
		return (0);
	}

	// We will work out a volume using the same function that the SFX themselves
	// started out using.  This is: if microphone nearer than MIN distance in SFX
	// then volume is 127 if microphone further than MAX distance in SFX then volume
	// is 0 else volume = ( ( MICROPHONE_DIST - MIN ) / ( MAX - MIN ) ) * 127

	// First get the position of the mega hearing the sound.
	pMega = MS->logic_structs[nMegaID];

	// If the object is not a mega, something has broken.
	if (pMega->image_type != VOXEL)
		Fatal_error("Non-mega [%s] is subscribed to sound logic engine - only works for megas", pMega->GetName());

	// Get the coordinates of the mega.
	nMegaX = (int32)pMega->mega->actor_xyz.x;
	nMegaZ = (int32)pMega->mega->actor_xyz.z;
	nMegaY = (int32)pMega->mega->actor_xyz.y;

	// Work out some terms.
	nDeltaX = abs(nMegaX - nSoundX);
	nDeltaY = abs(nMegaY - nSoundY);
	nDeltaZ = abs(nMegaZ - nSoundZ);

	// To approximate, we take just the largest term.
	nMaxDelta = MAX(nDeltaX, MAX(nDeltaY, nDeltaZ));

	// We deal with squared distances to avoid square root.
	nSqrMicDist = (nMaxDelta * nMaxDelta);

	// For debugging, the min and max values for the sound are simply bunged in at default values.
	if (pSFX) {
		nSqrMinDist = pSFX->m_min_distance * pSFX->m_min_distance;
		nSqrMaxDist = pSFX->m_max_distance * pSFX->m_max_distance;
	} else {
		nSqrMinDist = SL_MIN_SOUND * SL_MIN_SOUND;
		nSqrMaxDist = SL_MAX_SOUND * SL_MAX_SOUND;
	}

	// Now we have the terms to work out a first go at the volume.
	if (nSqrMicDist <= nSqrMinDist) {
		nRawVolume = SL_MAX_VOLUME;
	} else if (nSqrMicDist >= nSqrMaxDist) {
		nRawVolume = 0;
	} else {
		// Divide by 16 to keep within integer limits i.e. accurate to +/- ~16cm
		nSqrMicDist = (nSqrMicDist >> 4);
		nSqrMaxDist = (nSqrMaxDist >> 4);
		nSqrMinDist = (nSqrMinDist >> 4);

		nRawVolume = (int32)(((nSqrMaxDist - nSqrMicDist) * SL_MAX_VOLUME) / (nSqrMaxDist - nSqrMinDist));
	}

	// If we are working out the actual volume for the speakers then we already know that the sound
	// and the player are on the same or a linked floor, so we simply return the volume we have calculated.
	if (eMode == ACTUAL_VOLUME) {
		return ((uint8)nRawVolume);
	}

	// If we are working out a volume for use by the event system and the sound and ear are not on the
	// same or linked floor rectangles, we chop the sound by a factor.
	if (!SoundAndEarOnSameOrLinkedFloors(nMegaID, (PXreal)nSoundX, (PXreal)nSoundY, (PXreal)nSoundZ)) {
		// The sound and the ear are on different floors and the floors are not linked so we
		// chop the sound by a factor.
		nRawVolume /= SL_FLOOR_RECT_DROP_FACTOR;
	}

	// Return the volume.
	return ((uint8)nRawVolume);
}

bool8 _sound_logic::SoundAndEarOnSameOrLinkedFloors(uint32 nEarID, PXreal fSoundX, PXreal fSoundY, PXreal fSoundZ) const {
	uint32 i;
	uint32 nEarRect, nEarCamera;
	PXreal fGravitisedY;
	uint32 nSoundRect;
	uint32 nNumLinkedCameras;

	// Get the floor and camera for the player.
	nEarRect = MS->logic_structs[nEarID]->owner_floor_rect;
	nEarCamera = MS->floor_to_camera_index[nEarRect];

	// Now we need to place the origin of the sound on a floor rectangle.
	fGravitisedY = MS->floor_def->Floor_safe_gravitise_y(fSoundY);

	// First look for the sound being on the same floor as the player, adding a bounding box to the player's
	// floor rectangle so that doors always make a sound no matter which floor the marker is on.
	if (MS->floor_def->Point_on_rubber_floor(fSoundX, fSoundZ, fGravitisedY, SL_SAME_FLOOR_BOUNDARY, nEarRect))
		return (TRUE8);

	// Get the floor the sound is on.
	nSoundRect = MS->floor_def->Return_floor_rect(fSoundX, fSoundZ, fGravitisedY, 0);

	// If we failed to find a floor rectangle for the sound, then something has gone wrong.
	if (nSoundRect == PXNULL)
		return (FALSE8);

	// See if the sound and the player share the same camera.
	if (MS->floor_to_camera_index[nSoundRect] == nEarCamera)
		return (TRUE8);

	// See if the sound and ear are on floors linked for sound events.
	if (FloorsLinkedForSounds(nSoundRect, nEarRect))
		return (TRUE8);

	// Check if they are on linked floors.
	nNumLinkedCameras = MS->cam_floor_list[nEarCamera].num_extra_floors;

	for (i = 0; i < nNumLinkedCameras; ++i) {
		// yes, they are on different floors but the floors are linked.
		if (MS->cam_floor_list[nEarCamera].extra_floors[i] == nSoundRect)
			return (TRUE8);
	}

	// The sound and the ear do not share a camera and are not linked.
	return (FALSE8);
}

bool8 _sound_logic::FloorsLinkedForSounds(uint32 nFloor1, uint32 nFloor2) const {
	uint32 i;

	// Loop through listed links.
	for (i = 0; i < m_nNumLinkedFloors; ++i) {
		// Need to check both ways round.
		if (((m_pnLinkedFloors[i][0] == nFloor1) && (m_pnLinkedFloors[i][1] == nFloor2)) || ((m_pnLinkedFloors[i][1] == nFloor1) && (m_pnLinkedFloors[i][0] == nFloor2))) {
			return (TRUE8);
		}
	}

	// Didn't find a link.
	return (FALSE8);
}

} // End of namespace ICB
