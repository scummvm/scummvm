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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_SPELVALS_H
#define SAGA2_SPELVALS_H

namespace Saga2 {

// constants affecting all or most of the spells

const uint32 SpellJumpiness = 10;
const uint16 spellSpeed = 1;

// Bolt spell dimensions
const int16 boltSpellLength = 192;
const int16 boltSpellWidth = boltSpellLength / 6;

// Bolt spell dimensions
const int16 beamSpellLength = 192;
const int16 beamSpellWidth = beamSpellLength / 12;

// Cone spell dimensions
const int16 coneSpellLength = 128;
const int16 coneSpellWidth = coneSpellLength / 2;

// Wide Cone spell dimensions
const int16 waveSpellLength = 128;
const int16 waveSpellWidth = waveSpellLength;

// small ball spell dimensions
const int16 ballSpellRadius = 48;

// small ball spell dimensions
const int16 squareSpellSize = 48;

// large ball spell dimensions
const int16 stormSpellRadius = 64;

// small ball spell dimensions
const int16 wallSpellRadius = 32;
const int16 wallInnerRadius = 16;

/* ===================================================================== *
   Imports
 * ===================================================================== */

// ball shaped spell shapes
extern StaticTilePoint WallVectors[];
extern StaticTilePoint FireballVectors[];
extern StaticTilePoint SquareSpellVectors[];

/* ===================================================================== *
   Spell shape math explanations
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Fireball vectors
//
//         X0X           Vectors are numbered from 0 in clockwise order
//       XX   XX
//       X     X
//      X       X         ^
//      X   +   X         |
//      X       X         U
//       X     X          +V -->
//       XX   XX
//         XXX
//

//	SquareSpell vectors
//
//      XXXX0XXXX        Vectors are numbered from 0 in clockwise order
//      X       X
//      X       X
//      X       X         ^
//      X   +   X         |
//      X       X         U
//      X       X         +V -->
//      X       X
//      XXXXXXXXX
//

//
//  Cone spell vectors
//
//            X     targetVector+orth(targetVector)/2
//            X
//            X
//   +  ->    X     targetVector
//            X
//            X
//            X     targetVector-orth(targetVector)/2
//
//   Wide Cone is similar, but bound by
//     targetVector+orth(targetVector)
//     targetVector-orth(targetVector)
//

//  Bolt spell Vectors
//
//  OrthoVector=ortho(TargetVector){mag = boltWidth/2}
//
//
//     XXXXXXX     = projectile +
//    +XXXXXXX     = projectile     -> target vector
//     XXXXXXX     = projectile -
//
//

//-----------------------------------------------------------------------

} // end of namespace Saga2

#endif
