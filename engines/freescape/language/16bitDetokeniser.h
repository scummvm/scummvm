//
//  16bitDetokeniser.h
//  Phantasma
//
//  Created by Thomas Harte on 15/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma___6bitDetokeniser__
#define __Phantasma___6bitDetokeniser__

#include "common/str.h"
#include "common/array.h"

Common::String *detokenise16bitCondition(Common::Array <uint8> &tokenisedCondition);

#endif /* defined(__Phantasma___6bitDetokeniser__) */
