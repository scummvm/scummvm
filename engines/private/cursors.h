#include "common/scummsys.h"
#include "common/str.h"
#include "common/hash-str.h"
#include "common/rect.h"

#ifndef PRIVATE_CURSORS_H
#define PRIVATE_CURSORS_H


namespace Private {

typedef Common::HashMap<Common::String, const byte*> CursorDataMap;
typedef Common::HashMap<Common::String, Common::Point*> CursorPointMap;

}

#endif