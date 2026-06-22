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

#ifndef DIRECTOR_LINGO_XTRAS_A_ACROVIEWER_H
#define DIRECTOR_LINGO_XTRAS_A_ACROVIEWER_H

namespace Director {

class AcroviewerXtraObject : public Object<AcroviewerXtraObject> {
public:
	AcroviewerXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace AcroviewerXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_release(int nargs);
void m_getLastError(int nargs);
void m_lookForAcrobat(int nargs);
void m_launchViewer(int nargs);
void m_bringViewerToFront(int nargs);
void m_bringDirectorToFront(int nargs);
void m_quitViewer(int nargs);
void m_openViewerDocument(int nargs);
void m_openDocInvisible(int nargs);
void m_maximizeWindow(int nargs);
void m_printOpenDocument(int nargs);
void m_closeAllDocuments(int nargs);
void m_getAppWindowRect(int nargs);
void m_setAppWindowRect(int nargs);
void m_executeMenuItem(int nargs);
void m_getNumPages(int nargs);
void m_getNumOpenDocs(int nargs);
void m_bringDocForward(int nargs);
void m_setViewMode(int nargs);
void m_findText(int nargs);
void m_clearSelection(int nargs);
void m_printPages(int nargs);
void m_closeViewerDocument(int nargs);
void m_gotoPage(int nargs);
void m_gotoNext(int nargs);
void m_gotoPrevious(int nargs);
void m_gotoFirst(int nargs);
void m_gotoLast(int nargs);
void m_readPageUp(int nargs);
void m_readPageDown(int nargs);
void m_readPageLeft(int nargs);
void m_readPageRight(int nargs);
void m_scrollPage(int nargs);
void m_zoomPage(int nargs);
void m_goBackward(int nargs);
void m_goForward(int nargs);
void m_drawIntoXWindow(int nargs);
void m_hideXWindow(int nargs);
void m_showXWindow(int nargs);
void m_disposeXWindow(int nargs);
void m_getStageAndParentParams(int nargs);
void m_setStageAndParentParams(int nargs);
void m_status(int nargs);
void m_"(int nargs);

} // End of namespace AcroviewerXtra

} // End of namespace Director

#endif
