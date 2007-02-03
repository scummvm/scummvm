const ExtractEntry kyra1EspFloppy[] = {
	{ kKallakWritingSeq, 0x00030462, 0x00030C57 },
	{ kMalcolmTreeSeq, 0x00030C57, 0x00030E6D },
	{ kWestwoodLogoSeq, 0x00030E6D, 0x00030EB8 },
	{ kKyrandiaLogoSeq, 0x00030EB8, 0x00030F14 },
	{ kKallakMalcolmSeq, 0x00030F14, 0x0003117F },
	{ kForestSeq, 0x0003117F, 0x00031314 },
	{ kIntroCPSStrings, 0x00031314, 0x00031328 },
	{ kIntroWSAStrings, 0x00031328, 0x000313A8 },
	{ kIntroCOLStrings, 0x000313A8, 0x000313D3 },
	{ kIntroStrings, 0x000313D3, 0x000319A2 },
	{ kItemNames, 0x0003467A, 0x00034BAA },
	{ kTakenStrings, 0x00033F14, 0x00033F28 },
	{ kPlacedStrings, 0x00033F28, 0x00033F35 },
	{ kDroppedStrings, 0x00033F5E, 0x00033F66 },
	{ kNoDropStrings, 0x00033ED6, 0x00033F13 },
	{ kPutDownString, 0x0002F100, 0x0002F12D },
	{ kWaitAmuletString, 0x0002F12D, 0x0002F16F },
	{ kBlackJewelString, 0x0002F16F, 0x0002F194 },
	{ kHealingTipString, 0x0002F194, 0x0002F1BC },
	{ kPoisonGoneString, 0x0002F22C, 0x0002F25F },
	{ kThePoisonStrings, 0x0002FEC6, 0x0002FF1F },
	{ kFluteStrings, 0x00032361, 0x000323B3 },
	{ kWispJewelStrings, 0x0002F1BC, 0x0002F21B },
	{ kMagicJewelStrings, 0x0002F21B, 0x0002F22C },
	{ kFlaskFullString, 0x0002FFDA, 0x0002FFF5 },
	{ kFullFlaskString, 0x0002FF40, 0x0002FFDA },
	{ kOutroHomeString, 0x000320A8, 0x000320AD },
	{ kVeryCleverString, 0x00032004, 0x0003203A },
	{ kGUIStrings, 0x0002f3d0, 0x0002f60a },
	{ kNewGameString, 0x00032b62, 0x00032b7d },
	{ kConfigStrings, 0x0002fe50, 0x0002fe9a },
	{ -1, 0, 0 }
};

const Game kyra1EspGames[] = {
	{ kKyra1, ES_ESP, -1, "9ff130d2558bcd674d4074849d93c362", kyra1EspFloppy },
	GAME_DUMMY_ENTRY
};
