#include "base/plugins.h"
#include "engines/advancedDetector.h"

namespace Freescape {
static const PlainGameDescriptor freescapeGames[] = {
	{"3dkit", "The 3D Kit Game"},
	{"driller", "Driller"},
	{"spacestationoblivion", "Space Station Oblivion"},
	{"darkside", "Dark Side"},
	{"totaleclipse", "Total Eclipse"},
	{"castlemaster", "Castle Master"},
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
	{"3dkit",
	 "Simple example (cube)",
	 AD_ENTRY1s("CUBE.RUN", "3b7930be0f646b98885cfb70c26c89a2", 66138),
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
	{"driller",
	 "Driller",
	 {
		{"DRILLER.EXE", 0, "cafc0ea0d3424640a7723af87f8bfc0b", 17427},
		{"DRILLC.EXE", 0, "908dd1f8732ebcbaece7d8f0cffd8830", 43864},
		{"DRILLT.EXE", 0, "afce0e5e0ad8c508e0c31c3e0b18b9a5", 51096},
		{"DRILLE.EXE", 0, "eb7e9e0acb72e30cf6e9ed20a6480e7a", 51944},
		{"DRILLH.EXE", 0, "033d2d45390886d0bff78cf53e83b6ed", 47496},
		AD_LISTEND
	 },
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)},
	{"darkside",
	 "Dark Side",
	 {
		{"DARKSIDE.EXE", 0, "c6c0d0186ec45e6cecd72bf5550c7f98", 1600},
		{"DSIDEC.EXE", 0, "31e6c169d9270b6de8c1c2e746ac238e", 49504},
		{"DSIDEH.EXE", 0, "5e18e0746647bd04f43b9db24b6a036d", 53232},
		{"DSIDEE.EXE", 0, "524281f7d2dc49e0a41fcb1d38ee2559", 56800},
		AD_LISTEND
	 },
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)},
	{"totaleclipse",
	 "Total Eclipse",
	 {
		{"TOTAL.EXE", 0, "959703c1cd37b0d9744c492240a8178b", 13327},
		{"TOTEH.EXE", 0, "c68d59874ab2a93cc9cc1b1d3aed8f17", 60628},
		{"TOTEC.EXE", 0, "6c058c48255945a12479c8420321db75", 56900},
		{"TOTET.EXE", 0, "2bc2b0892ca91a77c58e9bedabf45afe", 63732},
		{"TOTEE.EXE", 0, "2ba865d2bdcda0c934baec7b31fb4ab2", 64196},
		AD_LISTEND
	 },
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)},
	{"spacestationoblivion",
	 "Space Station Oblivion",
	 {
		{"OBLIVION.EXE", 0, "80783622013750d7c88fd1d35dde919a", 6765},
		{"DRILLC.EXE", 0, "56394eae69f535cbddaa463888086ac6", 43864},
		{"DRILLE.EXE", 0, "30edf6be0037b2b0e8c6957df62c2a02", 51944},
		{"DRILLH.EXE", 0, "7f764048050e7a1b1f33aa466230edeb", 47496},
		AD_LISTEND
	 },
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 ADGF_NO_FLAGS,
	 GUIO3(GUIO_NOMIDI, GUIO_RENDEREGA, GUIO_RENDERCGA)},
	{"castlemaster",
	 "Castle Master",
	 {
		{"CASTLE.EXE", 0, "f1a141df0e47860246716db20d2ba061", 2806},
		{"CMC.EXE", 0, "03af2b79b1aad690684cf89025c5f425", 60240},
		{"CMT.EXE", 0, "a603d3f96e981ab0014b7b1a5a2dc28c", 81840},
		{"CME.EXE", 0, "99d8b4dbaad1fd73c9afdde550dc5195", 92320},
		{"CMH.EXE", 0, "1f3b67e649e718e239ebfd7c56e96d47", 63040},
		AD_LISTEND
	 },
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
