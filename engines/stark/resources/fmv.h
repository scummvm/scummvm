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

#ifndef STARK_RESOURCES_FMV_H
#define STARK_RESOURCES_FMV_H

#include "common/path.h"
#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

/**
 * A full motion video
 */
class FMV : public Object {
public:
	static const Type::ResourceType TYPE = Type::kFMV;

	FMV(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~FMV();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;

	/** Request the user interface to start playing the movie */
	void requestPlayback();

protected:
	void printData() override;

	Common::Path _filename;
	bool _diaryAddEntryOnPlay;
	uint32 _gameDisc;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_FMV_H
