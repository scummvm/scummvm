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

#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Command;

/**
 * A script resource
 *
 * Scripts are made of a collection of Command resources. Commands
 * return the next command to be executed, allowing for branches.
 */
class Script : public Object {
public:
	static const Type::ResourceType TYPE = Type::kScript;

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

	/**
	 * Script call modes.
	 *
	 * Most script types are only meaningful for a call mode.
	 * The shouldExecute method checks the consistency between
	 * the script type and the call mode.
	 */
	enum CallMode {
		kCallModeGameLoop = 1,
		kCallModeExitLocation = 2,
		kCallModeEnterLocation = 3,
		kCallModePlayerAction = 4,
		kCallModeDialogCreateSelections = 5,
		kCallModeDialogAnswer = 6
	};

	enum ResumeStatus {
		kResumeComplete,
		kResumeAbort,
		kResumeSuspend
	};

	Script(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Script();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void saveLoad(ResourceSerializer *serializer) override;
	void saveLoadCurrent(ResourceSerializer *serializer) override;
	void onAllLoaded() override;
	void onGameLoop() override;

	/** Reset the script so that it can be executed again from the beginning */
	void reset();

	/** Is the script enabled? */
	bool isEnabled();

	/** Enable the script */
	void enable(bool value);

	/** Disable and reset the script */
	void stop();

	/** Is the script on its Begin command? */
	bool isOnBegin();

	/** Has the script ended? */
	bool isOnEnd();

	/** Get the script's startup command */
	Command *getBeginCommand();

	/** Attempt to run the script using the specified call mode */
	void execute(uint32 callMode);

	/** Pause the script for the specified time */
	void pause(int32 msecs);

	/** Suspend the script while the specified resource is running */
	void suspend(Object *cause);

	/** Is the script paused, or waiting for a resource to complete? */
	bool isSuspended();

	/** Get the resource the script is waiting to complete, if any */
	Object *getSuspendingResource() const;

	/** Returns true if the script is enabled and valid for this call mode */
	bool shouldExecute(uint32 callMode);

	/** Step the script to the next command, overriding all checks */
	void goToNextCommand();

	/**
	 * Add an object to the return list.
	 *
	 * The script will resume execution of this object once it reaches an End opcode
	 */
	void addReturnObject(Object *object);

	/** Set the outcome of a suspension (completion or abortion) */
	void setResumeStatus(ResumeStatus status);

protected:
	void print(uint depth) override;
	void printData() override;

	void updateSuspended();

	void resumeCallerExecution(Object *callerObject);

	uint32 _scriptType;
	uint32 _runEvent;
	uint32 _minChapter;
	uint32 _maxChapter;
	bool _shouldResetGameSpeed;

	bool _enabled;
	Command *_nextCommand;

	int32 _pauseTimeLeft;
	Object *_suspendingResource;
	ResumeStatus _resumeStatus;

	Common::Array<Object *> _returnObjects;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_SCRIPT_H
