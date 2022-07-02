//
//  8bitDetokeniser.h
//  Phantasma
//
//  Created by Thomas Harte on 15/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef FREESCAPE_8BITDETOKENIZER_H
#define FREESCAPE_8BITDETOKENIZER_H

#include "common/array.h"
#include "common/str.h"

#include "freescape/language/instruction.h"

namespace Freescape {

Common::String *detokenise8bitCondition(Common::Array<uint8> &tokenisedCondition, FCLInstructionVector &instructions);

} // End of namespace Freescape

#endif /* defined(__Phantasma___8bitDetokeniser__) */
