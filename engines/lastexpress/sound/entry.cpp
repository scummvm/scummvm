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
	_status |= kSoundFlagClosed;
	// Loop until ready
	// _status |= kSoundFlagCloseRequested;
	//while (!(_status & kSoundFlagClosed) && !(getSoundQueue()->getFlag() & 8) && (getSoundQueue()->getFlag() & 1))
	//	;	// empty loop body

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

void SoundEntry::play() {
	if (!_stream)
		error("[SoundEntry::play] stream has been disposed");

	// Prepare sound stream
	if (!_soundStream)
		_soundStream = new StreamedSound();

	_stream->seek(0);

	// Load the stream and start playing
	_soundStream->load(_stream, _status & kSoundVolumeMask, (_status & kSoundFlagLooped) != 0);
}

bool SoundEntry::isFinished() {
	if (!_stream)
		return true;

	if (!_soundStream)
		return false;

	// TODO check that all data has been queued
	return _soundStream->isFinished();
}

void SoundEntry::setupTag(SoundFlag flag) {
	switch (flag & kSoundTypeMask) {
	default:
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

		_tag = kSoundTagIntro;
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
	}
}

void SoundEntry::setupStatus(SoundFlag flag) {
	_status = flag;
	if ((_status & kSoundVolumeMask) == kVolumeNone)
		_status |= kSoundFlagMuteRequested;

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
	assert(_name.size() < 16);

	bool result;
	char sub[16];

	if (_status & kSoundFlagClosed) {
		result = false;
	} else {
		if (_status & kSoundFlagDelayedActivate) {
			// counter overflow is processed correctly
			if (_engine->_system->getMillis() - _initTimeMS >= _activateDelayMS) {
				_status &= ~kSoundFlagDelayedActivate;
				play();

				// drop .SND extension
				strcpy(sub, _name.c_str());
				int l = _name.size();
				if (l > 4)
					sub[l - 4] = 0;
				showSubtitle(sub);
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
		result = true;
	}

	return result;
}

void SoundEntry::setVolume(SoundFlag newVolume) {
	assert((newVolume & kSoundVolumeMask) == newVolume);

	if (newVolume) {
		if (getSoundQueue()->getFlag() & 0x20 && _tag != kSoundTagNIS && _tag != kSoundTagLink)
			setVolumeSmoothly(newVolume);
		else
			_status = newVolume + (_status & ~kSoundVolumeMask);
	} else {
		_volumeWithoutNIS = 0;
		_status |= kSoundFlagMuteRequested;
		_status &= ~(kSoundFlagVolumeChanging | kSoundVolumeMask);
	}
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

void SoundEntry::kill() {
	_status |= kSoundFlagClosed;
	_entity = kEntityPlayer;

	if (_stream) {
		if (!_soundStream) {
			SAFE_DELETE(_stream);
		} else {
			// the original stream will be disposed
			_soundStream->stop();
			SAFE_DELETE(_soundStream);
		}

		_stream = NULL;
	}
}

void SoundEntry::showSubtitle(Common::String filename) {
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
	assert(_name.size() <= 16);

	if (_name.matchString("NISSND?") && ((_status & kSoundTypeMask) != kSoundTypeMenu)) {
		s.syncAsUint32LE(_status);
		s.syncAsUint32LE(_tag);
		s.syncAsUint32LE(_blockCount); // field_8;
		uint32 time = getTime();
		s.syncAsUint32LE(time);
		uint32 unused = 0;
		s.syncAsUint32LE(unused);
		s.syncAsUint32LE(unused);
		s.syncAsUint32LE(_entity);

		if (s.isLoading()) {
			uint32 delta;
			s.syncAsUint32LE(delta);
			_initTimeMS = _engine->_system->getMillis();
			_activateDelayMS = delta * 1000 / 15;
		} else {
			uint32 deltaMS = _initTimeMS + _activateDelayMS - _engine->_system->getMillis();
			if (deltaMS > 0x8000000u) // sanity check against overflow
				deltaMS = 0;
			uint32 delta = deltaMS * 15 / 1000;
			s.syncAsUint32LE(delta);
		}

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
