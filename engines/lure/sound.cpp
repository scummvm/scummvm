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

#include "lure/sound.h"
#include "lure/game.h"
#include "lure/lure.h"
#include "lure/memory.h"
#include "lure/res.h"
#include "lure/room.h"
#include "lure/surface.h"

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/endian.h"
#include "audio/midiparser.h"

namespace Common {
DECLARE_SINGLETON(Lure::SoundManager);
}

namespace Lure {

//#define SOUND_CROP_CHANNELS

SoundManager::SoundManager() {
	Disk &disk = Disk::getReference();

	int index;
	_descs = disk.getEntry(SOUND_DESC_RESOURCE_ID);
	_numDescs = _descs->size() / sizeof(SoundDescResource);
	_soundData = NULL;
	_paused = false;

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
	_isRoland = MidiDriver::getMusicType(dev) != MT_ADLIB;
	_nativeMT32 = ((MidiDriver::getMusicType(dev) == MT_MT32) || ConfMan.getBool("native_mt32"));

	Common::fill(_channelsInUse, _channelsInUse + NUM_CHANNELS, false);
	Common::fill(_sourcesInUse, _sourcesInUse + LURE_MAX_SOURCES, false);

	if (_isRoland) {
		_driver = _mt32Driver = new MidiDriver_MT32GM(MT_MT32);
	} else {
		_driver = MidiDriver::createMidi(dev);
	}
	int statusCode = _driver->open();
	if (statusCode)
		error("Sound driver returned error code %d", statusCode);

	if (!_isRoland) {
		for (index = 0; index < NUM_CHANNELS; ++index) {
			_channelsInner[index].midiChannel = _driver->allocateChannel();
			/* 90 is power on default for midi compliant devices */
			_channelsInner[index].volume = 90;
		}
	}

	syncSounds();
}

SoundManager::~SoundManager() {
	if (_driver)
		_driver->setTimerCallback(this, NULL);

	removeSounds();
	_activeSounds.clear();

	_soundMutex.lock();
	_playingSounds.clear();
	_soundMutex.unlock();

	delete _descs;
	delete _soundData;

	if (_driver) {
		_driver->stopAllNotes();
		_driver->close();
		delete _driver;
		_driver = NULL;
	}
}

void SoundManager::saveToStream(Common::WriteStream *stream) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::saveToStream");
	SoundListIterator i;

	for (i = _activeSounds.begin(); i != _activeSounds.end(); ++i) {
		stream->writeByte((*i)->soundNumber);
	}
	stream->writeByte(0xff);
}

void SoundManager::loadFromStream(Common::ReadStream *stream) {
	// Stop any existing sounds playing
	killSounds();

	// Load any playing sounds
	uint8 soundNumber;
	while ((soundNumber = stream->readByte()) != 0xff) {
		uint8 soundIndex = descIndexOf(soundNumber);
		if (soundIndex != 0xff) {
			// Make sure that the sound is allowed to be restored
			SoundDescResource &rec = soundDescs()[soundIndex];
			if ((rec.flags & SF_RESTORE) != 0)
				// Requeue the sound for playing
				addSound(soundIndex, false);
		}
	}
}


void SoundManager::loadSection(uint16 sectionId) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::loadSection = %xh", sectionId);
	killSounds();

	if (_soundData) {
		delete _soundData;
		_driver->setTimerCallback(this, NULL);
	}

	_soundData = Disk::getReference().getEntry(sectionId);
	_soundsTotal = *_soundData->data();

	_driver->setTimerCallback(this, &onTimer);
}

bool SoundManager::initCustomTimbres(bool canAbort) {
	if (!_isRoland || !_nativeMT32 || _mt32Driver == NULL)
		return false;

	if (!_soundData)
		error("SoundManager::initCustomTimbres - sound section has not been specified");

	// Locate timbre data
	uint32 headerSize = READ_LE_UINT32(_soundData->data() + 2); // Skip past the number of sounds
	uint16 timbreDataHeaderOffset = _soundsTotal * 4 + 2;
	if (timbreDataHeaderOffset + 6u > headerSize) {
		warning("SoundManager::initCustomTimbres - could not find timbre data header");
		return false;
	}

	uint32 timbreDataOffset = READ_LE_UINT32(_soundData->data() + timbreDataHeaderOffset + 2); // Skip past end of header mark
	if (timbreDataOffset + 17259 > _soundData->size()) {
		warning("SoundManager::initCustomTimbres - timbre data smaller than expected");
		return false;
	}
	byte *timbreData = _soundData->data() + timbreDataOffset;

	AudioInitIcon *icon = new AudioInitIcon();
	icon->show();
	uint32 iconTime = g_system->getMillis();

	// Send SysExes

	// System Area
	uint32 address = 0x10 << 14; // 10 00 00
	static const uint8 systemAreaSysExLengths[5] = { 1, 3, 9, 9, 1 };
	for (int i = 0; i < 5; ++i) {
		_mt32Driver->sysExMT32(timbreData, systemAreaSysExLengths[i], address, true);
		address += systemAreaSysExLengths[i];
		timbreData += systemAreaSysExLengths[i];
	}
	// Patch Temporary Area
	address = 0x03 << 14; // 03 00 00
	int sysexLength = 16;
	for (int i = 0; i < 8; ++i) {
		_mt32Driver->sysExMT32(timbreData, sysexLength, address, true);
		address += sysexLength;
		timbreData += sysexLength;
	}
	// Timbre Memory
	address = 0x08 << 14; // 08 00 00
	sysexLength = 246;
	for (int i = 0; i < 64; ++i) {
		_mt32Driver->sysExMT32(timbreData, sysexLength, address, true);
		address += 256;
		timbreData += sysexLength;
	}
	// Patch Memory
	address = 0x05 << 14; // 05 00 00
	sysexLength = 8;
	for (int i = 0; i < 128; ++i) {
		_mt32Driver->sysExMT32(timbreData, sysexLength, address, true);
		address += sysexLength;
		timbreData += sysexLength;
	}
	// Rhythm Part Setup Temporary Area
	address = 0x03 << 14 | 0x01 << 7 | 0x10; // 03 01 10
	sysexLength = 4;
	for (int i = 0; i < 85; ++i) {
		_mt32Driver->sysExMT32(timbreData, sysexLength, address, true);
		address += sysexLength;
		timbreData += sysexLength;
	}

	// Wait until SysExes have been transmitted.
	bool result = false;
	while (!_mt32Driver->isReady()) {
		Events &events = Events::getReference();

		if (events.interruptableDelay(10)) {
			if (LureEngine::getReference().shouldQuit() ||
					(canAbort && events.type() == Common::EVENT_KEYDOWN && events.event().kbd.keycode == 27)) {
				// User has quit the game or pressed Escape.
				_mt32Driver->clearSysExQueue();
				result = true;
				break;
			}
		}

		// Blink the audio initialization icon every 500 ms
		if (g_system->getMillis() > iconTime + 500) {
			icon->toggleVisibility();
			iconTime = g_system->getMillis();
		}
	}

	icon->hide();
	delete icon;

	return result;
}

void SoundManager::bellsBodge() {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::bellsBodge");
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();

	RoomData *roomData = res.getRoom(room.roomNumber());
	if (roomData && roomData->areaFlag != res.fieldList().getField(AREA_FLAG)) {
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
			// fall through
		default:
			killSound(1);
			break;
		}
	}
}

void SoundManager::killSounds() {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::killSounds");

	// Stop the player playing all sounds
	musicInterface_KillAll();

	// Clear the active sounds
	_activeSounds.clear();
	Common::fill(_channelsInUse, _channelsInUse + NUM_CHANNELS, false);
}

void SoundManager::addSound(uint8 soundIndex, bool tidyFlag) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::addSound index=%d", soundIndex);
	Game &game = Game::getReference();

	if (tidyFlag)
		tidySounds();

	if (game.preloadFlag())
		// Don't add a sound if in room preloading
		return;

	SoundDescResource &rec = soundDescs()[soundIndex];
	int numChannels;

	if (_isRoland)
		numChannels = (rec.numChannels & 3);
	else
		numChannels = ((rec.numChannels >> 2) & 3) + 1;

	int channelCtr, channelCtr2;
	for (channelCtr = 0; channelCtr <= (NUM_CHANNELS - numChannels); ++channelCtr) {
		for (channelCtr2 = 0; channelCtr2 < numChannels; ++channelCtr2)
			if (_channelsInUse[channelCtr + channelCtr2])
				break;

		if (channelCtr2 == numChannels)
			break;
	}

	if (channelCtr > (NUM_CHANNELS - numChannels)) {
		// No channels free
		debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::addSound - no channels free");
		return;
	}

	// Mark the found channels as in use
	Common::fill(_channelsInUse+channelCtr, _channelsInUse+channelCtr + numChannels, true);

	SoundDescResource *newEntry = new SoundDescResource();
	newEntry->soundNumber = rec.soundNumber;
	newEntry->channel = channelCtr;
	newEntry->numChannels = numChannels;
	newEntry->flags = rec.flags;

	if (_isRoland)
		newEntry->volume = rec.volume;
	else /* resource volumes do not seem to work well with our AdLib emu */
		newEntry->volume = 240; /* 255 causes clipping with AdLib */

	_activeSounds.push_back(SoundList::value_type(newEntry));

	musicInterface_Play(rec.soundNumber, channelCtr, false, numChannels, newEntry->volume);
}

void SoundManager::addSound2(uint8 soundIndex) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::addSound2 index=%d", soundIndex);
	tidySounds();

	if (soundIndex == 6) {
		// Chinese torture
		stopSound(6); // sometimes its still playing when restarted
		addSound(6, false);
	} else {
		SoundDescResource &descEntry = soundDescs()[soundIndex];
		SoundDescResource *rec = findSound(descEntry.soundNumber);
		if (rec == NULL)
			// Sound isn't active, so go and add it
			addSound(soundIndex, false);
	}
}


void SoundManager::stopSound(uint8 soundIndex) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::stopSound index=%d", soundIndex);
	SoundDescResource &rec = soundDescs()[soundIndex];
	musicInterface_Stop(rec.soundNumber);
}

void SoundManager::killSound(uint8 soundNumber) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::stopSound soundNumber=%d", soundNumber);
	musicInterface_Stop(soundNumber);
}

void SoundManager::setVolume(uint8 soundNumber, uint8 volume) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::setVolume soundNumber=%d, volume=%d",
		soundNumber, volume);
	musicInterface_TidySounds();

	SoundDescResource *entry = findSound(soundNumber);
	if (entry)
		musicInterface_SetVolume(entry->channel, volume);
}

uint8 SoundManager::descIndexOf(uint8 soundNumber) {
	SoundDescResource *rec = soundDescs();

	for (uint8 index = 0; index < _numDescs; ++index, ++rec) {
		if (rec->soundNumber == soundNumber)
			return index;
	}

	return 0xff;   // Couldn't find entry
}

// Used to sync the volume for all channels with the Config Manager
//
void SoundManager::syncSounds() {
	musicInterface_TidySounds();

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");
	_musicVolume = mute ? 0 : MIN(256, ConfMan.getInt("music_volume"));
	_sfxVolume = mute ? 0 : MIN(256, ConfMan.getInt("sfx_volume"));

	if (_isRoland) {
		_mt32Driver->syncSoundSettings();
	} else {
		_soundMutex.lock();
		MusicListIterator i;
		for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
			// FIXME This should not override the sound resource volume
			// on the MidiMusic object.
			if ((*i)->isMusic())
				(*i)->setVolume(_musicVolume);
			else
				(*i)->setVolume(_sfxVolume);
		}
		_soundMutex.unlock();
	}
}

SoundDescResource *SoundManager::findSound(uint8 soundNumber) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::findSound soundNumber=%d", soundNumber);
	SoundListIterator i;

	for (i = _activeSounds.begin(); i != _activeSounds.end(); ++i) {
		SoundDescResource *rec = (*i).get();

		if (rec->soundNumber == soundNumber) {
			debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "SoundManager::findSound - sound found");
			return rec;
		}
	}

	// Signal that sound wasn't found
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "SoundManager::findSound - sound not found");
	return NULL;
}

void SoundManager::tidySounds() {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "SoundManager::tidySounds");
	SoundListIterator i = _activeSounds.begin();

	while (i != _activeSounds.end()) {
		SoundDescResource const &rec = **i;

		if (musicInterface_CheckPlaying(rec.soundNumber))
			// Still playing, so move to next entry
			++i;
		else {
			// Mark the channels that it used as now being free
			Common::fill(_channelsInUse + rec.channel, _channelsInUse + rec.channel + rec.numChannels, false);

			i = _activeSounds.erase(i);
		}
	}
}

void SoundManager::removeSounds() {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::removeSounds");
	bellsBodge();

	SoundListIterator i = _activeSounds.begin();

	while (i != _activeSounds.end()) {
		SoundDescResource const &rec = **i;

		if ((rec.flags & SF_IN_USE) != 0)
			musicInterface_Stop(rec.soundNumber);

		++i;
	}
}

void SoundManager::restoreSounds() {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::restoreSounds");
	SoundListIterator i = _activeSounds.begin();

	while (i != _activeSounds.end()) {
		SoundDescResource const &rec = **i;

		if ((rec.numChannels != 0) && ((rec.flags & SF_RESTORE) != 0)) {
			Common::fill(_channelsInUse + rec.channel, _channelsInUse + rec.channel + rec.numChannels, true);

			musicInterface_Play(rec.soundNumber, rec.channel, false, rec.numChannels, rec.volume);
		}

		++i;
	}
}

bool SoundManager::fadeOut() {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::fadeOut");

	Events &events = Events::getReference();
	bool result = false;

	// Fade out all the active sounds
	musicInterface_TidySounds();

	if (_isRoland) {
		_mt32Driver->startFade(3000, 0);
		while (_mt32Driver->isFading()) {
			if (events.interruptableDelay(100)) {
				result = ((events.type() == Common::EVENT_KEYDOWN && events.event().kbd.keycode == 27) ||
					LureEngine::getReference().shouldQuit());
				_mt32Driver->abortFade();
				break;
			}
		}
	} else {
		bool inProgress = true;
		while (inProgress) {
			inProgress = false;

			_soundMutex.lock();
			MusicListIterator i;
			for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
				MidiMusic &music = **i;
				if (music.getVolume() > 0) {
					inProgress = true;
					music.setVolume(music.getVolume() >= 10 ? music.getVolume() - 10 : 0);
				}
			}

			_soundMutex.unlock();
			g_system->delayMillis(10);
		}
	}

	// Kill all the sounds
	musicInterface_KillAll();

	if (_isRoland)
		_mt32Driver->setSourceVolume(MidiDriver_MT32GM::DEFAULT_SOURCE_NEUTRAL_VOLUME);

	return result;
}

void SoundManager::pause() {
	_paused = true;

	_soundMutex.lock();

	MusicListIterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		(**i).pauseMusic();
	}

	_soundMutex.unlock();

	// Terminate any hanging notes, just in case
	_driver->stopAllNotes();
}

void SoundManager::resume() {
	_paused = false;

	_soundMutex.lock();

	MusicListIterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		(**i).resumeMusic();
	}

	_soundMutex.unlock();
}

/*------------------------------------------------------------------------*/

// musicInterface_Play
// Play the specified sound

void SoundManager::musicInterface_Play(uint8 soundNumber, uint8 channelNumber, bool isMusic, uint8 numChannels, uint8 volume) {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_Play soundNumber=%d, channel=%d",
		soundNumber, channelNumber);
	Game &game = Game::getReference();

	if (!_soundData)
		error("Sound section has not been specified");

	uint8 soundNum = soundNumber & 0x7f;
	if (soundNum > _soundsTotal)
		error("Invalid sound index %d requested", soundNum);

	if (_driver == NULL)
		// Only play sounds if a sound driver is active
		return;

	// Most significant bit indicates if the track should loop or not
	bool loop = (soundNumber & 0x80) != 0;

	if (!game.soundFlag())
		// Don't play sounds if sound is turned off
		return;

	uint32 dataOfs = READ_LE_UINT32(_soundData->data() + soundNum * 4 + 2);
	uint8 *soundStart = _soundData->data() + dataOfs;
	uint32 dataSize;

	if (soundNum == _soundsTotal - 1)
		dataSize = _soundData->size() - dataOfs;
	else {
		uint32 nextDataOfs = READ_LE_UINT32(_soundData->data() + (soundNum + 1) * 4 + 2);
		dataSize = nextDataOfs - dataOfs;
	}

	// Note: the original interpreter seems to keep track of the "volume"
	// (velocity) adjustment last used for each MIDI channel. The volume
	// is not set in the few instances where musicInterface_Play is used
	// directly to play a sound instead of addSound (mostly cutscenes).
	// As a result, the volume adjustment is used that was last set on
	// the MIDI channel by whatever sound played there previously.
	// I think this is unintentional, so in ScummVM volume is set to 80h
	// (neutral) by default when calling musicInterface_Play without
	// specifying volume.
	_soundMutex.lock();
	int8 source = -1;
	if (_isRoland) {
		if (isMusic) {
			source = 0;
		} else {
			for (int i = 1; i < LURE_MAX_SOURCES; ++i) {
				if (!_sourcesInUse[i]) {
					source = i;
					break;
				}
			}
		}
		if (source == -1)
			warning("Insufficient sources to play sound %i", soundNumber);
		else
			_sourcesInUse[source] = true;
	}
	MidiMusic *sound = new MidiMusic(_driver, _channelsInner, channelNumber, soundNum,
		isMusic, loop, source, numChannels, soundStart, dataSize, volume);
	_playingSounds.push_back(MusicList::value_type(sound));
	_soundMutex.unlock();
}

// musicInterface_Stop
// Stops the specified sound from playing

void SoundManager::musicInterface_Stop(uint8 soundNumber) {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_Stop soundNumber=%d", soundNumber);
	musicInterface_TidySounds();
	uint8 soundNum = soundNumber & 0x7f;

	_soundMutex.lock();
	MusicListIterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		if ((*i)->soundNumber() == soundNum) {
			if ((*i)->source() >= 0)
				_sourcesInUse[(*i)->source()] = false;
			_playingSounds.erase(i);
			break;
		}
	}
	_soundMutex.unlock();
}

// musicInterface_CheckPlaying
// Returns true if a sound is still playing

bool SoundManager::musicInterface_CheckPlaying(uint8 soundNumber) {
	debugC(ERROR_DETAILED, kLureDebugSounds, "musicInterface_CheckPlaying soundNumber=%d", soundNumber);
	musicInterface_TidySounds();
	uint8 soundNum = soundNumber & 0x7f;
	bool result = false;

	_soundMutex.lock();
	MusicListIterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		if ((*i)->soundNumber() == soundNum) {
			result = true;
			break;
		}
	}
	_soundMutex.unlock();

	return result;
}

// musicInterface_SetVolume
// Sets the volume of the specified channel

void SoundManager::musicInterface_SetVolume(uint8 channelNum, uint8 volume) {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_SetVolume channel=%d, volume=%d",
		channelNum, volume);
	musicInterface_TidySounds();

	_soundMutex.lock();
	MusicListIterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		MidiMusic &music = **i;
		if (music.channelNumber() == channelNum)
			music.setVolume(volume);
	}
	_soundMutex.unlock();
}

// musicInterface_KillAll
// Stops all currently active sounds playing

void SoundManager::musicInterface_KillAll() {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_KillAll");
	musicInterface_TidySounds();

	_soundMutex.lock();
	MusicListIterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		(*i)->stopMusic();
	}

	Common::fill(_sourcesInUse, _sourcesInUse + LURE_MAX_SOURCES, false);
	_playingSounds.clear();
	_activeSounds.clear();
	_soundMutex.unlock();
}

// musicInterface_ContinuePlaying
// The original player used this method for any sound managers needing continual calls

void SoundManager::musicInterface_ContinuePlaying() {
	// No implementation needed
}

/*
 * TL;DR: TrashReverb does not seem to work correctly in the original interpreter
 * and I don't know what the developer's intentions were, so I've disabled it in
 * ScummVM.
 *
 * In the original interpreter, TrashReverb sends a SysEx to the MT-32 which sets
 * the reverb parameters to mode Room, time 1, level 0. This practically turns off
 * reverb. It is triggered by opening a door in an outdoors location in the town.
 * The SysEx is sent when the door sound starts, and again 40ms after the first
 * one. TrashReverb also seems to be triggered when a door is opened offscreen (by
 * an NPC).
 * This means that, as soon as you enter town, reverb is turned off when the first
 * door is opened. It is not turned back on. Reverb is restored only when you quit
 * and restart the game (during MT-32 initialization); it is then turned off again
 * when the first door outside in town is opened. Turning off the reverb
 * repeatedly (twice whenever a door is opened) does not seem to accomplish
 * anything.
 * The best explanation for this behavior I can come up with is that the developer
 * intended to disable reverb while playing a door opening sound outdoors. The
 * second SysEx when opening a door was meant to turn reverb back on, but turns it
 * off again because of a bug. Also, TrashReverb being triggered by doors opening
 * offscreen would be a bug. There is a third problem: the door opening sound lasts
 * much longer than 40 ms, so turning reverb back on 40 ms after starting the door
 * opening sound still results in a noticable reverb. All in all this explanation
 * is not entirely convicing.
 * Another explanation would be that reverb was only meant to be on for the first
 * part of the game and should be turned off from the town onwards (this is what
 * the implementation in the original interpreter effectively does). However, why
 * disable reverb when opening a door, and not f.e. when the player first enters
 * a town screen? And why is it not immediately turned off when a player loads a
 * savegame of a point later in the game? And why is reverb repeatedly disabled?
 * This does not make much sense either.
 * All in all, I am convinced that this functionality does not work correctly in
 * the original interpreter, but I don't know what the developer's intentions
 * were either. So I can't make a proper implementation for this, and I think it
 * is best left disabled.
 */

// musicInterface_TrashReverb
// Trashes reverb on actively playing sounds

void SoundManager::musicInterface_TrashReverb() {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_TrashReverb");

	/*
	// TODO Should this do anything on AdLib? It does not have reverb AFAIK
	if (_isRoland) {
		// Set reverb parameters to mode Room, time 1, level 0
		static const byte sysExData[] = { 0x00, 0x00, 0x00 };
		_mt32Driver->sysExMT32(sysExData, 3, 0x10 << 14 | 0x00 << 7 | 0x01);
	}
	*/
}

// musicInterface_KillAll
// Scans all the active sounds and deallocates any objects that have finished playing

void SoundManager::musicInterface_TidySounds() {
	debugC(ERROR_DETAILED, kLureDebugSounds, "musicInterface_TidySounds");

	_soundMutex.lock();
	MusicListIterator i = _playingSounds.begin();
	while (i != _playingSounds.end()) {
		if (!(*i)->isPlaying()) {
			if ((*i)->source() >= 0)
				_sourcesInUse[(*i)->source()] = false;
			i = _playingSounds.erase(i);
		} else {
			++i;
		}
	}
	_soundMutex.unlock();
}

void SoundManager::onTimer(void *data) {
	SoundManager *snd = (SoundManager *) data;
	snd->doTimer();
}

void SoundManager::doTimer() {
	if (_paused)
		return;

	_soundMutex.lock();

	MusicListIterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		MidiMusic &music = **i;
		if (music.isPlaying())
			music.onTimer();
	}

	_soundMutex.unlock();
}

/*------------------------------------------------------------------------*/

MidiMusic::MidiMusic(MidiDriver *driver, ChannelEntry channels[NUM_CHANNELS],
					 uint8 channelNum, uint8 soundNum, bool isMus, bool loop, int8 source, uint8 numChannels, void *soundData, uint32 size, uint8 volume) {
	_driver = driver;
	assert(_driver);
	_mt32Driver = dynamic_cast<MidiDriver_MT32GM *>(_driver);
	assert(!Sound.isRoland() || _mt32Driver);
	_source = source;
	_channels = channels;
	_soundNumber = soundNum;
	_channelNumber = channelNum;
	_isMusic = isMus;
	_loop = loop;

	_numChannels = numChannels;
	_volume = 0;

	// Set sound resource volume (default is 80h - neutral).
	// TODO AdLib currently does not use sound resource volume, so use fixed 240.
	setVolume(Sound.isRoland() ? volume : 240);

	_parser = MidiParser::createParser_SMF(source);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	// All Notes Off on all channels does not work with multiple MIDI sources
	_parser->property(MidiParser::mpDisableAllNotesOffMidiEvents, 1);
	_parser->property(MidiParser::mpAutoLoop, _loop);

	_soundData = (uint8 *)soundData;
	_soundSize = size;

	// Check whether the music data is compressed - if so, decompress it for the duration
	// of playing the sound

	_decompressedSound = NULL;
	if ((*_soundData == 'C') || (*_soundData == 'c')) {
		uint32 packedSize = size - 0x201;
		_decompressedSound = Memory::allocate(packedSize * 2);

		uint16 *data = (uint16 *)(_soundData + 1);
		uint16 *dataDest = (uint16 *) _decompressedSound->data();
		byte *idx  = ((byte *)data) + 0x200;

		for (uint i = 0; i < packedSize; i++)
#if defined(SCUMM_NEED_ALIGNMENT)
			memcpy(dataDest++, (byte *)((byte *)data + *(idx + i) * sizeof(uint16)), sizeof(uint16));
#else
			*dataDest++ = data[*(idx + i)];
#endif

		_soundData = _decompressedSound->data() + ((*_soundData == 'c') ? 1 : 0);
		_soundSize = _decompressedSound->size();
	}

	playMusic();
}

MidiMusic::~MidiMusic() {
	_parser->unloadMusic();
	if (Sound.isRoland() && _isPlaying)
		_mt32Driver->deinitSource(_source);
	delete _parser;
	delete _decompressedSound;
}

void MidiMusic::setVolume(int volume) {
	volume = CLIP(volume, 0, 255);

	if (_volume == volume)
		return;

	_volume = volume;

	// MT-32 MIDI data sets channel volume using control change,
	// so this is only needed for AdLib.
	if (!Sound.isRoland()) {
		volume *= _isMusic ? Sound.musicVolume() : Sound.sfxVolume();

		for (int i = 0; i < _numChannels; ++i) {
			if (_channels[_channelNumber + i].midiChannel != NULL)
				_channels[_channelNumber + i].midiChannel->volume(
					_channels[_channelNumber + i].volume *
					volume / 65025);
		}
	}
}

void MidiMusic::playMusic() {
	debugC(ERROR_DETAILED, kLureDebugSounds, "MidiMusic::PlayMusic playing sound %d", _soundNumber);
	if (Sound.isRoland() && !_isMusic)
		_mt32Driver->allocateSourceChannels(_source, _numChannels);
	_parser->loadMusic(_soundData, _soundSize);
	_parser->setTrack(0);
	_isPlaying = true;
}

void MidiMusic::send(uint32 b) {
	send(-1, b);
}

void MidiMusic::send(int8 source, uint32 b) {
	byte channel;
	if (Sound.isRoland()) {
		// Channel mapping is handled by the driver
		channel = b & 0x0F;
	} else {
		// Remap data channel to (one of) the channel(s) assigned to this player
#ifdef SOUND_CROP_CHANNELS
		if ((b & 0xF) >= _numChannels) return;
		channel = _channelNumber + (byte)(b & 0x0F);
#else
		channel = _channelNumber + ((byte)(b & 0x0F) % _numChannels);
#endif

		if ((channel >= NUM_CHANNELS) || (_channels[channel].midiChannel == NULL))
			return;
	}

	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by song and master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channels[channel].volume = volume;
		if (!Sound.isRoland()) {
			// Scale volume for AdLib only.
			// MT-32 sound resource volume is applied to note velocity,
			// and user volume scaling is handled by the driver.
			// TODO AdLib might use velocity for sound resource volume
			// as well.
			uint16 master_volume = _isMusic ? Sound.musicVolume() : Sound.sfxVolume();
			volume = volume * _volume * master_volume / 65025;
		}
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xFFF0) == 0x18B0) {
		if (Sound.isRoland())
			// Some tracks use CC 18. This is undefined in the MIDI standard
			// and does nothing on an MT-32. Not sending this to the device
			// in case it is a GM device with non-standard behavior for this CC.
			return;
	} else if ((b & 0xFFF0) == 0x007BB0) {
		// No implementation
	} else if (((b & 0xF0) == 0x90)) {
		// Note On
		if (Sound.isRoland()) {
			// Scale velocity with sound resource volume
			byte velocity = (b >> 16) & 0x7F;
			velocity = (velocity * _volume) >> 7;
			if (velocity > 0x7F) velocity = 0x7F;
			b = (b & 0xFF00FFFF) | (velocity << 16);
		}
	} else if (((b & 0xF0) == 0x80)) {
		// Note Off
		if (Sound.isRoland()) {
			// Strip velocity
			b &= 0xFF00FFFF;
		}
	} else if (((b & 0xF0) == 0xD0)) {
		// Channel aftertouch
		if (Sound.isRoland()) {
			// Some tracks contain aftertouch events, but the MT-32 does
			// not support this and the original interpreter does not send
			// them to the device.
			return;
		}
	}

	if (Sound.isRoland()) {
		_driver->send(source, b);
	} else {
		_channels[channel].midiChannel->send(b);
	}
}

void MidiMusic::metaEvent(byte type, byte *data, uint16 length) {
	metaEvent(-1, type, data, length);
}

void MidiMusic::metaEvent(int8 source, byte type, byte *data, uint16 length) {
	//Only thing we care about is End of Track.
	if (type != 0x2F)
		return;

	_driver->metaEvent(source, type, data, length);
	stopMusic();
}

void MidiMusic::onTimer() {
	if (_isPlaying)
		_parser->onTimer();
}

void MidiMusic::stopMusic() {
	debugC(ERROR_DETAILED, kLureDebugSounds, "MidiMusic::stopMusic sound %d", _soundNumber);
	_isPlaying = false;
	_parser->unloadMusic();
	if (Sound.isRoland())
		_mt32Driver->deinitSource(_source);
}

void MidiMusic::pauseMusic() {
	_parser->pausePlaying();
}

void MidiMusic::resumeMusic() {
	_parser->resumePlaying();
}

} // End of namespace Lure
