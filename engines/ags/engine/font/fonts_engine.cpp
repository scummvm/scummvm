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
//
// Implementation from acfonts.cpp specific to Engine runtime
//
//=============================================================================

#include <alfont.h>
#include "ac/gamesetupstruct.h"

extern int our_eip;
extern GameSetupStruct game;

//=============================================================================
// Engine-specific implementation split out of acfonts.cpp
//=============================================================================

void set_our_eip(int eip)
{
  our_eip = eip;
}

int get_our_eip()
{
  return our_eip;
}
