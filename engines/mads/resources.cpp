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

#include "common/scummsys.h"
#include "common/archive.h"
#include "common/textconsole.h"
#include "mads/mads.h"
#include "mads/resources.h"

namespace MADS {

enum ResourceType {RESTYPE_ROOM, RESTYPE_SC, RESTYPE_TEXT, RESTYPE_QUO, RESTYPE_I,
	RESTYPE_OB, RESTYPE_FONT, RESTYPE_SOUND, RESTYPE_SPEECH, RESTYPE_HAS_EXT, RESTYPE_NO_EXT};

/**
 * HAG Archives implementation
 */
class HagArchive : public Common::Archive {
private:
	/**
	 * Details of a single entry in a HAG file index
	 */
	struct HagEntry {
		Common::String _resourceName;
		uint32 _offset;
		uint32 _size;

		HagEntry() : _offset(0), _size(0) {}
		HagEntry(Common::String resourceName, uint32 offset, uint32 size)
			: _resourceName(resourceName), _offset(offset), _size(size) {
		}
	};

	class HagIndex {
	public:
		Common::List<HagEntry> _entries;
		Common::String _filename;
	};

	Common::Array<HagIndex> _index;

	/**
	 * Load the index of all the game's HAG files
	 */
	void loadIndex(MADSEngine *vm);

	/**
	 * Given a resource name, opens up the correct HAG file and returns whether
	 * an entry with the given name exists.
	 */
	bool getHeaderEntry(const Common::String &resourceName, HagIndex &hagIndex, HagEntry &hagEntry) const;

	/**
	 * Returns the HAG resource filename that will contain a given resource
	 */
	Common::String getResourceFilename(const Common::String &resourceName) const;

	/**
	 * Return a resource type given a resource name
	 */
	ResourceType getResourceType(const Common::String &resourceName) const;
public:
	explicit HagArchive(MADSEngine *vm);
	~HagArchive() override;

	// Archive implementation
	bool hasFile(const Common::String &name) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;
};

const char *const MADSCONCAT_STRING = "MADSCONCAT";

HagArchive::HagArchive(MADSEngine *vm) {
	loadIndex(vm);
}

HagArchive::~HagArchive() {
}

// Archive implementation
bool HagArchive::hasFile(const Common::String &name) const {
	HagIndex hagIndex;
	HagEntry hagEntry;
	return getHeaderEntry(name, hagIndex, hagEntry);
}

int HagArchive::listMembers(Common::ArchiveMemberList &list) const {
	int members = 0;

	for (uint hagCtr = 0; hagCtr < _index.size(); ++hagCtr) {
		HagIndex hagIndex = _index[hagCtr];
		Common::List<HagEntry>::iterator i;

		for (i = hagIndex._entries.begin(); i != hagIndex._entries.end(); ++i) {
			list.push_back(Common::ArchiveMemberList::value_type(
				new Common::GenericArchiveMember((*i)._resourceName, this)));
			++members;
		}
	}

	return members;
}

const Common::ArchiveMemberPtr HagArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *HagArchive::createReadStreamForMember(const Common::String &name) const {
	HagIndex hagIndex;
	HagEntry hagEntry;

	if (getHeaderEntry(name, hagIndex, hagEntry)) {
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

void HagArchive::loadIndex(MADSEngine *vm) {
	Common::File hagFile;

	for (int sectionIndex = -1; sectionIndex < 11; ++sectionIndex) {
 		if (sectionIndex == 0 && !Common::File::exists("SECTION0.HAG"))
			continue;

		// Dragonsphere does not have some sections - skip them
		if (vm->getGameID() == GType_Dragonsphere)  {
			if (sectionIndex == 7 || sectionIndex == 8)
				continue;
		}

		// Phantom does not have some sections - skip them
		if (vm->getGameID() == GType_Phantom)  {
			if (sectionIndex == 6 || sectionIndex == 7 || sectionIndex == 8)
				continue;
		}

		Common::String filename = (sectionIndex == -1) ? "GLOBAL.HAG" :
			Common::String::format("SECTION%d.HAG", sectionIndex);
		if (sectionIndex == 10) {
			// Speech
			if (!Common::File::exists("SPEECH.HAG"))
				break;
			else
				filename = "SPEECH.HAG";
		}
		if (!hagFile.open(filename))
			error("Could not locate HAG file - %s", filename.c_str());

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

bool HagArchive::getHeaderEntry(const Common::String &resourceName,
		HagIndex &hagIndex, HagEntry &hagEntry) const {
	Common::String resName = resourceName;
	resName.toUppercase();
	if (resName[0] == '*')
		resName.deleteChar(0);

	Common::String hagFilename = getResourceFilename(resName);

	// Find the index for the given file
	for (uint hagCtr = 0; hagCtr < _index.size(); ++hagCtr) {
		hagIndex = _index[hagCtr];

		if (hagIndex._filename == hagFilename) {
			Common::List<HagEntry>::iterator ei;
			for (ei = hagIndex._entries.begin(); ei != hagIndex._entries.end(); ++ei) {
				hagEntry = *ei;
				if (hagEntry._resourceName.compareToIgnoreCase(resName) == 0)
					return true;
			}
		}
	}

	return false;
}

Common::String HagArchive::getResourceFilename(const Common::String &resourceName) const {
	ResourceType resType = getResourceType(resourceName);
	Common::String outputFilename = "GLOBAL.HAG";

	if ((resType == RESTYPE_ROOM) || (resType == RESTYPE_SC)) {
		int value = atoi(resourceName.c_str() + 2);
		int hagFileNum = (resType == RESTYPE_ROOM) ? value / 100 : value;

		if (hagFileNum >= 0)
			outputFilename = Common::String::format("SECTION%d.HAG", hagFileNum);
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

/*------------------------------------------------------------------------*/

void Resources::init(MADSEngine *vm) {
	SearchMan.add("HAG", new HagArchive(vm));
}

Common::String Resources::formatName(RESPREFIX resType, int id, const Common::String &ext) {
	Common::String result = "*";

	if (resType == 3 && !id) {
		id = id / 100;
	}

	if (!ext.empty()) {
		switch (resType) {
		case RESPREFIX_GL:
			result += "GL000";
			break;
		case RESPREFIX_SC:
			result += Common::String::format("SC%.3d", id);
			break;
		case RESPREFIX_RM:
			result += Common::String::format("RM%.3d", id);
			break;
		default:
			break;
		}

		result += ext;
	}

	return result;
}

Common::String Resources::formatName(int prefix, char asciiCh, int id, EXTTYPE extType,
		const Common::String &suffix) {
	Common::String result;
	if (prefix <= 0) {
		result = "*";
	} else {
		result = Common::String::format("%s%.3d",
			(prefix < 100) ? "*SC" : "*RM", prefix);
	}

	result += Common::String::format("%c", asciiCh);
	if (id >= 0)
		result += Common::String::format("%d", id);
	if (!suffix.empty())
		result += suffix;

	switch (extType) {
	case EXT_SS:
		result += ".SS";
		break;
	case EXT_AA:
		result += ".AA";
		break;
	case EXT_DAT:
		result += ".DAT";
		break;
	case EXT_HH:
		result += ".HH";
		break;
	case EXT_ART:
		result += ".ART";
		break;
	case EXT_INT:
		result += ".INT";
		break;
	default:
		break;
	}

	return result;
}

Common::String Resources::formatResource(const Common::String &resName,
		const Common::String &hagFilename) {
//	int v1 = 0, v2 = 0;

	if (resName.hasPrefix("*")) {
		// Resource file specified
		error("TODO: formatResource");
	} else {
		// File outside of hag file
		return resName;
	}
}

Common::String Resources::formatAAName(int idx) {
	return formatName(0, 'I', idx, EXT_AA, "");
}

/*------------------------------------------------------------------------*/

void File::openFile(const Common::String &filename) {
	if (!Common::File::open(filename))
		error("Could not open file - %s", filename.c_str());
}

/*------------------------------------------------------------------------*/

void SynchronizedList::synchronize(Common::Serializer &s) {
	int v = 0;
	int count = size();
	s.syncAsUint16LE(count);

	if (s.isSaving()) {
		for (int idx = 0; idx < count; ++idx) {
			v = (*this)[idx];
			s.syncAsSint32LE(v);
		}
	} else {
		clear();
		reserve(count);
		for (int idx = 0; idx < count; ++idx) {
			s.syncAsSint32LE(v);
			push_back(v);
		}
	}
}

} // End of namespace MADS
