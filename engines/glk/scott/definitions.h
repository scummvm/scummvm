#ifndef GLK_SCOTT_DEFINITIONS
#define GLK_SCOTT_DEFINITIONS

#include "common/str.h"

namespace Glk {
namespace Scott {

const int FOLLOWS = 0xFFFF;

const int GO = 1;
const int TAKE = 10;
const int DROP = 18;

const int LASTALL = 128;

enum GameIDType {
	UNKNOWN_GAME,
	SCOTTFREE,
	TI994A,
	PIRATE,
	VOODOO,
	STRANGE,
	BANZAI,
	BATON,
	BATON_C64,
	TIME_MACHINE,
	TIME_MACHINE_C64,
	ARROW1,
	ARROW1_C64,
	ARROW2,
	ARROW2_C64,
	PULSAR7,
	PULSAR7_C64,
	CIRCUS,
	CIRCUS_C64,
	FEASIBILITY,
	FEASIBILITY_C64,
	AKYRZ,
	AKYRZ_C64,
	PERSEUS,
	PERSEUS_C64,
	PERSEUS_ITALIAN,
	INDIANS,
	INDIANS_C64,
	WAXWORKS,
	WAXWORKS_C64,
	HULK,
	HULK_C64,
	ADVENTURELAND,
	ADVENTURELAND_C64,
	SECRET_MISSION,
	SECRET_MISSION_C64,
	CLAYMORGUE,
	CLAYMORGUE_C64,
	SPIDERMAN,
	SPIDERMAN_C64,
	SAVAGE_ISLAND,
	SAVAGE_ISLAND_C64,
	SAVAGE_ISLAND2,
	SAVAGE_ISLAND2_C64,
	GREMLINS,
	GREMLINS_ALT,
	GREMLINS_C64,
	GREMLINS_GERMAN,
	GREMLINS_GERMAN_C64,
	GREMLINS_SPANISH,
	SUPERGRAN,
	SUPERGRAN_C64,
	ROBIN_OF_SHERWOOD,
	ROBIN_OF_SHERWOOD_C64,
	SEAS_OF_BLOOD,
	SEAS_OF_BLOOD_C64,
	NUMGAMES
};

enum ExplicitResultType {
	ER_NO_RESULT,
	ER_SUCCESS = 0,
	ER_RAN_ALL_LINES_NO_MATCH = -1,
	ER_RAN_ALL_LINES = -2
};

enum ActionResultType {
	ACT_SUCCESS = 0,
	ACT_FAILURE = 1,
	ACT_CONTINUE,
	ACT_GAMEOVER
};

enum SysMessageType {
	NORTH,
	SOUTH,
	EAST,
	WEST,
	UP,
	DOWN,
	PLAY_AGAIN,
	IVE_STORED,
	TREASURES,
	ON_A_SCALE_THAT_RATES,
	DROPPED,
	TAKEN,
	OK,
	YOUVE_SOLVED_IT,
	I_DONT_UNDERSTAND,
	YOU_CANT_DO_THAT_YET,
	HUH,
	DIRECTION,
	YOU_HAVENT_GOT_IT,
	YOU_HAVE_IT,
	YOU_DONT_SEE_IT,
	THATS_BEYOND_MY_POWER,
	DANGEROUS_TO_MOVE_IN_DARK,
	YOU_FELL_AND_BROKE_YOUR_NECK,
	YOU_CANT_GO_THAT_WAY,
	I_DONT_KNOW_HOW_TO,
	SOMETHING,
	I_DONT_KNOW_WHAT_A,
	IS,
	TOO_DARK_TO_SEE,
	YOU_ARE,
	YOU_SEE,
	EXITS,
	INVENTORY,
	NOTHING,
	WHAT_NOW,
	HIT_ENTER,
	LIGHT_HAS_RUN_OUT,
	LIGHT_RUNS_OUT_IN,
	TURNS,
	YOURE_CARRYING_TOO_MUCH,
	IM_DEAD,
	RESUME_A_SAVED_GAME,
	NONE,
	NOTHING_HERE_TO_TAKE,
	YOU_HAVE_NOTHING,
	LIGHT_GROWING_DIM,
	EXITS_DELIMITER,
	MESSAGE_DELIMITER,
	ITEM_DELIMITER,
	WHAT,
	YES,
	NO,
	ANSWER_YES_OR_NO,
	ARE_YOU_SURE,
	MOVE_UNDONE,
	CANT_UNDO_ON_FIRST_TURN,
	NO_UNDO_STATES,
	SAVED,
	CANT_USE_ALL,
	TRANSCRIPT_OFF,
	TRANSCRIPT_ON,
	NO_TRANSCRIPT,
	TRANSCRIPT_ALREADY,
	FAILED_TRANSCRIPT,
	TRANSCRIPT_START,
	TRANSCRIPT_END,
	BAD_DATA,
	STATE_SAVED,
	STATE_RESTORED,
	NO_SAVED_STATE,
	LAST_SYSTEM_MESSAGE
};

const SysMessageType MAX_SYSMESS = LAST_SYSTEM_MESSAGE;

enum DictionaryType {
	NOT_A_GAME,
	FOUR_LETTER_UNCOMPRESSED,
	THREE_LETTER_UNCOMPRESSED,
	FIVE_LETTER_UNCOMPRESSED,
	FOUR_LETTER_COMPRESSED,
	FIVE_LETTER_COMPRESSED,
	GERMAN,
	SPANISH,
	ITALIAN
};

enum GameType {
	NO_TYPE,
	GREMLINS_VARIANT,
	SHERWOOD_VARIANT,
	SAVAGE_ISLAND_VARIANT,
	SECRET_MISSION_VARIANT,
	SEAS_OF_BLOOD_VARIANT,
	OLD_STYLE,
};

enum Subtype {
	ENGLISH = 0x1,
	MYSTERIOUS = 0x2,
	LOCALIZED = 0x4,
	C64 = 0x8
};

enum PaletteType {
	NO_PALETTE,
	ZX,
	ZXOPT,
	C64A,
	C64B,
	VGA
};

enum HeaderType {
	NO_HEADER,
	EARLY,
	LATE,
	HULK_HEADER,
	GREMLINS_C64_HEADER,
	ROBIN_C64_HEADER,
	SUPERGRAN_C64_HEADER,
	SEAS_OF_BLOOD_C64_HEADER,
	MYSTERIOUS_C64_HEADER,
	ARROW_OF_DEATH_PT_2_C64_HEADER,
	INDIANS_C64_HEADER
};

enum ActionTableType {
	UNKNOWN_ACTIONS_TYPE,
	COMPRESSED,
	UNCOMPRESSED,
	HULK_ACTIONS
};

struct GameInfo {
	Common::String _title;

	GameIDType _gameID;
	GameType _type;
	Subtype _subType;
	DictionaryType _dictionary;

	int _numberOfItems;
	int _numberOfActions;
	int _numberOfWords;
	int _numberOfRooms;
	int _maxCarried;
	int _wordLength;
	int _numberOfMessages;

	int _numberOfVerbs;
	int _numberOfNouns;

	int _startOfHeader;
	HeaderType _headerStyle;

	int _startOfRoomImageList;
	int _startOfItemFlags;
	int _startOfItemImageList;

	int _startOfActions;
	ActionTableType _actionsStyle;
	int _startOfDictionary;
	int _startOfRoomDescriptions;
	int _startOfRoomConnections;
	int _startOfMessages;
	int _startOfItemDescriptions;
	int _startOfItemLocations;

	int _startOfSystemMessages;
	int _startOfDirections;

	int _startOfCharacters;
	int _startOfImageData;
	int _imageAddressOffset; /* This is the difference between the value given by
							   the image data lookup table and a usable file
							   offset */
	int _numberOfPictures;
	PaletteType _palette;
	int _pictureFormatVersion;
	int _startOfIntroText;
};

} // End of namespace Scott
} // End of namespace Glk
#endif /* definitions_h */
