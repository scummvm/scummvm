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

#ifndef AGS_PLUGINS_CORE_BUTTON_H
#define AGS_PLUGINS_CORE_BUTTON_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Button : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void Animate(ScriptMethodParams &params);
	static void GetText(ScriptMethodParams &params);
	static void SetText(ScriptMethodParams &params);
	static void GetClipImage(ScriptMethodParams &params);
	static void SetClipImage(ScriptMethodParams &params);
	static void GetFont(ScriptMethodParams &params);
	static void SetFont(ScriptMethodParams &params);
	static void GetGraphic(ScriptMethodParams &params);
	static void GetMouseOverGraphic(ScriptMethodParams &params);
	static void SetMouseOverGraphic(ScriptMethodParams &params);
	static void GetNormalGraphic(ScriptMethodParams &params);
	static void SetNormalGraphic(ScriptMethodParams &params);
	static void GetPushedGraphic(ScriptMethodParams &params);
	static void SetPushedGraphic(ScriptMethodParams &params);
	static void GetText_New(ScriptMethodParams &params);
	static void GetTextColor(ScriptMethodParams &params);
	static void SetTextColor(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
