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

#include "common/compression/vise.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "director/director.h"

namespace Director {

class CachedArchive : public Common::Archive {
public:
	struct InputEntry {
		Common::Path name;

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

	typedef Common::HashMap<Common::Path, Entry, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> FileMap;
	FileMap _files;
};

struct CachedFile {
	const char *target;
	Common::Platform platform;

	const char *fileName;
	const byte *data;
	int32 size;			// Specify -1 if strlen(data) is the size
} const cachedFiles[] = {
	{
		"directortest", Common::kPlatformUnknown,
		"0testfile",
			(const byte *)"", 0
	},
	{ "nine", Common::kPlatformWindows,
		"TRIBECA.ini",
			(const byte *)"[Main]\r\nCDROM=d:", -1
	},
	{ "trektech", Common::kPlatformWindows,
		"NCC1701D.INI",
			(const byte *)"cdromdrive=D\n", -1
	},
	{ "wolfgang", Common::kPlatformUnknown,
		"WOLFGANG.dat",	// It needs an empty file
			(const byte *)"", 0
	},
	{ "teamxtreme1", Common::kPlatformWindows,
		// In Operation: Weather Disaster, the game will try and check if the
		// save file exists with getNthFileNameInFolder before attempting to
		// read it with FileIO (which uses the save data store).
		"WINDOWS/TXSAVES",
			(const byte *)"", 0
	},
	{ "teamxtreme2", Common::kPlatformWindows,
		// In Operation: Eco-Nightmare, the game will try and check if the
		// save file exists with getNthFileNameInFolder before attempting to
		// read it with FileIO (which uses the save data store).
		"WINDOWS/TX2SAVES",
			(const byte *)"", 0
	},
	{ "paws", Common::kPlatformWindows,
		// PAWS: Personal Automated Wagging System checks a file to determine
		// the location of the CD.
		"INSTALL.INF",
			(const byte *)"CDDrive=D:\\\r\nSourcePath=D:\\\r\nDestPath=C:\\", -1
	},
	{"tkkg1", Common::kPlatformWindows,
		// TKKG1 checks a file to determine the location of the CD.
		"PATH.INI",
		(const byte *)"[cd-path]\r\npath=d:\\", -1
	},
	// Professor Finkle's Times Table Factory has an installer that copies a bunch of empty files,
	// which the game gets upset about if they don't exist.
	{"finkletimes", Common::kPlatformWindows, "finkle.ini", (const byte *)"", 0},
	{"finkletimes", Common::kPlatformWindows, "beatswch.txt", (const byte *)"", 0},
	{"finkletimes", Common::kPlatformWindows, "fctrlist.txt", (const byte *)"", 0},
	{"finkletimes", Common::kPlatformWindows, "gridscor.txt", (const byte *)"", 0},
	{"finkletimes", Common::kPlatformWindows, "jokelist.txt", (const byte *)"", 0},
	{"finkletimes", Common::kPlatformWindows, "lastplay.txt", (const byte *)"", 0},
	{"finkletimes", Common::kPlatformWindows, "lernscor.txt", (const byte *)"", 0},
	{"finkletimes", Common::kPlatformWindows, "namelist.txt", (const byte *)"", 0},
	{"finkletimes", Common::kPlatformWindows, "userlist.txt", (const byte *)"", 0},
	{ nullptr, Common::kPlatformUnknown, nullptr, nullptr, 0 }
};

struct SaveFilePath {
	const char *target;
	Common::Platform platform;
	const char *path;
} const saveFilePaths[] = {
	{ "darkeye", Common::kPlatformWindows, "SAVEDDKY/" },
	{"simpsonsstudio", Common::kPlatformWindows, "SIMPSONS/SUPPORT/TOONDATA/"},
	{"simpsonsstudio", Common::kPlatformMacintosh, "SIMPSONS/SUPPORT/TOONDATA/"},
	{ nullptr, Common::kPlatformUnknown, nullptr },
};


static void quirkWarlock() {
	g_director->_loadSlowdownFactor = 150000;  // emulate a 1x CD drive
	g_director->_fpsLimit = 15;
}

static void quirkLimit15FPS() {
	g_director->_fpsLimit = 15;
}

static void quirkPretend16Bit() {
	g_director->_colorDepth = 16;
}

static void quirkHollywoodHigh() {
	// Hollywood High demo has a killswitch that stops playback
	// if the year is after 1996.
	g_director->_forceDate.tm_year = 1996 - 1900;
	g_director->_forceDate.tm_mon = 0;
	g_director->_forceDate.tm_mday = 1;
	g_director->_forceDate.tm_wday = 0;
}

static void quirkLzone() {
	SearchMan.addSubDirectoryMatching(g_director->_gameDataDir, "win_data", 0, 2);
}

static void quirkMcLuhanWin() {
	g_director->_extraSearchPath.push_back("mcluhan\\");
	Graphics::MacFontManager *fontMan = g_director->_wm->_fontMan;
	fontMan->loadWindowsFont("MCLUHAN/SYSTEM/MCBOLD13.FON");
	fontMan->loadWindowsFont("MCLUHAN/SYSTEM/MCLURG__.FON");
	fontMan->loadWindowsFont("MCLUHAN/SYSTEM/MCL1N___.FON");
}

static void quirkTrekTechWin() {
	Graphics::MacFontManager *fontMan = g_director->_wm->_fontMan;
	fontMan->loadWindowsFont("TREKCON4.FON");
}

static void quirkTrekGuideTNGWin() {
	Graphics::MacFontManager *fontMan = g_director->_wm->_fontMan;
	fontMan->loadWindowsFont("OMNI2/TREKENCY.FON");
	fontMan->loadWindowsFont("OMNI2/TREKOMNI.FON");
}


static void quirkPipCatalog() {
	g_director->_dirSeparator = '/';
}

static void quirkMcLuhanMac() {
	Common::SeekableReadStream *installer = Common::MacResManager::openFileOrDataFork("Understanding McLuhan Installer");

	if (!installer) {
		warning("quirkMcLuhanMac(): Cannot open installer file");
		return;
	}

	Common::Archive *archive = Common::createMacVISEArchive(installer);

	if (!archive) {
		warning("quirkMcLuhanMac(): Failed to open installer");
		return;
	}

	Common::MacResManager font;

	if (!font.open("McLuhan-Regular", *archive)) {
		warning("quirkMcLuhanMac(): Failed to load font file \"McLuhan-Regular\"");
		return;
	}

	Graphics::MacFontManager *fontMan = g_director->_wm->_fontMan;
	fontMan->loadFonts(&font);

	delete archive;
	delete installer;
}

const struct Quirk {
	const char *target;
	Common::Platform platform;
	void (*quirk)();
} quirks[] = {
	// Spaceship Warlock is designed to run as quickly as possible on a
	// single speed CD drive; there's often content just before a movie
	// transition which would otherwise get skipped past.
	{ "warlock", Common::kPlatformMacintosh, &quirkWarlock },
	{ "warlock", Common::kPlatformWindows, &quirkWarlock },

	// Eastern Mind sets the score to play back at a high frame rate,
	// however the developers were using slow hardware, so some
	// animations play back much faster than intended.
	// Limit the score framerate to be no higher than 15fps.
	{ "easternmind", Common::kPlatformMacintosh, &quirkLimit15FPS },
	{ "easternmind", Common::kPlatformWindows, &quirkLimit15FPS },

	// Sections of Hell Cab such as the prehistoric times need capped framerate.
	{ "hellcab", Common::kPlatformMacintosh, &quirkLimit15FPS },
	{ "hellcab", Common::kPlatformWindows, &quirkLimit15FPS },

	// Wrath of the Gods has shooting gallery minigames which are
	// clocked to 60fps; in reality this is far too fast to be playable.
	{ "wrath", Common::kPlatformMacintosh, &quirkLimit15FPS },
	{ "wrath", Common::kPlatformWindows, &quirkLimit15FPS },

	{ "hollywoodhigh", Common::kPlatformWindows, &quirkHollywoodHigh },

	{ "lzone", Common::kPlatformWindows, &quirkLzone },

	{ "mcluhan", Common::kPlatformWindows, &quirkMcLuhanWin },
	{ "mcluhan", Common::kPlatformMacintosh, &quirkMcLuhanMac },

	// Star Trek titles install fonts into the system
	{ "trektech", Common::kPlatformWindows, &quirkTrekTechWin },
	{ "trekguidetng", Common::kPlatformWindows, &quirkTrekGuideTNGWin },

	// Pippin game that uses Unix path separators rather than Mac
	{ "pipcatalog", Common::kPlatformPippin, &quirkPipCatalog },

	// Some games pop up a nag mesasage if the color depth isn't exactly 16 bit.
	{ "vnc", Common::kPlatformWindows, &quirkPretend16Bit },
	{ "vnc", Common::kPlatformMacintosh, &quirkPretend16Bit },
	{ "finkletimes", Common::kPlatformWindows, &quirkPretend16Bit },
	{ "finkletimes", Common::kPlatformMacintosh, &quirkPretend16Bit },
	{ "flipper", Common::kPlatformMacintosh, &quirkPretend16Bit },
	{ "flipper", Common::kPlatformWindows, &quirkPretend16Bit },

	{ nullptr, Common::kPlatformUnknown, nullptr }
};

void DirectorEngine::gameQuirks(const char *target, Common::Platform platform) {
	for (auto q = quirks; q->target != nullptr; q++) {
		if (q->platform == Common::kPlatformUnknown || q->platform == platform)
			if (!strcmp(q->target, target)) {
				debugC(1, kDebugLoading, "Applying quirk for the target %s", target);

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

				debugC(1, kDebugLoading, "Added file '%s' of size %d to the file cache", f->fileName, size);
			}
	}

	for (auto f = saveFilePaths; f->target != nullptr; f++) {
		if (f->platform == Common::kPlatformUnknown || f->platform == platform)
			if (!strcmp(f->target, target)) {
				// Inject files from the save game storage into the path
				Common::SaveFileManager *saves = g_system->getSavefileManager();
				// As save games are name-mangled by FileIO, demangle them here
				Common::String prefix = savePrefix() + '*';
				for (auto &it : saves->listSavefiles(prefix.c_str())) {
					Common::String demangled = f->path + it.substr(prefix.size() - 1);
					if (demangled.hasSuffixIgnoreCase(".txt")) {
						demangled = demangled.substr(0, demangled.size() - 4);
					}
					list.push_back(CachedArchive::InputEntry(demangled, nullptr, 0));
				}
			}

	}

	if (!list.empty()) {
		CachedArchive *archive = new CachedArchive(list);

		// If gameQuirks is called as an update we need to remove the old quirks cache
		// archive before adding the new one.
		if (SearchMan.hasArchive(kQuirksCacheArchive)) {
			SearchMan.remove(kQuirksCacheArchive);
		}

		SearchMan.add(kQuirksCacheArchive, archive);
	}
}

void DirectorEngine::loadSlowdownCooloff(uint32 delay) {
	if (_loadSlowdownFactor)
		_loadSlowdownCooldownTime = g_system->getMillis() + delay;
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

		Common::Path name = i->name;
		name.toLowercase();
		_files[name] = entry;
	}
}

CachedArchive::~CachedArchive() {
	_files.clear();
}

bool CachedArchive::hasFile(const Common::Path &path) const {
	return (_files.find(path) != _files.end());
}

int CachedArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (FileMap::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, *this)));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr CachedArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *CachedArchive::createReadStreamForMember(const Common::Path &path) const {
	FileMap::const_iterator fDesc = _files.find(path);
	if (fDesc == _files.end())
		return nullptr;

	return new Common::MemoryReadStream(fDesc->_value.data, fDesc->_value.size, DisposeAfterUse::NO);
}


} // End of namespace Director
