//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#ifndef __AC_INTERFACEELEMENT_H
#define __AC_INTERFACEELEMENT_H

#include "ac/interfacebutton.h" // InterfaceButton

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

#endif // __AC_INTERFACEELEMENT_H