//
//  Instruction.cpp
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "freescape/language/instruction.h"

FCLInstruction::FCLInstruction(Token::Type _type) {
	// TODO: learn modern constructor syntax
	type = _type;
	thenInstructions = nullptr;
	elseInstructions = nullptr;
}

FCLInstruction::FCLInstruction() {
	type = Token::UNKNOWN;
	thenInstructions = nullptr;
	elseInstructions = nullptr;
}

/*FCLInstruction::FCLInstruction(const FCLInstruction &source) {
	type = source.type;
	arguments.source = source.arguments.source;
	arguments.destination = source.arguments.destination;
	arguments.option = source.arguments.option;
	conditional.thenInstructions = source.conditional.thenInstructions;
	conditional.elseInstructions = source.conditional.elseInstructions;
}*/

/*
	Very routine setters for now; this code does not currently enforce good behaviour.
	TODO: allow mutation only once; delete supplied objects and raise an error if a
	second attempt at mutation is made
*/
void FCLInstruction::setSource(int32 _source) {
	source = _source;
}

void FCLInstruction::setDestination(int32 _destination) {
	destination = _destination;
}

void FCLInstruction::setBranches(FCLInstructionVector *thenBranch, FCLInstructionVector *elseBranch) {
	thenInstructions = thenBranch;
	elseInstructions = elseBranch;
}

/*
	Similarly routine getters...
*/
/*void FCLInstruction::getValue(CGameState &gameState, int32_t &source) {
	source = arguments.source.getValue(gameState, source);
}

void FCLInstruction::getValue(CGameState &gameState, int32_t &source, int32_t &destination) {
	source = arguments.source.getValue(gameState, source);
	destination = arguments.destination.getValue(gameState, destination);
}

void FCLInstruction::getValue(CGameState &gameState, int32_t &source, int32_t &destination, int32_t &option) {
	source = arguments.source.getValue(gameState, source);
	destination = arguments.destination.getValue(gameState, destination);
	option = arguments.option.getValue(gameState, option);
}*/

Token::Type FCLInstruction::getType() {
	return type;
}
