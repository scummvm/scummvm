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

#include "ags/plugins/core/listbox.h"
#include "ags/engine/ac/listbox.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Listbox::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(ListBox::AddItem^1, AddItem);
	SCRIPT_METHOD_EXT(ListBox::Clear^0, Clear);
	SCRIPT_METHOD_EXT(ListBox::FillDirList^1, FillDirList);
	SCRIPT_METHOD_EXT(ListBox::FillSaveGameList^0, FillSaveGameList);
	SCRIPT_METHOD_EXT(ListBox::GetItemAtLocation^2, GetItemAtLocation);
	SCRIPT_METHOD_EXT(ListBox::GetItemText^2, GetItemText);
	SCRIPT_METHOD_EXT(ListBox::InsertItemAt^2, InsertItemAt);
	SCRIPT_METHOD_EXT(ListBox::RemoveItem^1, RemoveItem);
	SCRIPT_METHOD_EXT(ListBox::ScrollDown^0, ScrollDown);
	SCRIPT_METHOD_EXT(ListBox::ScrollUp^0, ScrollUp);
	SCRIPT_METHOD_EXT(ListBox::SetItemText^2, SetItemText);
	SCRIPT_METHOD_EXT(ListBox::get_Font, GetFont);
	SCRIPT_METHOD_EXT(ListBox::set_Font, SetFont);
	SCRIPT_METHOD_EXT(ListBox::get_HideBorder, GetHideBorder);
	SCRIPT_METHOD_EXT(ListBox::set_HideBorder, SetHideBorder);
	SCRIPT_METHOD_EXT(ListBox::get_HideScrollArrows, GetHideScrollArrows);
	SCRIPT_METHOD_EXT(ListBox::set_HideScrollArrows, SetHideScrollArrows);
	SCRIPT_METHOD_EXT(ListBox::get_ItemCount, GetItemCount);
	SCRIPT_METHOD_EXT(ListBox::geti_Items, GetItems);
	SCRIPT_METHOD_EXT(ListBox::seti_Items, SetItemText);
	SCRIPT_METHOD_EXT(ListBox::get_RowCount, GetRowCount);
	SCRIPT_METHOD_EXT(ListBox::geti_SaveGameSlots, GetSaveGameSlots);
	SCRIPT_METHOD_EXT(ListBox::get_SelectedIndex, GetSelectedIndex);
	SCRIPT_METHOD_EXT(ListBox::set_SelectedIndex, SetSelectedIndex);
	SCRIPT_METHOD_EXT(ListBox::get_TopItem, GetTopItem);
	SCRIPT_METHOD_EXT(ListBox::set_TopItem, SetTopItem);
}

void Listbox::AddItem(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, lbb, const char *, text);
	params._result = AGS3::ListBox_AddItem(lbb, text);
}

void Listbox::Clear(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	AGS3::ListBox_Clear(listbox);
}

void Listbox::FillDirList(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, const char *, filemask);
	AGS3::ListBox_FillDirList(listbox, filemask);
}

void Listbox::FillSaveGameList(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_FillSaveGameList(listbox);
}

void Listbox::GetItemAtLocation(ScriptMethodParams &params) {
	PARAMS3(GUIListBox *, listbox, int, x, int, y);
	params._result = AGS3::ListBox_GetItemAtLocation(listbox, x, y);
}

void Listbox::GetItemText(ScriptMethodParams &params) {
	PARAMS3(GUIListBox *, listbox, int, index, char *, buffer);
	params._result = AGS3::ListBox_GetItemText(listbox, index, buffer);
}

void Listbox::InsertItemAt(ScriptMethodParams &params) {
	PARAMS3(GUIListBox *, lbb, int, index, const char *, text);
	params._result = AGS3::ListBox_InsertItemAt(lbb, index, text);
}

void Listbox::RemoveItem(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, itemIndex);
	AGS3::ListBox_RemoveItem(listbox, itemIndex);
}

void Listbox::ScrollDown(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	AGS3::ListBox_ScrollDown(listbox);
}

void Listbox::ScrollUp(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	AGS3::ListBox_ScrollUp(listbox);
}

void Listbox::SetItemText(ScriptMethodParams &params) {
	PARAMS3(GUIListBox *, listbox, int, index, const char *, newtext);
	AGS3::ListBox_SetItemText(listbox, index, newtext);
}

void Listbox::GetFont(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetFont(listbox);
}

void Listbox::SetFont(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, newfont);
	AGS3::ListBox_SetFont(listbox, newfont);
}

void Listbox::GetHideBorder(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetHideBorder(listbox);
}

void Listbox::SetHideBorder(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, newValue);
	AGS3::ListBox_SetHideBorder(listbox, newValue);
}

void Listbox::GetHideScrollArrows(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetHideScrollArrows(listbox);
}

void Listbox::SetHideScrollArrows(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, newValue);
	AGS3::ListBox_SetHideScrollArrows(listbox, newValue);
}

void Listbox::GetItemCount(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetItemCount(listbox);
}

void Listbox::GetItems(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, index);
	params._result = AGS3::ListBox_GetItems(listbox, index);
}

void Listbox::GetRowCount(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetRowCount(listbox);
}

void Listbox::GetSaveGameSlots(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, listbox, int, index);
	params._result = AGS3::ListBox_GetSaveGameSlots(listbox, index);
}

void Listbox::GetSelectedIndex(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetSelectedIndex(listbox);
}

void Listbox::SetSelectedIndex(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, guisl, int, newsel);
	AGS3::ListBox_SetSelectedIndex(guisl, newsel);
}

void Listbox::GetTopItem(ScriptMethodParams &params) {
	PARAMS1(GUIListBox *, listbox);
	params._result = AGS3::ListBox_GetTopItem(listbox);
}

void Listbox::SetTopItem(ScriptMethodParams &params) {
	PARAMS2(GUIListBox *, guisl, int, item);
	AGS3::ListBox_SetTopItem(guisl, item);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
