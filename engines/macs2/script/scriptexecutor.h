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

namespace Common {
	class MemoryReadStream;
}


namespace Macs2 {
class Macs2Engine;

	namespace Script {

		struct ScriptVariable {
			uint16 a;
			uint16 b;
		};

		enum class MouseMode {
			Use = 0x15,
			// TODO: Check if correct
			UseInventory = 0x17,
			// TODO: Check if correct
			Talk = 0x18
		};

		class ScriptExecutor {

			private:

			bool isTimerActive = false;
			uint32 timerEndMillis;

			// We use this array to gather the dialogue choices as they come in
			Common::Array<Common::StringArray> DialogueChoices;

			// [1014h] global - current assumption is that this is set when we run
			// the script for the scene initialization and reset when we run when the
			// scene is active
			bool IsSceneInitRun = false;

			// TODO: Put in a git module
			Common::String IdentifyScriptOpcode(uint8 opcode, uint8 opcode2);
			Common::String IdentifyHelperOpcode(uint8 opcode, uint16 value);

			// [1012h] global - current assumption is that this guards script runs that
			// are not guarded by the [1014h] global
			// TODO: I think I had this one right before, the meaning of "is repeated run"
			// could be better
			bool IsRepeatRun = false;

			// Does pretty much what 9F07 does
			byte ReadByte();
			uint16 ReadWord();

			Common::MemoryReadStream * _stream;

			// We use this to keep track of whether we have read all bytes we should have read
			uint32 expectedEndLocation;

			void FuncA3D2();

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

			// Implements a lookup in the "areas" map
			uint16 Func101D(uint16 x, uint16 y);

			// 01E7:A903
			void ScriptPrintString();


			

			

			

			public:

			// TODO: Identify number of variables and default values
			Common::Array<ScriptVariable> _variables;

			void SetVariableValue(uint16 index, uint16 a, uint16 b);

			// TODO: Expose in a better place and keep in sync
			uint16 _charPosX = 276;
			uint16 _charPosY = 140;

			MouseMode _mouseMode = MouseMode::Use;

			uint16 _interactedObjectID = 0;
			uint16 _interactedOtherObjectID = 0;

			// TODO: Mockup variable to simulate conditions where the scripting
			// function would be called again, like after a walk to event
			bool requestCallback = false;

			Macs2::Macs2Engine* _engine;

			ScriptExecutor();
		
			void ExecuteScript();

			// Will execute the script and any object scripts until execution should be stopped
			// TODO: Consider if we should let the executor also figure out where to get the
			// first script from
			void Run(bool firstRun = false);

			void SetScript(Common::MemoryReadStream *stream);

			void tick();

			void StartTimer(uint32 duration);

			bool isRunningScript = false;
			bool isAwaitingCallback = false;

			// TODO: Impplement mutexes correctly
			bool IsExecuting() const {
				return isRunningScript || isAwaitingCallback;
			}

	};
}	// namespace Script

} // namespace Macs2

#endif
