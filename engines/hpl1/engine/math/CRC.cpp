/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "hpl1/engine/math/CRC.h"

//----------------------------------------

namespace hpl {

	cCRCTable cCRC::mTable;

	void cCRCTable::Init (tCRCKey aKey)
	{
		if (aKey == mKey) return;

		mKey = aKey;

		// for all possible byte values
		for (unsigned i = 0; i < 256; ++i)
		{
			tCRCKey reg = i << 24;
			// for all bits in a byte
			for (int j = 0; j < 8; ++j)
			{
				bool topBit = (reg & 0x80000000) != 0;
				reg <<= 1;
				if (topBit)
					reg ^= mKey;
			}
			mTable [i] = reg;
		}
	}

	//----------------------------------------

	void cCRC::PutByte (unsigned aByte)
	{
		unsigned top = mRegister >> 24;
		top ^= aByte;
		mRegister = (mRegister << 8) ^ mTable [top];
	}


	//----------------------------------------
}
