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

#include "phoenixvr/commands_v2.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {
namespace {

struct Add : public Command {
	Common::String dstVar;
	Common::String arg0;
	Common::String arg1;

	Add(const Common::Array<Common::String> &args) : dstVar(args[0]), arg0(args[1]), arg1(args[2]) {}

	void exec(ExecutionContext &ctx) const override {
		debug("add %s %s %s", dstVar.c_str(), arg0.c_str(), arg1.c_str());
		g_engine->setVariable(dstVar, valueOf(arg0) + valueOf(arg1));
	}
};

struct Sub : public Command {
	Common::String dstVar;
	Common::String arg0;
	Common::String arg1;

	Sub(const Common::Array<Common::String> &args) : dstVar(args[0]), arg0(args[1]), arg1(args[2]) {}

	void exec(ExecutionContext &ctx) const override {
		debug("sub %s %s %s", dstVar.c_str(), arg0.c_str(), arg1.c_str());
		g_engine->setVariable(dstVar, valueOf(arg0) - valueOf(arg1));
	}
};

struct Go_Back : public Command {
	Go_Back(const Common::Array<Common::String> &args) {}
	void exec(ExecutionContext &ctx) const override {
		g_engine->returnToWarp();
	}
};

struct Goto_Level : public Command {
	Common::String name;
	Goto_Level(const Common::Array<Common::String> &args) : name(args[0]) {}
	void exec(ExecutionContext &ctx) const override {
		g_engine->goToLevel(name);
		ctx.running = false;
	}
};

struct Enter_Level : public Command {
	Enter_Level(const Common::Array<Common::String> &args) {}
	void exec(ExecutionContext &ctx) const override {
		warning("enter level");
	}
};

struct Leave_Save : public Command {
	Leave_Save(const Common::Array<Common::String> &args) {}
	void exec(ExecutionContext &ctx) const override {
		warning("leave save");
	}
};

struct Save_Slot : public Command {
	int index;
	Common::String name;
	Save_Slot(const Common::Array<Common::String> &args) : index(atoi(args[0].c_str())), name(args[1]) {}
	void exec(ExecutionContext &ctx) const override {
		warning("save slot %d: %s", index, name.c_str());
	}
};

struct Goto_Warp : public Command {
	Common::String name;
	Goto_Warp(const Common::Array<Common::String> &args) : name(args[0]) {}
	void exec(ExecutionContext &ctx) const override {
		g_engine->goToWarp(name);
	}
};

struct Play_AnimBloc : public Command {
	Common::String name;
	Common::String dstVar;
	int dstVarValue;
	float speed; // ticks per second

	Play_AnimBloc(const Common::Array<Common::String> &args) : name(args[0]), dstVar(args[1]), dstVarValue(atoi(args[2].c_str())), speed(args.size() >= 4 ? atof(args[3].c_str()) : 25) {}
	void exec(ExecutionContext &ctx) const override {
		debug("Play_AnimBloc %s %s %d %g", name.c_str(), dstVar.c_str(), dstVarValue, speed);
		g_engine->playAnimation(name, dstVar, dstVarValue, speed);
	}
};

struct Play_Amb : public Command {
	Common::String path;
	int volume;
	int loops;
	Play_Amb(const Common::Array<Common::String> &args) : path(args[0]), volume(atoi(args[1].c_str())), loops(atoi(args[2].c_str())) {}
	void exec(ExecutionContext &ctx) const override {
		g_engine->playSound(path, Audio::Mixer::kMusicSoundType, 100, loops);
	}
};

struct Delay_Sound : public Command {
	Common::String path;
	int delay;
	int volume;
	int loops;
	Delay_Sound(const Common::Array<Common::String> &args) : path(args[0]), delay(atoi(args[1].c_str())), volume(atoi(args[2].c_str())), loops(atoi(args[3].c_str())) {}
	void exec(ExecutionContext &ctx) const override {
		g_engine->playSound(path, Audio::Mixer::kMusicSoundType, 100, loops);
	}
};

struct Play_3DSound : public Command {
	Common::String path;
	int angle1;
	int angle2;
	int volume;
	int loops;
	Play_3DSound(const Common::Array<Common::String> &args) : path(args[0]),
															  angle1(atoi(args[1].c_str())), angle2(atoi(args[2].c_str())),
															  volume(atoi(args[3].c_str())), loops(atoi(args[4].c_str())) {}
	void exec(ExecutionContext &ctx) const override {
		g_engine->playSound(path, Audio::Mixer::kMusicSoundType, 100, loops, true, angle1);
	}
};

struct Play_Sound : public Command {
	Common::String sound;
	int volume;
	int loops;
	Audio::Mixer::SoundType type;

	Play_Sound(const Common::Array<Common::String> &args, Audio::Mixer::SoundType t = Audio::Mixer::kSFXSoundType) : sound(args[0]),
																													 volume(args.size() > 1 ? atoi(args[1].c_str()) : 100),
																													 loops(args.size() > 2 ? atoi(args[2].c_str()) : 0), type(t) {}

	void exec(ExecutionContext &ctx) const override {
		g_engine->playSound(sound, type, volume, loops);
	}
};

struct Stop_Sound : public Command {
	Common::String name;
	Stop_Sound(const Common::Array<Common::String> &args) : name(args[0]) {}

	void exec(ExecutionContext &ctx) const override {
		g_engine->stopSound(name);
	}
};

struct Stop_All_Sounds : public Command {
	Stop_All_Sounds(const Common::Array<Common::String> &args) {}

	void exec(ExecutionContext &ctx) const override {
		g_engine->stopAllSounds();
	}
};

struct Cursor_Load : public Command {
	Common::String index;
	Common::String path;
	Common::String width;
	Common::String height;

	Cursor_Load(const Common::Array<Common::String> &args) : index(args[0]), path(args[1]), width(args.size() > 2 ? args[2].c_str() : "0"), height(args.size() > 3 ? args[3].c_str() : "0") {}

	void exec(ExecutionContext &ctx) const override {
		g_engine->loadCursor(valueOf(index), path, valueOf(width), valueOf(height));
	}
};

struct Cursor_Set : public Command {
	Common::String type;
	Common::String index;

	Cursor_Set(const Common::Array<Common::String> &args) : type(args[0]), index(args[1]) {}

	void exec(ExecutionContext &ctx) const override {
		if (type == "_ON_IDDLE")
			g_engine->setCursorDefault(0, valueOf(index));
		else if (type == "_ON_TEST")
			g_engine->setCursorDefault(1, valueOf(index));
		else
			warning("invalid cursor type %s", type.c_str());
	}
};

struct Retrieve_State : public Command {
	Common::String index;
	Common::String var;

	Retrieve_State(const Common::Array<Common::String> &args) : index(args[0]), var(args[1]) {}

	void exec(ExecutionContext &ctx) const override {
		debug("retrieve state stub %s %s", index.c_str(), var.c_str());
		g_engine->setVariable(var, 1);
	}
};

struct Store_State : public Command {
	Common::String index;
	Common::String var;

	Store_State(const Common::Array<Common::String> &args) : index(args[0]), var(args[1]) {}

	void exec(ExecutionContext &ctx) const override {
		warning("store state stub %s %s -> %d", index.c_str(), var.c_str(), valueOf(var));
	}
};

struct Fade : public Command {
	int start;
	int stop;
	int speed;

	Fade(const Common::Array<Common::String> &args) : speed(valueOf(args[1].c_str())) {
		auto &type = args[0];
		if (type == "_FADE_IN") {
			start = -255;
			stop = 0;
		} else if (type == "_FADE_OUT") {
			start = 0;
			stop = -255;
		} else
			error("unknown fade type: %s", type.c_str());
	}

	void exec(ExecutionContext &ctx) const override {
		g_engine->fade(start, stop, speed);
	}
};

struct Sprite_Load : public Command {
	Common::String name;
	Common::String path;
	Common::String unk;
	Sprite_Load(const Common::Array<Common::String> &args) : name(args[0]), path(args[1]), unk(args[2]) {}
	void exec(ExecutionContext &ctx) const override {
		warning("sprite load %s %s %s", name.c_str(), path.c_str(), unk.c_str());
	}
};

struct Sprite_Screen : public Command {
	int index;
	Common::String name;
	Common::String x;
	Common::String y;
	Sprite_Screen(const Common::Array<Common::String> &args) : index(atoi(args[0].c_str())) {
		if (args.size() > 1)
			name = args[1];
		if (args.size() > 2)
			x = args[2];
		if (args.size() > 3)
			y = args[3];
	}
	void exec(ExecutionContext &ctx) const override {
		warning("sprite screen %d %s %s %s", index, name.c_str(), x.c_str(), y.c_str());
	}
};

struct Set_Lens : public Command {
	Common::String index;
	Common::String name;
	Common::String unk;
	Set_Lens(const Common::Array<Common::String> &args) : index(args[0]), name(args[1]), unk(args[2]) {}
	void exec(ExecutionContext &ctx) const override {
		warning("set lens %s %s %s", index.c_str(), name.c_str(), unk.c_str());
	}
};

struct Set_Lensflare : public Command {
	Set_Lensflare(const Common::Array<Common::String> &args) {}
	void exec(ExecutionContext &ctx) const override {
		warning("set lensflare");
	}
};

struct Start_Light : public Command {
	Common::String fx;
	Start_Light(const Common::Array<Common::String> &args) : fx(args[0]) {}
	void exec(ExecutionContext &ctx) const override {
		warning("start light %s", fx.c_str());
	}
};

struct Stop_Light : public Command {
	Stop_Light(const Common::Array<Common::String> &args) {}
	void exec(ExecutionContext &ctx) const override {
		warning("stop light");
	}
};

struct Set_Jump_Key : public Command {
	int key;
	Common::String warp;
	Set_Jump_Key(const Common::Array<Common::String> &args) : warp(args.size() > 1 ? args[1] : Common::String{}) {
		auto &keyName = args[0];
		if (keyName == "_KEY_ESCAPE")
			key = 0;
		else if (keyName == "_KEY_RIGHT_CLIC")
			key = 12;
		else
			error("unhandled key name: %s", keyName.c_str());
	}
	void exec(ExecutionContext &ctx) const override {
		g_engine->lockKey(key, warp);
	}
};

struct Play_Movie : public Command {
	Common::String path;
	Play_Movie(const Common::Array<Common::String> &args) : path(args[0]) {}
	void exec(ExecutionContext &ctx) const override {
		g_engine->playMovie(path);
	}
};

struct Test_Slot : public Command {
	int index;
	Common::String sprite;
	Common::String var;
	Test_Slot(const Common::Array<Common::String> &args) : index(atoi(args[0].c_str())), sprite(args[1]), var(args[2]) {}
	void exec(ExecutionContext &ctx) const override {
		debug("test slot %d %s %s", index, sprite.c_str(), var.c_str());
		g_engine->testSaveSlot(index);
	}
};

struct Load_Slot : public Command {
	Common::String slot;
	Load_Slot(const Common::Array<Common::String> &args) : slot(args[0]) {}
	void exec(ExecutionContext &ctx) const override {
		auto index = valueOf(slot);
		auto err = g_engine->loadGameState(index);
		if (err.getCode() != Common::ErrorCode::kNoError)
			error("loading state failed %d", index);
	}
};

struct Start_Timer : public Command {
	float seconds;
	Common::String warp;
	Start_Timer(const Common::Array<Common::String> &args) : seconds(atof(args[0].c_str())), warp(args[1].c_str()) {}
	void exec(ExecutionContext &ctx) const override {
		warning("start timer %g %s", seconds, warp.c_str());
	}
};

struct Stop_Timer : public Command {
	Stop_Timer(const Common::Array<Common::String> &args) {}
	void exec(ExecutionContext &ctx) const override {
		g_engine->killTimer();
	}
};

struct Limit_View : public Command {
	Common::String angle1, angle2;
	Limit_View(const Common::Array<Common::String> &args) : angle1(args[0]), angle2(args[1]) {}
	void exec(ExecutionContext &ctx) const override {
		warning("limit view %d %d", valueOf(angle1), valueOf(angle2));
	}
};

struct Set_View_Angle : public Command {
	Common::String angle1, angle2;
	Set_View_Angle(const Common::Array<Common::String> &args) : angle1(args[0]), angle2(args[1]) {}
	void exec(ExecutionContext &ctx) const override {
		warning("set view angle %d %d", valueOf(angle1), valueOf(angle2));
	}
};

struct Exit_Game : public Command {
	Exit_Game(const Common::Array<Common::String> &args) {}
	void exec(ExecutionContext &ctx) const override {
		debug("exit game");
		g_engine->quitGame();
	}
};

struct Quit_URL : public Command {
	Common::String name;
	Quit_URL(const Common::Array<Common::String> &args) : name(args[0]) {}
	void exec(ExecutionContext &ctx) const override {
		debug("quit url: %s", name.c_str());
	}
};

} // namespace

#define COMMAND_LIST(E) \
	E(Add)              \
	E(Cursor_Load)      \
	E(Cursor_Set)       \
	E(Delay_Sound)      \
	E(Enter_Level)      \
	E(Exit_Game)        \
	E(Fade)             \
	E(Go_Back)          \
	E(Goto_Level)       \
	E(Goto_Warp)        \
	E(Leave_Save)       \
	E(Limit_View)       \
	E(Load_Slot)        \
	E(Play_3DSound)     \
	E(Play_Amb)         \
	E(Play_AnimBloc)    \
	E(Play_Movie)       \
	E(Play_Sound)       \
	E(Quit_URL)         \
	E(Retrieve_State)   \
	E(Save_Slot)        \
	E(Set_Jump_Key)     \
	E(Set_Lens)         \
	E(Set_Lensflare)    \
	E(Set_View_Angle)   \
	E(Sprite_Load)      \
	E(Sprite_Screen)    \
	E(Start_Light)      \
	E(Start_Timer)      \
	E(Stop_All_Sounds)  \
	E(Stop_Light)       \
	E(Stop_Sound)       \
	E(Stop_Timer)       \
	E(Store_State)      \
	E(Sub)              \
	E(Test_Slot)

#define ADD_COMMAND(NAME)            \
	if (cmd.equalsIgnoreCase(#NAME)) \
		return CommandPtr(new NAME(args));

CommandPtr createV2Command(const Common::String &cmd, const Common::Array<Common::String> &args, int lineno) {
	COMMAND_LIST(ADD_COMMAND)
	error("unhandled command %s at line %d", cmd.c_str(), lineno);
}

} // namespace PhoenixVR
