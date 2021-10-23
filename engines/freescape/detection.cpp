#include "base/plugins.h"
#include "engines/advancedDetector.h"

namespace Freescape {
static const PlainGameDescriptor freescapeGames[] = {
	{"3dkit", "The 3D Kit Game"},
	{"driller", "Driller"},
	{"castle", "Castle Master"},
	{"menace", "Menace of Dr. Spoil Sport"},
	{0, 0}};

static const ADGameDescription gameDescriptions[] = {
	{"3dkit",
	 "Simple example",
	 AD_ENTRY1s("HEX.RUN", "084c0314d0cd8e6d2287f30483a732eb", 83242),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},
	{"menace",
	 "",
	 AD_ENTRY1s("MODSS.RUN", "409ac1100a15447e742ec1415b2741c3", 91176),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},

	{"3dkit",
	 "Chrismas",
	 AD_ENTRY1s("CHRISTMA.RUN", "106b8f0dd0384d3138a8f0f62caef392", 69910),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},
	{"3dkit",
	 "Maze",
	 AD_ENTRY1s("DESMAZE.RUN", "5cfab15e53d77029bdb02c87acae3186", 99212),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},
	{"3dkit",
	 "Mountain",
	 AD_ENTRY1s("MOUNTAIN.RUN", "ec3bb57fe23b1a6785e870af1baa74d7", 129106),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO1(GUIO_NOMIDI)},
	{"3dkit",
	 "Example game",
	 AD_ENTRY1s("3DKIT.RUN", "f35147729a2f5b6852a504223aeb6a57", 112158),
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
