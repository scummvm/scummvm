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

#include "common/savefile.h"
#include "common/system.h"

#include "engines/advancedDetector.h"

#include "buried/buried.h"

namespace Buried {

bool BuriedEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher)
		|| (f == kSupportsLoadingDuringRuntime)
		|| (f == kSupportsSavingDuringRuntime);
}

bool BuriedEngine::isDemo() const {
	// The trial is a demo for the user's sake, but not internally.
	return (_gameDescription->flags & ADGF_DEMO) != 0 && !isTrial();
}

bool BuriedEngine::isTrial() const {
	return (_gameDescription->flags & GF_TRIAL) != 0;
}

bool BuriedEngine::isTrueColor() const {
	return (_gameDescription->flags & GF_TRUECOLOR) != 0;
}

bool BuriedEngine::isWin95() const {
	return (_gameDescription->flags & GF_WIN95) != 0;
}

bool BuriedEngine::isCompressed() const {
	return (_gameDescription->flags & GF_COMPRESSED) != 0;
}

Common::String BuriedEngine::getEXEName() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

Common::String BuriedEngine::getLibraryName() const {
	return _gameDescription->filesDescriptions[1].fileName;
}

Common::Language BuriedEngine::getLanguage() const {
	return _gameDescription->language;
}

} // End of namespace Buried

class BuriedMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const {
		return "buried";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
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
	const Common::StringArray &fileNames = Buried::BuriedEngine::listSaveFiles();
	g_system->getSavefileManager()->removeSavefile(fileNames[slot].c_str());
}

Common::Error BuriedMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Buried::BuriedEngine(syst, desc);

	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(BURIED)
	REGISTER_PLUGIN_DYNAMIC(BURIED, PLUGIN_TYPE_ENGINE, BuriedMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BURIED, PLUGIN_TYPE_ENGINE, BuriedMetaEngine);
#endif
