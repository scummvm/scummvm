/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __dgRandom__
#define __dgRandom__

#include "dgStdafx.h"

const dgFloat64 fRandom = (dgFloat64 (1.0) / dgFloat64 ((dgUnsigned32)(0xffffffff))); 

// return a random number between 0 and 0xffffffff;
dgUnsigned32 dgApi dgRandom();

inline dgFloat32 dgfRandom()
{
	return (dgFloat32) (dgRandom() * fRandom);
}


void dgApi dgRandomize (dgUnsigned32 Seed);

#endif

