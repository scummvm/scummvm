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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/file.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "mortevielle/alert.h"
#include "mortevielle/disk.h"
#include "mortevielle/mor.h"
#include "mortevielle/mor2.h"
#include "mortevielle/mouse.h"
#include "mortevielle/ovd1.h"
#include "mortevielle/prog.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

/**
 * Ensure disk 1 data is available
 */
void dem1() {
/* Deprecated in ScummVM
	int k;

	// -- demande de disk 1 --           //Translation: Ask for disk #1
	assign(f, "mort.005");
	//*$i-*
	k = ioresult;
	reset(f);
	while (ioresult != 0) {
		show_mouse();
		k = do_alert(al_mess, 1);
		hide_mouse();
		reset(f);
	}
	close(f);
	*/
}

/**
 * Handle saving or loading savegame data
 */
static void sync_save(Common::Serializer &sz) {
	sz.syncAsSint16LE(s1.conf);
	sz.syncBytes((byte *)&s1.pourc[0], 11);
	sz.syncBytes((byte *)&s1.teauto[0], 43);
	sz.syncBytes((byte *)&s1.sjer[0], 31);
	sz.syncAsSint16LE(s1.mlieu);
	sz.syncAsSint16LE(s1.iboul);
	sz.syncAsSint16LE(s1.ibag);
	sz.syncAsSint16LE(s1.icave);
	sz.syncAsSint16LE(s1.ivier);
	sz.syncAsSint16LE(s1.ipuit);
	sz.syncAsSint16LE(s1.derobj);
	sz.syncAsSint16LE(s1.iloic);
	sz.syncAsSint16LE(s1.icryp);
	sz.syncAsSint16LE(s1.ipre);
	sz.syncAsSint16LE(s1.heure);

	sz.syncBytes(bufcha, 390);
}

void takesav(int n) {
	int i;
	Common::String st;

	dem1();
	// -- Load the file  'sauve#n.mor'
	Common::String saveName = Common::String::format("sav%d.mor", n);

	// Try loading first from the save area
	Common::SeekableReadStream *stream = g_system->getSavefileManager()->openForLoading(saveName);

	// If not present, try loading from the program folder
	Common::File f;
	if (stream == NULL) {
		if (!f.open(saveName))
			error("Unable to open save file '%s'", saveName);

		stream = f.readStream(f.size());
		f.close();
	}

	Common::Serializer sz(stream, NULL);
	sync_save(sz);

	s = s1;
	for (i = 0; i <= 389; i ++) tabdon[i + acha] = bufcha[i];

	// Close the stream
	delete stream;	
}

void ld_game(int n) {
	hide_mouse();
	maivid();
	takesav(n);
	/* -- disquette 2 -- */                    //Translation: Floppy #2
	dem2();
	/* -- mises en place -- */                 //Translation: Initialization
	theure();
	dprog();
	antegame();
	show_mouse();
}

void sv_game(int n) {
	Common::OutSaveFile *f;
	int i;

	hide_mouse();
	tmaj3();
	dem1();
	/* -- sauvegarde du fichier 'sauve#n.mor' -- */               //Translation: save file 'sauve%d.mor'
	for (i = 0; i <= 389; i ++) bufcha[i] = tabdon[i + acha];
	s1 = s;
	if (s1.mlieu == 26)  s1.mlieu = 15;
	
	Common::String saveName = Common::String::format("sav%d.mor", n);
	f = g_system->getSavefileManager()->openForSaving(saveName);

	Common::Serializer sz(NULL, f);
	sync_save(sz);

	f->finalize();
	delete f;

	dem2();
	show_mouse();
}

} // End of namespace Mortevielle
