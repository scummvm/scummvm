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

#ifndef DIRECTOR_LINGO_XTRAS_B_BAM_H
#define DIRECTOR_LINGO_XTRAS_B_BAM_H

namespace Director {

class BamXtraObject : public Object<BamXtraObject> {
public:
	BamXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BamXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_New(int nargs);
void m_GetState(int nargs);
void m_Start(int nargs);
void m_Stop(int nargs);
void m_Maintenance(int nargs);
void m_Pause(int nargs);
void m_Resume(int nargs);
void m_Finish(int nargs);
void m_EndLevel(int nargs);
void m_AlterSpeed(int nargs);
void m_GetSpeed(int nargs);
void m_GetInputChannels(int nargs);
void m_SendInput(int nargs);
void m_SetDebugLevel(int nargs);
void m_SetSecretsEnabled(int nargs);
void m_GetProperties(int nargs);
void m_GetAllProperties(int nargs);
void m_GetPointValue(int nargs);
void m_GetNewAssetObjects(int nargs);
void m_GetGameState(int nargs);
void m_SetGameState(int nargs);
void m_GetCamera(int nargs);
void m_GetCameraLocation(int nargs);
void m_SetCameraToEntity(int nargs);
void m_SetCameraToPoint(int nargs);
void m_GetCameraOffset(int nargs);
void m_SetCameraOffset(int nargs);
void m_GetDisplayRect(int nargs);
void m_GetLocalOrigin(int nargs);
void m_AutoUpdateRect(int nargs);
void m_PointToDirectorAngle(int nargs);
void m_"(int nargs);

} // End of namespace BamXtra

} // End of namespace Director

#endif
