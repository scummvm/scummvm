const ExtractEntry kyra2Demo[] = {
	{ k2SeqplayPakFiles, 0x0001C130, 0x0001C144 },
	{ k2SeqplaySfxFiles, 0x0001CDDE, 0x0001CEBC },
	{ k2SeqplaySeqData, 0x0001C920, 0x0001CB9C },
	{ k2SeqplayIntroTracks, 0x0001CEBC, 0x0001CECC },
	{ k2SeqplayShapeAnimData, 0x0001CC14, 0x0001CCCC },
	{ -1, 0, 0 }
};

const Game kyra2Demos[] = {
	{ kKyra2, EN_ANY, k2DemoVersion, "a620a37579dd44ab0403482285e3897f", kyra2Demo},
	GAME_DUMMY_ENTRY
};
