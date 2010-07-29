// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/fmv/oggtheora/oggstreamstate.h"

// -----------------------------------------------------------------------------

BS_OggStreamState::BS_OggStreamState(int SerialNo)
{
	ogg_stream_init(&m_State, SerialNo);
}

// -----------------------------------------------------------------------------

BS_OggStreamState::~BS_OggStreamState()
{
	ogg_stream_clear(&m_State);

	// Alle gepufferten Pages löschen.
	while (!m_PageBuffer.empty())
	{
		delete [] m_PageBuffer.front().header;
		delete [] m_PageBuffer.front().body;
		m_PageBuffer.pop();
	}
}

// -----------------------------------------------------------------------------

int BS_OggStreamState::PageIn(ogg_page * PagePtr)
{
	return ogg_stream_pagein(&m_State, PagePtr);
}

// -----------------------------------------------------------------------------

int BS_OggStreamState::PacketOut(ogg_packet * PacketPtr)
{
	return ogg_stream_packetout(&m_State, PacketPtr);
}

// -----------------------------------------------------------------------------

void BS_OggStreamState::BufferPage(ogg_page * PagePtr)
{
	if (PageBelongsToStream(PagePtr))
	{
		// Pages können nicht direkt gespeichert werden, da die Pointer im Laufe der Zeit ungültig werden.
		// Daher wird an dieser Stelle eine tiefe Kopie der Page im erzeugt und im Pagebuffer angelegt.
		ogg_page PageCopy;
		PageCopy.header_len = PagePtr->header_len;
		PageCopy.header = new unsigned char[PageCopy.header_len];
		memcpy(PageCopy.header, PagePtr->header, PageCopy.header_len);
		PageCopy.body_len = PagePtr->body_len;
		PageCopy.body = new unsigned char[PageCopy.body_len];
		memcpy(PageCopy.body, PagePtr->body, PageCopy.body_len);

		m_PageBuffer.push(PageCopy);
	}
}

// -----------------------------------------------------------------------------

int BS_OggStreamState::PageInBufferedPage()
{
	if (GetPageBufferSize() > 0)
	{
		// Page in den Stream einfügen, löschen und aus dem Puffer entfernen.
		int Result = PageIn(&m_PageBuffer.front());
		delete [] m_PageBuffer.front().header;
		delete [] m_PageBuffer.front().body;
		m_PageBuffer.pop();
		return Result;
	}

	return -1;
}

// -----------------------------------------------------------------------------

unsigned int BS_OggStreamState::GetPageBufferSize() const
{
	return m_PageBuffer.size();
}

// -----------------------------------------------------------------------------

unsigned int BS_OggStreamState::GetUnprocessedBytes() const
{
	return m_State.body_fill - m_State.body_returned;
}

// -----------------------------------------------------------------------------

bool BS_OggStreamState::PageBelongsToStream(ogg_page * PagePtr) const
{
	return m_State.serialno == ogg_page_serialno(PagePtr);
}
