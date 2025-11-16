#include "phoenixvr/script.h"
#include "common/debug.h"
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

	bool maybe(const Common::String &prefix) {
		skip();
		bool yes = scumm_strnicmp(_line.c_str() + _pos, prefix.c_str(), prefix.size()) == 0;
		if (yes) {
			_pos += prefix.size();
			skip();
		}
		return yes;
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

	Script::CommandPtr parseCommand() {
		using CommandPtr = Script::CommandPtr;
		if (maybe("setcursordefault")) {
			auto idx = nextInt();
			expect(',');
			return CommandPtr(new SetCursorDefault(idx, nextWord()));
		} else if (maybe("lockkey")) {
			auto idx = nextInt();
			expect(',');
			auto fname = nextWord();
			return CommandPtr(new LockKey(idx, Common::move(fname)));
		} else if (maybe("resetlockkey")) {
			return CommandPtr(new ResetLockKey());
		} else if (maybe("fade=")) {
			auto arg0 = nextInt();
			expect(',');
			auto arg1 = nextInt();
			expect(',');
			auto arg2 = nextInt();
			return CommandPtr(new Fade(arg0, arg1, arg2));
		} else if (maybe("setzoom=")) {
			return CommandPtr(new Zoom(nextInt()));
		} else if (maybe("anglexmax=")) {
			return CommandPtr(new AngleXMax(nextInt() / 1024.0f));
		} else if (maybe("angleymax=")) {
			auto y0 = nextInt() / 1024.0f;
			expect(',');
			auto y1 = nextInt() / 1024.0f;
			return CommandPtr(new AngleYMax(y0, y1));
		} else if (maybe("gotowarp")) {
			return CommandPtr(new GoToWarp(nextWord()));
		} else if (maybe("playsound3d")) {
			auto sound = nextWord();
			expect(',');
			auto arg0 = nextInt();
			expect(',');
			auto arg1 = nextInt();
			expect(',');
			auto arg2 = nextInt();
			return CommandPtr(new PlaySound3D(Common::move(sound), arg0, arg1 / 1024.0f, arg2));
		} else if (maybe("playsound")) {
			auto sound = nextWord();
			expect(',');
			auto arg0 = nextInt();
			expect(',');
			auto arg1 = nextInt();
			return CommandPtr(new PlaySound(Common::move(sound), arg0, arg1));
		} else if (maybe("stopsound3d")) {
			return CommandPtr(new StopSound3D(nextWord()));
		} else if (maybe("stopsound")) {
			return CommandPtr(new StopSound(nextWord()));
		} else if (maybe("setcursor")) {
			auto image = nextWord();
			expect(',');
			auto warp = nextWord();
			expect(',');
			auto idx = nextInt();
			return CommandPtr(new SetCursor(Common::move(image), Common::move(warp), idx));
		} else if (maybe("hidecursor")) {
			auto warp = nextWord();
			expect(',');
			auto idx = nextInt();
			return CommandPtr(new HideCursor(Common::move(warp), idx));
		} else if (maybe("set")) {
			auto var = nextWord();
			expect('=');
			auto value = nextInt();
			return CommandPtr(new Set(Common::move(var), value));
		} else if (maybe("return")) {
			return CommandPtr{new Return()};
		} else if (maybe("end")) {
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

void Script::Warp::setText(int idx, const TestPtr &text) {
	if (idx < -1)
		error("test id must be >= -1");
	uint realIdx = idx + 1;
	if (realIdx + 1 > tests.size())
		tests.resize(realIdx + 1);
	tests[realIdx] = text;
}

const Script::TestPtr &Script::Warp::getTest(int idx) const {
	return tests[idx + 1];
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
			auto name = p.nextWord();
			// FIXME: pass engine here?
			g_engine->declareVariable(name);
		} else if (p.maybe("warp]=")) {
			auto vr = p.nextWord();
			p.expect(',');
			auto test = p.nextWord();
			_currentWarp.reset(new Warp{vr, test, {}});
			_warpsIndex[vr] = _warps.size();
			_warps.push_back(_currentWarp);
		} else if (p.maybe("test]=")) {
			if (!_currentWarp)
				error("test without warp");
			auto idx = p.nextInt();
			if (!_currentWarp)
				error("text must have parent wrap section");
			_currentTest.reset(new Test{idx, {}});
			_currentWarp->setText(idx, _currentTest);
		} else {
			error("invalid [] directive on line %u: %s", lineno, line.c_str());
		}
		break;
	}
	default:
		if (_currentTest) {
			auto &commands = _currentTest->scope.commands;
			if (p.maybe("ifand=")) {
				_conditional.reset(new IfAnd(p.readStringList()));
			} else if (p.maybe("ifor=")) {
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
					if (cmd)
						commands.push_back(Common::move(cmd));
					else
						error("unhandled script command %s", line.c_str());
				}
			}
		} else
			error("invalid directive on line %u: %s", lineno, line.c_str());
	}
}

Script::~Script() {
}

void Script::exec(ExecutionContext &ctx) const {
	auto &warp = _warps.front();
	debug("execute warp script %s", warp->vrFile.c_str());
	auto &test = warp->getDefaultTest();
	test->scope.exec(ctx);
}

} // namespace PhoenixVR
