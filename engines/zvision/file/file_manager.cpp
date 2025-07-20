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

const char* genExcluded[] {"*.dll", "*.ini", "*.exe", "*.isu", "*.inf", "*path*.txt", "r.svr", "*.zix"};
//TODO - change this to a list of alternate pairs; only throw an error if both alternatives are unavailable
const char* zgiExcluded[] {
	"c000h01q.raw", "cm00h01q.raw", "dm00h01q.raw", "e000h01q.raw", "em00h11p.raw", "em00h50q.raw", "gjnph65p.raw", 
	"gjnph72p.raw", "h000h01q.raw", "m000h01q.raw", "p000h01q.raw", "q000h01q.raw", "sw00h01q.raw", "t000h01q.raw", 
	"u000h01q.raw"
	};
	
FileManager::FileManager(ZVision *engine) : 
	_engine(engine) {}

Common::File *FileManager::open(const Common::Path &filePath, bool allowSrc) {
	Common::File *file = new Common::File();
	Common::String fileName = filePath.baseName();
	bool found = SearchMan.hasFile(filePath);
	bool foundAlt = false;
	
	if (allowSrc) {
		Common::String altName = fileName;
		altName.setChar('s', altName.size() - 3);
		altName.setChar('r', altName.size() - 2);
		altName.setChar('c', altName.size() - 1);
		Common::Path altPath = filePath.getParent().appendComponent(altName);
		foundAlt = SearchMan.hasFile(altPath);
		if (!foundAlt) {
			if (!found)
				warning("Unable to find file %s or alternate file %s", fileName.c_str(), altName.c_str());
		}
		else {
			Common::File *altFile = new Common::File();
			if (!altFile->open(altPath))
				warning("File %s not found; alternate file %s found but unable to open", fileName.c_str(), altName.c_str());
			else {
				if (!file->open(filePath)) {
					delete file;
					debugC(2, kDebugFile,"Opened alternative file %s", altName.c_str());
					return altFile;
				}
				else if (file->size() < altFile->size()) {
					file->close();
					delete file;
					debugC(2, kDebugFile,"Opened alternative file %s", altName.c_str());
					return altFile;
				}
				else
					altFile->close();
			}
			delete altFile;
		}
	}
	
	if (!found) {
		if(!allowSrc)
			warning("File %s not found", fileName.c_str());
	}
	else if (!file->open(filePath))
		warning("File %s found, but unable to open", fileName.c_str());
	else {
		debugC(2, kDebugFile,"Opened file %s", fileName.c_str());
		return file;
	}
	
	delete file;
	return nullptr;
}

bool FileManager::loadZix(const Common::Path &name) {
	Common::File file;
	if (!file.open(name))
		return false;

	Common::String line;

	// Skip first block
	while (!file.eos()) {
		line = file.readLine();
		if (line.matchString("----------*", true))
			break;
	}
	
	if (file.eos())
		error("Corrupt ZIX file: %s", name.toString(Common::Path::kNativeSeparator).c_str());

	uint8 archives = 0;

	// Parse subdirectories & archives
	debugC(1, kDebugFile, "Parsing list of subdirectories & archives in %s", name.toString(Common::Path::kNativeSeparator).c_str());
	while (!file.eos()) {
		line = file.readLine();
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

			if (line.matchString("*.zfs", true)) {
				debugC(1, kDebugFile, "Adding archive %s to search manager.", path.toString().c_str());
				Common::Archive *arc;
				arc = new ZfsArchive(path);
				SearchMan.add(line, arc);
			}
			archives++;
		}
	}

	if (file.eos())
		error("Corrupt ZIX file: %s", name.toString(Common::Path::kNativeSeparator).c_str());

	//Parse files
	debugC(1, kDebugFile, "Parsing list of individual resource files in %s", name.toString(Common::Path::kNativeSeparator).c_str());
	while (!file.eos()) {
		line = file.readLine();
		line.trim();
		uint dr = 0;
		char buf[32];
		if (sscanf(line.c_str(), "%u %s", &dr, buf) == 2) {
			if (dr <= archives && dr > 0) {
				Common::String path(buf);
				bool exclude = false;
				for(auto filename : genExcluded)
					if(path.matchString(filename, true)) {
						exclude = true;
						break;
					}
				for(auto filename : zgiExcluded)
					if(path.matchString(filename, true)) {
						//exclude = true;
						break;
					}
				if(!exclude) {
					// No need to add file, just verify that it exists
					Common::File resource;
					if(!resource.exists(buf))
						warning("Missing file %s", path.c_str());
					else
						debugC(5, kDebugFile, "File found: %s", path.c_str());
					if(path.matchString("*.zfs", true)) {
						Common::Path path_(path);
						debugC(kDebugFile, "Adding archive %s to search manager.", path.c_str());
						Common::Archive *arc;
						arc = new ZfsArchive(path_);
						SearchMan.add(path, arc);
					}
				}
			}
		}
	}
	return true;
}

}	// End of namespace Zvision
