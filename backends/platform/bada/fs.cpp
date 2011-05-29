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
 */

#include "config.h"
#include "system.h"
#include "fs.h"

//
// BadaFileStream
//
class BadaFileStream : public Common::SeekableReadStream,
                       public Common::WriteStream,
                       public Common::NonCopyable {
public:
  static BadaFileStream* makeFromPath(const String &path, bool writeMode);

  BadaFileStream(File* file);
  ~BadaFileStream();

  bool err() const;
  void clearErr();
  bool eos() const;

  uint32 write(const void *dataPtr, uint32 dataSize);
  bool flush();

  int32 pos() const;
  int32 size() const;
  bool seek(int32 offs, int whence = SEEK_SET);
  uint32 read(void *dataPtr, uint32 dataSize);

private:
  File* file;
};

BadaFileStream::BadaFileStream(File* ioFile) : file(ioFile) {
  AppAssert(ioFile != 0);
}

BadaFileStream::~BadaFileStream() {
  delete file;
}

bool BadaFileStream::err() const {
  return (GetLastResult() != E_SUCCESS);
}

void BadaFileStream::clearErr() {
  SetLastResult(E_SUCCESS);
}

bool BadaFileStream::eos() const {
  return (GetLastResult() == E_END_OF_FILE);
}

int32 BadaFileStream::pos() const {
  return file->Tell();
}

int32 BadaFileStream::size() const {
  int32 oldPos = pos();
  file->Seek(FILESEEKPOSITION_END, 0);

  int32 length = pos();
  file->Seek(FILESEEKPOSITION_BEGIN, oldPos);

  return length;
}

bool BadaFileStream::seek(int32 offs, int whence) {
  bool result = false;
  switch (whence) {
  case SEEK_SET:
    // set from start of file
    result = (E_SUCCESS == file->Seek(FILESEEKPOSITION_BEGIN, offs));
    break;
  case SEEK_CUR:
    // set relative to offs
    result = (E_SUCCESS == file->Seek(FILESEEKPOSITION_CURRENT, offs));
    break;
  case SEEK_END:
    // set relative to end - positive will increase the file size
    result = (E_SUCCESS == file->Seek(FILESEEKPOSITION_END, offs));
    break;
  }
  return result;
}

uint32 BadaFileStream::read(void *ptr, uint32 len) {
  return file->Read(ptr, len);
}

uint32 BadaFileStream::write(const void *ptr, uint32 len) {
  return (E_SUCCESS == file->Write(ptr, len));
}

bool BadaFileStream::flush() {
  return (E_SUCCESS == file->Flush());
}

BadaFileStream *BadaFileStream::makeFromPath(const String &path, bool writeMode) {
  File* ioFile = new File();

  result r = ioFile->Construct(path, writeMode ? L"wb" : L"rb", false);
  if (r == E_SUCCESS) {
    return new BadaFileStream(ioFile);
  }
  
  AppLog("Failed to open file");
  delete ioFile;
  return 0;
}

//
// converts a bada (wchar) String into a scummVM (char) string
//
Common::String fromString(const Osp::Base::String& in) {
  ByteBuffer* buf = StringUtil::StringToUtf8N(in);
  Common::String result((const char*) buf->GetPointer());
  delete buf;
  
  return result;
}

//
// BadaFilesystemNode
//
BadaFilesystemNode::BadaFilesystemNode(const Common::String& nodePath) {
  AppAssert(nodePath.size() > 0);
  init(nodePath);
}

BadaFilesystemNode::BadaFilesystemNode(const Common::String& root,
                                       const Common::String& nodePath) {
  // Make sure the string contains no slashes
  AppAssert(!nodePath.contains('/'));

  // We assume here that path is already normalized (hence don't bother to
  // call Common::normalizePath on the final path).
  Common::String newPath(root);
  if (root.lastChar() != '/') {
    newPath += '/';
  }
  newPath += nodePath;

  init(newPath);
}

void BadaFilesystemNode::init(const Common::String& nodePath) {
  // Normalize the path (that is, remove unneeded slashes etc.)
  path = Common::normalizePath(nodePath, '/');
  displayName = Common::lastPathComponent(path, '/');

  StringUtil::Utf8ToString(path.c_str(), unicodePath);
  isValid = !IsFailed(File::GetAttributes(unicodePath, attr));
}

bool BadaFilesystemNode::exists() const {
  return isValid;
}

bool BadaFilesystemNode::isReadable() const {
  return (isValid && !attr.IsHidden());
}

bool BadaFilesystemNode::isDirectory() const {
  return (isValid && attr.IsDirectory());
}

bool BadaFilesystemNode::isWritable() const {
  return (isValid && !attr.IsDirectory() && !attr.IsReadOnly());
}

AbstractFSNode* BadaFilesystemNode::getChild(const Common::String &n) const {
  AppAssert(!path.empty());
  AppAssert(isDirectory());
  return new BadaFilesystemNode(path, n);
}

bool BadaFilesystemNode::getChildren(AbstractFSList &myList,
                                     ListMode mode, bool hidden) const {
  AppAssert(isDirectory());

  bool result = false;
  DirEnumerator* pDirEnum = 0;
  Directory* pDir = new Directory();

  // open directory
  if (IsFailed(pDir->Construct(unicodePath))) {
    AppLog("Failed to open directory");
  }
  else {
    // read all directory entries
    pDirEnum = pDir->ReadN();
    if (pDirEnum) {
      result = true;
    }

    // loop through all directory entries
    while (pDirEnum && pDirEnum->MoveNext() == E_SUCCESS) {
      DirEntry dirEntry = pDirEnum->GetCurrentDirEntry();

      // skip 'invisible' files if necessary
      Osp::Base::String fileName = dirEntry.GetName();

      if (fileName[0] == '.' && !hidden) {
        continue;
      }

      // skip '.' and '..' to avoid cycles
      if ((fileName[0] == '.' && fileName[1] == 0) ||
          (fileName[0] == '.' && fileName[1] == '.')) {
        continue;
      }

      // Honor the chosen mode
      if ((mode == Common::FSNode::kListFilesOnly && dirEntry.IsDirectory()) ||
          (mode == Common::FSNode::kListDirectoriesOnly && !dirEntry.IsDirectory())) {
        continue;
      }

      myList.push_back(new BadaFilesystemNode(path, fromString(fileName)));
    }
  }

  // cleanup
  if (pDirEnum) {
    delete pDirEnum;
  }

  // close the opened directory
  if (pDir) {
    delete pDir;
  }

  return result;
}

AbstractFSNode* BadaFilesystemNode::getParent() const {
  logEntered();
  if (path == "/") {
    return 0; // The filesystem root has no parent
  }

  const char *start = path.c_str();
  const char *end = start + path.size();

  // Strip of the last component. We make use of the fact that at this
  // point, path is guaranteed to be normalized
  while (end > start && *(end-1) != '/') {
    end--;
  }

  if (end == start) {
    // This only happens if we were called with a relative path, for which
    // there simply is no parent.
    // TODO: We could also resolve this by assuming that the parent is the
    //       current working directory, and returning a node referring to that.
    return 0;
  }

  return new BadaFilesystemNode(Common::String(start, end));
}

Common::SeekableReadStream* BadaFilesystemNode::createReadStream() {
  Common::SeekableReadStream* result = BadaFileStream::makeFromPath(unicodePath, false);
  if (result != null) {
    isValid = !IsFailed(File::GetAttributes(unicodePath, attr));
  }
  return result;
}

Common::WriteStream* BadaFilesystemNode::createWriteStream() {
  Common::WriteStream* result = BadaFileStream::makeFromPath(unicodePath, true);
  if (result != null) {
    isValid = !IsFailed(File::GetAttributes(unicodePath, attr));
  }
  return result;
}

//
// end of fs.cpp
//
