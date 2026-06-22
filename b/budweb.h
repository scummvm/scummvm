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

#ifndef DIRECTOR_LINGO_XTRAS_B_BUDWEB_H
#define DIRECTOR_LINGO_XTRAS_B_BUDWEB_H

namespace Director {

class BudwebXtraObject : public Object<BudwebXtraObject> {
public:
	BudwebXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BudwebXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_bwDownloadFile(int nargs);
void m_bwDownloadFolder(int nargs);
void m_bwDownloadFileList(int nargs);
void m_bwUploadFile(int nargs);
void m_bwUploadFolder(int nargs);
void m_bwUploadFileList(int nargs);
void m_bwFileExists(int nargs);
void m_bwFileSize(int nargs);
void m_bwFileAge(int nargs);
void m_bwFolderExists(int nargs);
void m_bwFileList(int nargs);
void m_bwFolderList(int nargs);
void m_bwCreateFolder(int nargs);
void m_bwDeleteFile(int nargs);
void m_bwRenameFile(int nargs);
void m_bwWriteList(int nargs);
void m_bwReadList(int nargs);
void m_bwWriteFile(int nargs);
void m_bwReadFile(int nargs);
void m_bwGetFile(int nargs);
void m_bwWriteText(int nargs);
void m_bwReadText(int nargs);
void m_bwCloseFile(int nargs);
void m_bwCloseSession(int nargs);
void m_bwPing(int nargs);
void m_bwUploadedFile(int nargs);
void m_bwWebResult(int nargs);
void m_bwSetOption(int nargs);
void m_bwEncryptText(int nargs);
void m_bwDecryptText(int nargs);
void m_bwMsgBox(int nargs);
void m_bwMsgBoxEx(int nargs);
void m_bwPrompt(int nargs);
void m_bwLogIn(int nargs);
void m_bwBusyDlg(int nargs);
void m_bwCloseBusyDlg(int nargs);
void m_bwProgressDlg(int nargs);
void m_bwUpdateProgressDlg(int nargs);
void m_bwCloseProgressDlg(int nargs);
void m_bwMenuAt(int nargs);
void m_bwRegister(int nargs);
void m_"(int nargs);

} // End of namespace BudwebXtra

} // End of namespace Director

#endif
