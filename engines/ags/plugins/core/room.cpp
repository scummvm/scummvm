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

#include "ags/plugins/core/room.h"
#include "ags/engine/ac/room.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Room::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Room::GetDrawingSurfaceForBackground^1, GetDrawingSurfaceForBackground);
	SCRIPT_METHOD_EXT(Room::GetProperty^1, GetProperty);
	SCRIPT_METHOD_EXT(Room::GetTextProperty^1, GetTextProperty);
	SCRIPT_METHOD_EXT(Room::get_BottomEdge, GetBottomEdge);
	SCRIPT_METHOD_EXT(Room::get_ColorDepth, GetColorDepth);
	SCRIPT_METHOD_EXT(Room::get_Height, GetHeight);
	SCRIPT_METHOD_EXT(Room::get_LeftEdge, GetLeftEdge);
	SCRIPT_METHOD_EXT(Room::geti_Messages, GetMessages);
	SCRIPT_METHOD_EXT(Room::get_MusicOnLoad, GetMusicOnLoad);
	SCRIPT_METHOD_EXT(Room::get_ObjectCount, GetObjectCount);
	SCRIPT_METHOD_EXT(Room::get_RightEdge, GetRightEdge);
	SCRIPT_METHOD_EXT(Room::get_TopEdge, GetTopEdge);
	SCRIPT_METHOD_EXT(Room::get_Width, GetWidth);
}

void Room::GetDrawingSurfaceForBackground(ScriptMethodParams &params) {
	PARAMS1(int, backgroundNumber);
	params._result = AGS3::Room_GetDrawingSurfaceForBackground(backgroundNumber);
}

void Room::GetProperty(ScriptMethodParams &params) {
	PARAMS1(const char *, property);
	params._result = AGS3::Room_GetProperty(property);
}

void Room::GetTextProperty(ScriptMethodParams &params) {
	PARAMS1(const char *, property);
	params._result = AGS3::Room_GetTextProperty(property);
}

void Room::GetBottomEdge(ScriptMethodParams &params) {
	params._result = AGS3::Room_GetBottomEdge();
}

void Room::GetColorDepth(ScriptMethodParams &params) {
	params._result = AGS3::Room_GetColorDepth();
}

void Room::GetHeight(ScriptMethodParams &params) {
	params._result = AGS3::Room_GetHeight();
}

void Room::GetLeftEdge(ScriptMethodParams &params) {
	params._result = AGS3::Room_GetLeftEdge();
}

void Room::GetMessages(ScriptMethodParams &params) {
	PARAMS1(int, index);
	params._result = AGS3::Room_GetMessages(index);
}

void Room::GetMusicOnLoad(ScriptMethodParams &params) {
	params._result = AGS3::Room_GetMusicOnLoad();
}

void Room::GetObjectCount(ScriptMethodParams &params) {
	params._result = AGS3::Room_GetObjectCount();
}

void Room::GetRightEdge(ScriptMethodParams &params) {
	params._result = AGS3::Room_GetRightEdge();
}

void Room::GetTopEdge(ScriptMethodParams &params) {
	params._result = AGS3::Room_GetTopEdge();
}

void Room::GetWidth(ScriptMethodParams &params) {
	params._result = AGS3::Room_GetWidth();
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
