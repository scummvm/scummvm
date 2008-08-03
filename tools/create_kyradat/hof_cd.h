const ExtractEntry kyra2File1CDE[] = {
	{ k2SeqplayPakFiles, 0x00029FA4, 0x00029FEA },
	{ k2SeqplayCredits, 0x00027920, 0x00028408 },
	{ k2SeqplayCreditsSpecial, 0x00029E01, 0x00029E1B },
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

const ExtractEntry kyra2File1CDI[] = {
	{ k2SeqplayStrings, 0x0002C566, 0x0002CE7C },
	{ k2SeqplayTlkFiles, 0x0002A2AC, 0x0002A349 },
	{ -1, 0, 0 }
};

const ExtractEntry kyra2File2CDE[] = {
	{ k2IngameSfxFiles, 0x0002CB30, 0x0002D221 },
	{ k2IngameSfxIndex, 0x000294F0, 0x00029848 },
	{ k2IngameTracks, 0x0002F2FE, 0x0002F37E },
	{ k2IngameTalkObjIndex, 0x0002BAC2, 0x0002BB46 },
	{ k2IngameItemAnimData, 0x0002FE68, 0x000301B0 },
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

	// Italian Fan Translation (using same offsets as English) 
	{ kKyra2, IT_ITA, k2CDFile1I, "130795aa8f2333250c895dae9028b9bb", kyra2File1CDI},

	GAME_DUMMY_ENTRY
};

/// debug\kyra_Towns debug\kyra_amiga_en debug\kyra_dos_CD_en debug\kyra_dos_CD_de debug\kyra_dos_CD_fr debug\kyra_dos_CD_ita debug\kyra_dos_Floppy_En debug\kyra_dos_Floppy_dE debug\kyra_dos_Floppy_fr debug\kyra_dos_floppy_ita debug\kyra_dos_Floppy_spa debug\kyra_dos_demo debug\hof_dos_floppy_De_1 debug\hof_dos_floppy_De_2 debug\hof_dos_floppy_En_1 debug\hof_dos_floppy_En_2 debug\hof_dos_floppy_Fr_1 debug\hof_dos_floppy_Fr_2 debug\hof_dos_CD_1 debug\hof_dos_CD_2 debug\hof_towns_1 debug\hof_towns_2 debug\hof_dos_demo debug\lol_dos_demo debug\hof_dos_CD_demo debug\mr_dos_cd 