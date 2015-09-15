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

#ifndef STARK_RESOURCES_KNOWLEDGE_H
#define STARK_RESOURCES_KNOWLEDGE_H

#include "common/str.h"

#include "engines/stark/resources/object.h"
#include "engines/stark/resourcereference.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

/**
 * A game logic state value holder
 */
class Knowledge : public Object {
public:
	static const Type::ResourceType TYPE = Type::kKnowledge;

	enum SubType {
		kBoolean = 0,
		kInteger = 2,
		kInteger2 = 3,
		kReference = 4,
		kBooleanWithChild = 5
	};

	Knowledge(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Knowledge();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void saveLoad(ResourceSerializer *serializer) override;

	/** Define the value for boolean Knowledge elements */
	void setBooleanValue(bool value);

	/** Obtain the value for boolean Knowledge elements */
	bool getBooleanValue();

	/** Define the value for integer Knowledge elements */
	void setIntegerValue(int32 value);

	/** Obtain the value for integer Knowledge elements */
	int32 getIntegerValue();

protected:
	void printData() override;

	bool _booleanValue;
	int32 _integerValue;
	ResourceReference _referenceValue;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_KNOWLEDGE_H
