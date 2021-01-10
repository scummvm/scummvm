#include "common/scummsys.h"
#include "common/str.h"
#include "common/hash-str.h"

#ifndef PRIVATE_CURSORS_H
#define PRIVATE_CURSORS_H

typedef Common::HashMap<Common::String, const byte*> CursorMap;

namespace Private {

extern const byte MOUSECURSOR_SCI[];
extern const byte MOUSECURSOR_kExit[];
extern const byte MOUSECURSOR_kZoomIn[];
extern const byte MOUSECURSOR_kZoomOut[];
extern const byte MOUSECURSOR_kInventory[];
extern const byte MOUSECURSOR_kTurnLeft[];
extern const byte MOUSECURSOR_kTurnRight[];
extern const byte MOUSECURSOR_kPhone[];

extern const byte cursorPalette[];
extern void initCursors();

extern CursorMap _cursors;

}

#endif