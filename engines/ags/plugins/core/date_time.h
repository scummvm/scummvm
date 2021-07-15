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

#ifndef AGS_PLUGINS_CORE_DATE_TIME_H
#define AGS_PLUGINS_CORE_DATE_TIME_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class DateTime : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void Now(ScriptMethodParams &params);
	static void GetDayOfMonth(ScriptMethodParams &params);
	static void GetHour(ScriptMethodParams &params);
	static void GetMinute(ScriptMethodParams &params);
	static void GetMonth(ScriptMethodParams &params);
	static void GetRawTime(ScriptMethodParams &params);
	static void GetSecond(ScriptMethodParams &params);
	static void GetYear(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
