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

#include "lastexpress/sound/entry.h"

#include "lastexpress/game/logic.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"
#include "lastexpress/sound/sound.h"

#include "lastexpress/graphics.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

#define SOUNDCACHE_ENTRY_SIZE 92160
#define FILTER_BUFFER_SIZE 2940

//////////////////////////////////////////////////////////////////////////
// SoundEntry
//////////////////////////////////////////////////////////////////////////
SoundEntry::SoundEntry(LastExpressEngine *engine) : _engine(engine) {
	_status = 0;
	_tag = kSoundTagNone;

	_blockCount = 0;
	_startTime = 0;

	_stream = NULL;

	_volumeWithoutNIS = 0;
	_entity = kEntityPlayer;
	_initTimeMS = 0;
	_activateDelayMS = 0;
	_priority = 0;

	_subtitle = NULL;

	_soundStream = NULL;
}

SoundEntry::~SoundEntry() {
	// Entries that have been queued will have their streamed disposed automatically
	if (!_soundStream)
		SAFE_DELETE(_stream);

	SAFE_DELETE(_soundStream);

	_subtitle = NULL;
	_stream = NULL;

	// Zero passed pointers
	_engine = NULL;
}

void SoundEntry::open(Common::String name, SoundFlag flag, int priority) {
	_priority = priority;
	setupTag(flag);
	setupStatus(flag);
	loadStream(name);
}

void SoundEntry::close() {
	if (_soundStream) {
		delete _soundStream; // stops the sound in destructor
		_soundStream = NULL;
		_stream = NULL; // disposed by _soundStream
	}
	_status |= kSoundFlagClosed;

	// The original game remove the entry from the cache here,
	// but since we are called from within an iterator loop
	// we will remove the entry there
	// removeFromCache(entry);

	if (_subtitle) {
		_subtitle->close();
		SAFE_DELETE(_subtitle);
	}

	if (_entity) {
		if (_entity == kEntitySteam)
			getSound()->playAmbientSound(2);
		else if (_entity != kEntityTrain)
			getSavePoints()->push(kEntityPlayer, _entity, kActionEndSound);
	}
}

void SoundEntry::play(uint32 startTime) {
	if (_status & kSoundFlagClosed)
		return; // failed to load sound file

	if (!_stream)
		error("[SoundEntry::play] stream has been disposed");

	// Prepare sound stream
	if (_soundStream)
		error("[SoundEntry::play] already playing");

	// BUG: the original game never checks for sound type when loading subtitles.
	// NIS files and LNK files have the same base name,
	// so without extra caution NIS subtitles would be loaded for LNK sounds as well.
	// The original game instead separates calls to play() and setSubtitles()
	// and does not call setSubtitles() for linked-after sounds.
	// Unfortunately, that does not work well with save/load.
	if ((_status & kSoundTypeMask) != kSoundTypeLink && (_status & kSoundTypeMask) != kSoundTypeConcert) {
		// Get subtitles name
		uint32 size = (_name.size() > 4 ? _name.size() - 4 : _name.size());
		setSubtitles(Common::String(_name.c_str(), size));
	}

	_soundStream = new StreamedSound();

	_stream->seek(0);

	// Load the stream and start playing
	_soundStream->load(_stream, _status & kSoundVolumeMask, (_status & kSoundFlagLooped) != 0, startTime);

	_status |= kSoundFlagPlaying;
}

void SoundEntry::setupTag(SoundFlag flag) {
	switch (flag & kSoundTypeMask) {
	case kSoundTypeNormal:
		_tag = getSoundQueue()->generateNextTag();
		break;

	case kSoundTypeAmbient: {
		SoundEntry *previous2 = getSoundQueue()->getEntry(kSoundTagOldAmbient);
		if (previous2)
			previous2->fade();

		SoundEntry *previous = getSoundQueue()->getEntry(kSoundTagAmbient);
		if (previous) {
			previous->_tag = kSoundTagOldAmbient;
			previous->fade();
		}

		_tag = kSoundTagAmbient;
		}
		break;

	case kSoundTypeWalla: {
		SoundEntry *previous = getSoundQueue()->getEntry(kSoundTagWalla);
		if (previous) {
			previous->_tag = kSoundTagOldWalla;
			previous->fade();
		}

		_tag = kSoundTagWalla;
		}
		break;

	case kSoundTypeLink: {
		SoundEntry *previous = getSoundQueue()->getEntry(kSoundTagLink);
		if (previous)
			previous->_tag = kSoundTagOldLink;

		_tag = kSoundTagLink;
		}
		break;

	case kSoundTypeNIS: {
		SoundEntry *previous = getSoundQueue()->getEntry(kSoundTagNIS);
		if (previous)
			previous->_tag = kSoundTagOldNIS;

		_tag = kSoundTagNIS;
		}
		break;

	case kSoundTypeIntro: {
		SoundEntry *previous = getSoundQueue()->getEntry(kSoundTagIntro);
		if (previous)
			previous->_tag = kSoundTagOldMenu;

		_tag = kSoundTagIntro;
		}
		break;

	case kSoundTypeMenu: {
		SoundEntry *previous = getSoundQueue()->getEntry(kSoundTagMenu);
		if (previous)
			previous->_tag = kSoundTagOldMenu;

		_tag = kSoundTagMenu;
		}
		break;

	default:
		assert(false);
		break;
	}
}

void SoundEntry::setupStatus(SoundFlag flag) {
	_status = flag;

	// set the flag for the case that our savefiles
	// will be ever loaded by the original game
	if (!(_status & kSoundFlagLooped))
		_status |= kSoundFlagCloseOnDataEnd;
}

void SoundEntry::loadStream(Common::String name) {
	_name = name;

	// Load sound data
	_stream = getArchive(name);

	if (!_stream)
		_stream = getArchive("DEFAULT.SND");

	if (!_stream)
		_status = kSoundFlagClosed;

	// read total count of sound blocks for the case that our savefiles
	// will be ever loaded by the original game
	if (_stream) {
		_stream->readUint32LE();
		_blockCount = _stream->readUint16LE();
		_status |= kSoundFlagHeaderProcessed;
	}
}

void SoundEntry::setVolumeSmoothly(SoundFlag newVolume) {
	assert((newVolume & kSoundVolumeMask) == newVolume);

	if (_status & kSoundFlagFading)
		return;

	// the original game sets kSoundFlagVolumeChanging here
	uint32 requestedVolume = (uint32)newVolume;

	if (newVolume == kVolumeNone) {
		_status |= kSoundFlagFading;
	} else if (getSoundQueue()->getFlag() & 32) {
		_volumeWithoutNIS = requestedVolume;
		requestedVolume = requestedVolume / 2 + 1;
	}

	_status = (_status & ~kSoundVolumeMask) | requestedVolume;
	if (_soundStream)
		_soundStream->setVolumeSmoothly(requestedVolume);
}

bool SoundEntry::update() {
	if (_soundStream && _soundStream->isFinished())
		_status |= kSoundFlagClosed;

	if (_status & kSoundFlagClosed)
		return false;

	if (_status & kSoundFlagDelayedActivate) {
		// counter overflow is processed correctly
		if (_engine->_system->getMillis() - _initTimeMS >= _activateDelayMS) {
			_status &= ~kSoundFlagDelayedActivate;
			play();
		}
	} else {
		if (!(getSoundQueue()->getFlag() & 0x20)) {
			if (!(_status & kSoundFlagFixedVolume)) {
				if (_entity) {
					if (_entity < 0x80) {
						setVolume(getSound()->getSoundFlag(_entity));
					}
				}
			}
		}
		//if (_status & kSoundFlagHasUnreadData && !(_status & kSoundFlagMute) && v1->soundBuffer)
		//	Sound_FillSoundBuffer(v1);
	}

	return true;
}

void SoundEntry::setVolume(SoundFlag newVolume) {
	assert((newVolume & kSoundVolumeMask) == newVolume);

	if (newVolume == kVolumeNone) {
		_volumeWithoutNIS = 0;
	} else if (getSoundQueue()->getFlag() & 0x20 && _tag != kSoundTagNIS && _tag != kSoundTagLink) {
		setVolumeSmoothly(newVolume);
		return;
	}

	_status = (_status & ~kSoundVolumeMask) | newVolume;
	if (_soundStream)
		_soundStream->setVolume(_status & kSoundVolumeMask);
}

void SoundEntry::adjustVolumeIfNISPlaying() {
	if (getSoundQueue()->getFlag() & 32) {
		if (_tag != kSoundTagNIS && _tag != kSoundTagLink && _tag != kSoundTagConcert) {
			uint32 baseVolume = _status & kSoundVolumeMask;
			uint32 actualVolume = baseVolume / 2 + 1;

			assert((actualVolume & kSoundVolumeMask) == actualVolume);

			_volumeWithoutNIS = baseVolume;
			_status &= ~kSoundVolumeMask;
			_status |= actualVolume;
		}
	}
}

void SoundEntry::initDelayedActivate(unsigned activateDelay) {
	_initTimeMS = _engine->_system->getMillis();
	_activateDelayMS = activateDelay * 1000 / 15;
	_status |= kSoundFlagDelayedActivate;
}

void SoundEntry::setSubtitles(Common::String filename) {
	_subtitle = new SubtitleEntry(_engine);
	_subtitle->load(filename, this);

	if (_subtitle->getStatus() & 0x400) {
		_subtitle->close();
		SAFE_DELETE(_subtitle);
	} else {
		_status |= kSoundFlagHasSubtitles;
	}
}

void SoundEntry::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.isLoading()) {
		// load the fields
		uint32 blocksLeft;

		s.syncAsUint32LE(_status);
		s.syncAsUint32LE(_tag);
		s.syncAsUint32LE(blocksLeft);
		s.syncAsUint32LE(_startTime);
		uint32 unused;
		s.syncAsUint32LE(unused);
		s.syncAsUint32LE(unused);
		s.syncAsUint32LE(_entity);

		uint32 activateDelay;
		s.syncAsUint32LE(activateDelay);
		s.syncAsUint32LE(_priority);

		char name[16];
		s.syncBytes((byte *)name, 16); // _linkAfter name, should be always empty for entries in savefile
		s.syncBytes((byte *)name, 16); // real name
		name[15] = 0;

		// load the sound
		_blockCount = blocksLeft + _startTime;

		// if we are loading a savefile from the original game
		// and this savefile has been saved at a bizarre moment,
		// we can see transient flags here.
		// Let's pretend that the IRQ handler has run once.
		if (_status & kSoundFlagPlayRequested)
			_status |= kSoundFlagPlaying;
		if (_status & (kSoundFlagCloseRequested | kSoundFlagFading))
			_status |= kSoundFlagClosed;
		_status &= kSoundVolumeMask
		         | kSoundFlagPlaying
		         | kSoundFlagClosed
		         | kSoundFlagCloseOnDataEnd
		         | kSoundFlagLooped
		         | kSoundFlagDelayedActivate
		         | kSoundFlagHasSubtitles
		         | kSoundFlagFixedVolume
		         | kSoundFlagHeaderProcessed
		         | kSoundTypeMask;

		loadStream(name); // also sets _name
		if (_status & kSoundFlagPlaying)
			play((_status & kSoundFlagLooped) ? 0 : _startTime); // also loads subtitles

		_initTimeMS = _engine->_system->getMillis();
		_activateDelayMS = activateDelay * 1000 / 30;

	} else {
		assert(_name.size() < 16);
		assert(needSaving());
		// we can save our flags as is
		// the original game can reconstruct kSoundFlagMute, kSoundFlagCyclicBuffer, kSoundFlagHasUnreadData,
		// and we set other important flags correctly
		s.syncAsUint32LE(_status);
		s.syncAsUint32LE(_tag);
		uint32 time = getTime();
		uint32 blocksLeft = _blockCount - time;
		s.syncAsUint32LE(blocksLeft);
		s.syncAsUint32LE(time);
		uint32 unused = 0;
		s.syncAsUint32LE(unused);
		s.syncAsUint32LE(unused);
		s.syncAsUint32LE(_entity);

		uint32 deltaMS = _initTimeMS + _activateDelayMS - _engine->_system->getMillis();
		if (deltaMS > 0x8000000u) // sanity check against overflow
			deltaMS = 0;
		uint32 delta = deltaMS * 30 / 1000;
		s.syncAsUint32LE(delta);
		s.syncAsUint32LE(_priority);

		char name[16] = {0};
		s.syncBytes((byte *)name, 16);

		strcpy((char *)name, _name.c_str());
		s.syncBytes((byte *)name, 16);
	}
}

//////////////////////////////////////////////////////////////////////////
// SubtitleEntry
//////////////////////////////////////////////////////////////////////////
SubtitleEntry::SubtitleEntry(LastExpressEngine *engine) : _engine(engine) {
	_status = 0;
	_sound = NULL;
	_data = NULL;
}

SubtitleEntry::~SubtitleEntry() {
	SAFE_DELETE(_data);

	// Zero-out passed pointers
	_sound = NULL;
	_engine = NULL;
}

void SubtitleEntry::load(Common::String filename, SoundEntry *soundEntry) {
	// Add ourselves to the list of active subtitles
	getSoundQueue()->addSubtitle(this);

	// Set sound entry and filename
	_filename = filename + ".SBE";
	_sound = soundEntry;

	// Load subtitle data
	if (_engine->getResourceManager()->hasFile(_filename)) {
		if (getSoundQueue()->getSubtitleFlag() & 2)
			return;

		loadData();
	} else {
		_status = kSoundFlagClosed;
	}
}

void SubtitleEntry::loadData() {
	_data = new SubtitleManager(_engine->getFont());
	_data->load(getArchive(_filename));

	getSoundQueue()->setSubtitleFlag(getSoundQueue()->getSubtitleFlag() | 2);
	getSoundQueue()->setCurrentSubtitle(this);
}

void SubtitleEntry::setupAndDraw() {
	if (!_sound)
		error("[SubtitleEntry::setupAndDraw] Sound entry not initialized");

	if (!_data) {
		_data = new SubtitleManager(_engine->getFont());
		_data->load(getArchive(_filename));
	}

	if (_data->getMaxTime() > _sound->getTime()) {
		_status = kSoundFlagClosed;
	} else {
		_data->setTime((uint16)_sound->getTime());

		if (getSoundQueue()->getSubtitleFlag() & 1)
			drawOnScreen();
	}

	getSoundQueue()->setCurrentSubtitle(this);

	// TODO Missing code
}

void SubtitleEntry::close() {
	// Remove ourselves from the queue
	getSoundQueue()->removeSubtitle(this);

	if (this == getSoundQueue()->getCurrentSubtitle()) {
		drawOnScreen();

		getSoundQueue()->setCurrentSubtitle(NULL);
		getSoundQueue()->setSubtitleFlag(0);
	}
}

void SubtitleEntry::drawOnScreen() {
	if (_data == NULL)
		return;

	getSoundQueue()->setSubtitleFlag(getSoundQueue()->getSubtitleFlag() & -2);
	_engine->getGraphicsManager()->draw(_data, GraphicsManager::kBackgroundOverlay);
}

} // End of namespace LastExpress
