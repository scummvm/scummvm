#include "phoenixvr/script.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/textconsole.h"

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
			error("expected '%c' at line %u, got %c", expected, _lineno, ch);
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

	Common::String nextWord() {
		skip();
		auto begin = _pos;
		while (_pos < _line.size() && !Common::isSpace(_line[_pos]) && _line[_pos] != ',' && _line[_pos] != '(')
			++_pos;
		auto end = _pos;
		skip();
		return _line.substr(begin, end - begin);
	}

	Common::String readString() {
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
			list.push_back(readString());
		} while (peek() == ',');
		return list;
	}
};

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
	}
};

struct End : public Script::Command {
	End() {}
	void exec(Script::ExecutionContext &ctx) const override {
		ctx.running = false;
	}
};

#define PLUGIN_LIST(E)               \
	E(MultiCD_Set_Transition_Script) \
	E(MultiCD_Set_Next_Script)
} // namespace

void Script::Warp::setText(int idx, const TestPtr &text) {
	if (idx < -1)
		error("test id must be >= -1");
	uint realIdx = idx + 1;
	if (realIdx + 1 > tests.size())
		tests.resize(realIdx + 1);
	tests[realIdx] = text;
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

	Parser p(line, lineno);
	if (p.atEnd())
		return;

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
			if (p.maybe("end")) {
				commands.emplace_back(new End());
			} else if (p.maybe("plugin")) {
				if (_pluginContext)
					error("nested plugin context is not allowed");
				_pluginContext = true;
			} else if (p.maybe("endplugin")) {
				if (!_pluginContext)
					error("endplugin without plugin");
				_pluginContext = false;
			} else {
				if (_pluginContext) {
					auto cmd = p.nextWord();
					p.expect('(');
					auto args = p.readStringList();
					p.expect(')');
#define ADD_PLUGIN(NAME)                       \
	if (cmd.equalsIgnoreCase(#NAME)) {         \
		commands.emplace_back(new NAME(args)); \
	} else

					PLUGIN_LIST(ADD_PLUGIN)
					error("unimplemented plugin command %s", cmd.c_str());
				} else
					error("unhandled script command %s, at line %u", line.c_str(), lineno);
			}

		} else
			error("invalid directive on line %u: %s\n", lineno, line.c_str());
	}
}

Script::~Script() {
}

} // namespace PhoenixVR
