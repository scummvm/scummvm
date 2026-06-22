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

#ifndef DIRECTOR_LINGO_XTRAS_A_APRESENTATIONTRAKKER_H
#define DIRECTOR_LINGO_XTRAS_A_APRESENTATIONTRAKKER_H

namespace Director {

class ApresentationtrakkerXtraObject : public Object<ApresentationtrakkerXtraObject> {
public:
	ApresentationtrakkerXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace ApresentationtrakkerXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_PT_SetINIFile(int nargs);
void m_PT_SetTrackingURL(int nargs);
void m_PT_GetVersion(int nargs);
void m_PT_SetOSVersion(int nargs);
void m_PT_SetRunLocation(int nargs);
void m_PT_SetDistributionGroup(int nargs);
void m_PT_SetAppVersion(int nargs);
void m_PT_SetExtraValue(int nargs);
void m_PT_SetETrackLocation(int nargs);
void m_PT_PostTracking(int nargs);
void m_PT_Register(int nargs);
void m_PT_TrackWebLink(int nargs);
void m_PT_GetTrackURL(int nargs);
void m_PT_Reset(int nargs);
void m_PT_GetStatus(int nargs);
void m_PT_SetTraceFile(int nargs);
void m_PT_TraceMsg(int nargs);
void m_PT_SetUserId(int nargs);
void m_PT_ClearTracking(int nargs);
void m_PT_SetViewCounter(int nargs);
void m_"(int nargs);

} // End of namespace ApresentationtrakkerXtra

} // End of namespace Director

#endif
