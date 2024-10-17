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

#include "glk/glk.h"
#include "glk/detection.h"
#include "glk/quetzal.h"
#include "glk/adrift/detection.h"
#include "glk/adrift/adrift.h"
#include "glk/advsys/detection.h"
#include "glk/advsys/advsys.h"
#include "glk/agt/detection.h"
#include "glk/agt/agt.h"
#include "glk/alan2/detection.h"
#include "glk/alan2/alan2.h"
#include "glk/alan3/detection.h"
#include "glk/alan3/alan3.h"
#include "glk/archetype/archetype.h"
#include "glk/archetype/detection.h"
#include "glk/comprehend/comprehend.h"
#include "glk/comprehend/detection.h"
#include "glk/zcode/detection.h"
#include "glk/zcode/zcode.h"
#include "glk/glulx/detection.h"
#include "glk/glulx/glulx.h"
#include "glk/hugo/detection.h"
#include "glk/hugo/hugo.h"
#include "glk/jacl/detection.h"
#include "glk/jacl/jacl.h"
#include "glk/level9/detection.h"
#include "glk/level9/level9.h"
#include "glk/magnetic/detection.h"
#include "glk/magnetic/magnetic.h"
#include "glk/quest/detection.h"
#include "glk/quest/quest.h"
#include "glk/scott/detection.h"
#include "glk/scott/scott.h"

#ifndef RELEASE_BUILD
#include "glk/tads/detection.h"
#include "glk/tads/tads2/tads2.h"
#include "glk/tads/tads3/tads3.h"
#endif

#include "base/plugins.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/surface.h"
#include "common/config-manager.h"
#include "common/file.h"

#define MAX_SAVES 99

class GlkMetaEngine : public MetaEngine {
public:
	const char* getName() const override {
		return "glk";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine,
	                             const DetectedGame &gameDescriptor, const void *metaEngineDescriptor) override;

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool GlkMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportCreationDate) ||
	    (f == kSavesSupportPlayTime) ||
	    (f == kSimpleSavesNames);
}

bool Glk::GlkEngine::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsReturnToLauncher) ||
	    (f == kSupportsLoadingDuringRuntime) ||
	    (f == kSupportsSavingDuringRuntime);
}

bool isGameAllowed(GameSupportLevel supportLevel) {
	bool showTestingWarning = false;
#ifdef RELEASE_BUILD
	showTestingWarning = true;
#endif

	if (((supportLevel == kUnstableGame
		|| (supportLevel == kTestingGame && showTestingWarning)))
		&& !Engine::warnUserAboutUnsupportedGame())
		return false;

	return true;
}

template<class META, class ENG>bool create(OSystem *syst,
		Glk::GlkGameDescription &gameDesc, Engine *&engine) {

	Glk::GameDescriptor gd = META::findGame(gameDesc._gameId.c_str());
	if (gd._description) {
		if (!isGameAllowed(gd._supportLevel))
			return true;

		gameDesc._options = gd._options;
		engine = new ENG(syst, gameDesc);
		return true;
	} else {
		return false;
	}
}

Common::Error GlkMetaEngine::createInstance(OSystem *syst, Engine **engine,
		const DetectedGame &gameDescriptor, const void *metaEngineDescriptor) {
#ifndef RELEASE_BUILD
	Glk::GameDescriptor td = Glk::GameDescriptor::empty();
#endif
	assert(engine);

	Glk::GlkGameDescription *gameDesc = static_cast<Glk::GlkGameDescription *>(
			const_cast<void *>(metaEngineDescriptor));
	assert(gameDesc);

	// Create the correct engine
	*engine = nullptr;
	if ((create<Glk::Adrift::AdriftMetaEngine, Glk::Adrift::Adrift>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::AdvSys::AdvSysMetaEngine, Glk::AdvSys::AdvSys>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::AGT::AGTMetaEngine, Glk::AGT::AGT>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::Alan2::Alan2MetaEngine, Glk::Alan2::Alan2>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::Alan3::Alan3MetaEngine, Glk::Alan3::Alan3>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::Archetype::ArchetypeMetaEngine, Glk::Archetype::Archetype>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::Comprehend::ComprehendMetaEngine, Glk::Comprehend::Comprehend>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::Glulx::GlulxMetaEngine, Glk::Glulx::Glulx>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::Hugo::HugoMetaEngine, Glk::Hugo::Hugo>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::JACL::JACLMetaEngine, Glk::JACL::JACL>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::Level9::Level9MetaEngine, Glk::Level9::Level9>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::Magnetic::MagneticMetaEngine, Glk::Magnetic::Magnetic>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::Quest::QuestMetaEngine, Glk::Quest::Quest>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::Scott::ScottMetaEngine, Glk::Scott::Scott>(syst, *gameDesc, *engine))) {}
	else if ((create<Glk::ZCode::ZCodeMetaEngine, Glk::ZCode::ZCode>(syst, *gameDesc, *engine))) {}
#ifndef RELEASE_BUILD
	else if ((td = Glk::TADS::TADSMetaEngine::findGame(gameDesc->_gameId.c_str()))._description) {
		if (!isGameAllowed(td._supportLevel))
			return Common::kUserCanceled;
		else if (td._options & Glk::TADS::OPTION_TADS3)
			new Glk::TADS::TADS3::TADS3(syst, *gameDesc);
		else
			new Glk::TADS::TADS2::TADS2(syst, *gameDesc);
	}
#endif
	else {
		delete gameDesc;
		return Common::kNoGameDataFoundError;
	}

	// gameDesc is copied in Glk
	delete gameDesc;
	return *engine ? Common::kNoError : Common::kUserCanceled;
}

const ExtraGuiOptions GlkMetaEngine::getExtraGuiOptions(const Common::String &) const {
	ExtraGuiOptions  options;
#if defined(USE_TTS)
	static const ExtraGuiOption ttsSpeakOptions = {
		_s("Enable Text to Speech"),
		_s("Use TTS to read the text"),
		"speak",
		false,
	        0,
		0
	};
	static const ExtraGuiOption ttsSpeakInputOptions = {
		_s("Also read input text"),
		_s("Use TTS to read the input text"),
		"speak_input",
		false,
		0,
		0
	};
	options.push_back(ttsSpeakOptions);
	options.push_back(ttsSpeakInputOptions);
#endif
	return options;
}

SaveStateList GlkMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot <= MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				Common::String saveName;
				if (Glk::QuetzalReader::getSavegameDescription(in, saveName))
					saveList.push_back(SaveStateDescriptor(this, slot, saveName));

				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int GlkMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

bool GlkMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor GlkMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename);
	SaveStateDescriptor ssd;
	bool result = false;

	if (in) {
		result = Glk::QuetzalReader::getSavegameMetaInfo(in, ssd);
		ssd.setSaveSlot(slot);
		delete in;
	}

	if (result)
		return ssd;

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(GLK)
	REGISTER_PLUGIN_DYNAMIC(GLK, PLUGIN_TYPE_ENGINE, GlkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GLK, PLUGIN_TYPE_ENGINE, GlkMetaEngine);
#endif
