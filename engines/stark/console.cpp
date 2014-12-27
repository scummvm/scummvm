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
#include "engines/stark/xrcreader.h"

#include "common/file.h"

namespace Stark {

Console::Console(StarkEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("dumpArchive",			WRAP_METHOD(Console, Cmd_DumpArchive));
	registerCmd("dumpScript",			WRAP_METHOD(Console, Cmd_DumpScript));
	registerCmd("listRooms",			WRAP_METHOD(Console, Cmd_ListRooms));
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

bool Console::Cmd_DumpScript(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Print the scripts from an archive.\n");
		debugPrintf("Usage :\n");
		debugPrintf("dumpScript [archive name]\n");
		return true;
	}

	Resource *resource = loadXARCScripts(argv[1]);
	if (resource == nullptr) {
		debugPrintf("Can't open archive with name '%s'\n", argv[1]);
		return true;
	}

	resource->print();
	delete resource;

	return true;
}

Resource *Console::loadXARCScripts(Common::String archive) {
	XARCArchive xarc;
	if (!xarc.open(archive)) {
		return nullptr;
	}

	Common::ArchiveMemberList members;
	xarc.listMatchingMembers(members, "*.xrc");

	if (members.size() == 0) {
		error("No scripts in archive '%s'", archive.c_str());
	}

	if (members.size() > 1) {
		error("Too many scripts in archive '%s'", archive.c_str());
	}

	Common::SeekableReadStream *stream = xarc.createReadStreamForMember(members.front()->getName());

	Resource *root = XRCReader::importTree(stream);

	delete stream;

	return root;
}

bool Console::Cmd_ListRooms(int argc, const char **argv) {
	Resource *root = loadXARCScripts("x.xarc");
	if (root == nullptr) {
		debugPrintf("Can't open archive 'x.xarc'\n");
		return true;
	}

	// Loop over the levels
	for (uint i = 0; i < root->getChildren().size(); i++) {
		Resource *level = root->getChildren()[i];

		// Only consider levels
		if (!level->getType().is(ResourceType::kLevel)) continue;

		Common::String levelArchive = level->getArchive();
		debugPrintf("%s - %s\n", levelArchive.c_str(), level->getName().c_str());

		// Load the detailed level archive
		level = loadXARCScripts(levelArchive);

		if (!level)
			error("Unable to load archive '%s'", levelArchive.c_str());

		// Loop over the rooms
		for (uint j = 0; j < level->getChildren().size(); j++) {
			Resource *room = level->getChildren()[j];

			// Only consider rooms
			if (!room->getType().is(ResourceType::kLocation)) continue;

			Common::String roomArchive = room->getArchive();
			debugPrintf("%s - %s\n", roomArchive.c_str(), room->getName().c_str());
		}

		delete level;
	}

	delete root;

	return true;
}

} // End of namespace Stark
