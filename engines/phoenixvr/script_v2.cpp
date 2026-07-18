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
#include "phoenixvr/script_v2.h"
#include "phoenixvr/commands_v2.h"
#include "phoenixvr/parser.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

namespace {
struct SetVariable : public Command {
	Common::String name;
	Common::String value;
	SetVariable(const Common::String &n, const Common::String &v) : name(n), value(v) {}
	void exec(ExecutionContext &ctx) const override {
		g_engine->setVariable(name, valueOf(value));
	}
};
} // namespace

void ScriptV2::parseLine(const Common::String &line, uint lineno) {
	if (line.empty())
		return;

	Parser p(line, lineno);
	if (p.atEnd())
		return;

	if (p.maybe("//"))
		return;

	if (p.maybe('[')) {
		if (p.maybe("var]:")) {
			auto name = p.nextWord();
			int value = 0;
			if (p.maybe('=')) {
				value = p.nextInt();
			}
			debug("declared var %s: %d", name.c_str(), value);
			_vars.push_back(name);
		} else if (p.maybe("warp]:")) {
			auto vr = p.nextWord();
			Common::String test;
			if (p.maybe(','))
				test = p.nextWord();
			_currentWarp.reset(new Warp{vr, Common::move(test), {}});
			if (!_warpsIndex.contains(vr))
				_warpsIndex[vr] = _warps.size();
			else
				warning("duplicate warp %s", vr.c_str());
			_warps.push_back(_currentWarp);
			_warpNames.push_back(vr);
		} else if (p.maybe("test]:")) {
			if (!_currentWarp)
				error("test without warp");
			auto idx = p.nextInt();
			auto hover = 0;
			if (!_currentWarp)
				error("text must have parent wrap section");
			if (p.maybe(',')) {
				hover = p.nextInt();
			}
			_currentTest.reset(new Test{idx, hover, {}});
			_currentWarp->tests.push_back(_currentTest);
		} else if (p.maybe("ifand]:")) {
			if (!_currentTest)
				error("ifand without test");
		} else if (p.maybe("ifor]:")) {
			if (!_currentTest)
				error("ifor without test");
		} else if (p.maybe("else]")) {
		} else if (p.maybe("endif]")) {
		} else if (p.maybe("clic]")) {
			if (!_currentTest)
				error("clic without test");
		} else if (p.maybe("in]")) {
			if (!_currentTest)
				error("clic without test");
		} else if (p.maybe("out]")) {
			if (!_currentTest)
				error("clic without test");
		} else {
			error("invalid [] directive on line %u: %s", lineno, line.c_str());
		}
	} else if (_currentTest) {
		auto name = p.nextWord();
		CommandPtr command;
		if (p.maybe('=')) {
			auto value = p.nextWord();
			if (!p.atEnd())
				error("garbage at the end of the assignment, line %d", lineno);
			command.reset(new SetVariable(name, Common::move(value)));
		} else {
			auto args = p.readStringList();
			p.expect(')');
			command = createV2Command(name, args, lineno);
		}

		auto &commands = _currentTest->scope.commands;
		if (command)
			commands.push_back(command);
		else {
			warning("unimplemented command %s at line %d", name.c_str(), lineno);
		}
	}
}

} // namespace PhoenixVR
