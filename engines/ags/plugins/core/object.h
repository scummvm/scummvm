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

#ifndef AGS_PLUGINS_CORE_OBJECT_H
#define AGS_PLUGINS_CORE_OBJECT_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Object : public ScriptContainer {
	BUILT_IN_HASH(Object)
public:
	virtual ~Object() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void Animate(ScriptMethodParams &params);
	void IsCollidingWithObject(ScriptMethodParams &params);
	void GetName(ScriptMethodParams &params);
	void GetProperty(ScriptMethodParams &params);
	void GetPropertyText(ScriptMethodParams &params);
	void GetTextProperty(ScriptMethodParams &params);
	void MergeIntoBackground(ScriptMethodParams &params);
	void Move(ScriptMethodParams &params);
	void RemoveTint(ScriptMethodParams &params);
	void RunInteraction(ScriptMethodParams &params);
	void SetPosition(ScriptMethodParams &params);
	void SetView(ScriptMethodParams &params);
	void StopAnimating(ScriptMethodParams &params);
	void StopMoving(ScriptMethodParams &params);
	void Tint(ScriptMethodParams &params);
	void GetObjectAtRoom(ScriptMethodParams &params);
	void GetObjectAtScreen(ScriptMethodParams &params);
	void GetAnimating(ScriptMethodParams &params);
	void GetBaseline(ScriptMethodParams &params);
	void SetBaseline(ScriptMethodParams &params);
	void GetBlockingHeight(ScriptMethodParams &params);
	void SetBlockingHeight(ScriptMethodParams &params);
	void GetBlockingWidth(ScriptMethodParams &params);
	void SetBlockingWidth(ScriptMethodParams &params);
	void GetClickable(ScriptMethodParams &params);
	void SetClickable(ScriptMethodParams &params);
	void GetFrame(ScriptMethodParams &params);
	void GetGraphic(ScriptMethodParams &params);
	void SetGraphic(ScriptMethodParams &params);
	void GetID(ScriptMethodParams &params);
	void GetIgnoreScaling(ScriptMethodParams &params);
	void SetIgnoreScaling(ScriptMethodParams &params);
	void GetIgnoreWalkbehinds(ScriptMethodParams &params);
	void SetIgnoreWalkbehinds(ScriptMethodParams &params);
	void GetLoop(ScriptMethodParams &params);
	void GetMoving(ScriptMethodParams &params);
	void GetName_New(ScriptMethodParams &params);
	void GetSolid(ScriptMethodParams &params);
	void SetSolid(ScriptMethodParams &params);
	void GetTransparency(ScriptMethodParams &params);
	void SetTransparency(ScriptMethodParams &params);
	void GetView(ScriptMethodParams &params);
	void GetVisible(ScriptMethodParams &params);
	void SetVisible(ScriptMethodParams &params);
	void GetX(ScriptMethodParams &params);
	void SetX(ScriptMethodParams &params);
	void GetY(ScriptMethodParams &params);
	void SetY(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
