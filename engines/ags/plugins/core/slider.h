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

#ifndef AGS_PLUGINS_CORE_SLIDER_H
#define AGS_PLUGINS_CORE_SLIDER_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Slider : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void GetBackgroundGraphic(ScriptMethodParams &params);
	static void SetBackgroundGraphic(ScriptMethodParams &params);
	static void GetHandleGraphic(ScriptMethodParams &params);
	static void SetHandleGraphic(ScriptMethodParams &params);
	static void GetHandleOffset(ScriptMethodParams &params);
	static void SetHandleOffset(ScriptMethodParams &params);
	static void GetMax(ScriptMethodParams &params);
	static void SetMax(ScriptMethodParams &params);
	static void GetMin(ScriptMethodParams &params);
	static void SetMin(ScriptMethodParams &params);
	static void GetValue(ScriptMethodParams &params);
	static void SetValue(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
