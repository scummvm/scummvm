/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <memory>
#include <limits>
#include "core/platform.h"
#include "ac/common.h"
#include "ac/gamesetupstruct.h"
#include "ac/runtime_defines.h"
#include "debug/agseditordebugger.h"
#include "debug/debug_log.h"
#include "debug/debugger.h"
#include "debug/debugmanager.h"
#include "debug/out.h"
#include "debug/consoleoutputtarget.h"
#include "debug/logfile.h"
#include "debug/messagebuffer.h"
#include "main/config.h"
#include "media/audio/audio_system.h"
#include "platform/base/agsplatformdriver.h"
#include "plugin/plugin_engine.h"
#include "script/script.h"
#include "script/script_common.h"
#include "script/cc_error.h"
#include "util/string_utils.h"
#include "util/textstreamwriter.h"

#if AGS_PLATFORM_OS_WINDOWS
#include <winalleg.h>
#endif

using namespace AGS::Shared;
using namespace AGS::Engine;

extern char check_dynamic_sprites_at_exit;
extern int displayed_room;
extern RoomStruct thisroom;
extern char pexbuf[STD_BUFFER_SIZE];
extern volatile char want_exit, abort_engine;
extern GameSetupStruct game;


int editor_debugging_enabled = 0;
int editor_debugging_initialized = 0;
char editor_debugger_instance_token[100];
IAGSEditorDebugger *editor_debugger = nullptr;
int break_on_next_script_step = 0;
volatile int game_paused_in_debugger = 0;

#if AGS_PLATFORM_OS_WINDOWS

#include "platform/windows/debug/namedpipesagsdebugger.h"

HWND editor_window_handle = 0;

IAGSEditorDebugger *GetEditorDebugger(const char *instanceToken) {
	return new NamedPipesAGSDebugger(instanceToken);
}

#else   // AGS_PLATFORM_OS_WINDOWS

IAGSEditorDebugger *GetEditorDebugger(const char *instanceToken) {
	return nullptr;
}

#endif

int debug_flags = 0;

String debug_line[DEBUG_CONSOLE_NUMLINES];
int first_debug_line = 0, last_debug_line = 0, display_console = 0;

float fps = std::numeric_limits<float>::quiet_NaN();
FPSDisplayMode display_fps = kFPS_Hide;

std::unique_ptr<MessageBuffer> DebugMsgBuff;
std::unique_ptr<LogFile> DebugLogFile;
std::unique_ptr<ConsoleOutputTarget> DebugConsole;

const String OutputMsgBufID = "buffer";
const String OutputFileID = "file";
const String OutputSystemID = "stdout";
const String OutputGameConsoleID = "console";



PDebugOutput create_log_output(const String &name, const String &path = "", LogFile::OpenMode open_mode = LogFile::kLogFile_Overwrite) {
	// Else create new one, if we know this ID
	if (name.CompareNoCase(OutputSystemID) == 0) {
		return DbgMgr.RegisterOutput(OutputSystemID, AGSPlatformDriver::GetDriver(), kDbgMsg_None);
	} else if (name.CompareNoCase(OutputFileID) == 0) {
		DebugLogFile.reset(new LogFile());
		String logfile_path = !path.IsEmpty() ? path : String::FromFormat("%s/ags.log", platform->GetAppOutputDirectory());
		if (!DebugLogFile->OpenFile(logfile_path, open_mode))
			return nullptr;
		platform->WriteStdOut("Logging to %s", logfile_path.GetCStr());
		auto dbgout = DbgMgr.RegisterOutput(OutputFileID, DebugLogFile.get(), kDbgMsg_None);
		return dbgout;
	} else if (name.CompareNoCase(OutputGameConsoleID) == 0) {
		DebugConsole.reset(new ConsoleOutputTarget());
		return DbgMgr.RegisterOutput(OutputGameConsoleID, DebugConsole.get(), kDbgMsg_None);
	}
	return nullptr;
}

// Parses a string where each character defines a single log group; returns list of real group names.
std::vector<String> parse_log_multigroup(const String &group_str) {
	std::vector<String> grplist;
	for (size_t i = 0; i < group_str.GetLength(); ++i) {
		switch (group_str[i]) {
		case 'm':
			grplist.push_back("main");
			break;
		case 'g':
			grplist.push_back("game");
			break;
		case 's':
			grplist.push_back("script");
			break;
		case 'c':
			grplist.push_back("sprcache");
			break;
		case 'o':
			grplist.push_back("manobj");
			break;
		}
	}
	return grplist;
}

MessageType get_messagetype_from_string(const String &mt) {
	int mtype;
	if (StrUtil::StringToInt(mt, mtype, 0) == StrUtil::kNoError)
		return (MessageType)mtype;

	if (mt.CompareNoCase("alert") == 0) return kDbgMsg_Alert;
	else if (mt.CompareNoCase("fatal") == 0) return kDbgMsg_Fatal;
	else if (mt.CompareNoCase("error") == 0) return kDbgMsg_Error;
	else if (mt.CompareNoCase("warn") == 0) return kDbgMsg_Warn;
	else if (mt.CompareNoCase("info") == 0) return kDbgMsg_Info;
	else if (mt.CompareNoCase("debug") == 0) return kDbgMsg_Debug;
	else if (mt.CompareNoCase("all") == 0) return kDbgMsg_All;
	return kDbgMsg_None;
}

typedef std::pair<CommonDebugGroup, MessageType> DbgGroupOption;

void apply_log_config(const ConfigTree &cfg, const String &log_id,
                      bool def_enabled,
                      std::initializer_list<DbgGroupOption> def_opts) {
	String value = INIreadstring(cfg, "log", log_id);
	if (value.IsEmpty() && !def_enabled)
		return;

	// First test if already registered, if not then try create it
	auto dbgout = DbgMgr.GetOutput(log_id);
	const bool was_created_earlier = dbgout != nullptr;
	if (!dbgout) {
		String path = INIreadstring(cfg, "log", String::FromFormat("%s-path", log_id.GetCStr()));
		dbgout = create_log_output(log_id, path);
		if (!dbgout)
			return; // unknown output type
	}
	dbgout->ClearGroupFilters();

	if (value.IsEmpty() || value.CompareNoCase("default") == 0) {
		for (const auto opt : def_opts)
			dbgout->SetGroupFilter(opt.first, opt.second);
	} else {
		const auto options = value.Split(',');
		for (const auto &opt : options) {
			String groupname = opt.LeftSection(':');
			MessageType msgtype = kDbgMsg_All;
			if (opt.GetLength() >= groupname.GetLength() + 1) {
				String msglevel = opt.Mid(groupname.GetLength() + 1);
				msglevel.Trim();
				if (msglevel.GetLength() > 0)
					msgtype = get_messagetype_from_string(msglevel);
			}
			groupname.Trim();
			if (groupname.CompareNoCase("all") == 0 || groupname.IsEmpty()) {
				dbgout->SetAllGroupFilters(msgtype);
			} else if (groupname[0u] != '+') {
				dbgout->SetGroupFilter(groupname, msgtype);
			} else {
				const auto groups = parse_log_multigroup(groupname);
				for (const auto &g : groups)
					dbgout->SetGroupFilter(g, msgtype);
			}
		}
	}

	// Delegate buffered messages to this new output
	if (DebugMsgBuff && !was_created_earlier)
		DebugMsgBuff->Send(log_id);
}

void init_debug(const ConfigTree &cfg, bool stderr_only) {
	// Register outputs
	apply_debug_config(cfg);
	platform->SetOutputToErr(stderr_only);

	if (stderr_only)
		return;

	// Message buffer to save all messages in case we read different log settings from config file
	DebugMsgBuff.reset(new MessageBuffer());
	DbgMgr.RegisterOutput(OutputMsgBufID, DebugMsgBuff.get(), kDbgMsg_All);
}

void apply_debug_config(const ConfigTree &cfg) {
	apply_log_config(cfg, OutputSystemID, /* defaults */ true, { DbgGroupOption(kDbgGroup_Main, kDbgMsg_Info) });
	bool legacy_log_enabled = INIreadint(cfg, "misc", "log", 0) != 0;
	apply_log_config(cfg, OutputFileID,
	                 /* defaults */
	legacy_log_enabled, {
		DbgGroupOption(kDbgGroup_Main, kDbgMsg_All),
		DbgGroupOption(kDbgGroup_Game, kDbgMsg_Info),
		DbgGroupOption(kDbgGroup_Script, kDbgMsg_All),
#ifdef DEBUG_SPRITECACHE
		DbgGroupOption(kDbgGroup_SprCache, kDbgMsg_All),
#else
		DbgGroupOption(kDbgGroup_SprCache, kDbgMsg_Info),
#endif
#ifdef DEBUG_MANAGED_OBJECTS
		DbgGroupOption(kDbgGroup_ManObj, kDbgMsg_All),
#else
		DbgGroupOption(kDbgGroup_ManObj, kDbgMsg_Info),
#endif
	});

	// Init game console if the game was compiled in Debug mode
	if (game.options[OPT_DEBUGMODE] != 0) {
		apply_log_config(cfg, OutputGameConsoleID,
		                 /* defaults */
		true, {
			DbgGroupOption(kDbgGroup_Main, kDbgMsg_All),
			DbgGroupOption(kDbgGroup_Game, kDbgMsg_All),
			DbgGroupOption(kDbgGroup_Script, kDbgMsg_All)
		});
		debug_set_console(true);
	}

	// If the game was compiled in Debug mode *and* there's no regular file log,
	// then open "warnings.log" for printing script warnings.
	if (game.options[OPT_DEBUGMODE] != 0 && !DebugLogFile) {
		auto dbgout = create_log_output(OutputFileID, "warnings.log", LogFile::kLogFile_OverwriteAtFirstMessage);
		if (dbgout) {
			dbgout->SetGroupFilter(kDbgGroup_Game, kDbgMsg_Warn);
			dbgout->SetGroupFilter(kDbgGroup_Script, kDbgMsg_Warn);
		}
	}

	// We don't need message buffer beyond this point
	DbgMgr.UnregisterOutput(OutputMsgBufID);
	DebugMsgBuff.reset();
}

void shutdown_debug() {
	// Shutdown output subsystem
	DbgMgr.UnregisterAll();

	DebugMsgBuff.reset();
	DebugLogFile.reset();
	DebugConsole.reset();
}

void debug_set_console(bool enable) {
	if (DebugConsole)
		DbgMgr.GetOutput(OutputGameConsoleID)->SetEnabled(enable);
}

// Prepends message text with current room number and running script info, then logs result
void debug_script_print(const String &msg, MessageType mt) {
	String script_ref;
	ccInstance *curinst = ccInstance::GetCurrentInstance();
	if (curinst != nullptr) {
		String scriptname;
		if (curinst->instanceof == gamescript)
			scriptname = "G ";
		else if (curinst->instanceof == thisroom.CompiledScript)
			scriptname = "R ";
		else if (curinst->instanceof == dialogScriptsScript)
			scriptname = "D ";
		else
			scriptname = "? ";
		script_ref.Format("[%s%d]", scriptname.GetCStr(), currentline);
	}

	Debug::Printf(kDbgGroup_Game, mt, "(room:%d)%s %s", displayed_room, script_ref.GetCStr(), msg.GetCStr());
}

void debug_script_warn(const char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	String full_msg = String::FromFormatV(msg, ap);
	va_end(ap);
	debug_script_print(full_msg, kDbgMsg_Warn);
}

void debug_script_log(const char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	String full_msg = String::FromFormatV(msg, ap);
	va_end(ap);
	debug_script_print(full_msg, kDbgMsg_Debug);
}


String get_cur_script(int numberOfLinesOfCallStack) {
	String callstack;
	ccInstance *sci = ccInstance::GetCurrentInstance();
	if (sci)
		callstack = sci->GetCallStack(numberOfLinesOfCallStack);
	if (callstack.IsEmpty())
		callstack = ccErrorCallStack;
	return callstack;
}

bool get_script_position(ScriptPosition &script_pos) {
	ccInstance *cur_instance = ccInstance::GetCurrentInstance();
	if (cur_instance) {
		cur_instance->GetScriptPosition(script_pos);
		return true;
	}
	return false;
}

struct Breakpoint {
	char scriptName[80];
	int lineNumber;
};

std::vector<Breakpoint> breakpoints;
int numBreakpoints = 0;

bool send_message_to_editor(const char *msg, const char *errorMsg) {
	String callStack = get_cur_script(25);
	if (callStack.IsEmpty())
		return false;

	char messageToSend[STD_BUFFER_SIZE];
	sprintf(messageToSend, "<?xml version=\"1.0\" encoding=\"Windows-1252\"?><Debugger Command=\"%s\">", msg);
#if AGS_PLATFORM_OS_WINDOWS
	sprintf(&messageToSend[strlen(messageToSend)], "  <EngineWindow>%d</EngineWindow> ", (int)win_get_window());
#endif
	sprintf(&messageToSend[strlen(messageToSend)], "  <ScriptState><![CDATA[%s]]></ScriptState> ", callStack.GetCStr());
	if (errorMsg != nullptr) {
		sprintf(&messageToSend[strlen(messageToSend)], "  <ErrorMessage><![CDATA[%s]]></ErrorMessage> ", errorMsg);
	}
	strcat(messageToSend, "</Debugger>");

	editor_debugger->SendMessageToEditor(messageToSend);

	return true;
}

bool send_message_to_editor(const char *msg) {
	return send_message_to_editor(msg, nullptr);
}

bool init_editor_debugging() {
#if AGS_PLATFORM_OS_WINDOWS
	editor_debugger = GetEditorDebugger(editor_debugger_instance_token);
#else
	// Editor isn't ported yet
	editor_debugger = nullptr;
#endif

	if (editor_debugger == nullptr)
		quit("editor_debugger is NULL but debugger enabled");

	if (editor_debugger->Initialize()) {
		editor_debugging_initialized = 1;

		// Wait for the editor to send the initial breakpoints
		// and then its READY message
		while (check_for_messages_from_editor() != 2) {
			platform->Delay(10);
		}

		send_message_to_editor("START");
		return true;
	}

	return false;
}

int check_for_messages_from_editor() {
	if (editor_debugger->IsMessageAvailable()) {
		char *msg = editor_debugger->GetNextMessage();
		if (msg == nullptr) {
			return 0;
		}

		if (strncmp(msg, "<Engine Command=\"", 17) != 0) {
			//Debug::Printf("Faulty message received from editor:");
			//Debug::Printf(msg);
			free(msg);
			return 0;
		}

		const char *msgPtr = &msg[17];

		if (strncmp(msgPtr, "START", 5) == 0) {
#if AGS_PLATFORM_OS_WINDOWS
			const char *windowHandle = strstr(msgPtr, "EditorWindow") + 14;
			editor_window_handle = (HWND)atoi(windowHandle);
#endif
		} else if (strncmp(msgPtr, "READY", 5) == 0) {
			free(msg);
			return 2;
		} else if ((strncmp(msgPtr, "SETBREAK", 8) == 0) ||
		           (strncmp(msgPtr, "DELBREAK", 8) == 0)) {
			bool isDelete = (msgPtr[0] == 'D');
			// Format:  SETBREAK $scriptname$lineNumber$
			msgPtr += 10;
			char scriptNameBuf[100];
			int i = 0;
			while (msgPtr[0] != '$') {
				scriptNameBuf[i] = msgPtr[0];
				msgPtr++;
				i++;
			}
			scriptNameBuf[i] = 0;
			msgPtr++;

			int lineNumber = atoi(msgPtr);

			if (isDelete) {
				for (i = 0; i < numBreakpoints; i++) {
					if ((breakpoints[i].lineNumber == lineNumber) &&
					        (strcmp(breakpoints[i].scriptName, scriptNameBuf) == 0)) {
						numBreakpoints--;
						breakpoints.erase(breakpoints.begin() + i);
						break;
					}
				}
			} else {
				breakpoints.push_back(Breakpoint());
				strcpy(breakpoints[numBreakpoints].scriptName, scriptNameBuf);
				breakpoints[numBreakpoints].lineNumber = lineNumber;
				numBreakpoints++;
			}
		} else if (strncmp(msgPtr, "RESUME", 6) == 0) {
			game_paused_in_debugger = 0;
		} else if (strncmp(msgPtr, "STEP", 4) == 0) {
			game_paused_in_debugger = 0;
			break_on_next_script_step = 1;
		} else if (strncmp(msgPtr, "EXIT", 4) == 0) {
			want_exit = 1;
			abort_engine = 1;
			check_dynamic_sprites_at_exit = 0;
		}

		free(msg);
		return 1;
	}

	return 0;
}




bool send_exception_to_editor(const char *qmsg) {
#if AGS_PLATFORM_OS_WINDOWS
	want_exit = 0;
	// allow the editor to break with the error message
	if (editor_window_handle != NULL)
		SetForegroundWindow(editor_window_handle);

	if (!send_message_to_editor("ERROR", qmsg))
		return false;

	while ((check_for_messages_from_editor() == 0) && (want_exit == 0)) {
		update_polled_mp3();
		platform->Delay(10);
	}
#endif
	return true;
}


void break_into_debugger() {
#if AGS_PLATFORM_OS_WINDOWS

	if (editor_window_handle != NULL)
		SetForegroundWindow(editor_window_handle);

	send_message_to_editor("BREAK");
	game_paused_in_debugger = 1;

	while (game_paused_in_debugger) {
		update_polled_stuff_if_runtime();
		platform->YieldCPU();
	}

#endif
}

int scrDebugWait = 0;
extern int pluginsWantingDebugHooks;

// allow LShift to single-step,  RShift to pause flow
void scriptDebugHook(ccInstance *ccinst, int linenum) {

	if (pluginsWantingDebugHooks > 0) {
		// a plugin is handling the debugging
		String scname = GetScriptName(ccinst);
		pl_run_plugin_debug_hooks(scname, linenum);
		return;
	}

	// no plugin, use built-in debugger

	if (ccinst == nullptr) {
		// come out of script
		return;
	}

	if (break_on_next_script_step) {
		break_on_next_script_step = 0;
		break_into_debugger();
		return;
	}

	const char *scriptName = ccinst->runningInst->instanceof->GetSectionName(ccinst->pc);

	for (int i = 0; i < numBreakpoints; i++) {
		if ((breakpoints[i].lineNumber == linenum) &&
		        (strcmp(breakpoints[i].scriptName, scriptName) == 0)) {
			break_into_debugger();
			break;
		}
	}
}

int scrlockWasDown = 0;

void check_debug_keys() {
	if (play.debug_mode) {
		// do the run-time script debugging

		if ((!key[KEY_SCRLOCK]) && (scrlockWasDown))
			scrlockWasDown = 0;
		else if ((key[KEY_SCRLOCK]) && (!scrlockWasDown)) {

			break_on_next_script_step = 1;
			scrlockWasDown = 1;
		}

	}

}
