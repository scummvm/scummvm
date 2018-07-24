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
#include "common/hashmap.h"

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Script;
class String;
class ItemTemplate;

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

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onAllLoaded() override;
	void onEnterLocation() override;
    void saveLoad(ResourceSerializer *serializer) override;

	ActionArray listPossibleActions() const;

	bool runScriptForAction(uint32 action);

	bool canPerformAction(uint32 action) const;

	/** If a default action is available, only it can be executed */
	int32 getDefaultAction() const;

	/** Replace the PAT tooltip with the name of a string resource */
	void setTooltip(String *string);

protected:
	struct Entry {
		uint32 _actionType;
		int32 _scriptIndex;
		Script *_script;
	};

	typedef Common::HashMap<uint32, Entry> EntryMap;

	void addOwnEntriesToItemEntries();
	Common::Array<Entry> listItemEntries() const;
	ItemTemplate *findItemTemplate();


	void printData() override;

	Common::Array<Entry> _ownEntries;
	EntryMap _itemEntries;
	int32 _defaultAction;
	int32 _tooltipOverrideIndex;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_PAT_TABLE_H
