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

#include "ags/plugins/core/inv_window.h"
#include "ags/engine/ac/inv_window.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void InvWindow::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(InvWindow::ScrollDown^0, ScrollDown);
	SCRIPT_METHOD_EXT(InvWindow::ScrollUp^0, ScrollUp);
	SCRIPT_METHOD_EXT(InvWindow::get_CharacterToUse, GetCharacterToUse);
	SCRIPT_METHOD_EXT(InvWindow::set_CharacterToUse, SetCharacterToUse);
	SCRIPT_METHOD_EXT(InvWindow::geti_ItemAtIndex, GetItemAtIndex);
	SCRIPT_METHOD_EXT(InvWindow::get_ItemCount, GetItemCount);
	SCRIPT_METHOD_EXT(InvWindow::get_ItemHeight, GetItemHeight);
	SCRIPT_METHOD_EXT(InvWindow::set_ItemHeight, SetItemHeight);
	SCRIPT_METHOD_EXT(InvWindow::get_ItemWidth, GetItemWidth);
	SCRIPT_METHOD_EXT(InvWindow::set_ItemWidth, SetItemWidth);
	SCRIPT_METHOD_EXT(InvWindow::get_ItemsPerRow, GetItemsPerRow);
	SCRIPT_METHOD_EXT(InvWindow::get_RowCount, GetRowCount);
	SCRIPT_METHOD_EXT(InvWindow::get_TopItem, GetTopItem);
	SCRIPT_METHOD_EXT(InvWindow::set_TopItem, SetTopItem);
}

void InvWindow::ScrollDown(ScriptMethodParams &params) {
	PARAMS1(GUIInvWindow *, guii);
	AGS3::InvWindow_ScrollDown(guii);
}

void InvWindow::ScrollUp(ScriptMethodParams &params) {
	PARAMS1(GUIInvWindow *, guii);
	AGS3::InvWindow_ScrollUp(guii);
}

void InvWindow::GetCharacterToUse(ScriptMethodParams &params) {
	PARAMS1(GUIInvWindow *, guii);
	params._result = AGS3::InvWindow_GetCharacterToUse(guii);
}

void InvWindow::SetCharacterToUse(ScriptMethodParams &params) {
	PARAMS2(GUIInvWindow *, guii, CharacterInfo *, chaa);
	AGS3::InvWindow_SetCharacterToUse(guii, chaa);
}

void InvWindow::GetItemAtIndex(ScriptMethodParams &params) {
	PARAMS2(GUIInvWindow *, guii, int, index);
	params._result = AGS3::InvWindow_GetItemAtIndex(guii, index);
}

void InvWindow::GetItemCount(ScriptMethodParams &params) {
	PARAMS1(GUIInvWindow *, guii);
	params._result = AGS3::InvWindow_GetItemCount(guii);
}

void InvWindow::GetItemHeight(ScriptMethodParams &params) {
	PARAMS1(GUIInvWindow *, guii);
	params._result = AGS3::InvWindow_GetItemHeight(guii);
}

void InvWindow::SetItemHeight(ScriptMethodParams &params) {
	PARAMS2(GUIInvWindow *, guii, int, newhit);
	AGS3::InvWindow_SetItemHeight(guii, newhit);
}

void InvWindow::GetItemWidth(ScriptMethodParams &params) {
	PARAMS1(GUIInvWindow *, guii);
	params._result = AGS3::InvWindow_GetItemWidth(guii);
}

void InvWindow::SetItemWidth(ScriptMethodParams &params) {
	PARAMS2(GUIInvWindow *, guii, int, newwidth);
	AGS3::InvWindow_SetItemWidth(guii, newwidth);
}

void InvWindow::GetItemsPerRow(ScriptMethodParams &params) {
	PARAMS1(GUIInvWindow *, guii);
	params._result = AGS3::InvWindow_GetItemsPerRow(guii);
}

void InvWindow::GetRowCount(ScriptMethodParams &params) {
	PARAMS1(GUIInvWindow *, guii);
	params._result = AGS3::InvWindow_GetRowCount(guii);
}

void InvWindow::GetTopItem(ScriptMethodParams &params) {
	PARAMS1(GUIInvWindow *, guii);
	params._result = AGS3::InvWindow_GetTopItem(guii);
}

void InvWindow::SetTopItem(ScriptMethodParams &params) {
	PARAMS2(GUIInvWindow *, guii, int, topitem);
	AGS3::InvWindow_SetTopItem(guii, topitem);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
