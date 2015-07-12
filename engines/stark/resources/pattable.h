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

#ifndef STARK_RESOURCES_PAT_TABLE_H
#define STARK_RESOURCES_PAT_TABLE_H

#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Script;

typedef Common::Array<uint32> ActionArray;

class PATTable : public Object {
public:
	static const Type::ResourceType TYPE = Type::kPATTable;

	enum ActionType {
		kActionUse  = 1,
		kActionLook = 2,
		kActionTalk = 3,
		kActionExit = 7
	};

	PATTable(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~PATTable();

	ActionArray listPossibleActions() const;

	bool runScriptForAction(uint32 action);

	bool canPerformAction(uint32 action) const;
	// Resource API
	void readData(Formats::XRCReadStream *stream) override;

protected:
	struct Entry {
		uint32 _actionType;
		int32 _scriptIndex;
	};

	void printData() override;

	Common::Array<Entry> _entries;
	int32 _defaultAction;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_PAT_TABLE_H
