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
 * $URL$
 * $Id$
 *
 */

#include "kyra/resource.h"
#include "kyra/resource_intern.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/func.h"
#include "common/system.h"

namespace Kyra {

Resource::Resource(KyraEngine_v1 *vm) : _archiveCache(), _files(), _archiveFiles(new Common::SearchSet()), _protectedFiles(new Common::SearchSet()), _loaders(), _vm(vm) {
	initializeLoaders();

	Common::SharedPtr<Common::Archive> path(new Common::FSDirectory(ConfMan.get("path")));
	Common::SharedPtr<Common::Archive> extrapath(new Common::FSDirectory(ConfMan.get("extrapath")));
	_vm->_system->addSysArchivesToSearchSet(_files);
	_files.add("path", path, 3);
	_files.add("extrapath", extrapath, 3);
	// compressed installer archives are added at level '2',
	// but that's done in Resource::reset not here
	_files.add("protected", _protectedFiles, 1);
	_files.add("archives", _archiveFiles, 0);
}

Resource::~Resource() {
	_loaders.clear();
}

bool Resource::reset() {
	unloadAllPakFiles();

	Common::FilesystemNode dir(ConfMan.get("path"));

	if (!dir.exists() || !dir.isDirectory())
		error("invalid game path '%s'", dir.getPath().c_str());

	if (!loadPakFile(StaticResource::staticDataFilename()) || !StaticResource::checkKyraDat()) {
		Common::String errorMessage = "You're missing the '" + StaticResource::staticDataFilename() + "' file or it got corrupted, (re)get it from the ScummVM website";
		_vm->GUIErrorMessage(errorMessage);
		error(errorMessage.c_str());
	}

	if (_vm->game() == GI_KYRA1) {
		// We only need kyra.dat for the demo.
		if (_vm->gameFlags().isDemo)
			return true;

		// only VRM file we need in the *whole* game for kyra1
		if (_vm->gameFlags().isTalkie)
			loadPakFile("CHAPTER1.VRM");
	} else if (_vm->game() == GI_KYRA2) {
		if (_vm->gameFlags().useInstallerPackage)
			_files.add("installer", loadInstallerArchive("WESTWOOD", "%03d", 6), 2);

		// mouse pointer, fonts, etc. required for initializing
		if (_vm->gameFlags().isDemo && !_vm->gameFlags().isTalkie) {
			loadPakFile("GENERAL.PAK");
		} else {
			loadPakFile("INTROGEN.PAK");
			loadPakFile("OTHER.PAK");
		}

		return true;
	} else if (_vm->game() == GI_KYRA3) {
		if (_vm->gameFlags().useInstallerPackage) {
			if (!loadPakFile("WESTWOOD.001"))
				error("couldn't load file: 'WESTWOOD.001'");
		}

		if (!loadFileList("FILEDATA.FDT"))
			error("couldn't load file: 'FILEDATA.FDT'");

		return true;
	} else if (_vm->game() == GI_LOL) {
		if (_vm->gameFlags().useInstallerPackage)
			_files.add("installer", loadInstallerArchive("WESTWOOD", "%d", 0), 2);

		return true;
	}

	Common::FSList fslist;
	if (!dir.getChildren(fslist, Common::FilesystemNode::kListFilesOnly))
		error("can't list files inside game path '%s'", dir.getPath().c_str());

	if (_vm->game() == GI_KYRA1 && _vm->gameFlags().isTalkie) {
		static const char *list[] = {
			"ADL.PAK", "CHAPTER1.VRM", "COL.PAK", "FINALE.PAK", "INTRO1.PAK", "INTRO2.PAK",
			"INTRO3.PAK", "INTRO4.PAK", "MISC.PAK",	"SND.PAK", "STARTUP.PAK", "XMI.PAK",
			"CAVE.APK", "DRAGON1.APK", "DRAGON2.APK", "LAGOON.APK"
		};

		for (uint i = 0; i < ARRAYSIZE(list); ++i) {
			Common::ArchivePtr archive = loadArchive(list[i]);
			_protectedFiles->add(list[i], archive, 0);
		}
	} else {
		for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			Common::String filename = file->getName();
			filename.toUppercase();

			// No real PAK file!
			if (filename == "TWMUSIC.PAK")
				continue;

			if (filename == ((_vm->gameFlags().lang == Common::EN_ANY) ? "JMC.PAK" : "EMC.PAK"))
				continue;

			if (filename.hasSuffix(".PAK") || filename.hasSuffix(".APK")) {
				if (!loadPakFile(file->getName()))
					error("couldn't open pakfile '%s'", file->getName().c_str());
			}
		}
	}

	return true;
}

bool Resource::loadPakFile(Common::String filename) {
	filename.toUppercase();

	if (_archiveFiles->hasArchive(filename) || _protectedFiles->hasArchive(filename))
		return true;

	Common::ArchivePtr archive = loadArchive(filename);
	if (!archive)
		return false;

	_archiveFiles->add(filename, archive, 0);

	return true;
}

bool Resource::loadFileList(const Common::String &filedata) {
	Common::File f;

	if (!f.open(filedata))
		return false;

	uint32 filenameOffset = 0;
	while ((filenameOffset = f.readUint32LE()) != 0) {
		uint32 offset = f.pos();
		f.seek(filenameOffset, SEEK_SET);

		uint8 buffer[13];
		f.read(buffer, sizeof(buffer)-1);
		buffer[12] = 0;
		f.seek(offset + 16, SEEK_SET);

		Common::String filename = Common::String((char *)buffer);
		filename.toUppercase();

		if (filename.hasSuffix(".PAK")) {
			if (!exists(filename.c_str()) && _vm->gameFlags().isDemo) {
				// the demo version supplied with Kyra3 does not 
				// contain all pak files listed in filedata.fdt
				// so we don't do anything here if they are non
				// existant.
			} else if (!loadPakFile(filename)) {
				error("couldn't load file '%s'", filename.c_str());
				return false;
			}
		}
	}

	return true;
}

bool Resource::loadFileList(const char * const *filelist, uint32 numFiles) {
	if (!filelist)
		return false;

	while (numFiles--) {
		if (!loadPakFile(filelist[numFiles])) {
			error("couldn't load file '%s'", filelist[numFiles]);
			return false;
		}
	}

	return true;
}

void Resource::unloadPakFile(Common::String filename) {
	filename.toUppercase();
	_archiveFiles->remove(filename);
	_protectedFiles->remove(filename);
}

bool Resource::isInPakList(Common::String filename) {
	filename.toUppercase();
	return (_archiveFiles->hasArchive(filename) || _protectedFiles->hasArchive(filename));
}

void Resource::unloadAllPakFiles() {
	_archiveFiles->clear();
	_protectedFiles->clear();
}

uint8 *Resource::fileData(const char *file, uint32 *size) {
	Common::SeekableReadStream *stream = getFileStream(file);
	if (!stream)
		return 0;

	uint32 bufferSize = stream->size();
	uint8 *buffer = new uint8[bufferSize];
	assert(buffer);
	if (size)
		*size = bufferSize;
	stream->read(buffer, bufferSize);
	delete stream;
	return buffer;
}

bool Resource::exists(const char *file, bool errorOutOnFail) {
	if (_files.hasFile(file))
		return true;
	else if (errorOutOnFail)
		error("File '%s' can't be found", file);
	return false;
}

uint32 Resource::getFileSize(const char *file) {
	Common::SeekableReadStream *stream = getFileStream(file);
	if (!stream)
		return 0;

	uint32 size = stream->size();
	delete stream;
	return size;
}

bool Resource::loadFileToBuf(const char *file, void *buf, uint32 maxSize) {
	Common::SeekableReadStream *stream = getFileStream(file);
	if (!stream)
		return false;

	memset(buf, 0, maxSize);
	stream->read(buf, (maxSize <= stream->size()) ? maxSize : stream->size());
	delete stream;
	return true;
}

Common::SeekableReadStream *Resource::getFileStream(const Common::String &file) {
	return _files.openFile(file);
}

Common::ArchivePtr Resource::loadArchive(const Common::String &file) {
	ArchiveMap::iterator cachedArchive = _archiveCache.find(file);
	if (cachedArchive != _archiveCache.end())
		return cachedArchive->_value;

	Common::SeekableReadStream *stream = getFileStream(file);
	if (!stream)
		return Common::ArchivePtr();

	Common::ArchivePtr archive;
	for (LoaderList::const_iterator i = _loaders.begin(); i != _loaders.end(); ++i) {
		if ((*i)->checkFilename(file)) {
			if ((*i)->isLoadable(file, *stream)) {
				stream->seek(0, SEEK_SET);
				archive = Common::ArchivePtr((*i)->load(this, file, *stream));
				break;
			} else {
				stream->seek(0, SEEK_SET);
			}
		}
	}

	delete stream;

	if (!archive)
		return Common::ArchivePtr();

	_archiveCache[file] = archive;
	return archive;
}

Common::ArchivePtr Resource::loadInstallerArchive(const Common::String &file, const Common::String &ext, const uint8 offset) {
	ArchiveMap::iterator cachedArchive = _archiveCache.find(file);
	if (cachedArchive != _archiveCache.end())
		return cachedArchive->_value;

	Common::ArchivePtr archive(InstallerLoader::load(this, file, ext, offset));
	if (!archive)
		return Common::ArchivePtr();

	_archiveCache[file] = archive;
	return archive;
}

#pragma mark -

void Resource::initializeLoaders() {
	_loaders.push_back(LoaderList::value_type(new ResLoaderPak()));
	_loaders.push_back(LoaderList::value_type(new ResLoaderInsMalcolm()));
	_loaders.push_back(LoaderList::value_type(new ResLoaderTlk()));
}

} // end of namespace Kyra



