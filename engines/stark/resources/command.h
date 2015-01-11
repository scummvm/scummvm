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

#ifndef STARK_RESOURCES_COMMAND_H
#define STARK_RESOURCES_COMMAND_H

#include "common/array.h"
#include "common/str.h"

#include "engines/stark/resources/resource.h"
#include "engines/stark/resourcereference.h"

namespace Stark {

class Script;
class ResourceReference;
class XRCReadStream;

class Command : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kCommand;

	Command(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Command();

	enum SubType {
		kCommandBegin = 0,
		kCommandEnd = 1,

		kGoTo = 5,

		kPause = 9,

		kItem3DPlaceOn = 81,

		kItemEnable = 87,

		kItemPlaceDirection = 133
	};

	struct Argument {
		enum Type {
			kTypeInteger1 = 1,
			kTypeInteger2 = 2,
			kTypeResourceReference = 3,
			kTypeString = 4
		};

		uint32 type;
		uint32 intValue;
		Common::String stringValue;
		ResourceReference referenceValue;
	};

	/** Execute the command */
	Command *execute(uint32 callMode, Script *script);

	/** Obtain the next command to be executed */
	Command *nextCommand();

	/** Obtain the next command to be executed, depending on a predicate */
	Command *nextCommandIf(bool predicate);

protected:
	void readData(XRCReadStream *stream) override;
	void printData() override;

	Command *resolveArgumentSiblingReference(const Argument &argument);


	void op3DPlaceOn(const ResourceReference &itemRef, const ResourceReference &targetRef);
	void opPause(Script *script, const ResourceReference &durationRef);
	void opItemEnable(const ResourceReference &itemRef, int32 enable);
	void opPlaceDirection(const ResourceReference &itemRef, int32 direction);

	Common::Array<Argument> _arguments;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_COMMAND_H
