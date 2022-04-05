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
					 _scottGraphicsWidth(255), _gliSlowDraw(0), _lastNoun(0), _firstErrorMessage(nullptr) {
	g_globals = this;
}

Globals::~Globals() {

}

} // End of namespace Scott
} // End of namespace Glk
