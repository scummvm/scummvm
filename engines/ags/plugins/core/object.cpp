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

#include "ags/plugins/core/object.h"
#include "ags/engine/ac/object.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Object::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Object::Animate^5, Animate);
	SCRIPT_METHOD_EXT(Object::IsCollidingWithObject^1, IsCollidingWithObject);
	SCRIPT_METHOD_EXT(Object::GetName^1, GetName);
	SCRIPT_METHOD_EXT(Object::GetProperty^1, GetProperty);
	SCRIPT_METHOD_EXT(Object::GetPropertyText^2, GetPropertyText);
	SCRIPT_METHOD_EXT(Object::GetTextProperty^1, GetTextProperty);
	SCRIPT_METHOD_EXT(Object::MergeIntoBackground^0, MergeIntoBackground);
	SCRIPT_METHOD_EXT(Object::Move^5, Move);
	SCRIPT_METHOD_EXT(Object::RemoveTint^0, RemoveTint);
	SCRIPT_METHOD_EXT(Object::RunInteraction^1, RunInteraction);
	SCRIPT_METHOD_EXT(Object::SetPosition^2, SetPosition);
	SCRIPT_METHOD_EXT(Object::SetView^3, SetView);
	SCRIPT_METHOD_EXT(Object::StopAnimating^0, StopAnimating);
	SCRIPT_METHOD_EXT(Object::StopMoving^0, StopMoving);
	SCRIPT_METHOD_EXT(Object::Tint^5, Tint);
	SCRIPT_METHOD_EXT(Object::GetAtRoomXY^2, GetObjectAtRoom);
	SCRIPT_METHOD_EXT(Object::GetAtScreenXY^2, GetObjectAtScreen);
	SCRIPT_METHOD_EXT(Object::get_Animating, GetAnimating);
	SCRIPT_METHOD_EXT(Object::get_Baseline, GetBaseline);
	SCRIPT_METHOD_EXT(Object::set_Baseline, SetBaseline);
	SCRIPT_METHOD_EXT(Object::get_BlockingHeight, GetBlockingHeight);
	SCRIPT_METHOD_EXT(Object::set_BlockingHeight, SetBlockingHeight);
	SCRIPT_METHOD_EXT(Object::get_BlockingWidth, GetBlockingWidth);
	SCRIPT_METHOD_EXT(Object::set_BlockingWidth, SetBlockingWidth);
	SCRIPT_METHOD_EXT(Object::get_Clickable, GetClickable);
	SCRIPT_METHOD_EXT(Object::set_Clickable, SetClickable);
	SCRIPT_METHOD_EXT(Object::get_Frame, GetFrame);
	SCRIPT_METHOD_EXT(Object::get_Graphic, GetGraphic);
	SCRIPT_METHOD_EXT(Object::set_Graphic, SetGraphic);
	SCRIPT_METHOD_EXT(Object::get_ID, GetID);
	SCRIPT_METHOD_EXT(Object::get_IgnoreScaling, GetIgnoreScaling);
	SCRIPT_METHOD_EXT(Object::set_IgnoreScaling, SetIgnoreScaling);
	SCRIPT_METHOD_EXT(Object::get_IgnoreWalkbehinds, GetIgnoreWalkbehinds);
	SCRIPT_METHOD_EXT(Object::set_IgnoreWalkbehinds, SetIgnoreWalkbehinds);
	SCRIPT_METHOD_EXT(Object::get_Loop, GetLoop);
	SCRIPT_METHOD_EXT(Object::get_Moving, GetMoving);
	SCRIPT_METHOD_EXT(Object::get_Name, GetName_New);
	SCRIPT_METHOD_EXT(Object::get_Solid, GetSolid);
	SCRIPT_METHOD_EXT(Object::set_Solid, SetSolid);
	SCRIPT_METHOD_EXT(Object::get_Transparency, GetTransparency);
	SCRIPT_METHOD_EXT(Object::set_Transparency, SetTransparency);
	SCRIPT_METHOD_EXT(Object::get_View, GetView);
	SCRIPT_METHOD_EXT(Object::get_Visible, GetVisible);
	SCRIPT_METHOD_EXT(Object::set_Visible, SetVisible);
	SCRIPT_METHOD_EXT(Object::get_X, GetX);
	SCRIPT_METHOD_EXT(Object::set_X, SetX);
	SCRIPT_METHOD_EXT(Object::get_Y, GetY);
	SCRIPT_METHOD_EXT(Object::set_Y, SetY);
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
