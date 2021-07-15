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

#ifndef AGS_PLUGINS_CORE_DIALOG_OPTIONS_RENDERING_INFO_H
#define AGS_PLUGINS_CORE_DIALOG_OPTIONS_RENDERING_INFO_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class DialogOptionsRenderingInfo : public ScriptContainer {
public:
	static void AGS_EngineStartup(IAGSEngine *engine);

	static void GetActiveOptionID(ScriptMethodParams &params);
	static void SetActiveOptionID(ScriptMethodParams &params);
	static void GetDialogToRender(ScriptMethodParams &params);
	static void GetHeight(ScriptMethodParams &params);
	static void SetHeight(ScriptMethodParams &params);
	static void GetParserTextboxX(ScriptMethodParams &params);
	static void SetParserTextboxX(ScriptMethodParams &params);
	static void GetParserTextboxY(ScriptMethodParams &params);
	static void SetParserTextboxY(ScriptMethodParams &params);
	static void GetParserTextboxWidth(ScriptMethodParams &params);
	static void SetParserTextboxWidth(ScriptMethodParams &params);
	static void GetSurface(ScriptMethodParams &params);
	static void GetWidth(ScriptMethodParams &params);
	static void SetWidth(ScriptMethodParams &params);
	static void GetX(ScriptMethodParams &params);
	static void SetX(ScriptMethodParams &params);
	static void GetY(ScriptMethodParams &params);
	static void SetY(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
