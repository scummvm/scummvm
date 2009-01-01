const ExtractEntry kyra2Demo[] = {
	{ k2SeqplayPakFiles, 0x0001C130, 0x0001C144 },
	{ k2SeqplaySfxFiles, 0x0001CDDE, 0x0001CEBC },
	{ k2SeqplaySeqData, 0x0001C920, 0x0001CB9C },
	{ k2SeqplayIntroTracks, 0x0001CEBC, 0x0001CECC },
	{ k2SeqplayShapeAnimData, 0x0001CC14, 0x0001CCCC },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2DemoCDE[] = {
	{ k2IngameTlkDemoStrings, 0x0002F367, 0x0002F432 },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2DemoCDF[] = {
	{ k2IngameTlkDemoStrings, 0x0002F432, 0x0002F4F0 },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2DemoCDG[] = {
	{ k2IngameTlkDemoStrings, 0x0002F4F0, 0x0002F5CC },
	{ -1, 0, 0 }
};

const Game kyra2Demos[] = {
	{ kKyra2, EN_ANY, k2DemoVersion, "a620a37579dd44ab0403482285e3897f", kyra2Demo},
	{ kKyra2, EN_ANY, k2CDFile2E, "fa54d8abfe05f9186c05f7de7eaf1480", kyra2DemoCDE},
	{ kKyra2, FR_FRA, k2CDFile2F, "fa54d8abfe05f9186c05f7de7eaf1480", kyra2DemoCDF},
	{ kKyra2, DE_DEU, k2CDFile2G, "fa54d8abfe05f9186c05f7de7eaf1480", kyra2DemoCDG},
	GAME_DUMMY_ENTRY
};


