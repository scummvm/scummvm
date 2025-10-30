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

#ifndef DIRECTOR_LINGO_XTRAS_S_SMACKER_H
#define DIRECTOR_LINGO_XTRAS_S_SMACKER_H

namespace Director {

class SmackerXtraObject : public Object<SmackerXtraObject> {
public:
	SmackerXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace SmackerXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_SmackQuickPlay(int nargs);
void m_SmackQuickPlayTrans(int nargs);
void m_SmackOpen(int nargs);
void m_SmackOpenTrans(int nargs);
void m_SmackClose(int nargs);
void m_SmackPlay(int nargs);
void m_SmackPlayFrames(int nargs);
void m_SmackPlayLooped(int nargs);
void m_SmackPlayNext(int nargs);
void m_SmackRemapToSystemPalette(int nargs);
void m_SmackRemapToPalette(int nargs);
void m_SmackRemapToBitmap(int nargs);
void m_SmackSetWindowStyle(int nargs);
void m_SmackSetWindowTitle(int nargs);
void m_SmackSetTransBackground(int nargs);
void m_SmackSetInterfaceKeys(int nargs);
void m_SmackSetBitmap(int nargs);
void m_SmackSetPosition(int nargs);
void m_SmackSetAlignment(int nargs);
void m_SmackGoto(int nargs);
void m_SmackGetFramesPerSecond(int nargs);
void m_SmackGetFrameNum(int nargs);
void m_SmackGetFrames(int nargs);
void m_SmackGetHeight(int nargs);
void m_SmackGetWidth(int nargs);
void m_SmackGetLastKey(int nargs);
void m_SmackGetMouseX(int nargs);
void m_SmackGetMouseY(int nargs);
void m_SmackGetMouseClickX(int nargs);
void m_SmackGetMouseClickY(int nargs);
void m_SmackHideVideo(int nargs);
void m_SmackSetDisplayMode(int nargs);
void m_SmackGetSummary(int nargs);
void m_SmackScreenMethod(int nargs);

} // End of namespace SmackerXtra

} // End of namespace Director

#endif
