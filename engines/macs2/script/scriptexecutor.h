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

#ifndef MACS2_SCRIPTEXECUTOR_H
#define MACS2_SCRIPTEXECUTOR_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/str-array.h"
#include "common/rect.h"

namespace Common {
	class MemoryReadStream;
}


namespace Macs2 {
class Macs2Engine;
class GameObject;

	namespace Script {

		

		struct ScriptVariable {
			uint16 a;
			uint16 b;
		};
		// Order of cursors when loading from the file is
		//  { Talk = 0, Look = 1, Touch = 2, Walk = 3};
		enum class MouseMode {
			Talk = 0x13,
			// TODO: Confirm
			Look = 0x14,
			Use = 0x15,
			// TODO: Check if correct
			Walk = 0x16,
			// TODO: Check if correct
			UseInventory = 0x17
		};

		enum class ExecutorState {
			// We are not executing anything at the moment
			Idle,
			// We are in the middle of executing
			Executing,
			// Executing but paused until a callback happens
			WaitingForCallback
		};

		enum class ExecutionResult {
			// We have finished executing the script
			ScriptFinished,

			// We are now waiting for a callback
			WaitingForCallback
		};

		enum class ScriptExecutionState {
			// We are executing the scene script
			ExecutingSceneScript,

			// We are executing the other scripts
			ExecutingOtherScripts
		};



		class ScriptExecutor {

			/**
			 * Here is how the state of the script executor works
			 * We can have a scheduled run (which will execute the next tick) or we can run it right
			 * away
			 * A run can have the two global variables set on it, and therefore might require being
			 * run a second time
			 * A run can remain within one function call or it can have time-dependent phases during
			 * which execution is paused and we wait (e.g. a timed wait event)
			 * After the scene script, we need to run the scripts for all objects in the scene as
			 * well as all the items in the inventory (TODO: Still need to find the place in code
			 * where this is done and see how exactly it works)
			 * 
			 */

			


			private:

			// State variables from here

			// Overall state
			ExecutorState _state = ExecutorState::Idle;

			// Currently executed script
			Common::MemoryReadStream *_stream;

			// [1014h] global - current assumption is that this is set when we run
			// the script for the scene initialization and reset when we run when the
			// scene is active
			bool IsSceneInitRun = false;

				
			// [1012h] global - current assumption is that this guards script runs that
			// are not guarded by the [1014h] global
			// TODO: I think I had this one right before, the meaning of "is repeated run"
			// could be better
			bool IsRepeatRun = false;

			uint16 executingObjectIndex;

		
			ScriptExecutionState scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;

			// Returns true iff the object is relevant to be executing in the current scene
			bool IsRelevantObject(const GameObject *obj);

			// Handles the next step of execution based on the current state.
			// Can be run right after a previous step or be called after execution was paused
			// Needs to update the state to be valid again
			void Step();

			// Depending on the current state, chooses the next script to run
			// and adjusts the state
			// Should return true iff a new script was loaded
			bool LoadNextScript();
			

			bool isTimerActive = false;
			uint32 timerEndMillis;

			// We use this array to gather the dialogue choices as they come in
			Common::Array<Common::StringArray> DialogueChoices;

			

			// TODO: Put in a git module
			Common::String IdentifyScriptOpcode(uint8 opcode, uint8 opcode2);
			Common::String IdentifyHelperOpcode(uint8 opcode, uint16 value);


			// Does pretty much what 9F07 does
			byte ReadByte();
			uint16 ReadWord();

			

			// We use this to keep track of whether we have read all bytes we should have read
			uint32 expectedEndLocation;

			void FuncA3D2();
			void FuncA37A();

			// Implementation of opcode 13
			// 0037h:0A439h
			void SkipUntil14();

			// void Func101D(uint16 x, uint16 y);
			
			void Func9F4D(uint16 &out1, uint16 &out2);

			// Function to be used if we only want to have the script be advanced
			// due to a skipped implementation
			void Func9F4D_Placeholder();

			// Combines both 16 bit values into a 32 bit value
			uint32 Func9F4D_32();

			// Returns only the first of the two 16 bit values
			uint16 Func9F4D_16();

			// fn0037_C991 proc
			// Implements a walk to
			void FuncC991();

			// Implements opcode 28 - TODO: What exactly?
			void FuncC8E4();

			// Implements opcode 0e - changing scene animations
			void FuncB6BE();

			

			// 01E7:A903
			void ScriptPrintString();

			Common::StringArray debugBuffer;
			bool lastOpcodeTriggeredSkip = false;
			void BeginBuffering();
			void EndBuffering(bool shouldMark = false);
			void SIS_Debug(const char *format, ...);

			// Global [0F92h], seems to be 0 if we execute the script of the scene
			// and the object ID if we execute the script of another object
			// TODO: Confirm that this covers all uses
			uint16 _executingScriptObjectID;
			

			

			public:

				// Implements a lookup in the "areas" map
				uint16 Func101D(uint16 x, uint16 y);

			// Determines if we actually look something up when looking up 9F4D FF 27
			// in the obstacles/pathfinding map
			// Should be 1 while we execute a "character stopped walking" script,
			// otherwise 0
			bool global1032 = false;

			// Scene data [di+53B7h] - TODO: Confirm that we use a script variable as well as this thing
			int chosenDialogueOption = 0;

			// TODO: Identify number of variables and default values
			Common::Array<ScriptVariable> _variables;

			void SetVariableValue(uint16 index, uint16 a, uint16 b);


			Common::Point GetCharPosition();

			MouseMode _mouseMode = MouseMode::Use;

			uint16 _interactedObjectID = 0;
			uint16 _interactedOtherObjectID = 0;

			// Is set to true in opcode 2C if an object is inside another target object
			bool global103C = false;

			// TODO: Mockup variable to simulate conditions where the scripting
			// function would be called again, like after a walk to event
			// TODO: Rename to reflect this
			// TODO: Check if used like this consistently - not really, let's get rid of it
			bool requestCallback = false;

			Macs2::Macs2Engine* _engine;

			ScriptExecutor();
		
			ExecutionResult ExecuteScript();

			// Will execute the script and any object scripts until execution should be stopped
			// TODO: Consider if we should let the executor also figure out where to get the
			// first script from
			void Run(bool firstRun = false);

			void SetScript(Common::MemoryReadStream *stream);

			void tick();

			void StartTimer(uint32 duration);
			void EndTimer();

			bool isRunningScript = false;
			bool isAwaitingCallback = false;

			// TODO: Impplement mutexes correctly
			bool IsExecuting() const {
				return _state != ExecutorState::Idle;
				// return isRunningScript || isAwaitingCallback;
			}


			// Mutex indicating if the A3D2 function is active
			bool isSkipping = false;

			// Resets the script to the beginning
			// TODO: CHeck where this happens vs. leaving it at the place it is when leaving
			// the function in the game code
			void Rewind();

	};
}	// namespace Script

} // namespace Macs2

#endif
