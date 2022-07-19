//
//  Instruction.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef FREESCAPE_INSTRUCTION_H
#define FREESCAPE_INSTRUCTION_H

#include "common/array.h"
#include "freescape/language/token.h"

namespace Freescape {

class FCLInstruction;
typedef Common::Array<FCLInstruction> FCLInstructionVector;

class FCLInstruction {
public:
	FCLInstruction();
	FCLInstruction(Token::Type type);
	void setSource(int32 _source);
	void setAdditional(int32 _additional);
	void setDestination(int32 _destination);

	Token::Type getType();
	void setBranches(FCLInstructionVector *thenBranch, FCLInstructionVector *elseBranch);

	int32 source;
	int32 additional;
	int32 destination;

	FCLInstructionVector *thenInstructions;
	FCLInstructionVector *elseInstructions;
private:
	enum Token::Type type;
};

} // End of namespace Freescape

#endif
