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

#ifndef STARK_RESOURCES_PATH_H
#define STARK_RESOURCES_PATH_H

#include <common/rect.h>
#include <math/vector3d.h>
#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Path : public Object {
public:
	static const Type::ResourceType TYPE = Type::kPath;

	enum SubType {
		kPath2D = 1,
		kPath3D = 2
	};

	/** Path factory */
	static Object *construct(Object *parent, byte subType, uint16 index, const Common::String &name);

	Path(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Path();

	// Resource API
	virtual	void readData(Formats::XRCReadStream *stream) override;

protected:
	void printData() override;

	uint32 _field_30;
};

class Path2D : public Path {
public:
	Path2D(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Path2D();

	struct Step {
		float weight;
		Common::Point position;
	};

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;

private:
	// Resource API
	void printData();

	Common::Array<Step> _steps;
};

class Path3D : public Path {
public:
	Path3D(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Path3D();

	struct Step {
		float weight;
		Math::Vector3d position;
	};

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;

private:
	// Resource API
	void printData();

	Common::Array<Step> _steps;
	float _sortKey;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_PATH_H
