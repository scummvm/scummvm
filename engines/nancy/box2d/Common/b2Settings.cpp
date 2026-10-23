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

/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

/*
 * Altered for ScummVM: b2Alloc keeps its memory aligned for 64-bit systems.
 */

#include "b2Settings.h"

b2Version b2_version = {2, 0, 1};

int32 b2_byteCount = 0;



// Memory allocators. Modify these to use your own allocator.
static const int32 b2_allocHeaderSize = 16;

void* b2Alloc(int32 size)
{
	size += b2_allocHeaderSize;
	b2_byteCount += size;
	char* bytes = (char*)malloc(size);
	*(int32*)bytes = size;
	return bytes + b2_allocHeaderSize;
}

void b2Free(void* mem)
{
	if (mem == NULL)
	{
		return;
	}

	char* bytes = (char*)mem;
	bytes -= b2_allocHeaderSize;
	int32 size = *(int32*)bytes;
	b2Assert(b2_byteCount >= size);
	b2_byteCount -= size;
	free(bytes);
}
