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

#ifndef DIRECTOR_LINGO_XTRAS_XSOUND_H
#define DIRECTOR_LINGO_XTRAS_XSOUND_H

namespace Director {

class XsoundXtraObject : public Object<XsoundXtraObject> {
public:
	XsoundXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace XsoundXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_forget(int nargs);
void m_Register(int nargs);
void m_GetError(int nargs);
void m_GetInfo(int nargs);
void m_SetInfo(int nargs);
void m_Status(int nargs);
void m_ConnectInputDevice(int nargs);
void m_DisconnectInputDevice(int nargs);
void m_SetSoundType(int nargs);
void m_ClearSoundType(int nargs);
void m_Play(int nargs);
void m_Record(int nargs);
void m_Stop(int nargs);
void m_Pause(int nargs);
void m_Resume(int nargs);
void m_GetCurrentTime(int nargs);
void m_IsASound(int nargs);
void m_DeleteSound(int nargs);
void m_SetPlaySegment(int nargs);
void m_ClearPlaySegment(int nargs);
void m_SetSampleRate(int nargs);
void m_SetSampleDepth(int nargs);
void m_SetCompression(int nargs);
void m_GetInputLevel(int nargs);
void m_FreeRecordingTime(int nargs);

} // End of namespace XsoundXtra

} // End of namespace Director

#endif
