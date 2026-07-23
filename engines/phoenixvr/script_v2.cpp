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

struct IfAnd : public ScriptV2::Conditional {
	IfAnd(const Common::Array<Common::String> &args) : ScriptV2::Conditional(args) {}
	void exec(ExecutionContext &ctx) const override {
		bool result = true;
		for (auto &c : conditions) {
			auto value = c->value();
			debug("ifand %s %s %s: %d", c->arg1.c_str(), c->op.c_str(), c->arg2.c_str(), value);
			if (!value) {
				result = false;
				break;
			}
		}
		branch(ctx, result);
	}
};

struct IfOr : public ScriptV2::Conditional {
	IfOr(const Common::Array<Common::String> &args) : ScriptV2::Conditional(args) {}
	void exec(ExecutionContext &ctx) const override {
		bool result = false;
		for (auto &c : conditions) {
			auto value = c->value();
			debug("ifor %s %s %s: %d", c->arg1.c_str(), c->op.c_str(), c->arg2.c_str(), value);
			if (value) {
				result = true;
				break;
			}
		}
		branch(ctx, result);
	}
};
} // namespace

ScriptV2::Conditional::Conditional(const Common::Array<Common::String> &args) {
	conditions.reserve(args.size());
	for (auto &arg : args) {
		conditions.push_back(ConditionPtr{new Condition(arg)});
	}
}

void ScriptV2::Conditional::branch(ExecutionContext &ctx, bool branch) const {
	if (branch && trueScope)
		trueScope->exec(ctx);
	if (!branch && falseScope)
		falseScope->exec(ctx);
}

ScriptV2::Condition::Condition(const Common::String &text) {
	if (
		!parse(text, "!=") &&
		!parse(text, "<>") &&
		!parse(text, "<=") &&
		!parse(text, ">=") &&
		!parse(text, "<") &&
		!parse(text, ">") &&
		!parse(text, "="))
		error("unknown condition %s", text.c_str());
}

bool ScriptV2::Condition::parse(const Common::String &text, const Common::String &maybeOp) {
	auto opPos = text.find(maybeOp);
	if (opPos == text.npos)
		return false;
	arg1 = text.substr(0, opPos);
	op = maybeOp;
	arg2 = text.substr(opPos + maybeOp.size());
	return true;
}

int ScriptV2::Condition::value() const {
	auto val1 = Command::valueOf(arg1);
	auto val2 = Command::valueOf(arg2);
	if (op == "=")
		return val1 == val2;
	else if (op == "!=" || op == "<>")
		return val1 != val2;
	else if (op == "<")
		return val1 < val2;
	else if (op == "<=")
		return val1 <= val2;
	else if (op == ">")
		return val1 > val2;
	else if (op == ">=")
		return val1 >= val2;
	error("invalid condition %s %s %s", arg1.c_str(), op.c_str(), arg2.c_str());
}

void ScriptV2::closeScope() {
	assert(!_conditionals.empty());
	auto conditional = _conditionals.back();
	_conditionals.pop_back();
	topScope().commands.push_back(Common::move(conditional.conditional));
}

void ScriptV2::closeAllScopes() {
	while (!_conditionals.empty())
		closeScope();
}

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
			_vars.push_back({name, value});
		} else if (p.maybe("warp]:")) {
			auto vr = p.nextWord();
			Common::String test;
			if (p.maybe(','))
				test = p.nextWord();

			if (!_conditionals.empty()) {
				warning("condition didn't have endif at the last test at line %d", lineno);
				assert(_currentTest);
				closeAllScopes();
			}

			_currentWarp.reset(new Warp{vr, Common::move(test), {}});
			if (!_warpsIndex.contains(vr))
				_warpsIndex[vr] = _warps.size();
			else
				warning("duplicate warp %s", vr.c_str());
			_warps.push_back(_currentWarp);
			_warpNames.push_back(vr);
			_currentTest.reset();
		} else if (p.maybe("test]:")) {
			if (!_currentWarp)
				error("test without warp");
			auto idx = p.nextInt();
			if (!_currentWarp)
				error("text must have parent wrap section");
			if (!_conditionals.empty()) {
				warning("condition didn't have endif at the last test at line %d", lineno);
				assert(_currentTest);
				closeAllScopes();
			}
			_currentTest.reset(new Test{idx, 0, {}, {}, {}});
			_currentWarp->tests.push_back(_currentTest);
		} else if (p.maybe("ifand]:")) {
			if (!_currentTest)
				error("ifand without test at line %d", lineno);
			ConditionalPtr conditional(new IfAnd(p.readStringList()));
			conditional->trueScope.reset(new Scope);
			_conditionals.push_back({conditional, conditional->trueScope});
		} else if (p.maybe("ifor]:")) {
			if (!_currentTest)
				error("ifor without test at line %d", lineno);
			ConditionalPtr conditional(new IfOr(p.readStringList()));
			conditional->trueScope.reset(new Scope);
			_conditionals.push_back({conditional, conditional->trueScope});
		} else if (p.maybe("else]")) {
			if (!_currentTest)
				error("else without test at line %d", lineno);
			if (_conditionals.empty())
				error("else without conditional at line %d", lineno);
			auto &top = _conditionals.back();
			if (top.conditional->falseScope)
				error("double else in condition at line %d", lineno);
			top.conditional->falseScope.reset(new Scope());
			top.scope = top.conditional->falseScope;
		} else if (p.maybe("endif]")) {
			if (!_currentTest)
				error("endif without test at line %d", lineno);
			if (!_conditionals.empty()) {
				closeScope();
			} else
				warning("endif without conditional at line %d", lineno);
		} else if (p.maybe("clic]")) {
			if (!_conditionals.empty())
				error("[clic] inside conditional at line %d", lineno);
			if (!_currentTest)
				error("[clic] without test at line %d", lineno);
		} else if (p.maybe("in]")) {
			if (!_conditionals.empty())
				error("[in] inside conditional at line %d", lineno);
			if (!_currentTest)
				error("[in] without test at line %d", lineno);
			if (_currentTest->enter)
				error("duplicate [in] handler");
			_currentTest->enter.reset(new Scope);
		} else if (p.maybe("out]")) {
			if (!_conditionals.empty())
				error("[out] inside conditional at line %d", lineno);
			if (!_currentTest)
				error("out without test at line %d", lineno);
			if (_currentTest->leave)
				error("duplicate [out] handler");
			_currentTest->leave.reset(new Scope);
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
		} else if (p.maybe('(')) {
			auto args = p.readStringList();
			p.expect(')');
			command = createV2Command(name, args, lineno);
		} else
			error("invalid syntax at %d", lineno);

		auto &commands = topScope().commands;
		if (command)
			commands.push_back(command);
	} else {
		error("command %s is out of the test block at line %d", line.c_str(), lineno);
	}
}

} // namespace PhoenixVR
