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

#include "common/std/memory.h"
#include "common/std/limits.h"
#include "common/std/initializer_list.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/debugging/ags_editor_debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/debugging/debug_manager.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/debugging/log_file.h"
#include "ags/engine/debugging/message_buffer.h"
#include "ags/engine/main/config.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/engine/script/script.h"
#include "ags/shared/script/cc_internal.h"
#include "ags/shared/script/cc_common.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/string_utils.h"
#include "ags/shared/util/text_stream_writer.h"
#include "ags/globals.h"
#if AGS_PLATFORM_OS_WINDOWS
#include "ags/shared/platform/windows/debug/namedpipesagsdebugger.h"
#endif

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

#if AGS_PLATFORM_OS_WINDOWS

HWND editor_window_handle = 0;

IAGSEditorDebugger *GetEditorDebugger(const char *instanceToken) {
	return new NamedPipesAGSDebugger(instanceToken);
}

#else   // AGS_PLATFORM_OS_WINDOWS

IAGSEditorDebugger *GetEditorDebugger(const char * /*instanceToken*/) {
	return nullptr;
}

#endif

void send_message_to_debugger(const std::vector<std::pair<String, String> > &tag_values, const String &command) {
	String messageToSend = String::FromFormat(R"(<?xml version=" 1.0 " encoding=" Windows - 1252 "?><Debugger Command=" % s ">)", command.GetCStr());
#if AGS_PLATFORM_OS_WINDOWS
	messageToSend.Append(String::FromFormat("  <EngineWindow>%d</EngineWindow> ", (int)sys_win_get_window()));
#endif

	for (const auto &tag_value : tag_values) {
		messageToSend.AppendFmt("  <%s><![CDATA[%s]]></%s> ", tag_value.first.GetCStr(), tag_value.second.GetCStr(), tag_value.first.GetCStr());
	}

	messageToSend.Append("</Debugger>\n");

	_G(editor_debugger)->SendMessageToEditor(messageToSend.GetCStr());
}

static const char *OutputMsgBufID = "buffer";
static const char *OutputFileID = "file";
static const char *OutputSystemID = "stdout";

// Create a new log output by ID

PDebugOutput create_log_output(const String &name, const String &dir = "", const String &filename = "", LogFile::OpenMode open_mode = LogFile::kLogFile_Overwrite) {
	// Else create new one, if we know this ID
	if (name.CompareNoCase(OutputSystemID) == 0) {
		return _GP(DbgMgr).RegisterOutput(OutputSystemID, AGSPlatformDriver::GetDriver(), kDbgMsg_None);
	} else if (name.CompareNoCase(OutputFileID) == 0) {
		_GP(DebugLogFile).reset(new LogFile());
		String logfile_dir = dir;
		if (dir.IsEmpty()) {
			FSLocation fs = _G(platform)->GetAppOutputDirectory();
			CreateFSDirs(fs);
			logfile_dir = fs.FullDir;
		} else if (Path::IsRelativePath(dir) && _G(platform)->IsLocalDirRestricted()) {
			FSLocation fs = GetGameUserDataDir();
			CreateFSDirs(fs);
			logfile_dir = fs.FullDir;
		}
		String logfilename = filename.IsEmpty() ? "ags.log" : filename;
#if AGS_PLATFORM_SCUMMVM
		logfile_dir = "";  // ignore path
#endif
		String logfile_path = Path::ConcatPaths(logfile_dir, logfilename);
		if (!_GP(DebugLogFile)->OpenFile(logfile_path, open_mode))
			return nullptr;
		Debug::Printf(kDbgMsg_Info, "Logging to %s", logfile_path.GetCStr());
		auto dbgout = _GP(DbgMgr).RegisterOutput(OutputFileID, _GP(DebugLogFile).get(), kDbgMsg_None);
		return dbgout;
	}
	return nullptr;
}

// Parses a string where each character defines a single log group; returns list of real group names.
std::vector<String> parse_log_multigroup(const String &group_str) {
	std::vector<String> grplist;
	for (size_t i = 0; i < group_str.GetLength(); ++i) {
		switch (group_str[i]) {
		case 'm':
			grplist.emplace_back("main");
			break;
		case 'g':
			grplist.emplace_back("game");
			break;
		case 's':
			grplist.emplace_back("script");
			break;
		case 'c':
			grplist.emplace_back("sprcache");
			break;
		case 'o':
			grplist.emplace_back("manobj");
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
	String value = CfgReadString(cfg, "log", log_id);
	if (value.IsEmpty() && !def_enabled)
		return;

	// First test if already registered, if not then try create it
	auto dbgout = _GP(DbgMgr).GetOutput(log_id);
	const bool was_created_earlier = dbgout != nullptr;
	if (!dbgout) {
		String path = CfgReadString(cfg, "log", String::FromFormat("%s-path", log_id.GetCStr()));
		dbgout = create_log_output(log_id, path);
		if (!dbgout)
			return; // unknown output type
	}
	dbgout->ClearGroupFilters();

	if (value.IsEmpty() || value.CompareNoCase("default") == 0) {
		for (const auto &opt : def_opts)
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
	if (_GP(DebugMsgBuff) && !was_created_earlier)
		_GP(DebugMsgBuff)->Send(log_id);
}

void init_debug(const ConfigTree &cfg, bool stderr_only) {
	// Register outputs
	apply_debug_config(cfg);
	_G(platform)->SetOutputToErr(stderr_only);

	if (stderr_only)
		return;

	// Message buffer to save all messages in case we read different log settings from config file
	_GP(DebugMsgBuff).reset(new MessageBuffer());
	_GP(DbgMgr).RegisterOutput(OutputMsgBufID, _GP(DebugMsgBuff).get(), kDbgMsg_All);
}

void apply_debug_config(const ConfigTree &cfg) {
	apply_log_config(cfg, OutputSystemID, /* defaults */ true, { DbgGroupOption(kDbgGroup_Main, kDbgMsg_Info) });
	bool legacy_log_enabled = CfgReadBoolInt(cfg, "misc", "log", false);

	apply_log_config(cfg, OutputFileID,
	                 /* defaults */
	legacy_log_enabled, {
		DbgGroupOption(kDbgGroup_Main, kDbgMsg_All),
		DbgGroupOption(kDbgGroup_Game, kDbgMsg_Info),
#if DEBUG_SPRITECACHE
		DbgGroupOption(kDbgGroup_SprCache, kDbgMsg_All),
#else
		DbgGroupOption(kDbgGroup_SprCache, kDbgMsg_Info),
#endif
#if DEBUG_MANAGED_OBJECTS
		DbgGroupOption(kDbgGroup_ManObj, kDbgMsg_All),
#else
		DbgGroupOption(kDbgGroup_ManObj, kDbgMsg_Info),
#endif
	});

	// If the game was compiled in Debug mode *and* there's no regular file log,
	// then open "warnings.log" for printing script warnings.
	if (_GP(game).options[OPT_DEBUGMODE] != 0 && !_GP(DebugLogFile)) {
		auto dbgout = create_log_output(OutputFileID, "./", "warnings.log", LogFile::kLogFile_OverwriteAtFirstMessage);
		if (dbgout)
			dbgout->SetGroupFilter(kDbgGroup_Game, kDbgMsg_Warn);
	}

	// We don't need message buffer beyond this point
	_GP(DbgMgr).UnregisterOutput(OutputMsgBufID);
	_GP(DebugMsgBuff).reset();
}

void shutdown_debug() {
	// Shutdown output subsystem
	_GP(DbgMgr).UnregisterAll();

	_GP(DebugMsgBuff).reset();
	_GP(DebugLogFile).reset();
}

// Prepends message text with current room number and running script info, then logs result
static void debug_script_print_impl(const String &msg, MessageType mt) {
	String script_ref;
	ccInstance *curinst = ccInstance::GetCurrentInstance();
	if (curinst != nullptr) {
		String scriptname;
		if (curinst->instanceof == _GP(gamescript))
			scriptname = "G ";
		else if (curinst->instanceof == _GP(thisroom).CompiledScript)
			scriptname = "R ";
		else if (curinst->instanceof == _GP(dialogScriptsScript))
			scriptname = "D ";
		else
			scriptname = "? ";
		script_ref.Format("[%s%d]", scriptname.GetCStr(), _G(currentline));
	}

	Debug::Printf(kDbgGroup_Game, mt, "(room:%d)%s %s", _G(displayed_room), script_ref.GetCStr(), msg.GetCStr());
}

void debug_script_print(MessageType mt, const char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	String full_msg = String::FromFormatV(msg, ap);
	va_end(ap);
	debug_script_print_impl(full_msg, mt);
}

void debug_script_warn(const char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	String full_msg = String::FromFormatV(msg, ap);
	va_end(ap);
	debug_script_print_impl(full_msg, kDbgMsg_Warn);
}

void debug_script_log(const char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	String full_msg = String::FromFormatV(msg, ap);
	va_end(ap);
	debug_script_print_impl(full_msg, kDbgMsg_Debug);
}

struct Breakpoint {
	char scriptName[80]{};
	int lineNumber = 0;
};

bool send_state_to_debugger(const String& msg, const String& errorMsg) {
	// Get either saved callstack from a script error, or current execution point
	String callStack = (!errorMsg.IsEmpty() && cc_has_error()) ?
		cc_get_error().CallStack : cc_get_callstack();
	if (callStack.IsEmpty())
		return false;

	std::vector<std::pair<String, String>> script_info = {{"ScriptState", callStack}};

	if (!errorMsg.IsEmpty()) {
		script_info.emplace_back("ErrorMessage", errorMsg);
	}

	send_message_to_debugger(script_info, msg);
	return true;
}

bool send_state_to_debugger(const char *msg) {
	return send_state_to_debugger(String(msg), String());
}

bool init_editor_debugging() {
#if AGS_PLATFORM_OS_WINDOWS
	_G(editor_debugger) = GetEditorDebugger(_G(editor_debugger_instance_token));
#else
	// Editor isn't ported yet
	_G(editor_debugger) = nullptr;
#endif

	if (_G(editor_debugger) == nullptr)
		quit("editor_debugger is NULL but debugger enabled");

	if (_G(editor_debugger)->Initialize()) {
		_G(editor_debugging_initialized) = 1;

		// Wait for the editor to send the initial breakpoints
		// and then its READY message
		while (check_for_messages_from_debugger() != 2) {
			_G(platform)->Delay(10);
		}

		send_state_to_debugger("START");
		Debug::Printf(kDbgMsg_Info, "External debugger initialized");
		return true;
	}

	Debug::Printf(kDbgMsg_Error, "Failed to initialize external debugger");
	return false;
}

int check_for_messages_from_debugger() {
	if (_G(editor_debugger)->IsMessageAvailable()) {
		char *msg = _G(editor_debugger)->GetNextMessage();
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
			char scriptNameBuf[sizeof(Breakpoint::scriptName)]{};
			for (size_t i = 0; msgPtr[0] != '$'; ++msgPtr, ++i) {
				if (i < sizeof(scriptNameBuf) - 1)
					scriptNameBuf[i] = msgPtr[0];
			}
			msgPtr++;

			int lineNumber = atoi(msgPtr);

			if (isDelete) {
				for (size_t i = 0; i < _G(breakpoints).size(); ++i) {
					if ((_G(breakpoints)[i].lineNumber == lineNumber) &&
					        (strcmp(_G(breakpoints)[i].scriptName, scriptNameBuf) == 0)) {
						_G(breakpoints).erase(_G(breakpoints).begin() + i);
						break;
					}
				}
			} else {
				Globals::Breakpoint bp;
				snprintf(bp.scriptName, sizeof(Breakpoint::scriptName), "%s", scriptNameBuf);
				bp.lineNumber = lineNumber;
				_G(breakpoints).push_back(bp);
			}
		} else if (strncmp(msgPtr, "RESUME", 6) == 0) {
			_G(game_paused_in_debugger) = 0;
		} else if (strncmp(msgPtr, "STEP", 4) == 0) {
			_G(game_paused_in_debugger) = 0;
			_G(break_on_next_script_step) = 1;
		} else if (strncmp(msgPtr, "EXIT", 4) == 0) {
			_G(want_exit) = true;
			_G(abort_engine) = true;
			_G(check_dynamic_sprites_at_exit) = 0;
		}

		free(msg);
		return 1;
	}

	return 0;
}




bool send_exception_to_debugger(const char *qmsg) {
#if AGS_PLATFORM_OS_WINDOWS
	_G(want_exit) = false;
	// allow the editor to break with the error message
	if (editor_window_handle != NULL)
		SetForegroundWindow(editor_window_handle);

	if (!send_state_to_debugger("ERROR", qmsg))
		return false;

	while ((check_for_messages_from_debugger() == 0) && (!_G(want_exit))) {
		_G(platform)->Delay(10);
	}
#endif
	return true;
}


void break_into_debugger() {
#if AGS_PLATFORM_OS_WINDOWS

	if (editor_window_handle != NULL)
		SetForegroundWindow(editor_window_handle);

	send_state_to_debugger("BREAK");
	_G(game_paused_in_debugger) = 1;

	while (_G(game_paused_in_debugger)) {
		update_polled_stuff();
		_G(platform)->YieldCPU();
	}

#endif
}

int scrDebugWait = 0;

// allow LShift to single-step,  RShift to pause flow
void scriptDebugHook(ccInstance *ccinst, int linenum) {

	if (_G(pluginsWantingDebugHooks) > 0) {
		// a plugin is handling the debugging
		String scname = GetScriptName(ccinst);
		pl_run_plugin_debug_hooks(scname.GetCStr(), linenum);
		return;
	}

	// no plugin, use built-in debugger

	if (ccinst == nullptr) {
		// come out of script
		return;
	}

	if (_G(break_on_next_script_step)) {
		_G(break_on_next_script_step) = 0;
		break_into_debugger();
		return;
	}

	const char *scriptName = ccinst->runningInst->instanceof->GetSectionName(ccinst->pc);

	for (const auto &breakpoint : _G(breakpoints)) {
		if ((breakpoint.lineNumber == linenum) && (strcmp(breakpoint.scriptName, scriptName) == 0)) {
			break_into_debugger();
			break;
		}
	}
}

int scrlockWasDown = 0;

void check_debug_keys() {
#ifdef TODO
	if (_GP(play).debug_mode) {
		// do the run-time script debugging

		const Uint8 *ks = SDL_GetKeyboardState(nullptr);
		if ((!ks[SDL_SCANCODE_SCROLLLOCK]) && (scrlockWasDown))
			scrlockWasDown = 0;
		else if ((ks[SDL_SCANCODE_SCROLLLOCK]) && (!scrlockWasDown)) {

			_G(break_on_next_script_step) = 1;
			scrlockWasDown = 1;
		}
	}
#endif
}

} // namespace AGS3
