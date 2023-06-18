
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

#ifndef M4_GUI_GUI_VMNG_H
#define M4_GUI_GUI_VMNG_H

#include "m4/m4_types.h"

namespace M4 {

/**
 * To initialize the GUI view manager.
 * @returns		The success of the call
 * @remarks		Should be called once during program initialization,
 *				after dpmi_init_mem() has been called. 
 */
extern bool vmng_init();

/**
 * Shutdown the GUI view manager, and release all resources.
 *
 * Since applications can be written in which Dialogs are designed, created, and
 * Their callback procedures are executed by the view manager, it is not always
 * necessary for the application to retain the (Dialog*)s which it created.
 * Therefore, Any windows which contain a (Dialog*) and still exist also destroy
 * The Dialog for which they were created.  The same goes for (TextScrn*)s.
 */
void vmng_shutdown();

} // End of namespace M4

#endif
