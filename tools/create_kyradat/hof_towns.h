const ExtractEntry kyra2File1FMTownsE[] = {
	{ k2SeqplayPakFiles, 0x00000540, 0x00000573 },
	{ k2SeqplayStrings, 0x00001614, 0x00001FA4 },
	{ k2SeqplaySfxFiles, 0x0000284C, 0x000029EF },
	{ k2SeqplaySeqData, 0x000050D8, 0x00005794 },
	{ k2SeqplayIntroCDA, 0x00007FF8, 0x00008010 },
	{ k2SeqplayFinaleCDA, 0x00008010, 0x00008018 },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2File1FMTownsJ[] = {
	{ k2SeqplayStrings, 0x00001FA4, 0x0000284C },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2File2FMTownsE[] = {
	{ k2IngamePakFiles, 0x00000540, 0x0000065C },
	{ k2IngameSfxFilesTns, 0x00003E3C, 0x000047A3 },
	{ k2IngameSfxIndex, 0x00013980, 0x00013CD8 },
	{ k2IngameCDA, 0x0001808C, 0x000181BC },
	{ k2IngameTalkObjIndex, 0x000199B6, 0x00019A30 },
	{ k2IngameTimJpStrings, 0x0000677C, 0x000067A9 },
	{ k2IngameItemAnimData, 0x00012F28, 0x00013270 },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2File2FMTownsJ[] = {
	{ -1, 0, 0 }
};

const Game kyra2TownsGames[] = {
	{ kKyra2, EN_ANY, k2TownsFile1E, "74f50d79c919cc8e7196c24942ce43d7", kyra2File1FMTownsE},
	{ kKyra2, JA_JPN, k2TownsFile1J, "74f50d79c919cc8e7196c24942ce43d7", kyra2File1FMTownsJ},
	{ kKyra2, EN_ANY, k2TownsFile2E, "a9a7fd4f05d00090e9e8bda073e6d431", kyra2File2FMTownsE},
	{ kKyra2, JA_JPN, k2TownsFile2J, "a9a7fd4f05d00090e9e8bda073e6d431", kyra2File2FMTownsJ},
	GAME_DUMMY_ENTRY
};
