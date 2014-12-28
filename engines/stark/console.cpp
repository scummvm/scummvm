/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/console.h"
#include "engines/stark/archive.h"
#include "engines/stark/resources/resource.h"
#include "engines/stark/archiveloader.h"

#include "common/file.h"

namespace Stark {

Console::Console(StarkEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("dumpArchive",			WRAP_METHOD(Console, Cmd_DumpArchive));
	registerCmd("dumpResources",		WRAP_METHOD(Console, Cmd_DumpResources));
	registerCmd("listLocations",		WRAP_METHOD(Console, Cmd_ListLocations));
}

Console::~Console() {
}

bool Console::Cmd_DumpArchive(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Extract all the files from a game archive.\n");
		debugPrintf("The destination folder, named 'dump', must exist.\n");
		debugPrintf("Usage :\n");
		debugPrintf("dumpArchive [archive name]\n");
		return true;
	}

	XARCArchive xarc;
	if (!xarc.open(argv[1])) {
		debugPrintf("Can't open archive with name '%s'\n", argv[1]);
		return true;
	}

	Common::ArchiveMemberList members;
	xarc.listMembers(members);

	for (Common::ArchiveMemberList::const_iterator it = members.begin(); it != members.end(); it++) {
		Common::String fileName = Common::String::format("dump/%s", it->get()->getName().c_str());

		// Open the output file
		Common::DumpFile outFile;
		if (!outFile.open(fileName)) {
			debugPrintf("Unable to open file '%s' for writing\n", fileName.c_str());
			return true;
		}

		// Copy the archive content to the output file using a temporary buffer
		Common::SeekableReadStream *inStream = it->get()->createReadStream();
		uint8 *buf = new uint8[inStream->size()];

		inStream->read(buf, inStream->size());
		outFile.write(buf, inStream->size());

		delete[] buf;
		delete inStream;
		outFile.close();

		debugPrintf("Extracted '%s'\n", it->get()->getName().c_str());
	}

	return true;
}

bool Console::Cmd_DumpResources(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Print the scripts from an archive.\n");
		debugPrintf("Usage :\n");
		debugPrintf("dumpScript [archive name]\n");
		return true;
	}

	ArchiveLoader *archiveLoader = new ArchiveLoader();
	archiveLoader->load(argv[1]);

	Resource *resource = archiveLoader->useRoot(argv[1]);
	if (resource == nullptr) {
		debugPrintf("Can't open archive with name '%s'\n", argv[1]);
		return true;
	}

	resource->print();

	delete archiveLoader;

	return true;
}

bool Console::Cmd_ListLocations(int argc, const char **argv) {
	ArchiveLoader *archiveLoader = new ArchiveLoader();

	archiveLoader->load("x.xarc");
	Resource *root = archiveLoader->useRoot("x.xarc");

	// Loop over the levels
	for (uint i = 0; i < root->getChildren().size(); i++) {
		Resource *level = root->getChildren()[i];

		// Only consider levels
		if (level->getType() != ResourceType::kLevel) continue;

		Common::String levelArchive = archiveLoader->buildArchiveName((Level *) level);
		debugPrintf("%s - %s\n", levelArchive.c_str(), level->getName().c_str());

		// Load the detailed level archive
		archiveLoader->load(levelArchive);
		level = archiveLoader->useRoot(levelArchive);

		if (!level)
			error("Unable to load archive '%s'", levelArchive.c_str());

		// Loop over the locations
		for (uint j = 0; j < level->getChildren().size(); j++) {
			Resource *location = level->getChildren()[j];

			// Only consider locations
			if (location->getType() != ResourceType::kLocation) continue;

			Common::String roomArchive = archiveLoader->buildArchiveName((Level *) level, (Location *) location);
			debugPrintf("%s - %s\n", roomArchive.c_str(), location->getName().c_str());
		}

		archiveLoader->returnRoot(levelArchive);
		archiveLoader->unloadUnused();
	}

	delete archiveLoader;

	return true;
}

} // End of namespace Stark
