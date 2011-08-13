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
#include "cge/cge.h"
#include "common/debug.h"
#include "common/debug-channels.h"

namespace CGE {

Dat *VFile::_dat = NULL;
BtFile *VFile::_cat = NULL;
VFile *VFile::_recent = NULL;

/*-----------------------------------------------------------------------*/

Dat::Dat(): _file(DAT_NAME, kModeRead, CRP) {
	debugC(1, kCGEDebugFile, "Dat::Dat()");
}

/*-----------------------------------------------------------------------*/

void VFile::init() {
	debugC(1, kCGEDebugFile, "VFile::init()");

	_dat = new Dat();
	_cat = new BtFile(CAT_NAME, kModeRead, CRP);
	_recent = NULL;
}

void VFile::deinit() {
	delete _dat;
	delete _cat;
}

VFile::VFile(const char *name, IOMode mode)
	: IoBuf(mode, NULL) {
	debugC(3, kCGEDebugFile, "VFile::VFile(%s, %d)", name, mode);

	if (mode == kModeRead) {
		if (_dat->_file._error || _cat->_error)
			error("Bad volume data");
		BtKeypack *kp = _cat->find(name);
		if (scumm_stricmp(kp->_key, name) != 0)
			_error = 1;
		_endMark = (_bufMark = _begMark = kp->_mark) + kp->_size;
	}
}


VFile::~VFile() {
	if (_recent == this)
		_recent = NULL;
}


bool VFile::exist(const char *name) {
	debugC(1, kCGEDebugFile, "VFile::exist(%s)", name);

	return scumm_stricmp(_cat->find(name)->_key, name) == 0;
}


void VFile::readBuf() {
	debugC(3, kCGEDebugFile, "VFile::readBuf()");

	if (_recent != this) {
		_dat->_file.seek(_bufMark + _lim);
		_recent = this;
	}
	_bufMark = _dat->_file.mark();
	long n = _endMark - _bufMark;
	if (n > kBufferSize)
		n = kBufferSize;
	_lim = _dat->_file.read(_buff, (uint16) n);
	_ptr = 0;
}

long VFile::mark() {
	debugC(5, kCGEDebugFile, "VFile::mark()");

	return (_bufMark + _ptr) - _begMark;
}

long VFile::size() {
	debugC(1, kCGEDebugFile, "VFile::size()");

	return _endMark - _begMark;
}

long VFile::seek(long pos) {
	debugC(1, kCGEDebugFile, "VFile::seel(%ld)", pos);

	_recent = NULL;
	_lim = 0;
	return (_bufMark = _begMark + pos);
}

} // End of namespace CGE
