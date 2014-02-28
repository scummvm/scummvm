static const char *const kLoLSeqplayPakFilesDOSDemo[1] = {
	"GENERAL.PAK"
};

static const StringListProvider kLoLSeqplayPakFilesDOSDemoProvider = { ARRAYSIZE(kLoLSeqplayPakFilesDOSDemo), kLoLSeqplayPakFilesDOSDemo };

static const HoFSequence kLoLSeqplaySeqDataDOSDemoSeq[11] = {
	{ 0x0002, "", "scene1.cps", 3, 1, 0xFFFF, 0xFFFF, 0x0000, 0x0001, 0x00C8, 0, 0, 0x003C },
	{ 0x0004, "", "", 3, 0, 0x0000, 0x0001, 0x0000, 0x0001, 0x00C8, 0, 0, 0x0000 },
	{ 0x0003, "scene2.wsa", "scene2.cps", 4, 0, 0xFFFF, 0xFFFF, 0x0000, 0x0053, 0x0003, 0, 0, 0x001E },
	{ 0x0004, "", "", 4, 0, 0x0002, 0x0003, 0x0000, 0x0001, 0x00C8, 0, 0, 0x0000 },
	{ 0x0003, "scene3.wsa", "scene3.cps", 5, 0, 0xFFFF, 0xFFFF, 0x0000, 0x006B, 0x0005, 0, 0, 0x001E },
	{ 0x0004, "", "", 4, 0, 0x0004, 0x0005, 0x0000, 0x0001, 0x00C8, 0, 0, 0x0000 },
	{ 0x0003, "scene4.wsa", "scene4.cps", 4, 0, 0xFFFF, 0xFFFF, 0x0000, 0x0044, 0x000A, 0, 0, 0x0028 },
	{ 0x0004, "", "", 4, 0, 0x0006, 0x0007, 0x0000, 0x0001, 0x00C8, 0, 0, 0x0000 },
	{ 0x0003, "scene5.wsa", "scene5.cps", 4, 1, 0xFFFF, 0xFFFF, 0x0000, 0x0031, 0x0008, 0, 0, 0x000A },
	{ 0x0004, "", "", 3, 1, 0x0008, 0x0009, 0x0000, 0x0001, 0x00C8, 0, 0, 0x0000 },
	{ 0x0002, "", "scene1.cps", 3, 1, 0xFFFF, 0xFFFF, 0x0000, 0x0001, 0x0014, 0, 0, 0x012C }
};


static const HoFSequenceProvider kLoLSeqplaySeqDataDOSDemoProvider = { ARRAYSIZE(kLoLSeqplaySeqDataDOSDemoSeq), kLoLSeqplaySeqDataDOSDemoSeq, 0, NULL };

static const char *const kLoLSeqplaySfxFilesDOSDemo[18] = {
	"tclap1",
	"tclap2",
	"beeorc2",
	"tclap4",
	"tclap5",
	"scotia9",
	"bolt",
	"bolt2",
	"wokout1",
	"cdamage1",
	"wokmisc1",
	"mwhoa1",
	"mstdoom2",
	"loragasp",
	"ratwok",
	"swing1",
	"bludyell",
	""
};

static const StringListProvider kLoLSeqplaySfxFilesDOSDemoProvider = { ARRAYSIZE(kLoLSeqplaySfxFilesDOSDemo), kLoLSeqplaySfxFilesDOSDemo };

static const char *const kLoLSeqplayIntroTracksDOSDemo[3] = {
	"loredemo",
	"loredemo",
	""
};

static const StringListProvider kLoLSeqplayIntroTracksDOSDemoProvider = { ARRAYSIZE(kLoLSeqplayIntroTracksDOSDemo), kLoLSeqplayIntroTracksDOSDemo };

