/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGS_JOY_AGS_JOY_H
#define AGS_PLUGINS_AGS_JOY_AGS_JOY_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace AGSJoy {

class AGSJoy : public PluginBase {
private:
	static IAGSEngine *_engine;
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *lpEngine);

private:
	static void Count(ScriptMethodParams &params);
	static void Name(ScriptMethodParams &params);
	static void Rescan(ScriptMethodParams &params);
	static void Open(ScriptMethodParams &params);
	static void IsOpen(ScriptMethodParams &params);
	static void Click(ScriptMethodParams &params);
	static void Close(ScriptMethodParams &params);
	static void Valid(ScriptMethodParams &params);
	static void Unplugged(ScriptMethodParams &params);
	static void GetName(ScriptMethodParams &params);
	static void GetAxis(ScriptMethodParams &params);
	static void IsButtonDown(ScriptMethodParams &params);
	static void IsJoyBtnDown(ScriptMethodParams &params);
	static void Update(ScriptMethodParams &params);
	static void DisableEvents(ScriptMethodParams &params);
	static void EnableEvents(ScriptMethodParams &params);

public:
	AGSJoy();
};

} // namespace AGSJoy
} // namespace Plugins
} // namespace AGS3

#endif
