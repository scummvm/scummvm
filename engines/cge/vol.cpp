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

namespace CGE {

Dat *VFile::_dat = NULL;
BtFile *VFile::_cat = NULL;
VFile *VFile::_recent = NULL;

/*-----------------------------------------------------------------------*/

Dat::Dat():
#ifdef VOL_UPD
	_file(DAT_NAME, UPD, CRP)
#else
	_file(DAT_NAME, REA, CRP)
#endif
{
}

/*-----------------------------------------------------------------------*/

void VFile::init() {
	_dat = new Dat();
#ifdef VOL_UPD
	_cat = new BtFile(CAT_NAME, UPD, CRP);
#else
	_cat = new BtFile(CAT_NAME, REA, CRP);
#endif

	_recent = NULL;
}

void VFile::deinit() {
	delete _dat;
	delete _cat;
}

VFile::VFile(const char *name, IOMODE mode)
	: IoBuf(mode) {
	if (mode == REA) {
		if (_dat->_file._error || _cat->_error)
			error("Bad volume data");
		BtKeypack *kp = _cat->find(name);
		if (scumm_stricmp(kp->_key, name) != 0)
			_error = 1;
		_endMark = (_bufMark = _begMark = kp->_mark) + kp->_size;
	}
#ifdef VOL_UPD
	else
		Make(name);
#endif
}


VFile::~VFile() {
	if (_recent == this)
		_recent = NULL;
}


bool VFile::exist(const char *name) {
	return scumm_stricmp(_cat->find(name)->_key, name) == 0;
}


void VFile::readBuff() {
	if (_recent != this) {
		_dat->_file.seek(_bufMark + _lim);
		_recent = this;
	}
	_bufMark = _dat->_file.mark();
	long n = _endMark - _bufMark;
	if (n > IOBUF_SIZE)
		n = IOBUF_SIZE;
	_lim = _dat->_file.read(_buff, (uint16) n);
	_ptr = 0;
}

long VFile::mark() {
	return (_bufMark + _ptr) - _begMark;
}

long VFile::size() {
	return _endMark - _begMark;
}

long VFile::seek(long pos) {
	_recent = NULL;
	_lim = 0;
	return (_bufMark = _begMark + pos);
}

} // End of namespace CGE
