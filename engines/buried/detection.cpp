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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"

#include "buried/buried.h"

namespace Buried {

struct BuriedGameDescription {
	ADGameDescription desc;
};

enum {
	GF_TRUECOLOR  = (1 << 1),
	GF_WIN95      = (1 << 2),
	GF_COMPRESSED = (1 << 3),
	GF_TRIAL      = (1 << 4)
};

bool BuriedEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL)
		|| (f == kSupportsLoadingDuringRuntime)
		|| (f == kSupportsSavingDuringRuntime);
}

bool BuriedEngine::isDemo() const {
	// The trial is a demo for the user's sake, but not internally.
	return (_gameDescription->desc.flags & ADGF_DEMO) != 0 && !isTrial();
}

bool BuriedEngine::isTrial() const {
	return (_gameDescription->desc.flags & GF_TRIAL) != 0;
}

bool BuriedEngine::isTrueColor() const {
	return (_gameDescription->desc.flags & GF_TRUECOLOR) != 0;
}

bool BuriedEngine::isWin95() const {
	return (_gameDescription->desc.flags & GF_WIN95) != 0;
}

bool BuriedEngine::isCompressed() const {
	return (_gameDescription->desc.flags & GF_COMPRESSED) != 0;
}

Common::String BuriedEngine::getEXEName() const {
	return _gameDescription->desc.filesDescriptions[0].fileName;
}

Common::String BuriedEngine::getLibraryName() const {
	return _gameDescription->desc.filesDescriptions[1].fileName;
}

Common::Language BuriedEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Buried

static const PlainGameDescriptor buriedGames[] = {
	{"buried", "The Journeyman Project 2: Buried in Time"},
	{0, 0}
};

#include "buried/detection_tables.h"

namespace Buried {

static const char *directoryGlobs[] = {
	"win31",
	"manual",
	0
};

} // End of namespace Buried


class BuriedMetaEngine : public AdvancedMetaEngine {
public:
	BuriedMetaEngine() : AdvancedMetaEngine(Buried::gameDescriptions, sizeof(Buried::BuriedGameDescription), buriedGames) {
		_singleid = "buried";
		_flags = kADFlagUseExtraAsHint;
		_maxScanDepth = 3;
		_directoryGlobs = Buried::directoryGlobs;
	}

	virtual const char *getName() const {
		return "The Journeyman Project 2: Buried in Time";
	}

	virtual const char *getOriginalCopyright() const {
		return "The Journeyman Project 2: Buried in Time (C) Presto Studios";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const { return 999; }
	virtual void removeSaveState(const char *target, int slot) const;
};

bool BuriedMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves)
		|| (f == kSupportsLoadingDuringStartup)
		|| (f == kSupportsDeleteSave);
}

SaveStateList BuriedMetaEngine::listSaves(const char *target) const {
	// The original had no pattern, so the user must rename theirs
	// Note that we ignore the target because saves are compatible between
	// all versions
	Common::StringArray fileNames = Buried::BuriedEngine::listSaveFiles();

	SaveStateList saveList;
	for (uint32 i = 0; i < fileNames.size(); i++) {
		// Isolate the description from the file name
		Common::String desc = fileNames[i].c_str() + 7;
		for (int j = 0; j < 4; j++)
			desc.deleteLastChar();

		saveList.push_back(SaveStateDescriptor(i, desc));
	}

	return saveList;
}

void BuriedMetaEngine::removeSaveState(const char *target, int slot) const {
	// See listSaves() for info on the pattern
	Common::StringArray fileNames = Buried::BuriedEngine::listSaveFiles();
	g_system->getSavefileManager()->removeSavefile(fileNames[slot].c_str());
}

bool BuriedMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Buried::BuriedGameDescription *gd = (const Buried::BuriedGameDescription *)desc;

	if (gd)
		*engine = new Buried::BuriedEngine(syst, gd);

	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(BURIED)
	REGISTER_PLUGIN_DYNAMIC(BURIED, PLUGIN_TYPE_ENGINE, BuriedMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BURIED, PLUGIN_TYPE_ENGINE, BuriedMetaEngine);
#endif
