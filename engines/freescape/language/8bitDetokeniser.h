/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FREESCAPE_8BITDETOKENIZER_H
#define FREESCAPE_8BITDETOKENIZER_H

#include "freescape/language/instruction.h"

namespace Freescape {

enum {
	k8bitGameBitTravelRock = 30
};

enum {
	k8bitVariableCrawling = 30,
	k8bitVariableSpiritsDestroyed = 28,
	k8bitVariableEnergy = 62,
	k8bitVariableScore = 61,
	k8bitVariableShieldDrillerTank = 60,
	k8bitVariableEnergyDrillerTank = 59,
	k8bitVariableShieldDrillerJet = 58,
	k8bitVariableEnergyDrillerJet = 57,
	k8bitMaxVariable = 64
};

enum {
	kConditionalShot = 1 << 0,
	kConditionalTimeout = 1 << 1,
	kConditionalCollided = 1 << 2,
	kConditionalActivated = 1 << 3,
};

extern uint8 k8bitVariableShield;

Common::String detokenise8bitCondition(Common::Array<uint16> &tokenisedCondition, FCLInstructionVector &instructions, bool enableActivated);

} // End of namespace Freescape

#endif // FREESCAPE_8BITDETOKENIZER_H
