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

#include "common/system.h"
#include "common/savefile.h"
#include "common/algorithm.h"
#include "common/translation.h"

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "teenagent/resources.h"
#include "teenagent/teenagent.h"
#include "teenagent/detection.h"
#include "graphics/thumbnail.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

enum {
	MAX_SAVES = 20
};

#ifdef USE_TTS

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_TTS,
		{
			_s("Enable Text to Speech"),
			_s("Use TTS to read text in the game (if TTS is available)"),
			"tts_enabled",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

#endif

class TeenAgentMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "teenagent";
	}

	bool hasFeature(MetaEngineFeature f) const override {
		switch (f) {
		case kSupportsListSaves:
		case kSupportsDeleteSave:
		case kSupportsLoadingDuringStartup:
		case kSavesSupportMetaInfo:
		case kSavesSupportThumbnail:
			return true;
		default:
			return false;
		}
	}

#ifdef USE_TTS
	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}
#endif

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new TeenAgent::TeenAgentEngine(syst, desc);
		return Common::kNoError;
	}

	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (!target)
			target = getName();
		if (saveGameIdx == kSavegameFilePattern)
			return Common::String::format("%s.##", target);
		else
			return Common::String::format("%s.%02d", target, saveGameIdx);
	}

	SaveStateList listSaves(const char *target) const override {
		Common::String pattern = target;
		pattern += ".##";

		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);

		SaveStateList saveList;
		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			int slot = atoi(file->c_str() + file->size() - 2);
			if (slot >= 0 && slot < MAX_SAVES) {
				Common::ScopedPtr<Common::InSaveFile> in(g_system->getSavefileManager()->openForLoading(*file));
				if (!in)
					continue;

				char buf[25];
				in->seek(0);
				in->read(buf, 24);
				buf[24] = 0;
				saveList.push_back(SaveStateDescriptor(this, slot, buf));
			}
		}
		// Sort saves based on slot number.
		Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
		return saveList;
	}

	int getMaximumSaveSlot() const override {
		return MAX_SAVES - 1;
	}

	bool removeSaveState(const char *target, int slot) const override {
		return g_system->getSavefileManager()->removeSavefile(getSavegameFile(slot, target));
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Common::String filename = getSavegameFile(slot, target);
		Common::ScopedPtr<Common::InSaveFile> in(g_system->getSavefileManager()->openForLoading(filename));
		if (!in)
			return SaveStateDescriptor();

		char buf[25];
		in->seek(0);
		in->read(buf, 24);
		buf[24] = 0;

		Common::String desc = buf;

		in->seek(TeenAgent::saveStateSize);

		uint32 tag = in->readUint32BE();
		if (tag == MKTAG('T', 'N', 'G', 'T')) {
			// Skip save version
			in->skip(1);

			// Skip scene object data
			uint32 size = in->readUint32LE();
			in->skip(size);
		} else {
			in->seek(-4, SEEK_CUR);
		}

		if (!Graphics::checkThumbnailHeader(*in))
			return SaveStateDescriptor(this, slot, desc);

		SaveStateDescriptor ssd(this, slot, desc);

		//checking for the thumbnail
		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*in, thumbnail)) {
			return SaveStateDescriptor();
		}
		ssd.setThumbnail(thumbnail);

		return ssd;
	}

		Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::KeymapArray TeenAgentMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace TeenAgent;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "teenagent-default", _("Default keymappings"));

	Common::Action *act;

	act = new Common::Action(kStandardActionLeftClick, _("Move / Examine"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Common::Action(kStandardActionRightClick, _("Interact"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Common::Action("SKIPDLG", _("Skip dialog"));
	act->setCustomEngineActionEvent(kActionSkipDialog);
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	act = new Common::Action("CLOSEINV", _("Close inventory"));
	act->setCustomEngineActionEvent(kActionCloseInventory);
	act->addDefaultInputMapping("ESCAPE");
	engineKeyMap->addAction(act);

	act = new Common::Action("TOGGLEINV", _("Toggle inventory"));
	act->setCustomEngineActionEvent(kActionToggleInventory);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Common::Action("SKIPINTRO", _("Skip intro"));
	act->setCustomEngineActionEvent(kActionSkipIntro);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	// I18N: Speeds up the game to twice its normal speed
	act = new Common::Action("FASTMODE", _("Toggle fast mode"));
	act->setCustomEngineActionEvent(kActionFastMode);
	act->addDefaultInputMapping("C+f");
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(TEENAGENT)
	REGISTER_PLUGIN_DYNAMIC(TEENAGENT, PLUGIN_TYPE_ENGINE, TeenAgentMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TEENAGENT, PLUGIN_TYPE_ENGINE, TeenAgentMetaEngine);
#endif
