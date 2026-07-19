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
#include "phoenixvr/commands.h"

namespace Common {
class SeekableReadStream;
}

namespace PhoenixVR {
class Parser;
class Script {
public:
	struct Test {
		int idx;
		int hover;
		Scope scope;
		ScopePtr enter;
		ScopePtr leave;
	};
	using TestPtr = Common::SharedPtr<Test>;

	struct Warp {
		Common::String vrFile;
		Common::String testFile;
		Common::Array<TestPtr> tests;

		void parseLine(const Common::String &line, uint lineno);
		TestPtr getTest(int idx) const;
		TestPtr getLastTest(int idx) const;
		TestPtr getDefaultTest() const {
			return getTest(-1);
		}
	};

	using WarpPtr = Common::SharedPtr<Warp>;
	using ConstWarpPtr = Common::SharedPtr<const Warp>;

protected:
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _warpsIndex;
	Common::Array<Common::String> _warpNames;
	Common::Array<Common::String> _vars;
	Common::Array<WarpPtr> _warps;

	virtual void parseLine(const Common::String &line, uint lineno) = 0;

public:
	Script();
	virtual ~Script();

	static Script *load(Common::SeekableReadStream &s, int version);

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
