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

#include "common/memstream.h"
#include "director/director.h"
#include "graphics/macgui/macfontmanager.h"

namespace Director {

class CachedArchive : public Common::Archive {
public:
	struct InputEntry {
		Common::String name;

		const byte *data;
		uint32 size;

		InputEntry(Common::String n, const byte *d, uint32 s) : name(n), data(d), size(s) {}
	};

	typedef Common::List<InputEntry> FileInputList;

	CachedArchive(const FileInputList &files);
	~CachedArchive() override;

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
private:
	struct Entry {
		const byte *data;
		uint32 size;
	};

	typedef Common::HashMap<Common::String, Entry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;
	FileMap _files;
};

struct CachedFile {
	const char *target;
	Common::Platform platform;

	const char *fileName;
	const byte *data;
	int32 size;			// Specify -1 if strlen(data) is the size
} const cachedFiles[] = {
	{ "trektech", Common::kPlatformWindows,
		"NCC1701D.INI",
			(const byte *)"cdromdrive=D\n", -1
	},
	{ "wolfgang", Common::kPlatformUnknown,
		"WOLFGANG.dat",	// It needs an empty file
			(const byte *)"", 0
	},
	{ "mcluhan", Common::kPlatformWindows,
		"prefs/Markers/List.Txt",	// It needs an empty file
			(const byte *)"", 0
	},
	{ nullptr, Common::kPlatformUnknown, nullptr, nullptr, 0 }
};


static void quirk640x480Desktop() {
    g_director->_wmMode &= ~Graphics::kWMModeNoDesktop;
    g_director->_wmWidth = 640;
    g_director->_wmHeight = 480;
}

static void quirkLzone() {
	SearchMan.addSubDirectoryMatching(g_director->_gameDataDir, "win_data", 0, 2);
}

static void quirkMcLuhan() {
	// TODO. Read fonts from MCLUHAN/SYSTEM directory
	g_director->_extraSearchPath.push_back("mcluhan");
	Graphics::MacFontManager *fontMan = g_director->_wm->_fontMan;
	fontMan->loadWindowsFont("MCLUHAN/SYSTEM/MCBOLD13.FON");
	fontMan->loadWindowsFont("MCLUHAN/SYSTEM/MCLURG__.FON");
	fontMan->loadWindowsFont("MCLUHAN/SYSTEM/MCL1N___.FON");
}

struct Quirk {
	const char *target;
	Common::Platform platform;
	void (*quirk)();
} quirks[] = {
	// Rodem expects to be able to track the mouse cursor outside the
	// window, which is impossible in ScummVM. Giving it a virtual
	// desktop allows it to work like it would ahve on the original OS.
	{ "henachoco05", Common::kPlatformMacintosh, &quirk640x480Desktop },
	{ "henachoco05", Common::kPlatformWindows, &quirk640x480Desktop },
    // Kids Box opens with a 320x150 splash screen before switching to
    // a full screen 640x480 game window. If desktop mode is off, ScummVM
    // will pick a game window that fits the splash screen and then try
    // to squish the full size game window into it.
    // It runs in 640x480; clipping it to this size ensures the main
    // game window takes up the full screen, and only the splash is windowed.
    { "kidsbox", Common::kPlatformMacintosh, &quirk640x480Desktop },
	{ "lzone", Common::kPlatformWindows, &quirkLzone },
	{ "mamauta1", Common::kPlatformMacintosh, &quirk640x480Desktop },
	{ "mamauta1", Common::kPlatformWindows, &quirk640x480Desktop },
	{ "mcluhan", Common::kPlatformWindows, &quirkMcLuhan },
	{ nullptr, Common::kPlatformUnknown, nullptr }
};

void DirectorEngine::gameQuirks(const char *target, Common::Platform platform) {
	for (auto q = quirks; q->target != nullptr; q++) {
		if (q->platform == Common::kPlatformUnknown || q->platform == platform)
			if (!strcmp(q->target, target)) {
				q->quirk();
				break;
			}
	}

	CachedArchive::FileInputList list;
	for (auto f = cachedFiles; f->target != nullptr; f++) {
		if (f->platform == Common::kPlatformUnknown || f->platform == platform)
			if (!strcmp(f->target, target)) {
				int32 size = f->size;
				if (size == -1)
					size = strlen((const char *)f->data);
				list.push_back(CachedArchive::InputEntry(f->fileName, f->data, size));
			}
	}

	if (!list.empty()) {
		CachedArchive *archive = new CachedArchive(list);

		SearchMan.add(kQuirksCacheArchive, archive);
	}
}

/*****************
 * CachedArchive
 *****************/

CachedArchive::CachedArchive(const FileInputList &files)
	: _files() {
	for (FileInputList::const_iterator i = files.begin(); i != files.end(); ++i) {
		Entry entry;

		entry.data = i->data;
		entry.size = i->size;

		Common::String name = i->name;
		name.toLowercase();
		_files[name] = entry;
	}
}

CachedArchive::~CachedArchive() {
	_files.clear();
}

bool CachedArchive::hasFile(const Common::Path &path) const {
	Common::String name = path.toString();
	return (_files.find(name) != _files.end());
}

int CachedArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (FileMap::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, this)));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr CachedArchive::getMember(const Common::Path &path) const {
	Common::String name = path.toString();
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *CachedArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::String name = path.toString();
	FileMap::const_iterator fDesc = _files.find(name);
	if (fDesc == _files.end())
		return nullptr;

	return new Common::MemoryReadStream(fDesc->_value.data, fDesc->_value.size, DisposeAfterUse::NO);
}


} // End of namespace Director
