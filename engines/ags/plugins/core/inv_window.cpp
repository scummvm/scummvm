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

#include "ags/plugins/core/inv_window.h"
#include "ags/engine/ac/inv_window.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void InvWindow::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(InvWindow::ScrollDown^0, InvWindow::ScrollDown);
	SCRIPT_METHOD(InvWindow::ScrollUp^0, InvWindow::ScrollUp);
	SCRIPT_METHOD(InvWindow::get_CharacterToUse, InvWindow::GetCharacterToUse);
	SCRIPT_METHOD(InvWindow::set_CharacterToUse, InvWindow::SetCharacterToUse);
	SCRIPT_METHOD(InvWindow::geti_ItemAtIndex, InvWindow::GetItemAtIndex);
	SCRIPT_METHOD(InvWindow::get_ItemCount, InvWindow::GetItemCount);
	SCRIPT_METHOD(InvWindow::get_ItemHeight, InvWindow::GetItemHeight);
	SCRIPT_METHOD(InvWindow::set_ItemHeight, InvWindow::SetItemHeight);
	SCRIPT_METHOD(InvWindow::get_ItemWidth, InvWindow::GetItemWidth);
	SCRIPT_METHOD(InvWindow::set_ItemWidth, InvWindow::SetItemWidth);
	SCRIPT_METHOD(InvWindow::get_ItemsPerRow, InvWindow::GetItemsPerRow);
	SCRIPT_METHOD(InvWindow::get_RowCount, InvWindow::GetRowCount);
	SCRIPT_METHOD(InvWindow::get_TopItem, InvWindow::GetTopItem);
	SCRIPT_METHOD(InvWindow::set_TopItem, InvWindow::SetTopItem);
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
