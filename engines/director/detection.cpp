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

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "common/file.h"
#include "common/config-manager.h"

#include "director/director.h"

namespace Director {

struct DirectorGameDescription {
	ADGameDescription desc;

	DirectorGameGID gameGID;
	uint16 version;
};

DirectorGameGID DirectorEngine::getGameGID() const {
	return _gameDescription->gameGID;
}

const char *DirectorEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Platform DirectorEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 DirectorEngine::getVersion() const {
	return _gameDescription->version;
}

Common::Language DirectorEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::String DirectorEngine::getEXEName() const {
	if (ConfMan.hasKey("start_movie"))
		return ConfMan.get("start_movie");

	return _gameDescription->desc.filesDescriptions[0].fileName;
}

bool DirectorEngine::hasFeature(EngineFeature f) const {
	return false;
		//(f == kSupportsReturnToLauncher);
}

} // End of Namespace Director

static const PlainGameDescriptor directorGames[] = {
	{ "director",			"Macromedia Director Game" },
	{ "directortest",		"Macromedia Director Test Target" },
	{ "directortest-all",	"Macromedia Director All Movies Test Target" },
	{ "theapartment",		"The Apartment, Interactive demo" },

	{ "9worlds",			"Nine Worlds hosted by Patrick Stewart"},
	{ "alexworld",			"ALeX-WORLD"},
	{ "alice",				"Alice: An Interactive Museum"},
	{ "amandastories",		"AmandaStories"},
	{ "amber",				"AMBER: Journeys Beyond"},
	{ "ankh1",				"Ankh: Mystery of the Pyramids"},
	{ "ankh2",				"Ankh 2: Mystery of Tutankhamen"},
	{ "ankh3",				"Ankh 3"},
	{ "arcofdoom",			"Arc of Doom"},
	{ "ataripack",			"Activision's Atari 2600 Action Pack"},
	{ "badday",				"Bad Day on the Midway"},
	{ "beyondthewall",		"Beyond the Wall of Stars"},
	{ "bowie",				"JUMP: The David Bowie Interactive CD-ROM"},
	{ "chaos",				"The C.H.A.O.S. Continuum"},
	{ "chopsuey",   		"Chop Suey" },
	{ "chuteng",   			"Chu-Teng" },
	{ "daedalus",			"The Daedalus Encounter"},
	{ "darkeye",			"The Dark Eye"},
	{ "derratsorcerum",		"Derrat Sorcerum"},
	{ "devo",				"DEVO Presents: Adventures of the Smart Patrol"},
	{ "earthtia",			"Earthtia Saga: Larthur's Legend"},
	{ "easternmind",		"Eastern Mind: The Lost Souls of Tong Nou"},
	{ "earthwormjim",		"Earthworm Jim"},
	{ "ernie",				"Ernie"},
	{ "frankenstein",		"Frankenstein: Through the Eyes of the Monster"},
	{ "freakshow",			"Freak Show"},
	{ "gadget",				"Gadget: Invention, Travel, & Adventure"},
	{ "gundam0079",			"Gundam 0079: The War for Earth" },
	{ "hamsterland1",		"Busy People of Hamsterland" },
	{ "hamsterland2",		"Hamsterland: The Time Machine" },
	{ "horrortour1",		"Zeddas: Servant of Sheol"},
	{ "horrortour2",		"Zeddas: Horror Tour 2"},
	{ "horrortour3",		"Labyrinthe"},
	{ "hyperblade",			"HyperBlade" },
	{ "id4p1",     			"iD4 Mission Disk 1 - Alien Supreme Commander" },
	{ "id4p2",      		"iD4 Mission Disk 2 - Alien Science Officer" },
	{ "id4p3",      		"iD4 Mission Disk 3 - Warrior Alien" },
	{ "id4p4",      		"iD4 Mission Disk 4 - Alien Navigator" },
	{ "id4p5",      		"iD4 Mission Disk 5 - Captain Steve Hiller" },
	{ "id4p6",      		"iD4 Mission Disk 6 - Dave's Computer" },
	{ "id4p7",      		"iD4 Mission Disk 7 - President Whitmore" },
	{ "id4p8",      		"iD4 Mission Disk 8 - Alien Attack Fighter" },
	{ "id4p9",      		"iD4 Mission Disk 9 - FA-18 Fighter Jet" },
	{ "id4p10",     		"iD4 Mission Disk 10 - Alien Bomber" },
	{ "id4p11",     		"iD4 Mission Disk 11 - Area 51" },
	{ "improv",     		"Don't Quit Your Day Job" },
	{ "ironhelix",			"Iron Helix" },
	{ "isis",				"Isis"},
	{ "jewels",				"Jewels of the Oracle" },
	{ "jman",				"The Journeyman Project" },
	{ "jman2",				"The Journeyman Project 2: Buried in Time" },
	{ "karma",				"Karma: Curse of the 12 Caves" },
	{ "kyoto",				"Cosmology of Kyoto" },
	{ "lion",				"Lion" },
	{ "louiscatorze",		"Louis Cat Orze: The Mystery of the Queen's Necklace" },
	{ "lzone",				"L-ZONE"},
	{ "madmac",				"Mad Mac Cartoons"},
	{ "majestic",			"Majestic Part I: Alien Encounter" },
	{ "martian",			"Ray Bradbury's The Martian Chronicles Adventure Game" },
	{ "maze",				"The Riddle of the Maze"},
	{ "mechwarrior2",		"MechWarrior 2" },
	{ "mediaband",			"Meet Mediaband" },
	{ "melements",			"Masters of the Elements" },
	{ "mirage",				"Mirage" },
	{ "mummy",				"Mummy: Tomb of the Pharaoh"},
	{ "muppets",			"Muppet Treasure Island" },
	{ "murderbrett",		"Who Killed Brett Penance?"},
	{ "murdermagic",		"The Magic Death"},
	{ "murdersam",			"Who Killed Sam Rupert?"},
	{ "murdertaylor",		"Who Killed Taylor French? The Case of the Undressed Reporter"},
	{ "mylk",				"Mylk"},
	{ "necrobius",			"Necrobius"},
	{ "nile",				"Nile: Passage to Egypt"},
	{ "noir",				"Noir: A Shadowy Thriller"},
	{ "operafatal",			"Opera Fatal"},
	{ "phantasplanet",		"Phantasmagoria Amusement Planet"},
	{ "pitfall",			"Pitfall: The Mayan Adventure" },
	{ "planetarizona",		"Escape from Planet Arizona" },
	{ "prescue",			"Paradise Rescue" },
	{ "refixion1",			"Refixion"},
	{ "refixion2",			"Refixion II: Museum or Hospital"},
	{ "refixion3",			"Refixion III: The Reindeer Story"},
	{ "rodney",				"Rodney's Funscreen"},
	{ "santafe1",			"Santa Fe Mysteries: The Elk Moon Murder"},
	{ "screamingmetal",		"Screaming Metal"},
	{ "shanghai",			"Shanghai: Great Moments"},
	{ "skyborg",			"SkyBorg: Into the Vortex"},
	{ "snh",				"A Silly Noisy House"},
	{ "spyclub",			"Spy Club" },
	{ "spycraft",			"Spycraft: The Great Game" },
	{ "superspy",			"SuperSpy 1" },
	{ "teamxtreme1",		"Operation: Weather Disaster" },
	{ "teamxtreme2",		"Operation: Eco-Nightmare" },
	{ "the7colors",			"The Seven Colors: Legend of PSY-S City"},
	{ "totaldistortion",	"Total Distortion"},
	{ "trekborg",			"Star Trek: Borg"},
	{ "trekguideds9",		"Star Trek: Deep Space Nine Episode Guide"},
	{ "trekguidetng",		"Star Trek: The Next Generation Episode Guide"},
	{ "trekklingon",		"Star Trek: Klingon"},
	{ "trekomni",			"Star Trek Omnipedia"},
	{ "trekpedia98",		"Star Trek Encyclopedia 1998"},
	{ "tri3dtrial",			"Tri-3D-Trial"},
	{ "vvcyber",			"Victor Vector & Yondo: The Cyberplasm Formula"},
	{ "vvdinosaur",			"Victor Vector & Yondo: The Last Dinosaur Egg"},
	{ "vvharp",				"Victor Vector & Yondo: The Hypnotic Harp"},
	{ "vvvampire",			"Victor Vector & Yondo: The Vampire's Coffin"},
	{ "warlock", 			"Spaceship Warlock"},
	{ "wrath",				"Wrath of the Gods"},
	{ "xanthus",			"Xanthus"},
	{ "ybr1",				"Yellow Brick Road"},
	{ "ybr2",				"Yellow Brick Road II"},
	{ "ybr3",				"Yellow Brick Road III"},
	{ "znemesis",			"Zork Nemesis: The Forbidden Lands"},
	{ 0, 0 }
};

#include "director/detection_tables.h"

static const char *directoryGlobs[] = {
	"install",
	0
};

class DirectorMetaEngine : public AdvancedMetaEngine {
public:
	DirectorMetaEngine() : AdvancedMetaEngine(Director::gameDescriptions, sizeof(Director::DirectorGameDescription), directorGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "director";
	}

	const char *getName() const override {
		return "Macromedia Director";
	}

	const char *getOriginalCopyright() const override {
		return "Macromedia Director (C) 1990-1995 Macromedia";
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override;
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool DirectorMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Director::DirectorGameDescription *gd = (const Director::DirectorGameDescription *)desc;

	if (gd)
		*engine = new Director::DirectorEngine(syst, gd);

	return (gd != 0);
}

static Director::DirectorGameDescription s_fallbackDesc = {
	{
		"director",
		"",
		AD_ENTRY1(0, 0),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	Director::GID_GENERIC,
	0
};

static char s_fallbackFileNameBuffer[51];

ADDetectedGame DirectorMetaEngine::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	// TODO: Handle Mac fallback

	// reset fallback description
	Director::DirectorGameDescription *desc = &s_fallbackDesc;
	desc->desc.gameId = "director";
	desc->desc.extra = "";
	desc->desc.language = Common::UNK_LANG;
	desc->desc.flags = ADGF_NO_FLAGS;
	desc->desc.platform = Common::kPlatformWindows;
	desc->desc.guiOptions = GUIO0();
	desc->desc.filesDescriptions[0].fileName = 0;
	desc->version = 0;
	desc->gameGID = Director::GID_GENERIC;

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String fileName = file->getName();
		fileName.toLowercase();
		if (!fileName.hasSuffix(".exe"))
			continue;

		Common::File f;
		if (!f.open(*file))
			continue;

		f.seek(-4, SEEK_END);

		uint32 offset = f.readUint32LE();

		if (f.eos() || offset == 0 || offset >= (uint32)(f.size() - 4))
			continue;

		f.seek(offset);

		uint32 tag = f.readUint32LE();

		switch (tag) {
		case MKTAG('P', 'J', '9', '3'):
		case MKTAG('3', '9', 'J', 'P'):
			desc->version = 4;
			break;
		case MKTAG('P', 'J', '9', '5'):
			desc->version = 5;
			break;
		case MKTAG('P', 'J', '0', '0'):
			desc->version = 7;
			break;
		default:
			// Prior to version 4, there was no tag here. So we'll use a bit of a
			// heuristic to detect. The first field is the entry count, of which
			// there should only be one.
			if ((tag & 0xFFFF) != 1)
				continue;

			f.skip(3);

			uint32 mmmSize = f.readUint32LE();

			if (f.eos() || mmmSize == 0)
				continue;

			byte fileNameSize = f.readByte();

			if (f.eos())
				continue;

			f.skip(fileNameSize);
			byte directoryNameSize = f.readByte();

			if (f.eos())
				continue;

			f.skip(directoryNameSize);

			if (f.pos() != f.size() - 4)
				continue;

			// Assume v3 at this point (for now at least)
			desc->version = 3;
		}

		strncpy(s_fallbackFileNameBuffer, fileName.c_str(), 50);
		s_fallbackFileNameBuffer[50] = '\0';
		desc->desc.filesDescriptions[0].fileName = s_fallbackFileNameBuffer;

		warning("Director fallback detection D%d", desc->version);

		return ADDetectedGame(&desc->desc);
	}

	return ADDetectedGame();
}

#if PLUGIN_ENABLED_DYNAMIC(DIRECTOR)
	REGISTER_PLUGIN_DYNAMIC(DIRECTOR, PLUGIN_TYPE_ENGINE, DirectorMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DIRECTOR, PLUGIN_TYPE_ENGINE, DirectorMetaEngine);
#endif
