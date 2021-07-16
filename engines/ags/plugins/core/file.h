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

#ifndef AGS_PLUGINS_CORE_FILE_H
#define AGS_PLUGINS_CORE_FILE_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class File : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void Delete(ScriptMethodParams &params);
	static void Exists(ScriptMethodParams &params);
	static void OpenFile(ScriptMethodParams &params);
	static void Close(ScriptMethodParams &params);
	static void ReadInt(ScriptMethodParams &params);
	static void ReadRawChar(ScriptMethodParams &params);
	static void ReadRawInt(ScriptMethodParams &params);
	static void ReadRawLine(ScriptMethodParams &params);
	static void ReadRawLineBack(ScriptMethodParams &params);
	static void ReadString(ScriptMethodParams &params);
	static void ReadStringBack(ScriptMethodParams &params);
	static void WriteInt(ScriptMethodParams &params);
	static void WriteRawChar(ScriptMethodParams &params);
	static void WriteRawLine(ScriptMethodParams &params);
	static void WriteString(ScriptMethodParams &params);
	static void GetEOF(ScriptMethodParams &params);
	static void GetError(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
