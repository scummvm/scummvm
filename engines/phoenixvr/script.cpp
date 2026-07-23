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

#include "phoenixvr/script.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "phoenixvr/script_v1.h"
#include "phoenixvr/script_v2.h"

namespace PhoenixVR {

Script::Script() = default;
Script::~Script() = default;

Script::TestPtr Script::Warp::getTest(int idx) const {
	auto it = Common::find_if(tests.begin(), tests.end(), [&](const TestPtr &test) { return test->idx == idx; });
	return it != tests.end() ? *it : Script::TestPtr{};
}

Script::TestPtr Script::Warp::getLastTest(int idx) const {
	for (uint i = tests.size(); i > 0; --i) {
		if (tests[i - 1]->idx == idx)
			return tests[i - 1];
	}

	return Script::TestPtr{};
}

Script *Script::load(Common::SeekableReadStream &s, int version) {
	Common::ScopedPtr<Script> script;
	switch (version) {
	case 1:
		script.reset(new ScriptV1);
		break;
	case 2:
		script.reset(new ScriptV2);
		break;
	default:
		error("unsupported script version: %d", version);
	}
	s.seek(0);
	Common::Array<char> text(s.size());
	if (s.read(text.data(), text.size()) != text.size())
		error("script: short read");
	uint lineno = 1;
	uint lineStartOffset = 0;
	auto textSize = text.size();
	while (lineStartOffset < textSize) {
		auto lineStart = text.begin() + lineStartOffset;
		auto lineEndCR = Common::find(lineStart, text.end(), '\r');
		auto lineEndLF = Common::find(lineStart, text.end(), '\n');
		bool hasCR = lineEndCR != text.end();
		bool hasLF = lineEndLF != text.end();
		char *lineEnd;
		if (hasCR && hasLF) {
			if (lineEndCR + 1 == lineEndLF) {
				// CR LF
				lineEnd = lineEndLF;
			} else if (lineEndCR < lineEndLF) {
				// CR first, but not followed by LF
				lineEnd = lineEndCR;
			} else
				lineEnd = lineEndLF;
		} else if (hasCR) {
			lineEnd = lineEndCR;
		} else if (hasLF) {
			lineEnd = lineEndLF;
		} else {
			lineEnd = text.end();
		}
		lineStartOffset += Common::distance(lineStart, lineEnd) + 1;
		while (lineEnd > lineStart && Common::isSpace(lineEnd[-1]))
			--lineEnd;
		script->parseLine({lineStart, lineEnd}, lineno++);
	}
	return script.release();
}

int Script::getWarp(const Common::String &name) const {
	auto it = _warpsIndex.find(name);
	if (it != _warpsIndex.end())
		return it->_value;

	for (uint i = 0; i < _warps.size(); ++i) {
		if (_warps[i]->vrFile.equalsIgnoreCase(name))
			return i;
	}

	return -1;
}

Script::ConstWarpPtr Script::getWarp(int idx) const {
	return idx >= 0 && idx < static_cast<int>(_warps.size()) ? Script::ConstWarpPtr{_warps[idx]} : Script::ConstWarpPtr{};
}

Script::ConstWarpPtr Script::getInitScript() const {
	return _warps.front();
}

} // namespace PhoenixVR
