#include "dm/dm.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"

#include "engines/advancedDetector.h"
namespace DM {
static const PlainGameDescriptor DMGames[] = {
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {
	AD_TABLE_END_MARKER
};

static const ADExtraGuiOptionsMap optionsList[] = {
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};


class DMMetaEngine : public AdvancedMetaEngine {
public:


	DMMetaEngine() : AdvancedMetaEngine(DM::gameDescriptions, sizeof(ADGameDescription), DMGames, optionsList) {
		_singleId = "Dummy";
	}

	virtual const char *getName() const {
		return "Dummy";
	}

	virtual const char *getOriginalCopyright() const {
		return "Dummy";
	}

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const { return gameDescriptions; }
	virtual bool hasFeature(MetaEngineFeature f) const { return false; }
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		*engine = new DM::DMEngine(syst);
		return true;
	}
	virtual int getMaximumSaveSlot() const { return 99; }
	virtual SaveStateList listSaves(const char *target) const { return SaveStateList(); }
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const { return SaveStateDescriptor(); }
	virtual void removeSaveState(const char *target, int slot) const {}
};

}
#if PLUGIN_ENABLED_DYNAMIC(DM)
REGISTER_PLUGIN_DYNAMIC(DM, PLUGIN_TYPE_ENGINE, DM::DMMetaEngine);
#else
REGISTER_PLUGIN_STATIC(DM, PLUGIN_TYPE_ENGINE, DM::DMMetaEngine);
#endif
