//
//  8bitDetokeniser.h
//  Phantasma
//
//  Created by Thomas Harte on 15/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma___8bitDetokeniser__
#define __Phantasma___8bitDetokeniser__

#include "common/array.h"
#include "common/str.h"

#include "freescape/language/instruction.h"

Common::String *detokenise8bitCondition(Common::Array<uint8> &tokenisedCondition, FCLInstructionVector *instructions);

#endif /* defined(__Phantasma___8bitDetokeniser__) */
