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
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* AVALOT		The kernel of the program. */

#include "avalanche/avalanche.h"

#include "avalanche/graph.h"
#include "avalanche/avalot.h"
#include "avalanche/gyro2.h"
#include "avalanche/trip6.h"
#include "avalanche/gyro2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/basher2.h"
#include "avalanche/dropdown2.h"
#include "avalanche/pingo2.h"
#include "avalanche/logger2.h"
#include "avalanche/timeout2.h"
#include "avalanche/celer2.h"
#include "avalanche/enid2.h"
#include "avalanche/visa2.h"
#include "avalanche/closing2.h"

/*#include "Incline.h"*/

#include "common/str.h"
#include "common/textconsole.h"



namespace Avalanche {

void Avalot::setup() {
	int16 gd, gm;

	checkbreak = false;
	_vm->_gyro.visible = _vm->_gyro.m_no;
	_vm->_gyro.to_do = 0;
	_vm->_gyro.lmo = false;
	_vm->_scrolls.resetscroll();
	_vm->_gyro.setup_vmc();
	_vm->_gyro.on_virtual();
	gd = 3;
	gm = 0;
	//initgraph(gd, gm, "");
	_vm->_gyro.holdthedawn = true;
	_vm->_lucerna.dusk();
	_vm->_gyro.cmp = 177;
	_vm->_lucerna.mouse_init();  /*on;*/
	_vm->_gyro.dropsok = true;
	_vm->_gyro.ctrl = ckey;
	_vm->_gyro.oldjw = 177;
	_vm->_gyro.mousetext = "";
	_vm->_gyro.c = 999;
	//settextjustify(0, 0);
	_vm->_gyro.ddmnow = false;
	_vm->_lucerna.load_digits();
	_vm->_gyro.cheat = false;
	_vm->_gyro.cp = 0;
	_vm->_gyro.curpos = 1;
	_vm->_gyro.quote = true;
	_vm->_gyro.ledstatus = 177;
	_vm->_gyro.defaultled = 2;
	/* TSkellern:=0; { Replace with a more local variable sometime }*/
	_vm->_gyro.dna.rw = _vm->_gyro.stopped;
	_vm->_gyro.enid_filename = ""; /* undefined. */
	_vm->_lucerna.toolbar();
	_vm->_scrolls.state(2);
	_vm->_pingo.copy03();
	for (byte i = 0; i < 3; i++)
		_vm->_gyro.lastscore[i] = -1; /* impossible digits */

	/* for gd:=0 to 1 do
	 begin
	  setactivepage(gd); outtextxy(7,177,chr(48+gd));
	 end;*/

	_vm->_trip.loadtrip();

	_vm->_gyro.reloaded = false; // TODO: Remove it later, when SAVE/LOAD system is implemented. Until then, we always start a new game.

	if ((_vm->_gyro.filetoload.empty()) && (! _vm->_gyro.reloaded))
		_vm->_gyro.newgame(); /* no game was requested- load the default */
	else {
		if (! _vm->_gyro.reloaded)
			_vm->_enid.avvy_background();
		_vm->_dropdown.standard_bar();
		_vm->_lucerna.sprite_run();
		if (_vm->_gyro.reloaded)
			_vm->_enid.edna_reload();
		else {
			/* Filename given on the command line (or loadfirst) */
			_vm->_enid.edna_load(_vm->_gyro.filetoload);
			if (_vm->_enid.there_was_a_problem()) {
				_vm->_scrolls.display("So let's start from the beginning instead...");
				_vm->_gyro.holdthedawn = true;
				_vm->_lucerna.dusk();
				_vm->_gyro.newgame();
			}
		}
	}

	if (! _vm->_gyro.reloaded) {
		_vm->_gyro.soundfx = ! _vm->_gyro.soundfx;
		_vm->_lucerna.fxtoggle();
		_vm->_lucerna.thinkabout(_vm->_gyro.money, _vm->_gyro.a_thing);
	}

	_vm->_trip.get_back_loretta();
	//gm = getpixel(0, 0);
	//setcolor(7);
	_vm->_gyro.holdthedawn = false;
	_vm->_lucerna.dawn();
	_vm->_gyro.cursoron = false;
	_vm->_basher.cursor_on();
	_vm->_trip.newspeed();

	if (! _vm->_gyro.reloaded)
		_vm->_visa.dixi('q', 83); /* Info on the game, etc. */
}

void Avalot::setParent(AvalancheEngine *vm) {
	_vm = vm;
}


void Avalot::run(Common::String arg) {
	setup();

	do {
		_vm->_lucerna.clock_lucerna();
		_vm->_basher.keyboard_link();
		_vm->_dropdown.menu_link();
		_vm->_trip.readstick();
		_vm->_gyro.force_numlock();
		_vm->_trip.get_back_loretta();
		_vm->_trip.trippancy_link();
		_vm->_celer.pics_link();
		_vm->_lucerna.checkclick();

		if (_vm->_gyro.visible == _vm->_gyro.m_virtual)
			_vm->_gyro.plot_vmc(_vm->_gyro.mx, _vm->_gyro.my, _vm->_gyro.cp);
		_vm->_lucerna.flip_page(); /* <<<! */
		_vm->_gyro.slowdown();
		if (_vm->_gyro.visible == _vm->_gyro.m_virtual) 
			_vm->_gyro.wipe_vmc(_vm->_gyro.cp);

		_vm->_timeout.one_tick();

		_vm->_graph.refreshScreen(); // TODO: Maybe it'll have a better place later. Move it there when it's needed.

	} while (false /*! _vm->_gyro.lmo*/); // So we run only one cycle during the testing. Of course it will be restored later.

	//restorecrtmode();
	//if (logging)
	//	close(logfile);

	_vm->_closing.end_of_program();
}

} // End of namespace Avalanche
