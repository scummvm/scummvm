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

namespace Common {
	class MemoryReadStream;
}


namespace Macs2 {
	namespace Script {

		struct ScriptVariable {
			uint32 a;
			uint32 b;
		};

		enum class MouseMode {
			Use = 0x15
		};

		class ScriptExecutor {

			private:
			// Does pretty much what 9F07 does
			byte ReadByte();
			uint16 ReadWord();

			Common::MemoryReadStream * _stream;

			void FuncA3D2();

			// void Func101D(uint16 x, uint16 y);
			
			void Func9F4D(uint16 &out1, uint16 &out2);

			// fn0037_C991 proc
			// Implements a walk to
			void FuncC991();

			// Implements opcode 28 - TODO: What exactly?
			void FuncC8E4();

			// Implements opcode 0e - changing scene animations
			void FuncB6BE();

			// TODO: Identify number of variables and default values
			ScriptVariable _variables[10000];

			MouseMode _mouseMode = MouseMode::Use;

			uint16 _interactedObjectID = 0;

			public:

			// TODO: Mockup variable to simulate conditions where the scripting
			// function would be called again, like after a walk to event
			bool requestCallback = false;

			ScriptExecutor();
		
			void ExecuteScript();
			void SetScript(Common::MemoryReadStream *stream);

	};
}	// namespace Script

} // namespace Macs2

#endif
