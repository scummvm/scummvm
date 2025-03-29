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

#ifndef DIRECTOR_LINGO_XTRAS_RTK_H
#define DIRECTOR_LINGO_XTRAS_RTK_H

namespace Director {

class RolloverToolkitXtraObject : public Object<RolloverToolkitXtraObject> {
public:
	RolloverToolkitXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace RolloverToolkitXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_CheckForRollovers(int nargs);
void m_CurrentRollover(int nargs);
void m_EndAnyRollovers(int nargs);
void m_ResetRollovers(int nargs);
void m_EnableCastmemberDetection(int nargs);
void m_DisableCastmemberDetection(int nargs);
void m_GetCastmemberDetectionStatus(int nargs);
void m_AutoSetSpriteRange(int nargs);
void m_MonitorAllSprites(int nargs);
void m_MonitorNoSprites(int nargs);
void m_MonitorSpriteRange(int nargs);
void m_UnMonitorSpriteRange(int nargs);
void m_MonitorSprite(int nargs);
void m_UnMonitorSprite(int nargs);
void m_GetMonitorStatus(int nargs);
void m_DumpMonitorStatus(int nargs);
void m_ShowRTKVersion(int nargs);
void m_EnableMatteDetection(int nargs);
void m_DisableMatteDetection(int nargs);
void m_GetMatteDetectionStatus(int nargs);
void m_rtkRegisterMac(int nargs);
void m_rtkRegisterX16(int nargs);
void m_rtkRegisterX32(int nargs);

} // End of namespace RolloverToolkitXtra

} // End of namespace Director

#endif
