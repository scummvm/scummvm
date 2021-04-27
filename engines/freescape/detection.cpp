#include "base/plugins.h"
#include "engines/advancedDetector.h"

namespace Freescape {
static const PlainGameDescriptor freescapeGames[] = {
	{"3dkit", "The 3D Kit Game"},
	{"3dkitcube", "The 3D Kit Game with only one cube"},
	{"driller", "Driller"},
	{"castle", "Castle Master"},
	{0, 0}};

static const ADGameDescription gameDescriptions[] = {
	{"3Dkit",
	 0,
	 AD_ENTRY1s("3DKIT.RUN", "f35147729a2f5b6852a504223aeb6a57", 112158),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},

	{"3Dkitcube",
	 "One cube",
	 AD_ENTRY1s("CUBE.RUN", "3b7930be0f646b98885cfb70c26c89a2", 66138),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},

	{"Driller",
	 0,
	 AD_ENTRY1s("DRILLE.EXE", "eb7e9e0acb72e30cf6e9ed20a6480e7a", 51944),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},

	{"Castle",
	 0,
	 AD_ENTRY1s("CME.EXE", "99d8b4dbaad1fd73c9afdde550dc5195", 92320),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},
	AD_TABLE_END_MARKER};
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
		return "Copyright (C) Incentive Software";
	}
};

REGISTER_PLUGIN_STATIC(FREESCAPE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, FreescapeMetaEngineDetection);
