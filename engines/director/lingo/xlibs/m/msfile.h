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

#ifndef DIRECTOR_LINGO_XLIBS_MSFILE_H
#define DIRECTOR_LINGO_XLIBS_MSFILE_H

namespace Director {

enum MSFileError {
	msErrorNone = 0,
};

class MSFileObject : public Object<MSFileObject> {
public:
	MSFileError _lastError;

public:
	MSFileObject(ObjectType objType);
	MSFileObject(const MSFileObject &obj);
	~MSFileObject() override;

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;

	void dispose() override;
};

namespace MSFile {
extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

Common::Path resolveSourceFilePath(const Common::String &srcParam);
Common::SeekableReadStream *openSourceStream(const Common::Path &srcFilePath, const Common::String &srcParam, char dirSeperator);
Common::SeekableWriteStream *openDestinationStream(Common::String &destFileName);
bool copyStream(Common::SeekableReadStream *srcStream, Common::SeekableWriteStream *destStream);
bool copyGameFile(const Common::String &srcParam, const Common::String &destParam);

void m_new(int nargs);
void m_dispose(int nargs);
void m_name(int nargs);
void m_getFullDirList(int nargs);
void m_getFileList(int nargs);
void m_getDirList(int nargs);
void m_getDiskFree(int nargs);
void m_getVolLabel(int nargs);
void m_createDir(int nargs);
void m_removeDir(int nargs);
void m_getCurrDir(int nargs);
void m_setCurrDir(int nargs);
void m_removeFile(int nargs);
void m_renameFile(int nargs);
void m_copyFile(int nargs);
void m_copyFiles(int nargs);
void m_setAppINI(int nargs);
void m_getAppINI(int nargs);
void m_getWindowsDirectory(int nargs);
void m_getWinVer(int nargs);
void m_restartWindows(int nargs);
void m_openFileDlg(int nargs);
void m_saveFileDlg(int nargs);
void m_getOFileName(int nargs);
void m_getSFileName(int nargs);
void m_displayDlg(int nargs);
void m_progMgrAdd(int args);

} // End of namespace MSFile

} // End of namespace Director

#endif
