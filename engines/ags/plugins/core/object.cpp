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

#include "ags/plugins/core/object.h"
#include "ags/engine/ac/object.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Object::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Object::Animate^5, Object::Animate);
	SCRIPT_METHOD(Object::IsCollidingWithObject^1, Object::IsCollidingWithObject);
	SCRIPT_METHOD(Object::GetName^1, Object::GetName);
	SCRIPT_METHOD(Object::GetProperty^1, Object::GetProperty);
	SCRIPT_METHOD(Object::GetPropertyText^2, Object::GetPropertyText);
	SCRIPT_METHOD(Object::GetTextProperty^1, Object::GetTextProperty);
	SCRIPT_METHOD(Object::MergeIntoBackground^0, Object::MergeIntoBackground);
	SCRIPT_METHOD(Object::Move^5, Object::Move);
	SCRIPT_METHOD(Object::RemoveTint^0, Object::RemoveTint);
	SCRIPT_METHOD(Object::RunInteraction^1, Object::RunInteraction);
	SCRIPT_METHOD(Object::SetPosition^2, Object::SetPosition);
	SCRIPT_METHOD(Object::SetView^3, Object::SetView);
	SCRIPT_METHOD(Object::StopAnimating^0, Object::StopAnimating);
	SCRIPT_METHOD(Object::StopMoving^0, Object::StopMoving);
	SCRIPT_METHOD(Object::Tint^5, Object::Tint);
	SCRIPT_METHOD(Object::GetAtRoomXY^2, Object::GetObjectAtRoom);
	SCRIPT_METHOD(Object::GetAtScreenXY^2, Object::GetObjectAtScreen);
	SCRIPT_METHOD(Object::get_Animating, Object::GetAnimating);
	SCRIPT_METHOD(Object::get_Baseline, Object::GetBaseline);
	SCRIPT_METHOD(Object::set_Baseline, Object::SetBaseline);
	SCRIPT_METHOD(Object::get_BlockingHeight, Object::GetBlockingHeight);
	SCRIPT_METHOD(Object::set_BlockingHeight, Object::SetBlockingHeight);
	SCRIPT_METHOD(Object::get_BlockingWidth, Object::GetBlockingWidth);
	SCRIPT_METHOD(Object::set_BlockingWidth, Object::SetBlockingWidth);
	SCRIPT_METHOD(Object::get_Clickable, Object::GetClickable);
	SCRIPT_METHOD(Object::set_Clickable, Object::SetClickable);
	SCRIPT_METHOD(Object::get_Frame, Object::GetFrame);
	SCRIPT_METHOD(Object::get_Graphic, Object::GetGraphic);
	SCRIPT_METHOD(Object::set_Graphic, Object::SetGraphic);
	SCRIPT_METHOD(Object::get_ID, Object::GetID);
	SCRIPT_METHOD(Object::get_IgnoreScaling, Object::GetIgnoreScaling);
	SCRIPT_METHOD(Object::set_IgnoreScaling, Object::SetIgnoreScaling);
	SCRIPT_METHOD(Object::get_IgnoreWalkbehinds, Object::GetIgnoreWalkbehinds);
	SCRIPT_METHOD(Object::set_IgnoreWalkbehinds, Object::SetIgnoreWalkbehinds);
	SCRIPT_METHOD(Object::get_Loop, Object::GetLoop);
	SCRIPT_METHOD(Object::get_Moving, Object::GetMoving);
	SCRIPT_METHOD(Object::get_Name, Object::GetName_New);
	SCRIPT_METHOD(Object::get_Solid, Object::GetSolid);
	SCRIPT_METHOD(Object::set_Solid, Object::SetSolid);
	SCRIPT_METHOD(Object::get_Transparency, Object::GetTransparency);
	SCRIPT_METHOD(Object::set_Transparency, Object::SetTransparency);
	SCRIPT_METHOD(Object::get_View, Object::GetView);
	SCRIPT_METHOD(Object::get_Visible, Object::GetVisible);
	SCRIPT_METHOD(Object::set_Visible, Object::SetVisible);
	SCRIPT_METHOD(Object::get_X, Object::GetX);
	SCRIPT_METHOD(Object::set_X, Object::SetX);
	SCRIPT_METHOD(Object::get_Y, Object::GetY);
	SCRIPT_METHOD(Object::set_Y, Object::SetY);
}

void Object::Animate(ScriptMethodParams &params) {
	PARAMS6(ScriptObject *, objj, int, loop, int, delay, int, repeat, int, blocking, int, direction);
	AGS3::Object_Animate(objj, loop, delay, repeat, blocking, direction);
}

void Object::IsCollidingWithObject(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, ScriptObject *, obj2);
	params._result = AGS3::Object_IsCollidingWithObject(objj, obj2);
}

void Object::GetName(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, char *, buffer);
	AGS3::Object_GetName(objj, buffer);
}

void Object::GetProperty(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, const char *, property);
	params._result = AGS3::Object_GetProperty(objj, property);
}

void Object::GetPropertyText(ScriptMethodParams &params) {
	PARAMS3(ScriptObject *, objj, const char *, property, char *, buffer);
	AGS3::Object_GetPropertyText(objj, property, buffer);
}

void Object::GetTextProperty(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, const char *, property);
	params._result = AGS3::Object_GetTextProperty(objj, property);
}

void Object::MergeIntoBackground(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	AGS3::Object_MergeIntoBackground(objj);
}

void Object::Move(ScriptMethodParams &params) {
	PARAMS6(ScriptObject *, objj, int, x, int, y, int, speed, int, blocking, int, direct);
	AGS3::Object_Move(objj, x, y, speed, blocking, direct);
}

void Object::RemoveTint(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	AGS3::Object_RemoveTint(objj);
}

void Object::RunInteraction(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, mode);
	AGS3::Object_RunInteraction(objj, mode);
}

void Object::SetPosition(ScriptMethodParams &params) {
	PARAMS3(ScriptObject *, objj, int, xx, int, yy);
	AGS3::Object_SetPosition(objj, xx, yy);
}

void Object::SetView(ScriptMethodParams &params) {
	PARAMS4(ScriptObject *, objj, int, view, int, loop, int, frame);
	AGS3::Object_SetView(objj, view, loop, frame);
}

void Object::StopAnimating(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	AGS3::Object_StopAnimating(objj);
}

void Object::StopMoving(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	AGS3::Object_StopMoving(objj);
}

void Object::Tint(ScriptMethodParams &params) {
	PARAMS6(ScriptObject *, objj, int, red, int, green, int, blue, int, saturation, int, luminance);
	AGS3::Object_Tint(objj, red, green, blue, saturation, luminance);
}

void Object::GetObjectAtRoom(ScriptMethodParams &params) {
	PARAMS2(int, x, int, y);
	params._result = AGS3::GetObjectAtRoom(x, y);
}

void Object::GetObjectAtScreen(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetObjectAtScreen(xx, yy);
}

void Object::GetAnimating(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetAnimating(objj);
}

void Object::GetBaseline(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetBaseline(objj);
}

void Object::SetBaseline(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, basel);
	AGS3::Object_SetBaseline(objj, basel);
}

void Object::GetBlockingHeight(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetBlockingHeight(objj);
}

void Object::SetBlockingHeight(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, bhit);
	AGS3::Object_SetBlockingHeight(objj, bhit);
}

void Object::GetBlockingWidth(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetBlockingWidth(objj);
}

void Object::SetBlockingWidth(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, bwid);
	AGS3::Object_SetBlockingWidth(objj, bwid);
}

void Object::GetClickable(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetClickable(objj);
}

void Object::SetClickable(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, clik);
	AGS3::Object_SetClickable(objj, clik);
}

void Object::GetFrame(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetFrame(objj);
}

void Object::GetGraphic(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetGraphic(objj);
}

void Object::SetGraphic(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, slott);
	AGS3::Object_SetGraphic(objj, slott);
}

void Object::GetID(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetID(objj);
}

void Object::GetIgnoreScaling(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetIgnoreScaling(objj);
}

void Object::SetIgnoreScaling(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, newval);
	AGS3::Object_SetIgnoreScaling(objj, newval);
}

void Object::GetIgnoreWalkbehinds(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetIgnoreWalkbehinds(objj);
}

void Object::SetIgnoreWalkbehinds(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, chaa, int, clik);
	AGS3::Object_SetIgnoreWalkbehinds(chaa, clik);
}

void Object::GetLoop(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetLoop(objj);
}

void Object::GetMoving(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetMoving(objj);
}

void Object::GetName_New(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetName_New(objj);
}

void Object::GetSolid(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetSolid(objj);
}

void Object::SetSolid(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, solid);
	AGS3::Object_SetSolid(objj, solid);
}

void Object::GetTransparency(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetTransparency(objj);
}

void Object::SetTransparency(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, trans);
	AGS3::Object_SetTransparency(objj, trans);
}

void Object::GetView(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetView(objj);
}

void Object::GetVisible(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetVisible(objj);
}

void Object::SetVisible(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, onOrOff);
	AGS3::Object_SetVisible(objj, onOrOff);
}

void Object::GetX(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetX(objj);
}

void Object::SetX(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, xx);
	AGS3::Object_SetY(objj, xx);
}

void Object::GetY(ScriptMethodParams &params) {
	PARAMS1(ScriptObject *, objj);
	params._result = AGS3::Object_GetY(objj);
}

void Object::SetY(ScriptMethodParams &params) {
	PARAMS2(ScriptObject *, objj, int, yy);
	AGS3::Object_SetY(objj, yy);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
