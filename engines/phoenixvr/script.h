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
#include "phoenixvr/math.h"

namespace Common {
class SeekableReadStream;
}

namespace PhoenixVR {
namespace {
inline float toAngle(int a) {
	static const float angleToFloat = kPi / 4096.0f;
	return angleToFloat * static_cast<float>(a);
}

inline int fromAngle(float a) {
	if (a == INFINITY || a == -INFINITY)
		return -1;
	static const float floatToAngle = 4096.0f / kPi;
	return static_cast<int>(floatToAngle * a);
}
} // namespace
class Script {
public:
	struct Scope;
	struct ExecutionContext {
		bool running = true;
		bool subroutine = false;
		const Scope *scope = nullptr;
	};
	struct Command {
		virtual ~Command() {}
		virtual void exec(ExecutionContext &ctx) const = 0;
	};
	using CommandPtr = Common::SharedPtr<Command>;

	struct Scope : public Script::Command {
		Common::Array<CommandPtr> commands;

		struct Label {
			Common::String name;
			uint offset;
		};
		Common::Array<Label> labels;

		const Label *findLabel(const Common::String &name) const {
			auto it = Common::find_if(labels.begin(), labels.end(), [&](const Label &label) { return label.name.equalsIgnoreCase(name); });
			return it != labels.end() ? &*it : nullptr;
		}

		void exec(ExecutionContext &ctx) const override;
		void exec(ExecutionContext &ctx, uint offset) const;
	};
	using ScopePtr = Common::SharedPtr<Scope>;

	struct Conditional : public Script::Command {
		Common::Array<Common::String> vars;
		Script::CommandPtr target;
		Conditional(Common::Array<Common::String> args) : vars(Common::move(args)) {}
	};
	using ConditionalPtr = Common::SharedPtr<Conditional>;

	struct Test {
		int idx;
		int hover;
		Scope scope;
	};
	using TestPtr = Common::SharedPtr<Test>;

	struct Warp {
		Common::String vrFile;
		Common::String testFile;
		Common::Array<TestPtr> tests;

		void parseLine(const Common::String &line, uint lineno);
		TestPtr getTest(int idx) const;
		TestPtr getDefaultTest() const {
			return getTest(-1);
		}
	};

	using WarpPtr = Common::SharedPtr<Warp>;
	using ConstWarpPtr = Common::SharedPtr<const Warp>;

private:
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _warpsIndex;
	Common::Array<Common::String> _warpNames;
	Common::Array<Common::String> _vars;
	Common::Array<WarpPtr> _warps;
	WarpPtr _currentWarp;
	TestPtr _currentTest;
	ScopePtr _pluginScope;
	ConditionalPtr _conditional;

private:
	static Common::String strip(const Common::String &str);
	void parseLine(const Common::String &line, uint lineno);

public:
	Script(Common::SeekableReadStream &s);
	~Script();

	int getWarp(const Common::String &name) const;
	ConstWarpPtr getWarp(int index) const;
	ConstWarpPtr getInitScript() const;

	uint numWarps() const {
		return _warps.size();
	}

	const Common::Array<Common::String> &getWarpNames() const {
		return _warpNames;
	}
	const Common::Array<Common::String> &getVarNames() const {
		return _vars;
	}
};
} // namespace PhoenixVR

#endif
