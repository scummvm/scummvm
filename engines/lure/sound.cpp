/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
#include "audio/adlib_ms.h"
#include "audio/midiparser.h"

namespace Common {
DECLARE_SINGLETON(Lure::SoundManager);
}

namespace Lure {

//#define SOUND_CROP_CHANNELS

SoundManager::SoundManager() {
	Disk &disk = Disk::getReference();

	_descs = disk.getEntry(SOUND_DESC_RESOURCE_ID);
	_numDescs = _descs->size() / sizeof(SoundDescResource);
	_soundData = nullptr;
	_paused = false;

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
	_isRoland = MidiDriver::getMusicType(dev) != MT_ADLIB;
	_nativeMT32 = ((MidiDriver::getMusicType(dev) == MT_MT32) || ConfMan.getBool("native_mt32"));

	Common::fill(_sourcesInUse, _sourcesInUse + LURE_MAX_SOURCES, false);

	if (_isRoland) {
		_driver = _mt32Driver = new MidiDriver_MT32GM(MT_MT32);
	} else {
		_driver = new MidiDriver_ADLIB_Lure();
	}
	_driver->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);

	int statusCode = _driver->open();
	if (statusCode)
		error("Sound driver returned error code %d", statusCode);

	syncSounds();
}

SoundManager::~SoundManager() {
	if (_driver)
		_driver->setTimerCallback(this, nullptr);

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
		_driver = nullptr;
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
				addSound2(soundIndex);
		}
	}
}


void SoundManager::loadSection(uint16 sectionId) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::loadSection = %xh", sectionId);
	killSounds();

	if (_soundData) {
		delete _soundData;
		_driver->setTimerCallback(this, nullptr);
	}

	_soundData = Disk::getReference().getEntry(sectionId);
	_soundsTotal = *_soundData->data();

	_driver->setTimerCallback(this, &onTimer);
}

bool SoundManager::initCustomTimbres(bool canAbort) {
	if (!_isRoland || !_nativeMT32 || _mt32Driver == nullptr)
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
					(canAbort && events.type() == Common::EVENT_CUSTOM_ENGINE_ACTION_START &&
						events.event().customType == kActionEscape)) {
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
		numChannels = ((rec.numChannels >> 2) & 3);

	if (numChannels == 0)
		// Don't play sounds for which 0 channels are defined.
		return;

	SoundDescResource *newEntry = new SoundDescResource();
	newEntry->soundNumber = rec.soundNumber;
	newEntry->channel = rec.channel;
	newEntry->numChannels = numChannels;
	newEntry->flags = rec.flags;
	newEntry->volume = rec.volume;

	_activeSounds.push_back(SoundList::value_type(newEntry));

	musicInterface_Play(rec.soundNumber, false, numChannels, newEntry->volume);
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
		if (rec == nullptr)
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
		musicInterface_SetVolume(entry->soundNumber, volume);
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

	_driver->syncSoundSettings();
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
	return nullptr;
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
			musicInterface_Play(rec.soundNumber, false, rec.numChannels, rec.volume);
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

	_driver->startFade(3000, 0);
	while (_driver->isFading()) {
		if (events.interruptableDelay(100)) {
			result = ((events.type() == Common::EVENT_CUSTOM_ENGINE_ACTION_START && events.event().customType == kActionEscape)
						|| LureEngine::getReference().shouldQuit());
			_driver->abortFade();
			break;
		}
	}

	// Kill all the sounds
	musicInterface_KillAll();

	_driver->setSourceVolume(MidiDriver_Multisource::DEFAULT_SOURCE_NEUTRAL_VOLUME);

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

void SoundManager::musicInterface_Play(uint8 soundNumber, bool isMusic, uint8 numChannels, uint8 volume) {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_Play soundNumber=%d", soundNumber);
	Game &game = Game::getReference();

	if (!_soundData)
		error("Sound section has not been specified");

	uint8 soundNum = soundNumber & 0x7f;
	if (soundNum > _soundsTotal)
		error("Invalid sound index %d requested", soundNum);

	if (_driver == nullptr)
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
	if (source == -1) {
		warning("Insufficient sources to play sound %i", soundNumber);
	} else {
		_sourcesInUse[source] = true;
		MidiMusic *sound = new MidiMusic(_driver, soundNum, isMusic,
			loop, source, numChannels, soundStart, dataSize, volume);
		_playingSounds.push_back(MusicList::value_type(sound));
	}
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
			if ((*i)->getSource() >= 0)
				_sourcesInUse[(*i)->getSource()] = false;
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

void SoundManager::musicInterface_SetVolume(uint8 soundNumber, uint8 volume) {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_SetVolume soundNumber=%d, volume=%d",
		   soundNumber, volume);
	musicInterface_TidySounds();

	_soundMutex.lock();
	MusicListIterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		MidiMusic &music = **i;
		if (music.soundNumber() == soundNumber)
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
 * opening sound still results in a noticeable reverb. All in all this explanation
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
			if ((*i)->getSource() >= 0)
				_sourcesInUse[(*i)->getSource()] = false;
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

MidiMusic::MidiMusic(MidiDriver_Multisource *driver, uint8 soundNum, bool isMus, bool loop,
		int8 source, uint8 numChannels, void *soundData, uint32 size, uint8 volume) {
	_driver = driver;
	assert(_driver);
	_mt32Driver = dynamic_cast<MidiDriver_MT32GM *>(_driver);
	assert(!Sound.isRoland() || _mt32Driver);
	_source = source;
	_soundNumber = soundNum;
	_isMusic = isMus;
	_loop = loop;

	_numChannels = numChannels;
	_volume = volume;

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

	_decompressedSound = nullptr;
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
	if (_isPlaying)
		_driver->deinitSource(_source);
	delete _parser;
	delete _decompressedSound;
}

void MidiMusic::setVolume(int volume) {
	volume = CLIP(volume, 0, 255);

	_volume = volume;
}

void MidiMusic::playMusic() {
	debugC(ERROR_DETAILED, kLureDebugSounds, "MidiMusic::PlayMusic playing sound %d", _soundNumber);
	if (Sound.isRoland() && !_isMusic) {
		bool result = _mt32Driver->allocateSourceChannels(_source, _numChannels);
		if (!result) {
			stopMusic();
			return;
		}
	}
	_parser->loadMusic(_soundData, _soundSize);
	_parser->setTrack(0);
	_isPlaying = true;
}

void MidiMusic::send(uint32 b) {
	send(-1, b);
}

void MidiMusic::send(int8 source, uint32 b) {
	if ((b & 0xFFF0) == 0x18B0) {
		if (Sound.isRoland())
			// Some tracks use CC 18. This is undefined in the MIDI standard
			// and does nothing on an MT-32. Not sending this to the device
			// in case it is a GM device with non-standard behavior for this CC.
			return;
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

	_driver->send(source, b);
}

void MidiMusic::metaEvent(byte type, byte *data, uint16 length) {
	metaEvent(-1, type, data, length);
}

void MidiMusic::metaEvent(int8 source, byte type, byte *data, uint16 length) {
	if (type == MIDI_META_END_OF_TRACK)
		stopMusic();

	_driver->metaEvent(source, type, data, length);
}

void MidiMusic::onTimer() {
	if (_isPlaying)
		_parser->onTimer();
}

void MidiMusic::stopMusic() {
	debugC(ERROR_DETAILED, kLureDebugSounds, "MidiMusic::stopMusic sound %d", _soundNumber);
	_isPlaying = false;
	_parser->unloadMusic();
	_driver->deinitSource(_source);
}

void MidiMusic::pauseMusic() {
	_parser->pausePlaying();
}

void MidiMusic::resumeMusic() {
	_parser->resumePlaying();
}

// Note that the values higher than 0xF000 are one octave higher than the other
// values. Other than that only the lower 10 bits are significant.
const uint16 MidiDriver_ADLIB_Lure::OPL_FREQUENCY_LOOKUP[192] = {
	0x02B2, 0x02B4, 0x02B7, 0x02B9, 0x02BC, 0x02BE, 0x02C1, 0x02C3, 0x02C6, 0x02C9, 0x02CB, 0x02CE, 0x02D0, 0x02D3, 0x02D6, 0x02D8,
	0x02DB, 0x02DD, 0x02E0, 0x02E3, 0x02E5, 0x02E8, 0x02EB, 0x02ED, 0x02F0, 0x02F3, 0x02F6, 0x02F8, 0x02FB, 0x02FE, 0x0301, 0x0303,
	0x0306, 0x0309, 0x030C, 0x030F, 0x0311, 0x0314, 0x0317, 0x031A, 0x031D, 0x0320, 0x0323, 0x0326, 0x0329, 0x032B, 0x032E, 0x0331,
	0x0334, 0x0337, 0x033A, 0x033D, 0x0340, 0x0343, 0x0346, 0x0349, 0x034C, 0x034F, 0x0352, 0x0356, 0x0359, 0x035C, 0x035F, 0x0362,
	0x0365, 0x0368, 0x036B, 0x036F, 0x0372, 0x0375, 0x0378, 0x037B, 0x037F, 0x0382, 0x0385, 0x0388, 0x038C, 0x038F, 0x0392, 0x0395,
	0x0399, 0x039C, 0x039F, 0x03A3, 0x03A6, 0x03A9, 0x03AD, 0x03B0, 0x03B4, 0x03B7, 0x03BB, 0x03BE, 0x03C1, 0x03C5, 0x03C8, 0x03CC,
	0x03CF, 0x03D3, 0x03D7, 0x03DA, 0x03DE, 0x03E1, 0x03E5, 0x03E8, 0x03EC, 0x03F0, 0x03F3, 0x03F7, 0x03FB, 0x03FE, 0xFE01, 0xFE03,
	0xFE05, 0xFE07, 0xFE08, 0xFE0A, 0xFE0C, 0xFE0E, 0xFE10, 0xFE12, 0xFE14, 0xFE16, 0xFE18, 0xFE1A, 0xFE1C, 0xFE1E, 0xFE20, 0xFE21,
	0xFE23, 0xFE25, 0xFE27, 0xFE29, 0xFE2B, 0xFE2D, 0xFE2F, 0xFE31, 0xFE34, 0xFE36, 0xFE38, 0xFE3A, 0xFE3C, 0xFE3E, 0xFE40, 0xFE42,
	0xFE44, 0xFE46, 0xFE48, 0xFE4A, 0xFE4C, 0xFE4F, 0xFE51, 0xFE53, 0xFE55, 0xFE57, 0xFE59, 0xFE5C, 0xFE5E, 0xFE60, 0xFE62, 0xFE64,
	0xFE67, 0xFE69, 0xFE6B, 0xFE6D, 0xFE6F, 0xFE72, 0xFE74, 0xFE76, 0xFE79, 0xFE7B, 0xFE7D, 0xFE7F, 0xFE82, 0xFE84, 0xFE86, 0xFE89,
	0xFE8B, 0xFE8D, 0xFE90, 0xFE92, 0xFE95, 0xFE97, 0xFE99, 0xFE9C, 0xFE9E, 0xFEA1, 0xFEA3, 0xFEA5, 0xFEA8, 0xFEAA, 0xFEAD, 0xFEAF
};

MidiDriver_ADLIB_Lure::MidiDriver_ADLIB_Lure() :
		MidiDriver_ADLIB_Multisource(OPL::Config::kOpl2),
		_pitchBendSensitivity(1) {
	for (int i = 0; i < LURE_MAX_SOURCES; i++) {
		for (int j = 0; j < MIDI_CHANNEL_COUNT; j++) {
			memset(_instrumentDefs, 0, sizeof(_instrumentDefs));
		}
	}

	// The MIDI data uses monophonic channels and the original interpreter
	// allocates a fixed OPL channel to each MIDI channel of each source. This
	// behavior is similar to the static allocation mode, though the actual
	// channel allocations are a bit different.
	_allocationMode = ALLOCATION_MODE_STATIC;

	// These global settings are different from the base class defaults.
	_modulationDepth = MODULATION_DEPTH_LOW;
	_vibratoDepth = VIBRATO_DEPTH_LOW;
}

void MidiDriver_ADLIB_Lure::channelAftertouch(uint8 channel, uint8 pressure, uint8 source) {
	_activeNotesMutex.lock();

	// Find the active note on the specified channel.
	for (int i = 0; i < _numMelodicChannels; i++) {
		uint8 oplChannel = _melodicChannels[i];
		if (_activeNotes[oplChannel].noteActive && _activeNotes[oplChannel].source == source &&
				_activeNotes[oplChannel].channel == channel) {
			// Set the velocity of the note and recalculate and write the
			// volume.
			_activeNotes[oplChannel].velocity = pressure;

			recalculateVolumes(channel, source);

			break;
		}
	}

	_activeNotesMutex.unlock();
}

void MidiDriver_ADLIB_Lure::metaEvent(int8 source, byte type, byte *data, uint16 length) {
	if (type == MIDI_META_SEQUENCER && length >= 6 &&
			data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x3F && data[3] == 0x00) {
		// Custom sequencer meta event
		switch (data[4]) {
		case 0x01:
			// Instrument definition
			uint8 channel;
			channel = data[5];
			assert(length == 0x22);
			assert(source >= 0);
			assert(channel < MIDI_CHANNEL_COUNT);

			// Instrument definitions use the AdLib BNK format, but omit the
			// first 2 fields.
			AdLibBnkInstrumentDefinition bnkInstDef;
			memset(&bnkInstDef, 0, sizeof(bnkInstDef));
			memcpy((uint8*)&bnkInstDef + 2, &data[6], sizeof(AdLibBnkInstrumentDefinition) - 2);
			// Store the definition in the _instrumentDefs array.
			bnkInstDef.toOplInstrumentDefinition(_instrumentDefs[source][channel]);
			break;
		case 0x02:
			// Rhythm mode
			
			// data[5] == 0: off, >= 1: on.
			// This is never turned on in the game's music data, so this is not
			// implemented.
			break;
		case 0x03:
			// Pitch bend sensitivity

			_pitchBendSensitivity = data[5];
			break;
		default:
			// Unknown sequencer meta event.
			warning("MidiDriver_ADLIB_Lure::metaEvent - Unknown sequencer meta event type %X", data[4]);
			break;
		}
		return;
	}

	// Use default handling for other meta events.
	MidiDriver_ADLIB_Multisource::metaEvent(source, type, data, length);
}

MidiDriver_ADLIB_Lure::InstrumentInfo MidiDriver_ADLIB_Lure::determineInstrument(uint8 channel, uint8 source, uint8 note) {
	InstrumentInfo instrument = { 0, nullptr, 0 };

	// Lure does not use a rhythm channel.
	instrument.oplNote = note;
	// Get the instrument definition set by the last meta event.
	instrument.instrumentDef = &_instrumentDefs[source][channel];
	// Identify the instrument by source and channel.
	instrument.instrumentId = (source << 4) | channel;

	return instrument;
}

uint16 MidiDriver_ADLIB_Lure::calculateFrequency(uint8 channel, uint8 source, uint8 note) {
	// Lower the note by an octave. Notes in the lowest octave get clipped to 0.
	note -= (note >= 0xC ? 0xC : note);

	// The pitch bend is a number of semitones (in bits 8+) and an 8 bit
	// fraction of a semitone (only the most significant 4 bits are used).
	int32 newPitchBend = calculatePitchBend(channel, source, 0);

	// Discard the lower 4 bits of the pitch bend (the +8 is for rounding), 
	// add the MIDI note and clip the result to the range 0-5FF. Note that
	// MIDI notes 60-7F get clipped to 5F.
	uint16 noteValue = CLIP((note << 4) + ((newPitchBend + 8) >> 4), (int32)0, (int32)0x5FF);
	// Convert the note value to octave note and octave (block).
	uint8 octaveNote = (noteValue >> 4) % 12;
	uint8 block = (noteValue >> 4) / 12;

	// Add the note fraction to the octave note and look up the OPL frequency
	// (F-num) value to use.
	uint8 octaveNoteValue = (octaveNote << 4) | (noteValue & 0xF);
	uint16 oplFrequency = OPL_FREQUENCY_LOOKUP[octaveNoteValue];
	if (oplFrequency < 0xF000) {
		// Lookup values which have 0 in the highest 6 bits need to be lowered
		// one octave.
		if (block > 0) {
			block--;
		} else {
			// If the octave is already 0, bitshift the frequency to halve it.
			oplFrequency >>= 1;
		}
	}

	// Return the F-num and block in OPL register format.
	return (oplFrequency & 0x3FF) | (block << 10);
}

int32 MidiDriver_ADLIB_Lure::calculatePitchBend(uint8 channel, uint8 source, uint16 oplFrequency) {
	// Convert MIDI pitch bend value to a 14 bit signed value
	// (range -0x2000 - 0x2000).
	int16 newPitchBend = _controlData[source][channel].pitchBend - 0x2000;
	// Discard the lower 5 bits to turn it into a 9 bit value (-0x100 - 0x100).
	newPitchBend >>= 5;
	// Double it for every sensitivity semitone over 1. Note that sensitivity
	// is typically specified as 1, which will not change the value.
	newPitchBend <<= _pitchBendSensitivity - 1;

	return newPitchBend;
}

uint8 MidiDriver_ADLIB_Lure::calculateUnscaledVolume(uint8 channel, uint8 source, uint8 velocity, const OplInstrumentDefinition &instrumentDef, uint8 operatorNum) {
	uint8 operatorVolume = instrumentDef.getOperatorDefinition(operatorNum).level & OPL_MASK_LEVEL;

	// Scale the instrument definition operator volume by velocity.
	// Invert it, multiply by velocity, add 0x40 for rounding and divide by 7F.
	uint8 invertedVolume = (((0x3F - operatorVolume) * velocity) + 0x40) >> 7;

	// Invert the volume again before returning it.
	return 0x3F - invertedVolume;
}

} // End of namespace Lure
