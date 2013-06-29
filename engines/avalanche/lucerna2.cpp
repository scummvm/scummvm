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

/* LUCERNA		The screen, [keyboard] and mouse handler.*/

#include "avalanche/avalanche.h"

#include "common/system.h"
#include "avalanche/lucerna2.h"
#include "avalanche/gyro2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/logger2.h"
#include "avalanche/enhanced2.h"


#include "avalanche/visa2.h"
#include "avalanche/timeout2.h"
#include "avalanche/trip6.h"
#include "avalanche/enid2.h"
#include "avalanche/celer2.h"
#include "avalanche/pingo2.h"
#include "avalanche/sequence2.h"
//#include "avalanche/acci2.h"


//#include "dropdown.h"
//#include "basher.h"


namespace Avalanche {

Lucerna::Lucerna() {
	// Will be needed.
}

void Lucerna::setParent(AvalancheEngine *vm) {
	_vm = vm;
}
	
void Lucerna::callverb(char n) {
	/*if (n == pardon)
	Scrolls::display(
	"The f5 key lets you do a particular action in certain "
	"situations. However, at the moment there is nothing "
	"assigned to it. You may press alt-A to see what the "
	"current setting of this key is.");*/

	// Needs const char pardon located in Acci.

}



} // End of namespace Avalanche
