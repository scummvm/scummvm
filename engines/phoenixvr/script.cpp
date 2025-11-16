#include "phoenixvr/script.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

namespace {
struct MultiCD_Set_Transition_Script : public Script::Command {
	Common::String path;

	MultiCD_Set_Transition_Script(const Common::Array<Common::String> &args) : path(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("MultiCD_Set_Transition_Script %s", path.c_str());
	}
};

struct MultiCD_Set_Next_Script : public Script::Command {
	Common::String filename;

	MultiCD_Set_Next_Script(const Common::Array<Common::String> &args) : filename(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("MultiCD_Set_Next_Script %s", filename.c_str());
		ctx.engine->setNextScript(filename);
	}
};

struct LoadSave_Enter_Script : public Script::Command {
	Common::String reloading, notReloading;

	LoadSave_Enter_Script(const Common::Array<Common::String> &args) : reloading(args[0]), notReloading(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Enter_Script %s, %s", reloading.c_str(), notReloading.c_str());
	}
};

struct Play_Movie : public Script::Command {
	Common::String filename;

	Play_Movie(const Common::Array<Common::String> &args) : filename(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("Play_Movie %s", filename.c_str());
	}
};

struct While : public Script::Command {
	double seconds;

	While(const Common::Array<Common::String> &args) : seconds(atof(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("while %g", seconds);
	}
};

struct Cmp : public Script::Command {
	Common::String var;
	Common::String negativeVar;
	Common::String arg0;
	Common::String op;
	Common::String arg1;

	Cmp(const Common::Array<Common::String> &args) : var(args[0]), negativeVar(args[1]),
													 arg0(args[2]), op(args[3]), arg1(args[4]) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("cmp");
	}
};

struct End : public Script::Command {
	End() {}
	void exec(Script::ExecutionContext &ctx) const override {
		ctx.running = false;
	}
};

#define PLUGIN_LIST(E)               \
	E(Cmp)                           \
	E(LoadSave_Enter_Script)         \
	E(MultiCD_Set_Transition_Script) \
	E(MultiCD_Set_Next_Script)       \
	E(Play_Movie)                    \
	E(While)                         \
	/* */

#define ADD_PLUGIN(NAME)             \
	if (cmd.equalsIgnoreCase(#NAME)) \
		return Script::CommandPtr(new NAME(args));

Script::CommandPtr createCommand(const Common::String &cmd, const Common::Array<Common::String> &args) {
	PLUGIN_LIST(ADD_PLUGIN)
	error("unhandled plugin command %s", cmd.c_str());
}

class Parser {
	const Common::String &_line;
	uint _lineno;
	uint _pos;
	bool &_pluginContext;

public:
	Parser(const Common::String &line, uint lineno, bool &pluginContext) : _line(line), _lineno(lineno), _pos(0), _pluginContext(pluginContext) {
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
		do {
			if (peek() == '"')
				list.push_back(nextString());
			else {
				list.push_back(nextArg());
			}
			if (peek() == ',')
				next();
		} while (peek() != ')');
		return list;
	}

	Script::CommandPtr parseCommand() {
		using CommandPtr = Script::CommandPtr;
		if (maybe("plugin")) {
			if (_pluginContext)
				error("nested plugin context is not allowed, line: %u", _lineno);
			_pluginContext = true;
		} else if (maybe("endplugin")) {
			if (!_pluginContext)
				error("endplugin without plugin");
			_pluginContext = false;
		} else if (maybe("end")) {
			return CommandPtr{new End()};
		} else if (maybe("setcursordefault")) {
			auto idx = atoi(nextWord().c_str());
			expect(',');
			auto fname = nextWord();
			debug("setcursordefault %d: %s", idx, fname.c_str());
		} else if (maybe("lockkey")) {
			auto idx = atoi(nextWord().c_str());
			expect(',');
			auto fname = nextWord();
			debug("lockkey %d: %s", idx, fname.c_str());
		} else if (maybe("resetlockkey")) {
			debug("resetlockkey");
		} else if (maybe("setzoom=")) {
			auto zoom = atoi(nextWord().c_str());
			debug("setzoom %d\n", zoom);
		} else if (maybe("anglexmax=")) {
			auto xmax = atoi(nextWord().c_str());
			debug("anglexmax %d", xmax);
		} else if (maybe("ifand=")) {
			auto var = nextWord();
			debug("ifand %s\n", var.c_str());
		} else if (maybe("gotowarp")) {
			auto id = nextWord();
			debug("gotowarp %s", id.c_str());
		} else if (maybe("playsound")) {
			auto sound = nextWord();
			expect(',');
			auto arg0 = nextWord();
			expect(',');
			auto arg1 = nextWord();
			debug("playsound %s %s %s", sound.c_str(), arg0.c_str(), arg1.c_str());
		} else if (maybe("stopsound")) {
			auto sound = nextWord();
			debug("stopsound %s", sound.c_str());
		} else if (maybe("setcursor")) {
			auto image = nextWord();
			expect(',');
			auto warp = nextWord();
			expect(',');
			auto idx = nextWord();
			debug("setcursor %s %s %s", image.c_str(), warp.c_str(), idx.c_str());
		} else if (maybe("set")) {
			auto var = nextWord();
			expect('=');
			auto value = nextWord();
			debug("set %s = %s", var.c_str(), value.c_str());
		} else {
			if (_pluginContext) {
				auto cmd = nextWord();
				expect('(');
				auto args = readStringList();
				expect(')');
				return createCommand(cmd, args);
			} else
				error("unhandled script command %s, at line %u", _line.c_str(), _lineno);
		}
		return {};
	};
};

} // namespace

void Script::Test::exec(ExecutionContext &ctx) const {
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

Script::Script(Common::SeekableReadStream &s) : _pluginContext(false) {
	uint lineno = 1;
	while (!s.eos()) {
		auto line = s.readLine();
		parseLine(line, lineno++);
	}
}

void Script::parseLine(const Common::String &line, uint lineno) {
	if (line.empty())
		return;

	Parser p(line, lineno, _pluginContext);
	if (p.atEnd())
		return;

	debug("line %s at %u", line.c_str(), lineno);

	switch (p.peek()) {
	case '[': {
		p.next();
		if (p.maybe("bool]=")) {
			auto name = p.nextWord();
			debug("bool flag %s", name.c_str());
		} else if (p.maybe("warp]=")) {
			auto vr = p.nextWord();
			p.expect(',');
			auto test = p.nextWord();
			debug("got warp %s %s", vr.c_str(), test.c_str());
			_currentWarp.reset(new Warp{vr, test, {}});
			_warpsIndex[vr] = _warps.size();
			_warps.push_back(_currentWarp);
		} else if (p.maybe("test]=")) {
			if (!_currentWarp)
				error("test without warp");
			auto word = p.nextWord();
			debug("test %s", word.c_str());
			auto idx = std::atoi(word.c_str());
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
			auto &commands = _currentTest->commands;
			auto cmd = p.parseCommand();
			if (cmd)
				commands.push_back(Common::move(cmd));
		} else
			error("invalid directive on line %u: %s\n", lineno, line.c_str());
	}
}

Script::~Script() {
}

void Script::exec(ExecutionContext &ctx) const {
	for (auto &warp : _warps) {
		if (!ctx.running)
			break;
		debug("warp %s", warp->vrFile.c_str());
		auto &test = warp->getDefaultTest();
		test->exec(ctx);
	}
}

} // namespace PhoenixVR
