/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/savemetadata.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/services/stateprovider.h"
#include "engines/stark/services/userinterface.h"

#include "graphics/surface.h"

namespace Stark {

SaveMetadata::SaveMetadata() :
		_readErrorCode(Common::kNoError),
		levelIndex(0),
		locationIndex(0),
		version(0),
		totalPlayTime(0),
		saveYear(0),
		saveMonth(0),
		saveDay(0),
		saveHour(0),
		saveMinute(0),
		saveSecond(0),
		isAutoSave(false),
		gameWindowThumbnail(nullptr) {
}

void SaveMetadata::saveLoad(ResourceSerializer *s) {
	s->syncAsString32(description);
	syncResourceIndexAsString(s, levelIndex);
	syncResourceIndexAsString(s, locationIndex);
	_readErrorCode = syncVersion(s);

	if (_readErrorCode != Common::kNoError) {
		return;
	}

	if (version >= 9) {
		s->syncAsByte(saveDay);
		s->syncAsByte(saveMonth);
		s->syncAsUint16BE(saveYear);
		s->syncAsByte(saveHour);
		s->syncAsByte(saveMinute);
		s->syncAsSint32LE(totalPlayTime);
	}

	if (version >= 10) {
		s->syncAsByte(saveSecond);
	}

	if (version >= 13) {
		s->syncAsUint32LE(isAutoSave);
	}
}

Common::ErrorCode SaveMetadata::read(Common::SeekableReadStream *stream, const Common::String &filename) {
	_readFilename = filename;
	ResourceSerializer s(stream, nullptr, 0);
	saveLoad(&s);
	return _readErrorCode;
}

void SaveMetadata::write(Common::WriteStream *stream) {
	ResourceSerializer s(nullptr, stream, 0);
	saveLoad(&s);
}

void SaveMetadata::syncResourceIndexAsString(ResourceSerializer *s, uint &index) {
	Common::String indexRaw = Common::String::format("%02x", index);
	s->syncAsString32(indexRaw);
	if (s->isLoading()) {
		index = strtol(indexRaw.c_str(), nullptr, 16);
	}
}

Common::ErrorCode SaveMetadata::syncVersion(ResourceSerializer *s) {
	Common::String versionRaw = Common::String::format("Version:\t%02d", version);
	s->syncAsString32(versionRaw);

	if (s->isLoading()) {
		if (!versionRaw.hasPrefix("Version:\t")) {
			warning("The save file '%s' does not match the expected format", _readFilename.c_str());
			return Common::kReadingFailed;
		}

		version = atoi(&(versionRaw.c_str()[8]));
		if (version < StateProvider::kMinSaveVersion || version > StateProvider::kSaveVersion) {
			warning("The save file '%s' version (v%d) is not supported by this version of ScummVM. Only versions v%d to v%d are allowed.",
			        _readFilename.c_str(), version, StateProvider::kMinSaveVersion, StateProvider::kSaveVersion);
			return Common::kReadingFailed;
		}
	}

	return Common::kNoError;
}

void SaveMetadata::setSaveTime(const TimeDate &timeDate) {
	saveDay = timeDate.tm_mday;
	saveMonth = timeDate.tm_mon + 1;
	saveYear = timeDate.tm_year + 1900;
	saveHour = timeDate.tm_hour;
	saveMinute = timeDate.tm_min;
	saveSecond = timeDate.tm_sec;
}

void SaveMetadata::writeGameScreenThumbnail(Common::WriteStream *stream) {
	assert(gameWindowThumbnail);
	assert(gameWindowThumbnail->pitch * gameWindowThumbnail->h == kThumbnailSize);

	stream->write((const byte *)gameWindowThumbnail->getPixels(), kThumbnailSize);
}

void SaveMetadata::skipGameScreenThumbnail(Common::SeekableReadStream *stream) {
	stream->skip(kThumbnailSize);
}

Graphics::Surface *SaveMetadata::readGameScreenThumbnail(Common::SeekableReadStream *stream) {
	Graphics::Surface *thumb = new Graphics::Surface();
	thumb->create(kThumbnailWidth, kThumbnailHeight, Gfx::Driver::getRGBAPixelFormat());

	stream->read(thumb->getPixels(), kThumbnailSize);

	return thumb;
}

} // End of namespace Stark
