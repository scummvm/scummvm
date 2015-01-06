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

#ifndef STARK_RESOURCES_SCRIPT_H
#define STARK_RESOURCES_SCRIPT_H

#include "common/str.h"

#include "engines/stark/resources/resource.h"

namespace Stark {

class Command;
class XRCReadStream;

class Script : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kScript;

	enum SubType {
		kSubTypeGameEvent = 4,
		kSubTypePlayerAction = 5,
		kSubTypeDialog = 6
	};

	enum ScriptType {
		kScriptTypeOnGameEvent = 0,
		kScriptTypePassiveDialog = 1,
		kScriptTypeOnPlayerAction = 2,
		kScriptType3 = 3,
		kScriptType4 = 4
	};

	enum GameEvent {
		kGameEventOnGameLoop = 0,
		kGameEventOnEnterLocation = 1,
		kGameEventOnExitLocation = 2
	};

	enum CallMode {
		kCallModeGameLoop = 1,
		kCallModeExitLocation = 2,
		kCallModeEnterLocation = 3,
		kCallModePlayerAction = 4,
		kCallModeDialogCreateSelections = 5,
		kCallModeDialogAnswer = 6
	};

	Script(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Script();

	// Resource API
	void readData(XRCReadStream *stream) override;
	void onAllLoaded();

	void reset();
	bool isEnabled();
	bool isOnBegin();
	void execute(uint32 callMode);

protected:
	void printData() override;

	bool shouldExecute(uint32 callMode);

	uint32 _scriptType;
	uint32 _runEvent;
	uint32 _minChapter;
	uint32 _maxChapter;
	bool _shouldResetGameSpeed;

	bool _enabled;
	Command *_nextCommand;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_SCRIPT_H
