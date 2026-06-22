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

#ifndef DIRECTOR_LINGO_XTRAS_A_ATLAS-TOPO3D_H
#define DIRECTOR_LINGO_XTRAS_A_ATLAS-TOPO3D_H

namespace Director {

class Topo3dXtraObject : public Object<Topo3dXtraObject> {
public:
	Topo3dXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace Topo3dXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_XAdSSpriteInit(int nargs);
void m_XMouseWithin(int nargs);
void m_XMouseLeave(int nargs);
void m_XMouseDown(int nargs);
void m_XKeyDown(int nargs);
void m_XUpdate(int nargs);
void m_XAddOverlapRect(int nargs);
void m_XRemoveOverlapRect(int nargs);
void m_XIdle(int nargs);
void m_XSetValueF(int nargs);
void m_XSetValueS(int nargs);
void m_XResetTopo3D(int nargs);
void m_XDraw(int nargs);
void m_XSetInteractMode(int nargs);
void m_XReadTopoDescXML(int nargs);
void m_XSetTopoOffset(int nargs);
void m_XSetColoringState(int nargs);
void m_XColoringLoadModules(int nargs);
void m_XSetColoringVisPart(int nargs);
void m_XColoringVisPart(int nargs);
void m_XColoringRemoveModules(int nargs);
void m_XSetColoringValue(int nargs);
void m_XSetColoringReLoadModul(int nargs);
void m_XColoringDialog(int nargs);
void m_XColoringDialogCommand(int nargs);
void m_XColoringAnalyzerChangeColor(int nargs);
void m_XColoringValiDate(int nargs);
void m_XUpdatePreviewImage(int nargs);
void m_XPreview(int nargs);
void m_XRefMap(int nargs);
void m_XProfileDraw(int nargs);
void m_XProfile(int nargs);
void m_XTextLoad(int nargs);
void m_XText(int nargs);
void m_XListLoad(int nargs);
void m_XListCount(int nargs);
void m_XListPosition(int nargs);
void m_XListCommand(int nargs);
void m_XListEntriesByPosition(int nargs);
void m_XListEntriesBySearchString(int nargs);
void m_XListThemesPositionByID(int nargs);
void m_XHistory(int nargs);
void m_XImportXML(int nargs);
void m_XExportXML(int nargs);
void m_XPrint(int nargs);
void m_XExportImage(int nargs);
void m_XSetPreferences(int nargs);
void m_XPreferences(int nargs);
void m_"(int nargs);

} // End of namespace Topo3dXtra

} // End of namespace Director

#endif
