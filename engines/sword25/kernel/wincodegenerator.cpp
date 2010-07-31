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
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/md5.h"
#include "sword25/kernel/wincodegenerator.h"
using namespace std;

// -----------------------------------------------------------------------------
// Hilfsfunktionen und Konstanten
// -----------------------------------------------------------------------------

namespace
{
	const char SECRET[] = "LSZNRVWQJHITMIEGESJMZAYVKGTCDT";

	// -------------------------------------------------------------------------

	string EncodeValue(unsigned int Value)
	{
		string Result;

		for (unsigned int i = 0; i < 7; ++i)
		{
			Result.push_back(65 + Value % 26);
			Value /= 26;
		}

		return Result;
	}
}

// -----------------------------------------------------------------------------

string BS_WinCodeGenerator::GetWinCode()
{
	// Die System-ID generieren und als String codieren.
	string SystemID = EncodeValue(GetSystemID());

	// Den Hashwert der System-ID mit dem geheimen String berechnen.
	BS_MD5 md5;
	string HashData = SystemID + SECRET;
	md5.Update(reinterpret_cast<const unsigned char *>(&HashData[0]), HashData.size());
	unsigned char Digest[16];
	md5.GetDigest(Digest);

	// Die ersten 32-Bit des Digest werden aus dem Digest extrahiert. Zudem wird das oberste Bit ausmaskiert.
	// So ist es einfacher den Code serverseitig zu überprüfen, da viele Scriptsprachen mit 32-Bit signed integern rechnen.
	unsigned int ValidationHash = ((Digest[3] & 0x7f) << 24) + (Digest[2] << 16) + (Digest[1] << 8) + Digest[0];
	
	// Der Code besteht aus der codierten System-ID und dem codierten Hash.
	return SystemID + EncodeValue(ValidationHash);
}
