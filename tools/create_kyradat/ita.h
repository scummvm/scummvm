const ExtractEntry kyra1ItaFloppy[] = {
	{ kKallakWritingSeq, 0x000304fa, 0x00030cef },
	{ kMalcolmTreeSeq, 0x00030cef, 0x00030F05 },
	{ kWestwoodLogoSeq, 0x00030F05, 0x00030F50 },
	{ kKyrandiaLogoSeq, 0x00030F50, 0x00030FAC },
	{ kKallakMalcolmSeq, 0x00030FAC, 0x00031217 },
	{ kForestSeq, 0x00031217, 0x000313ac },
	{ kIntroCPSStrings, 0x000313AC, 0x000313C0 },
	{ kIntroWSAStrings, 0x000313C0, 0x00031440 },
	{ kIntroCOLStrings, 0x00031440, 0x0003146B },
	{ kIntroStrings, 0x0003146B, 0x000319E7 },
	{ kItemNames, 0x000346DC, 0x00034C6D },
	{ kTakenStrings, 0x00033F60, 0x00033F74 },
	{ kPlacedStrings, 0x00033F74, 0x00033F81 },
	{ kDroppedStrings, 0x00033FAA, 0x00033FB5 },
	{ kNoDropStrings, 0x00033F1A, 0x00033F5F },
	{ kPutDownString, 0x0002f120, 0x0002f154 },
	{ kWaitAmuletString, 0x0002f154, 0x0002f19e },
	{ kBlackJewelString, 0x0002f19e, 0x0002f1c0 },
	{ kHealingTipString, 0x0002f1c0, 0x0002f1ff },
	{ kPoisonGoneString, 0x0002f290, 0x0002f2c4 },
	{ kThePoisonStrings, 0x0002ff20, 0x0002ff83 },
	{ kFluteStrings, 0x000323ab, 0x000323f7 },
	{ kWispJewelStrings, 0x0002f1ff, 0x0002f273 },
	{ kMagicJewelStrings, 0x0002f273, 0x0002f290 },
	{ kFlaskFullString, 0x00030059, 0x0003007a },
	{ kFullFlaskString, 0x0002ffa4, 0x00030059 },
	{ kOutroHomeString, 0x000320f2, 0x000320f7 },
	{ kVeryCleverString, 0x0003204c, 0x00032084 },
	{ kGUIStrings, 0x0002f434, 0x0002f659 },
	{ kNewGameString, 0x00032ba6, 0x00032bc2 },
	{ kConfigStrings, 0x0002fe9f, 0x0002fef3 },
	{ -1, 0, 0 }
};

const Game kyra1ItaGames[] = {
	{ kKyra1, IT_ITA, -1, "5d7550306b369a3492f9f3402702477c", kyra1ItaFloppy },
	GAME_DUMMY_ENTRY
};
