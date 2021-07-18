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

#ifndef AGS_PLUGINS_CORE_VIEW_FRAME_H
#define AGS_PLUGINS_CORE_VIEW_FRAME_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class ViewFrame : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void GetFlipped(ScriptMethodParams &params);
	static void GetFrame(ScriptMethodParams &params);
	static void GetGraphic(ScriptMethodParams &params);
	static void SetGraphic(ScriptMethodParams &params);
	static void GetLinkedAudio(ScriptMethodParams &params);
	static void SetLinkedAudio(ScriptMethodParams &params);
	static void GetLoop(ScriptMethodParams &params);
	static void GetSound(ScriptMethodParams &params);
	static void SetSound(ScriptMethodParams &params);
	static void GetSpeed(ScriptMethodParams &params);
	static void GetView(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
