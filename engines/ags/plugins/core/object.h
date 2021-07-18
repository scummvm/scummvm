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

#ifndef AGS_PLUGINS_CORE_OBJECT_H
#define AGS_PLUGINS_CORE_OBJECT_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Object : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void Animate(ScriptMethodParams &params);
	static void IsCollidingWithObject(ScriptMethodParams &params);
	static void GetName(ScriptMethodParams &params);
	static void GetProperty(ScriptMethodParams &params);
	static void GetPropertyText(ScriptMethodParams &params);
	static void GetTextProperty(ScriptMethodParams &params);
	static void MergeIntoBackground(ScriptMethodParams &params);
	static void Move(ScriptMethodParams &params);
	static void RemoveTint(ScriptMethodParams &params);
	static void RunInteraction(ScriptMethodParams &params);
	static void SetPosition(ScriptMethodParams &params);
	static void SetView(ScriptMethodParams &params);
	static void StopAnimating(ScriptMethodParams &params);
	static void StopMoving(ScriptMethodParams &params);
	static void Tint(ScriptMethodParams &params);
	static void GetObjectAtRoom(ScriptMethodParams &params);
	static void GetObjectAtScreen(ScriptMethodParams &params);
	static void GetAnimating(ScriptMethodParams &params);
	static void GetBaseline(ScriptMethodParams &params);
	static void SetBaseline(ScriptMethodParams &params);
	static void GetBlockingHeight(ScriptMethodParams &params);
	static void SetBlockingHeight(ScriptMethodParams &params);
	static void GetBlockingWidth(ScriptMethodParams &params);
	static void SetBlockingWidth(ScriptMethodParams &params);
	static void GetClickable(ScriptMethodParams &params);
	static void SetClickable(ScriptMethodParams &params);
	static void GetFrame(ScriptMethodParams &params);
	static void GetGraphic(ScriptMethodParams &params);
	static void SetGraphic(ScriptMethodParams &params);
	static void GetID(ScriptMethodParams &params);
	static void GetIgnoreScaling(ScriptMethodParams &params);
	static void SetIgnoreScaling(ScriptMethodParams &params);
	static void GetIgnoreWalkbehinds(ScriptMethodParams &params);
	static void SetIgnoreWalkbehinds(ScriptMethodParams &params);
	static void GetLoop(ScriptMethodParams &params);
	static void GetMoving(ScriptMethodParams &params);
	static void GetName_New(ScriptMethodParams &params);
	static void GetSolid(ScriptMethodParams &params);
	static void SetSolid(ScriptMethodParams &params);
	static void GetTransparency(ScriptMethodParams &params);
	static void SetTransparency(ScriptMethodParams &params);
	static void GetView(ScriptMethodParams &params);
	static void GetVisible(ScriptMethodParams &params);
	static void SetVisible(ScriptMethodParams &params);
	static void GetX(ScriptMethodParams &params);
	static void SetX(ScriptMethodParams &params);
	static void GetY(ScriptMethodParams &params);
	static void SetY(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
