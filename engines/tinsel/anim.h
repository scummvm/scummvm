/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Object animation definitions
 */

#ifndef TINSEL_ANIM_H     // prevent multiple includes
#define TINSEL_ANIM_H

#include "tinsel/dw.h"	// for SCNHANDLE

namespace Tinsel {

struct OBJECT;

/** animation structure */
struct ANIM {
	int aniRate;		//!< animation speed
	int aniDelta;		//!< animation speed delta counter
	OBJECT *pObject;	//!< object to animate (assumed to be multi-part)
	uint32 hScript;		//!< animation script handle
	int scriptIndex;	//!< current position in animation script
};
typedef ANIM *PANIM;


/*----------------------------------------------------------------------*\
|*			Anim Function Prototypes			*|
\*----------------------------------------------------------------------*/

/** states for DoNextFrame */
enum SCRIPTSTATE {ScriptFinished, ScriptNoSleep, ScriptSleep};

SCRIPTSTATE DoNextFrame(	// Execute the next animation frame of a animation script
	ANIM *pAnim);		// animation data structure

void InitStepAnimScript(	// Init a ANIM struct for single stepping through a animation script
	ANIM *pAnim,		// animation data structure
	OBJECT *pAniObj,	// object to animate
	SCNHANDLE hNewScript,	// handle to script of multipart frames
	int aniSpeed);		// sets speed of animation in frames

SCRIPTSTATE StepAnimScript(	// Execute the next command in a animation script
	ANIM *pAnim);		// animation data structure

void SkipFrames(		// Skip the specified number of frames
	ANIM *pAnim,		// animation data structure
	int numFrames);		// number of frames to skip

} // end of namespace Tinsel

#endif		// TINSEL_ANIM_H
