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
		g_engine->setNextScript(filename);
	}
};

struct LoadSave_Enter_Script : public Script::Command {
	Common::String reloading, notReloading;

	LoadSave_Enter_Script(const Common::Array<Common::String> &args) : reloading(args[0]), notReloading(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("LoadSave_Enter_Script %s, %s", reloading.c_str(), notReloading.c_str());
	}
};

struct Play_Movie : public Script::Command {
	Common::String filename;

	Play_Movie(const Common::Array<Common::String> &args) : filename(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->playMovie(filename);
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
		g_engine->playAnimation(name, block);
	}
};

struct Play_AnimBloc_Number : public Script::Command {
	Common::String prefix, var;
	Common::String block;
	int start;
	int stop;

	Play_AnimBloc_Number(const Common::Array<Common::String> &args) : prefix(args[0]), var(args[1]),
																	  block(args[2]), start(atoi(args[3].c_str())), stop(atoi(args[4].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("Play_AnimBloc_Number %s %s %s %d-%d", prefix.c_str(), var.c_str(), block.c_str(), start, stop);
		int value = g_engine->getVariable(var);
		auto name = Common::String::format("%s%04d", prefix.c_str(), value);
		g_engine->playAnimation(name, block);
	}
};

struct Until : public Script::Command {
	Common::String block;
	int frame;

	Until(const Common::Array<Common::String> &args) : block(args[0]), frame(atoi(args[1].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("until %s %d", block.c_str(), frame);
	}
};

struct While : public Script::Command {
	float seconds;

	While(const Common::Array<Common::String> &args) : seconds(atof(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("while %g", seconds);
	}
};

struct StartTimer : public Script::Command {
	float seconds;

	StartTimer(const Common::Array<Common::String> &args) : seconds(atof(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("starttimer %g", seconds);
		g_engine->startTimer(seconds);
	}
};

struct PauseTimer : public Script::Command {
	int paused, deactivate;

	PauseTimer(const Common::Array<Common::String> &args) : paused(atoi(args[0].c_str())), deactivate(atoi(args[1].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("pausetimer %d %d", paused, deactivate);
		g_engine->pauseTimer(paused, deactivate);
	}
};

struct KillTimer : public Script::Command {
	KillTimer(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("killtimer");
		g_engine->killTimer();
	}
};

struct ChangeCurseur : public Script::Command {
	int cursor;
	ChangeCurseur(const Common::Array<Common::String> &args) : cursor(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("changecurseur %d", cursor);
		g_engine->executeTest(cursor);
	}
};

struct Add : public Script::Command {
	Common::String dstVar;
	Common::String srcVar;
	int imm;

	Add(const Common::Array<Common::String> &args) : dstVar(args[0]), srcVar(args[1]), imm(atoi(args[2].c_str())) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("add %s %s %d", dstVar.c_str(), srcVar.c_str(), imm);
		g_engine->setVariable(dstVar, g_engine->getVariable(srcVar) + imm);
	}
};

struct Sub : public Script::Command {
	Common::String dstVar;
	Common::String srcVar;
	int imm;

	Sub(const Common::Array<Common::String> &args) : dstVar(args[0]), srcVar(args[1]), imm(atoi(args[2].c_str())) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("sub %s %s %d", dstVar.c_str(), srcVar.c_str(), imm);
		g_engine->setVariable(dstVar, g_engine->getVariable(srcVar) - imm);
	}
};

struct Cmp : public Script::Command {
	Common::String var;
	Common::String negativeVar;
	Common::String arg0;
	Common::String op;
	int arg1;

	Cmp(const Common::Array<Common::String> &args) : var(args[0]), negativeVar(args[1]),
													 arg0(args[2]), op(args[3]), arg1(atoi(args[4].c_str())) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("cmp %s %s %s %s %d", var.c_str(), negativeVar.c_str(), arg0.c_str(), op.c_str(), arg1);
		if (op == "==") {
			auto value0 = g_engine->getVariable(arg0);
			bool r = value0 == arg1;
			g_engine->setVariable(var, r);
			g_engine->setVariable(negativeVar, !r);
		} else {
			error("invalid cmp op %s", op.c_str());
		}
	}
};

struct LoadSave_Init_Slots : public Script::Command {
	int slots;

	LoadSave_Init_Slots(const Common::Array<Common::String> &args) : slots(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("LoadSave_Init_Slots %d", slots);
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
		warning("LoadSave_Draw_Slot %d %d %d %d", slot, arg0, arg1, arg2);
	}
};

struct LoadSave_Test_Slot : public Script::Command {
	int slot;
	Common::String show;
	Common::String hide;

	LoadSave_Test_Slot(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())), show(args[1]), hide(args[2]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("LoadSave_Test_Slot %d %s %s", slot, show.c_str(), hide.c_str());
		g_engine->setVariable(show, 0);
		g_engine->setVariable(hide, 1);
	}
};

struct LoadSave_Capture_Context : public Script::Command {
	LoadSave_Capture_Context(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("LoadSave_Capture_Context");
	}
};

struct LoadSave_Context_Restored : public Script::Command {
	Common::String progress;
	Common::String done;

	LoadSave_Context_Restored(const Common::Array<Common::String> &args) : progress(args[0]), done(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("LoadSave_Context_Restored %s %s", progress.c_str(), done.c_str());
	}
};

struct LoadSave_Load : public Script::Command {
	int slot;

	LoadSave_Load(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("LoadSave_Load %d", slot);
	}
};

struct LoadSave_Save : public Script::Command {
	int slot;

	LoadSave_Save(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("LoadSave_Save %d", slot);
	}
};

struct LoadSave_Set_Context_Label : public Script::Command {
	Common::String label;

	LoadSave_Set_Context_Label(const Common::Array<Common::String> &args) : label(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("LoadSave_Set_Context_Label %s", label.c_str());
	}
};

struct RolloverMalette : public Script::Command {
	int arg;

	RolloverMalette(const Common::Array<Common::String> &args) : arg(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("RolloverMalette %d", arg);
	}
};

struct RolloverSecretaire : public Script::Command {
	int arg;

	RolloverSecretaire(const Common::Array<Common::String> &args) : arg(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("RolloverSecretaire %d", arg);
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
		bool result = true;
		for (auto &var : vars) {
			auto value = g_engine->getVariable(var);
			debug("ifand, %s: %d", var.c_str(), value);
			if (!value)
				result = false;
		}
		if (!result)
			return;
		debug("ifand: executing conditional block");
		target->exec(ctx);
	}
};

struct IfOr : public Script::Conditional {
	using Script::Conditional::Conditional;
	void exec(Script::ExecutionContext &ctx) const override {
		bool result = false;
		for (auto &var : vars) {
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

struct Set : public Script::Command {
	Common::String name;
	int value;

	Set(Common::String n, int v) : name(Common::move(n)), value(v) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setVariable(name, value);
	}
};

struct End : public Script::Command {
	End() {}
	void exec(Script::ExecutionContext &ctx) const override {
		ctx.running = false;
		g_engine->end();
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
		g_engine->setCursorDefault(idx, fname);
	}
};

struct SetCursor : public Script::Command {
	Common::String fname;
	Common::String wname;
	int idx;
	SetCursor(Common::String f, Common::String w, int i) : fname(Common::move(f)), wname(Common::move(w)), idx(i) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setCursor(fname, wname, idx);
	}
};

struct HideCursor : public Script::Command {
	Common::String warp;
	int idx;
	HideCursor(Common::String w, int i) : warp(Common::move(w)), idx(i) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->hideCursor(warp, idx);
	}
};

struct ResetLockKey : public Script::Command {
	void exec(Script::ExecutionContext &ctx) const override {
		debug("resetlockkey");
		// FIXME: scripts don't restore lockkey after reset
	}
};

struct LockKey : public Script::Command {
	int idx;
	Common::String warp;

	LockKey(int i, Common::String w) : idx(i), warp(Common::move(w)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("lock key F%d: %s", idx, warp.c_str());
		Common::KeyCode code;
		switch (idx) {
		case 0:
			code = Common::KeyCode::KEYCODE_ESCAPE;
			break;
		case 1:
			code = Common::KeyCode::KEYCODE_F1;
			break;
		case 2:
			code = Common::KeyCode::KEYCODE_F2;
			break;
		case 3:
			code = Common::KeyCode::KEYCODE_F3;
			break;
		case 4:
			code = Common::KeyCode::KEYCODE_F4;
			break;
		case 5:
			code = Common::KeyCode::KEYCODE_F5;
			break;
		case 6:
			code = Common::KeyCode::KEYCODE_F6;
			break;
		case 7:
			code = Common::KeyCode::KEYCODE_F7;
			break;
		case 8:
			code = Common::KeyCode::KEYCODE_F8;
			break;
		case 9:
			code = Common::KeyCode::KEYCODE_F9;
			break;
		case 10:
			code = Common::KeyCode::KEYCODE_F10;
			break;
		case 11:
			code = Common::KeyCode::KEYCODE_F11;
			break;
		case 12:
			code = Common::KeyCode::KEYCODE_F12;
			break;
		default:
			warning("unknown lock key %d", idx);
			return;
		}
		g_engine->lockKey(code, warp);
	}
};

struct SetZoom : public Script::Command {
	int fov;
	SetZoom(int f) : fov(f) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setZoom(fov);
	}
};

struct AngleXMax : public Script::Command {
	float xMax;
	AngleXMax(float x) : xMax(x) {}

	void exec(Script::ExecutionContext &ctx) const override {
		warning("angle x max %g", xMax);
	}
};

struct AngleYMax : public Script::Command {
	float yMax0, yMax1;
	AngleYMax(float y0, float y1) : yMax0(y0), yMax1(y1) {}

	void exec(Script::ExecutionContext &ctx) const override {
		warning("angle y max %g %g", yMax0, yMax1);
	}
};

struct SetAngle : public Script::Command {
	float a0, a1;
	SetAngle(float a0_, float a1_) : a0(a0_), a1(a1_) {}

	void exec(Script::ExecutionContext &ctx) const override {
		warning("set angle %g %g", a0, a1);
	}
};

struct GoToWarp : public Script::Command {
	Common::String warp;
	GoToWarp(Common::String w) : warp(Common::move(w)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->goToWarp(warp);
	}
};

struct PlaySound : public Script::Command {
	Common::String sound;
	int volume;
	int loops;

	PlaySound(Common::String s, int v, int l) : sound(Common::move(s)), volume(v), loops(l) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->playSound(sound, volume, loops);
	}
};

struct StopSound : public Script::Command {
	Common::String sound;

	StopSound(Common::String s) : sound(Common::move(s)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->stopSound(sound);
	}
};

struct PlaySound3D : public Script::Command {
	Common::String sound;
	int volume;
	float angle;
	int loops;

	PlaySound3D(Common::String s, int v, float a, int l) : sound(Common::move(s)), volume(v), angle(a), loops(l) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->playSound(sound, volume, loops, true, angle);
	}
};

struct StopSound3D : public Script::Command {
	Common::String sound;

	StopSound3D(Common::String s) : sound(Common::move(s)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->stopSound(sound);
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
