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

#ifndef STARK_RESOURCES_SCROLL_H
#define STARK_RESOURCES_SCROLL_H

#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

/**
 * Scroll position for a location
 */
class Scroll : public Object {
public:
	static const Type::ResourceType TYPE = Type::kScroll;

	Scroll(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Scroll();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onGameLoop() override;
	void saveLoadCurrent(ResourceSerializer *serializer) override;

	/** Start scrolling the location to this position */
	void start();

	/** Stop scrolling the location from this object */
	void stop();

	/** Is this scroll currently running? */
	bool isActive();

	/** Scroll the location to this position, with immediate effect */
	void applyToLocationImmediate();

protected:
	void printData() override;

	uint32 _coordinate;
	uint32 _field_30;
	uint32 _field_34;
	uint32 _bookmarkIndex;

	bool _active;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_SCROLL_H
