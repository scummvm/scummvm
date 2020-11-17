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
#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"

#include "pegasus/pegasus.h"
#include "pegasus/detection.h"

namespace Pegasus {

bool PegasusEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher)
		|| (f == kSupportsLoadingDuringRuntime)
		|| (f == kSupportsSavingDuringRuntime);
}

bool PegasusEngine::isDemo() const {
	return (_gameDescription->desc.flags & ADGF_DEMO) != 0;
}

bool PegasusEngine::isDVD() const {
	return (_gameDescription->desc.flags & GF_DVD) != 0;
}

bool PegasusEngine::isDVDDemo() const {
	return isDemo() && isDVD();
}

bool PegasusEngine::isOldDemo() const {
	return isDemo() && !isDVD();
}

bool PegasusEngine::isWindows() const {
	return _gameDescription->desc.platform == Common::kPlatformWindows;
}

} // End of namespace Pegasus

class PegasusMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "pegasus";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override { return 999; }
	void removeSaveState(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool PegasusMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves)
		|| (f == kSupportsLoadingDuringStartup)
		|| (f == kSupportsDeleteSave);
}

SaveStateList PegasusMetaEngine::listSaves(const char *target) const {
	// The original had no pattern, so the user must rename theirs
	// Note that we ignore the target because saves are compatible between
	// all versions
	Common::StringArray fileNames = Pegasus::PegasusEngine::listSaveFiles();

	SaveStateList saveList;
	for (uint32 i = 0; i < fileNames.size(); i++) {
		// Isolate the description from the file name
		Common::String desc = fileNames[i].c_str() + 8;
		for (int j = 0; j < 4; j++)
			desc.deleteLastChar();

		saveList.push_back(SaveStateDescriptor(i, desc));
	}

	return saveList;
}

void PegasusMetaEngine::removeSaveState(const char *target, int slot) const {
	// See listSaves() for info on the pattern
	Common::StringArray fileNames = Pegasus::PegasusEngine::listSaveFiles();
	g_system->getSavefileManager()->removeSavefile(fileNames[slot].c_str());
}

Common::KeymapArray PegasusMetaEngine::initKeymaps(const char *target) const {
	return Pegasus::PegasusEngine::initKeymaps();
}

Common::Error PegasusMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Pegasus::PegasusEngine(syst, (const Pegasus::PegasusGameDescription *)desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(PEGASUS)
	REGISTER_PLUGIN_DYNAMIC(PEGASUS, PLUGIN_TYPE_ENGINE, PegasusMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PEGASUS, PLUGIN_TYPE_ENGINE, PegasusMetaEngine);
#endif
