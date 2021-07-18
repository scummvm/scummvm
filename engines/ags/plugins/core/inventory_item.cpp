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

#include "ags/plugins/core/inventory_item.h"
#include "ags/engine/ac/inventory_item.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void InventoryItem::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(InventoryItem::GetAtScreenXY ^ 2, GetInvAtLocation);
	SCRIPT_METHOD_EXT(InventoryItem::IsInteractionAvailable ^ 1, CheckInteractionAvailable);
	SCRIPT_METHOD_EXT(InventoryItem::GetName ^ 1, GetName);
	SCRIPT_METHOD_EXT(InventoryItem::GetProperty ^ 1, GetProperty);
	SCRIPT_METHOD_EXT(InventoryItem::GetPropertyText ^ 2, GetPropertyText);
	SCRIPT_METHOD_EXT(InventoryItem::GetTextProperty ^ 1, GetTextProperty);
	SCRIPT_METHOD_EXT(InventoryItem::RunInteraction ^ 1, RunInteraction);
	SCRIPT_METHOD_EXT(InventoryItem::SetName ^ 1, SetName);
	SCRIPT_METHOD_EXT(InventoryItem::get_CursorGraphic, GetCursorGraphic);
	SCRIPT_METHOD_EXT(InventoryItem::set_CursorGraphic, SetCursorGraphic);
	SCRIPT_METHOD_EXT(InventoryItem::get_Graphic, GetGraphic);
	SCRIPT_METHOD_EXT(InventoryItem::set_Graphic, SetGraphic);
	SCRIPT_METHOD_EXT(InventoryItem::get_ID, GetID);
	SCRIPT_METHOD_EXT(InventoryItem::get_Name, GetName_New);
	SCRIPT_METHOD_EXT(InventoryItem::set_Name, SetName);
}

void InventoryItem::GetInvAtLocation(ScriptMethodParams &params) {
	PARAMS2(int, xx, int, yy);
	params._result = AGS3::GetInvAtLocation(xx, yy);
}

void InventoryItem::CheckInteractionAvailable(ScriptMethodParams &params) {
	PARAMS2(ScriptInvItem *, iitem, int, mood);
	params._result = AGS3::InventoryItem_CheckInteractionAvailable(iitem, mood);
}

void InventoryItem::GetName(ScriptMethodParams &params) {
	PARAMS2(ScriptInvItem *, iitem, char *, buff);
	AGS3::InventoryItem_GetName(iitem, buff);
}

void InventoryItem::GetProperty(ScriptMethodParams &params) {
	PARAMS2(ScriptInvItem *, scii, const char *, property);
	params._result = AGS3::InventoryItem_GetProperty(scii, property);
}

void InventoryItem::GetPropertyText(ScriptMethodParams &params) {
	PARAMS3(ScriptInvItem *, scii, const char *, property, char *, bufer);
	AGS3::InventoryItem_GetPropertyText(scii, property, bufer);
}

void InventoryItem::GetTextProperty(ScriptMethodParams &params) {
	PARAMS2(ScriptInvItem *, scii, const char *, property);
	params._result = AGS3::InventoryItem_GetTextProperty(scii, property);
}

void InventoryItem::RunInteraction(ScriptMethodParams &params) {
	PARAMS2(ScriptInvItem *, iitem, int, mood);
	AGS3::InventoryItem_RunInteraction(iitem, mood);
}

void InventoryItem::SetName(ScriptMethodParams &params) {
	PARAMS2(ScriptInvItem *, scii, const char *, newname);
	AGS3::InventoryItem_SetName(scii, newname);
}

void InventoryItem::GetCursorGraphic(ScriptMethodParams &params) {
	PARAMS1(ScriptInvItem *, iitem);
	params._result = AGS3::InventoryItem_GetCursorGraphic(iitem);
}

void InventoryItem::SetCursorGraphic(ScriptMethodParams &params) {
	PARAMS2(ScriptInvItem *, iitem, int, newSprite);
	AGS3::InventoryItem_SetCursorGraphic(iitem, newSprite);
}

void InventoryItem::GetGraphic(ScriptMethodParams &params) {
	PARAMS1(ScriptInvItem *, iitem);
	params._result = AGS3::InventoryItem_GetGraphic(iitem);
}

void InventoryItem::SetGraphic(ScriptMethodParams &params) {
	PARAMS2(ScriptInvItem *, iitem, int, piccy);
	AGS3::InventoryItem_SetGraphic(iitem, piccy);
}

void InventoryItem::GetID(ScriptMethodParams &params) {
	PARAMS1(ScriptInvItem *, iitem);
	params._result = AGS3::InventoryItem_GetID(iitem);
}

void InventoryItem::GetName_New(ScriptMethodParams &params) {
	PARAMS1(ScriptInvItem *, iitem);
	params._result = AGS3::InventoryItem_GetName_New(iitem);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
