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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lastexpress/sound/entry.h"

#include "lastexpress/game/sound.h"

#include "lastexpress/graphics.h"
#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

#include "common/stream.h"

namespace LastExpress {

//////////////////////////////////////////////////////////////////////////
// SoundEntry
//////////////////////////////////////////////////////////////////////////
SoundEntry::SoundEntry(LastExpressEngine *engine) : _engine(engine) {
	type = kSoundTypeNone;

	currentDataPtr = 0;
	soundData = NULL;

	blockCount = 0;
	time = 0;

	stream = NULL;

	field_34 = 0;
	field_38 = 0;
	field_3C = 0;
	field_40 = 0;
	entity = kEntityPlayer;
	field_48 = 0;
	priority = 0;

	subtitle = NULL;

	soundStream = NULL;
}

SoundEntry::~SoundEntry() {
	// Entries that have been queued would have their streamed disposed automatically
	if (!soundStream)
		SAFE_DELETE(stream);

	delete soundStream;

	_engine = NULL;
}

void SoundEntry::setStatus(SoundFlag flag) {
	SoundStatus statusFlag = (SoundStatus)flag;
	if (!((statusFlag & 0xFF) & kSoundStatusClear1))
		statusFlag = (SoundStatus)(statusFlag | kSoundStatusClear2);

	if (((statusFlag & 0xFF00) >> 8) & kSoundStatusClear0)
		status.status = (uint32)statusFlag;
	else
		status.status = (statusFlag | kSoundStatusClear4);
}

void SoundEntry::setInCache() {
	status.status |= kSoundStatusClear2;
}

void SoundEntry::update(uint val) {
	if (!(status.status3 & 64)) {
		int value2 = val;

		status.status |= kSoundStatus_100000;

		if (val) {
			if (getSound()->getFlag() & 32) {
				field_40 = val;
				value2 = val * 2 + 1;
			}

			field_3C = value2;
		} else {
			field_3C = 0;
			status.status |= kSoundStatus_40000000;
		}
	}
}

void SoundEntry::updateState() {
	if (getSound()->getFlag() & 32) {
		if (type != kSoundType9 && type != kSoundType7 && type != kSoundType5) {
			uint32 newStatus = status.status & kSoundStatusClear1;

			status.status &= kSoundStatusClearAll;

			field_40 = newStatus;
			status.status |= newStatus * 2 + 1;
		}
	}

	status.status |= kSoundStatus_20;
}

void SoundEntry::reset() {
	status.status |= kSoundStatusRemoved;
	entity = kEntityPlayer;

	if (stream) {
		if (!soundStream) {
			SAFE_DELETE(stream);
		} else {
			soundStream->stop();
			SAFE_DELETE(soundStream);
		}

		stream = NULL;
	}
}

void SoundEntry::showSubtitle(Common::String filename) {
	subtitle = new SubtitleEntry(_engine);
	subtitle->load(filename, this);

	if (subtitle->getStatus().status2 & 4) {
		subtitle->draw();
		SAFE_DELETE(subtitle);
	} else {
		status.status |= kSoundStatus_20000;
	}
}

//////////////////////////////////////////////////////////////////////////
// SubtitleEntry
//////////////////////////////////////////////////////////////////////////
SubtitleEntry::SubtitleEntry(LastExpressEngine *engine) : _engine(engine) {
	_sound = NULL;
	_data = NULL;
}

SubtitleEntry::~SubtitleEntry() {
	SAFE_DELETE(_data);
}

void SubtitleEntry::load(Common::String filename, SoundEntry *soundEntry) {
	// Add ourselves to the list of active subtitles
	getSound()->addSubtitle(this);

	// Set sound entry and filename
	_filename = filename + ".SBE";
	_sound = soundEntry;

	// Load subtitle data
	if (_engine->getResourceManager()->hasFile(filename)) {
		if (getSound()->getSubtitleFlag() & 2)
			return;

		loadData();
	} else {
		_status.status = kSoundStatus_400;
	}
}

void SubtitleEntry::loadData() {
	_data = new SubtitleManager(_engine->getFont());
	_data->load(getArchive(_filename));

	getSound()->setSubtitleFlag(getSound()->getSubtitleFlag() | 2);
	getSound()->setCurrentSubtitle(this);
}

void SubtitleEntry::setupAndDraw() {
	if (!_data) {
		_data = new SubtitleManager(_engine->getFont());
		_data->load(getArchive(_filename));
	}

	if (_data->getMaxTime() > _sound->time) {
		_status.status = kSoundStatus_400;
	} else {
		_data->setTime((uint16)_sound->time);

		if (getSound()->getSubtitleFlag() & 1)
			drawOnScreen();
	}

	getSound()->setCurrentSubtitle(this);
}

void SubtitleEntry::draw() {
	// Remove ourselves from the queue
	getSound()->removeSubtitle(this);

	if (this == getSound()->getCurrentSubtitle()) {
		drawOnScreen();

		getSound()->setCurrentSubtitle(NULL);
		getSound()->setSubtitleFlag(0);
	}
}

void SubtitleEntry::drawOnScreen() {
	getSound()->setSubtitleFlag(getSound()->getSubtitleFlag() & -1);

	if (_data == NULL)
		return;

	if (getSound()->getSubtitleFlag() & 1)
		_engine->getGraphicsManager()->draw(_data, GraphicsManager::kBackgroundOverlay);
}

} // End of namespace LastExpress
