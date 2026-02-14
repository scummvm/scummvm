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
		auto loading = g_engine->enterScript();
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
	Common::String dstVar;
	int dstVarValue;
	float speed; // ticks per second

	Play_AnimBloc(const Common::Array<Common::String> &args) : name(args[0]), dstVar(args[1]), dstVarValue(atoi(args[2].c_str())), speed(atof(args[3].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("Play_AnimBloc %s %s %d, %g", name.c_str(), dstVar.c_str(), dstVarValue, speed);
		g_engine->playAnimation(name, dstVar, dstVarValue, speed);
	}
};

struct Stop_AnimBloc : public Script::Command {
	Common::String name;

	Stop_AnimBloc(const Common::Array<Common::String> &args) : name(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("Stop_AnimBloc %s", name.c_str());
	}
};

struct Play_AnimBloc_Number : public Script::Command {
	Common::String prefix, var;
	Common::String dstVar;
	int dstVarValue;
	float speed;

	Play_AnimBloc_Number(const Common::Array<Common::String> &args) : prefix(args[0]), var(args[1]),
																	  dstVar(args[2]), dstVarValue(atoi(args[3].c_str())),
																	  speed(atof(args[4].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("Play_AnimBloc_Number %s %s %s %d, %g", prefix.c_str(), var.c_str(), dstVar.c_str(), dstVarValue, speed);
		int value = g_engine->getVariable(var);
		auto name = Common::String::format("%s%04d", prefix.c_str(), value);
		g_engine->playAnimation(name, dstVar, dstVarValue, speed);
	}
};

struct Until : public Script::Command {
	Common::String var;
	int value;

	Until(const Common::Array<Common::String> &args) : var(args[0]), value(atoi(args[1].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->until(var, value);
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

struct Not : public Script::Command {
	Common::String var;

	Not(Common::String v) : var(Common::move(v)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("not %s", var.c_str());
		g_engine->setVariable(var, g_engine->getVariable(var) ? 0 : 1);
	}
};

struct GetMonde4 : public Script::Command {
	Common::String var;
	Common::String negativeVar;

	GetMonde4(const Common::Array<Common::String> &args) : var(args[0]), negativeVar(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("GetMonde4 %s %s", var.c_str(), negativeVar.c_str());
		g_engine->setVariable(var, 0);
		g_engine->setVariable(negativeVar, 1);
	}
};

struct SetMonde4 : public Script::Command {
	int value;

	SetMonde4(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("SetMonde4 %d", value);
	}
};

struct AddObject : public Script::Command {
	int object;
	Common::String var;
	Common::String negativeVar;

	AddObject(const Common::Array<Common::String> &args) : object(atoi(args[0].c_str())), var(args[1]), negativeVar(args[2]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("AddObject %d %s %s", object, var.c_str(), negativeVar.c_str());
	}
};

struct AddCoffreObject : public Script::Command {
	int object;

	AddCoffreObject(const Common::Array<Common::String> &args) : object(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("AddCoffreObject %d", object);
	}
};

struct IsPresent : public Script::Command {
	int object;
	Common::String var;
	Common::String negativeVar;

	IsPresent(const Common::Array<Common::String> &args) : object(atoi(args[0].c_str())), var(args[1]), negativeVar(args[2]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("IsPresent %d %s %s", object, var.c_str(), negativeVar.c_str());
	}
};

struct RemoveObject : public Script::Command {
	int object;

	RemoveObject(const Common::Array<Common::String> &args) : object(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("RemoveObject %d", object);
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

struct Select : public Script::Command {
	int value;
	Common::String arg0;
	Common::String arg1;

	Select(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())), arg0(args[1]), arg1(args[2]) {}

	void exec(Script::ExecutionContext &ctx) const override {
		warning("Select %d %s %s", value, arg0.c_str(), arg1.c_str());
	}
};

struct DoAction : public Script::Command {
	int value;
	Common::String arg;

	DoAction(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())), arg(args[1]) {}

	void exec(Script::ExecutionContext &ctx) const override {
		warning("DoAction %d %s", value, arg.c_str());
	}
};

struct IsHere : public Script::Command {
	IsHere(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("IsHere");
	}
};

struct InitCoffre : public Script::Command {
	InitCoffre(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("InitCoffre");
	}
};

struct LoadCoffre : public Script::Command {
	LoadCoffre(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("LoadCoffre");
	}
};

struct SaveCoffre : public Script::Command {
	SaveCoffre(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("SaveCoffre");
	}
};

struct AfficheCoffre : public Script::Command {
	AfficheCoffre(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("AfficheCoffre");
	}
};

struct AfficheSelection : public Script::Command {
	AfficheSelection(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("AfficheSelection");
	}
};

struct AffichePorteF : public Script::Command {
	int value;
	AffichePorteF(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("AffichePorteF %d", value);
	}
};

struct SelectPorteF : public Script::Command {
	int value;
	Common::String var;
	SelectPorteF(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())), var(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("SelectPorteF %d %s", value, var.c_str());
	}
};

struct SelectCoffre : public Script::Command {
	int value;
	Common::String var;
	SelectCoffre(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())), var(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("SelectCoffre %d %s", value, var.c_str());
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
		int value = g_engine->getVariable(progress);
		debug("LoadSave_Context_Restored: %s -> %d -> %s", progress.c_str(), value, done.c_str());
		g_engine->setVariable(progress, 0);
		g_engine->setVariable(done, value);
	}
};

struct LoadSave_Load : public Script::Command {
	int slot;

	LoadSave_Load(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Load %d", slot);
		auto err = g_engine->loadGameState(slot);
		if (err.getCode() != Common::ErrorCode::kNoError)
			error("loading state failed %d", slot);
	}
};

struct LoadSave_Save : public Script::Command {
	int slot;

	LoadSave_Save(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Save %d", slot);
		auto err = g_engine->saveGameState(slot, {});
		if (err.getCode() != Common::ErrorCode::kNoError)
			error("saving state failed %d", slot);
	}
};

struct LoadSave_Set_Context_Label : public Script::Command {
	Common::String label;

	LoadSave_Set_Context_Label(const Common::Array<Common::String> &args) : label(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setContextLabel(label);
	}
};

struct Discocier : public Script::Command {
	Discocier(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("Discocier");
	}
};

struct Reset : public Script::Command {
	Reset(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("Reset");
	}
};

struct MemoryRelease : public Script::Command {
	MemoryRelease(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("MemoryRelease");
	}
};

struct DrawTextSelection : public Script::Command {
	DrawTextSelection(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("DrawTextSelection");
	}
};

struct CarteDestination : public Script::Command {
	Common::String varX;
	Common::String varY;
	CarteDestination(const Common::Array<Common::String> &args) : varX(args[0]), varY(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("CarteDestination %s %s", varX.c_str(), varY.c_str());
	}
};

struct Scroll : public Script::Command {
	int value;
	Scroll(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("Scroll");
	}
};

struct Rollover : public Script::Command {
	int textId;
	RolloverType type;

	Rollover(const Common::Array<Common::String> &args, RolloverType t = RolloverType::Default) : textId(atoi(args[0].c_str())), type(t) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->rollover(textId, type);
	}
};

struct RolloverMalette : public Rollover {
	RolloverMalette(const Common::Array<Common::String> &args) : Rollover(args, RolloverType::Malette) {}
};

struct RolloverSecretaire : public Rollover {
	RolloverSecretaire(const Common::Array<Common::String> &args) : Rollover(args, RolloverType::Secretaire) {}
};

struct PorteFRollover : public Script::Command {
	int arg;

	PorteFRollover(const Common::Array<Common::String> &args) : arg(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		warning("PorteFRollover %d", arg);
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

struct End : public Script::Command {
	End(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->quitGame();
	}
};

#define PLUGIN_LIST(E)               \
	E(Add)                           \
	E(AddCoffreObject)               \
	E(AddObject)                     \
	E(AfficheCoffre)                 \
	E(AffichePorteF)                 \
	E(AfficheSelection)              \
	E(CarteDestination)              \
	E(ChangeCurseur)                 \
	E(Cmp)                           \
	E(Discocier)                     \
	E(DrawTextSelection)             \
	E(End)                           \
	E(GetMonde4)                     \
	E(SetMonde4)                     \
	E(IsPresent)                     \
	E(KillTimer)                     \
	E(InitCoffre)                    \
	E(IsHere)                        \
	E(LoadCoffre)                    \
	E(SaveCoffre)                    \
	E(SelectPorteF)                  \
	E(SelectCoffre)                  \
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
	E(MemoryRelease)                 \
	E(MultiCD_Set_Transition_Script) \
	E(MultiCD_Set_Next_Script)       \
	E(PauseTimer)                    \
	E(Play_AnimBloc)                 \
	E(Play_AnimBloc_Number)          \
	E(Play_Movie)                    \
	E(Reset)                         \
	E(RemoveObject)                  \
	E(Rollover)                      \
	E(RolloverMalette)               \
	E(RolloverSecretaire)            \
	E(PorteFRollover)                \
	E(SaveVariable)                  \
	E(Select)                        \
	E(Scroll)                        \
	E(Stop_AnimBloc)                 \
	E(DoAction)                      \
	E(StartTimer)                    \
	E(Sub)                           \
	E(Until)                         \
	E(While)                         \
	/* */

#define ADD_PLUGIN(NAME)             \
	if (cmd.equalsIgnoreCase(#NAME)) \
		return Script::CommandPtr(new NAME(args));

Script::CommandPtr createCommand(const Common::String &cmd, const Common::Array<Common::String> &args, int lineno) {
	PLUGIN_LIST(ADD_PLUGIN)
	error("unhandled plugin command %s at line %d", cmd.c_str(), lineno);
}

struct IfAnd : public Script::Conditional {
	IfAnd(Common::Array<Common::String> args) : Script::Conditional(Common::move(args)) {}
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
	IfOr(Common::Array<Common::String> args) : Script::Conditional(Common::move(args)) {}
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

struct EndScript : public Script::Command {
	EndScript() {}
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

struct InterpolAngle : public Script::Command {
	float x, y;
	int unk;
	InterpolAngle(float x_, float y_, int u) : x(x_), y(y_), unk(u) {}

	void exec(Script::ExecutionContext &ctx) const override {
		warning("interpolangle %g,%g %d", x, y, unk);
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

struct PlayMusique : public PlaySound {
	PlayMusique(Common::String s, int v) : PlaySound(Common::move(s), v, -1) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setCurrentMusic(sound, volume);
		PlaySound::exec(ctx);
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
