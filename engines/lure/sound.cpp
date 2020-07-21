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

	_driver = MidiDriver::createMidi(dev);
	int statusCode = _driver->open();
	if (statusCode) {
		warning("Sound driver returned error code %d", statusCode);
		_driver = NULL;

	} else {
		if (_nativeMT32) {
			_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
			_driver->sendMT32Reset();
		} else {
			_driver->sendGMReset();
		}

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

void SoundManager::initCustomTimbres() {
	if (!_isRoland || !_nativeMT32 || _driver == NULL)
		return;

	if (!_soundData)
		error("SoundManager::initCustomTimbres - sound section has not been specified");

	// Locate timbre data
	uint32 headerSize = READ_LE_UINT32(_soundData->data() + 2); // Skip past the number of sounds
	uint16 timbreDataHeaderOffset = _soundsTotal * 4 + 2;
	if (timbreDataHeaderOffset + 6 > headerSize) {
		warning("SoundManager::initCustomTimbres - could not find timbre data header");
		return;
	}

	uint32 timbreDataOffset = READ_LE_UINT32(_soundData->data() + timbreDataHeaderOffset + 2); // Skip past end of header mark
	if (timbreDataOffset + 17259 > _soundData->size()) {
		warning("SoundManager::initCustomTimbres - timbre data smaller than expected");
		return;
	}
	byte *timbreData = _soundData->data() + timbreDataOffset;

	AudioInitIcon *icon = new AudioInitIcon();
	icon->show();

	// Send SysExes

	// System Area
	uint32 address = 0x10 << 14; // 10 00 00
	static const uint8 systemAreaSysExLengths[5] = { 1, 3, 9, 9, 1 };
	for (int i = 0; i < 5; ++i) {
		mt32SysEx(address, timbreData, systemAreaSysExLengths[i]);
		address += systemAreaSysExLengths[i];
		timbreData += systemAreaSysExLengths[i];
	}
	// Patch Temporary Area
	address = 0x03 << 14; // 03 00 00
	int sysexLength = 16;
	for (int i = 0; i < 8; ++i) {
		mt32SysEx(address, timbreData, sysexLength);
		address += sysexLength;
		timbreData += sysexLength;
	}
	// Timbre Memory
	address = 0x08 << 14; // 08 00 00
	sysexLength = 246;
	for (int i = 0; i < 64; ++i) {
		mt32SysEx(address, timbreData, sysexLength);
		address += 256;
		timbreData += sysexLength;
	}
	// Patch Memory
	address = 0x05 << 14; // 05 00 00
	sysexLength = 8;
	for (int i = 0; i < 128; ++i) {
		mt32SysEx(address, timbreData, sysexLength);
		address += sysexLength;
		timbreData += sysexLength;
	}
	// Rhythm Part Setup Temporary Area
	address = 0x03 << 14 | 0x01 << 7 | 0x10; // 03 01 10
	sysexLength = 4;
	for (int i = 0; i < 85; ++i) {
		mt32SysEx(address, timbreData, sysexLength);
		address += sysexLength;
		timbreData += sysexLength;
	}

	icon->hide();
	delete icon;
}

void SoundManager::mt32SysEx(const uint32 targetAddress, const byte *dataPtr, uint8 length) {
	byte   sysExMessage[270];
	uint16 sysExPos = 0;
	byte   sysExByte;
	uint16 sysExChecksum = 0;

	memset(&sysExMessage, 0, sizeof(sysExMessage));

	sysExMessage[0] = 0x41; // Roland
	sysExMessage[1] = 0x10;
	sysExMessage[2] = 0x16; // Model MT32
	sysExMessage[3] = 0x12; // Command DT1

	sysExChecksum = 0;

	sysExMessage[4] = (targetAddress >> 14) & 0x7F;
	sysExMessage[5] = (targetAddress >> 7) & 0x7F;
	sysExMessage[6] = targetAddress & 0x7F;

	for (byte targetAddressByte = 4; targetAddressByte < 7; targetAddressByte++) {
		assert(sysExMessage[targetAddressByte] < 0x80); // security check
		sysExChecksum -= sysExMessage[targetAddressByte];
	}

	sysExPos = 7;
	for (int i = 0; i < length; ++i) {
		sysExByte = *dataPtr++;

		assert(sysExPos < sizeof(sysExMessage));
		assert(sysExByte < 0x80); // security check
		sysExMessage[sysExPos++] = sysExByte;
		sysExChecksum -= sysExByte;
	}

	// Calculate checksum
	assert(sysExPos < sizeof(sysExMessage));
	sysExMessage[sysExPos++] = sysExChecksum & 0x7F;

	_driver->sysEx(sysExMessage, sysExPos);

	// Wait the time it takes to send the SysEx data
	uint32 delay = (length + 2) * 1000 / 3125;

	// Plus an additional delay for the MT-32 rev00
	if (_nativeMT32)
		delay += 40;

	g_system->delayMillis(delay);
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

	musicInterface_Play(rec.soundNumber, channelCtr, false, numChannels);
	musicInterface_SetVolume(channelCtr, newEntry->volume);
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

			musicInterface_Play(rec.soundNumber, rec.channel, false, rec.numChannels);
			musicInterface_SetVolume(rec.channel, rec.volume);
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

	// Kill all the sounds
	musicInterface_KillAll();
}

/*------------------------------------------------------------------------*/

// musicInterface_Play
// Play the specified sound

void SoundManager::musicInterface_Play(uint8 soundNumber, uint8 channelNumber, bool isMusic, uint8 numChannels) {
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

	_soundMutex.lock();
	MidiMusic *sound = new MidiMusic(_driver, _channelsInner, channelNumber, soundNum,
		isMusic, loop, numChannels, soundStart, dataSize);
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

	_playingSounds.clear();
	_activeSounds.clear();
	_soundMutex.unlock();
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

	_soundMutex.lock();
	MusicListIterator i = _playingSounds.begin();
	while (i != _playingSounds.end()) {
		if (!(*i)->isPlaying())
			i = _playingSounds.erase(i);
		else
			++i;
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
					 uint8 channelNum, uint8 soundNum, bool isMus, bool loop, uint8 numChannels, void *soundData, uint32 size) {
	_driver = driver;
	assert(_driver);
	_channels = channels;
	_soundNumber = soundNum;
	_channelNumber = channelNum;
	_isMusic = isMus;
	_loop = loop;

	_numChannels = numChannels;
	_volume = 0;

	// Default sound resource volume: 80h (neutral).
	// TODO AdLib does not use sound resource volume, so use fixed 240.
	setVolume(Sound.isRoland() ? 0x80 : 240);

	_parser = MidiParser::createParser_SMF();
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
	_parser->loadMusic(_soundData, _soundSize);
	_parser->setTrack(0);
	_isPlaying = true;
}

void MidiMusic::send(uint32 b) {
	byte channel;
	if (Sound.isRoland() && _isMusic) {
		// Use the channel as defined in the MIDI data
		channel = b & 0x0F;
	} else {
		// Remap data channel to (one of) the channel(s) assigned to this player
#ifdef SOUND_CROP_CHANNELS
		if ((b & 0xF) >= _numChannels) return;
		channel = _channelNumber + (byte)(b & 0x0F);
#else
		if (Sound.isRoland()) {
			// Roland channels start at 1
			channel = _channelNumber + (((byte)(b & 0x0F) - 1) % _numChannels);
		} else {
			// AdLib channels start at 0
			channel = _channelNumber + ((byte)(b & 0x0F) % _numChannels);
		}
#endif

		if ((channel >= NUM_CHANNELS) || (_channels[channel].midiChannel == NULL))
			return;
	}

	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by song and master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channels[channel].volume = volume;
		uint16 master_volume = _isMusic ? Sound.musicVolume() : Sound.sfxVolume();
		if (Sound.isRoland()) {
			// MT-32 sound resource volume is applied to note velocity,
			// so scale MIDI data volume only with ScummVM master volume.
			volume = (volume * master_volume) >> 8;
		} else {
			// TODO AdLib might use velocity for sound resource volume
			// as well.
			volume = volume * _volume * master_volume / 65025;
		}
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0) {
		if (Sound.isRoland() && !Sound.hasNativeMT32() && channel != 9) {
			b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
		}
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
	}

	if (Sound.isRoland() && _isMusic) {
		_driver->send(b);
	} else {
		_channels[channel].midiChannel->send(b);
	}
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
}

} // End of namespace Lure
