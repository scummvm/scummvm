/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 Andreas 'Sprawl' Karlsson - Original EPOC port, ESDL
 * Copyright (C) 2003-2005 Lars 'AnotherGuest' Persson - Original EPOC port, Audio System
 * Copyright (C) 2005 Jurgen 'SumthinWicked' Braam - EPOC/CVS maintainer
 * Copyright (C) 2005 The ScummVM project
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
 * $Header$
 */

#include "backends/epoc/ScummApp.h"

#include "backends/epoc/ScummVM.hrh"

#ifdef __WINS__
extern "C" int _chkstk(int /*a*/) {
	return 1;
}
#endif

#ifdef EPOC_AS_APP
// this function is called by Symbian to deliver the new CApaApplication object
EXPORT_C CApaApplication *NewApplication() {
	// Return pointer to newly created CQMApp
	return (new CScummApp);
}
#endif

GLDEF_C  TInt E32Dll(TDllReason) {
	return KErrNone;
}

CScummApp::CScummApp() {
}

CScummApp::~CScummApp() {
}

/**
 *   Responsible for returning the unique UID of this application
 * @return unique UID for this application in a TUid
 **/
TUid CScummApp::AppDllUid() const {
	return TUid::Uid(ScummUid);
}

/////////////////////////////////////////////////////////////////////////////////////////////////


