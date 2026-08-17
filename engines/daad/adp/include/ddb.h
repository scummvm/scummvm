#ifndef __DDB_H__
#define __DDB_H__

#include <os_types.h>

#ifndef HAS_PSG
#define HAS_PSG 0
#endif

#define MAX_DDB_SIZE	 65536
// Upper bound for a raw container (disk/tape image) we are willing to read into
// memory to scan for an embedded DDB (see DDB_Load's carve fallback).
#define MAX_CONTAINER_SIZE (8UL*1024UL*1024UL)
#define MAX_PROC_STACK 	 16
#define HISTORY_SIZE     1024
#define UNDO_BUFFER_SIZE 512

/* ───────────────────────────────────────────────────────────────────────── */
/*  .DDB - DAAD Database Functions			                                 */
/* ───────────────────────────────────────────────────────────────────────── */

struct DDB_Interpreter;
struct File;

enum DDB_WordType
{
	WordType_Verb         = 0,
	WordType_Adverb       = 1,
	WordType_Noun         = 2,
	WordType_Adjective    = 3,
	WordType_Preposition  = 4,
	WordType_Conjunction  = 5,
	WordType_Pronoun      = 6,
	WordType_Unknown      = 255
};

enum DDB_Flag
{
	Flag_Darkness 		= 0,
	Flag_NumCarried		= 1,			// 01  Numbers of objects currently carried by the player

	// Flags 2 to 10 are used only in PAWS and Version 1 DDB

	Flag_AutoDecDesc       = 2,			// 02 decrements by 1 during DESC
	Flag_AutoDecDescDark   = 3,			// 03 decrements by 1 during DESC in dark locations
	Flag_AutoDecDescDarkNL = 4,			// 04 decrements by 1 during DESC in dark locations without lantern
	Flag_AutoDec           = 5,			// 05 to 08 decrements by 1 every turn
	Flag_AutoDecDark       = 9,			// 09 to 12 decrements by 1 every turn in dark locations
	Flag_AutoDecDarkNL     = 10,		// 0D to 16 decrements by 1 every turn in dark locations without lantern

	// Flags 11 to 28 are unused

	Flag_GraphicFlags	= 29,			// 1D  Graphic flags (can be checked with HASAT):

			// ^ Bit 7: 1 if graphics are available
			//   Bit 6: 1 do not clear window before desc		(manual: 1 to set invisible draw mode)
			//   Bit 5: 1 to turn pictures off
			//   Bit 4: 1 to wait for a key after drawing picture
			//   Bit 3: 1 to change border to picture colours
			//   Bit 0: 1 if mouse is present
			//
			// Old condact GRAPHIC [0-3] [0-1] sets bits 3-6 of this flag
			//
			// In PAWS, only bit 5-7 are used

	Flag_Score     		= 30,			// 1E  User score
	Flag_Turns     		= 31,			// 1F  Turn count (2 bytes) incremented by PARSE 0

	Flag_Verb      		= 33,			// 21
	Flag_Noun1     		= 34,			// 22
	Flag_Adjective1 	= 35,			// 23
	Flag_Adverb    		= 36,			// 24

	Flag_MaxCarried		= 37,			// 25  Maximum number of objects to be carried (ABILITY)
	Flag_Locno     		= 38,			// 26  Current player location

	// Flags 39 and 40 are used only in PAWS, flag 41 has a differnt meaning in PAW

	Flag_TopLine        = 39,			// 27  Top line of the text window (PAWS only)
	Flag_PAWMode        = 40,			// 28  Graphics mode (PAWS only)

			// ^ Bit 1: 1 to supress More... message
			//   Bit 0: 0 to clear the screen every DESC

	Flag_HasPicture     = 40,			// 28 128 if current location has a picture (DAAD only)

	Flag_SplitLine      = 41,			// 29  Split line of the text window (PAWS only)

	Flag_InputStream    = 41,			// 29  Input stream (window 1-7, 0 = current)
	Flag_Prompt    		= 42,			// 2A  System message for prompt (0 = random)

	Flag_Preposition    = 43,			// 2B
	Flag_Noun2     		= 44,			// 2C
	Flag_Adjective2     = 45,			// 2D
	Flag_CPNoun    		= 46,			// 2E  Current pronoun noun ("IT")
	Flag_CPAdjective    = 47,			// 2F  Current pronoun adjective ("IT")

	Flag_Timeout   		= 48,			// 30  Timeout in seconds
	Flag_TimeoutFlags	= 49,			// 31  Timeout control flags

			// ^ Bit 0: 1 to enable timeout during input
			//   Bit 1: 1 to enable timeout during the "More..." prompt
			//   Bit 2: 1 to enable timeout during the ANYKEY action
			//   Bit 5: 1 to recall INPUT in timeout (PAWS)
			//   Bits 6-7: PAW input completion/timeout result

	Flag_DoAllLocNo		= 50,			// 32  Location number for DOALL
	Flag_Objno     		= 51,			// 33  Current object (for _) referenced by WHATO etc.
	Flag_Strength  		= 52,			// 34  Maximum weight that can be carried (ABILITY)
	Flag_ListFlags		= 53,			// 35  Object list print flags
	Flag_ObjLocno  		= 54,			// 36  Location of the currently referenced object
	Flag_ObjWeight    	= 55,			// 37  Weight of the currently referenced object
	Flag_ObjContainer   = 56,			// 38  128 is current object is a container
	Flag_ObjWearable 	= 57,			// 39  128 if current object is wearable

	// The following flag has this meaning only in PAWS A16

	Flag_MatchPhrase    = 58,			// 3A  128 to perform verb/noun match in processes (PAWS A16 only)

	// The following flags are version 2 only

	Flag_ObjAttributes  = 58,			// 3A  (2 bytes) Attributes of the current object
	Flag_Key1      		= 60,			// 3C  Current ASCII key code after INKEY
	Flag_Key2      		= 61,			// 3D  Extended key code after INKEY
	Flag_ScreenMode 	= 62,			// 3E  2: Text, 4: CGA, 13 ($0D): EGA, 141 ($8D): VGA
	Flag_Window    		= 63,			// 3F  Which window is active at the moment

	// Flags 64 to 255 are unused
};

enum DDB_ObjectFlags
{
	Obj_Container = 0x40,
	Obj_Wearable  = 0x80,
	Obj_Weight    = 0x3F
};

enum DDB_GraphicsFlags
{
	Graphics_MousePresnet    = 0x01,
	Graphics_ColorBorder     = 0x08,
	Graphics_KeyAfterPicture = 0x10,
	Graphics_PicturesOff     = 0x20,
	Graphics_NoClsBeforeDesc = 0x40,
	Graphics_Available       = 0x80,

	// PAWS

	Graphics_Once            = 0x80,
	Graphics_On              = 0x40,
	Graphics_Off             = 0x20,

};

enum DDB_ObjectListFlags
{
	ListFlag_Found			= 0x80,
	ListFlag_Continuous		= 0x40,
	ListFlag_UnknownWord	= 0x20,		// 1 if an unknown word was found after the verb
	ListFlag_Preposition	= 0x10,		// 1 if a preposition was found before the first noun
	ListFlag_NoSuffixesHi	= 0x04,		// 1 to deactivate spanish pronoun terminations in verbs 240+
	ListFlag_AltHasAtRange	= 0x02,		// 1 to move HASAT to range 60-91
	ListFlag_DoallFailed	= 0x01,		// 1 if DOALL found no objects on start
};

enum DDB_WindowFlags
{
	Win_ForceGraphics     = 0x01,
	Win_NoMorePrompt      = 0x02,
	Win_Over              = 0x08,
	Win_Inverse           = 0x10,
	Win_Flash             = 0x20,
	Win_Bright            = 0x40,
	Win_ExpectingCodeByte = 0x80,
};

enum DDB_TimeoutFlags
{
	Timeout_Input      = 0x01,
	Timeout_MorePrompt = 0x02,
	Timeout_AnyKey     = 0x04,
	Timeout_LastFrame  = 0x80,
};

enum DDB_InputFlags
{
	Input_ClearWindow       = 0x01,
	Input_PrintAfterInput   = 0x02,
	Input_PrintAfterTimeout = 0x04,
};

enum DDB_Location
{
	Loc_Destroyed = 252,
	Loc_Worn      = 253,
	Loc_Carried   = 254,
	Loc_Here      = 255
};

enum DDB_ScreenMode
{
    ScreenMode_Default = 0x00,
	ScreenMode_Text    = 0x02,
	ScreenMode_CGA     = 0x04,
	ScreenMode_EGA     = 0x0D,
	ScreenMode_PCW     = 0x0F,		 // 640x200 2c
	ScreenMode_VGA16   = 0x8D,
	ScreenMode_VGA     = 0x9D,       // 320x200 256c
    ScreenMode_HiRes   = 0x90,       // 640x200 256c
    ScreenMode_SHiRes  = 0x91,       // 640x400 256c
};

enum DDB_DataFileModeFlags
{
	DDB_DataFileMode_CGA    = 0x01,
	DDB_DataFileMode_EGA    = 0x02,
	DDB_DataFileMode_VGA16  = 0x04,
	DDB_DataFileMode_VGA    = 0x08,
	DDB_DataFileMode_HiRes  = 0x10,
	DDB_DataFileMode_SHiRes = 0x20,
};

enum DDB_StartupVideoModePolicy
{
	DDB_StartupVideoModePolicy_Configurable,
	DDB_StartupVideoModePolicy_OverrideOrHighest,
};

enum DDB_Error
{
	DDB_ERROR_NONE,
	DDB_ERROR_FILE_NOT_FOUND,
	DDB_ERROR_READING_FILE,
	DDB_ERROR_CREATING_FILE,
	DDB_ERROR_SEEKING_FILE,
	DDB_ERROR_WRITING_FILE,
	DDB_ERROR_OUT_OF_MEMORY,
	DDB_ERROR_INVALID_FILE,
	DDB_ERROR_FILE_NOT_SUPPORTED,
	DDB_ERROR_VIDEO_MODE_NOT_SUPPORTED,
	DDB_ERROR_VIDEO_HARDWARE_NOT_SUPPORTED,
	DDB_ERROR_SDL,
	DDB_ERROR_NO_DDBS_FOUND,
};

enum DAT5_ColorMode
{
    DAT5_COLORMODE_UNIVERSAL = 0,
	DAT5_COLORMODE_CGA       = 1,
	DAT5_COLORMODE_EGA       = 2,
	DAT5_COLORMODE_PLANAR4   = 3,
	DAT5_COLORMODE_PLANAR5   = 4,
	DAT5_COLORMODE_PLANAR8   = 5,
	DAT5_COLORMODE_PLANAR4ST = 6,
	DAT5_COLORMODE_PLANAR8ST = 7,
	DAT5_COLORMODE_EHB6      = 8,
	DAT5_COLORMODE_HAM6      = 9,
	DAT5_COLORMODE_INDEXEDX  = 10,
	DAT5_COLORMODE_INDEXED   = 11,

	DAT5_COLORMODE_I16  = DAT5_COLORMODE_PLANAR4,
	DAT5_COLORMODE_I32  = DAT5_COLORMODE_PLANAR5,
	DAT5_COLORMODE_I256 = DAT5_COLORMODE_PLANAR8,
	DAT5_COLORMODE_VESA = DAT5_COLORMODE_INDEXED,
};

enum DDB_Condact
{
	CONDACT_ABILITY,
	CONDACT_ABSENT,
	CONDACT_ADD,
	CONDACT_ADJECT1,
	CONDACT_ADJECT2,
	CONDACT_ADVERB,
	CONDACT_ANYKEY,
	CONDACT_AT,
	CONDACT_ATGT,
	CONDACT_ATLT,
	CONDACT_AUTOD,
	CONDACT_AUTOG,
	CONDACT_AUTOP,
	CONDACT_AUTOR,
	CONDACT_AUTOT,
	CONDACT_AUTOW,
	CONDACT_BACKAT,
	CONDACT_BEEP,
	CONDACT_BIGGER,
	CONDACT_BORDER,
	CONDACT_CALL,
	CONDACT_CARRIED,
	CONDACT_CENTRE,
	CONDACT_CHANCE,
	CONDACT_CLEAR,
	CONDACT_CLS,
	CONDACT_COPYBF,
	CONDACT_COPYFF,
	CONDACT_COPYFO,
	CONDACT_COPYOF,
	CONDACT_COPYOO,
	CONDACT_CREATE,
	CONDACT_DESC,
	CONDACT_DESTROY,
	CONDACT_DISPLAY,
	CONDACT_DOALL,
	CONDACT_DONE,
	CONDACT_DPRINT,
	CONDACT_DROP,
	CONDACT_DROPALL,
	CONDACT_END,
	CONDACT_EQ,
	CONDACT_EXIT,
	CONDACT_EXTERN,
	CONDACT_GET,
	CONDACT_GFX,
	CONDACT_GOTO,
	CONDACT_GRAPHIC,
	CONDACT_GT,
	CONDACT_HASAT,
	CONDACT_HASNAT,
	CONDACT_INK,
	CONDACT_INKEY,
	CONDACT_INPUT,
	CONDACT_ISAT,
	CONDACT_ISDONE,
	CONDACT_ISNDONE,
	CONDACT_ISNOTAT,
	CONDACT_LET,
	CONDACT_LISTAT,
	CONDACT_LISTOBJ,
	CONDACT_LOAD,
	CONDACT_LT,
	CONDACT_MES,
	CONDACT_MESSAGE,
	CONDACT_MINUS,
	CONDACT_MODE,
	CONDACT_MOUSE,
	CONDACT_MOVE,
	CONDACT_NEWLINE,
	CONDACT_NEWTEXT,
	CONDACT_NOTAT,
	CONDACT_NOTCARR,
	CONDACT_NOTDONE,
	CONDACT_NOTEQ,
	CONDACT_NOTSAME,
	CONDACT_NOTWORN,
	CONDACT_NOTZERO,
	CONDACT_NOUN2,
	CONDACT_OK,
	CONDACT_PAPER,
	CONDACT_PARSE,
	CONDACT_PAUSE,
	CONDACT_PICTURE,
	CONDACT_PLACE,
	CONDACT_PLUS,
	CONDACT_PREP,
	CONDACT_PRESENT,
	CONDACT_PRINT,
	CONDACT_PRINTAT,
	CONDACT_PROCESS,
	CONDACT_PROMPT,
	CONDACT_PUTIN,
	CONDACT_PUTO,
	CONDACT_QUIT,
	CONDACT_RAMLOAD,
	CONDACT_RAMSAVE,
	CONDACT_RANDOM,
	CONDACT_REDO,
	CONDACT_REMOVE,
	CONDACT_RESET,
	CONDACT_RESTART,
	CONDACT_SAME,
	CONDACT_SAVE,
	CONDACT_SAVEAT,
	CONDACT_SET,
	CONDACT_SETCO,
	CONDACT_SFX,
	CONDACT_SKIP,
	CONDACT_SMALLER,
	CONDACT_SPACE,
	CONDACT_SUB,
	CONDACT_SWAP,
	CONDACT_SYNONYM,
	CONDACT_SYSMESS,
	CONDACT_TAB,
	CONDACT_TAKEOUT,
	CONDACT_TIME,
	CONDACT_TIMEOUT,
	CONDACT_TURNS,
	CONDACT_WEAR,
	CONDACT_WEIGH,
	CONDACT_WEIGHT,
	CONDACT_WHATO,
	CONDACT_WINAT,
	CONDACT_WINDOW,
	CONDACT_WINSIZE,
	CONDACT_WORN,
	CONDACT_ZERO,

	// PAWS-specific
	CONDACT_INVEN,
	CONDACT_SCORE,
	CONDACT_CHARSET,
	CONDACT_LINE,
	CONDACT_PROTECT,

	// DAAD V3
	CONDACT_INDIR,
	CONDACT_SETAT,
	CONDACT_XMESSAGE,

	CONDACT_INVALID,
};

struct DDB_CondactMap
{
	DDB_Condact	condact;
	uint8_t     parameters;
};

enum DDB_Version
{
	DDB_VERSION_PAWS = 0,
	DDB_VERSION_1    = 1,
	DDB_VERSION_2    = 2,
	DDB_VERSION_3    = 3,
};

static inline bool DDB_IsPAWS(const DDB_Version version)
{
	return version == DDB_VERSION_PAWS;
}

enum DDB_Machine
{
	DDB_MACHINE_IBMPC		= 0,
	DDB_MACHINE_SPECTRUM,
	DDB_MACHINE_C64,
	DDB_MACHINE_CPC,
	DDB_MACHINE_MSX,
	DDB_MACHINE_ATARIST,
	DDB_MACHINE_AMIGA,
	DDB_MACHINE_PCW,
	DDB_MACHINE_PLUS4		= 14,		// Commodore Plus/4
	DDB_MACHINE_MSX2
};

static inline bool DDB_Is16Bits(DDB_Machine m)
{
	return m == DDB_MACHINE_IBMPC ||
	       m == DDB_MACHINE_ATARIST ||
		   m == DDB_MACHINE_AMIGA;
}

static inline bool DDB_IsAmigaOrAtari(DDB_Machine m)
{
	return m == DDB_MACHINE_IBMPC ||
	       m == DDB_MACHINE_ATARIST ;
}

enum DDB_Language
{
	DDB_ENGLISH = 0,
	DDB_SPANISH = 1,
};

enum DDB_Flow
{
	FLOW_STARTING,
	FLOW_DESC,
	FLOW_AFTER_TURN,
	FLOW_INPUT,
	FLOW_RESPONSES,
};

enum DDB_HasAtOp
{
	HASAT_CLEAR = 0,
	HASAT_SET = 1,
	HASAT_TOGGLE = 2,
	HASAT_ISSET = -1,		// Internal use only
	HASAT_ISNOTSET = -2,
};

struct DDB
{
	DDB_Version		version;			// 1: Original/Jabato - 2: Later
	DDB_Machine     machine;            // Snapshot machine
	DDB_Machine		target;				// Target machine
	DDB_Language	language;			// Target language
	DDB_CondactMap*	condactMap;			// maps 0-127 to condacts

	bool			littleEndian;
	bool			oldMainLoop;
	bool			drawString;
	uint8_t			sdbMemoryModel;
	uint8_t			sdbSegmentCount;

	uint8_t			numObjects;
	uint8_t			numLocations;
	uint8_t			numMessages;
	uint8_t			numSystemMessages;
	uint8_t			numProcesses;
	uint8_t         numCharsets;
	uint8_t         nullWordChar;

	bool			hasTokens;
	uint8_t         firstToken;
	uint8_t*        tokens;
	uint8_t*		tokensPtr[128];
	size_t 			tokenBlockSize;

	uint16_t*		processTable;
	uint16_t*		msgTable;
	uint16_t*		sysMsgTable;
	uint16_t*		objNamTable;
	uint8_t*		objWordsTable;
	uint8_t*		objAttrTable;
	uint16_t*		objExAttrTable;
	uint8_t*		objLocTable;
	uint16_t*		locDescTable;
	uint16_t*		conTable;
	uint8_t*		vocabulary;
	uint8_t*        externData;
	uint16_t*       externPsgTable;
	uint8_t         externPsgCount;
	uint8_t*        charsets;
	uint8_t         curCharset;

	uint8_t*		messages[256];
	uint8_t*		locConnections[256];
	uint8_t*		locDescriptions[256];

	uint8_t         defaultBorder;
	uint8_t         defaultInk;
	uint8_t         defaultPaper;
	uint8_t         defaultCharset;

	// Data storage: all pointers above are required to point to this block

	uint8_t*		memory;
	uint8_t*		data;
	uint32_t		dataSize;
	uint16_t		baseOffset;
};

static inline bool DDB_DecodeStoredOffset(const DDB* ddb, uint16_t rawOffset, uint32_t limit, bool allowNull, uint32_t* offset)
{
	if (allowNull && rawOffset == 0)
	{
		if (offset != 0)
			*offset = 0;
		return true;
	}

	if (ddb->baseOffset != 0 && rawOffset >= ddb->baseOffset)
	{
		uint32_t relativeOffset = rawOffset - ddb->baseOffset;
		if (relativeOffset < limit)
		{
			if (offset != 0)
				*offset = relativeOffset;
			return true;
		}
	}

	if (rawOffset < limit)
	{
		if (offset != 0)
			*offset = rawOffset;
		return true;
	}

	return false;
}

static inline uint16_t DDB_EncodeStoredOffset(uint32_t offset, uint16_t storedBase)
{
	return (uint16_t)(offset + storedBase);
}

enum
{
	DDB_PSG_SAMPLE_RATE = 12500,
	DDB_PSG_TICK_HZ = 50,
	DDB_PSG_SAMPLES_PER_TICK = DDB_PSG_SAMPLE_RATE / DDB_PSG_TICK_HZ,
};

struct DDB_PSGState
{
	uint8_t regs[16];
	uint8_t tempRegister;
	double tonePhase[3];
	double toneStep[3];
	double noisePhase;
	double noiseStep;
	double envelopePhase;
	double envelopeStep;
	uint32_t lfsr;
	uint8_t envelopeLevel;
	int16_t smoothedMix;
	uint8_t rampRemaining;
	bool outputInitialized;
	bool envelopeContinue;
	bool envelopeAttack;
	bool envelopeAlternate;
	bool envelopeHold;
	bool envelopeHolding;
	bool envelopeAscending;
};

struct DDB_Window
{
	uint16_t		x;
	uint16_t		y;
	uint16_t		width;
	uint16_t		height;

	uint16_t		posX;
	uint16_t		posY;
	uint8_t			ink;
	uint8_t			paper;

	uint8_t			flags;
	uint16_t		saveX;
	uint16_t		saveY;

	bool			graphics;
	bool 			smooth;

	uint8_t			scrollCount;			// For More... prompt
};

typedef struct
{
	uint8_t			process;
	uint16_t		entry;
	uint16_t		offset;
}
DDB_ProcAddr;

typedef enum
{
	DDB_MSG,
	DDB_SYSMSG,
	DDB_OBJNAME,
	DDB_LOCDESC
}
DDB_MsgType;

typedef enum
{
	DDB_RUNNING,
	DDB_PAUSED,
	DDB_FINISHED,
	DDB_FATAL_ERROR,
	DDB_QUIT,
	DDB_WAITING_FOR_KEY,
	DDB_CHECKING_KEY,
	DDB_VSYNC,
	DDB_AUTOLOAD,

	// Remaining ones are input states
	DDB_INPUT,
	DDB_INPUT_QUIT,
	DDB_INPUT_END,
	DDB_INPUT_SAVE,
	DDB_INPUT_LOAD
}
DDB_State;

typedef enum
{
	SentenceFlag_UnknownWord 	= 0x01,
	SentenceFlag_Question		= 0x02,
	SentenceFlag_Colon			= 0x04,
}
DDB_SentenceFlags;

struct DDB_Interpreter
{
	DDB*			ddb;
	DDB_ScreenMode  screenMode;
	DDB_State		state;
	bool			autoloadEnabled;	// host player can reload parts on EXIT n
	uint8_t			autoloadPart;		// part requested by an EXIT n autoload
	DDB_Flow		oldMainLoopState;
	int32_t         pauseFrames;
	uint32_t        pauseStart;
	uint16_t        pauseTickRemainderMs;
	uint32_t        quitStart;
	#if HAS_TESTMODE
	bool            skipTimedPauses;
	#endif

	uint8_t*		buffer;
	size_t			bufferSize;
	uint16_t		saveStateSize;

	uint8_t			keyClick;
	bool			keyChecked;
	bool			keyCheckInProgress;
	bool			keyPressed;
	bool            saveKeyToFlags;
	uint32_t        lastClick;
	uint8_t         keyReuseCount;
	uint8_t			lastKey1;
	uint8_t			lastKey2;

	bool			timeout;
	int32_t			timeoutRemainingMs;
	uint16_t        timeoutTickRemainderMs;

	// State saved inside buffer

	uint8_t*	 	flags;			// Guaranteed to be equal to buffer
	uint8_t*		objloc;
	uint8_t*        visited;		// 32 bytes, one bit per location
	uint8_t*		ramSaveArea;
	bool            ramSaveAvailable;
	bool            restartPending;		// LOAD read a foreign save: RESTART after the key

	DDB_Window		win;
	DDB_Window		windef[8];

	uint8_t			inkMap[16];
	uint8_t			curwin;
	uint8_t			inputFlags;
	uint8_t         pawsControlParams;
	uint8_t         pawsPermanentInk;
	uint8_t         pawsPermanentPaper;
	uint8_t         pawsPermanentCharset;
	uint8_t         pawsPermanentFlags;
	uint8_t         pawsBorder;
	uint16_t        pawsRandomSeed;
	uint8_t			cellX;
	uint8_t			cellW;

			// ^ Bit 0: 1 to clear window after input
			//   Bit 1: 1 to print input line to current stream after input
			//   Bit 2: 1 to print input line to current stream after timeout

	uint8_t         prompt;

	uint8_t         currentPicture;

	DDB_ProcAddr	procstack[MAX_PROC_STACK];
	uint8_t			procstackptr;
	bool			doall;
	uint8_t         doallDepth;
	uint8_t         doallLocno;
	uint8_t         doallObjno;
	uint8_t         doallProcess;
	uint16_t        doallEntry;
	uint16_t        doallOffset;
	bool 			done;

	uint8_t			inputHistory[HISTORY_SIZE];
	uint16_t		inputHistoryLength;
	uint16_t		inputHistoryLastEntry;
	uint16_t		inputHistoryCurrentEntry;
	uint8_t			undoBuffer[UNDO_BUFFER_SIZE];
	uint16_t		undoBufferLength;
	uint16_t		undoBufferCurrentEntry;
	uint8_t			sentenceFlags;

	uint8_t 		inputBuffer[256];
	uint8_t			inputBufferPtr;
	uint8_t			inputBufferLength;
	uint8_t			inputCursorX;
	uint8_t         inputCompletionX;
	uint8_t*        quotedString;
	uint8_t			quotedStringLength;

	uint8_t			pending[128];
	uint8_t			pendingPtr;
	bool            suppressTranscript;
};

enum SCR_Operation
{
	SCR_OP_DRAWTEXT,
	SCR_OP_DRAWPICTURE
};

enum PlayerState
{
	Player_Starting,
	Player_SelectingPart,
	Player_ShowingScreen,
	Player_FadingOut,
	Player_InGame,
	Player_Finished,
	Player_Error,
};

extern DDB_Interpreter* interpreter;

typedef int (*DDB_PrintFunc)(const char* format, ...);

typedef struct
{
	bool includeMessageSamples;
	bool strictPAWCompatibility;
	bool rawTokens;					// Emit PAWS dictionary references as {n} instead of expanding them
}
DDB_DumpOptions;

extern DDB*				DDB_Load				 (const char* filename);
extern bool             DDB_Check                (const char* filename, DDB_Machine* target, DDB_Language* language, DDB_Version* version);
extern bool             DDB_RequiresBackBuffer   (DDB* ddb);
extern bool             DDB_FileRequiresBackBuffer(const char* filename);
extern bool             DDB_CheckVideoMode       (const char* fileName, DDB_ScreenMode* mode);
extern DDB_ScreenMode   DDB_GetDefaultScreenMode (DDB_Machine machine);
extern bool             DDB_CheckDataFileConfig  (const char* fileName, DDB_Machine target, DDB_ScreenMode* mode, uint8_t* planes);
extern uint32_t         DDB_GetDataFileModes     (const char* fileName, DDB_Machine target);
extern void             DDB_FlushDataFileProbeCache();
extern bool             DDB_ResolveDataFile      (const char* fileName, DDB_Machine target, DDB_ScreenMode requestedMode, char* resolvedFileName, size_t resolvedFileNameSize, DDB_ScreenMode* resolvedMode, uint8_t* planes);
extern DDB_Error        DDB_GetError             ();
extern DDB*             DDB_Create               ();
extern bool             DDB_SupportsDataFile     (DDB_Version ddb, DDB_Machine target);
extern bool				DDB_Write				 (DDB* ddb, const char* filename);
extern const char* 		DDB_GetDebugMessage 	 (DDB* ddb, DDB_MsgType type, uint8_t msgId);
extern bool             DDB_GetExternalPSGStreamRange(const DDB* ddb, uint8_t soundIndex, uint32_t* start, uint32_t* end);
extern bool             DDB_InitializePSGPlayback   ();
extern bool             DDB_EstimatePSGStreamTicks(const uint8_t* start, const uint8_t* end, uint32_t maxTicks, uint32_t* totalTicks);
extern bool             DDB_RenderPSGStream      (const uint8_t* start, const uint8_t* end, uint8_t* output, uint32_t totalTicks, DDB_PSGState* finalState);
extern void             DDB_RenderPSGTicks       (DDB_PSGState* state, uint8_t* output, uint32_t ticks);
extern bool             DDB_PlayExternalPSG      (DDB* ddb, uint8_t soundIndex);
extern const char*		DDB_GetMessage 			 (DDB* ddb, DDB_MsgType type, uint8_t msgId, char* buffer, size_t bufferSize);
extern void				DDB_Dump				 (DDB* ddb, DDB_PrintFunc print);
extern void				DDB_DumpWithOptions		 (DDB* ddb, DDB_PrintFunc print, const DDB_DumpOptions* options);
extern void				DDB_DumpMetrics			 (DDB* ddb, DDB_PrintFunc print);
extern void 			DDB_DumpProcess			 (DDB* ddb, uint8_t process, DDB_PrintFunc print);
extern void 			DDB_DumpProcessWithOptions(DDB* ddb, uint8_t process, DDB_PrintFunc print, const DDB_DumpOptions* options);
extern void				DDB_Close				 (DDB* ddb);

extern DDB_Interpreter* DDB_CreateInterpreter	 (DDB* ddb, DDB_ScreenMode mode);
extern void				DDB_Run					 (DDB_Interpreter* interpreter);
extern void				DDB_Step				 (DDB_Interpreter* interpreter, int lines);
extern void				DDB_Reset				 (DDB_Interpreter* interpreter);
extern void				DDB_ResetWindows		 (DDB_Interpreter* interpreter);
extern void				DDB_ResetPAWSColors	 (DDB_Interpreter* interpreter, DDB_Window* window);
extern void				DDB_Restart				 (DDB_Interpreter* interpreter);
extern void				DDB_SetAutoloadEnabled	 (DDB_Interpreter* interpreter, bool enabled);
#if HAS_TESTMODE
extern void				DDB_SetSkipTimedPauses (DDB_Interpreter* interpreter, bool skip);
#endif
extern void				DDB_CloseInterpreter	 (DDB_Interpreter* interpreter);

extern DDB_Error    	DDB_GetError             ();
extern void		    	DDB_SetError             (DDB_Error error);
extern const char*  	DDB_GetErrorString       ();
extern void         	DDB_SetWarningHandler    (void (*handler)(const char* message));
extern void             DDB_Warning              (const char* format, ...);
extern const char*      DDB_GetCondactName       (DDB_Condact condact);

extern void             DDB_Flush                (DDB_Interpreter* i);
extern void             DDB_FlushWindow          (DDB_Interpreter* i, DDB_Window* w);
extern void             DDB_ResetScrollCounts    (DDB_Interpreter* i);
extern void             DDB_ResetSmoothScrollFlags (DDB_Interpreter* i);
extern void             DDB_OutputUserPrompt     (DDB_Interpreter* i);
extern void             DDB_OutputInputPrompt    (DDB_Interpreter* i);
extern void             DDB_OutputChar           (DDB_Interpreter* i, const char c);
extern void             DDB_OutputText           (DDB_Interpreter* i, const char* text);
extern bool             DDB_OutputMessage        (DDB_Interpreter* i, DDB_MsgType type, uint8_t index);
extern bool             DDB_OutputMessageToWindow(DDB_Interpreter* i, DDB_MsgType type, uint8_t msgId, DDB_Window* w);
extern void             DDB_Desc                 (DDB_Interpreter* i, uint8_t locno);
extern bool             DDB_NewLine              (DDB_Interpreter* i);
extern bool             DDB_NewLineAtWindow      (DDB_Interpreter* i, DDB_Window* w);
extern bool             DDB_NextLine             (DDB_Interpreter* i);
extern bool             DDB_NextLineAtWindow     (DDB_Interpreter* i, DDB_Window* w);
extern void             DDB_ClearWindow          (DDB_Interpreter* i, DDB_Window* w);
extern void             DDB_SetWindow            (DDB_Interpreter* i, int winno);
extern void             DDB_PlayClick            (DDB_Interpreter* i, bool allowRepeats);
extern void             DDB_CalculateCells       (DDB_Interpreter* i, DDB_Window* w, uint8_t* cellX, uint8_t* cellW);
extern void             DDB_UseTranscriptFile    (const char* fileName);
extern void             DDB_TranscriptBreak      ();
extern void             DDB_TranscriptNewLine    ();
extern void             DDB_TranscriptWrite      (const void* text, size_t length);
extern void             DDB_TranscriptFlush      ();

extern void             DDB_ProcessInputFrame    ();
extern void             DDB_StartInput           (DDB_Interpreter* i, bool withPrompt);
extern void             DDB_FinishInput          (DDB_Interpreter * i, bool timeout);
extern void             DDB_PrintInputLine       (DDB_Interpreter* i, bool withCursor);
extern void             DDB_ResolveInputEnd      (DDB_Interpreter* i);
extern void             DDB_ResolveInputQuit     (DDB_Interpreter* i);
extern void             DDB_ResolveInputLoad     (DDB_Interpreter* i);
extern void             DDB_ResolveInputSave     (DDB_Interpreter* i);
extern void             DDB_NewText              (DDB_Interpreter* i);
extern DDB_Window*      DDB_GetInputWindow       (DDB_Interpreter* i);

extern PlayerState      DDB_RunPlayerAsync       (const char* location);
extern void             DDB_SetStartupVideoModePolicy(DDB_StartupVideoModePolicy policy);
extern void             DDB_SetStartupScreenModeOverride(DDB_ScreenMode mode);
extern void             DDB_ClearStartupScreenModeOverride();
extern bool             DDB_RunPlayer            ();
extern void             DDB_RestartAsyncPlayer   ();

extern bool             SCR_GetScreen            (const char* fileName, DDB_Machine target, uint8_t* buffer, size_t bufferSize, uint8_t* output, int width, int height, uint32_t* palette, bool* isHAM);

#if HAS_SNAPSHOTS
extern bool             DDB_LoadSnapshot         (File* file, const char* filename, uint8_t** ram, size_t* size, DDB_Machine* machine);
#endif

#if HAS_DRAWSTRING
extern bool             DDB_LoadVectorGraphics   (DDB_Machine machine, DDB_Version version, const uint8_t* data, size_t size);
extern bool             DDB_HasVectorPicture     (uint8_t picno);
extern bool             DDB_HasVectorWindow      (uint8_t picno);
extern const uint8_t*   DDB_GetVectorInkMap      ();
// Low-level command executor. Screen-producing code must enqueue pictures
// through SCR_DrawVectorPicture so their ordering is preserved.
extern bool             DDB_ExecuteVectorPicture (uint8_t picno);
extern bool             DDB_HasVectorDatabase    ();
extern bool             DDB_VectorPictureInRange (uint8_t picno);
extern bool             DDB_GetVectorPictureWindow (uint8_t picno, int* x, int* y, int* w, int* h);
extern bool             DDB_WriteVectorDatabase  (const char* filename);

#if HAS_PAWS
extern bool             DDB_LoadPAWSGraphics     (const uint8_t* data, size_t size);
extern void             DDB_LoadUDGs             ();
#endif

#endif

extern void 			DDB_SetCharset           (DDB* ddb, uint8_t c);
extern void             DDB_GetCurrentColors     (DDB* ddb, DDB_Window* w, uint8_t* ink, uint8_t* paper);

extern const char*      DDB_DescribeLanguage     (DDB_Language lang);
extern const char*      DDB_DescribeMachine      (DDB_Machine machine);
extern const char*      DDB_DescribeVersion      (DDB_Version version);

#endif
