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

#ifndef DIRECTOR_LINGO_XLIBS_PRINTOMATIC_H
#define DIRECTOR_LINGO_XLIBS_PRINTOMATIC_H

namespace Director {

class PrintOMaticXObject : public Object<PrintOMaticXObject> {
public:
	PrintOMaticXObject(ObjectType objType);
};

namespace PrintOMaticXObj {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_reset(int nargs);
void m_newPage(int nargs);
void m_setPage(int nargs);
void m_setMargins(int nargs);
void m_setPrintableMargins(int nargs);
void m_getPageWidth(int nargs);
void m_getPageHeight(int nargs);
void m_getPaperWidth(int nargs);
void m_getPaperHeight(int nargs);
void m_setColor(int nargs);
void m_setGray(int nargs);
void m_setPenSize(int nargs);
void m_setLineWeight(int nargs);
void m_setTextFont(int nargs);
void m_setTextSize(int nargs);
void m_setTextStyle(int nargs);
void m_setTextJust(int nargs);
void m_setTextLineSpacing(int nargs);
void m_textBox(int nargs);
void m_setText(int nargs);
void m_appendText(int nargs);
void m_appendTextFile(int nargs);
void m_appendTextResource(int nargs);
void m_newFrame(int nargs);
void m_drawRect(int nargs);
void m_drawLine(int nargs);
void m_drawRoundRect(int nargs);
void m_drawOval(int nargs);
void m_drawText(int nargs);
void m_drawPicture(int nargs);
void m_drawStagePicture(int nargs);
void m_getInsertionPoint(int nargs);
void m_masterTextBox(int nargs);
void m_appendMasterText(int nargs);
void m_appendMasterTextFiles(int nargs);
void m_masterPictBox(int nargs);
void m_appendMasterPict(int nargs);
void m_stageToMasterPict(int nargs);
void m_strokedRect(int nargs);
void m_filledRect(int nargs);
void m_strokedRoundRect(int nargs);
void m_filledRoundRect(int nargs);
void m_strokedOval(int nargs);
void m_filledOval(int nargs);
void m_line(int nargs);
void m_picture(int nargs);
void m_stagePicture(int nargs);
void m_1BitStagePicture(int nargs);
void m_ePSFile(int nargs);
void m_setLandscapeMode(int nargs);
void m_setDocumentName(int nargs);
void m_doPageSetup(int nargs);
void m_doJobSetup(int nargs);
void m_setProgressMsg(int nargs);
void m_setProgressPict(int nargs);
void m_setProgressLoc(int nargs);
void m_printPreview(int nargs);
void m_printPicts(int nargs);
void m_print(int nargs);
void m_savePageSetup(int nargs);
void m_getPageSetup(int nargs);
void m_hideMessages(int nargs);
void m_setPageNumSymbol(int nargs);
void m_register(int nargs);

} // End of namespace PrintOMaticXObj

} // End of namespace Director

#endif
