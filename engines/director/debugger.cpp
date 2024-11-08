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

#include "common/language.h"
#include "common/platform.h"
#include "director/director.h"
#include "director/debugger.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/util.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-codegen.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-the.h"

namespace Director {

#define PROMPT "lingo) "

Debugger *g_debugger;

Debugger::Debugger(): GUI::Debugger() {
	g_debugger = this;
	registerCmd("help", WRAP_METHOD(Debugger, cmdHelp));

	registerCmd("version", WRAP_METHOD(Debugger, cmdVersion));
	registerCmd("info", WRAP_METHOD(Debugger, cmdInfo));
	registerCmd("movie", WRAP_METHOD(Debugger, cmdMovie));
	registerCmd("m", WRAP_METHOD(Debugger, cmdMovie));
	registerCmd("frame", WRAP_METHOD(Debugger, cmdFrame));
	registerCmd("f", WRAP_METHOD(Debugger, cmdFrame));
	registerCmd("channels", WRAP_METHOD(Debugger, cmdChannels));
	registerCmd("chan", WRAP_METHOD(Debugger, cmdChannels));
	registerCmd("cast", WRAP_METHOD(Debugger, cmdCast));
	registerCmd("nextframe", WRAP_METHOD(Debugger, cmdNextFrame));
	registerCmd("nf", WRAP_METHOD(Debugger, cmdNextFrame));
	registerCmd("nextmovie", WRAP_METHOD(Debugger, cmdNextMovie));
	registerCmd("nm", WRAP_METHOD(Debugger, cmdNextMovie));

	registerCmd("print", WRAP_METHOD(Debugger, cmdPrint));
	registerCmd("p", WRAP_METHOD(Debugger, cmdPrint));
	registerCmd("repl", WRAP_METHOD(Debugger, cmdRepl));
	registerCmd("stack", WRAP_METHOD(Debugger, cmdStack));
	registerCmd("st", WRAP_METHOD(Debugger, cmdStack));
	registerCmd("scriptframe", WRAP_METHOD(Debugger, cmdScriptFrame));
	registerCmd("sf", WRAP_METHOD(Debugger, cmdScriptFrame));
	registerCmd("funcs", WRAP_METHOD(Debugger, cmdFuncs));
	registerCmd("backtrace", WRAP_METHOD(Debugger, cmdBacktrace));
	registerCmd("bt", WRAP_METHOD(Debugger, cmdBacktrace));
	registerCmd("disasm", WRAP_METHOD(Debugger, cmdDisasm));
	registerCmd("da", WRAP_METHOD(Debugger, cmdDisasm));
	registerCmd("var", WRAP_METHOD(Debugger, cmdVar));
	registerCmd("v", WRAP_METHOD(Debugger, cmdVar));
	registerCmd("actions", WRAP_METHOD(Debugger, cmdActions));
	registerCmd("act", WRAP_METHOD(Debugger, cmdActions));
	registerCmd("markers", WRAP_METHOD(Debugger, cmdMarkers));
	registerCmd("mk", WRAP_METHOD(Debugger, cmdMarkers));
	registerCmd("step", WRAP_METHOD(Debugger, cmdStep));
	registerCmd("s", WRAP_METHOD(Debugger, cmdStep));
	registerCmd("next", WRAP_METHOD(Debugger, cmdNext));
	registerCmd("n", WRAP_METHOD(Debugger, cmdNext));
	registerCmd("finish", WRAP_METHOD(Debugger, cmdFinish));
	registerCmd("fin", WRAP_METHOD(Debugger, cmdFinish));
	registerCmd("continue", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("c", WRAP_METHOD(Debugger, cmdExit));

	registerCmd("bpset", WRAP_METHOD(Debugger, cmdBpSet));
	registerCmd("b", WRAP_METHOD(Debugger, cmdBpSet));
	registerCmd("bpmovie", WRAP_METHOD(Debugger, cmdBpMovie));
	registerCmd("bm", WRAP_METHOD(Debugger, cmdBpMovie));
	registerCmd("bpframe", WRAP_METHOD(Debugger, cmdBpFrame));
	registerCmd("bf", WRAP_METHOD(Debugger, cmdBpFrame));
	registerCmd("bpentity", WRAP_METHOD(Debugger, cmdBpEntity));
	registerCmd("be", WRAP_METHOD(Debugger, cmdBpEntity));
	registerCmd("bpprop", WRAP_METHOD(Debugger, cmdBpProp));
	registerCmd("bp", WRAP_METHOD(Debugger, cmdBpProp));
	registerCmd("bpvar", WRAP_METHOD(Debugger, cmdBpVar));
	registerCmd("bv", WRAP_METHOD(Debugger, cmdBpVar));
	registerCmd("bpevent", WRAP_METHOD(Debugger, cmdBpEvent));
	registerCmd("bn", WRAP_METHOD(Debugger, cmdBpEvent));
	registerCmd("bpdel", WRAP_METHOD(Debugger, cmdBpDel));
	registerCmd("bpenable", WRAP_METHOD(Debugger, cmdBpEnable));
	registerCmd("bpdisable", WRAP_METHOD(Debugger, cmdBpDisable));
	registerCmd("bplist", WRAP_METHOD(Debugger, cmdBpList));

	registerCmd("draw", WRAP_METHOD(Debugger, cmdDraw));
	registerCmd("forceredraw", WRAP_METHOD(Debugger, cmdForceRedraw));

	_nextFrame = false;
	_nextFrameCounter = 0;
	_nextMovie = false;
	_step = false;
	_stepCounter = 0;
	_finish = false;
	_finishCounter = 0;
	_next = false;
	_nextCounter = 0;
	_lingoEval = false;
	_lingoReplMode = false;
}

Debugger::~Debugger() {
	if (_out.isOpen())
		_out.close();
}

bool Debugger::cmdHelp(int argc, const char **argv) {
	debugPrintf("\n");
	debugPrintf("Debug flags\n");
	debugPrintf("-----------\n");
	debugPrintf("debugflag_list - Lists the available debug flags and their status\n");
	debugPrintf("debugflag_enable - Enables a debug flag\n");
	debugPrintf("debugflag_disable - Disables a debug flag\n");
	debugPrintf("debuglevel - Shows or sets debug level\n");
	debugPrintf("\n");
	debugPrintf("Commands\n");
	debugPrintf("--------\n");
	debugPrintf("Player:\n");
	debugPrintf(" version - Shows the Director version\n");
	debugPrintf(" info - Shows information about the current movie\n");
	debugPrintf(" movie / m [moviePath] - Get or sets the current movie\n");
	debugPrintf(" frame / f [frameNum] - Gets or sets the current score frame\n");
	debugPrintf(" channels / chan [frameNum] - Shows channel information for a score frame\n");
	debugPrintf(" cast [castNum] - Shows the cast list or castNum for the current movie\n");
	debugPrintf(" nextframe / nf [n] - Steps forward one or more score frames\n");
	debugPrintf(" nextmovie / nm - Steps forward until the next change of movie\n");
	debugPrintf("\n");
	debugPrintf("Lingo execution:\n");
	debugPrintf(" print / p [statement] - Evaluates a single Lingo statement\n");
	debugPrintf(" repl - Switches to a REPL interface for evaluating Lingo statements\n");
	debugPrintf(" backtrace / bt - Prints a backtrace of all stack frames\n");
	debugPrintf(" disasm / da [scriptId:funcName] - Lists the bytecode disassembly for a script function\n");
	debugPrintf(" disasm / da all - Lists the bytecode disassembly for all available script functions\n");
	debugPrintf(" stack / st - Lists the elements on the stack\n");
	debugPrintf(" scriptframe / sf - Prints the current script frame\n");
	debugPrintf(" funcs - Lists all of the functions available in the current script frame\n");
	debugPrintf(" actions / act - Lists all of the action scripts available in the current score\n");
	debugPrintf(" var / v - Lists all of the variables available in the current script frame\n");
	debugPrintf(" markers / mk - Lists all of the frame markers in the current score\n");
	debugPrintf(" step / s [n] - Steps forward one or more operations\n");
	debugPrintf(" next / n [n] - Steps forward one or more operations, skips over calls\n");
	debugPrintf(" finish / fin - Steps until the current stack frame returns\n");
	debugPrintf(" continue / c - Continues execution\n");
	debugPrintf("\n");
	debugPrintf("Breakpoints:\n");
	debugPrintf(" bpset / b - Creates a breakpoint at the current Lingo function and offset\n");
	debugPrintf(" bpset / b [funcName] - Creates a breakpoint on a Lingo function matching a name\n");
	debugPrintf(" bpset / b [offset] - Creates a breakpoint on the current Lingo function matching an offset\n");
	debugPrintf(" bpset / b [funcName] [offset] - Creates a breakpoint on a Lingo function matching a name and offset\n");
	debugPrintf(" bpset / b [scriptId:funcName] - Creates a breakpoint on a Lingo function matching a script ID and name\n");
	debugPrintf(" bpset / b [scriptId:funcName] [offset] - Creates a breakpoint on a Lingo function matching a script ID, name and offset\n");
	debugPrintf(" bpmovie / bm [moviePath] - Create a breakpoint on a switch to a movie\n");
	debugPrintf(" bpframe / bf [frameId] - Create a breakpoint on a frame in the score\n");
	debugPrintf(" bpframe / bf [moviePath] [frameId] - Create a breakpoint on a frame in the score of a specific movie\n");
	debugPrintf(" bpentity / be [entityName] - Create a breakpoint on a Lingo \"the\" entity being read or modified\n");
	debugPrintf(" bpentity / be [entityName] [r/w/rw] - Create a breakpoint on a Lingo \"the\" entity being accessed in a specific way\n");
	debugPrintf(" bpentity / be [entityName:fieldName] - Create a breakpoint on a Lingo \"the\" field being read or modified\n");
	debugPrintf(" bpentity / be [entityName:fieldName] [r/w/rw] - Create a breakpoint on a Lingo \"the\" field being accessed in a specific way\n");
	debugPrintf(" bpprop / bp [varName] - Create a breakpoint on a Lingo object property being read or modified\n");
	debugPrintf(" bpprop / bp [varName] [r/w/rw] - Create a breakpoint on a Lingo object property being accessed in a specific way\n");
	debugPrintf(" bpvar / bv [varName] - Create a breakpoint on a Lingo variable being read or modified\n");
	debugPrintf(" bpvar / bv [varName] [r/w/rw] - Create a breakpoint on a Lingo variable being accessed in a specific way\n");
	debugPrintf(" bpevent / bn [eventName] - Create a breakpoint on a Lingo event\n");
	debugPrintf(" bpdel [n] - Deletes a specific breakpoint\n");
	debugPrintf(" bpenable [n] - Enables a specific breakpoint\n");
	debugPrintf(" bpdisable [n] - Disables a specific breakpoint\n");
	debugPrintf(" bplist - Lists all breakpoints\n");
	debugPrintf("\n");
	debugPrintf("GFX:\n");
	debugPrintf(" draw [cast|frame|off] - Draws debug outlines for cast or frame number\n");
	return true;
}

Common::String Breakpoint::format() const {
	Common::String result = Common::String::format("Breakpoint %d, ", id);
	switch (type) {
	case kBreakpointFunction:
		result += "Function ";
		if (scriptId)
			result += Common::String::format("%d:", scriptId);
		result += funcName;
		if (funcOffset)
			result += Common::String::format(" [%5d]", funcOffset);
		break;
	case kBreakpointMovie:
		result += "Movie " + moviePath;
		break;
	case kBreakpointMovieFrame:
		result += Common::String::format("Movie %s:%d", moviePath.c_str(), frameOffset);
		break;
	case kBreakpointProperty:
		result += "Property "+ varName + ":";
		result += varRead ? "r" : "";
		result += varWrite ? "w" : "";
		break;
	case kBreakpointVariable:
		result += "Variable "+ varName + ":";
		result += varRead ? "r" : "";
		result += varWrite ? "w" : "";
		break;
	case kBreakpointEntity:
		result += "Entity ";
		result += g_lingo->entity2str(entity);
		result += field ? ":" : "";
		result += field ? g_lingo->field2str(field) : "";
		result += ":";
		result += varRead ? "r" : "";
		result += varWrite ? "w" : "";
		break;
	case kBreakpointEvent:
		result += "Event ";
		if (eventId == kEventNone) {
			result += "none";
		} else {
			result += g_lingo->_eventHandlerTypes[eventId];
		}
		break;
	default:
		break;
	}
	return result;
}

bool Debugger::cmdVersion(int argc, const char **argv) {
	debugPrintf("Director version: %d\n", g_director->getVersion());
	debugPrintf("Director platform: %s\n", Common::getPlatformCode(g_director->getPlatform()));
	debugPrintf("Game ID: %s\n", g_director->getGameId());
	debugPrintf("Game variant: %s\n", g_director->getExtra());
	debugPrintf("Language: %s\n", Common::getLanguageCode(g_director->getLanguage()));
	debugPrintf("Expected Director version: %d\n", g_director->getDescriptionVersion());
	debugPrintf("Executable name: %s\n", g_director->getEXEName().c_str());
	debugPrintf("Startup file name: %s\n", g_director->_gameDescription->desc.filesDescriptions[0].fileName);
	debugPrintf("Startup file MD5: %s\n", g_director->_gameDescription->desc.filesDescriptions[0].md5);
	debugPrintf("\n");
	return true;
}

bool Debugger::cmdInfo(int argc, const char **argv) {
	Movie *movie = g_director->getCurrentMovie();
	Score *score = movie->getScore();
	Archive *archive = movie->getArchive();
	Cast *cast = movie->getCast();
	debugPrintf("Movie path: %s\n", archive->getPathName().toString(g_director->_dirSeparator).c_str());
	debugPrintf("Movie file size: %d\n", archive->getFileSize());
	debugPrintf("Movie archive format: %s\n", archive->formatArchiveInfo().c_str());
	debugPrintf("Movie platform: %s (%s)\n", Common::getPlatformCode(movie->_platform), Common::getPlatformDescription(movie->_platform));
	debugPrintf("Movie format version: 0x%x\n", movie->_version);

	debugPrintf("Created by: %s\n", movie->_createdBy.c_str());
	debugPrintf("Modified by: %s\n", movie->_changedBy.c_str());
	debugPrintf("Original directory: %s\n", movie->_origDirectory.c_str());
	debugPrintf("Stage size: %dx%d\n", movie->_movieRect.width(), movie->_movieRect.height());
	debugPrintf("Default palette ID: %s\n", movie->_defaultPalette.asString().c_str());
	debugPrintf("Default stage color: %d\n", cast->_stageColor);
	debugPrintf("Copy protected: %d\n", cast->_isProtected);
	debugPrintf("Remap palettes when needed flag: %d\n", movie->_remapPalettesWhenNeeded);
	debugPrintf("Allow outdated Lingo flag: %d\n", movie->_allowOutdatedLingo);
	debugPrintf("Frame count: %d\n", score->getFramesNum());
	debugPrintf("Cast member count: %d\n", cast->getCastSize());
	debugPrintf("Search paths:\n");
	if (g_lingo->_searchPath.isArray() && g_lingo->_searchPath.u.farr->arr.size() > 0) {
		for (auto &it : g_lingo->_searchPath.u.farr->arr) {
			debugPrintf("    %s\n", it.asString().c_str());
		}
	} else {
		debugPrintf("    [empty]\n");
	}
	debugPrintf("\n");
	return true;
}

bool Debugger::cmdFrame(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	Score *score = g_director->getCurrentMovie()->getScore();
	if (argc == 2 && atoi(argv[1]) > 0) {
		Datum frame, movie;
		if (atoi(argv[1]) > 0) {
			frame = Datum(atoi(argv[1]));
		} else {
			frame = Datum(argv[1]);
		}
		lingo->func_goto(frame, movie);
	} else {
		debugPrintf("%d\n", score->getCurrentFrameNum());
	}
	return true;
}

bool Debugger::cmdMovie(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	Movie *movie = g_director->getCurrentMovie();
	if (argc == 2) {
		Datum frame, mov(argv[1]);
		lingo->func_goto(frame, mov);
	} else {
		debugPrintf("%s\n", movie->getArchive()->getFileName().c_str());
	}
	return true;
}

bool Debugger::cmdChannels(int argc, const char **argv) {
	Score *score = g_director->getCurrentMovie()->getScore();

	int maxSize = (int)score->getFramesNum();
	int frameId = score->getCurrentFrameNum();
	if (argc == 1) {
		debugPrintf("Channel info for current frame %d of %d\n", frameId, maxSize);
		debugPrintf("%s\n", score->formatChannelInfo().c_str());
		return true;
	}

	if (argc == 2)
		frameId = atoi(argv[1]);

	if (frameId >= 1 && frameId <= maxSize) {
		debugPrintf("Channel info for frame %d of %d\n", frameId, maxSize);
		Frame *frame = score->_scoreCache[frameId - 1];
		if (frame) {
			debugPrintf("%s\n", frame->formatChannelInfo().c_str());
		} else {
			debugPrintf("  not found\n");
		}
	} else {
		debugPrintf("Must specify a frame number between 1 and %d.\n", maxSize);
	}
	return true;
}

bool Debugger::cmdCast(int argc, const char **argv) {
	Movie *movie = g_director->getCurrentMovie();
	Cast *sharedCast = movie->getSharedCast();

	int castId = -1;
	if (argc == 2)
		castId = atoi(argv[1]);

	for (auto it : *movie->getCasts()) {
		debugPrintf("Cast %d (%s):\n", it._key, it._value->getMacName().c_str());
		Cast *cast = it._value;
		if (!cast) {
			debugPrintf("[empty]\n");
		} else if (castId > -1 && !cast->getCastMember(castId, false)) {
			debugPrintf("[not found]\n");
		} else {
			debugPrintf("%s\n", cast->formatCastSummary(castId).c_str());
		}
		debugPrintf("\n");
	}

	debugPrintf("Shared cast:\n");
	if (!sharedCast) {
		debugPrintf("[empty]\n");
	} else if (castId > -1 && !sharedCast->getCastMember(castId, false)) {
		debugPrintf("[not found]\n");
	} else {
		debugPrintf("%s\n", sharedCast->formatCastSummary(castId).c_str());
	}
	debugPrintf("\n");

	return true;
}

bool Debugger::cmdNextFrame(int argc, const char **argv) {
	_nextFrame = true;
	if (argc == 2 && atoi(argv[1]) > 0) {
		_nextFrameCounter = atoi(argv[1]);
	} else {
		_nextFrameCounter = 1;
	}
	return cmdExit(0, nullptr);
}

bool Debugger::cmdNextMovie(int argc, const char **argv) {
	_nextMovie = true;
	return cmdExit(0, nullptr);
}

bool Debugger::cmdPrint(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Missing expression");
		return true;
	}
	Common::String command;
	for (int i = 1; i < argc; i++) {
		command += " ";
		command += argv[i];
	}
	command.trim();
	return lingoEval(command.c_str());
}

bool Debugger::cmdRepl(int argc, const char **argv) {
	debugPrintf("Switching to Lingo REPL mode, type 'lingo off' to return to the debug console.\n");
	registerDefaultCmd(WRAP_DEFAULTCOMMAND(Debugger, lingoCommandProcessor));
	_lingoReplMode = true;
	setPrompt(PROMPT);
	return true;
}

bool Debugger::cmdStack(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	debugPrintf("%s\n", lingo->formatStack().c_str());
	debugPrintf("\n");
	return true;
}

bool Debugger::cmdScriptFrame(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	debugPrintf("%s\n", lingo->formatFrame().c_str());
	debugPrintf("%s\n", lingo->formatCurrentInstruction().c_str());
	return true;
}

bool Debugger::cmdFuncs(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	Movie *movie = g_director->getCurrentMovie();
	Score *score = movie->getScore();
	ScriptContext *csc = lingo->_state->context;
	if (csc) {
		debugPrintf("Functions attached to frame %d:\n", score->getCurrentFrameNum());
		debugPrintf("  %d:", csc->_id);
		debugPrintf("%s", csc->formatFunctionList("    ").c_str());
	} else {
		debugPrintf("Functions attached to frame %d:\n", score->getCurrentFrameNum());
		debugPrintf("  [empty]\n");
	}
	debugPrintf("\n");
	for (auto it : *movie->getCasts()) {
		debugPrintf("Cast %d functions:\n", it._key);
		Cast *cast = it._value;
		if (cast && cast->_lingoArchive) {
			debugPrintf("%s", cast->_lingoArchive->formatFunctionList("  ").c_str());
		} else {
			debugPrintf("  [empty]\n");
		}
		debugPrintf("\n");
	}
	debugPrintf("Shared cast functions:\n");
	Cast *sharedCast = movie->getSharedCast();
	if (sharedCast && sharedCast->_lingoArchive) {
		debugPrintf("%s", sharedCast->_lingoArchive->formatFunctionList("  ").c_str());
	} else {
		debugPrintf("  [empty]\n");
	}
	debugPrintf("\n");
	debugPrintf("Frame script mappings:\n");
	for (int i = 0; i < (int)score->_scoreCache.size(); i++) {
		Frame *frame = score->_scoreCache[i];
		if (frame && frame->_mainChannels.actionId.member) {
			debugPrintf("  %d: %s\n", i + 1, frame->_mainChannels.actionId.asString().c_str());
		}
	}
	debugPrintf("Sprite script mappings:\n");
	for (int i = 0; i < (int)score->_scoreCache.size(); i++) {
		Frame *frame = score->_scoreCache[i];
		if (frame) {
			for (int j = 0; j < (int)frame->_sprites.size(); j++) {
				Sprite *sprite = frame->_sprites[j];
				if (sprite->_scriptId.member) {
					debugPrintf("  %d, sprite %d: %s\n", i + 1, j, sprite->_scriptId.asString().c_str());
				}
			}
		}
	}


	return true;
}

bool Debugger::cmdActions(int argc, const char **argv) {
	Movie *movie = g_director->getCurrentMovie();
	Score *score = movie->getScore();
	debugPrintf("Actions:\n");
	for (auto &it : score->_actions) {
		debugPrintf("  %d:\n", it._key);
		debugPrintf("%s\n", formatStringForDump(it._value).c_str());
	}
	debugPrintf("D3 movie script:\n");
	debugPrintf("%s\n", formatStringForDump(movie->_script).c_str());

	return true;
}

bool Debugger::cmdBacktrace(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	debugPrintf("%s\n", lingo->formatCallStack(lingo->_state->pc).c_str());
	return true;
}

bool Debugger::cmdDisasm(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	if (argc == 2) {
		if (!strcmp(argv[1], "all")) {
			Movie *movie = g_director->getCurrentMovie();
			Score *score = movie->getScore();
			Cast *targets[2] = {movie->getCast(), movie->getSharedCast()};
			const char *targetNames[2] = {"Cast", "Shared cast"};
			ScriptContext *csc = lingo->_state->context;
			if (csc) {
				debugPrintf("Functions attached to frame %d:\n", score->getCurrentFrameNum());
				for (auto &it : csc->_functionHandlers) {
					debugPrintf("%s\n\n", g_lingo->formatFunctionBody(it._value).c_str());
				}
			} else {
				debugPrintf("Functions attached to frame %d:\n", score->getCurrentFrameNum());
				debugPrintf("  [empty]\n");
			}
			debugPrintf("\n");
			for (int t = 0; t < 2; t++) {
				debugPrintf("%s functions:\n", targetNames[t]);
				Cast *cast = targets[t];
				if (cast && cast->_lingoArchive) {
					for (int i = 0; i <= kMaxScriptType; i++) {
						debugPrintf("  %s:\n", scriptType2str((ScriptType)i));
						if (cast->_lingoArchive->scriptContexts[i].size() == 0)
							debugPrintf("    [empty]\n");

						for (auto &it : cast->_lingoArchive->scriptContexts[i]) {
							for (auto &jt : it._value->_functionHandlers) {
								debugPrintf("%s\n", g_lingo->formatFunctionBody(jt._value).c_str());
							}
						}
					}
					debugPrintf("  Factories:\n");
					if (cast->_lingoArchive->factoryContexts.empty()) {
						debugPrintf("    [empty]\n");
					} else {
						for (auto it : cast->_lingoArchive->factoryContexts) {
							debugPrintf("  %d:\n", it._key);
							if (it._value->empty()) {
								debugPrintf("    [empty]\n");
							} else {
								for (auto jt : *it._value) {
									debugPrintf("    %s:\n", jt._key.c_str());
									for (auto &kt : jt._value->_functionHandlers) {
										debugPrintf("%s\n", g_lingo->formatFunctionBody(kt._value).c_str());
									}
								}
							}
						}
					}
				} else {
					debugPrintf("  [empty]\n");
				}
				debugPrintf("\n");
			}
			return true;
		}
		Common::String target(argv[1]);
		uint splitPoint = target.findFirstOf(":");
		if (splitPoint == Common::String::npos) {
			debugPrintf("Must provide target in format scriptid:funcname.\n");
			return true;
		}
		Common::String scriptIdStr = target.substr(0, splitPoint);
		int scriptId = atoi(scriptIdStr.c_str());
		if (!scriptId) {
			debugPrintf("Invalid scriptid, must be an integer.\n");
			return true;
		}

		Common::String funcName = target.substr(splitPoint + 1, Common::String::npos);
		Movie *movie = g_director->getCurrentMovie();
		Cast *targets[2] = {movie->getCast(), movie->getSharedCast()};

		for (int i = 0; i < 2; i++) {
			Cast *cast = targets[i];
			if (cast && cast->_lingoArchive) {
				ScriptContext *ctx = cast->_lingoArchive->findScriptContext(scriptId);
				if (ctx && ctx->_functionHandlers.contains(funcName)) {
					debugPrintf("%s\n", lingo->formatFunctionBody(ctx->_functionHandlers[funcName]).c_str());
					return true;
				}
				if (cast->_lingoArchive->factoryContexts.contains(scriptId)) {
					for (auto &it : *cast->_lingoArchive->factoryContexts.getVal(scriptId)) {
						Common::String prefix = Common::String::format("%s:", it._key.c_str());
						if (funcName.hasPrefixIgnoreCase(prefix)) {
							Common::String handler = funcName.substr(prefix.size());
							if (it._value->_functionHandlers.contains(handler)) {
								debugPrintf("%s\n", lingo->formatFunctionBody(it._value->_functionHandlers[handler]).c_str());
								return true;
							}
						}
					}
				}
			}
		}
	} else {
		Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;
		if (callstack.size() == 0) {
			debugPrintf("Lingo is not executing, nothing to disassemble.\n");
			return true;
		} else {
			CFrame *frame = callstack[callstack.size() - 1];
			debugPrintf("%s\n", lingo->formatFunctionBody(frame->sp).c_str());
			return true;
		}
	}
	debugPrintf("Script not found.\n");
	return true;
}

bool Debugger::cmdVar(int argc, const char **argv) {
	Lingo *lingo = g_director->getLingo();
	debugPrintf("%s\n", lingo->formatAllVars().c_str());
	return true;
}

bool Debugger::cmdMarkers(int argc, const char **argv) {
	Score *score = g_director->getCurrentMovie()->getScore();
	if (score->_labels && score->_labels->size()) {
		debugPrintf("Score markers:\n");
		for (auto &it : *score->_labels) {
			debugPrintf("\"%s\" -> %d\n", it->name.c_str(), it->number);
		}
	} else {
		debugPrintf("No score markers found.\n");
	}
	return true;
}

bool Debugger::cmdStep(int argc, const char **argv) {
	_step = true;
	if (argc == 2 && atoi(argv[1]) > 0) {
		_stepCounter = atoi(argv[1]);
	} else {
		_stepCounter = 1;
	}
	return cmdExit(0, nullptr);
}

bool Debugger::cmdNext(int argc, const char **argv) {
	_step = true;
	_next = true;
	if (argc == 2 && atoi(argv[1]) > 0) {
		_stepCounter = atoi(argv[1]);
	} else {
		_stepCounter = 1;
	}
	return cmdExit(0, nullptr);
}

bool Debugger::cmdFinish(int argc, const char **argv) {
	_finish = true;
	_finishCounter = 1;
	return cmdExit(0, nullptr);
}

bool Debugger::cmdBpSet(int argc, const char **argv) {
	Breakpoint bp;
	bp.type = kBreakpointFunction;
	if (argc == 1) {
		Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;
		if (callstack.size() == 0) {
			debugPrintf("Lingo is not executing, no current function to add breakpoint to.\n");
			return true;
		}
		CFrame *frame = callstack[callstack.size() - 1];
		if (!frame->sp.ctx) {
			debugPrintf("Unable to add breakpoint, current script context is not addressable.\n");
			return true;
		}
		if (!frame->sp.name) {
			debugPrintf("Unable to add breakpoint, current function is not addressable.\n");
			return true;
		}
		bp.scriptId = frame->sp.ctx->_id;
		bp.funcName = *frame->sp.name;
		bp.funcOffset = g_lingo->_state->pc;
	} else if (argc == 2 || argc == 3) {
		Common::String target(argv[1]);
		uint splitPoint = target.findFirstOf(":");
		if (splitPoint == Common::String::npos) {
			if (argc == 2 && atoi(argv[1]) > 0) {
				// first and only argument is a number, use as an offset for the current function
				Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;
				if (callstack.size() == 0) {
					debugPrintf("Lingo is not executing, no current function to add breakpoint to.\n");
					return true;
				}
				CFrame *frame = callstack[callstack.size() - 1];
				if (!frame->sp.ctx) {
					debugPrintf("Unable to add breakpoint, current script context is not addressable.\n");
					return true;
				}
				if (!frame->sp.name) {
					debugPrintf("Unable to add breakpoint, current function is not addressable.\n");
					return true;
				}
				bp.scriptId = frame->sp.ctx->_id;
				bp.funcName = *frame->sp.name;
				bp.funcOffset = atoi(argv[1]);
			} else {
				// first argument is a string, do a function name match
				bp.funcName = target;
			}
		} else {
			// first argument is n:funcname, do an exact function match
			bp.scriptId = atoi(target.substr(0, splitPoint).c_str());
			bp.funcName = target.substr(splitPoint + 1, Common::String::npos);
		}
		if (argc == 3) {
			// if there's a second argument, use it as the function offset
			bp.funcOffset = atoi(argv[2]);
		}
	} else {
		debugPrintf("Too many arguments.\n");
		return true;
	}
	g_lingo->addBreakpoint(bp);
	bpUpdateState();
	debugPrintf("Added %s\n", bp.format().c_str());
	return true;
}

bool Debugger::cmdBpMovie(int argc, const char **argv) {
	if (argc == 2) {
		Breakpoint bp;
		bp.type = kBreakpointMovie;
		bp.moviePath = argv[1];
		g_lingo->addBreakpoint(bp);
		bpUpdateState();
		debugPrintf("Added %s\n", bp.format().c_str());
	} else {
		debugPrintf("Must specify a movie path.\n");
	}
	return true;
}

bool Debugger::cmdBpEntity(int argc, const char **argv) {
	if (argc == 2 || argc == 3) {
		Breakpoint bp;
		bp.type = kBreakpointEntity;
		Common::String entityName = Common::String(argv[1]);
		Common::String fieldName;
		uint splitPoint = entityName.findFirstOf(":");
		if (splitPoint != Common::String::npos) {
			fieldName = entityName.substr(splitPoint + 1, Common::String::npos);
			entityName = entityName.substr(0, splitPoint);
		}
		if (!g_lingo->_theEntities.contains(entityName)) {
			debugPrintf("Entity %s not found.\n", entityName.c_str());
			return true;
		}
		bp.entity = g_lingo->_theEntities[entityName]->entity;
		if (!fieldName.empty()) {
			Common::String target = Common::String::format("%d%s", bp.entity, fieldName.c_str());
			if (!g_lingo->_theEntityFields.contains(target)) {
				debugPrintf("Field %s not found for entity %s.\n", fieldName.c_str(), entityName.c_str());
				return true;
			}
			bp.field = g_lingo->_theEntityFields[target]->field;
		}

		if (argc == 3) {
			Common::String props = argv[2];
			bp.varRead = props.contains("r") || props.contains("R");
			bp.varWrite = props.contains("w") || props.contains("W");
			if (!(bp.varRead || bp.varWrite)) {
				debugPrintf("Must specify r, w, or rw.\n");
				return true;
			}
		} else {
			bp.varRead = true;
			bp.varWrite = true;
		}
		g_lingo->addBreakpoint(bp);
		bpUpdateState();
		debugPrintf("Added %s\n", bp.format().c_str());
	} else {
		debugPrintf("Must specify a variable.\n");
	}
	return true;
}

bool Debugger::cmdBpProp(int argc, const char **argv) {
	if (argc == 2 || argc == 3) {
		Breakpoint bp;
		bp.type = kBreakpointProperty;
		bp.varName = argv[1];
		if (argc == 3) {
			Common::String props = argv[2];
			bp.varRead = props.contains("r") || props.contains("R");
			bp.varWrite = props.contains("w") || props.contains("W");
			if (!(bp.varRead || bp.varWrite)) {
				debugPrintf("Must specify r, w, or rw.");
				return true;
			}
		} else {
			bp.varRead = true;
			bp.varWrite = true;
		}
		g_lingo->addBreakpoint(bp);
		bpUpdateState();
		debugPrintf("Added %s\n", bp.format().c_str());
	} else {
		debugPrintf("Must specify a property.\n");
	}
	return true;
}

bool Debugger::cmdBpVar(int argc, const char **argv) {
	if (argc == 2 || argc == 3) {
		Breakpoint bp;
		bp.type = kBreakpointVariable;
		bp.varName = argv[1];
		if (argc == 3) {
			Common::String props = argv[2];
			bp.varRead = props.contains("r") || props.contains("R");
			bp.varWrite = props.contains("w") || props.contains("W");
			if (!(bp.varRead || bp.varWrite)) {
				debugPrintf("Must specify r, w, or rw.");
				return true;
			}
		} else {
			bp.varRead = true;
			bp.varWrite = true;
		}
		g_lingo->addBreakpoint(bp);
		bpUpdateState();
		debugPrintf("Added %s\n", bp.format().c_str());
	} else {
		debugPrintf("Must specify a variable.\n");
	}
	return true;
}

bool Debugger::cmdBpEvent(int argc, const char **argv) {
	if (argc == 2) {
		Breakpoint bp;
		bp.type = kBreakpointEvent;
		for (auto &it : g_lingo->_eventHandlerTypeIds) {
			if (it._key.equalsIgnoreCase(argv[1])) {
				bp.eventId = (LEvent)it._value;
				break;
			}
		}
		if (bp.eventId == kEventNone) {
			debugPrintf("Event %s not found.\n", argv[1]);
			return true;
		}
		g_lingo->addBreakpoint(bp);
		bpUpdateState();
		debugPrintf("Added %s\n", bp.format().c_str());
	} else {
		debugPrintf("Must specify an event name. Choices are:\n");
		for (auto &it : g_lingo->_eventHandlerTypeIds) {
			debugPrintf("%s ", it._key.c_str());
		}
		debugPrintf("\n");
	}
	return true;
}

bool Debugger::cmdBpFrame(int argc, const char **argv) {
	Movie *movie = g_director->getCurrentMovie();
	if (argc == 2 || argc == 3) {
		Breakpoint bp;
		bp.type = kBreakpointMovieFrame;
		Common::String target(argv[1]);
		if (argc == 3) {
			bp.moviePath = argv[1];
			bp.frameOffset = atoi(argv[2]);
		} else {
			bp.moviePath = movie->getArchive()->getFileName();
			bp.frameOffset = atoi(argv[1]);
		}
		if (bp.frameOffset == 0) {
			debugPrintf("Must specify a valid frame ID.\n");
			return true;
		}
		g_lingo->addBreakpoint(bp);
		bpUpdateState();
		debugPrintf("Added %s\n", bp.format().c_str());
	} else {
		debugPrintf("Must specify a valid frame ID.\n");
	}
	return true;
}

bool Debugger::cmdBpDel(int argc, const char **argv) {
	if (argc == 2 && atoi(argv[1]) > 0) {
		if (g_lingo->delBreakpoint(atoi(argv[1]))) {
			debugPrintf("Deleted breakpoint %s.\n", argv[1]);
		} else {
			debugPrintf("No breakpoint with ID %s.\n", argv[1]);
		}
		bpUpdateState();
	} else {
		debugPrintf("Must specify a breakpoint ID.\n");
	}
	return true;
}

bool Debugger::cmdBpEnable(int argc, const char **argv) {
	if (argc == 2 && atoi(argv[1]) > 0) {
		Breakpoint *bp = g_lingo->getBreakpoint(atoi(argv[1]));
		if (bp) {
			bp->enabled = true;
			bpUpdateState();
			debugPrintf("Enabled breakpoint %s.\n", argv[1]);
		} else
			debugPrintf("No breakpoint with ID %s.\n", argv[1]);
	} else {
		debugPrintf("Must specify a breakpoint ID.\n");
	}
	return true;
}

bool Debugger::cmdBpDisable(int argc, const char **argv) {
	if (argc == 2 && atoi(argv[1]) > 0) {
		Breakpoint *bp = g_lingo->getBreakpoint(atoi(argv[1]));
		if (bp) {
			bp->enabled = false;
			bpUpdateState();
			debugPrintf("Disabled breakpoint %s.\n", argv[1]);
		} else
			debugPrintf("No breakpoint with ID %s.\n", argv[1]);
	} else {
		debugPrintf("Must specify a breakpoint ID.\n");
	}
	return true;
}

bool Debugger::cmdBpList(int argc, const char **argv) {
	const Common::Array<Breakpoint> &bps = g_lingo->getBreakpoints();
	if (bps.size()) {
		for (auto &it : bps) {
			debugPrintf("%s (%s)\n", it.format().c_str(), it.enabled ? "enabled" : "disabled");
		}
	} else {
		debugPrintf("No breakpoints set.\n");
	}
	return true;
}

bool Debugger::cmdDraw(int argc, const char **argv) {
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (!scumm_stricmp(argv[i], "off")) {
				g_director->_debugDraw = 0;
			} else if (!strncmp(argv[i], "cast", 4)) { // allow "castS"
				g_director->_debugDraw |= kDebugDrawCast;
			} else if (!strncmp(argv[i], "frame", 5)) { // allow "frameS"
				g_director->_debugDraw |= kDebugDrawFrame;
			} else if (!scumm_stricmp(argv[i], "all")) {
				g_director->_debugDraw |= kDebugDrawCast | kDebugDrawFrame;
			} else {
				debugPrintf("Valid parameters are 'cast', 'frame', 'all' or 'off'.\n");
				return true;
			}
		}
	}

	debugPrintf("Draw: ");
	if (g_director->_debugDraw & kDebugDrawCast)
		debugPrintf("cast ");

	if (g_director->_debugDraw & kDebugDrawFrame)
		debugPrintf("frame ");

	if (!g_director->_debugDraw)
		debugPrintf("off ");

	debugPrintf("\n");

	return true;
}

static void forceWindowRedraw(Window *window) {
	if (!window->getCurrentMovie())
		return;

	Score *score = window->getCurrentMovie()->getScore();

	if (!score)
		return;

	for (uint16 c = 0; c < score->_channels.size(); c++)
		score->_channels[c]->_dirty = true;
}

bool Debugger::cmdForceRedraw(int argc, const char **argv) {
	forceWindowRedraw(g_director->getStage());

	FArray *windowList = g_lingo->_windowList.u.farr;
	for (uint i = 0; i < windowList->arr.size(); i++) {
		if (windowList->arr[i].type != OBJECT || windowList->arr[i].u.obj->getObjType() != kWindowObj)
			continue;

		Window *window = static_cast<Window *>(windowList->arr[i].u.obj);

		forceWindowRedraw(window);
	}

	debugPrintf("Requested full refresh\n");

	return true;
}

void Debugger::bpUpdateState() {
	_bpCheckFunc = false;
	_bpCheckMoviePath = false;
	_bpNextMovieMatch = false;
	_bpMatchFuncOffsets.clear();
	_bpMatchFuncName.clear();
	_bpMatchScriptId = 0;
	_bpMatchMoviePath.clear();
	_bpMatchFrameOffsets.clear();
	_bpCheckPropRead = false;
	_bpCheckPropWrite = false;
	_bpCheckVarRead = false;
	_bpCheckVarWrite = false;
	_bpCheckEntityRead = false;
	_bpCheckEntityWrite = false;
	_bpCheckEvent = false;
	Movie *movie = g_director->getCurrentMovie();
	Common::Array<CFrame *> &callstack = g_lingo->_state->callstack;
	for (auto &it : g_lingo->getBreakpoints()) {
		if (!it.enabled)
			continue;
		if (it.type == kBreakpointFunction) {
			_bpCheckFunc = true;
			if (!callstack.size())
				continue;
			CFrame *head = callstack[callstack.size() - 1];
			if (!head->sp.name)
				continue;
			if (!head->sp.ctx)
				continue;
			// check for a straight function name match
			bool nameTest = it.funcName.equalsIgnoreCase(*head->sp.name);
			if (head->sp.ctx->isFactory()) {
				// for factories, check for a "FactoryName:funcName" match
				nameTest |= it.funcName.equalsIgnoreCase(Common::String::format("%s:%s", head->sp.ctx->getName().c_str(), head->sp.name->c_str()));
			}
			if (nameTest) {
				if (it.scriptId) {
					if (it.scriptId == head->sp.ctx->_id) {
						_bpMatchScriptId = head->sp.ctx->_id;
						_bpMatchFuncName = it.funcName;
						_bpMatchFuncOffsets.setVal(it.funcOffset, nullptr);
					}
				} else {
					_bpMatchFuncName = it.funcName;
					_bpMatchFuncOffsets.setVal(it.funcOffset, nullptr);
				}
			}
		} else if (it.type == kBreakpointMovie || it.type == kBreakpointMovieFrame) {
			_bpCheckMoviePath = true;
			if (it.moviePath.equalsIgnoreCase(movie->getArchive()->getFileName())) {
				_bpNextMovieMatch |= it.type == kBreakpointMovie;
				_bpMatchMoviePath = it.moviePath;
				_bpMatchFrameOffsets.setVal(it.frameOffset, nullptr);
			}
		} else if (it.type == kBreakpointProperty) {
			_bpCheckPropRead |= it.varRead;
			_bpCheckPropWrite |= it.varWrite;
		} else if (it.type == kBreakpointVariable) {
			_bpCheckVarRead |= it.varRead;
			_bpCheckVarWrite |= it.varWrite;
		} else if (it.type == kBreakpointEntity) {
			_bpCheckEntityRead |= it.varRead;
			_bpCheckEntityWrite |= it.varWrite;
		} else if (it.type == kBreakpointEvent) {
			_bpCheckEvent = true;
		}
	}
}

void Debugger::bpTest(bool forceCheck) {
	// don't check for breakpoints if we're in eval mode
	if (_lingoEval)
		return;

	// Check if there's a funcName/offset or frame/movie match
	bool stop = forceCheck;
	uint funcOffset = g_lingo->_state->pc;
	Score *score = g_director->getCurrentMovie()->getScore();
	uint frameOffset = score->getCurrentFrameNum();
	if (_bpCheckFunc) {
		stop |= _bpMatchFuncOffsets.contains(funcOffset);
	}
	if (_bpCheckMoviePath) {
		stop |= _bpMatchFrameOffsets.contains(frameOffset);
	}

	// Print the breakpoints that matched
	if (stop) {
		debugPrintf("Hit a breakpoint:\n");
		for (auto &it : g_lingo->getBreakpoints()) {
			if (!it.enabled)
				continue;
			if (it.type == kBreakpointFunction) {
				if (it.funcName.equalsIgnoreCase(_bpMatchFuncName) && it.scriptId == _bpMatchScriptId && it.funcOffset == funcOffset)
					debugPrintf("%s\n", it.format().c_str());
			} else if (it.type == kBreakpointMovie && _bpNextMovieMatch) {
				if (it.moviePath.equalsIgnoreCase(_bpMatchMoviePath))
					debugPrintf("%s\n", it.format().c_str());
			} else if (it.type == kBreakpointMovieFrame) {
				if (it.moviePath.equalsIgnoreCase(_bpMatchMoviePath) && it.frameOffset == frameOffset)
					debugPrintf("%s\n", it.format().c_str());
			}
		}
		// reset all step commands before returning to console
		_nextMovie = false;
		_nextFrame = false;
		cmdScriptFrame(0, nullptr);
		attach();
		g_system->updateScreen();
	}
}

bool Debugger::lingoCommandProcessor(const char *inputOrig) {
	if (!strcmp(inputOrig, "lingo off")) {
		registerDefaultCmd(nullptr);
		_lingoReplMode = false;
		resetPrompt();
		return true;
	}
	bool ret = lingoEval(inputOrig);

	return ret;
}

bool Debugger::lingoEval(const char *inputOrig) {
	Common::String inputSan = inputOrig;
	inputSan.trim();
	if (inputSan.empty())
		return true;

	// Compile the code to an anonymous function and call it
	ScriptContext *sc = g_lingo->_compiler->compileAnonymous(inputSan);
	if (!sc) {
		debugPrintf("Failed to parse expression!\n");
		return true;
	}
	Symbol sym = sc->_eventHandlers[kEventGeneric];
	_lingoEval = true;
	LC::call(sym, 0, true);
	g_lingo->execute();

	debugPrintf("\n");
	return true;
}

void Debugger::stepHook() {
	bpTest();
	if (_step && _nextCounter == 0) {
		_stepCounter--;
		if (_stepCounter == 0) {
			_step = false;
			_next = false;
			cmdScriptFrame(0, nullptr);
			attach();
			g_system->updateScreen();
		}
	}
	if (_finish && _finishCounter == 0) {
		_finish = false;
		if (_lingoEval) {
			_lingoEval = false;
			Datum result = g_lingo->pop();
			debugPrintf("%s\n\n", result.asString(true).c_str());
		} else {
			cmdScriptFrame(0, nullptr);
		}
		attach();
		g_system->updateScreen();
	}
}

void Debugger::frameHook() {
	bpTest();
	if (_nextFrame) {
		_nextFrameCounter--;
		if (_nextFrameCounter == 0) {
			_nextFrame = false;
			cmdFrame(0, nullptr);
			attach();
			g_system->updateScreen();
		}
	}
}

void Debugger::movieHook() {
	bpUpdateState();
	bpTest(_bpNextMovieMatch);
	if (_nextMovie) {
		_nextMovie = false;
		cmdMovie(0, nullptr);
		attach();
		g_system->updateScreen();
	}
}

void Debugger::eventHook(LEvent eventId) {
	if (_bpCheckEvent) {
		for (auto &it : g_lingo->getBreakpoints()) {
			if (it.type == kBreakpointEvent && eventId == it.eventId) {
				debugPrintf("Hit a breakpoint:\n");
				debugPrintf("%s\n", it.format().c_str());
				cmdScriptFrame(0, nullptr);
				attach();
				g_system->updateScreen();
				break;
			}
		}
	}
}

void Debugger::pushContextHook() {
	if (_next)
		_nextCounter++;
	if (_finish)
		_finishCounter++;
	bpUpdateState();
}

void Debugger::popContextHook() {
	if (_next && _nextCounter > 0)
		_nextCounter--;
	if (_finish)
		_finishCounter--;
	bpUpdateState();
}

void Debugger::builtinHook(const Symbol &funcSym) {
	if (!funcSym.name)
		return;
	bpUpdateState();
	bool builtinMatch = false;
	if (_bpCheckFunc) {
		for (auto &it : g_lingo->getBreakpoints()) {
			if (it.type != kBreakpointFunction)
				continue;
			if (it.funcName.equalsIgnoreCase(*funcSym.name)) {
				builtinMatch = true;
				break;
			}
		}
	}
	bpTest(builtinMatch);
}

void Debugger::propReadHook(const Common::String &name) {
	if (name.empty())
		return;
	if (_bpCheckPropRead) {
		for (auto &it : g_lingo->getBreakpoints()) {
			if (it.type == kBreakpointProperty && it.varRead && it.varName.equalsIgnoreCase(name)) {
				debugPrintf("Hit a breakpoint:\n");
				debugPrintf("%s\n", it.format().c_str());
				cmdScriptFrame(0, nullptr);
				attach();
				g_system->updateScreen();
				break;
			}
		}
	}
}

void Debugger::propWriteHook(const Common::String &name) {
	if (name.empty())
		return;
	if (_bpCheckPropWrite) {
		for (auto &it : g_lingo->getBreakpoints()) {
			if (it.type == kBreakpointProperty && it.varWrite && it.varName.equalsIgnoreCase(name)) {
				debugPrintf("Hit a breakpoint:\n");
				debugPrintf("%s\n", it.format().c_str());
				cmdScriptFrame(0, nullptr);
				attach();
				g_system->updateScreen();
				break;
			}
		}
	}
}

void Debugger::varReadHook(const Common::String &name) {
	if (name.empty())
		return;
	if (_bpCheckVarRead) {
		for (auto &it : g_lingo->getBreakpoints()) {
			if (it.type == kBreakpointVariable && it.varRead && it.varName.equalsIgnoreCase(name)) {
				debugPrintf("Hit a breakpoint:\n");
				debugPrintf("%s\n", it.format().c_str());
				cmdScriptFrame(0, nullptr);
				attach();
				g_system->updateScreen();
				break;
			}
		}
	}
}

void Debugger::varWriteHook(const Common::String &name) {
	if (name.empty())
		return;
	if (_bpCheckVarWrite) {
		for (auto &it : g_lingo->getBreakpoints()) {
			if (it.type == kBreakpointVariable && it.varWrite && it.varName.equalsIgnoreCase(name)) {
				debugPrintf("Hit a breakpoint:\n");
				debugPrintf("%s\n", it.format().c_str());
				cmdScriptFrame(0, nullptr);
				attach();
				g_system->updateScreen();
				break;
			}
		}
	}
}

void Debugger::entityReadHook(int entity, int field) {
	if (_bpCheckEntityRead) {
		for (auto &it : g_lingo->getBreakpoints()) {
			if (it.type == kBreakpointEntity && it.varRead && it.entity == entity && it.field == field) {
				debugPrintf("Hit a breakpoint:\n");
				debugPrintf("%s\n", it.format().c_str());
				cmdScriptFrame(0, nullptr);
				attach();
				g_system->updateScreen();
				break;
			}
		}
	}
}

void Debugger::entityWriteHook(int entity, int field) {
	if (_bpCheckEntityWrite) {
		for (auto &it : g_lingo->getBreakpoints()) {
			if (it.type == kBreakpointEntity && it.varWrite && it.entity == entity && it.field == field) {
				debugPrintf("Hit a breakpoint:\n");
				debugPrintf("%s\n", it.format().c_str());
				cmdScriptFrame(0, nullptr);
				attach();
				g_system->updateScreen();
				break;
			}
		}
	}
}

void Debugger::debugLogFile(Common::String logs, bool prompt) {
	if (prompt)
		debugPrintf("-- %s", logs.c_str());
	else
		debugPrintf("%s", logs.c_str());
	if (g_director->_traceLogFile.empty()) {
		if (_out.isOpen())
			_out.close();
		_outName.clear();
	} else {
		if (_outName != g_director->_traceLogFile) {
			if (_out.isOpen())
				_out.close();
			if (!_out.open(g_director->_traceLogFile, true))
				return;
			_outName = g_director->_traceLogFile;
		}
		if(_out.isOpen()) {
			_out.seek(_out.size());
			_out.write(logs.c_str(), logs.size());
			_out.flush();
		}
	}
}

} // End of namespace Director
