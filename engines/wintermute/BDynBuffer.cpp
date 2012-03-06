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

#include "BGame.h"
#include "BDynBuffer.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBDynBuffer::CBDynBuffer(CBGame *inGame, uint32 InitSize, uint32 GrowBy): CBBase(inGame) {
	m_Buffer = NULL;
	m_Size = 0;
	m_RealSize = 0;

	m_Offset = 0;
	m_InitSize = InitSize;
	m_GrowBy = GrowBy;

	m_Initialized = false;
}


//////////////////////////////////////////////////////////////////////////
CBDynBuffer::~CBDynBuffer() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::Cleanup() {
	if (m_Buffer) free(m_Buffer);
	m_Buffer = NULL;
	m_Size = 0;
	m_RealSize = 0;
	m_Offset = 0;
	m_Initialized = false;
}


//////////////////////////////////////////////////////////////////////////
uint32 CBDynBuffer::GetSize() {
	return m_Size;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDynBuffer::Init(uint32 InitSize) {
	Cleanup();

	if (InitSize == 0) InitSize = m_InitSize;

	m_Buffer = (byte  *)malloc(InitSize);
	if (!m_Buffer) {
		Game->LOG(0, "CBDynBuffer::Init - Error allocating %d bytes", InitSize);
		return E_FAIL;
	}

	m_RealSize = InitSize;
	m_Initialized = true;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDynBuffer::PutBytes(byte  *Buffer, uint32 Size) {
	if (!m_Initialized) Init();

	while (m_Offset + Size > m_RealSize) {
		m_RealSize += m_GrowBy;
		m_Buffer = (byte  *)realloc(m_Buffer, m_RealSize);
		if (!m_Buffer) {
			Game->LOG(0, "CBDynBuffer::PutBytes - Error reallocating buffer to %d bytes", m_RealSize);
			return E_FAIL;
		}
	}

	memcpy(m_Buffer + m_Offset, Buffer, Size);
	m_Offset += Size;
	m_Size += Size;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBDynBuffer::GetBytes(byte  *Buffer, uint32 Size) {
	if (!m_Initialized) Init();

	if (m_Offset + Size > m_Size) {
		Game->LOG(0, "CBDynBuffer::GetBytes - Buffer underflow");
		return E_FAIL;
	}

	memcpy(Buffer, m_Buffer + m_Offset, Size);
	m_Offset += Size;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::PutDWORD(uint32 Val) {
	PutBytes((byte  *)&Val, sizeof(uint32));
}


//////////////////////////////////////////////////////////////////////////
uint32 CBDynBuffer::GetDWORD() {
	uint32 ret;
	GetBytes((byte  *)&ret, sizeof(uint32));
	return ret;
}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::PutString(const char *Val) {
	if (!Val) PutString("(null)");
	else {
		PutDWORD(strlen(Val) + 1);
		PutBytes((byte  *)Val, strlen(Val) + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBDynBuffer::GetString() {
	uint32 len = GetDWORD();
	char *ret = (char *)(m_Buffer + m_Offset);
	m_Offset += len;

	if (!strcmp(ret, "(null)")) return NULL;
	else return ret;
}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::PutText(LPCSTR fmt, ...) {
	va_list va;

	va_start(va, fmt);
	PutTextForm(fmt, va);
	va_end(va);

}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::PutTextIndent(int Indent, LPCSTR fmt, ...) {
	va_list va;

	PutText("%*s", Indent, "");

	va_start(va, fmt);
	PutTextForm(fmt, va);
	va_end(va);
}


//////////////////////////////////////////////////////////////////////////
void CBDynBuffer::PutTextForm(const char *format, va_list argptr) {
	char buff[32768];
	vsprintf(buff, format, argptr);
	PutBytes((byte  *)buff, strlen(buff));
}

} // end of namespace WinterMute
