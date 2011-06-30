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

DAT *VFILE::_dat = NULL;
BtFile *VFILE::_cat = NULL;
VFILE *VFILE::_recent = NULL;

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
	_dat = new DAT();
#ifdef VOL_UPD
	_cat = new BtFile(CAT_NAME, UPD, CRP);
#else
	_cat = new BtFile(CAT_NAME, REA, CRP);
#endif

	_recent = NULL;
}

void VFILE::deinit() {
	delete _dat;
	delete _cat;
}

VFILE::VFILE(const char *name, IOMODE mode)
	: IoBuf(mode) {
	if (mode == REA) {
		if (_dat->_File.Error || _cat->Error)
			error("Bad volume data");
		BtKeypack *kp = _cat->find(name);
		if (scumm_stricmp(kp->_key, name) != 0)
			Error = 1;
		_endMark = (_bufMark = _begMark = kp->_mark) + kp->_size;
	}
#ifdef VOL_UPD
	else
		Make(name);
#endif
}


VFILE::~VFILE(void) {
	if (_recent == this)
		_recent = NULL;
}


bool VFILE::exist(const char *name) {
	return scumm_stricmp(_cat->find(name)->_key, name) == 0;
}


void VFILE::readBuff(void) {
	if (_recent != this) {
		_dat->_File.seek(_bufMark + _lim);
		_recent = this;
	}
	_bufMark = _dat->_File.mark();
	long n = _endMark - _bufMark;
	if (n > IOBUF_SIZE)
		n = IOBUF_SIZE;
	_lim = _dat->_File.read(_buff, (uint16) n);
	_ptr = 0;
}

} // End of namespace CGE
