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

#ifndef AGS_PLUGINS_CORE_ROOM_H
#define AGS_PLUGINS_CORE_ROOM_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class Room : public ScriptContainer {
	BUILT_IN_HASH(Room)
public:
	virtual ~Room() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void GetDrawingSurfaceForBackground(ScriptMethodParams &params);
	void GetProperty(ScriptMethodParams &params);
	void GetTextProperty(ScriptMethodParams &params);
	void GetBottomEdge(ScriptMethodParams &params);
	void GetColorDepth(ScriptMethodParams &params);
	void GetHeight(ScriptMethodParams &params);
	void GetLeftEdge(ScriptMethodParams &params);
	void GetMessages(ScriptMethodParams &params);
	void GetMusicOnLoad(ScriptMethodParams &params);
	void GetObjectCount(ScriptMethodParams &params);
	void GetRightEdge(ScriptMethodParams &params);
	void GetTopEdge(ScriptMethodParams &params);
	void GetWidth(ScriptMethodParams &params);
	void RoomExists(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
