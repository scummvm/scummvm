#include "phoenixvr/script.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "phoenixvr/commands.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

namespace {

class Parser {
	const Common::String &_line;
	uint _lineno;
	uint _pos;

public:
	Parser(const Common::String &line, uint lineno) : _line(line), _lineno(lineno), _pos(0) {
		skip();
	}

	void skip() {
		while (_pos < _line.size() && Common::isSpace(_line[_pos]))
			++_pos;
		if (_pos < _line.size() && _line[_pos] == ';')
			_pos = _line.size();
	}

	bool atEnd() const { return _pos >= _line.size(); }

	int peek() const { return _pos < _line.size() ? _line[_pos] : 0; }
	int next() { return _pos < _line.size() ? _line[_pos++] : 0; }

	void expect(int expected) {
		skip();
		auto ch = next();
		if (ch != expected)
			error("expected '%c' at line %u, got %c in %s", expected, _lineno, ch, _line.c_str());
		skip();
	}

	bool maybe(char ch) {
		skip();
		if (peek() == ch) {
			next();
			return true;
		} else
			return false;
	}

	bool peek(const Common::String &prefix) {
		skip();
		return scumm_strnicmp(_line.c_str() + _pos, prefix.c_str(), prefix.size()) == 0;
	}

	bool maybe(const Common::String &prefix) {
		if (peek(prefix)) {
			_pos += prefix.size();
			skip();
			return true;
		}
		return false;
	}

	bool keyword(const Common::String &prefix) {
		skip();
		bool yes = peek(prefix);
		// keywords ends either on non-alpha or eof
		if (yes) {
			auto end = _pos + prefix.size();
			if (end >= _line.size() || !Common::isAlpha(_line[end])) {
				_pos += prefix.size();
				skip();
				return true;
			}
		}
		return false;
	}

	Common::String nextArg() {
		skip();
		auto begin = _pos;
		while (_pos < _line.size() && !Common::isSpace(_line[_pos]) && _line[_pos] != ',' && _line[_pos] != ')')
			++_pos;
		auto end = _pos;
		skip();
		return _line.substr(begin, end - begin);
	}

	int nextInt() {
		bool negative = false;
		int value = 0;
		if (maybe('-'))
			negative = true;
		do {
			auto ch = next();
			if (ch < '0' || ch > '9')
				error("expected digit at %d, line: %s", _pos, _line.c_str());
			value = value * 10 + (ch - '0');
		} while (Common::isDigit(peek()));
		return negative ? -value : value;
	}

	Common::String nextWord() {
		skip();
		auto begin = _pos;
		while (_pos < _line.size() && !Common::isSpace(_line[_pos]) && _line[_pos] != ',' && _line[_pos] != '(' && _line[_pos] != '=' && _line[_pos] != ')')
			++_pos;
		auto end = _pos;
		skip();
		return _line.substr(begin, end - begin);
	}

	Common::String nextString() {
		skip();
		expect('"');
		Common::String str;
		while (_pos < _line.size()) {
			auto ch = _line[_pos++];
			if (ch == '"')
				break;
			str += ch;
		}
		skip();
		return str;
	}

	Common::Array<Common::String> readStringList() {
		Common::Array<Common::String> list;
		while (!atEnd() && peek() != ')') {
			if (peek() == '"')
				list.push_back(nextString());
			else {
				list.push_back(nextArg());
			}
			if (peek() == ',')
				next();
		}
		return list;
	}

	static float toAngle(int a) {
		static const float angleToFloat = M_PI / 4096.0f;
		return angleToFloat * a;
	}

	Script::CommandPtr parseCommand() {
		using CommandPtr = Script::CommandPtr;
		if (keyword("setcursordefault")) {
			auto idx = nextInt();
			expect(',');
			return CommandPtr(new SetCursorDefault(idx, nextWord()));
		} else if (keyword("lockkey")) {
			auto idx = nextInt();
			expect(',');
			auto fname = nextWord();
			return CommandPtr(new LockKey(idx, Common::move(fname)));
		} else if (keyword("resetlockkey")) {
			return CommandPtr(new ResetLockKey());
		} else if (maybe("fade=")) {
			auto arg0 = nextInt();
			expect(',');
			auto arg1 = nextInt();
			expect(',');
			auto arg2 = nextInt();
			return CommandPtr(new Fade(arg0, arg1, arg2));
		} else if (maybe("setzoom=")) {
			return CommandPtr(new SetZoom(nextInt()));
		} else if (maybe("setangle=")) {
			auto i0 = nextInt();
			if (i0 > 4095)
				i0 -= 8192;
			auto a0 = toAngle(i0);
			expect(',');
			auto a1 = toAngle(nextInt());
			return CommandPtr(new SetAngle(a0, a1));
		} else if (maybe("anglexmax=")) {
			return CommandPtr(new AngleXMax(toAngle(nextInt())));
		} else if (maybe("angleymax=")) {
			auto y0 = toAngle(nextInt());
			expect(',');
			auto y1 = toAngle(nextInt());
			return CommandPtr(new AngleYMax(y0, y1));
		} else if (keyword("gotowarp")) {
			return CommandPtr(new GoToWarp(nextWord()));
		} else if (keyword("playsound3d")) {
			auto sound = nextWord();
			expect(',');
			auto arg0 = nextInt();
			expect(',');
			auto arg1 = nextInt();
			expect(',');
			auto arg2 = nextInt();
			return CommandPtr(new PlaySound3D(Common::move(sound), arg0, toAngle(arg1), arg2));
		} else if (keyword("playsound")) {
			auto sound = nextWord();
			expect(',');
			auto arg0 = nextInt();
			expect(',');
			auto arg1 = nextInt();
			return CommandPtr(new PlaySound(Common::move(sound), arg0, arg1));
		} else if (keyword("stopsound3d")) {
			return CommandPtr(new StopSound3D(nextWord()));
		} else if (keyword("stopsound")) {
			return CommandPtr(new StopSound(nextWord()));
		} else if (keyword("setcursor")) {
			auto image = nextWord();
			expect(',');
			auto warp = nextWord();
			expect(',');
			auto idx = nextInt();
			return CommandPtr(new SetCursor(Common::move(image), Common::move(warp), idx));
		} else if (keyword("hidecursor")) {
			auto warp = nextWord();
			expect(',');
			auto idx = nextInt();
			return CommandPtr(new HideCursor(Common::move(warp), idx));
		} else if (keyword("set")) {
			auto var = nextWord();
			expect('=');
			auto value = nextInt();
			return CommandPtr(new Set(Common::move(var), value));
		} else if (keyword("return")) {
			return CommandPtr{new Return()};
		} else if (keyword("end")) {
			return CommandPtr{new End()};
		}
		return {};
	};
};

} // namespace

void Script::Scope::exec(ExecutionContext &ctx) const {
	for (auto &cmd : commands) {
		if (!ctx.running)
			break;
		cmd->exec(ctx);
	}
}

Script::TestPtr Script::Warp::getTest(int idx) const {
	auto it = Common::find_if(tests.begin(), tests.end(), [&](const TestPtr &test) { return test->idx == idx; });
	return it != tests.end() ? *it : Script::TestPtr{};
}

Script::Script(Common::SeekableReadStream &s) {
	uint lineno = 1;
	while (!s.eos()) {
		auto line = s.readLine();
		parseLine(line, lineno++);
	}
}

void Script::parseLine(const Common::String &line, uint lineno) {
	if (line.empty())
		return;

	Parser p(line, lineno);
	if (p.atEnd())
		return;

	switch (p.peek()) {
	case '[': {
		p.next();
		if (p.maybe("bool]=")) {
			g_engine->declareVariable(p.nextWord());
		} else if (p.maybe("warp]=")) {
			auto vr = p.nextWord();
			p.expect(',');
			auto test = p.nextWord();
			_currentWarp.reset(new Warp{vr, test, {}});
			_warpsIndex[vr] = _warps.size();
			_warps.push_back(_currentWarp);
			_warpNames.push_back(vr);
		} else if (p.maybe("test]=")) {
			if (!_currentWarp)
				error("test without warp");
			auto idx = p.nextInt();
			if (!_currentWarp)
				error("text must have parent wrap section");
			_currentTest.reset(new Test{idx, {}});
			_currentWarp->tests.push_back(_currentTest);
		} else {
			error("invalid [] directive on line %u: %s", lineno, line.c_str());
		}
		break;
	}
	default:
		if (_currentTest) {
			auto &commands = _currentTest->scope.commands;
			if (p.maybe("ifand=")) {
				if (_pluginScope)
					error("ifand in plugin scope");
				_conditional.reset(new IfAnd(p.readStringList()));
			} else if (p.maybe("ifor=")) {
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
			} else {
				if (_pluginScope) {
					auto name = p.nextWord();
					p.expect('(');
					auto args = p.readStringList();
					p.expect(')');
					auto cmd = createCommand(name, args);
					if (cmd) {
						if (_conditional) {
							_conditional->target = Common::move(cmd);
							commands.push_back(Common::move(_conditional));
							_conditional.reset();
						} else
							_pluginScope->commands.push_back(Common::move(cmd));
					}
				} else {
					auto cmd = p.parseCommand();
					if (cmd) {
						if (_conditional) {
							_conditional->target = Common::move(cmd);
							commands.push_back(Common::move(_conditional));
							_conditional.reset();
						} else
							commands.push_back(Common::move(cmd));
					} else
						error("unhandled script command %s", line.c_str());
				}
			}
		} else
			error("invalid directive on line %u: %s", lineno, line.c_str());
	}
}

Script::~Script() {
}

int Script::getWarp(const Common::String &name) const {
	return _warpsIndex.getVal(name);
}

Script::ConstWarpPtr Script::getWarp(int idx) const {
	return idx >= 0 && idx < static_cast<int>(_warps.size()) ? Script::ConstWarpPtr{_warps[idx]} : Script::ConstWarpPtr{};
}

Script::ConstWarpPtr Script::getInitScript() const {
	return _warps.front();
}

} // namespace PhoenixVR
