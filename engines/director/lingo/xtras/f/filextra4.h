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

#ifndef DIRECTOR_LINGO_XTRAS_F_FILEXTRA4_H
#define DIRECTOR_LINGO_XTRAS_F_FILEXTRA4_H

namespace Director {

class FileXtra4XtraObject : public Object<FileXtra4XtraObject> {
public:
	FileXtra4XtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace FileXtra4Xtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_fx_GetVersion(int nargs);
void m_fx_FileOpenDialog(int nargs);
void m_fx_FileSaveAsDialog(int nargs);
void m_fx_FileExists(int nargs);
void m_fx_FileIsLink(int nargs);
void m_fx_FileRename(int nargs);
void m_fx_FileDelete(int nargs);
void m_fx_FileRecycle(int nargs);
void m_fx_FileCopy(int nargs);
void m_fx_FileMove(int nargs);
void m_fx_FileGetWriteState(int nargs);
void m_fx_FileSetWriteState(int nargs);
void m_fx_FileGetModDate(int nargs);
void m_fx_FileGetModNumber(int nargs);
void m_fx_FileGetSize(int nargs);
void m_fx_FileGetType(int nargs);
void m_fx_FileSetType(int nargs);
void m_fx_FileCompare(int nargs);
void m_fx_FileOpenDocument(int nargs);
void m_fx_FilePrintDocument(int nargs);
void m_fx_FileGetAppPath(int nargs);
void m_fx_FileRunApp(int nargs);
void m_fx_LinkCreate(int nargs);
void m_fx_LinkResolve(int nargs);
void m_fx_FolderSelectDialog(int nargs);
void m_fx_FolderGetSpecialPath(int nargs);
void m_fx_FolderExists(int nargs);
void m_fx_FolderCreate(int nargs);
void m_fx_FolderRename(int nargs);
void m_fx_FolderDelete(int nargs);
void m_fx_FolderRecycle(int nargs);
void m_fx_FolderCopy(int nargs);
void m_fx_FolderMove(int nargs);
void m_fx_FolderGetWriteState(int nargs);
void m_fx_FolderSetWriteState(int nargs);
void m_fx_FolderSyncOneWay(int nargs);
void m_fx_FolderSyncBothWays(int nargs);
void m_fx_FolderToList(int nargs);
void m_fx_VolumeSelectDialog(int nargs);
void m_fx_VolumeExists(int nargs);
void m_fx_VolumeGetFreeBytes(int nargs);
void m_fx_VolumeGetTotalBytes(int nargs);
void m_fx_VolumeIsCDROM(int nargs);
void m_fx_VolumeIsRemovable(int nargs);
void m_fx_VolumeEject(int nargs);
void m_fx_VolumesToList(int nargs);
void m_fx_ErrorNumber(int nargs);
void m_fx_ErrorString(int nargs);

} // End of namespace FileXtra4Xtra

} // End of namespace Director

#endif
