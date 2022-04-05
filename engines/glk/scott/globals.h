#ifndef GLK_SCOTT_GLOBALS
#define GLK_SCOTT_GLOBALS

#include "glk/glk_types.h"
#include "definitions.h"
#include "saga_draw.h"
#include "scott.h"
#include "line_drawing.h"
#include "glk/scott/command_parser.h"

namespace Glk {
namespace Scott {

class Globals {
public:
	//scott
	Header _gameHeader;
	Common::Array<Item> _items;
	Common::Array<Room> _rooms;
	Common::StringArray _verbs;
	Common::StringArray _nouns;
	Common::StringArray _messages;
	Common::Array<Action> _actions;
	Common::StringArray _sys;
	Common::StringArray _systemMessages;
	GameInfo *_game;
	winid_t _graphics;
	uint8_t *_entireFile;
	size_t _fileLength;
	strid_t _roomDescriptionStream;
	int _fileBaselineOffset;
	int _header[24];
	int _lightRefill;
	winid_t _bottomWindow, _topWindow;
	Command *_currentCommand;
	int _stopTime = 0;
	strid_t _transcript = nullptr;

	//sagadraw
	int _drawToBuffer;
	uint8_t _sprite[256][8];
	uint8_t _screenchars[768][8];
	uint8_t _buffer[384][9];
	Common::Array<Image> _images;
	int _pixelSize;
	int _xOffset;
	PALETTE _pal;
	int _whiteColour;
	int _blueColour;
	glui32 _diceColour;
	int32_t _errorCount;
	PaletteType _palChosen;
	size_t _hulkCoordinates;
	size_t _hulkItemImageOffsets;
	size_t _hulkLookImageOffsets;
	size_t _hulkSpecialImageOffsets;
	size_t _hulkImageOffset;


	//line_drawing
	Common::Array<LineImage> _lineImages;
	VectorStateType _vectorState;
	PixelToDraw **_pixelsToDraw;
	int _totalDrawInstructions;
	int _currentDrawInstruction;
	int _vectorImageShown;
	uint8_t *_pictureBitmap;
	int _lineColour;
	int _bgColour;
	int _scottGraphicsWidth;
	int _scottGraphicsHeight;

	//connect
	int _gliSlowDraw;

	//parser
	int _lastNoun;
	glui32 *_firstErrorMessage;
	glui32 **_unicodeWords = nullptr;
	char **_charWords = nullptr;
	int _wordsInInput = 0;
	Common::StringArray _directions;
	Common::StringArray _englishDirections;
	Common::StringArray _skipList;
	Common::StringArray _englishSkipList;
	Common::StringArray _delimiterList;
	Common::StringArray _englishDelimiterList;
	Common::StringArray _extraCommands;
	Common::StringArray _extraNouns;
	Common::StringArray _englishExtraNouns;
	Common::Array<ExtraCommand> _extraNounsKey;
	Common::Array<ExtraCommand> _extraCommandsKey;
	Common::StringArray _abbreviations;
	Common::StringArray _abbreviationsKey;

public:
	Globals();
	~Globals();
};

extern Globals *g_globals;

#define _G(FIELD) (g_globals->##FIELD)

} // End of namespace Scott
} // End of namespace Glk

#endif
