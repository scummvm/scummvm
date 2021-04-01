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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "trecision/trecision.h"
#include "trecision/nl/sys/ffile.h"

#include "common/str.h"
#include "common/textconsole.h"
#include "common/file.h"
#include "common/substream.h"
#include "trecision/video.h"

namespace Trecision {

/* -----------------19/01/98 17.12-------------------
 * Compare
 *
 * bsearch comparison routine
 * --------------------------------------------------*/
int Compare(const void *p1, const void *p2) {
	SFileEntry *p1c = (SFileEntry *)p1, *p2c = (SFileEntry*)p2;
	return (scumm_stricmp((p1c)->name, (p2c)->name));
}

FastFile::FastFile() : Common::Archive(), _stream(nullptr) {
}

FastFile::~FastFile() {
	close();
}

const FastFile::FileEntry *FastFile::getEntry(const Common::String &name) const {
	FileEntry key;
	strncpy(key.name, name.c_str(), ARRAYSIZE(key.name));

	FileEntry *entry = (FileEntry *)bsearch(&key, &_fileEntries[0], _fileEntries.size(), sizeof(FileEntry), Compare);
	return entry;
}

bool FastFile::open(const Common::String &name) {
	close();

	_stream = SearchMan.createReadStreamForMember(name);
	if (!_stream)
		return false;

	int numFiles = _stream->readUint32LE();
	_fileEntries.resize(numFiles);
	for (int i = 0; i < numFiles; i++) {
		FileEntry *entry = &_fileEntries[i];
		_stream->read(entry->name, ARRAYSIZE(entry->name));
		entry->offset = _stream->readUint32LE();
	}

	return true;
}

void FastFile::close() {
	delete _stream;
	_stream = nullptr;
	_fileEntries.clear();
}

bool FastFile::hasFile(const Common::String &name) const {
	const FileEntry *entry = getEntry(name);
	return entry != nullptr;
}

int FastFile::listMembers(Common::ArchiveMemberList &list) const {
	list.clear();
	for (Common::Array<FileEntry>::const_iterator i = _fileEntries.begin(); i != _fileEntries.end(); i++)
		list.push_back(getMember(i->name));

	return list.size();
}

const Common::ArchiveMemberPtr FastFile::getMember(const Common::String &name) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *FastFile::createReadStreamForMember(const Common::String &name) const {
	if (!_stream)
		return nullptr;

	const FileEntry *entry = getEntry(name);
	if (entry) {
		uint32 size = (entry + 1)->offset - entry->offset;
		return new Common::SeekableSubReadStream(_stream, entry->offset, entry->offset + size);
	}

	return nullptr;
}

FastFile dataFile;

/* -----------------19/01/98 17.13-------------------
 * FastFileInit
 *
 * Initialize for fast file access. The master file and maximum number
 * of open "files" are specified.
 * --------------------------------------------------*/
char FastFileInit(const char *fname) {
	FastFileFinish();

	if (!dataFile.open(fname)) {
		warning("FastFileInit: failed to open %s", fname);
		CloseSys(g_vm->_sysText[kMessageFilesMissing]);
		return false;
	}

	return true;
}

/* -----------------19/01/98 17.14-------------------
 * FastFileFinish
 *
 * Clean up resources
 * --------------------------------------------------*/
void FastFileFinish() {
	dataFile.close();
}

/* -----------------19/01/98 17.15-------------------
 * FastFileOpen
 *
 * Search the directory for the file, and return a file handle if found.
 * --------------------------------------------------*/
Common::SeekableReadStream *FastFileOpen(const char *name) {
	if (!dataFile.isOpen()) {
		warning("FastFileOpen: not initialized");
		return nullptr;
	}
	if (name == nullptr || name[0] == 0) {
		warning("FastFileOpen: invalid name");
		return nullptr;
	}

	Common::SeekableReadStream *stream = dataFile.createReadStreamForMember(name);
	if (stream == nullptr) {
		CheckFileInCD(name);
		stream = dataFile.createReadStreamForMember(name);
	}
	if (stream == nullptr) {
		warning("FastFileOpen: File %s not found", name);
		CloseSys(g_vm->_sysText[kMessageFilesMissing]);
	}

	return stream;
}

/* -----------------19/01/98 17.16-------------------
 * FastFileClose
 *
 * Mark a fast file handle as closed
 * --------------------------------------------------*/
void FastFileClose(Common::SeekableReadStream *stream) {
	if (stream == nullptr) {
		warning("FastFileClose: invalid handle");
		return;
	}
	delete stream;
}

/* -----------------19/01/98 17.17-------------------
 * FastFileRead
 *
 * read from a fast file (memcpy!)
 * --------------------------------------------------*/
int FastFileRead(Common::SeekableReadStream *stream, void *ptr, int size) {
	if (stream == nullptr) {
		warning("FastFileRead: invalid handle");
		return 0;
	}
	if (size < 0) {
		warning("FastFileRead: invalid size");
		return 0;
	}
	if (stream->pos() + size > stream->size()) {
		warning("FastFileRead: read past end of file");
		return 0;
	}
	return stream->read(ptr, size);
}

/* -----------------19/01/98 17.17-------------------
 * FastFileLen
 *
 * Get the current length in a fast file
 * --------------------------------------------------*/
int FastFileLen(Common::SeekableReadStream *stream) {
	if (stream == nullptr) {
		warning("FastFileRead: invalid handle");
		return 0;
	}
	return stream->size();
}

// AnimFile
FastFile animFile[MAXSMACK];

/* -----------------19/01/98 17.13-------------------
 * AnimFileInit
 * --------------------------------------------------*/
bool AnimFileInit(Common::String fname) {
	AnimFileFinish();

	for (int a = 0; a < MAXSMACK; a++) {
		if (!animFile[a].open(fname)) {
			warning("AnimFileInit: failed to open file %s", fname.c_str());
			AnimFileFinish();
			CloseSys(g_vm->_sysText[kMessageFilesMissing]);
			return false;
		}
	}
	return true;
}

/* -----------------19/01/98 17.14-------------------
 * AnimFileFinish
 * --------------------------------------------------*/
void AnimFileFinish() {
	for (int a = 0; a < MAXSMACK; a++) {
		animFile[a].close();
	}
}

/* -----------------19/01/98 17.15-------------------
 * AnimFileOpen
 * --------------------------------------------------*/
Common::SeekableReadStream *AnimFileOpen(Common::String name) {
	if (!animFile[g_vm->_animMgr->_curSmackBuffer].isOpen()) {
		warning("AnimFileOpen: not initialized");
		return nullptr;
	}
	if (name.empty()) {
		warning("AnimFileOpen: invalid name");
		return nullptr;
	}

	Common::SeekableReadStream *stream = animFile[g_vm->_animMgr->_curSmackBuffer].createReadStreamForMember(name);
	if (stream == nullptr) {
		CheckFileInCD(name);
		stream = animFile[g_vm->_animMgr->_curSmackBuffer].createReadStreamForMember(name);
	}
	if (stream == nullptr) {
		warning("AnimFileOpen: File %s not found", name.c_str());
		CloseSys(g_vm->_sysText[kMessageFilesMissing]);
	}

	return stream;
}

/* -----------------19/01/98 17.15-------------------
 * FmvFileOpen
 * --------------------------------------------------*/
Common::SeekableReadStream *FmvFileOpen(const char *name) {
	if (name == nullptr || name[0] == 0) {
		warning("FmvFileOpen: invalid name");
		return nullptr;
	}

	Common::File *file = new Common::File();
	if (!file->open(name)) {
		warning("Fmv file %s not found!", name);
		delete file;
		CloseSys(g_vm->_sysText[kMessageFilesMissing]);
		return nullptr;
	}

	return file;
}

// SpeechFile
FastFile speechFile;

/* -----------------04/08/98 11.33-------------------
 * SpeechFileInit
 * --------------------------------------------------*/
bool SpeechFileInit(const char *fname) {
	SpeechFileFinish();

	if (!speechFile.open(fname)) {
		warning("SpeechFileInit: failed to open %s", fname);
		SpeechFileFinish();
		CloseSys(g_vm->_sysText[kMessageFilesMissing]);
		return false;
	}

	return true;
}

/* -----------------04/08/98 11.33-------------------
 * SpeechFileFinish
 * --------------------------------------------------*/
void SpeechFileFinish() {
	speechFile.close();
}

Common::SeekableReadStream *SpeechFileOpen(const char *name) {
	if (!speechFile.isOpen()) {
		warning("SpeechFileOpen: not initialized");
		return nullptr;
	}
	if (name == nullptr || name[0] == 0) {
		warning("SpeechFileOpen: invalid name");
		return nullptr;
	}

	Common::SeekableReadStream *stream = speechFile.createReadStreamForMember(name);
	if (stream == nullptr) {
		CheckFileInCD(name);
		stream = speechFile.createReadStreamForMember(name);
	}
	if (stream == nullptr) {
		warning("SpeechFileOpen: File %s not found", name);
		CloseSys(g_vm->_sysText[kMessageFilesMissing]);
	}

	return stream;
}

/* -----------------04/08/98 11.34-------------------
 * SpeechFileLen
 * --------------------------------------------------*/
int SpeechFileLen(const char *name) {
	Common::SeekableReadStream *stream = SpeechFileOpen(name);
	if (stream != nullptr) {
		return stream->size();
	}
	return 0;
}

/* -----------------04/08/98 11.12-------------------
 * SpeechFileRead
 * --------------------------------------------------*/
int SpeechFileRead(const char *name, unsigned char *buf) {
	Common::SeekableReadStream *stream = SpeechFileOpen(name);
	if (stream != nullptr) {
		return stream->read(buf, stream->size());
	}
	return 0;
}

} // End of namespace Trecision
