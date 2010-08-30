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

#ifndef SWORD25_VORBISSTATE_H
#define SWORD25_VORBISSTATE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include <vorbis/codec.h>

namespace Sword25 {

// -----------------------------------------------------------------------------
// Class definitions
// -----------------------------------------------------------------------------

class VorbisState {
public:
	VorbisState();
	virtual ~VorbisState();

	int	SynthesisHeaderIn(ogg_packet *OggPacketPtr);
	int SynthesisInit();
	int BlockInit();
	int SynthesisPCMout(float ***PCM);
	int SynthesisRead(int Samples);
	int Synthesis(ogg_packet *OggPacketPtr);
	int SynthesisBlockIn();

	const vorbis_info &GetInfo() const { return m_Info; }

private:
	vorbis_info					m_Info;
	bool						m_DSPStateInitialized;
	vorbis_dsp_state			m_DSPState;
	bool						m_BlockInitialized;
	vorbis_block				m_Block;
	vorbis_comment				m_Comment;
};

} // End of namespace Sword25

#endif
