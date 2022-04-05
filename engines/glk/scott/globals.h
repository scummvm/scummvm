#ifndef GLOBALS_H
#define GLOBALS_H

#include "glk/glk_types.h"

namespace Glk {
namespace Scott {

struct LineImage;
enum VectorStateType;
struct PixelToDraw;
struct Image;
enum PaletteType;
struct GameInfo;

typedef uint8_t RGB[3];
typedef RGB PALETTE[16];

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
	GameInfo *_game;
	winid_t _graphics;
	uint8_t *_entireFile;
	size_t _fileLength;
	strid_t _roomDescriptionStream;
	int _fileBaselineOffset;

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
	LineImage *_lineImages;
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

public:
	Globals();
	~Globals();
};

extern Globals *g_globals;

#define _G(FIELD) (g_globals->##FIELD)

} // End of namespace Scott
} // End of namespace Glk

#endif
