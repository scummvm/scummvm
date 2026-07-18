#include "phoenixvr/script.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "phoenixvr/commands.h"
#include "phoenixvr/parser.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

Script::CommandPtr Script::parseCommand(Parser &p) {
	if (p.keyword("setcursordefault")) {
		auto idx = p.nextWord();
		p.expect(',');
		bool valid = !idx.empty() && Common::isDigit(idx[0]);
		// this skips garbage cursor default found in Amerzone
		// e.g. `setcursordefault cursor1.pcx,cursor1`
		return CommandPtr(new SetCursorDefault(valid ? atoi(idx.c_str()) : -1, p.nextWord()));
	} else if (p.keyword("lockkey")) {
		auto idx = p.nextInt();
		p.expect(',');
		auto fname = p.nextWord();
		return CommandPtr(new LockKey(idx, Common::move(fname)));
	} else if (p.keyword("resetlockkey")) {
		return CommandPtr(new ResetLockKey());
	} else if (p.maybe("fade=")) {
		auto arg0 = p.nextInt();
		p.expect(',');
		auto arg1 = p.nextInt();
		p.expect(',');
		auto arg2 = p.nextInt();
		return CommandPtr(new Fade(arg0, arg1, arg2));
	} else if (p.keyword("transfade")) {
		return CommandPtr(new Transfade(p.nextInt()));
	} else if (p.maybe("setzoom=")) {
		return CommandPtr(new SetZoom(toRadian(p.nextInt())));
	} else if (p.maybe("setangle=") || p.keyword("setangle")) {
		auto a0 = toAngle(p.nextInt());
		p.expect(',');
		auto a1 = toAngle(p.nextInt());
		return CommandPtr(new SetAngle(a0, a1));
	} else if (p.maybe("setnord=")) {
		auto a0 = toAngle(p.nextInt());
		return CommandPtr(new SetNord(a0));
	} else if (p.keyword("interpolangle") || p.keyword("interpolanglezoom")) {
		p.maybe(',');
		p.maybe('=');
		auto a0 = toAngle(p.nextInt());
		p.expect(',');
		auto a1 = toAngle(p.nextInt());
		p.expect(',');
		float a2 = p.nextInt();
		float a3 = 0;
		if (p.maybe(','))
			a3 = p.nextInt();
		// x, y, speed
		// or
		// x, y, zoom, speed
		return CommandPtr(a3 != 0 ? new InterpolAngle(a0, a1, a3, toRadian(a2)) : new InterpolAngle(a0, a1, a2, 0));
	} else if (p.maybe("anglexmax=") || p.keyword("anglexmax")) {
		return CommandPtr(new AngleXMax(toAngle(p.nextInt())));
	} else if (p.maybe("angleymax=") || p.keyword("angleymax")) {
		auto y0 = toAngle(p.nextInt());
		p.expect(',');
		auto y1 = toAngle(p.nextInt());
		return CommandPtr(new AngleYMax(y0, y1));
	} else if (p.keyword("gotowarp")) {
		return CommandPtr(new GoToWarp(p.nextWord()));
	} else if (p.keyword("playsound3d")) {
		auto sound = p.nextWord();
		p.expect(',');
		auto arg0 = p.nextInt();
		p.expect(',');
		auto arg1 = p.nextInt();
		int arg2 = 0;
		if (p.maybe(','))
			arg2 = p.nextInt();
		return CommandPtr(new PlaySound3D(Common::move(sound), arg0, toAngle(arg1), arg2));
	} else if (p.keyword("playmusique")) {
		auto sound = p.nextWord();
		int vol = 255;
		if (p.maybe(','))
			vol = p.nextInt();
		return CommandPtr(new PlayMusique(Common::move(sound), vol));
	} else if (p.keyword("playsound")) {
		auto sound = p.nextWord();
		p.expect(',');
		auto arg0 = p.nextInt();
		int arg1 = 0;
		if (p.maybe(','))
			arg1 = p.nextInt();
		return CommandPtr(new PlaySound(Common::move(sound), arg0, arg1));
	} else if (p.keyword("playrndsound")) {
		auto sound = p.nextWord();
		p.expect(',');
		auto arg0 = p.nextInt();
		p.expect(',');
		auto arg1 = p.nextInt();
		int arg2 = 0;
		if (p.maybe(','))
			arg2 = p.nextInt();
		return CommandPtr(new PlayRndSound(Common::move(sound), arg0, arg1, arg2));
	} else if (p.keyword("stopsound3d")) {
		return CommandPtr(new StopSound3D(p.nextWord()));
	} else if (p.keyword("stopsound") || p.keyword("stopmusique")) {
		return CommandPtr(new StopSound(p.nextWord()));
	} else if (p.keyword("setcursor")) {
		auto image = p.nextWord();
		p.maybe(',');
		auto warp = p.nextWord();
		int idx = 0;
		if (p.maybe(','))
			idx = p.nextInt();
		return CommandPtr(new SetCursor(Common::move(image), Common::move(warp), idx));
	} else if (p.keyword("hidecursor")) {
		auto warp = p.nextWord();
		p.expect(',');
		auto idx = p.nextInt();
		return CommandPtr(new HideCursor(Common::move(warp), idx));
	} else if (p.keyword("set")) {
		auto var = p.nextWord();
		if (p.maybe(',')) {
			// this is typo in amerzone, this meant to be setCursor
			auto warp = p.nextWord();
			int idx = 0;
			if (p.maybe(','))
				idx = p.nextInt();
			return CommandPtr(new SetCursor(Common::move(var), Common::move(warp), idx));
		}
		int value = 0;
		if (p.maybe('=')) {
			p.skip();
			if (!p.atEnd() && p.peek() != '!')
				value = p.nextInt();
		}
		return CommandPtr(new SetVar(Common::move(var), value));
	} else if (p.keyword("not")) {
		auto var = p.nextWord();
		return CommandPtr(new Not(Common::move(var)));
	} else if (p.keyword("gosub")) {
		return CommandPtr(new GoSub(p.nextWord()));
	} else if (p.keyword("return")) {
		return CommandPtr{new Return()};
	} else if (p.keyword("end")) {
		return CommandPtr{new EndScript()};
	}
	return {};
};

void Script::Scope::exec(ExecutionContext &ctx) const {
	exec(ctx, 0);
}

void Script::Scope::exec(ExecutionContext &ctx, uint offset) const {
	auto oldScope = ctx.scope;
	if (!ctx.rootScope)
		ctx.rootScope = this;
	ctx.scope = this;
	for (uint i = offset, n = commands.size(); i < n; ++i) {
		if (!ctx.running)
			break;
		commands[i]->exec(ctx);
	}
	ctx.scope = oldScope;
}

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

Script::Script(Common::SeekableReadStream &s) {
	s.seek(0);
	Common::Array<char> text(s.size());
	if (s.read(text.data(), text.size()) != text.size())
		error("script: short read");
	uint lineno = 1;
	uint lineStartOffset = 0;
	auto textSize = text.size();
	while (lineStartOffset < textSize) {
		auto lineStart = text.begin() + lineStartOffset;
		auto lineEnd = Common::find(lineStart, text.end(), '\n');
		lineStartOffset += Common::distance(lineStart, lineEnd) + 1;
		while (lineEnd > lineStart && Common::isSpace(lineEnd[-1]))
			--lineEnd;
		parseLine({lineStart, lineEnd}, lineno++);
	}
}

void Script::parseLine(const Common::String &line, uint lineno) {
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
			_pluginScope.reset(new Script::Scope);
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
				auto cmd = createCommand(name, args, lineno);
				if (cmd)
					_pluginScope->commands.push_back(Common::move(cmd));
				else
					error("unhandled plugin command %s at line %d", line.c_str(), lineno);
			} else {
				auto cmd = parseCommand(p);
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

int Script::Command::valueOf(const Common::String &value) {
	if (!value.empty() && (Common::isDigit(value[0]) || value[0] == '-' || value[0] == '+'))
		return atoi(value.c_str());
	return g_engine->getVariable(value);
}

Script::~Script() {
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
