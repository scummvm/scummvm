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
	void expect(int ch) {
		skip();
		if (_pos >= _line.size() || _line[_pos] != ch)
			error("expected '%c' at line %u", ch, _lineno);
		++_pos;
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
		while (_pos < _line.size() && !Common::isSpace(_line[_pos]) && _line[_pos] != ',')
			++_pos;
		auto end = _pos;
		skip();
		return _line.substr(begin, end - begin);
	}
};
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
			auto test = p.nextWord();
			debug("got warp %s %s", vr.c_str(), test.c_str());
			_currentWarp.reset(new Warp{vr, test, {}});
			_warpsIndex[vr] = _warps.size();
			_warps.push_back(_currentWarp);
		} else if (p.maybe("test]=")) {
			if (!_currentWarp)
				error("test without warp");
			auto word = p.nextWord();
			debug("test %s\n", word.c_str());
			auto idx = std::atoi(word.c_str());
			if (!_currentWarp)
				error("text must have parent wrap section");
			_currentTest.reset(new Test{idx});
			_currentWarp->setText(idx, _currentTest);
		} else {
			error("invalid [] directive on line %u: %s", lineno, line.c_str());
		}
		break;
	}
	default:
		if (_currentTest) {
			if (p.maybe("plugin")) {
				if (_pluginContext)
					error("nested plugin context is not allowed");
				_pluginContext = true;
			} else if (p.maybe("endplugin")) {
				if (!_pluginContext)
					error("endplugin without plugin");
				_pluginContext = false;
			} else {
				error("unhandled script command %s, at line %u", line.c_str(), lineno);
			}

		} else
			error("invalid directive on line %u: %s\n", lineno, line.c_str());
	}
}

Script::~Script() {
}

} // namespace PhoenixVR
