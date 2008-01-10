const ExtractEntry kyra2File1CDE[] = {
	{ k2SeqplayPakFiles, 0x00029FA4, 0x00029FEA },
	{ k2SeqplayCredits, 0x00027920, 0x00028408 },
	{ k2SeqplayStrings, 0x0002C566, 0x0002CE7C },
	{ k2SeqplaySfxFiles, 0x0002E284, 0x0002E4B8 },
	{ k2SeqplayTlkFiles, 0x0002A2AC, 0x0002A349 },
	{ k2SeqplayIntroTracks, 0x0002E4C5, 0x0002E4CD },
	{ k2SeqplayFinaleTracks, 0x0002E4DA, 0x0002E4E3 },
	{ k2SeqplaySeqData, 0x0002BB4C, 0x0002C20A },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2File1CDF[] = {
	{ k2SeqplayStrings, 0x0002CE7C, 0x0002D845 },
	{ k2SeqplayTlkFiles, 0x0002A349, 0x0002A3E6 },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2File1CDG[] = {
	{ k2SeqplayStrings, 0x0002D845, 0x0002E284 },
	{ k2SeqplayTlkFiles, 0x0002A3E6, 0x0002A483 },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2File2CDE[] = {
	{ k2IngameTracks, 0x0002F2FE, 0x0002F37E },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2File2CDF[] = {
	{ -1, 0, 0 }
};

const ExtractEntry kyra2File2CDG[] = {
	{ -1, 0, 0 }
};

const Game kyra2TalkieGames[] = {
	{ kKyra2, EN_ANY, k2CDFile1E, "85bbc1cc6c4cef6ad31fc6ee79518efb", kyra2File1CDE},
	{ kKyra2, FR_FRA, k2CDFile1F, "85bbc1cc6c4cef6ad31fc6ee79518efb", kyra2File1CDF},
	{ kKyra2, DE_DEU, k2CDFile1G, "85bbc1cc6c4cef6ad31fc6ee79518efb", kyra2File1CDG},
	{ kKyra2, EN_ANY, k2CDFile2E, "e20d0d2e500f01e399ec588247a7e213", kyra2File2CDE},
	{ kKyra2, FR_FRA, k2CDFile2F, "e20d0d2e500f01e399ec588247a7e213", kyra2File2CDF},
	{ kKyra2, DE_DEU, k2CDFile2G, "e20d0d2e500f01e399ec588247a7e213", kyra2File2CDG},
	GAME_DUMMY_ENTRY
};
