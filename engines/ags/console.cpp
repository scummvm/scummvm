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

#include "ags/console.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/gfx/allegro_bitmap.h"
#include "ags/shared/script/cc_options.h"
#include "image/png.h"

namespace AGS {

AGSConsole::AGSConsole(AGSEngine *vm) : GUI::Debugger(), _vm(vm), _logOutputTarget(nullptr), _agsDebuggerOutput(nullptr) {
	registerCmd("ags_debug_groups_list",   WRAP_METHOD(AGSConsole, Cmd_listDebugGroups));
	registerCmd("ags_debug_groups_set",  WRAP_METHOD(AGSConsole, Cmd_setDebugGroupLevel));
	registerCmd("ags_set_script_dump", WRAP_METHOD(AGSConsole, Cmd_SetScriptDump));
	registerCmd("ags_sprite_info",   WRAP_METHOD(AGSConsole, Cmd_getSpriteInfo));
	registerCmd("ags_sprite_dump",  WRAP_METHOD(AGSConsole, Cmd_dumpSprite));

	_logOutputTarget = new LogOutputTarget();
	_agsDebuggerOutput = _GP(DbgMgr).RegisterOutput("ScummVMLog", _logOutputTarget, AGS3::AGS::Shared::kDbgMsg_None);
}

AGSConsole::~AGSConsole() {
	delete _logOutputTarget;
}

struct LevelName {
	const char *name;
	AGS3::AGS::Shared::MessageType level;
};

static const LevelName levelNames[] = {
	{"none", AGS3::AGS::Shared::kDbgMsg_None},
	{"alerts", AGS3::AGS::Shared::kDbgMsg_Alert},
	{"fatal", AGS3::AGS::Shared::kDbgMsg_Fatal},
	{"errors", AGS3::AGS::Shared::kDbgMsg_Error},
	{"warnings", AGS3::AGS::Shared::kDbgMsg_None},
	{"info", AGS3::AGS::Shared::kDbgMsg_Info},
	{"debug", AGS3::AGS::Shared::kDbgMsg_Debug},
	{nullptr, AGS3::AGS::Shared::kDbgMsg_None}
};

struct GroupName {
	const char *name;
	AGS3::uint32_t group;
};

static const GroupName groupNames[] = {
	{"Main", AGS3::AGS::Shared::kDbgGroup_Main},
	{"Game", AGS3::AGS::Shared::kDbgGroup_Game},
	{"Script", AGS3::AGS::Shared::kDbgGroup_Script},
	{"SpriteCache", AGS3::AGS::Shared::kDbgGroup_SprCache},
	{"ManObj", AGS3::AGS::Shared::kDbgGroup_ManObj},
	{nullptr, (AGS3::uint32_t)-1}
};

bool AGSConsole::Cmd_listDebugGroups(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	debugPrintf("%-16s %-16s\n", "Name", "Level");
	for (int i = 0 ; groupNames[i].name != nullptr ; ++i)
		debugPrintf("%-16s %-16s\n", groupNames[i].name, getVerbosityLevel(groupNames[i].group));
	return true;
}

bool AGSConsole::Cmd_setDebugGroupLevel(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s group level\n", argv[0]);
		debugPrintf("   valid groups: ");
		printGroupList();
		debugPrintf("\n");
		debugPrintf("   valid levels: ");
		printLevelList();
		debugPrintf("\n");
		return true;
	}

	bool found = false;
	AGS3::uint32_t group = parseGroup(argv[1], found);
	if (!found) {
		debugPrintf("Unknown debug group '%s'\n", argv[1]);
		debugPrintf("Valid groups are: ");
		printGroupList();
		debugPrintf("\n");
		return true;
	}

	AGS3::AGS::Shared::MessageType level = parseLevel(argv[2], found);
	if (!found) {
		debugPrintf("Unknown level '%s'\n", argv[2]);
		debugPrintf("Valid levels are: ");
		printLevelList();
		debugPrintf("\n");
		return true;
	}

	_agsDebuggerOutput->SetGroupFilter(group, level);
	return true;
}

const char *AGSConsole::getVerbosityLevel(AGS3::uint32_t groupID) const {
	int i = 1;
	while (levelNames[i].name != nullptr) {
		if (!_agsDebuggerOutput->TestGroup(groupID, levelNames[i].level))
			break;
		++i;
	}
	return levelNames[i - 1].name;
}

AGS3::uint32_t AGSConsole::parseGroup(const char *name, bool &found) const {
	int i = 0;
	while (groupNames[i].name != nullptr) {
		if (scumm_stricmp(name, groupNames[i].name) == 0) {
			found = true;
			return groupNames[i].group;
		}
		++i;
	}

	found = false;
	return (AGS3::uint32_t)-1;
}

AGS3::AGS::Shared::MessageType AGSConsole::parseLevel(const char *name, bool &found) const {
	int i = 0;
	while (levelNames[i].name != nullptr) {
		if (scumm_stricmp(name, levelNames[i].name) == 0) {
			found = true;
			return levelNames[i].level;
		}
		++i;
	}

	found = false;
	return AGS3::AGS::Shared::kDbgMsg_None;
}

void AGSConsole::printGroupList() {
	debugPrintf("%s", groupNames[0].name);
	for (int i = 1 ; groupNames[i].name != nullptr ; ++i)
		debugPrintf(", %s", groupNames[i].name);
}

void AGSConsole::printLevelList() {
	debugPrintf("%s", levelNames[0].name);
	for (int i = 1 ; levelNames[i].name != nullptr ; ++i)
		debugPrintf(", %s", levelNames[i].name);
}

bool AGSConsole::Cmd_SetScriptDump(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s [on|off]\n", argv[0]);
		return true;
	}

	if (strcmp(argv[1], "on") == 0 || strcmp(argv[1], "true") == 0)
		AGS3::ccSetOption(SCOPT_DEBUGRUN, 1);
	else
		AGS3::ccSetOption(SCOPT_DEBUGRUN, 0);
	return true;
}

bool AGSConsole::Cmd_getSpriteInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s SpriteNumber\n", argv[0]);
		return true;
	}

	int spriteId = atoi(argv[1]);
	if (!_GP(spriteset).DoesSpriteExist(spriteId)) {
		debugPrintf("Sprite %d does not exist\n", spriteId);
		return true;
	}

	AGS3::Shared::Bitmap *sprite = _GP(spriteset)[spriteId];
	if (!sprite) {
		debugPrintf("Failed to get sprite %d\n", spriteId);
		return true;
	}

	debugPrintf("Size: %dx%d\n", sprite->GetWidth(), sprite->GetHeight());
	debugPrintf("Color depth: %d\n", sprite->GetColorDepth());
	return true;
}

bool AGSConsole::Cmd_dumpSprite(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s SpriteNumber\n", argv[0]);
		return true;
	}

	int spriteId = atoi(argv[1]);
	if (!_GP(spriteset).DoesSpriteExist(spriteId)) {
		debugPrintf("Sprite %d does not exist\n", spriteId);
		return true;
	}

	AGS3::Shared::Bitmap *sprite = _GP(spriteset)[spriteId];
	if (!sprite) {
		debugPrintf("Failed to get sprite %d\n", spriteId);
		return true;
	}

	Common::String pngFile = Common::String::format("%s-sprite%03d.png", _vm->getGameId().c_str(), spriteId);
	Common::DumpFile df;
	if (df.open(pngFile)) {
		byte *palette = nullptr;
		if (sprite->GetColorDepth() == 8) {
			palette = new byte[256 * 3];
			for (int c = 0, i = 0 ; c < 256 ; ++c, i += 3) {
				palette[i] = _G(current_palette)[c].r * 255 / 63;
				palette[i + 1] = _G(current_palette)[c].g * 255 / 63;
				palette[i + 2] = _G(current_palette)[c].b * 255 / 63;
			}
		}
		Image::writePNG(df, sprite->GetAllegroBitmap()->getSurface().rawSurface(), palette);
		delete[] palette;
	}

	return true;
}

LogOutputTarget::LogOutputTarget() {
}

LogOutputTarget::~LogOutputTarget() {
}

void LogOutputTarget::PrintMessage(const AGS3::AGS::Shared::DebugMessage &msg) {
	LogMessageType::Type msgType = LogMessageType::kInfo;
	switch (msg.MT) {
	case AGS3::AGS::Shared::kDbgMsg_None:
		return;
	case AGS3::AGS::Shared::kDbgMsg_Alert:
	case AGS3::AGS::Shared::kDbgMsg_Fatal:
	case AGS3::AGS::Shared::kDbgMsg_Error:
		msgType = LogMessageType::kError;
		break;
	case AGS3::AGS::Shared::kDbgMsg_Warn:
		msgType = LogMessageType::kWarning;
		break;
	case AGS3::AGS::Shared::kDbgMsg_Info:
		msgType = LogMessageType::kInfo;
		break;
	case AGS3::AGS::Shared::kDbgMsg_Debug:
		msgType = LogMessageType::kDebug;
		break;
	}
	Common::String text = Common::String::format("%s\n", msg.Text.GetCStr());
	g_system->logMessage(msgType, text.c_str());
}


} // End of namespace AGS
