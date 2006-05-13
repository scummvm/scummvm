enum SAGAGameType {
	GType_ITE = 0,
	GType_IHNM = 1
};

enum GameIds {
	// Dreamers Guild
	GID_ITE_DEMO_G = 0,
	GID_ITE_DISK_G,
	GID_ITE_DISK_G2,
	GID_ITE_CD_G,
	GID_ITE_CD_G2,
	GID_ITE_MACCD_G,

	// Wyrmkeep
	GID_ITE_CD,       // data for Win rerelease is same as in old DOS
	GID_ITE_WINCD,    // but it has a bunch of patch files
	GID_ITE_MACCD,
	GID_ITE_LINCD,
	GID_ITE_MULTICD,  // Wyrmkeep combined Windows/Mac/Linux version
	GID_ITE_WINDEMO1, // older Wyrmkeep windows demo
	GID_ITE_MACDEMO1, // older Wyrmkeep mac demo
	GID_ITE_LINDEMO,
	GID_ITE_WINDEMO2,
	GID_ITE_MACDEMO2,

	// German
	GID_ITE_DISK_DE,
	GID_ITE_DISK_DE2,
	GID_ITE_AMIGACD_DE, // TODO
	GID_ITE_OLDMAC_DE,  // TODO
	GID_ITE_AMIGA_FL_DE,// TODO
	GID_ITE_CD_DE,      // reported by mld. Bestsellergamers cover disk
	GID_ITE_CD_DE2,
	GID_ITE_AMIGA_AGA_DEMO, // TODO
	GID_ITE_AMIGA_ECS_DEMO, // TODO

	GID_IHNM_DEMO,
	GID_IHNM_CD,
	GID_IHNM_CD_DE,   // reported by mld. German retail
	GID_IHNM_CD_ES,
	GID_IHNM_CD_RU,
	GID_IHNM_CD_FR
};

enum GameFileTypes {
	GAME_RESOURCEFILE = 1 << 0,
	GAME_SCRIPTFILE   = 1 << 1,
	GAME_SOUNDFILE    = 1 << 2,
	GAME_VOICEFILE    = 1 << 3,
	GAME_DEMOFILE     = 1 << 4,
	GAME_MUSICFILE    = 1 << 5,
	GAME_MUSICFILE_GM = 1 << 6,
	GAME_MUSICFILE_FM = 1 << 7,
	GAME_PATCHFILE    = 1 << 8,
	GAME_MACBINARY    = 1 << 9,
	GAME_SWAPENDIAN   = 1 << 10
};

enum GameFeatures {
        GF_BIG_ENDIAN_DATA   = 1 << 0,
        GF_WYRMKEEP          = 1 << 1,
        GF_CD_FX             = 1 << 2,
        GF_SCENE_SUBSTITUTES = 1 << 3
};

enum VerbTypeIds {
        kVerbITENone = 0,
        kVerbITEPickUp = 1,
        kVerbITELookAt = 2,
        kVerbITEWalkTo = 3,
        kVerbITETalkTo = 4,
        kVerbITEOpen = 5,
        kVerbITEClose = 6,
        kVerbITEGive = 7,
        kVerbITEUse = 8,
        kVerbITEOptions = 9,
        kVerbITEEnter = 10,
        kVerbITELeave = 11,
        kVerbITEBegin = 12,
        kVerbITEWalkOnly = 13,
        kVerbITELookOnly = 14,


        kVerbIHNMNone = 0,
        kVerbIHNMWalk = 1,
        kVerbIHNMLookAt = 2,
        kVerbIHNMTake = 3,
        kVerbIHNMUse = 4,
        kVerbIHNMTalkTo = 5,
        kVerbIHNMSwallow = 6,
        kVerbIHNMGive = 7,
        kVerbIHNMPush = 8,
        kVerbIHNMOptions = 9,
        kVerbIHNMEnter = 10,
        kVerbIHNMLeave = 11,
        kVerbIHNMBegin = 12,
        kVerbIHNMWalkOnly = 13,
        kVerbIHNMLookOnly = 14,

        kVerbTypeIdsMax = kVerbITELookOnly + 1
};
enum PanelButtonType {
	kPanelButtonVerb = 1 << 0,
	kPanelButtonArrow = 1 << 1,
	kPanelButtonConverseText = 1 << 2,
	kPanelButtonInventory = 1 << 3,

	kPanelButtonOption = 1 << 4,
	kPanelButtonOptionSlider = 1 << 5,
	kPanelButtonOptionSaveFiles = 1 << 6,
	kPanelButtonOptionText = 1 << 7,

	kPanelButtonQuit = 1 << 8,
	kPanelButtonQuitText = 1 << 9,

	kPanelButtonLoad = 1 << 10,
	kPanelButtonLoadText = 1 << 11,

	kPanelButtonSave = 1 << 12,
	kPanelButtonSaveText = 1 << 13,
	kPanelButtonSaveEdit = 1 << 14,

	kPanelButtonProtectText = 1 << 15,
	kPanelButtonProtectEdit = 1 << 16,

	kPanelAllButtons = 0xFFFFF
};

enum GameSoundTypes {
	kSoundPCM = 0,
	kSoundVOX = 1,
	kSoundVOC = 2,
	kSoundWAV = 3,
	kSoundMacPCM = 4
};

enum TextStringIds {
	kTextWalkTo,
	kTextLookAt,
	kTextPickUp,
	kTextTalkTo,
	kTextOpen,
	kTextClose,
	kTextUse,
	kTextGive,
	kTextOptions,
	kTextTest,
	kTextDemo,
	kTextHelp,
	kTextQuitGame,
	kTextFast,
	kTextSlow,
	kTextOn,
	kTextOff,
	kTextContinuePlaying,
	kTextLoad,
	kTextSave,
	kTextGameOptions,
	kTextReadingSpeed,
	kTextMusic,
	kTextSound,
	kTextCancel,
	kTextQuit,
	kTextOK,
	kTextMid,
	kTextClick,
	kText10Percent,
	kText20Percent,
	kText30Percent,
	kText40Percent,
	kText50Percent,
	kText60Percent,
	kText70Percent,
	kText80Percent,
	kText90Percent,
	kTextMax,
	kTextQuitTheGameQuestion,
	kTextLoadSuccessful,
	kTextEnterSaveGameName,
	kTextGiveTo,
	kTextUseWidth,
	kTextNewSave,
	kTextICantPickup,
	kTextNothingSpecial,
	kTextNoPlaceToOpen,
	kTextNoOpening,
	kTextDontKnow,
	kTextShowDialog,
	kTextEnterProtectAnswer
};


struct GameFileDescription {
	const char *fileName;
	uint16 fileType;
	const char *md5;
};

struct GameResourceDescription {
	uint32 sceneLUTResourceId;
	uint32 moduleLUTResourceId;
	uint32 mainPanelResourceId;
	uint32 conversePanelResourceId;
	uint32 optionPanelResourceId;
	uint32 mainSpritesResourceId;
	uint32 mainPanelSpritesResourceId;
	uint32 defaultPortraitsResourceId;
	uint32 mainStringsResourceId;
	uint32 actorsStringsResourceId;
};

struct GameFontDescription {
	uint32 fontResourceId;
};

struct PanelButton {
	PanelButtonType type;
	int xOffset;
	int yOffset;
	int width;
	int height;
	int id;
	uint16 ascii;
	int state;
	int upSpriteNumber;
	int downSpriteNumber;
	int overSpriteNumber;
};

struct GameDisplayInfo {
	int logicalWidth;
	int logicalHeight;

	int pathStartY;
	int sceneHeight;

	int statusXOffset;
	int statusYOffset;
	int statusWidth;
	int statusHeight;
	int statusTextY;
	int statusTextColor;
	int statusBGColor;

	int saveReminderXOffset;
	int saveReminderYOffset;
	int saveReminderWidth;
	int saveReminderHeight;
	int saveReminderFirstSpriteNumber;
	int saveReminderSecondSpriteNumber;

	int leftPortraitXOffset;
	int leftPortraitYOffset;
	int rightPortraitXOffset;
	int rightPortraitYOffset;

	int inventoryUpButtonIndex;
	int inventoryDownButtonIndex;
	int inventoryRows;
	int inventoryColumns;

	int mainPanelXOffset;
	int mainPanelYOffset;
	int mainPanelButtonsCount;
	PanelButton *mainPanelButtons;

	int converseMaxTextWidth;
	int converseTextHeight;
	int converseTextLines;
	int converseUpButtonIndex;
	int converseDownButtonIndex;

	int conversePanelXOffset;
	int conversePanelYOffset;
	int conversePanelButtonsCount;
	PanelButton *conversePanelButtons;

	int optionSaveFilePanelIndex;
	int optionSaveFileSliderIndex;
	uint32 optionSaveFileVisible;

	int optionPanelXOffset;
	int optionPanelYOffset;
	int optionPanelButtonsCount;
	PanelButton *optionPanelButtons;

	int quitPanelXOffset;
	int quitPanelYOffset;
	int quitPanelWidth;
	int quitPanelHeight;
	int quitPanelButtonsCount;
	PanelButton *quitPanelButtons;

	int loadPanelXOffset;
	int loadPanelYOffset;
	int loadPanelWidth;
	int loadPanelHeight;
	int loadPanelButtonsCount;
	PanelButton *loadPanelButtons;

	int saveEditIndex;
	int savePanelXOffset;
	int savePanelYOffset;
	int savePanelWidth;
	int savePanelHeight;
	int savePanelButtonsCount;
	PanelButton *savePanelButtons;

	int protectEditIndex;
	int protectPanelXOffset;
	int protectPanelYOffset;
	int protectPanelWidth;
	int protectPanelHeight;
	int protectPanelButtonsCount;
	PanelButton *protectPanelButtons;
};

struct GameSoundInfo {
	GameSoundTypes resourceType;
	long frequency;
	int sampleBits;
	bool stereo;
	bool isBigEndian;
	bool isSigned;
};

struct GamePatchDescription {
	const char *fileName;
	uint16 fileType;
	uint32 resourceId;
	GameSoundInfo *soundInfo;
};

struct GameDescription {
	const char *name;
	SAGAGameType gameType;
	GameIds gameId;
	const char *extra;
	GameDisplayInfo *gameDisplayInfo;
	int startSceneNumber;
	GameResourceDescription *resourceDescription;
	int filesCount;
	GameFileDescription *filesDescriptions;
	int fontsCount;
	GameFontDescription *fontDescriptions;
	GameSoundInfo *voiceInfo;
	GameSoundInfo *sfxInfo;
	GameSoundInfo *musicInfo;
	int patchesCount;
	GamePatchDescription *patchDescriptions;
	uint32 features;
	Common::Language language;
	Common::Platform platform;
};

#define FILE_MD5_BYTES 5000
