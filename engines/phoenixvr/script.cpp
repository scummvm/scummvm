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

struct Play_AnimBloc : public Script::Command {
	Common::String name;
	Common::String block;
	int start;
	int stop;

	Play_AnimBloc(const Common::Array<Common::String> &args) : name(args[0]), block(args[1]), start(atoi(args[2].c_str())), stop(atoi(args[3].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("Play_AnimBloc %s %s %d-%d", name.c_str(), block.c_str(), start, stop);
	}
};

struct Play_AnimBloc_Number : public Script::Command {
	Common::String name1, name2;
	Common::String block;
	int start;
	int stop;

	Play_AnimBloc_Number(const Common::Array<Common::String> &args) : name1(args[0]), name2(args[1]),
																	  block(args[2]), start(atoi(args[3].c_str())), stop(atoi(args[4].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("Play_AnimBloc_Number %s %s %s %d-%d", name1.c_str(), name2.c_str(), block.c_str(), start, stop);
	}
};

struct Until : public Script::Command {
	Common::String block;
	int frame;

	Until(const Common::Array<Common::String> &args) : block(args[0]), frame(atoi(args[1].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("until %s %d", block.c_str(), frame);
	}
};

struct While : public Script::Command {
	double seconds;

	While(const Common::Array<Common::String> &args) : seconds(atof(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("while %g", seconds);
	}
};

struct StartTimer : public Script::Command {
	double seconds;

	StartTimer(const Common::Array<Common::String> &args) : seconds(atof(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("starttimer %g", seconds);
	}
};

struct PauseTimer : public Script::Command {
	int arg1, arg2;

	PauseTimer(const Common::Array<Common::String> &args) : arg1(atoi(args[0].c_str())), arg2(atoi(args[1].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("pause_timer %d %d", arg1, arg2);
	}
};

struct KillTimer : public Script::Command {
	KillTimer(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("killtimer");
	}
};

struct ChangeCurseur : public Script::Command {
	int cursor;
	ChangeCurseur(const Common::Array<Common::String> &args) : cursor(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("changecurseur %d", cursor);
	}
};

struct Add : public Script::Command {
	Common::String dstVar;
	Common::String srcVar;
	int addend;

	Add(const Common::Array<Common::String> &args) : dstVar(args[0]), srcVar(args[1]), addend(atoi(args[2].c_str())) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("add %s %s %d", dstVar.c_str(), srcVar.c_str(), addend);
	}
};

struct Sub : public Script::Command {
	Common::String dstVar;
	Common::String srcVar;
	int addend;

	Sub(const Common::Array<Common::String> &args) : dstVar(args[0]), srcVar(args[1]), addend(atoi(args[2].c_str())) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("sub %s %s %d", dstVar.c_str(), srcVar.c_str(), addend);
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

struct LoadSave_Init_Slots : public Script::Command {
	int slots;

	LoadSave_Init_Slots(const Common::Array<Common::String> &args) : slots(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Init_Slots %d", slots);
	}
};

struct LoadSave_Draw_Slot : public Script::Command {
	int slot;
	int arg0;
	int arg1;
	int arg2;

	LoadSave_Draw_Slot(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())),
																	arg0(atoi(args[1].c_str())),
																	arg1(atoi(args[2].c_str())),
																	arg2(atoi(args[3].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Draw_Slot %d %d %d %d", slot, arg0, arg1, arg2);
	}
};

struct LoadSave_Test_Slot : public Script::Command {
	int slot;
	Common::String show;
	Common::String hide;

	LoadSave_Test_Slot(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())), show(args[1]), hide(args[2]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Test_Slot %d %s %s", slot, show.c_str(), hide.c_str());
	}
};

struct LoadSave_Capture_Context : public Script::Command {
	LoadSave_Capture_Context(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Capture_Context");
	}
};

struct LoadSave_Context_Restored : public Script::Command {
	Common::String progress;
	Common::String done;

	LoadSave_Context_Restored(const Common::Array<Common::String> &args) : progress(args[0]), done(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Context_Restored %s %s", progress.c_str(), done.c_str());
	}
};

struct LoadSave_Load : public Script::Command {
	int slot;

	LoadSave_Load(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Load %d", slot);
	}
};

struct LoadSave_Save : public Script::Command {
	int slot;

	LoadSave_Save(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Save %d", slot);
	}
};

struct LoadSave_Set_Context_Label : public Script::Command {
	Common::String label;

	LoadSave_Set_Context_Label(const Common::Array<Common::String> &args) : label(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Set_Context_Label %s", label.c_str());
	}
};

struct Branch : public Script::Command {
	Common::Array<Common::String> vars;
	Script::CommandPtr target;
	Branch(const Common::Array<Common::String> &args) : vars(args) {}
};

struct RolloverMalette : public Script::Command {
	int arg;

	RolloverMalette(const Common::Array<Common::String> &args) : arg(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("RolloverMalette %d", arg);
	}
};

struct RolloverSecretaire : public Script::Command {
	int arg;

	RolloverSecretaire(const Common::Array<Common::String> &args) : arg(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("RolloverSecretaire %d", arg);
	}
};

struct End : public Script::Command {
	End() {}
	void exec(Script::ExecutionContext &ctx) const override {
		ctx.running = false;
	}
};

#define PLUGIN_LIST(E)               \
	E(Add)                           \
	E(ChangeCurseur)                 \
	E(Cmp)                           \
	E(KillTimer)                     \
	E(LoadSave_Capture_Context)      \
	E(LoadSave_Context_Restored)     \
	E(LoadSave_Enter_Script)         \
	E(LoadSave_Init_Slots)           \
	E(LoadSave_Load)                 \
	E(LoadSave_Save)                 \
	E(LoadSave_Set_Context_Label)    \
	E(LoadSave_Draw_Slot)            \
	E(LoadSave_Test_Slot)            \
	E(MultiCD_Set_Transition_Script) \
	E(MultiCD_Set_Next_Script)       \
	E(PauseTimer)                    \
	E(Play_AnimBloc)                 \
	E(Play_AnimBloc_Number)          \
	E(Play_Movie)                    \
	E(RolloverMalette)               \
	E(RolloverSecretaire)            \
	E(StartTimer)                    \
	E(Sub)                           \
	E(Until)                         \
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

public:
	Parser(const Common::String &line, uint lineno, bool &pluginContext) : _line(line), _lineno(lineno), _pos(0) {
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
		if (maybe("end")) {
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
			debug("setzoom %d", zoom);
		} else if (maybe("anglexmax=")) {
			auto xmax = atoi(nextWord().c_str());
			debug("anglexmax %d", xmax);
		} else if (maybe("ifand=")) {
			auto var = nextWord();
			debug("ifand %s", var.c_str());
		} else if (maybe("ifor=")) {
			auto var = nextWord();
			debug("ifor %s", var.c_str());
		} else if (maybe("gotowarp")) {
			auto id = nextWord();
			debug("gotowarp %s", id.c_str());
		} else if (maybe("playsound3d")) {
			auto sound = nextWord();
			expect(',');
			auto arg0 = nextWord();
			expect(',');
			auto arg1 = nextWord();
			expect(',');
			auto arg2 = nextWord();
			debug("playsound3d %s %s %s %s", sound.c_str(), arg0.c_str(), arg1.c_str(), arg2.c_str());
		} else if (maybe("playsound")) {
			auto sound = nextWord();
			expect(',');
			auto arg0 = nextWord();
			expect(',');
			auto arg1 = nextWord();
			debug("playsound %s %s %s", sound.c_str(), arg0.c_str(), arg1.c_str());
		} else if (maybe("stopsound3d")) {
			auto sound = nextWord();
			debug("stopsound3d %s", sound.c_str());
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

	// debug("line %u: %s", lineno, line.c_str());

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
			if (p.maybe("plugin")) {
				if (_pluginContext)
					error("nested plugin context is not allowed, line: %u", lineno);
				_pluginContext = true;
			} else if (p.maybe("endplugin")) {
				if (!_pluginContext)
					error("endplugin without plugin");
				_pluginContext = false;
			} else {
				auto &commands = _currentTest->scope.commands;
				if (_pluginContext) {
					auto name = p.nextWord();
					p.expect('(');
					auto args = p.readStringList();
					p.expect(')');
					auto cmd = createCommand(name, args);
					if (cmd)
						commands.push_back(Common::move(cmd));
				} else {
					auto cmd = p.parseCommand();
					if (cmd)
						commands.push_back(Common::move(cmd));
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
	debug("warp %s", warp->vrFile.c_str());
	auto &test = warp->getDefaultTest();
	test->scope.exec(ctx);
}

} // namespace PhoenixVR
