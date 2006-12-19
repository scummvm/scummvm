#define ITE_CONVERSE_MAX_TEXT_WIDTH (256 - 60)
#define ITE_CONVERSE_TEXT_HEIGHT        10
#define ITE_CONVERSE_TEXT_LINES     4

//TODO: ihnm
#define IHNM_CONVERSE_MAX_TEXT_WIDTH (256 - 60)
#define IHNM_CONVERSE_TEXT_HEIGHT       10
#define IHNM_CONVERSE_TEXT_LINES        10

// ITE section
static PanelButton ITE_MainPanelButtons[] = {
	{kPanelButtonVerb,		52,4,	57,10,	kVerbITEWalkTo,'w',0,	0,1,0},
	{kPanelButtonVerb,		52,15,	57,10,	kVerbITELookAt,'l',0,	2,3,0},
	{kPanelButtonVerb,		52,26,	57,10,	kVerbITEPickUp,'p',0,	4,5,0},
	{kPanelButtonVerb,		52,37,	57,10,	kVerbITETalkTo,'t',0,	0,1,0},
	{kPanelButtonVerb,		110,4,	56,10,	kVerbITEOpen,'o',0,	6,7,0},
	{kPanelButtonVerb,		110,15,	56,10,	kVerbITEClose,'c',0,	8,9,0},
	{kPanelButtonVerb,		110,26,	56,10,	kVerbITEUse,'u',0,		10,11,0},
	{kPanelButtonVerb,		110,37,	56,10,	kVerbITEGive,'g',0,	12,13,0},
	{kPanelButtonArrow,		306,6,	8,5,	-1,'U',0,			0,4,2},
	{kPanelButtonArrow,		306,41,	8,5,	1,'D',0,			1,5,3},

	{kPanelButtonInventory,	181 + 32*0,6,	27,18,	0,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*1,6,	27,18,	1,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*2,6,	27,18,	2,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*3,6,	27,18,	3,'-',0,	0,0,0},

	{kPanelButtonInventory,	181 + 32*0,27,	27,18,	4,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*1,27,	27,18,	5,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*2,27,	27,18,	6,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*3,27,	27,18,	7,'-',0,	0,0,0}
};

static PanelButton ITE_ConversePanelButtons[] = {
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 0, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	0,'1',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 1, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	1,'2',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 2, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	2,'3',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 3, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	3,'4',0,	0,0,0},
	{kPanelButtonArrow,			257,6,	9,6,	-1,'u',0,	0,4,2},
	{kPanelButtonArrow,			257,41,	9,6,	1,'d',0,	1,5,3},
};

static PanelButton ITE_OptionPanelButtons[] = {
	{kPanelButtonOptionSlider,	284,19, 13,75,	0,'-',0,	0,0,0}, //slider-scroller
	{kPanelButtonOption,	113,18, 45,17,	kTextReadingSpeed,'r',0,	0,0,0}, //read speed
	{kPanelButtonOption,	113,37, 45,17,	kTextMusic,'m',0,	0,0,0}, //music
	{kPanelButtonOption,	113,56, 45,17,	kTextSound,'n',0,	0,0,0}, //sound-noise
	{kPanelButtonOption,	13,79, 135,17,	kTextQuitGame,'q',0,	0,0,0}, //quit
	{kPanelButtonOption,	13,98, 135,17,	kTextContinuePlaying,'c',0,	0,0,0}, //continue
	{kPanelButtonOption,	164,98, 57,17,	kTextLoad,'l',0,	0,0,0}, //load
	{kPanelButtonOption,	241,98, 57,17,	kTextSave,'s',0,	0,0,0},	//save
	{kPanelButtonOptionSaveFiles,	166,20, 112,74,	0,'-',0,	0,0,0},	//savefiles

	{kPanelButtonOptionText,106,4, 0,0,	kTextGameOptions,'-',0,	0,0,0},	// text: game options
	{kPanelButtonOptionText,11,22, 0,0,	kTextReadingSpeed,'-',0, 0,0,0},	// text: read speed
	{kPanelButtonOptionText,28,22, 0,0,	kTextShowDialog,'-',0, 0,0,0},	// text: read speed
	{kPanelButtonOptionText,73,41, 0,0,	kTextMusic,'-',0, 0,0,0},	// text: music
	{kPanelButtonOptionText,69,60, 0,0,	kTextSound,'-',0, 0,0,0},	// text: noise
};

static PanelButton ITE_QuitPanelButtons[] = {
	{kPanelButtonQuit, 11,17, 60,16, kTextQuit,'q',0, 0,0,0},
	{kPanelButtonQuit, 121,17, 60,16, kTextCancel,'c',0, 0,0,0},
	{kPanelButtonQuitText, -1,5, 0,0, kTextQuitTheGameQuestion,'-',0, 0,0,0},
};

static PanelButton ITE_LoadPanelButtons[] = {
	{kPanelButtonLoad, 101,19, 60,16, kTextOK,'o',0, 0,0,0},
	{kPanelButtonLoadText, -1,5, 0,0, kTextLoadSuccessful,'-',0, 0,0,0},
};

static PanelButton ITE_SavePanelButtons[] = {
	{kPanelButtonSave, 11,37, 60,16, kTextSave,'s',0, 0,0,0},
	{kPanelButtonSave, 101,37, 60,16, kTextCancel,'c',0, 0,0,0},
	{kPanelButtonSaveEdit, 26,17, 119,17, 0,'-',0, 0,0,0},
	{kPanelButtonSaveText, -1,5, 0,0, kTextEnterSaveGameName,'-',0, 0,0,0},
};

static PanelButton ITE_ProtectPanelButtons[] = {
	{kPanelButtonProtectEdit, 26,17, 119,17, 0,'-',0, 0,0,0},
	{kPanelButtonProtectText, -1,5, 0,0, kTextEnterProtectAnswer,'-',0, 0,0,0},
};

/*
static PanelButton ITE_ProtectionPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};*/

static const GameDisplayInfo ITE_DisplayInfo = {
	320, 200,		// logical width&height

	35,				// scene path y offset
	137,			// scene height

	0,				// status x offset
	137,			// status y offset
	320,			// status width
	11,				// status height
	2,				// status text y offset
	186,			// status text color
	15,				// status BG color
	308,137,		// save reminder pos
	12,12,			// save reminder w & h
	6,7,			// save reminder sprite numbers

	5, 4,			// left portrait x, y offset
	274, 4,			// right portrait x, y offset

	8, 9,			// inventory Up & Down button indexies
	2, 4,			// inventory rows, columns

	0, 148,			// main panel offsets
	ARRAYSIZE(ITE_MainPanelButtons),
	ITE_MainPanelButtons,

	ITE_CONVERSE_MAX_TEXT_WIDTH,
	ITE_CONVERSE_TEXT_HEIGHT,
	ITE_CONVERSE_TEXT_LINES,
	4, 5,			// converse Up & Down button indexies
	0, 148,			// converse panel offsets
	ARRAYSIZE(ITE_ConversePanelButtons),
	ITE_ConversePanelButtons,

	8, 0,			// save file index
	8,				// optionSaveFileVisible
	8, 8,			// option panel offsets
	ARRAYSIZE(ITE_OptionPanelButtons),
	ITE_OptionPanelButtons,

	64,54,			// quit panel offsets
	192,38,			// quit panel width & height
	ARRAYSIZE(ITE_QuitPanelButtons),
	ITE_QuitPanelButtons,

	74, 53,			// load panel offsets
	172, 40,		// load panel width & height
	ARRAYSIZE(ITE_LoadPanelButtons),
	ITE_LoadPanelButtons,

	2,				// save edit index
	74, 44,			// save panel offsets
	172, 58,		// save panel width & height
	ARRAYSIZE(ITE_SavePanelButtons),
	ITE_SavePanelButtons,

	0,				// protect edit index
	74, 44,			// protect panel offsets
	172, 58,		// protect panel width & height
	ARRAYSIZE(ITE_ProtectPanelButtons),
	ITE_ProtectPanelButtons
};

static const GameResourceDescription ITE_Resources = {
	RID_ITE_SCENE_LUT,  // Scene lookup table RN
	RID_ITE_SCRIPT_LUT, // Script lookup table RN
	RID_ITE_MAIN_PANEL,
	RID_ITE_CONVERSE_PANEL,
	RID_ITE_OPTION_PANEL,
	RID_ITE_MAIN_SPRITES,
	RID_ITE_MAIN_PANEL_SPRITES,
	RID_ITE_DEFAULT_PORTRAITS,
	RID_ITE_MAIN_STRINGS,
	RID_ITE_ACTOR_NAMES
};

static const GameResourceDescription ITEDemo_Resources = {
	RID_ITEDEMO_SCENE_LUT,  // Scene lookup table RN
	RID_ITEDEMO_SCRIPT_LUT, // Script lookup table RN
	RID_ITEDEMO_MAIN_PANEL,
	RID_ITEDEMO_CONVERSE_PANEL,
	RID_ITEDEMO_OPTION_PANEL,
	RID_ITEDEMO_MAIN_SPRITES,
	RID_ITEDEMO_MAIN_PANEL_SPRITES,
	RID_ITEDEMO_DEFAULT_PORTRAITS,
	RID_ITEDEMO_MAIN_STRINGS,
	RID_ITEDEMO_ACTOR_NAMES
};

// Inherit the Earth - DOS Demo version
static const ADGameFileDescription ITE_DEMO_G_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,					"986c79c4d2939dbe555576529fd37932"},
	//{"ite.dmo",	GAME_DEMOFILE},						"0b9a70eb4e120b6f00579b46c8cae29e"
	{"scripts.rsc", GAME_SCRIPTFILE,					"d5697dd3240a3ceaddaa986c47e1a2d7"},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c58e67c506af4ffa03fd0aac2079deb0"},
	{ NULL, 0, NULL}
};

static const GameFontDescription ITEDEMO_GameFonts[] = {
	{0},
	{1}
};

static const GameSoundInfo ITEDEMO_GameSound = {
	kSoundVOC,
	-1,
	-1,
	false,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Win32 Demo version

static const ADGameFileDescription ITE_WINDEMO2_GameFiles[] = {
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac"},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb"},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"95a6c148e22e99a8c243f2978223583c"},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription ITE_WINDEMO1_GameFiles[] = {
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac"},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb"},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"a741139dd7365a13f463cd896ff9969a"},
	{"voicesd.rsc",		GAME_VOICEFILE,		"0759eaf5b64ae19fd429920a70151ad3"},
	{ NULL, 0, NULL}
};

static const GameFontDescription ITEWINDEMO_GameFonts[] = {
	{2},
	{0}
};

static const GameSoundInfo ITEWINDEMO1_GameSound = {
	kSoundPCM,
	22050,
	8,
	false,
	false,
	false
};

static const GameSoundInfo ITEWINDEMO2_GameVoice = {
	kSoundVOX,
	22050,
	16,
	false,
	false,
	true
};

static const GameSoundInfo ITEWINDEMO2_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Mac Demo version
static const ADGameFileDescription ITE_MACDEMO2_GameFiles[] = {
	{"ited.rsc",		GAME_RESOURCEFILE,	"addfc9d82bc2fa1f4cab23743c652c08"},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"fded5c59b8b7c5976229f960d21e6b0b"},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"b3a831fbed337d1f1300fee1dd474f6c"},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4"},
	{"musicd.rsc",		GAME_MUSICFILE,		"495bdde51fd9f4bea2b9c911091b1ab2"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription ITE_MACDEMO1_GameFiles[] = {
	{"ited.rsc",		GAME_RESOURCEFILE,	"addfc9d82bc2fa1f4cab23743c652c08"},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"fded5c59b8b7c5976229f960d21e6b0b"},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"b3a831fbed337d1f1300fee1dd474f6c"},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4"},
	{"musicd.rsc",		GAME_MUSICFILE,		"1a91cd60169f367ecb6c6e058d899b2f"},
	{ NULL, 0, NULL}
};

static const GameSoundInfo ITEMACDEMO_GameVoice = {
	kSoundVOX,
	22050,
	16,
	false,
	false,
	true
};

static const GameSoundInfo ITEMACDEMO_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	true,
	true
};

static const GameSoundInfo ITEMACDEMO_GameMusic = {
	kSoundPCM,
	11025,
	16,
	false,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Linux Demo version
static const ADGameFileDescription ITE_LINDEMO_GameFiles[] = {
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac"},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb"},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"95a6c148e22e99a8c243f2978223583c"},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4"},
	{"musicd.rsc",		GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4"},
	{ NULL, 0, NULL}
};

static const GameSoundInfo ITELINDEMO_GameMusic = {
	kSoundPCM,
	11025,
	16,
	true,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Linux version

static const ADGameFileDescription ITE_LINCD_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6"},
	{"music.rsc",	GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4"},
	{ NULL, 0, NULL}
};

// Inherit the Earth - Wyrmkeep combined Windows/Mac/Linux version. This
// version is different from the other Wyrmkeep re-releases in that it does
// not have any substitute files. Presumably the ite.rsc file has been
// modified to include the Wyrmkeep changes. The resource files are little-
// endian, except for the voice file which is big-endian.

static const ADGameFileDescription ITE_MULTICD_GameFiles[] = {
	{"ite.rsc",						GAME_RESOURCEFILE,					"a6433e34b97b15e64fe8214651012db9"},
	{"scripts.rsc",					GAME_SCRIPTFILE,					"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",					GAME_SOUNDFILE,						"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"inherit the earth voices",	GAME_VOICEFILE | GAME_SWAPENDIAN,	"c14c4c995e7a0d3828e3812a494301b7"},
	{"music.rsc",					GAME_MUSICFILE,						"d6454756517f042f01210458abe8edd4"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription ITE_MACCD_G_GameFiles[] = {
	{"ite resources.bin",	GAME_RESOURCEFILE | GAME_MACBINARY,	"0bd506aa887bfc7965f695c6bd28237d"},
	{"ite scripts.bin",		GAME_SCRIPTFILE | GAME_MACBINARY,	"af0d7a2588e09ad3ecbc5b474ea238bf"},
	{"ite sounds.bin",		GAME_SOUNDFILE | GAME_MACBINARY,	"441426c6bb2a517f65c7e49b57f7a345"},
	{"ite music.bin",		GAME_MUSICFILE_GM | GAME_MACBINARY,	"c1d20324b7cdf1650e67061b8a93251c"},
	{"ite voices.bin",		GAME_VOICEFILE | GAME_MACBINARY,	"dba92ae7d57e942250fe135609708369"},
	{ NULL, 0, NULL}
};

static const GameSoundInfo ITEMACCD_G_GameSound = {
	kSoundMacPCM,
	22050,
	8,
	false,
	false,
	false
};

// Inherit the Earth - Mac Wyrmkeep version
static const ADGameFileDescription ITE_MACCD_GameFiles[] = {
	{"ite.rsc",						GAME_RESOURCEFILE,	"4f7fa11c5175980ed593392838523060"},
	{"scripts.rsc",					GAME_SCRIPTFILE,	"adf1f46c1d0589083996a7060c798ad0"},
	{"sounds.rsc",					GAME_SOUNDFILE,		"95863b89a0916941f6c5e1789843ba14"},
	{"inherit the earth voices",	GAME_VOICEFILE,		"c14c4c995e7a0d3828e3812a494301b7"},
	{"music.rsc",					GAME_MUSICFILE,		"1a91cd60169f367ecb6c6e058d899b2f"},
	{ NULL, 0, NULL}
};

static const GameSoundInfo ITEMACCD_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	true,
	true
};

static const GameSoundInfo ITEMACCD_GameMusic = {
	kSoundPCM,
	11025,
	16,
	true,
	false,
	true
};

// Inherit the Earth - Diskette version
static const ADGameFileDescription ITE_DISK_DE_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,					"869fc23c8f38f575979ec67152914fee"},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef"},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"0c9113e630f97ef0996b8c3114badb08"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription ITE_DISK_G_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef"},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c46e4392fcd2e89bc91e5567db33b62d"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription ITE_DISK_DE2_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,					"869fc23c8f38f575979ec67152914fee"},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef"},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"0c9113e630f97ef0996b8c3114badb08"},
	{"music.rsc",	GAME_MUSICFILE,						"d6454756517f042f01210458abe8edd4"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription ITE_DISK_G2_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef"},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c46e4392fcd2e89bc91e5567db33b62d"},
	{"music.rsc",	GAME_MUSICFILE,						"d6454756517f042f01210458abe8edd4"},
	{ NULL, 0, NULL}
};

static const GameFontDescription ITEDISK_GameFonts[] = {
	{2},
	{0},
	{1}
};

static const GameSoundInfo ITEDISK_GameSound = {
	kSoundVOC,
	-1,
	-1,
	false,
	false,
	true
};

// Inherit the Earth - CD Enhanced version
static const ADGameFileDescription ITE_WINCD_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription ITE_CD_G_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"50a0d2d7003c926a3832d503c8534e90"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6"},
	{ NULL, 0, NULL}
};

// reported by mld. Bestsellergamers cover disk
static const ADGameFileDescription ITE_CD_DE_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"869fc23c8f38f575979ec67152914fee"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"2fbad5d10b9b60a3415dc4aebbb11718"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription ITE_CD_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6"},
	{ NULL, 0, NULL}
};


static const ADGameFileDescription ITE_CD_G2_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"50a0d2d7003c926a3832d503c8534e90"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6"},
	{"music.rsc",	GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription ITE_CD_DE2_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"869fc23c8f38f575979ec67152914fee"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"2fbad5d10b9b60a3415dc4aebbb11718"},
	{"music.rsc",	GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4"},
	{ NULL, 0, NULL}
};


static const GameFontDescription ITECD_GameFonts[] = {
	{2},
	{0},
	{1}
};

static const GameSoundInfo ITECD_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	false,
	true
};

static const GamePatchDescription ITEWinPatch1_Files[] = {
	{ "cave.mid", GAME_RESOURCEFILE, 9, NULL},
	{ "intro.mid", GAME_RESOURCEFILE, 10, NULL},
	{ "fvillage.mid", GAME_RESOURCEFILE, 11, NULL},
	{ "elkhall.mid", GAME_RESOURCEFILE, 12, NULL},
	{ "mouse.mid", GAME_RESOURCEFILE, 13, NULL},
	{ "darkclaw.mid", GAME_RESOURCEFILE, 14, NULL},
	{ "birdchrp.mid", GAME_RESOURCEFILE, 15, NULL},
	{ "orbtempl.mid", GAME_RESOURCEFILE, 16, NULL},
	{ "spooky.mid", GAME_RESOURCEFILE, 17, NULL},
	{ "catfest.mid", GAME_RESOURCEFILE, 18, NULL},
	{ "elkfanfare.mid", GAME_RESOURCEFILE, 19, NULL},
	{ "bcexpl.mid", GAME_RESOURCEFILE, 20, NULL},
	{ "boargtnt.mid", GAME_RESOURCEFILE, 21, NULL},
	{ "boarking.mid", GAME_RESOURCEFILE, 22, NULL},
	{ "explorea.mid", GAME_RESOURCEFILE, 23, NULL},
	{ "exploreb.mid", GAME_RESOURCEFILE, 24, NULL},
	{ "explorec.mid", GAME_RESOURCEFILE, 25, NULL},
	{ "sunstatm.mid", GAME_RESOURCEFILE, 26, NULL},
	{ "nitstrlm.mid", GAME_RESOURCEFILE, 27, NULL},
	{ "humruinm.mid", GAME_RESOURCEFILE, 28, NULL},
	{ "damexplm.mid", GAME_RESOURCEFILE, 29, NULL},
	{ "tychom.mid", GAME_RESOURCEFILE, 30, NULL},
	{ "kitten.mid", GAME_RESOURCEFILE, 31, NULL},
	{ "sweet.mid", GAME_RESOURCEFILE, 32, NULL},
	{ "brutalmt.mid", GAME_RESOURCEFILE, 33, NULL},
	{ "shiala.mid", GAME_RESOURCEFILE, 34, NULL},

	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "wyrm4.dlt", GAME_RESOURCEFILE, 1533, NULL},
	{ "credit3n.dlt", GAME_RESOURCEFILE, 1796, NULL},
	{ "credit4n.dlt", GAME_RESOURCEFILE, 1797, NULL},
	{ "p2_a.voc", GAME_VOICEFILE, 4, NULL}
};

static const GamePatchDescription ITEWinPatch2_Files[] = {
	{ "cave.mid", GAME_RESOURCEFILE, 9, NULL},
	{ "intro.mid", GAME_RESOURCEFILE, 10, NULL},
	{ "fvillage.mid", GAME_RESOURCEFILE, 11, NULL},
	{ "elkfanfare.mid", GAME_RESOURCEFILE, 19, NULL},
	{ "bcexpl.mid", GAME_RESOURCEFILE, 20, NULL},
	{ "boargtnt.mid", GAME_RESOURCEFILE, 21, NULL},
	{ "explorea.mid", GAME_RESOURCEFILE, 23, NULL},
	{ "sweet.mid", GAME_RESOURCEFILE, 32, NULL},

	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "p2_a.iaf", GAME_VOICEFILE, 4, &ITECD_GameSound}
/*	boarhall.bbm
	elkenter.bbm
	ferrets.bbm
	ratdoor.bbm
	sanctuar.bbm
	tycho.bbm*/
};

static const GamePatchDescription ITEMacPatch_Files[] = {
	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "wyrm4.dlt", GAME_RESOURCEFILE, 1533, NULL},
	{ "credit3m.dlt", GAME_RESOURCEFILE, 1796, NULL},
	{ "credit4m.dlt", GAME_RESOURCEFILE, 1797, NULL},
	{ "p2_a.iaf", GAME_VOICEFILE, 4, &ITEMACCD_GameSound}
};

static const GamePatchDescription ITELinPatch_Files[] = {
	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "credit3n.dlt", GAME_RESOURCEFILE, 1796, NULL},
	{ "credit4n.dlt", GAME_RESOURCEFILE, 1797, NULL},
	{ "P2_A.iaf", GAME_VOICEFILE, 4, &ITECD_GameSound}
};

// IHNM section

static PanelButton IHNM_MainPanelButtons[] = {
	{kPanelButtonVerb,		106,12,		114,30,	kVerbIHNMWalk,'w',0,	0,1,0},
	{kPanelButtonVerb,		106,44,		114,30,	kVerbIHNMLookAt,'l',0,	2,3,0},
	{kPanelButtonVerb,		106,76,		114,30, kVerbIHNMTake,'k',0,	4,5,0},
	{kPanelButtonVerb,		106,108,	114,30, kVerbIHNMUse,'u',0,		6,7,0},
	{kPanelButtonVerb,		223,12,		114,30, kVerbIHNMTalkTo,'t',0,	8,9,0},
	{kPanelButtonVerb,		223,44,		114,30, kVerbIHNMSwallow,'s',0,	10,11,0},
	{kPanelButtonVerb,		223,76,		114,30, kVerbIHNMGive,'g',0,	12,13,0},
	{kPanelButtonVerb,		223,108,	114,30, kVerbIHNMPush,'p',0,	14,15,0},
	{kPanelButtonArrow,		606,22,		20,25,	-1,'[',0,			0,0,0}, //TODO: arrow Sprite Numbers
	{kPanelButtonArrow,		606,108,	20,25,	1,']',0,			0,0,0},

	{kPanelButtonInventory,	357 + 64*0,18,	54,54,	0,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*1,18,	54,54,	1,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*2,18,	54,54,	2,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*3,18,	54,54,	3,'-',0,	0,0,0},

	{kPanelButtonInventory,	357 + 64*0,80,	54,54,	4,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*1,80,	54,54,	5,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*2,80,	54,54,	6,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*3,80,	54,54,	7,'-',0,	0,0,0}
};

static PanelButton IHNM_ConversePanelButtons[] = {
	{kPanelButtonConverseText,	117,18 + IHNM_CONVERSE_TEXT_HEIGHT * 0, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	0,'1',0,	0,0,0},
	{kPanelButtonConverseText,	52,18 + IHNM_CONVERSE_TEXT_HEIGHT * 1, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	1,'2',0,	0,0,0},
	{kPanelButtonConverseText,	52,18 + IHNM_CONVERSE_TEXT_HEIGHT * 2, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	2,'3',0,	0,0,0},
	{kPanelButtonConverseText,	52,18 + IHNM_CONVERSE_TEXT_HEIGHT * 3, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	3,'4',0,	0,0,0},
	//.....
	{kPanelButtonArrow,			606,22,		20,25,	-1,'[',0,	0,0,0}, //TODO: arrow Sprite Numbers
	{kPanelButtonArrow,			606,108,	20,25,	1,']',0,	0,0,0}
};

static PanelButton IHNM_OptionPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};

static PanelButton IHNM_QuitPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};

static PanelButton IHNM_LoadPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};

static PanelButton IHNM_SavePanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};


static const GameDisplayInfo IHNM_DisplayInfo = { //TODO: fill it all
	640, 480,	// logical width&height

	0,			// scene path y offset
	304,		// scene height

	0,			// status x offset
	304,		// status y offset
	616,		// status width
	24,			// status height
	8,			// status text y offset
	253,		// status text color
	250,		// status BG color
	616, 303,	// save reminder pos
	24, 24,		// save reminder w&h
	0,1,		// save reminder sprite numbers

	11, 12,		// left portrait x, y offset
	-1, -1,		// right portrait x, y offset

	-1, -1,		// inventory Up & Down button indexies
	2, 4,		// inventory rows, columns

	0, 328,		// main panel offsets
	ARRAYSIZE(IHNM_MainPanelButtons),
	IHNM_MainPanelButtons,

	-1, -1,		// converse Up & Down button indexies

	IHNM_CONVERSE_MAX_TEXT_WIDTH,
	IHNM_CONVERSE_TEXT_HEIGHT,
	IHNM_CONVERSE_TEXT_LINES,
	0, 328,		// converse panel offsets
	ARRAYSIZE(IHNM_ConversePanelButtons),
	IHNM_ConversePanelButtons,

	-1, -1,		// save file index
	0,			// optionSaveFileVisible
	0, 0,		// option panel offsets
	ARRAYSIZE(IHNM_OptionPanelButtons),
	IHNM_OptionPanelButtons,

	0,0,			// quit panel offsets
	0,0,			// quit panel width & height
	ARRAYSIZE(IHNM_QuitPanelButtons),
	IHNM_QuitPanelButtons,

	0, 0,			// load panel offsets
	0, 0,			// load panel width & height
	ARRAYSIZE(IHNM_LoadPanelButtons),
	IHNM_LoadPanelButtons,

	-1,				// save edit index
	0, 0,			// save panel offsets
	0, 0,			// save panel width & height
	ARRAYSIZE(IHNM_SavePanelButtons),
	IHNM_SavePanelButtons,

	// No protection panel in IHNM
	-1,				// protect edit index
	0, 0,			// protect panel offsets
	0, 0,			// protect panel width & height
	ARRAYSIZE(IHNM_SavePanelButtons),
	IHNM_SavePanelButtons
};

static const GameResourceDescription IHNM_Resources = {
	RID_IHNM_SCENE_LUT,  // Scene lookup table RN
	RID_IHNM_SCRIPT_LUT, // Script lookup table RN
	RID_IHNM_MAIN_PANEL,
	RID_IHNM_CONVERSE_PANEL,
	RID_IHNM_OPTION_PANEL,
	RID_IHNM_MAIN_SPRITES,
	RID_IHNM_MAIN_PANEL_SPRITES,
	0,
	RID_IHNM_MAIN_STRINGS,
	0
};

// I Have No Mouth and I Must Scream - Demo version
static const ADGameFileDescription IHNM_DEMO_GameFiles[] = {
	{"scream.res",		GAME_RESOURCEFILE,	"46bbdc65d164ba7e89836a0935eec8e6"},
	{"scripts.res",		GAME_SCRIPTFILE,	"9626bda8978094ff9b29198bc1ed5f9a"},
	{"sfx.res",			GAME_SOUNDFILE,		"1c610d543f32ec8b525e3f652536f269"},
	{"voicesd.res",		GAME_VOICEFILE,		"3bbc16a8f741dbb511da506c660a0b54"},
	{ NULL, 0, NULL}
};

// I Have No Mouth and I Must Scream - Retail CD version

static const ADGameFileDescription IHNM_CD_GameFiles[] = {
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52"},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583"},
	{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6"},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e"},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01"},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269"},
	{"voicess.res",	GAME_VOICEFILE,						"54b1f2013a075338ceb0e258d97808bd"}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"fc6440b38025f4b2cc3ff55c3da5c3eb"},
	{"voices2.res",	GAME_VOICEFILE,						"b37f10fd1696ade7d58704ccaaebceeb"},
	{"voices3.res",	GAME_VOICEFILE,						"3bbc16a8f741dbb511da506c660a0b54"},
	{"voices4.res",	GAME_VOICEFILE,						"ebfa160122d2247a676ca39920e5d481"},
	{"voices5.res",	GAME_VOICEFILE,						"1f501ce4b72392bdd1d9ec38f6eec6da"},
	{"voices6.res",	GAME_VOICEFILE,						"f580ed7568c7d6ef34e934ba20adf834"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription IHNM_CD_ES_GameFiles[] = {
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52"},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583"},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224"},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e"},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01"},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269"},
	{"voicess.res",	GAME_VOICEFILE,						"d869de9883c8faea7f687217a9ec7057"}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"dc6a34e3d1668730ea46815a92c7847f"},
	{"voices2.res",	GAME_VOICEFILE,						"dc6a5fa7a4cdc2ca5a6fd924e969986c"},
	{"voices3.res",	GAME_VOICEFILE,						"dc6a5fa7a4cdc2ca5a6fd924e969986c"},
	{"voices4.res",	GAME_VOICEFILE,						"0f87400b804232a58dd22e404420cc45"},
	{"voices5.res",	GAME_VOICEFILE,						"172668cfc5d8c305cb5b1a9b4d995fc0"},
	{"voices6.res",	GAME_VOICEFILE,						"96c9bda9a5f41d6bc232ed7bf6d371d9"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription IHNM_CD_RU_GameFiles[] = {
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52"},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583"},
	{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6"},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e"},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01"},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269"},
	{"voicess.res",	GAME_VOICEFILE,						"9df7cd3b18ddaa16b5291b3432567036"}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"d6100d2dc3b2b9f2e1ad247f613dce9b"},
	{"voices2.res",	GAME_VOICEFILE,						"84f6f48ecc2832841ea6417a9a379430"},
	{"voices3.res",	GAME_VOICEFILE,						"ebb9501283047f27a0f54e27b3c8ba1e"},
	{"voices4.res",	GAME_VOICEFILE,						"4c145da5fa6d1306162a7ca8ce5a4f2e"},
	{"voices5.res",	GAME_VOICEFILE,						"871a559644281917677eca4af1b05620"},
	{"voices6.res",	GAME_VOICEFILE,						"211be5c24f066d69a2f6cfa953acfba6"},
	{ NULL, 0, NULL}
};

// I Have No Mouth and I Must Scream - Censored CD version (without Nimdok)

// Reported by mld. German Retail
static const ADGameFileDescription IHNM_CD_DE_GameFiles[] = {
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52"},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583"},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224"},
	{"scripts.res",	GAME_SCRIPTFILE,					"32aa01a89937520fe0ea513950117292"},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e"},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269"},
	{"voicess.res",	GAME_VOICEFILE,						"8b09a196a52627cacb4eab13bfe0b2c3"}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"424971e1e2373187c3f5734fe36071a2"},
	{"voices2.res",	GAME_VOICEFILE,						"c270e0980782af43641a86e4a14e2a32"},
	{"voices3.res",	GAME_VOICEFILE,						"49e42befea883fd101ec3d0f5d0647b9"},
	{"voices5.res",	GAME_VOICEFILE,						"c477443c52a0aa56e686ebd8d051e4ab"},
	{"voices6.res",	GAME_VOICEFILE,						"2b9aea838f74b4eecfb29a8f205a2bd4"},
	{ NULL, 0, NULL}
};

static const ADGameFileDescription IHNM_CD_FR_GameFiles[] = {
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52"},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583"},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224"},
	{"scripts.res",	GAME_SCRIPTFILE,					"32aa01a89937520fe0ea513950117292"},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e"},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269"},
	{"voicess.res",	GAME_VOICEFILE,						"b8642e943bbebf89cef2f48b31cb4305"}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"424971e1e2373187c3f5734fe36071a2"},
	{"voices2.res",	GAME_VOICEFILE,						"c2d93a35d2c2def9c3d6d242576c794b"},
	{"voices3.res",	GAME_VOICEFILE,						"49e42befea883fd101ec3d0f5d0647b9"},
	{"voices5.res",	GAME_VOICEFILE,						"f4c415de7c03de86b73f9a12b8bd632f"},
	{"voices6.res",	GAME_VOICEFILE,						"3fc5358a5d8eee43bdfab2740276572e"},
	{ NULL, 0, NULL}
};

static const GameFontDescription IHNMDEMO_GameFonts[] = {
	{2},
	{3},
	{4}
};

static const GameFontDescription IHNMCD_GameFonts[] = {
	{2},
	{3},
	{4},
	{5},
	{6},  // kIHNMFont8
	{7},
	{8}   // kIHNMMainFont
};

static const GameSoundInfo IHNM_GameSound = {
	kSoundWAV,
	-1,
	-1,
	false,
	false,
	true
};

#define FILE_MD5_BYTES 5000

static const SAGAGameDescription gameDescriptions[] = {
	// Inherit the earth - DOS Demo version
	// sound unchecked
	{
		{
			"ite",
			"Demo", // Game title
			ITE_DEMO_G_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_ITE,
		GID_ITE_DEMO_G, // Game id
		0, // features
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE, // Starting scene number
		&ITEDemo_Resources,
		ARRAYSIZE(ITEDEMO_GameFonts),
		ITEDEMO_GameFonts,
		&ITEDEMO_GameSound,
		&ITEDEMO_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - MAC Demo version
	{
		{
			"ite",
			"Demo",
			ITE_MACDEMO2_GameFiles,
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},
		GType_ITE,
		GID_ITE_MACDEMO2,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACDEMO_GameVoice,
		&ITEMACDEMO_GameSound,
		&ITEMACDEMO_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
	},

	// Inherit the earth - early MAC Demo version
	{
		{
			"ite",
			"early Demo",
			ITE_MACDEMO1_GameFiles,
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},
		GType_ITE,
		GID_ITE_MACDEMO1,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACDEMO_GameVoice,
		&ITEMACDEMO_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
	},

	// Inherit the earth - MAC CD Guild version
	{
		{
			"ite",
			"CD",
			ITE_MACCD_G_GameFiles,
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},
		GType_ITE,
		GID_ITE_MACCD_G,
		GF_BIG_ENDIAN_DATA | GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACCD_G_GameSound,
		&ITEMACCD_G_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - MAC CD Wyrmkeep version
	{
		{
			"ite",
			"Wyrmkeep CD",
			ITE_MACCD_GameFiles,
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},
		GType_ITE,
		GID_ITE_MACCD,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACCD_GameSound,
		&ITEMACCD_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
	},

	// Inherit the earth - Linux Demo version
	// Note: it should be before GID_ITE_WINDEMO2 version
	{
		{
			"ite",
			"Demo",
			ITE_LINDEMO_GameFiles,
			Common::EN_ANY,
			Common::kPlatformLinux,
		},
		GType_ITE,
		GID_ITE_LINDEMO,
		GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO2_GameVoice,
		&ITEWINDEMO2_GameSound,
		&ITELINDEMO_GameMusic,
		ARRAYSIZE(ITELinPatch_Files),
		ITELinPatch_Files,
	},

	// Inherit the earth - Win32 Demo version
	{
		{
			"ite",
			"Demo",
			ITE_WINDEMO2_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},
		GType_ITE,
		GID_ITE_WINDEMO2,
		GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO2_GameVoice,
		&ITEWINDEMO2_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch2_Files),
		ITEWinPatch2_Files,
	},

	// Inherit the earth - early Win32 Demo version
	{
		{
			"ite",
			"early Demo",
			ITE_WINDEMO1_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},
		GType_ITE,
		GID_ITE_WINDEMO1,
		GF_WYRMKEEP | GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO1_GameSound,
		&ITEWINDEMO1_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch1_Files),
		ITEWinPatch1_Files,
	},

	// Inherit the earth - Wyrmkeep combined Windows/Mac/Linux CD
	{
		{
			"ite",
			"Multi-OS CD Version",
			ITE_MULTICD_GameFiles,
			Common::EN_ANY,
			Common::kPlatformUnknown,
		},
		GType_ITE,
		GID_ITE_MULTICD,
		GF_WYRMKEEP | GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITEMACCD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
	},

	// Inherit the earth - Wyrmkeep Linux CD version
	{
		{
			"ite",
			"CD Version",
			ITE_LINCD_GameFiles,
			Common::EN_ANY,
			Common::kPlatformLinux,
		},
		GType_ITE,
		GID_ITE_LINCD,
		GF_WYRMKEEP | GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITELinPatch_Files),
		ITELinPatch_Files,
	},

	// Inherit the earth - Wyrmkeep Windows CD version
	{
		{
			"ite",
			"CD Version",
			ITE_WINCD_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},
		GType_ITE,
		GID_ITE_WINCD,
		GF_WYRMKEEP | GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch1_Files),
		ITEWinPatch1_Files,
	},

	// Inherit the earth - DOS CD version
	{
		{
			"ite",
			"CD Version",
			ITE_CD_G_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_ITE,
		GID_ITE_CD_G,
		GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - DOS CD version with digital music
	{
		{
			"ite",
			"CD Version",
			ITE_CD_G2_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_ITE,
		GID_ITE_CD_G2,
		GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
	},

	// Inherit the earth - DOS CD German version
	{
		{
			"ite",
			"CD Version",
			ITE_CD_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},
		GType_ITE,
		GID_ITE_CD_DE,
		GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - DOS CD German version with digital music
	{
		{
			"ite",
			"CD Version",
			ITE_CD_DE2_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},
		GType_ITE,
		GID_ITE_CD_DE2,
		GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
	},

	// Inherit the earth - CD version
	{
		{
			"ite",
			"CD Version",
			ITE_CD_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_ITE,
		GID_ITE_CD,
		GF_CD_FX,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - German Floppy version
	{
		{
			"ite",
			"Floppy",
			ITE_DISK_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},
		GType_ITE,
		GID_ITE_DISK_DE,
		0,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - German Floppy version with digital music
	{
		{
			"ite",
			"Floppy",
			ITE_DISK_DE2_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},
		GType_ITE,
		GID_ITE_DISK_DE2,
		0,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
	},

	// Inherit the earth - Disk version
	{
		{
			"ite",
			"Floppy",
			ITE_DISK_G_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_ITE,
		GID_ITE_DISK_G,
		0,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		NULL,
		0,
		NULL,
	},

	// Inherit the earth - Disk version with digital music
	{
		{
			"ite",
			"Floppy",
			ITE_DISK_G2_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_ITE,
		GID_ITE_DISK_G2,
		0,
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Demo version
	{
		{
			"ihnm",
			"Demo",
			IHNM_DEMO_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_IHNM,
		GID_IHNM_DEMO,
		0,
		&IHNM_DisplayInfo,
		0,
		&IHNM_Resources,
		ARRAYSIZE(IHNMDEMO_GameFonts),
		IHNMDEMO_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - CD version
	{
		{
			"ihnm",
			"",
			IHNM_CD_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_IHNM,
		GID_IHNM_CD,
		0,
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - De CD version
	{
		{
			"ihnm",
			"",
			IHNM_CD_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},
		GType_IHNM,
		GID_IHNM_CD_DE,
		0,
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Sp CD version
	{
		{
			"ihnm",
			"",
			IHNM_CD_ES_GameFiles,
			Common::ES_ESP,
			Common::kPlatformPC,
		},
		GType_IHNM,
		GID_IHNM_CD_ES,
		0,
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Ru CD version
	{
		{
			"ihnm",
			"",
			IHNM_CD_RU_GameFiles,
			Common::RU_RUS,
			Common::kPlatformPC,
		},
		GType_IHNM,
		GID_IHNM_CD_RU,
		0,
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},

	// I Have No Mouth And I Must Scream - Fr CD version
	{
		{
			"ihnm",
			"",
			IHNM_CD_FR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformPC,
		},
		GType_IHNM,
		GID_IHNM_CD_FR,
		0,
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
	},
	{ { NULL, NULL, NULL, Common::UNK_LANG, Common::kPlatformUnknown }, 0, 0, 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL, 0, NULL }
};
