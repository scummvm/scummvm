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

#ifndef DIRECTOR_LINGO_XLIBS_FILEIO_H
#define DIRECTOR_LINGO_XLIBS_FILEIO_H

namespace Common {
class SeekableReadStream;
typedef SeekableReadStream InSaveFile;
class OutSaveFile;
class MemoryWriteStreamDynamic;
class String;
}

namespace Director {

enum FileIOError {
	kErrorNone = 0,
	kErrorMemAlloc = 1,
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
	Common::SeekableReadStream *_inStream;
	Common::OutSaveFile *_outFile;
	Common::MemoryWriteStreamDynamic *_outStream;
	FileIOError _lastError;

public:
	FileObject(ObjectType objType);
	FileObject(const FileObject &obj);
	~FileObject() override;

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;

	FileIOError open(const Common::String &origpath, const Common::String &mode);
	void clear();
	FileIOError saveFileError();
	void dispose() override;
};

namespace FileIO {
	extern const char *const xlibName;
	extern const XlibFileDesc fileNames[];

	void open(ObjectType type, const Common::Path &path);
	void close(ObjectType type);

	bool charInMatchString(char ch, const Common::String &matchString);
	void m_delete(int nargs);
	void m_dispose(int nargs);
	void m_error(int nargs);
	void m_fileName(int nargs);
	void m_getFinderInfo(int nargs);
	void m_getLength(int nargs);
	void m_getPosition(int nargs);
	void m_new(int nargs);
	void m_readChar(int nargs);
	void m_readFile(int nargs);
	void m_readLine(int nargs);
	void m_readPict(int nargs);
	void m_readToken(int nargs);
	void m_readWord(int nargs);
	void m_setFinderInfo(int nargs);
	void m_setPosition(int nargs);
	void m_status(int nargs);
	void m_writeChar(int nargs);
	void m_writeString(int nards);

	void m_setOverrideDrive(int nargs);

	void m_closeFile(int nargs);
	void m_createFile(int nargs);
	void m_displayOpen(int nargs);
	void m_displaySave(int nargs);
	void m_openFile(int nargs);
	void m_setFilterMask(int nargs);
	void m_getOSDirectory(int nargs);

} // End of namespace FileIO

} // End of namespace Director

#endif
