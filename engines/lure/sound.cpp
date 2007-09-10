/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "lure/sound.h"
#include "lure/game.h"
#include "lure/res.h"
#include "lure/room.h"

DECLARE_SINGLETON(Lure::SoundManager);

namespace Lure {

SoundManager::SoundManager() {
	_descs = Disk::getReference().getEntry(SOUND_DESC_RESOURCE_ID);
	_numDescs = _descs->size() / sizeof(SoundDescResource);

	for (int channelNum = 0; channelNum < NUM_CHANNELS; ++channelNum)
		_channels[channelNum] = 0;
}

void SoundManager::bellsBodge() {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();

	RoomData *roomData = res.getRoom(room.roomNumber());
	if (roomData->areaFlag != res.fieldList().getField(AREA_FLAG)) {
		res.fieldList().setField(AREA_FLAG, roomData->areaFlag);

		switch (roomData->areaFlag) {
		case 0:
			killSound(1);
			break;
		case 1:
			addSound(2);
			killSound(33);
			break;
		case 2:
			setVolume(0, 15);
			// Deliberate fall through
		default:
			killSound(1);
			break;
		}
	}
}

void SoundManager::killSounds() {
	// Stop the player playing all sounds
	musicInterface_KillAll();

	// Clear the active sounds
	_activeSounds.clear();
	for (int channelNum = 0; channelNum < NUM_CHANNELS; ++channelNum)
		_channels[channelNum] = 0;
}

void SoundManager::addSound(uint8 soundIndex, bool tidyFlag) {
	Game &game = Game::getReference();

	if (tidyFlag)
		tidySounds();

	if (game.preloadFlag())
		// Don't add a sound if in room preloading
		return;

	SoundDescResource &rec = soundDescs()[soundIndex];
	int numChannels = (rec.numChannels >> 2) & 3;

	int channelCtr = 0;
	while (channelCtr < NUM_CHANNELS) {
		if (_channels[channelCtr] == 0) {
			bool foundSpace = true;

			int channelCtr2 = 0;
			while (channelCtr2 < numChannels) {
				foundSpace = _channels[channelCtr2] == 0;
				if (!foundSpace) break;
				++channelCtr2;
			}

			if (foundSpace) 
				break;
		}
		
		++channelCtr;
	}

	if (channelCtr == 8) 
		// No channels free
		return;

	// Mark the found channels as in use
	for (int channelCtr2 = 0; channelCtr2 < numChannels; ++channelCtr2)
		_channels[channelCtr + channelCtr2] = 1;

	SoundDescResource *newEntry = new SoundDescResource();
	newEntry->soundNumber = rec.soundNumber;
	newEntry->channel = channelCtr;
	newEntry->numChannels = numChannels;
	newEntry->flags = rec.flags;
	newEntry->volume = rec.volume;
	_activeSounds.push_back(newEntry);

	musicInterface_Play(rec.soundNumber, false, channelCtr);
	setVolume(rec.soundNumber, rec.volume);	
}

void SoundManager::addSound2(uint8 soundIndex) {
	tidySounds();

	if (soundIndex == 6)
		// Chinese torture
		addSound(6);
	else {
		SoundDescResource &descEntry = soundDescs()[soundIndex];
		SoundDescResource *rec = findSound(descEntry.soundNumber);
		if (rec == NULL)
			// Sound isn't active, so go and add it
			addSound(soundIndex, false);
	}
}


void SoundManager::stopSound(uint8 soundIndex) {
	SoundDescResource &rec = soundDescs()[soundIndex];
	musicInterface_Stop(rec.soundNumber & 0x7f);
}

void SoundManager::killSound(uint8 soundNumber) {
	musicInterface_Stop(soundNumber & 0x7f);
}

void SoundManager::setVolume(uint8 soundNumber, uint8 volume) {
	SoundDescResource *entry = findSound(soundNumber);
	if (entry  == NULL) return;

	// Special check is done for Adlib in original game, to ignore any volume changes
}

SoundDescResource *SoundManager::findSound(uint8 soundNumber) {
	ManagedList<SoundDescResource *>::iterator i;

	for (i = _activeSounds.begin(); i != _activeSounds.end(); ++i) {
		SoundDescResource *rec = *i;

		if (rec->soundNumber == soundNumber)
			return rec;
	}

	// Signal that sound wasn't found
	return NULL;
}

void SoundManager::tidySounds() {
	ManagedList<SoundDescResource *>::iterator i = _activeSounds.begin();

	while (i != _activeSounds.end()) {
		SoundDescResource *rec = *i;

		if (musicInterface_CheckPlaying(rec->soundNumber & 0x7f))
			// Still playing, so move to next entry
			++i;
		else {
			// Mark the channels that it used as now being free
			for (int channelCtr = 0; channelCtr < rec->numChannels; ++channelCtr) 
				_channels[rec->channel + channelCtr] = 0;
			
			i = _activeSounds.erase(i);
		}
	}
}

void SoundManager::removeSounds() {
	bellsBodge();

	ManagedList<SoundDescResource *>::iterator i = _activeSounds.begin();

	while (i != _activeSounds.end()) {
		SoundDescResource *rec = *i;

		if ((rec->flags & SF_IN_USE) != 0) 
			musicInterface_Stop(rec->soundNumber);

		++i;
	}
}

void SoundManager::restoreSounds() {

	ManagedList<SoundDescResource *>::iterator i = _activeSounds.begin();

	while (i != _activeSounds.end()) {
		SoundDescResource *rec = *i;

		if ((rec->numChannels != 0) && ((rec->flags & SF_RESTORE) != 0)) {
			for (int channelCtr = 0; channelCtr < rec->numChannels; ++channelCtr)
				_channels[rec->channel + channelCtr] = 1;

			musicInterface_Play(rec->soundNumber, false, rec->channel);
			musicInterface_SetVolume(rec->soundNumber, rec->volume);
		}

		++i;
	}
}


/*------------------------------------------------------------------------*/

// musicInterface_CheckPlaying
// Play the specified sound

void SoundManager::musicInterface_Play(uint8 soundNumber, bool isEffect, uint8 channelNumber) {

}

// musicInterface_Stop
// Stops the specified sound from playing

void SoundManager::musicInterface_Stop(uint8 soundNumber) {
	
}

// musicInterface_CheckPlaying
// Returns true if a sound is still player

bool SoundManager::musicInterface_CheckPlaying(uint8 soundNumber) {
	return true;
}

// musicInterface_SetVolume
// Sets the volume of the specified channel

void SoundManager::musicInterface_SetVolume(uint8 channelNum, uint8 volume) {
	
}

void SoundManager::musicInterface_KillAll() {

}

void SoundManager::musicInterface_ContinuePlaying() {

}

void SoundManager::musicInterface_TrashReverb() {

}

} // end of namespace Lure
