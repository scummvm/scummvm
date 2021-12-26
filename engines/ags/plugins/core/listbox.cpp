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

#include "ags/plugins/core/listbox.h"
#include "ags/engine/ac/listbox.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void ListBox::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(ListBox::AddItem^1, ListBox::AddItem);
	SCRIPT_METHOD(ListBox::Clear^0, ListBox::Clear);
	SCRIPT_METHOD(ListBox::FillDirList^1, ListBox::FillDirList);
	SCRIPT_METHOD(ListBox::FillSaveGameList^0, ListBox::FillSaveGameList);
	SCRIPT_METHOD(ListBox::GetItemAtLocation^2, ListBox::GetItemAtLocation);
	SCRIPT_METHOD(ListBox::GetItemText^2, ListBox::GetItemText);
	SCRIPT_METHOD(ListBox::InsertItemAt^2, ListBox::InsertItemAt);
	SCRIPT_METHOD(ListBox::RemoveItem^1, ListBox::RemoveItem);
	SCRIPT_METHOD(ListBox::ScrollDown^0, ListBox::ScrollDown);
	SCRIPT_METHOD(ListBox::ScrollUp^0, ListBox::ScrollUp);
	SCRIPT_METHOD(ListBox::SetItemText^2, ListBox::SetItemText);
	SCRIPT_METHOD(ListBox::get_Font, ListBox::GetFont);
	SCRIPT_METHOD(ListBox::set_Font, ListBox::SetFont);
	SCRIPT_METHOD(ListBox::get_HideBorder, ListBox::GetHideBorder);
	SCRIPT_METHOD(ListBox::set_HideBorder, ListBox::SetHideBorder);
	SCRIPT_METHOD(ListBox::get_HideScrollArrows, ListBox::GetHideScrollArrows);
	SCRIPT_METHOD(ListBox::set_HideScrollArrows, ListBox::SetHideScrollArrows);
	SCRIPT_METHOD(ListBox::get_ItemCount, ListBox::GetItemCount);
	SCRIPT_METHOD(ListBox::geti_Items, ListBox::GetItems);
	SCRIPT_METHOD(ListBox::seti_Items, ListBox::SetItemText);
	SCRIPT_METHOD(ListBox::get_RowCount, ListBox::GetRowCount);
	SCRIPT_METHOD(ListBox::geti_SaveGameSlots, ListBox::GetSaveGameSlots);
	SCRIPT_METHOD(ListBox::get_SelectedIndex, ListBox::GetSelectedIndex);
	SCRIPT_METHOD(ListBox::set_SelectedIndex, ListBox::SetSelectedIndex);
	SCRIPT_METHOD(ListBox::get_TopItem, ListBox::GetTopItem);
	SCRIPT_METHOD(ListBox::set_TopItem, ListBox::SetTopItem);
}

void ListBox::AddItem(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, lbb, const char *, text);
	params._result = AGS3::ListBox_AddItem(lbb, text);
}

void ListBox::Clear(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	AGS3::ListBox_Clear(listbox);
}

void ListBox::FillDirList(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, const char *, filemask);
	AGS3::ListBox_FillDirList(listbox, filemask);
}

void ListBox::FillSaveGameList(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_FillSaveGameList(listbox);
}

void ListBox::GetItemAtLocation(ScriptMethodParams &params) {
	PARAMS3(GUIListBox *, listbox, int, x, int, y);
	params._result = AGS3::ListBox_GetItemAtLocation(listbox, x, y);
}

void ListBox::GetItemText(ScriptMethodParams &params) {
	PARAMS3(GUIListBox *, listbox, int, index, char *, buffer);
	params._result = AGS3::ListBox_GetItemText(listbox, index, buffer);
}

void ListBox::InsertItemAt(ScriptMethodParams &params) {
	PARAMS3(GUIListBox *, lbb, int, index, const char *, text);
	params._result = AGS3::ListBox_InsertItemAt(lbb, index, text);
}

void ListBox::RemoveItem(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, itemIndex);
	AGS3::ListBox_RemoveItem(listbox, itemIndex);
}

void ListBox::ScrollDown(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	AGS3::ListBox_ScrollDown(listbox);
}

void ListBox::ScrollUp(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	AGS3::ListBox_ScrollUp(listbox);
}

void ListBox::SetItemText(ScriptMethodParams &params) {
	PARAMS3(GUIListBox *, listbox, int, index, const char *, newtext);
	AGS3::ListBox_SetItemText(listbox, index, newtext);
}

void ListBox::GetFont(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetFont(listbox);
}

void ListBox::SetFont(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, newfont);
	AGS3::ListBox_SetFont(listbox, newfont);
}

void ListBox::GetHideBorder(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetHideBorder(listbox);
}

void ListBox::SetHideBorder(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, newValue);
	AGS3::ListBox_SetHideBorder(listbox, newValue);
}

void ListBox::GetHideScrollArrows(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetHideScrollArrows(listbox);
}

void ListBox::SetHideScrollArrows(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, newValue);
	AGS3::ListBox_SetHideScrollArrows(listbox, newValue);
}

void ListBox::GetItemCount(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetItemCount(listbox);
}

void ListBox::GetItems(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, index);
	params._result = AGS3::ListBox_GetItems(listbox, index);
}

void ListBox::GetRowCount(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetRowCount(listbox);
}

void ListBox::GetSaveGameSlots(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, index);
	params._result = AGS3::ListBox_GetSaveGameSlots(listbox, index);
}

void ListBox::GetSelectedIndex(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetSelectedIndex(listbox);
}

void ListBox::SetSelectedIndex(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, guisl, int, newsel);
	AGS3::ListBox_SetSelectedIndex(guisl, newsel);
}

void ListBox::GetTopItem(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetTopItem(listbox);
}

void ListBox::SetTopItem(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, guisl, int, item);
	AGS3::ListBox_SetTopItem(guisl, item);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
