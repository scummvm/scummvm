#include "globals.h"

namespace Glk {
namespace Scott {

Globals *g_globals;

Globals::Globals() : _sys(MAX_SYSMESS), _game(nullptr), _graphics(nullptr), _entireFile(nullptr), _fileLength(0),
					 _roomDescriptionStream(nullptr), _fileBaselineOffset(0), _lightRefill(0), _currentCommand(nullptr),
					 _bottomWindow(nullptr), _topWindow(nullptr), _drawToBuffer(0), _pixelSize(0),
					 _xOffset(0), _whiteColour(15), _blueColour(9), _diceColour(0xFF0000), _errorCount(0),
					 _palChosen(NO_PALETTE), _hulkCoordinates(0x26DB), _hulkItemImageOffsets(0x2798), _hulkLookImageOffsets(0x27BC),
					 _hulkSpecialImageOffsets(0x276E), _hulkImageOffset(0x441B), _vectorState(NO_VECTOR_IMAGE), _pixelsToDraw(nullptr),
					 _totalDrawInstructions(0), _currentDrawInstruction(0), _vectorImageShown(-1),
					 _pictureBitmap(nullptr), _lineColour(15), _bgColour(0), _scottGraphicsHeight(94),
					 _scottGraphicsWidth(255), _gliSlowDraw(0), _lastNoun(0), _firstErrorMessage(nullptr),
					 _unicodeWords(nullptr), _charWords(nullptr), _wordsInInput(0), _transcript(nullptr),
					 _directions(NUMBER_OF_DIRECTIONS), _extraNouns(NUMBER_OF_EXTRA_NOUNS), _skipList(NUMBER_OF_SKIPPABLE_WORDS),
					 _delimiterList(NUMBER_OF_DELIMITERS) {
	g_globals = this;
	_englishDirections = {nullptr, "north", "south", "east", "west", "up", "down", "n", "s", "e", "w", "u", "d", " "};
	_extraCommands = {nullptr, "restart", "save", "restore", "load", "transcript", "script", "oops", "undo", "ram",
						"ramload", "ramrestore", "ramsave", "except", "but", " ", " ", " ", " ", " "};
	_extraCommandsKey = { NO_COMMAND, RESTART, SAVE, RESTORE, RESTORE, SCRIPT, SCRIPT, UNDO, UNDO, RAM,
							RAMLOAD, RAMLOAD, RAMSAVE, EXCEPT, EXCEPT, RESTORE, RESTORE, SCRIPT, UNDO, RESTART};
	_englishExtraNouns = {nullptr, "game", "story", "on", "off", "load", "restore", "save", "move",
							"command", "turn", "all", "everything", "it", " ", " "};
	_abbreviations = {nullptr, "i", "l", "x", "z", "q"};
	_abbreviationsKey = {nullptr, "inventory", "look", "examine", "wait", "quit"};
	_englishSkipList = {nullptr, "at", "to", "in", "into", "the", "a", "an", "my", "quickly",
						"carefully", "quietly", "slowly", "violently", "fast", "hard", "now", "room"};
	_englishDelimiterList = {nullptr, ",", "and", "then", " "};
}

Globals::~Globals() {

}

} // End of namespace Scott
} // End of namespace Glk
