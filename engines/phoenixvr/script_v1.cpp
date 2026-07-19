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
#include "phoenixvr/script_v1.h"
#include "phoenixvr/commands_v1.h"
#include "phoenixvr/parser.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

namespace {
struct IfAnd : public ScriptV1::Conditional {
	IfAnd(Common::Array<Common::String> args) : ScriptV1::Conditional(Common::move(args)) {}
	void exec(ExecutionContext &ctx) const override {
		bool result = true;
		for (auto &var : vars) {
			if (var.empty() || !g_engine->hasVariable(var))
				continue;
			auto value = g_engine->getVariable(var);
			debug("ifand, %s: %d", var.c_str(), value);
			if (!value)
				result = false;
		}
		if (!result) {
			debug("ifand: not executing conditional block");
			return;
		}
		debug("ifand: executing conditional block");
		target->exec(ctx);
	}
};

struct IfOr : public ScriptV1::Conditional {
	IfOr(Common::Array<Common::String> args) : ScriptV1::Conditional(Common::move(args)) {}
	void exec(ExecutionContext &ctx) const override {
		bool result = false;
		for (auto &var : vars) {
			if (var.empty() || !g_engine->hasVariable(var))
				continue;
			auto value = g_engine->getVariable(var);
			debug("ifor, %s: %d", var.c_str(), value);
			if (value)
				result = true;
		}
		if (!result)
			return;
		debug("ifor: executing conditional block");
		target->exec(ctx);
	}
};

} // namespace

void ScriptV1::parseLine(const Common::String &line, uint lineno) {
	if (line.empty())
		return;

	Parser p(line, lineno);
	if (p.atEnd())
		return;

	if (p.maybe('[')) {
		if (p.maybe("bool]=") || p.maybe("bool)=") || p.maybe({"b\x00\x00ool]=", 8})) {
			_vars.push_back(p.nextWord());
		} else if (p.maybe("warp]=")) {
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
		} else if (p.maybe("test]=")) {
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
		} else {
			error("invalid [] directive on line %u: %s", lineno, line.c_str());
		}
	} else if (_currentTest) {
		auto &commands = _currentTest->scope.commands;
		if (p.maybe("ifand=") || p.maybe("ifand")) {
			if (_pluginScope)
				error("ifand in plugin scope");
			_conditional.reset(new IfAnd(p.readStringList()));
		} else if (p.maybe("ifor=") || p.maybe("ifor")) {
			if (_pluginScope)
				error("ifor in plugin scope");
			_conditional.reset(new IfOr(p.readStringList()));
		} else if (p.maybe("plugin")) {
			if (_pluginScope)
				error("nested plugin context is not allowed, line: %u", lineno);
			_pluginScope.reset(new Scope);
		} else if (p.maybe("endplugin")) {
			if (!_pluginScope)
				error("endplugin without plugin");
			if (_conditional) {
				_conditional->target = Common::move(_pluginScope);
				_pluginScope.reset();
				commands.push_back(Common::move(_conditional));
				_conditional.reset();
			} else {
				commands.push_back(Common::move(_pluginScope));
				_pluginScope.reset();
			}
		} else if (p.maybe("label")) {
			if (_pluginScope)
				error("no labels in plugin scope allowed");
			auto name = p.nextWord();
			auto offset = _currentTest->scope.commands.size();
			_currentTest->scope.labels.push_back({Common::move(name), offset});
		} else {
			if (_pluginScope) {
				auto name = p.nextWord();
				p.expect('(');
				auto args = p.readStringList();
				p.expect(')');
				auto cmd = createV1PluginCommand(name, args, lineno);
				if (cmd)
					_pluginScope->commands.push_back(Common::move(cmd));
				else
					error("unhandled plugin command %s at line %d", line.c_str(), lineno);
			} else {
				auto cmd = parseV1Command(p);
				if (cmd) {
					if (_conditional) {
						_conditional->target = Common::move(cmd);
						commands.push_back(Common::move(_conditional));
						_conditional.reset();
					} else
						commands.push_back(Common::move(cmd));
				} else
					error("unhandled script command %s at line %d", line.c_str(), lineno);
			}
		}
	} else
		error("invalid directive at line %u: %s", lineno, line.c_str());
}

} // namespace PhoenixVR
