#include "base/plugins.h"
#include "engines/advancedDetector.h"

namespace Private {
static const PlainGameDescriptor privateGames[] = {
	{ "private-eye", "Private Eye" },
	{ 0, 0 }
};


static const ADGameDescription gameDescriptions[] = {
	{
		"private-eye",
		0,
		AD_ENTRY1s("game.dat", "7a3eb7d9dedf40680ac3d088524f976e", 190735),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	AD_TABLE_END_MARKER
};
} // End of namespace Private

class PrivateMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	PrivateMetaEngineDetection() : AdvancedMetaEngineDetection(Private::gameDescriptions, sizeof(ADGameDescription), Private::privateGames) {
	}	

	const char *getEngineId() const override {
		return "private";
	}

	const char *getName() const override {
		return "Private Eye";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) Brooklyn Multimedia";
	}
};

REGISTER_PLUGIN_STATIC(PRIVATE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PrivateMetaEngineDetection);
