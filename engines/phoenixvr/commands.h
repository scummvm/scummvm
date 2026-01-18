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
		debug("LoadSave_Enter_Script %s, %s", reloading.c_str(), notReloading.c_str());
		auto loading = g_engine->isLoading();
		g_engine->setVariable(reloading, loading);
		g_engine->setVariable(notReloading, !loading);
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
		g_engine->wait(seconds);
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
		g_engine->scheduleTest(cursor);
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
		bool r;
		auto value0 = g_engine->getVariable(arg0);
		if (op == "==") {
			r = value0 == arg1;
		} else if (op == "<") {
			r = value0 < arg1;
		} else if (op == "<=") {
			r = value0 <= arg1;
		} else if (op == ">") {
			r = value0 > arg1;
		} else if (op == ">=") {
			r = value0 >= arg1;
		} else {
			error("invalid cmp op %s", op.c_str());
		}
		g_engine->setVariable(var, r);
		g_engine->setVariable(negativeVar, !r);
	}
};

struct LoadSave_Init_Slots : public Script::Command {
	int slots;

	LoadSave_Init_Slots(const Common::Array<Common::String> &args) : slots(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
	}
};

struct LoadSave_Draw_Slot : public Script::Command {
	int slot;
	int face;
	int x;
	int y;

	LoadSave_Draw_Slot(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())),
																	face(atoi(args[1].c_str())),
																	x(atoi(args[2].c_str())),
																	y(atoi(args[3].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Draw_Slot %d %d %d %d", slot, face, x, y);
		g_engine->drawSlot(slot, face, x, y);
	}
};

struct LoadSave_Test_Slot : public Script::Command {
	int slot;
	Common::String show;
	Common::String hide;

	LoadSave_Test_Slot(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())), show(args[1]), hide(args[2]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		bool exists = g_engine->testSaveSlot(slot);
		debug("LoadSave_Test_Slot %d %s %s -> %d", slot, show.c_str(), hide.c_str(), exists);
		g_engine->setVariable(show, exists);
		g_engine->setVariable(hide, !exists);
	}
};

struct LoadSave_Capture_Context : public Script::Command {
	LoadSave_Capture_Context(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Capture_Context");
		g_engine->captureContext();
	}
};

struct LoadSave_Context_Restored : public Script::Command {
	Common::String progress;
	Common::String done;

	LoadSave_Context_Restored(const Common::Array<Common::String> &args) : progress(args[0]), done(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Context_Restored: semi-stub: can be used to report that loading is in a progress");
		g_engine->setVariable(progress, 0);
		g_engine->setVariable(done, g_engine->isLoading());
	}
};

struct LoadSave_Load : public Script::Command {
	int slot;

	LoadSave_Load(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Load %d", slot);
		g_engine->loadSaveSlot(slot);
	}
};

struct LoadSave_Save : public Script::Command {
	int slot;

	LoadSave_Save(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Save %d", slot);
		g_engine->saveSaveSlot(slot);
	}
};

struct LoadSave_Set_Context_Label : public Script::Command {
	Common::String label;

	LoadSave_Set_Context_Label(const Common::Array<Common::String> &args) : label(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setContextLabel(label);
	}
};

struct Rollover : public Script::Command {
	int arg;

	Rollover(const Common::Array<Common::String> &args) : arg(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->rollover({57, 427, 409, 480}, arg, 14, 1, 0);
	}
};

struct RolloverMalette : public Script::Command {
	int arg;

	RolloverMalette(const Common::Array<Common::String> &args) : arg(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->rollover({251, 346, 522, 394}, arg, 18, 1, 0xD698);
	}
};

struct RolloverSecretaire : public Script::Command {
	int arg;

	RolloverSecretaire(const Common::Array<Common::String> &args) : arg(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->rollover({216, 367, 536, 430}, arg, 12, 1, 0xFFFF);
	}
};

struct SaveVariable : public Script::Command {
	SaveVariable(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->saveVariables();
	}
};

struct LoadVariable : public Script::Command {
	LoadVariable(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->loadVariables();
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
	E(LoadVariable)                  \
	E(MultiCD_Set_Transition_Script) \
	E(MultiCD_Set_Next_Script)       \
	E(PauseTimer)                    \
	E(Play_AnimBloc)                 \
	E(Play_AnimBloc_Number)          \
	E(Play_Movie)                    \
	E(Rollover)                      \
	E(RolloverMalette)               \
	E(RolloverSecretaire)            \
	E(SaveVariable)                  \
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
		if (!result) {
			debug("ifand: not executing conditional block");
			return;
		}
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

struct GoSub : public Script::Command {
	Common::String label;
	GoSub(const Common::String &l) : label(l) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("gosub %s", label.c_str());
		assert(ctx.scope);
		auto *labelPtr = ctx.scope->findLabel(label);
		assert(labelPtr);
		Script::ExecutionContext sub = {};
		sub.subroutine = true;
		ctx.scope->exec(sub, labelPtr->offset);
	}
};

struct End : public Script::Command {
	End() {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("end");
		ctx.running = false;
		g_engine->end();
	}
};

struct Return : public Script::Command {
	Return() {}
	void exec(Script::ExecutionContext &ctx) const override {
		ctx.running = false;
		if (ctx.subroutine) {
			debug("return to caller");
		} else {
			debug("return to previous warp");
			g_engine->returnToWarp();
		}
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
		g_engine->resetLockKey();
	}
};

struct LockKey : public Script::Command {
	int idx;
	Common::String warp;

	LockKey(int i, Common::String w) : idx(i), warp(Common::move(w)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("lock key %d: %s", idx, warp.c_str());
		g_engine->lockKey(idx, warp);
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
		debug("anglexmax %g", xMax);
		g_engine->setXMax(xMax);
	}
};

struct AngleYMax : public Script::Command {
	float yMin, yMax;
	AngleYMax(float min, float max) : yMin(min), yMax(max) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("angleymax %g %g", yMin, yMax);
		g_engine->setYMax(yMin, yMax);
	}
};

struct SetAngle : public Script::Command {
	float x, y;
	SetAngle(float x_, float y_) : x(x_), y(y_) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("setangle %g %g", x, y);
		g_engine->setAngle(x, y);
	}
};

struct GoToWarp : public Script::Command {
	Common::String warp;
	GoToWarp(Common::String w) : warp(Common::move(w)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->goToWarp(warp);
		ctx.running = false; // terminate script after warp
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
