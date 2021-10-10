#include "base/plugins.h"
#include "engines/advancedDetector.h"

namespace Freescape {
static const PlainGameDescriptor freescapeGames[] = {
	{"3dkit", "The 3D Kit Game"},
	{"driller", "Driller"},
	{"castle", "Castle Master"},
	{0, 0}};

static const ADGameDescription gameDescriptions[] = {
	{"3dkit",
	 "Example game",
	 AD_ENTRY1s("3DKIT.RUN", "f35147729a2f5b6852a504223aeb6a57", 112158),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},

	{"3dkit",
	 "Empty scene",
	 AD_ENTRY1s("EMPTY.RUN", "7e7238e12dd9da4336a77a58d1bcfc0e", 83184),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},

	{"3dkit",
	 "One cube in scene",
	 AD_ENTRY1s("CUBE.RUN", "3b7930be0f646b98885cfb70c26c89a2", 66138),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},

	{"Driller",
	 "",
	 AD_ENTRY1s("DRILLER.EXE", "cafc0ea0d3424640a7723af87f8bfc0b", 17427),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)},

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
