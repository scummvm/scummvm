#ifndef PHOENIXVR_COMMANDS_H
#define PHOENIXVR_COMMANDS_H

#include "common/debug.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "phoenixvr/phoenixvr.h"
#include "phoenixvr/script.h"

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
	float seconds;

	While(const Common::Array<Common::String> &args) : seconds(atof(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("while %g", seconds);
	}
};

struct StartTimer : public Script::Command {
	float seconds;

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

struct IfAnd : public Script::Conditional {
	using Script::Conditional::Conditional;
	void exec(Script::ExecutionContext &ctx) const override {
		debug("ifand");
	}
};

struct IfOr : public Script::Conditional {
	using Script::Conditional::Conditional;
	void exec(Script::ExecutionContext &ctx) const override {
		debug("ifor");
	}
};

struct Set : public Script::Command {
	Common::String name;
	int value;

	Set(Common::String n, int v) : name(Common::move(n)), value(v) {}
	void exec(Script::ExecutionContext &ctx) const override {
		ctx.engine->setVariable(name, value);
	}
};

struct End : public Script::Command {
	End() {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("end");
		ctx.running = false;
	}
};

struct Return : public Script::Command {
	Return() {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("return");
		ctx.running = false;
	}
};

struct SetCursorDefault : public Script::Command {
	int idx;
	Common::String fname;
	SetCursorDefault(int i, Common::String f) : idx(i), fname(Common::move(f)) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("setcursordefault %d, %s", idx, fname.c_str());
	}
};

struct SetCursor : public Script::Command {
	Common::String fname;
	Common::String warp;
	int idx;
	SetCursor(Common::String f, Common::String w, int i) : fname(Common::move(f)), warp(Common::move(w)), idx(i) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("setcursor %s %s:%d", fname.c_str(), warp.c_str(), idx);
	}
};

struct HideCursor : public Script::Command {
	Common::String warp;
	int idx;
	HideCursor(Common::String w, int i) : warp(Common::move(w)), idx(i) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("setcursor %s:%d", warp.c_str(), idx);
	}
};

struct ResetLockKey : public Script::Command {
	void exec(Script::ExecutionContext &ctx) const override {
		debug("resetlockkey");
	}
};

struct LockKey : public Script::Command {
	int idx;
	Common::String warp;

	LockKey(int i, Common::String w) : idx(i), warp(Common::move(w)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("lock key F%d: %s", idx, warp.c_str());
	}
};

struct Zoom : public Script::Command {
	int zoom;
	Zoom(int z) : zoom(z) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("zoom %d", zoom);
	}
};

struct AngleXMax : public Script::Command {
	float xMax;
	AngleXMax(float x) : xMax(x) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("angle x max %g", xMax);
	}
};

struct AngleYMax : public Script::Command {
	float yMax0, yMax1;
	AngleYMax(float y0, float y1) : yMax0(y0), yMax1(y1) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("angle y max %g %g", yMax0, yMax1);
	}
};

struct GoToWarp : public Script::Command {
	Common::String warp;
	GoToWarp(Common::String w) : warp(Common::move(w)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		ctx.engine->goToWarp(warp);
	}
};

struct PlaySound : public Script::Command {
	Common::String sound;
	int volume;
	int unk;

	PlaySound(Common::String s, int v, int u) : sound(Common::move(s)), volume(v), unk(u) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("play sound %s %d %d", sound.c_str(), volume, unk);
	}
};

struct StopSound : public Script::Command {
	Common::String sound;

	StopSound(Common::String s) : sound(Common::move(s)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("stop sound %s", sound.c_str());
	}
};

struct PlaySound3D : public Script::Command {
	Common::String sound;
	int volume;
	float angle;
	int unk;

	PlaySound3D(Common::String s, int v, float a, int u) : sound(Common::move(s)), volume(v), angle(a), unk(u) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("play sound %s %d %g %d", sound.c_str(), volume, angle, unk);
	}
};

struct StopSound3D : public Script::Command {
	Common::String sound;

	StopSound3D(Common::String s) : sound(Common::move(s)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("stop sound 3d %s", sound.c_str());
	}
};

struct Fade : public Script::Command {
	int arg0, arg1, arg2;

	Fade(int a0, int a1, int a2) : arg0(a0), arg1(a1), arg2(a2) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("fade %d %d %d", arg0, arg1, arg2);
	}
};

} // namespace
} // namespace PhoenixVR

#endif
