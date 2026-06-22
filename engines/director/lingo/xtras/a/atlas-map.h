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

#ifndef DIRECTOR_LINGO_XTRAS_A_ATLAS-MAP_H
#define DIRECTOR_LINGO_XTRAS_A_ATLAS-MAP_H

namespace Director {

class MapXtraObject : public Object<MapXtraObject> {
public:
	MapXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace MapXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_XInitMapEnv(int nargs);
void m_XAdSSpriteInit(int nargs);
void m_XAdSSpriteVisibility(int nargs);
void m_XQueryNameList(int nargs);
void m_XGetNameList(int nargs);
void m_XGetNameListPosition(int nargs);
void m_XQuery(int nargs);
void m_XMouseDown(int nargs);
void m_XUpdate(int nargs);
void m_XUpdateLock(int nargs);
void m_XRedrawLock(int nargs);
void m_XIdle(int nargs);
void m_XSetMapArea(int nargs);
void m_XSetMapScale(int nargs);
void m_XSetNavigationMode(int nargs);
void m_XBaseMapVis(int nargs);
void m_XThematicLayerBlend(int nargs);
void m_XSetActiveLayer(int nargs);
void m_XSpaceAndTimeMenuCall(int nargs);
void m_XMarkMapItem(int nargs);
void m_XGetColorByID(int nargs);
void m_XValuePlusCommand(int nargs);
void m_XAddOverlapRect(int nargs);
void m_XRemoveOverlapRect(int nargs);
void m_XPinVisibility(int nargs);
void m_XPin(int nargs);
void m_XKeyDown(int nargs);
void m_XAnalyzerCommand(int nargs);
void m_XAnalyzerChangeColor(int nargs);
void m_XThemeMenuLoad(int nargs);
void m_XThemeMenuUnload(int nargs);
void m_XThemeMenuCall(int nargs);
void m_XThemeMenuCall_2(int nargs);
void m_XThemeMenuCurrent(int nargs);
void m_XThemeMenuLastResult(int nargs);
void m_XThemeMenuGetIndex(int nargs);
void m_XTextLoad(int nargs);
void m_XText(int nargs);
void m_XHistory(int nargs);
void m_XImportXML(int nargs);
void m_XExportXML(int nargs);
void m_XPrint(int nargs);
void m_XExportImage(int nargs);
void m_XSetPreferences(int nargs);
void m_XPreferences(int nargs);
void m_"(int nargs);

} // End of namespace MapXtra

} // End of namespace Director

#endif
