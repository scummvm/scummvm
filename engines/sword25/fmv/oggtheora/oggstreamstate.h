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

#ifndef BS_OGGSTREAMSTATE_H
#define BS_OGGSTREAMSTATE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "ogg/ogg.h"
#include <queue>
// XXX
#include <iostream>
// XXX

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_OggStreamState
{
public:
	BS_OggStreamState(int SerialNo);
	virtual ~BS_OggStreamState();

	int PageIn(ogg_page * PagePtr);
	int PacketOut(ogg_packet * PacketPtr);

	void BufferPage(ogg_page * PagePtr);
	int PageInBufferedPage();
	unsigned int GetPageBufferSize() const;

	unsigned int GetUnprocessedBytes() const;
	bool PageBelongsToStream(ogg_page * PagePtr) const;

	// XXX
	void DumpInternals()
	{
		using namespace std;

		cout << "body_storage: " << m_State.body_storage << endl;
		cout << "body_fill: " << m_State.body_fill << endl;
		cout << "body_returned: " << m_State.body_returned << endl;
		cout << "lacing_storage: " << m_State.lacing_storage << endl;
		cout << "lacing_fill: " << m_State.lacing_fill << endl;
		cout << "lacing_returned: " << m_State.lacing_returned << endl;
		cout << endl;
	}
	// XXX

private:
	ogg_stream_state		m_State;
	std::queue<ogg_page>	m_PageBuffer;
};

#endif
