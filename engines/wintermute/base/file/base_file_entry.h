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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BFILEENTRY_H
#define WINTERMUTE_BFILEENTRY_H


#include "engines/wintermute/base/base.h"

namespace WinterMute {

class CBPackage;

class CBFileEntry : public CBBase {
public:
	uint32 _timeDate2;
	uint32 _timeDate1;
	uint32 _flags;
	uint32 _journalTime;
	Common::String _filename;
	uint32 _compressedLength;
	uint32 _length;
	uint32 _offset;
	CBPackage *_package;
	CBFileEntry(CBGame *inGame);
	virtual ~CBFileEntry();

};

} // end of namespace WinterMute

#endif
