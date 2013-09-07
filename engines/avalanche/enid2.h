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
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* ENID		Edna's manager. Loads/saves files. */

#ifndef AVALANCHE_ENID2_H
#define AVALANCHE_ENID2_H

#include "common/scummsys.h"

namespace Avalanche {
class AvalancheEngine;

class Enid {
public:
	Enid(AvalancheEngine *vm);
	void dir(Common::String where);
	void backToBootstrap(byte what);

private:
	AvalancheEngine *_vm;

//  CHECKME: useless?
//	static const Common::String kCrlf;
//	static const char kTab, kEof;
//	static const Common::String kEdnaFirst, kEdnaId, kWasHere;
//	static const int16 kAge;
//	void expandMonthName(Common::String x);
//	Common::String expandDate(byte d, byte m, uint16 y);
//	void ednaSave(Common::String name);
//	void showBug(char icon, Common::String strn);
//	bool testBug(byte what);
//	void loadError(Common::String x, char icon);
//	void ednaLoad(Common::String name);
//	void showHeader();
//	void toSundry(sundry &sund);
//	void fromSundry(sundry sund);
//	void avvyBackground();
//	void ednaReload(); // From Bootstrap's storage.
//	void restoreDna();
//	bool thereWasAProblem();

//	typedef char FourType[5];
//	static const FourType kAvariciusFile;

//	bool _bug;
//	Common::String _path, _groi;
//	Common::String _month, _day;

};

} // End of namespace Avalanche.

#endif // AVALANCHE_ENID2_H
