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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_DYNAMIC_BUFFER_H
#define WINTERMUTE_BASE_DYNAMIC_BUFFER_H

#include "engines/wintermute/base/base.h"

namespace Wintermute {

class BaseDynamicBuffer {
public:
	void putText(const char *fmt, ...);
	void putTextIndent(int indent, const char *fmt, ...);
	uint32 getDWORD();
	void putDWORD(uint32 val);
	char *getString();
	void putString(const char *val);
	bool getBytes(byte *buffer, uint32 size);
	bool putBytes(const byte *buffer, uint32 size);
	uint32 getSize() const;
	bool init(uint32 initSize = 0);
	void cleanup();
	BaseDynamicBuffer(BaseGame *inGame, uint32 initSize = 1000, uint32 growBy = 1000);
	virtual ~BaseDynamicBuffer();

private:
	uint32 _size;
	byte *_buffer;
	bool _initialized;
	uint32 _realSize;
	uint32 _growBy;
	uint32 _initSize;
	uint32 _offset;
	void putTextForm(const char *format, va_list argptr);
};

} // End of namespace Wintermute

#endif
