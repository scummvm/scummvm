static const char *const k2SeqplayPakFilesDOSDemo[2] = {
	"GENERAL.PAK",
	"VOC.PAK"
};

static const StringListProvider k2SeqplayPakFilesDOSDemoProvider = { ARRAYSIZE(k2SeqplayPakFilesDOSDemo), k2SeqplayPakFilesDOSDemo };

static const HoFSequence k2SeqplaySeqDataDOSDemoSeq[8] = {
	{ 0x0002, "", "virgin.cps", 4, 0, 0xFFFF, 0xFFFF, 0x0000, 0x0001, 0x0064, 0, 0, 0x001E },
	{ 0x0001, "westwood.wsa", "", 4, 0, 0xFFFF, 0xFFFF, 0x0000, 0x0012, 0x000C, 0, 0, 0x001E },
	{ 0x0001, "title.wsa", "", 4, 0, 0xFFFF, 0xFFFF, 0x0000, 0x001A, 0x0006, 0, 0, 0x0064 },
	{ 0x0001, "hill.wsa", "", 4, 0, 0xFFFF, 0xFFFF, 0x0000, 0x0082, 0x0007, 0, 0, 0x001E },
	{ 0x0003, "outhome.wsa", "outhome.cps", 4, 0, 0xFFFF, 0xFFFF, 0x0000, 0x009A, 0x0005, 0, 0, 0x001E },
	{ 0x0001, "wharf.wsa", "", 5, 0, 0xFFFF, 0xFFFF, 0x0000, 0x001C, 0x000A, 0, 0, 0x001E },
	{ 0x0001, "dinob.wsa", "", 5, 0, 0xFFFF, 0xFFFF, 0x0000, 0x0004, 0x0008, 0, 0, 0x001E },
	{ 0x0002, "", "fisher.cps", 4, 0, 0xFFFF, 0xFFFF, 0x0000, 0x0001, 0x0FA0, 0, 0, 0x0004 }
};

static const HoFNestedSequence k2SeqplaySeqDataDOSDemoSeqNested[5] = {
	{ 0x0002, "wharf2.wsa", 0, 70, 1, 0, 0, 0, NULL, 0, 0 },
	{ 0x0002, "dinob2.wsa", 0, 80, 2, 0, 0, 0, NULL, 0, 0 },
	{ 0x0004, "water.wsa", 0, 11, 4, 0, 0, 0, NULL, 0, 0 },
	{ 0x0004, "bail.wsa", 0, 16, 24, 0, 0, 0, NULL, 0, 0 },
	{ 0x0004, "dig.wsa", 0, 12, 18, 0, 0, 0, NULL, 0, 0 }
};

static const HoFSequenceProvider k2SeqplaySeqDataDOSDemoProvider = { ARRAYSIZE(k2SeqplaySeqDataDOSDemoSeq), k2SeqplaySeqDataDOSDemoSeq, ARRAYSIZE(k2SeqplaySeqDataDOSDemoSeqNested), k2SeqplaySeqDataDOSDemoSeqNested };

static const char *const k2SeqplaySfxFilesDOSDemo[20] = {
	"bigslam",
	"clothes1",
	"doormtop",
	"fngrsnap",
	"frogmix2",
	"frogsigh",
	"h2odrop2",
	"orcmove",
	"rowboat1",
	"squish1",
	"stretch1",
	"ocean1",
	"splash1",
	"trex1",
	"trex3",
	"trex5",
	"zno2",
	"big",
	"dreamer",
	""
};

static const StringListProvider k2SeqplaySfxFilesDOSDemoProvider = { ARRAYSIZE(k2SeqplaySfxFilesDOSDemo), k2SeqplaySfxFilesDOSDemo };

static const char *const k2SeqplayIntroTracksDOSDemo[2] = {
	"k2_demo",
	"k2_demo"
};

static const StringListProvider k2SeqplayIntroTracksDOSDemoProvider = { ARRAYSIZE(k2SeqplayIntroTracksDOSDemo), k2SeqplayIntroTracksDOSDemo };

static const HoFSeqItemAnimData k2SeqplayShapeAnimDataDOSDemo[4] = {
	{   0,  67,
	  { 0x0000, 0x0001, 0x0002, 0x0003, 0x0002, 0x0001, 0x0000, 0x0001, 0x0002, 0x0003,
	    0x0002, 0x0001, 0x0000, 0x0001, 0x0002, 0x0003, 0x0002, 0x0001, 0x0000, 0x0000 } },
	{   4, 109,
	  { 0x0000, 0x0001, 0x0002, 0x0003, 0x0000, 0x0001, 0x0002, 0x0003, 0x0000, 0x0001,
	    0x0002, 0x0003, 0x0000, 0x0001, 0x0002, 0x0003, 0x0000, 0x0001, 0x0002, 0x0003 } },
	{   8, 145,
	  { 0x0000, 0x0001, 0x0002, 0x0003, 0x0003, 0x0002, 0x0001, 0x0000, 0x0000, 0x0000,
	    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 } },
	{  12, 180,
	  { 0x0000, 0x0001, 0x0002, 0x0003, 0x0000, 0x0001, 0x0002, 0x0003, 0x0000, 0x0001,
	    0x0002, 0x0003, 0x0000, 0x0001, 0x0002, 0x0003, 0x0000, 0x0001, 0x0002, 0x0003 } }
};

static const HoFSeqItemAnimDataProvider k2SeqplayShapeAnimDataDOSDemoProvider = { ARRAYSIZE(k2SeqplayShapeAnimDataDOSDemo), k2SeqplayShapeAnimDataDOSDemo };

