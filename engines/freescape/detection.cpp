#include "base/plugins.h"
#include "engines/advancedDetector.h"

namespace Freescape {
static const PlainGameDescriptor freescapeGames[] = {
	{ "3dkitgame", "Quux the Example Module" },
	{ "quuxcd", "Quux the Example Module (CD version)" },
	{ 0, 0 }
};


static const ADGameDescription gameDescriptions[] = {
	{
		"3Dkitgame",
		0,
		AD_ENTRY1s("3DKIT.RUN", "f35147729a2f5b6852a504223aeb6a57", 112158),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	AD_TABLE_END_MARKER
};
} // End of namespace Freescape

class FreescapeMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	FreescapeMetaEngineDetection() : AdvancedMetaEngineDetection(Freescape::gameDescriptions, sizeof(ADGameDescription), Freescape::freescapeGames) {
	}	

	const char *getEngineId() const override {
		return "freescape";
	}

	const char *getName() const override {
		return "Freescape";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) TODO Entertainment Ltd.";
	}
};

REGISTER_PLUGIN_STATIC(FREESCAPE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, FreescapeMetaEngineDetection);
