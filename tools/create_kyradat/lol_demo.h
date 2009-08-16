const ExtractEntry lolDemo[] = {
	{ k2SeqplayPakFiles, 0x0001AC10, 0x0001AC1C },
	{ k2SeqplayStrings, 0x0001B5EE, 0x0001B6F0 },
	{ k2SeqplaySfxFiles, 0x0001B6F0, 0x0001B7B5 },
	{ k2SeqplaySeqData, 0x0001B320, 0x0001B56C },
	{ kLolSeqplayIntroTracks, 0x0001B7B5, 0x0001B7CF },
	{ -1, 0, 0 }
};

const Game lolDemos[] = {
	{ kLol, EN_ANY, k2DemoLol, "30bb5af87d38adb47d3e6ce06b1cb042", lolDemo},
	GAME_DUMMY_ENTRY
};


