//
//  Instruction.h
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Instruction__
#define __Phantasma__Instruction__

#include "common/array.h"
#include "freescape/language/token.h"

class CGameState;

class FCLInstruction;
typedef Common::Array<FCLInstruction> FCLInstructionVector;

class FCLInstruction {
public:
	FCLInstruction();
	FCLInstruction(Token::Type type);
	void setSource(int32 _source);
	void setDestination(int32 _destination);

	Token::Type getType();
	void setBranches(FCLInstructionVector *thenBranch, FCLInstructionVector *elseBranch);

	int32 source;
	int32 destination;

	FCLInstructionVector *thenInstructions;
	FCLInstructionVector *elseInstructions;
private:
	enum Token::Type type;
};

#endif /* defined(__Phantasma__Instruction__) */
