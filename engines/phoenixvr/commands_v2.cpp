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

} // namespace

#define COMMAND_LIST(E) \
	E(Add)              \
	E(Cursor_Load)      \
	E(Cursor_Set)       \
	E(Play_Sound)       \
	E(Play_AnimBloc)    \
	E(Stop_All_Sounds)  \
	E(Stop_Sound)       \
	E(Sub)

#define ADD_COMMAND(NAME)            \
	if (cmd.equalsIgnoreCase(#NAME)) \
		return CommandPtr(new NAME(args));

CommandPtr createV2Command(const Common::String &cmd, const Common::Array<Common::String> &args, int lineno) {
	COMMAND_LIST(ADD_COMMAND)
	warning("unhandled command %s at line %d", cmd.c_str(), lineno);
	return nullptr;
}

} // namespace PhoenixVR
