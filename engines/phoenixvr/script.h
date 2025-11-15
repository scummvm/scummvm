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

#ifndef PHOENIXVR_SCRIPT_H
#define PHOENIXVR_SCRIPT_H

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace PhoenixVR {

class PhoenixVREngine;

class Script {
public:
	struct ExecutionContext {
		PhoenixVREngine *engine;
		bool running = true;
	};
	struct Command {
		virtual ~Command() = default;
		virtual void exec(ExecutionContext &ctx) const = 0;
	};
	using CommandPtr = Common::SharedPtr<Command>;

	struct Test {
		int idx;
		Common::Array<CommandPtr> commands;
	};
	using TestPtr = Common::SharedPtr<Test>;

	struct Warp {
		Common::String vrFile;
		Common::String testFile;
		Common::Array<TestPtr> tests;

		void parseLine(const Common::String &line, uint lineno);
		void setText(int idx, const TestPtr &text);
	};

	using WarpPtr = Common::SharedPtr<Warp>;

private:
	Common::HashMap<Common::String, uint> _warpsIndex;
	Common::Array<WarpPtr> _warps;
	WarpPtr _currentWarp;
	TestPtr _currentTest;
	bool _pluginContext;

private:
	static Common::String strip(const Common::String &str);
	void parseLine(const Common::String &line, uint lineno);

public:
	Script(Common::SeekableReadStream &s);
	~Script();
};
} // namespace PhoenixVR

#endif
