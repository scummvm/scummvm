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
#include "lure/memory.h"
#include "lure/res.h"
#include "lure/room.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "sound/midiparser.h"

DECLARE_SINGLETON(Lure::SoundManager);

namespace Lure {

SoundManager::SoundManager() {
	_descs = Disk::getReference().getEntry(SOUND_DESC_RESOURCE_ID);
	_numDescs = _descs->size() / sizeof(SoundDescResource);
	_soundData = NULL;

	int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	_nativeMT32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));

	memset(_channelsInUse, false, NUM_CHANNELS_OUTER);

	_driver = MidiDriver::createMidi(midiDriver);
	int statusCode = _driver->open();
	if (statusCode) {
		warning("Sound driver returned error code %d", statusCode);
		_driver = NULL;

	} else {
		if (_nativeMT32)
			_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

		for (int index = 0; index < NUM_CHANNELS_INNER; ++index) {
			_channelsInner[index].midiChannel = _driver->allocateChannel();
			_channelsInner[index].volume = DEFAULT_VOLUME;
		}
	}
}

SoundManager::~SoundManager() {
	if (_driver)
		_driver->setTimerCallback(this, NULL);

	removeSounds();
	_activeSounds.clear();
	_playingSounds.clear();


	delete _descs;
	if (_soundData)
		delete _soundData;

	if (_driver)
		_driver->close();
	_driver = NULL;
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

void SoundManager::bellsBodge() {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::bellsBodge");
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
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::killSounds");

	// Stop the player playing all sounds
	musicInterface_KillAll();

	// Clear the active sounds
	_activeSounds.clear();
	for (int channelNum = 0; channelNum < NUM_CHANNELS_INNER; ++channelNum)
		_channelsInUse[channelNum] = false;
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
	int numChannels = (rec.numChannels >> 2) & 3;

	int channelCtr = 0;
	while (channelCtr <= (NUM_CHANNELS_OUTER - numChannels)) {
		if (!_channelsInUse[channelCtr]) {
			bool foundSpace = true;

			int channelCtr2 = 1;
			while (channelCtr2 < numChannels) {
				foundSpace = !_channelsInUse[channelCtr + channelCtr2];
				if (!foundSpace) break;
				++channelCtr2;
			}

			if (foundSpace) 
				break;
		}
		
		++channelCtr;
	}

	if (channelCtr > NUM_CHANNELS_OUTER - numChannels) {
		// No channels free
		debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::addSound - no channels free");
		return;
	}

	// Mark the found channels as in use
	for (int channelCtr2 = 0; channelCtr2 < numChannels; ++channelCtr2)
		_channelsInUse[channelCtr + channelCtr2] = true;

	SoundDescResource *newEntry = new SoundDescResource();
	newEntry->soundNumber = rec.soundNumber;
	newEntry->channel = channelCtr;
	newEntry->numChannels = numChannels;
	newEntry->flags = rec.flags;
	newEntry->volume = rec.volume;
	_activeSounds.push_back(newEntry);

	// TODO: Figure a better way of sharing channels between multiple parsers - currently
	// each parser seems to use 8 channels of a maximum 16 available, but here I'm 
	// overlapping channels 4 - 7 (3rd & 4th parser) across the other two
	byte innerChannel = (channelCtr < 4) ? ((channelCtr / 2) * 8) :
		(4 + (channelCtr / 2) * 8);

	musicInterface_Play(rec.soundNumber, innerChannel);
	setVolume(rec.soundNumber, rec.volume);	
}

void SoundManager::addSound2(uint8 soundIndex) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::addSound2 index=%d", soundIndex);
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
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::stopSound index=%d", soundIndex);
	SoundDescResource &rec = soundDescs()[soundIndex];
	musicInterface_Stop(rec.soundNumber & 0x7f);
}

void SoundManager::killSound(uint8 soundNumber) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::stopSound soundNumber=%d", soundNumber);
	musicInterface_Stop(soundNumber & 0x7f);
}

void SoundManager::setVolume(uint8 soundNumber, uint8 volume) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::setVolume soundNumber=%d, volume=%d", 
		soundNumber, volume);
	musicInterface_TidySounds();

	SoundDescResource *entry = findSound(soundNumber);
	if (entry) 
		musicInterface_SetVolume(entry->channel, volume);
}

void SoundManager::setVolume(uint8 volume) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::setVolume volume=%d", volume);

	for (int index = 0; index < NUM_CHANNELS_INNER; ++index) {
		_channelsInner[index].midiChannel->volume(volume);
		_channelsInner[index].volume = volume;
	}
}

SoundDescResource *SoundManager::findSound(uint8 soundNumber) {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::findSound soundNumber=%d", soundNumber);
	ManagedList<SoundDescResource *>::iterator i;

	for (i = _activeSounds.begin(); i != _activeSounds.end(); ++i) {
		SoundDescResource *rec = *i;

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
	ManagedList<SoundDescResource *>::iterator i = _activeSounds.begin();

	while (i != _activeSounds.end()) {
		SoundDescResource *rec = *i;

		if (musicInterface_CheckPlaying(rec->soundNumber & 0x7f))
			// Still playing, so move to next entry
			++i;
		else {
			// Mark the channels that it used as now being free
			for (int channelCtr = 0; channelCtr < rec->numChannels; ++channelCtr) 
				_channelsInUse[rec->channel + channelCtr] = false;
			
			i = _activeSounds.erase(i);
		}
	}
}

void SoundManager::removeSounds() {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::removeSounds");
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
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::restoreSounds");
	ManagedList<SoundDescResource *>::iterator i = _activeSounds.begin();

	while (i != _activeSounds.end()) {
		SoundDescResource *rec = *i;

		if ((rec->numChannels != 0) && ((rec->flags & SF_RESTORE) != 0)) {
			for (int channelCtr = 0; channelCtr < rec->numChannels; ++channelCtr)
				_channelsInUse[rec->channel + channelCtr] = true;

			musicInterface_Play(rec->soundNumber, rec->channel);
			musicInterface_SetVolume(rec->soundNumber, rec->volume);
		}

		++i;
	}
}

void SoundManager::fadeOut() {
	debugC(ERROR_BASIC, kLureDebugSounds, "SoundManager::fadeOut");

	// Fade out all the active sounds
	musicInterface_TidySounds();

	bool inProgress = true;
	while (inProgress)
	{
		inProgress = false;

		ManagedList<MidiMusic *>::iterator i;
		for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
			MidiMusic *music = *i;
			if (music->getVolume() > 0) {
				inProgress = true;
				music->setVolume(music->getVolume() > 4 ? (music->getVolume() - 10) : 0);
			}
		}

		g_system->delayMillis(10);
	}

	// Kill all the sounds
	musicInterface_KillAll();
}

/*------------------------------------------------------------------------*/

// musicInterface_Play
// Play the specified sound

void SoundManager::musicInterface_Play(uint8 soundNumber, uint8 channelNumber) {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_Play soundNumber=%d, channel=%d", 
		soundNumber, channelNumber);

	if (!_soundData)
		error("Sound section has not been specified");

	uint8 soundNum = soundNumber & 0x7f;
	if (soundNum > _soundsTotal) 
		error("Invalid sound index %d requested", soundNum);

	if (_driver == NULL)
		// Only play sounds if a sound driver is active
		return;

	uint32 dataOfs = READ_LE_UINT32(_soundData->data() + soundNum * 4 + 2);
	uint8 *soundStart = _soundData->data() + dataOfs;
	uint32 dataSize;

	if (soundNumber == _soundsTotal - 1)
		dataSize = _soundData->size() - dataOfs;
	else {
		uint32 nextDataOfs = READ_LE_UINT32(_soundData->data() + (soundNum + 1) * 4 + 2);
		dataSize = nextDataOfs - dataOfs;
	}

	MidiMusic *sound = new MidiMusic(_driver, _channelsInner, channelNumber, soundNumber, 
		soundStart, dataSize);
	_playingSounds.push_back(sound);		
}

// musicInterface_Stop
// Stops the specified sound from playing

void SoundManager::musicInterface_Stop(uint8 soundNumber) {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_Stop soundNumber=%d", soundNumber);
	musicInterface_TidySounds();
	uint8 soundNum = soundNumber & 0x7f;

	ManagedList<MidiMusic *>::iterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		MidiMusic *music = *i;
		if (music->soundNumber() == soundNum) {
			_playingSounds.erase(i);
			return;
		}
	}
}

// musicInterface_CheckPlaying
// Returns true if a sound is still playing

bool SoundManager::musicInterface_CheckPlaying(uint8 soundNumber) {
	debugC(ERROR_DETAILED, kLureDebugSounds, "musicInterface_CheckPlaying soundNumber=%d", soundNumber);
	musicInterface_TidySounds();
	uint8 soundNum = soundNumber & 0x7f;

	ManagedList<MidiMusic *>::iterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		MidiMusic *music = *i;
		if (music->soundNumber() == soundNum) 
			return true;
	}

	return false;
}

// musicInterface_SetVolume
// Sets the volume of the specified channel

void SoundManager::musicInterface_SetVolume(uint8 channelNum, uint8 volume) {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_SetVolume channel=%d, volume=%d", 
		channelNum, volume);
	musicInterface_TidySounds();

	ManagedList<MidiMusic *>::iterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		MidiMusic *music = *i;
		if (music->channelNumber() == channelNum)
			music->setVolume(volume);
	}
}

// musicInterface_KillAll
// Stops all currently active sounds playing

void SoundManager::musicInterface_KillAll() {
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_KillAll");
	musicInterface_TidySounds();

	ManagedList<MidiMusic *>::iterator i;
	for (i = _playingSounds.begin(); i != _playingSounds.end(); ++i) {
		MidiMusic *music = *i;
		music->stopMusic();
	}

	_playingSounds.clear();
	_activeSounds.clear();
}

// musicInterface_ContinuePlaying
// The original player used this method for any sound managers needing continual calls

void SoundManager::musicInterface_ContinuePlaying() {
	// No implementation needed
}

// musicInterface_TrashReverb
// Trashes reverb on actively playing sounds

void SoundManager::musicInterface_TrashReverb() {
	// TODO: Handle support for trashing reverb
	debugC(ERROR_INTERMEDIATE, kLureDebugSounds, "musicInterface_TrashReverb");
}

// musicInterface_KillAll
// Scans all the active sounds and deallocates any objects that have finished playing

void SoundManager::musicInterface_TidySounds() {
	debugC(ERROR_DETAILED, kLureDebugSounds, "musicInterface_TidySounds");
	ManagedList<MidiMusic *>::iterator i = _playingSounds.begin(); 
	while (i != _playingSounds.end()) {
		MidiMusic *music = *i;
		if (!music->isPlaying()) 
			i = _playingSounds.erase(i);
		else
			++i;
	}
}

void SoundManager::onTimer(void *data) {
	SoundManager *snd = (SoundManager *) data;

	ManagedList<MidiMusic *>::iterator i;
	for (i = snd->_playingSounds.begin(); i != snd->_playingSounds.end(); ++i) {
		MidiMusic *music = *i;
		if (music->isPlaying()) 
			music->onTimer();
	}
}

/*------------------------------------------------------------------------*/

MidiMusic::MidiMusic(MidiDriver *driver, ChannelEntry channels[NUM_CHANNELS_INNER], 
					 uint8 channelNum, uint8 soundNum, void *soundData, uint32 size) {

	_driver = driver;
	_channels = channels;
	_soundNumber = soundNum;
	_channelNumber = channelNum;
	_numChannels = 8;
	_volume = _channels[channelNum].volume;

	_passThrough = false;

	_parser = MidiParser::createParser_SMF();
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());

	this->open();

	_soundData = (uint8 *) soundData;
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
			memcpy(dataDest++, (byte*)((byte*)data + *(idx + i) * sizeof(uint16)), sizeof(uint16));
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
	delete _parser;
	this->close();
	if (_decompressedSound != NULL) 
		delete _decompressedSound;
}

void MidiMusic::setVolume(int volume) {
	if (volume < 0)
		volume = 0;
	else if (volume > 255)
		volume = 255;

	if (_volume == volume)
		return;

	_volume = volume;

	for (int i = 0; i < _numChannels; ++i) 
		_channels[_channelNumber + i].midiChannel->volume(
			_channels[_channelNumber + i].volume * _volume / 255);
}

void MidiMusic::playMusic() {
	debugC(ERROR_DETAILED, kLureDebugSounds, "MidiMusic::PlayMusic playing sound %d", _soundNumber);
	_parser->loadMusic(_soundData, _soundSize);
	_parser->setTrack(0);
	_isPlaying = true;
}

int MidiMusic::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	return 0;
}

void MidiMusic::close() {
}

void MidiMusic::send(uint32 b) {
	if (_passThrough) {
		_driver->send(b);
		return;
	}

	byte channel = _channelNumber + (byte)(b & 0x0F);
	if ((channel >= NUM_CHANNELS_INNER) || (_channels[channel].midiChannel == NULL))
		return;

	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channels[channel].volume = volume;
		volume = volume * _volume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}
	else if ((b & 0xFFF0) == 0x007BB0) {
		// No implementation
	}

	_channels[channel].midiChannel->send(b);
}

void MidiMusic::metaEvent(byte type, byte *data, uint16 length) {
	//Only thing we care about is End of Track.
	if (type != 0x2F)
		return;

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
	close();
}

} // end of namespace Lure
