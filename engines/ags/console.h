
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

#ifndef AGS_CONSOLE_H
#define AGS_CONSOLE_H

#include "gui/debugger.h"
#include "ags/shared/debugging/debug_manager.h"
#include "ags/shared/debugging/output_handler.h"
#include "ags/shared/core/types.h"

namespace AGS {

class AGSEngine;
class LogOutputTarget;

class AGSConsole : public GUI::Debugger {
public:
	AGSConsole(AGSEngine *vm);
	~AGSConsole() override;

private:
	AGSEngine *_vm;
	LogOutputTarget *_logOutputTarget;
	AGS3::AGS::Shared::PDebugOutput _agsDebuggerOutput;

	bool Cmd_listDebugGroups(int argc, const char **argv);
	bool Cmd_setDebugGroupLevel(int argc, const char **argv);

	bool Cmd_SetScriptDump(int argc, const char **argv);

	bool Cmd_getSpriteInfo(int argc, const char **argv);
	bool Cmd_dumpSprite(int argc, const char **argv);

	const char *getVerbosityLevel(AGS3::uint32_t groupID) const;
	AGS3::uint32_t parseGroup(const char *, bool &) const;
	AGS3::AGS::Shared::MessageType parseLevel(const char *, bool &) const;
	void printGroupList();
	void printLevelList();
};

class LogOutputTarget : public AGS3::AGS::Shared::IOutputHandler {
public:
	LogOutputTarget();
	~LogOutputTarget() override;

	void PrintMessage(const AGS3::AGS::Shared::DebugMessage &msg) override;
};

} // End of namespace AGS

#endif
