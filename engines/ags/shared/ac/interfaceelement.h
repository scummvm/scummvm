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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_SHARED_AC_INTERFACEELEMENT_H
#define AGS_SHARED_AC_INTERFACEELEMENT_H

#include "ags/shared/ac/interfacebutton.h" // InterfaceButton

namespace AGS3 {

// this struct should go in a Game struct, not the room structure.
struct InterfaceElement {
	int             x, y, x2, y2;
	int             bgcol, fgcol, bordercol;
	int             vtextxp, vtextyp, vtextalign;  // X & Y relative to topleft of interface
	char            vtext[40];
	int             numbuttons;
	InterfaceButton button[MAXBUTTON];
	int             flags;
	int             reserved_for_future;
	int             popupyp;   // pops up when mousey < this
	char            popup;     // does it pop up? (like sierra icon bar)
	char            on;
	InterfaceElement();
};

/*struct InterfaceStyle {
int  playareax1,playareay1,playareax2,playareay2; // where the game takes place
int  vtextxp,vtextyp;
char vtext[40];
int  numbuttons,popupbuttons;
InterfaceButton button[MAXBUTTON];
int  invx1,invy1,invx2,invy2;  // set invx1=-1 for Sierra-style inventory
InterfaceStyle() {  // sierra interface
playareax1=0; playareay1=13; playareax2=319; playareay2=199;
vtextxp=160; vtextyp=2; strcpy(vtext,"@SCORETEXT@$r@GAMENAME@");
invx1=-1; numbuttons=2; popupbuttons=1;
button[0].set(0,13,3,-1,0);
}
};*/

} // namespace AGS3

#endif
