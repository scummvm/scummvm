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

#ifndef DIRECTOR_LINGO_XTRAS_B_BUDFILE_H
#define DIRECTOR_LINGO_XTRAS_B_BUDFILE_H

namespace Director {

class BudfileXtraObject : public Object<BudfileXtraObject> {
public:
	BudfileXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BudfileXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_baFileResult(int nargs);
void m_baReadFile(int nargs);
void m_baWriteFile(int nargs);
void m_baAppendFile(int nargs);
void m_baInsertFile(int nargs);
void m_baReadBinFile(int nargs);
void m_baWriteBinFile(int nargs);
void m_baAppendBinFile(int nargs);
void m_baInsertBinFile(int nargs);
void m_baReadList(int nargs);
void m_baWriteList(int nargs);
void m_baEncryptBF(int nargs);
void m_baDecryptBF(int nargs);
void m_baReadFileBF(int nargs);
void m_baWriteFileBF(int nargs);
void m_baIsEncryptedBF(int nargs);
void m_baGetFile(int nargs);
void m_baMovePointer(int nargs);
void m_baGetPointer(int nargs);
void m_baSeekTo(int nargs);
void m_baSeekAfter(int nargs);
void m_baClearFile(int nargs);
void m_baEndFile(int nargs);
void m_baCloseFile(int nargs);
void m_baReadText(int nargs);
void m_baWriteText(int nargs);
void m_baInsertText(int nargs);
void m_baReturnStyle(int nargs);
void m_baReadLine(int nargs);
void m_baWriteLine(int nargs);
void m_baInsertLine(int nargs);
void m_baReadChunk(int nargs);
void m_baReadBin(int nargs);
void m_baWriteBin(int nargs);
void m_baInsertBin(int nargs);
void m_baByteOrder(int nargs);
void m_baReadByte(int nargs);
void m_baReadUByte(int nargs);
void m_baWriteByte(int nargs);
void m_baWriteUByte(int nargs);
void m_baInsertByte(int nargs);
void m_baInsertUByte(int nargs);
void m_baReadShort(int nargs);
void m_baReadUShort(int nargs);
void m_baWriteShort(int nargs);
void m_baWriteUShort(int nargs);
void m_baInsertShort(int nargs);
void m_baInsertUShort(int nargs);
void m_baReadLong(int nargs);
void m_baReadULong(int nargs);
void m_baWriteLong(int nargs);
void m_baWriteULong(int nargs);
void m_baInsertLong(int nargs);
void m_baInsertULong(int nargs);
void m_"(int nargs);

} // End of namespace BudfileXtra

} // End of namespace Director

#endif
