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

#include "common/config-manager.h"

#include "graphics/renderer.h"

#include "engines/grim/debugger.h"
#include "engines/grim/md5check.h"
#include "engines/grim/grim.h"
#include "engines/grim/imuse/imuse_engine.h"

namespace Grim {

Debugger::Debugger() :
		GUI::Debugger() {

	registerCmd("check_gamedata", WRAP_METHOD(Debugger, cmd_checkFiles));
	registerCmd("lua_do", WRAP_METHOD(Debugger, cmd_lua_do));
	registerCmd("jump", WRAP_METHOD(Debugger, cmd_jump));
	registerCmd("renderer_set", WRAP_METHOD(Debugger, cmd_renderer_set));
	registerCmd("renderer_get", WRAP_METHOD(Debugger, cmd_renderer_get));
	registerCmd("save", WRAP_METHOD(Debugger, cmd_save));
	registerCmd("load", WRAP_METHOD(Debugger, cmd_load));
	registerCmd("imuse", WRAP_METHOD(Debugger, cmd_imuse));
}

Debugger::~Debugger() {

}

void Debugger::onFrame() {
	GUI::Debugger::onFrame();

	if (g_imuse)
		g_imuse->imProcessStreams();
}

bool Debugger::cmd_checkFiles(int argc, const char **argv) {
	if (MD5Check::checkFiles()) {
		debugPrintf("All files are ok.\n");
	} else {
		debugPrintf("Some files are corrupted or missing.\n");
	}

	return true;
}

bool Debugger::cmd_lua_do(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: lua_do <lua command>\n");
		return true;
	}

	Common::String cmd;
	for (int i = 1; i < argc; ++i) {
		cmd += argv[i];
		cmd += " ";
	}
	cmd.deleteLastChar();
	debugPrintf("Executing command: <%s>\n", cmd.c_str());
	cmd = Common::String::format("__temp_fn__ = function()\n%s\nend\nstart_script(__temp_fn__)", cmd.c_str());
	g_grim->debugLua(cmd);
	return true;
}

bool Debugger::cmd_jump(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: jump <jump target>\n");
		return true;
	}
	// Escape from Monkey Island keeps the jump script in a separate file, so load it first
	if (g_grim->getGameType() == GType_MONKEY4) {
		Common::String loadJS = Common::String::format("dofile(\"_jumpscripts.lua\")\n");
		g_grim->debugLua(loadJS.c_str());
	}

	// Start the jump_script Lua function with the desired target
	Common::String cmd = Common::String::format("start_script(jump_script,\"%s\")", argv[1]);
	g_grim->debugLua(cmd.c_str());
	return true;
}

bool Debugger::cmd_renderer_set(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: renderer_set <renderer>\n");
		debugPrintf("Where <renderer> is 'software', 'opengl' or 'opengl_shaders'\n");
		return true;
	}

	Graphics::RendererType renderer = Graphics::Renderer::parseTypeCode(argv[1]);
	if (renderer == Graphics::kRendererTypeDefault) {
		debugPrintf("Invalid renderer '%s'\n", argv[1]);
		return true;
	}

	ConfMan.set("renderer", Graphics::Renderer::getTypeCode(renderer));
	g_grim->changeHardwareState();
	return false;
}

bool Debugger::cmd_renderer_get(int argc, const char **argv) {
	auto rendererCodeStr = Graphics::Renderer::getTypeCode(g_grim->getRendererType());
	debugPrintf("%s\n", rendererCodeStr.c_str());
	return true;
}

bool Debugger::cmd_save(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: save <save name>\n");
		return true;
	}
	Common::String file = Common::String::format("%s.gsv", argv[1]);
	g_grim->saveGame(file);
	return true;
}

bool Debugger::cmd_load(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: load <save name>\n");
		return true;
	}
	Common::String file = Common::String::format("%s.gsv", argv[1]);
	g_grim->loadGame(file);
	return true;
}

bool Debugger::cmd_imuse(int argc, const char **argv) {
	if (!g_imuse) {
		debugPrintf("No Digital iMUSE engine is active.\n");
		return true;
	}

	if (argc > 1) {
		if (!strcmp(argv[1], "stop")) {
			if (argc > 2 && (!strcmp(argv[2], "all") || atoi(argv[2]) != 0)) {
				if (!strcmp(argv[2], "all")) {
					g_imuse->imStopAllSounds();
					debugPrintf("Stopping all sounds.\n");
				} else {
					g_imuse->imStopSound(atoi(argv[2]));
					debugPrintf("Attempted to stop sound %d.\n", atoi(argv[2]));
				}
			} else {
				debugPrintf("Specify a soundId or \"all\".\n");
			}
			return true;
		} else if (!strcmp(argv[1], "stopSpeech")) {
			debugPrintf("Attempting to stop the currently playing speech file, if any.\n");
			g_imuse->imStopSound(kTalkSoundID);
			return true;
		} else if (!strcmp(argv[1], "list") || !strcmp(argv[1], "tracks")) {
			g_imuse->listTracks();
			return true;
		} else if (!strcmp(argv[1], "playSfx")) {
			if (argc > 2 && atoi(argv[2]) >= 100 && atoi(argv[2]) < 100 + IMUSE_MAX_CACHED_SOUNDS) {
				debugPrintf("Attempting to play SFX %d...\n", atoi(argv[2]));
				g_imuse->imStartSound(atoi(argv[2]), 126);
			} else {
				debugPrintf("Specify a SFX soundId from 100-%d.\n", 100 + IMUSE_MAX_CACHED_SOUNDS - 1);
			}
			return true;
		} else if (!strcmp(argv[1], "playState") || !strcmp(argv[1], "setState")) {
			if (argc > 2 && atoi(argv[2]) >= 0) {
				debugPrintf("Attempting to play state %d...\n", atoi(argv[2]));
				g_imuse->imSetState(atoi(argv[2]));
			} else {
				debugPrintf("Specify a valid stateId; available states for this game:\n");
				g_imuse->listStates();
			}
			return true;
		} else if (!strcmp(argv[1], "playSeq") || !strcmp(argv[1], "setSeq")) {
			if (argc > 2 && atoi(argv[2]) >= 0) {
				debugPrintf("Attempting to play sequence %d...\n", atoi(argv[2]));
				g_imuse->imSetSequence(atoi(argv[2]));
			} else {
				debugPrintf("Specify a valid seqId; available sequences for this game:\n");
				g_imuse->listSeqs();
			}
			return true;
		} else if (!strcmp(argv[1], "playCue") || !strcmp(argv[1], "setCue")) {
			debugPrintf("Cues are not used by this game.\n");
			return true;
		} else if (!strcmp(argv[1], "hook")) {
			if (argc > 3 && atoi(argv[3]) != 0) {
				debugPrintf("Attempting to set hookId %d for sound %d...\n", atoi(argv[2]), atoi(argv[3]));
				g_imuse->imSetHook(atoi(argv[3]), atoi(argv[2]));
			} else {
				debugPrintf("Specify a hookId and a soundId;\nuse \"list\" to get a list of currently playing sounds.\n");
			}
			return true;
		} else if (!strcmp(argv[1], "states")) {
			debugPrintf("Available states for this game:\n");
			g_imuse->listStates();
			return true;
		} else if (!strcmp(argv[1], "seqs")) {
			debugPrintf("Available sequences for this game:\n");
			g_imuse->listSeqs();
			return true;
		} else if (!strcmp(argv[1], "cues")) {
			debugPrintf("Available cues for this game:\n");
			g_imuse->listCues();
			return true;
		} else if (!strcmp(argv[1], "groups") || !strcmp(argv[1], "vols")) {
			g_imuse->listGroups();
			return true;
		} else if (!strcmp(argv[1], "getParam")) {
			if (argc > 3) {
				int result = g_imuse->imGetParam(atoi(argv[2]), strtol(argv[3], nullptr, 16));
				if (result != -5 && result != -4 && result != -1) {
					debugPrintf("Parameter value for sound %d: %d\n", atoi(argv[2]), result);
					return true;
				}
				debugPrintf("Invalid parameter id or soundId.\n");
			}
			debugPrintf("Usage: getParam <soundId> <param>.\nReadable params (use the hex id):\n");
			debugPrintf("\tP_SND_TRACK_NUM  0x100 \n");
			debugPrintf("\tP_MARKER         0x300 \n");
			debugPrintf("\tP_GROUP          0x400 \n");
			debugPrintf("\tP_PRIORITY       0x500 \n");
			debugPrintf("\tP_VOLUME         0x600 \n");
			debugPrintf("\tP_PAN            0x700 \n");
			debugPrintf("\tP_DETUNE         0x800 \n");
			debugPrintf("\tP_TRANSPOSE      0x900 \n");
			debugPrintf("\tP_MAILBOX        0xA00 \n");
			debugPrintf("\tP_SND_HAS_STREAM 0x1800\n");
			debugPrintf("\tP_STREAM_BUFID   0x1900\n");
			debugPrintf("\tP_SND_POS_IN_MS  0x1A00\n");
			return true;
		} else if (!strcmp(argv[1], "setParam")) {
			if (argc > 4) {
				int result = g_imuse->imSetParam(atoi(argv[2]), strtol(argv[3], nullptr, 16), atoi(argv[4]));
				if (result != -5)
					return true;

				debugPrintf("Invalid parameter id, value or soundId.\n");
			}
			debugPrintf("Usage: setParam <soundId> <param> <val>.\nWritable params (use the hex id):\n");
			debugPrintf("\tP_GROUP          0x400 \n");
			debugPrintf("\tP_PRIORITY       0x500 \n");
			debugPrintf("\tP_VOLUME         0x600 \n");
			debugPrintf("\tP_PAN            0x700 \n");
			debugPrintf("\tP_DETUNE         0x800 \n");
			debugPrintf("\tP_TRANSPOSE      0x900 \n");
			debugPrintf("\tP_MAILBOX        0xA00 \n");
			debugPrintf("Please note that editing values for some parameters might lead to unexpected behavior.\n\n");
			return true;
		}

		debugPrintf("Unknown command. ");
	}

	debugPrintf("Available Digital iMUSE commands:\n");
	debugPrintf("\tstates                           - Display music states available for the current game\n");
	debugPrintf("\tseqs                             - Display music sequences available for the current game\n");
	debugPrintf("\tcues                             - Display music cues available for the current sequence\n");
	debugPrintf("\tplaySfx <soundId>                - Play a SFX resource by soundId\n");
	debugPrintf("\tplayState|setState <stateId>     - Play a music state resource by soundId\n");
	debugPrintf("\tplaySeq|setSeq <seqId>           - Play a music sequence resource by soundId\n");
	debugPrintf("\tplayCue|setCue <cueId>           - Play a music cue between the ones available\n");
	debugPrintf("\tstop <soundId>|all               - Stop a SFX, speech or music resource by soundId\n");
	debugPrintf("\tstopSpeech                       - Stop the current speech file, if any\n");
	debugPrintf("\thook <hookId> <soundId>          - Set hookId for a sound\n");
	debugPrintf("\tlist|tracks                      - Display info for every virtual audio track\n");
	debugPrintf("\tgroups|vols                      - Show volume groups info\n");
	debugPrintf("\tgetParam <soundId> <param>       - Get parameter info from a sound\n");
	debugPrintf("\tsetParam <soundId> <param> <val> - Set parameter value for a sound (dangerous!)\n");
	debugPrintf("\n");

	return true;
}

}
