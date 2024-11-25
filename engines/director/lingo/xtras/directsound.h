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

#ifndef DIRECTOR_LINGO_XTRAS_DIRECTSOUND_H
#define DIRECTOR_LINGO_XTRAS_DIRECTSOUND_H

namespace Director {

class DirectsoundXtraObject : public Object<DirectsoundXtraObject> {
public:
	DirectsoundXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace DirectsoundXtra {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dsOpen(int nargs);
void m_dsNewSound(int nargs);
void m_dsDelSound(int nargs);
void m_dsDupSound(int nargs);
void m_dsPlay(int nargs);
void m_dsStop(int nargs);
void m_dsGetSize(int nargs);
void m_dsGetFreq(int nargs);
void m_dsSetFreq(int nargs);
void m_dsGetVolume(int nargs);
void m_dsSetVolume(int nargs);
void m_dsGetPan(int nargs);
void m_dsSetPan(int nargs);
void m_dsGetPosition(int nargs);
void m_dsSetPosition(int nargs);
void m_dsGetLoop(int nargs);
void m_dsSetLoop(int nargs);
void m_dsIsPlaying(int nargs);
void m_dsGetCaps(int nargs);
void m_ds3DOpen(int nargs);
void m_ds3DLGetPosition(int nargs);
void m_ds3DLSetPosition(int nargs);
void m_ds3DLGetOrientation(int nargs);
void m_ds3DLSetOrientation(int nargs);
void m_ds3DLGetVelocity(int nargs);
void m_ds3DLSetVelocity(int nargs);
void m_ds3DLGetDopplerFactor(int nargs);
void m_ds3DLSetDopplerFactor(int nargs);
void m_ds3DLGetRolloffFactor(int nargs);
void m_ds3DLSetRolloffFactor(int nargs);
void m_ds3DGetPosition(int nargs);
void m_ds3DSetPosition(int nargs);
void m_ds3DGetOrientation(int nargs);
void m_ds3DSetOrientation(int nargs);
void m_ds3DGetVelocity(int nargs);
void m_ds3DSetVelocity(int nargs);
void m_ds3DGetOutsideVolume(int nargs);
void m_ds3DSetOutsideVolume(int nargs);
void m_ds3DGetAngles(int nargs);
void m_ds3DSetAngles(int nargs);
void m_ds3DGetMaxDistance(int nargs);
void m_ds3DSetMaxDistance(int nargs);
void m_ds3DGetMinDistance(int nargs);
void m_ds3DSetMinDistance(int nargs);
void m_dsClose(int nargs);

} // End of namespace DirectsoundXtra

} // End of namespace Director

#endif
