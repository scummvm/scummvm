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

#ifndef STARK_SAVE_METADATA_H
#define STARK_SAVE_METADATA_H

#include "common/error.h"
#include "common/system.h"

namespace Stark {

class ResourceSerializer;

/**
 * Utility class for reading and writing the save file header
 *
 * The save file header contains metadata related to the save
 * as well as a thumbnail of the game screen.
 */
class SaveMetadata {
public:
	Common::String description;
	uint levelIndex;
	uint locationIndex;
	uint version;
	uint32 totalPlayTime;

	uint8 saveDay;
	uint8 saveMonth;
	uint16 saveYear;

	uint8 saveHour;
	uint8 saveMinute;
	uint8 saveSecond;

	bool isAutoSave;

	const Graphics::Surface *gameWindowThumbnail;

	SaveMetadata();

	/** Set the time the save was created on */
	void setSaveTime(const TimeDate &timeDate);

	/**
	 * Read the metadata from a stream
	 *
	 * @param filename Save filename, used for the error messages only
	 */
	Common::ErrorCode read(Common::SeekableReadStream *stream, const Common::String &filename);

	/** Write the metadata to a stream */
	void write(Common::WriteStream *stream);

	/** Read the game screen thumbnail from a stream */
	Graphics::Surface *readGameScreenThumbnail(Common::SeekableReadStream *stream);

	/** Skip the game screen thumbnail in a stream */
	void skipGameScreenThumbnail(Common::SeekableReadStream *stream);

	/** Write the game screen thumbnail to a stream */
	void writeGameScreenThumbnail(Common::WriteStream *stream);

private:
	void saveLoad(ResourceSerializer *s);
	static void syncResourceIndexAsString(ResourceSerializer *s, uint &index);
	Common::ErrorCode syncVersion(ResourceSerializer *s);

	Common::String _readFilename;
	Common::ErrorCode _readErrorCode;
};

} // End of namespace Stark

#endif // STARK_SAVE_METADATA_H
