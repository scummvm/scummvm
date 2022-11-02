/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PX_APIROUTINES_H__
#define ICB_PX_APIROUTINES_H__

#include "engines/icb/common/px_rcutypes.h"
// Interpreter return codes

namespace ICB {

enum mcodeFunctionReturnCodes { IR_STOP = 0, IR_CONT = 1, IR_TERMINATE = 2, IR_REPEAT = 3, IR_GOSUB = 4 };

#define NO_API_ROUTINES_ELDORADO 357
#define NO_API_ROUTINES_ICB 419

extern mcodeFunctionReturnCodes (*McodeTableICB[NO_API_ROUTINES_ICB])(int32 &, int32 *);
extern mcodeFunctionReturnCodes (*McodeTableED[NO_API_ROUTINES_ELDORADO])(int32 &, int32 *);

#define FN_ROUTINES_DATA_VERSION_ELDORADO 800750
#define FN_ROUTINES_DATA_VERSION_ICB 963730

} // End of namespace ICB

#endif
