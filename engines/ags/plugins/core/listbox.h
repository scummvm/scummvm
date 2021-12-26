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

#ifndef AGS_PLUGINS_CORE_LISTBOX_H
#define AGS_PLUGINS_CORE_LISTBOX_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class ListBox : public ScriptContainer {
	BUILT_IN_HASH(ListBox)
public:
	virtual ~ListBox() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void AddItem(ScriptMethodParams &params);
	void Clear(ScriptMethodParams &params);
	void FillDirList(ScriptMethodParams &params);
	void FillSaveGameList(ScriptMethodParams &params);
	void GetItemAtLocation(ScriptMethodParams &params);
	void GetItemText(ScriptMethodParams &params);
	void InsertItemAt(ScriptMethodParams &params);
	void RemoveItem(ScriptMethodParams &params);
	void ScrollDown(ScriptMethodParams &params);
	void ScrollUp(ScriptMethodParams &params);
	void SetItemText(ScriptMethodParams &params);
	void GetFont(ScriptMethodParams &params);
	void SetFont(ScriptMethodParams &params);
	void GetHideBorder(ScriptMethodParams &params);
	void SetHideBorder(ScriptMethodParams &params);
	void GetHideScrollArrows(ScriptMethodParams &params);
	void SetHideScrollArrows(ScriptMethodParams &params);
	void GetItemCount(ScriptMethodParams &params);
	void GetItems(ScriptMethodParams &params);
	void GetRowCount(ScriptMethodParams &params);
	void GetSaveGameSlots(ScriptMethodParams &params);
	void GetSelectedIndex(ScriptMethodParams &params);
	void SetSelectedIndex(ScriptMethodParams &params);
	void GetTopItem(ScriptMethodParams &params);
	void SetTopItem(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
