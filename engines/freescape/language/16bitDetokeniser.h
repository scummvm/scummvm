//
//  16bitDetokeniser.h
//  Phantasma
//
//  Created by Thomas Harte on 15/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma___16bitDetokeniser__
#define __Phantasma___16bitDetokeniser__

#include "common/array.h"
#include "common/str.h"

Common::String *detokenise16bitCondition(Common::Array<uint8> &tokenisedCondition);

#endif /* defined(__Phantasma___16bitDetokeniser__) */
