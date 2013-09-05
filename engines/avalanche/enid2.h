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

#ifndef ENID2_H
#define ENID2_H

#include "common/scummsys.h"


namespace Avalanche {
class AvalancheEngine;

class Enid {
public:
	Enid(AvalancheEngine *vm);

	void edna_save(Common::String name);

	void edna_load(Common::String name);

	void edna_reload();    // From Bootstrap's storage.

	void dir(Common::String where);

	void avvy_background();

	void back_to_bootstrap(byte what);

	bool there_was_a_problem();

private:
	AvalancheEngine *_vm;

	static const Common::String crlf;
	static const char tab;
	static const char eof_;

	static const Common::String ednafirst;

	static const Common::String ednaid;

	static const int16 ttage;
	static const Common::String ttwashere;

	typedef char fourtype[5];
	static const fourtype avaricius_file;



	bool bug;

	Common::String path, groi;

	Common::String month;
	Common::String day;



	void addon(Common::String x);
	Common::String expanddate(byte d, byte m, uint16 y);

	void show_bug(char icon, Common::String strn);
	bool test_bug(byte what);

	void loaderror(Common::String x, char icon);

	void showheader();

	void to_sundry(sundry &sund);
	void from_sundry(sundry sund);

	void restore_dna();

	
};

} // End of namespace Avalanche.

#endif // ENID2_H
