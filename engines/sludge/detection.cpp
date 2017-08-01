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
#include "common/debug.h"
#include "common/stream.h"

#include "engines/advancedDetector.h"

#include "sludge/sludge.h"

namespace Sludge {

struct SludgeGameDescription {
	ADGameDescription desc;
	uint languageID;
};

uint SludgeEngine::getLanguageID() const { return _gameDescription->languageID; }
const char *SludgeEngine::getGameId() const { return _gameDescription->desc.gameId;}
uint32 SludgeEngine::getFeatures() const { return _gameDescription->desc.flags; }
Common::Language SludgeEngine::getLanguage() const { return _gameDescription->desc.language; }
const char *SludgeEngine::getGameFile() const {
	return _gameDescription->desc.filesDescriptions[0].fileName;
}

} // End of namespace Sludge

static const PlainGameDescriptor sludgeGames[] = {
	{ "sludge", "Sludge Game" },
	{ "welcome", "Welcome Example" },
	{ "verbcoin", "Verb Coin" },
	{ "robinsrescue", "Robin's Rescue" },
	{ "outoforder", "Out Of Order" },
	{ "frasse", "Frasse and the Peas of Kejick" },
	{ "interview", "The Interview" },
	{ "life", "Life Flashed By"},
	{ "tgttpoacs", "The Game That Takes Place on a Cruise Ship" },
	{ "mandy", "Mandy Christmas Adventure" },
	{ "cubert", "Cubert Badbone, P.I." },
	{ 0, 0 }
};

#include "sludge/detection_tables.h"

static Sludge::SludgeGameDescription s_fallbackDesc =
{
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	0
};

static char s_fallbackFileNameBuffer[51];

class SludgeMetaEngine : public AdvancedMetaEngine {
public:
	SludgeMetaEngine() : AdvancedMetaEngine(Sludge::gameDescriptions, sizeof(Sludge::SludgeGameDescription), sludgeGames) {
		_singleId = "sludge";
		_maxScanDepth = 1;
	}

	virtual const char *getName() const {
		return "Sludge Engine";
	}
 
	virtual const char *getOriginalCopyright() const {
		return "Copyright (C) 2000-2014 Hungry Software and contributors";
	}
 
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		const Sludge::SludgeGameDescription *gd = (const Sludge::SludgeGameDescription *)desc;
			if (gd) {
				*engine = new Sludge::SludgeEngine(syst, gd);
			}
			return gd != 0;
	}

	// for fall back detection
	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const;
};

const ADGameDescription *SludgeMetaEngine::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	// reset fallback description
	s_fallbackDesc.desc.gameId = "sludge";
	s_fallbackDesc.desc.extra = "";
	s_fallbackDesc.desc.language = Common::EN_ANY;
	s_fallbackDesc.desc.flags = ADGF_UNSTABLE;
	s_fallbackDesc.desc.platform = Common::kPlatformUnknown;
	s_fallbackDesc.desc.guiOptions = GUIO0();
	s_fallbackDesc.languageID = 0;

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String fileName = file->getName();
		fileName.toLowercase();
		if (!(fileName.hasSuffix(".slg") || fileName == "gamedata"))
			continue;

		SearchMan.clear();
		SearchMan.addDirectory(file->getParent().getName(), file->getParent());

		Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(file->getName());

		if (!stream)
			continue;

		bool headerBad = false;
		if (stream->readByte() != 'S')
			headerBad = true;
		if (stream->readByte() != 'L')
			headerBad = true;
		if (stream->readByte() != 'U')
			headerBad = true;
		if (stream->readByte() != 'D')
			headerBad = true;
		if (stream->readByte() != 'G')
			headerBad = true;
		if (stream->readByte() != 'E')
			headerBad = true;
		if (headerBad) {
			continue;
		}

		strncpy(s_fallbackFileNameBuffer, fileName.c_str(), 50);
		s_fallbackFileNameBuffer[50] = '\0';
		s_fallbackDesc.desc.filesDescriptions[0].fileName = s_fallbackFileNameBuffer;

		return (const ADGameDescription *)&s_fallbackDesc;;
	}
	return 0;
}

#if PLUGIN_ENABLED_DYNAMIC(SLUDGE)
	REGISTER_PLUGIN_DYNAMIC(SLUDGE, PLUGIN_TYPE_ENGINE, SludgeMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SLUDGE, PLUGIN_TYPE_ENGINE, SludgeMetaEngine);
#endif
