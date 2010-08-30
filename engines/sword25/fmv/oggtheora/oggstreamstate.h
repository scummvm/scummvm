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
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_OGGSTREAMSTATE_H
#define SWORD25_OGGSTREAMSTATE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include <ogg/ogg.h>
#include "common/queue.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Class definitions
// -----------------------------------------------------------------------------

class OggStreamState {
public:
	OggStreamState(int SerialNo);
	virtual ~OggStreamState();

	int PageIn(ogg_page *PagePtr);
	int PacketOut(ogg_packet *PacketPtr);

	void BufferPage(ogg_page *PagePtr);
	int PageInBufferedPage();
	unsigned int GetPageBufferSize() const;

	unsigned int GetUnprocessedBytes() const;
	bool PageBelongsToStream(ogg_page *PagePtr) const;

	// XXX
	void DumpInternals() {
		warning("body_storage: %d", m_State.body_storage);
		warning("body_fill: %d", m_State.body_fill);
		warning("body_returned: %d", m_State.body_returned);
		warning("lacing_storage: %d", m_State.lacing_storage);
		warning("lacing_fill: %d", m_State.lacing_fill);
		warning("lacing_returned: %d", m_State.lacing_returned);
	}
	// XXX

private:
	ogg_stream_state		m_State;
	Common::Queue<ogg_page>	m_PageBuffer;
};

} // End of namespace Sword25

#endif
