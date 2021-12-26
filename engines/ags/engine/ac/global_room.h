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

#ifndef AGS_ENGINE_AC_GLOBAL_ROOM_H
#define AGS_ENGINE_AC_GLOBAL_ROOM_H

namespace AGS3 {

void SetAmbientTint(int red, int green, int blue, int opacity, int luminance);
void SetAmbientLightLevel(int light_level);
void NewRoom(int nrnum);
void NewRoomEx(int nrnum, int newx, int newy);
void NewRoomNPC(int charid, int nrnum, int newx, int newy);
void ResetRoom(int nrnum);
int  HasPlayerBeenInRoom(int roomnum);
void CallRoomScript(int value);
int  HasBeenToRoom(int roomnum);
void GetRoomPropertyText(const char *property, char *bufer);

void SetBackgroundFrame(int frnum);
int GetBackgroundFrame();

} // namespace AGS3

#endif
