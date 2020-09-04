/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DIRECTOR_LINGO_XOBJECT_FILEIO_H
#define DIRECTOR_LINGO_XOBJECT_FILEIO_H

namespace Common {
class SeekableReadStream;
typedef SeekableReadStream InSaveFile;
class OutSaveFile;
class MemoryWriteStreamDynamic;
}

namespace Director {

enum FileIOError {
	kErrorNone = 0,
	kErrorEOF = -1,
	kErrorDirectoryFull = -33,
	kErrorVolumeFull = -34,
	kErrorVolumeNotFound = -35,
	kErrorIO = -36,
	kErrorBadFileName = -37,
	kErrorFileNotOpen = -38,
	kErrorInvalidPos = -39, // undocumented
	kErrorTooManyFilesOpen = -42,
	kErrorFileNotFound = -43,
	kErrorNoSuchDrive = -56,
	kErrorReadOnly = -61, // undocumented
	kErrorNoDiskInDrive = -65,
	kErrorDirectoryNotFound = -120
};

class FileObject : public Object<FileObject> {
public:
	Common::String *_filename;
	Common::InSaveFile *_inFile;
	Common::SeekableReadStream *_inStream;
	Common::OutSaveFile *_outFile;
	Common::MemoryWriteStreamDynamic *_outStream;

public:
	FileObject(ObjectType objType);
	FileObject(const FileObject &obj);
	~FileObject() override;

	void clear();
	void dispose() override;
};

namespace FileIO {
	bool charInMatchString(char ch, const Common::String &matchString);
	void initialize(int type);
	void saveFileError();
	void m_delete(int nargs);
	void m_dispose(int nargs);
	void m_fileName(int nargs);
	void m_getLength(int nargs);
	void m_getPosition(int nargs);
	void m_new(int nargs);
	void m_readChar(int nargs);
	void m_readLine(int nargs);
	void m_readToken(int nargs);
	void m_readWord(int nargs);
	void m_setPosition(int nargs);
	void m_writeChar(int nargs);
	void m_writeString(int nards);

} // End of namespace FileIO

} // End of namespace Director

#endif
