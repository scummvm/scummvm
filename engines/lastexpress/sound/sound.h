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

#ifndef LASTEXPRESS_SOUND_H
#define LASTEXPRESS_SOUND_H

#include "lastexpress/shared.h"

#include "common/str.h"

namespace LastExpress {

class LastExpressEngine;
class SoundQueue;

class SoundManager {
public:
	SoundManager(LastExpressEngine *engine);
	~SoundManager();

	// Sound playing
	// the original game uses byte in playSound but unsigned in playSoundWithSubtitles for activateDelay, no idea why
	void playSound(EntityIndex entity, Common::String filename, SoundFlag flag = kSoundVolumeEntityDefault, byte activateDelay = 0);
	bool playSoundWithSubtitles(Common::String filename, uint32 flag, EntityIndex entity, unsigned activateDelay = 0);
	void playSoundEvent(EntityIndex entity, byte action, byte activateDelay = 0);
	void playDialog(EntityIndex entity, EntityIndex entityDialog, SoundFlag flag, byte a4);
	void playSteam(CityIndex index);
	void playFightSound(byte action, byte a4);
	void playLocomotiveSound();
	void playWarningCompartment(EntityIndex entity, ObjectIndex compartment);
	void playAmbientSound(int param);

	// Dialog & Letters
	void readText(int id);
	const char *getDialogName(EntityIndex entity) const;

	// Sound bites
	void excuseMe(EntityIndex entity, EntityIndex entity2 = kEntityPlayer, SoundFlag flag = kVolumeNone);
	void excuseMeCath();
	const char *justCheckingCath() const;
	const char *wrongDoorCath() const;
	const char *justAMinuteCath() const;

	// Flags
	SoundFlag getSoundFlag(EntityIndex index) const;

	// Accessors
	SoundQueue *getQueue() { return _queue; }
	uint32 getAmbientSoundDuration() { return _ambientSoundDuration; }
	bool needToChangeAmbientVolume();
	SoundFlag getChangedAmbientVolume() { return _ambientScheduledVolume; }

	// Setters
	void clearAmbientVolumeChange() { _ambientScheduledVolume = kVolumeNone; }

private:
	LastExpressEngine *_engine;
	SoundQueue *_queue;

	// Compartment warnings by Mertens or Coudert
	uint32 _lastWarning[12];

	// Ambient sound
	int _ambientSoundDuration;
	uint32 _ambientVolumeChangeTimeMS, _ambientVolumeChangeDelayMS;
	SoundFlag _ambientScheduledVolume;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SOUND_H
