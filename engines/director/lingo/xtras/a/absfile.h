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

#ifndef DIRECTOR_LINGO_XTRAS_A_ABSFILE_H
#define DIRECTOR_LINGO_XTRAS_A_ABSFILE_H

namespace Director {

class AbsfileXtraObject : public Object<AbsfileXtraObject> {
public:
	AbsfileXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace AbsfileXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_forget(int nargs);
void m_AbsFileRegister(int nargs);
void m_GetError(int nargs);
void m_GetErrorText(int nargs);
void m_SetIntegerFormat(int nargs);
void m_Open(int nargs);
void m_Close(int nargs);
void m_ReadByte(int nargs);
void m_ReadUByte(int nargs);
void m_ReadShort(int nargs);
void m_ReadUShort(int nargs);
void m_ReadInt(int nargs);
void m_ReadUInt(int nargs);
void m_ReadFloat(int nargs);
void m_ReadPoint(int nargs);
void m_ReadRect(int nargs);
void m_ReadBinary(int nargs);
void m_ReadString(int nargs);
void m_ReadText(int nargs);
void m_ReadSymbol(int nargs);
void m_ReadList(int nargs);
void m_BytesReaded(int nargs);
void m_SetPosition(int nargs);
void m_GetPosition(int nargs);
void m_SetMarker(int nargs);
void m_GotoMarker(int nargs);
void m_Length(int nargs);
void m_SetEndOfFile(int nargs);
void m_Write(int nargs);
void m_WriteByte(int nargs);
void m_WriteUByte(int nargs);
void m_WriteShort(int nargs);
void m_WriteUShort(int nargs);
void m_WriteInt(int nargs);
void m_WriteUInt(int nargs);
void m_WriteFloat(int nargs);
void m_WritePoint(int nargs);
void m_WriteRect(int nargs);
void m_WriteString(int nargs);
void m_WriteSymbol(int nargs);
void m_WriteList(int nargs);
void m_Flush(int nargs);
void m_OpenDialog(int nargs);
void m_SaveDialog(int nargs);
void m_FileOpenDialog(int nargs);
void m_FileSaveDialog(int nargs);
void m_FolderDialog(int nargs);
void m_ComputerDialog(int nargs);
void m_PrinterDialog(int nargs);
void m_"(int nargs);

} // End of namespace AbsfileXtra

} // End of namespace Director

#endif
