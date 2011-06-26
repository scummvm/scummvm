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
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/vol.h"
#include "common/system.h"
#include "common/str.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

namespace CGE {

DAT *VFILE::_Dat = NULL;
BTFILE *VFILE::_Cat = NULL;
VFILE *VFILE::_Recent = NULL;

/*-----------------------------------------------------------------------*/

DAT::DAT():
#ifdef VOL_UPD
	_File(DAT_NAME, UPD, CRP)
#else
	_File(DAT_NAME, REA, CRP)
#endif
{
}

/*-----------------------------------------------------------------------*/

void VFILE::init() {
	_Dat = new DAT();
#ifdef VOL_UPD
	_Cat = new BTFILE(CAT_NAME, UPD, CRP);
#else
	_Cat = new BTFILE(CAT_NAME, REA, CRP);
#endif

	_Recent = NULL;
}

void VFILE::deinit() {
	delete _Dat;
	delete _Cat;
}

VFILE::VFILE(const char *name, IOMODE mode)
	: IOBUF(mode) {
	if (mode == REA) {
		if (_Dat->_File.Error || _Cat->Error)
			error("Bad volume data");
		BT_KEYPACK *kp = _Cat->Find(name);
		if (scumm_stricmp(kp->Key, name) != 0)
			Error = 1;
		EndMark = (BufMark = BegMark = kp->Mark) + kp->Size;
	}
#ifdef VOL_UPD
	else
		Make(name);
#endif
}


VFILE::~VFILE(void) {
	if (_Recent == this)
		_Recent = NULL;
}


bool VFILE::Exist(const char *name) {
	return scumm_stricmp(_Cat->Find(name)->Key, name) == 0;
}


void VFILE::ReadBuff(void) {
	if (_Recent != this) {
		_Dat->_File.Seek(BufMark + Lim);
		_Recent = this;
	}
	BufMark = _Dat->_File.Mark();
	long n = EndMark - BufMark;
	if (n > IOBUF_SIZE)
		n = IOBUF_SIZE;
	Lim = _Dat->_File.Read(Buff, (uint16) n);
	Ptr = 0;
}

} // End of namespace CGE
