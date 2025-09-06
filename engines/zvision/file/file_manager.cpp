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

#include "common/debug.h"
#include "zvision/file/file_manager.h"
#include "zvision/file/zfs_archive.h"

namespace ZVision {

const char* genExcluded[] {"*.dll", "*.ini", "*.exe", "*.isu", "*.inf", "*path*.txt", "r.svr", "*.zix", "*.hlp", "*.gid"};

const char* zgiAlternates[] {
	"c000h01q.raw", "cm00h01q.raw", "dm00h01q.raw", "e000h01q.raw", "em00h11p.raw", "em00h50q.raw", "gjnph65p.raw",
	"gjnph72p.raw", "h000h01q.raw", "m000h01q.raw", "p000h01q.raw", "q000h01q.raw", "sw00h01q.raw", "t000h01q.raw",
	"u000h01q.raw"
	};

FileManager::FileManager(ZVision *engine) {
}

Common::File *FileManager::open(const Common::Path &filePath, bool allowSrc) {
	debugC(5, kDebugFile, "FileManager::open()");
	Common::File *file = new Common::File();
	Common::File *out = nullptr;

	Common::String fileName = filePath.baseName();
	bool open = false;
	bool altFound = false;
	bool altOpen = false;

	bool found = SearchMan.hasFile(filePath);
	if(found) {
		debugC(5, kDebugFile,"File %s found", fileName.c_str());
		open = file->open(filePath);
		if(open)
			debugC(5, kDebugFile,"File %s opened", fileName.c_str());
	}

	if (allowSrc) {
		Common::File *altFile = new Common::File();
		Common::String altName = fileName;
		altName.setChar('s', altName.size() - 3);
		altName.setChar('r', altName.size() - 2);
		altName.setChar('c', altName.size() - 1);
		Common::Path altPath = filePath.getParent().appendComponent(altName);
		altFound = SearchMan.hasFile(altPath);
		if (altFound) {
			debugC(5, kDebugFile,"Alternate file %s found", altName.c_str());
			altOpen = altFile->open(altPath);
			if (altOpen)
				debugC(5, kDebugFile,"Alternate file %s opened", altName.c_str());
		}

		if(altOpen) {
			if(open)
				out = file->size() < altFile->size() ? altFile : file;
			else
				out = altFile;
		}
		else if(open)
			out = file;
		else {
			if (found && altFound)
				warning("Found file %s and alternate file %s but unable to open either", fileName.c_str(), altName.c_str());
			else if (found)
				warning("Found file %s but unable to open; alternate file %s not found", fileName.c_str(), altName.c_str());
			else if (altFound)
				warning("File %s not found; alternate file %s found but but unable to open", fileName.c_str(), altName.c_str());
			else
				warning("Unable to find file %s or alternate file %s", fileName.c_str(), altName.c_str());
		}

		if (out == altFile)
			debugC(5, kDebugFile,"Returning alternate file %s", altName.c_str());
		else {
			if(altOpen)
				altFile->close();
			delete altFile;
		}
	}
	else {
		if(open)
			out = file;
		else if (found)
			warning("File %s found, but unable to open", fileName.c_str());
		else
			warning("File %s not found", fileName.c_str());
	}

	if (out == file)
		debugC(5, kDebugFile,"Returning file %s", fileName.c_str());
	else {
		if(open)
			file->close();
		delete file;
	}

	return out;
}

bool FileManager::exists(Common::Path filePath, bool allowSrc) {
	Common::File file;
	if (file.exists(filePath))
		return true;
	else if (allowSrc) {
		if (file.exists(srcPath(filePath)))
			return true;
	}
	return false;
}

Common::Path FileManager::srcPath(Common::Path filePath) {
		Common::String name = filePath.baseName();
		name.setChar('s', name.size() - 3);
		name.setChar('r', name.size() - 2);
		name.setChar('c', name.size() - 1);
		return filePath.getParent().appendComponent(name);
}

bool FileManager::loadZix(const Common::Path &zixPath) {
	Common::File zixFile;
	if (!zixFile.open(zixPath))
		return false;

	Common::String line;

	// Skip first block
	while (!zixFile.eos()) {
		line = zixFile.readLine();
		if (line.matchString("----------*", true))
			break;
	}

	if (zixFile.eos())
		error("Corrupt ZIX file: %s", zixPath.toString(Common::Path::kNativeSeparator).c_str());

	uint8 archives = 0;

	// Parse subdirectories & archives
	debugC(1, kDebugFile, "Parsing list of subdirectories & archives in %s", zixPath.toString(Common::Path::kNativeSeparator).c_str());
	while (!zixFile.eos()) {
		line = zixFile.readLine();
		line.trim();
		if (line.matchString("----------*", true))
			break;
		else if (line.matchString("DIR:*", true) || line.matchString("CD0:*", true) || line.matchString("CD1:*", true) || line.matchString("CD2:*", true)) {
			line = Common::String(line.c_str() + 5);
			for (uint i = 0; i < line.size(); i++)
				if (line[i] == '\\')
					line.setChar('/', i);

			// Check if NEMESIS.ZIX/MEDIUM.ZIX refers to the znemesis folder, and
			// check the game root folder instead
			if (line.hasPrefix("znemesis/"))
				line = Common::String(line.c_str() + 9);

			// Check if INQUIS.ZIX refers to the ZGI folder, and check the game
			// root folder instead
			if (line.hasPrefix("zgi/"))
				line = Common::String(line.c_str() + 4);
			if (line.hasPrefix("zgi_e/"))
				line = Common::String(line.c_str() + 6);

			if (line.size() && line[0] == '.')
				line.deleteChar(0);
			if (line.size() && line[0] == '/')
				line.deleteChar(0);
			if (line.size() && line.hasSuffix("/"))
				line.deleteLastChar();

			Common::Path path(line, '/');
			path = path.getLastComponent();	//We are using the search manager in "flat" mode, so only filenames are needed

			if (line.matchString("*.zfs", true))
				if (!SearchMan.hasArchive(line)) {
				debugC(1, kDebugFile, "Adding archive %s to search manager.", path.toString().c_str());
				Common::Archive *arc;
				arc = new ZfsArchive(path);
				SearchMan.add(line, arc);
			}
			archives++;
		}
	}

	if (zixFile.eos())
		error("Corrupt ZIX file: %s", zixPath.toString(Common::Path::kNativeSeparator).c_str());

	//Parse files
	debugC(1, kDebugFile, "Parsing list of individual resource files in %s", zixPath.toString(Common::Path::kNativeSeparator).c_str());
	while (!zixFile.eos()) {
		line = zixFile.readLine();
		line.trim();
		uint dr = 0;
		char buf[32];
		if (sscanf(line.c_str(), "%u %s", &dr, buf) == 2) {
			if (dr <= archives && dr > 0) {
				Common::String name(buf);
				bool exclude = false;
				bool allowSrc = false;
				for (auto excName : genExcluded)
					if(name.matchString(excName, true)) {
						exclude = true;
						break;
					}
				for (auto altName : zgiAlternates)
					if(name.matchString(altName, true)) {
						allowSrc = true;
						break;
					}
				if (!exclude) {
					Common::Path path(name);
					// No need to add file, just verify that it exists
					if (allowSrc) {
						Common::Path altPath = srcPath(path);
						if (!SearchMan.hasFile(path) && !SearchMan.hasFile(altPath))
							warning("Missing files %s and/or %s", path.toString().c_str(), altPath.toString().c_str());
						else if (SearchMan.hasFile(path))
							debugC(5, kDebugFile, "File found: %s", path.toString().c_str());
						else
							debugC(5, kDebugFile, "Alternate file found: %s", altPath.toString().c_str());
					}
					else {
						if (!SearchMan.hasFile(path))
							warning("Missing file %s", path.toString().c_str());
						else
							debugC(5, kDebugFile, "File found: %s", path.toString().c_str());
					}
					if (name.matchString("*.zfs", true))
						if (!SearchMan.hasArchive(name)) {
							Common::Path path_(path);
							debugC(kDebugFile, "Adding archive %s to search manager.", path.toString().c_str());
							Common::Archive *arc;
							arc = new ZfsArchive(path_);
							SearchMan.add(name, arc);
						}
				}
			}
		}
	}
	return true;
}

}	// End of namespace Zvision
