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

#include "ags/plugins/core/room.h"
#include "ags/engine/ac/room.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Room::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Room::GetDrawingSurfaceForBackground^1, Room::GetDrawingSurfaceForBackground);
	SCRIPT_METHOD(Room::GetProperty^1, Room::GetProperty);
	SCRIPT_METHOD(Room::GetTextProperty^1, Room::GetTextProperty);
	SCRIPT_METHOD(Room::SetProperty^2, Room::SetProperty);
	SCRIPT_METHOD(Room::SetTextProperty^2, Room::SetTextProperty);
	SCRIPT_METHOD(Room::get_BottomEdge, Room::GetBottomEdge);
	SCRIPT_METHOD(Room::get_ColorDepth, Room::GetColorDepth);
	SCRIPT_METHOD(Room::get_Height, Room::GetHeight);
	SCRIPT_METHOD(Room::get_LeftEdge, Room::GetLeftEdge);
	SCRIPT_METHOD(Room::geti_Messages, Room::GetMessages);
	SCRIPT_METHOD(Room::get_MusicOnLoad, Room::GetMusicOnLoad);
	SCRIPT_METHOD(Room::get_ObjectCount, Room::GetObjectCount);
	SCRIPT_METHOD(Room::get_RightEdge, Room::GetRightEdge);
	SCRIPT_METHOD(Room::get_TopEdge, Room::GetTopEdge);
	SCRIPT_METHOD(Room::get_Width, Room::GetWidth);
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

void Room::SetProperty(ScriptMethodParams &params) {
	PARAMS2(const char *, property, int, value);
	params._result = AGS3::Room_SetProperty(property, value);
}

void Room::SetTextProperty(ScriptMethodParams &params) {
	PARAMS2(const char *, property, const char *, value);
	params._result = AGS3::Room_SetTextProperty(property, value);
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

void Room::RoomExists(ScriptMethodParams &params) {
	PARAMS1(int, room);
	params._result = AGS3::Room_Exists(room);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
