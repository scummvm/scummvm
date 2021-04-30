//
//  8bitBinaryLoader.h
//  Phantasma
//
//  Created by X.
//  Copyright (c) X. All rights reserved.
//

#ifndef __Phantasma___8bitBinaryLoader__
#define __Phantasma___8bitBinaryLoader__

#include "common/str.h"

#include "../freescape.h"

namespace Freescape {

static float specColors[16][3] = {
    {0, 0, 0}, {0, 0, 0.75}, {0.75, 0, 0}, {0.75, 0, 0.75}, {0, 0.75, 0}, {0, 0.75, 0.75}, {0.75, 0.75, 0}, {0.75, 0.75, 0.75},
    {0, 0, 0}, {0, 0, 1}, {1, 0, 0}, {1, 0, 1}, {0, 1, 0}, {0, 1, 1}, {1, 1, 0}, {1, 1, 1}
};


Binary load8bitBinary(Common::String, uint offset);

}

#endif /* defined(__Phantasma___8bitBinaryLoader__) */