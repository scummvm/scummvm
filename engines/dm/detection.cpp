#include "dm/dm.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"

#include "engines/advancedDetector.h"
namespace DM {
static const PlainGameDescriptor DMGames[] = {
	{"dm", "Dungeon Master"},
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {
	{
		"dm", "Amiga 2.0v English",
		{
		{"graphics.dat", 0, "6A2F135B53C2220F0251FA103E2A6E7E", 411960},
		{"Dungeon.dat", 0, "30028FB6A301ECB20127EF0B3AF32B05", 25006},
		AD_LISTEND
		},
	    Common::EN_GRB, Common::kPlatformAmiga, ADGF_NO_FLAGS, GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

static ADGameDescription fallbackDesc = {
	"dm",
	"Unknown version",
	AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
	Common::UNK_LANG,
	Common::kPlatformDOS,
	ADGF_NO_FLAGS,
	GUIO1(GUIO_NONE)
};


static const ADExtraGuiOptionsMap optionsList[] = {
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};


class DMMetaEngine : public AdvancedMetaEngine {
public:
	DMMetaEngine() : AdvancedMetaEngine(DM::gameDescriptions, sizeof(ADGameDescription), DMGames, optionsList) {
		_singleId = "dm";
	}

	virtual const char *getName() const {
		return "Dungeon Master";
	}

	virtual const char *getOriginalCopyright() const {
		return "Dummy";
	}

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const { return gameDescriptions; }
	virtual bool hasFeature(MetaEngineFeature f) const { return false; }
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		if(desc)
			*engine = new DM::DMEngine(syst);
		return desc != nullptr;
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
