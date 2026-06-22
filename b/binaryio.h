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

#ifndef DIRECTOR_LINGO_XTRAS_BINARYIO_H
#define DIRECTOR_LINGO_XTRAS_BINARYIO_H

namespace Director {

class BinaryioXtraObject : public Object<BinaryioXtraObject> {
public:
	BinaryioXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BinaryioXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_openFile(int nargs);
void m_getFileSize(int nargs);
void m_setFileSize(int nargs);
void m_getFilePosition(int nargs);
void m_setFilePosition(int nargs);
void m_readBytes(int nargs);
void m_writeBytes(int nargs);
void m_relax64kLimit(int nargs);
void m_enforce64kLimit(int nargs);
void m_writeChar(int nargs);
void m_readSignedChar(int nargs);
void m_readUnsignedChar(int nargs);
void m_usePlatformByteOrder(int nargs);
void m_useMacByteOrder(int nargs);
void m_useWinByteOrder(int nargs);
void m_writeShort(int nargs);
void m_readSignedShort(int nargs);
void m_readUnsignedShort(int nargs);
void m_writeSignedLong(int nargs);
void m_writeUnsignedLong(int nargs);
void m_readSignedLong(int nargs);
void m_readUnsignedLong(int nargs);
void m_writeFloat(int nargs);
void m_readFloat(int nargs);
void m_writeFloat32(int nargs);
void m_readFloat32(int nargs);
void m_closeFile(int nargs);
void m_bnioGetFileCreationTime(int nargs);
void m_bnioSetFileCreationTime(int nargs);
void m_bnioGetFileModificationTime(int nargs);
void m_bnioSetFileModificationTime(int nargs);
void m_bnioDeleteFile(int nargs);
void m_register(int nargs);

} // End of namespace BinaryioXtra

} // End of namespace Director

#endif
