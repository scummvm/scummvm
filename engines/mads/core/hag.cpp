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

#include "common/file.h"
#include "mads/core/hag.h"
#include "mads/detection.h"

namespace MADS {

const char *const MADSCONCAT_STRING = "MADSCONCAT";

HagArchive::HagArchive(int gameID, bool isDemo) {
	loadIndex(gameID, isDemo);
}

HagArchive::~HagArchive() {
}

// Archive implementation
bool HagArchive::hasFile(const Common::Path &path) const {
	HagIndex hagIndex;
	HagEntry hagEntry;
	return getHeaderEntry(path, hagIndex, hagEntry);
}

int HagArchive::listMembers(Common::ArchiveMemberList &list) const {
	int members = 0;

	for (uint hagCtr = 0; hagCtr < _index.size(); ++hagCtr) {
		HagIndex hagIndex = _index[hagCtr];
		Common::List<HagEntry>::iterator i;

		for (i = hagIndex._entries.begin(); i != hagIndex._entries.end(); ++i) {
			list.push_back(Common::ArchiveMemberList::value_type(
				new Common::GenericArchiveMember((*i)._resourceName, *this)));
			++members;
		}
	}

	return members;
}

const Common::ArchiveMemberPtr HagArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *HagArchive::createReadStreamForMember(const Common::Path &path) const {
	HagIndex hagIndex;
	HagEntry hagEntry;

	if (getHeaderEntry(path, hagIndex, hagEntry)) {
		// Entry found. If the correct file is not already open, open it
		Common::File f;
		if (!f.open(hagIndex._filename))
			error("Could not open HAG file");

		// Return a new stream for the specific resource
		f.seek(hagEntry._offset);
		return f.readStream(hagEntry._size);
	}

	return nullptr;
}

void HagArchive::loadIndex(int gameID, bool isDemo) {
	Common::File hagFile;

	for (int sectionIndex = -1; sectionIndex < 11; ++sectionIndex) {
 		if (sectionIndex == 0 && !Common::File::exists("SECTION0.HAG"))
			continue;

		// Rex Nebular and Dragonsphere demos only have sections 1 and 9 - skip the rest
		if ((gameID == GType_RexNebular || gameID == GType_Dragonsphere) && isDemo)  {
			if (sectionIndex != 1 && sectionIndex != 9)
				continue;
		}

		// Phantom demo only has sections 1, 2 and 9 - skip the rest
		if (gameID == GType_Phantom && isDemo)  {
			if (sectionIndex != 1 && sectionIndex != 2 && sectionIndex != 9)
				continue;
		}

		// Dragonsphere does not have some sections - skip them
		if (gameID == GType_Dragonsphere)  {
			if (sectionIndex == 7 || sectionIndex == 8)
				continue;
		}

		// Phantom and Forest don't have some sections - skip them
		if (gameID == GType_Phantom || gameID == GType_Forest)  {
			if (sectionIndex == 6 || sectionIndex == 7 || sectionIndex == 8)
				continue;
		}

		Common::Path filename = (sectionIndex == -1) ? Common::Path("GLOBAL.HAG") :
			Common::Path(Common::String::format("SECTION%d.HAG", sectionIndex));
		if (sectionIndex == 10) {
			// Speech
			if (!Common::File::exists("SPEECH.HAG"))
				break;
			else
				filename = "SPEECH.HAG";
		}
		if (!hagFile.open(filename))
			error("Could not locate HAG file - %s", filename.toString().c_str());

		// Check for header
		char headerBuffer[16];
		if ((hagFile.read(headerBuffer, 16) != 16) ||
				(strncmp(headerBuffer, MADSCONCAT_STRING, 10) != 0))
			error("Invalid HAG file opened");

		// Scan through the HAG index
		int numEntries = hagFile.readUint16LE();

		HagIndex hagIndex;
		hagIndex._filename = filename;

		for (int idx = 0; idx < numEntries; ++idx) {
			// Read in the details of the next resource
			char resourceBuffer[14];
			uint32 offset = hagFile.readUint32LE();
			uint32 size = hagFile.readUint32LE();
			hagFile.read(resourceBuffer, 14);

			hagIndex._entries.push_back(HagEntry(resourceBuffer, offset, size));
		}

		hagFile.close();
		_index.push_back(hagIndex);
	}
}

bool HagArchive::getHeaderEntry(const Common::Path &resourceName,
		HagIndex &hagIndex, HagEntry &hagEntry) const {
	Common::Path resName = resourceName;
	resName.toUppercase();
	Common::String baseName(resName.baseName());
	if (baseName[0] == '*') {
		baseName.deleteChar(0);
		resName = resName.getParent().appendComponent(baseName);
	}

	Common::Path hagFilename = getResourceFilename(resName);

	// Find the index for the given file
	for (uint hagCtr = 0; hagCtr < _index.size(); ++hagCtr) {
		hagIndex = _index[hagCtr];

		if (hagIndex._filename == hagFilename) {
			Common::List<HagEntry>::iterator ei;
			for (ei = hagIndex._entries.begin(); ei != hagIndex._entries.end(); ++ei) {
				hagEntry = *ei;
				if (hagEntry._resourceName.equalsIgnoreCase(resName))
					return true;
			}
		}
	}

	return false;
}

Common::Path HagArchive::getResourceFilename(const Common::Path &resourceName) const {
	Common::String baseName(resourceName.baseName());
	ResourceType resType = getResourceType(baseName);
	Common::Path outputFilename = "GLOBAL.HAG";

	if ((resType == RESTYPE_ROOM) || (resType == RESTYPE_SC)) {
		int value = atoi(baseName.c_str() + 2);
		int hagFileNum = (resType == RESTYPE_ROOM) ? value / 100 : value;

		if (hagFileNum >= 0)
			outputFilename = Common::Path(Common::String::format("SECTION%d.HAG", hagFileNum));
	}

	if (resType == RESTYPE_SPEECH)
		outputFilename = "SPEECH.HAG";

	return outputFilename;
}

ResourceType HagArchive::getResourceType(const Common::String &resourceName) const {
	if (resourceName.hasPrefix("RM")) {
		// Room resource
		return RESTYPE_ROOM;
	} else if (resourceName.hasPrefix("SC")) {
		// SC resource
		return RESTYPE_SC;
	} else if (resourceName.hasSuffix(".TXT")) {
		// Text resource
		return RESTYPE_TEXT;
	} else if (resourceName.hasSuffix(".QUO")) {
		// QUO resource
		return RESTYPE_QUO;
	} else if (resourceName.hasPrefix("I")) {
		// I resource
		return RESTYPE_I;
	} else if (resourceName.hasPrefix("OB")) {
		// OB resource
		return RESTYPE_OB;
	} else if (resourceName.hasPrefix("FONT")) {
		// FONT resource
		return RESTYPE_FONT;
	} else if (resourceName.hasPrefix("SOUND")) {
		// SOUND resource
		return RESTYPE_SOUND;
	} else if (resourceName.hasPrefix("SPCHC")) {
		// SPEECH resource
		return RESTYPE_SPEECH;
	}

	// Check for a known extension
	const char *extPos = strchr(resourceName.c_str(), '.');
	if (extPos) {
		++extPos;
		if (!strcmp(extPos, "FL") || !strcmp(extPos, "LBM") || !strcmp(extPos, "ANM") ||
			!strcmp(extPos, "AA") || !strcmp(extPos, "SS")) {
			return RESTYPE_HAS_EXT;
		}
	}

	return RESTYPE_NO_EXT;
}

} // namespace MADS
