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

#ifndef BS_VORBISSTATE_H
#define BS_VORBISSTATE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "vorbis/codec.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_VorbisState
{
public:
	BS_VorbisState();
	virtual ~BS_VorbisState();

	int	SynthesisHeaderIn(ogg_packet * OggPacketPtr);
	int SynthesisInit();
	int BlockInit();
	int SynthesisPCMout(float *** PCM);
	int SynthesisRead(int Samples);
	int Synthesis(ogg_packet * OggPacketPtr);
	int SynthesisBlockIn();

	const vorbis_info & GetInfo() const { return m_Info; }

private:
	vorbis_info					m_Info;
	bool						m_DSPStateInitialized;
	vorbis_dsp_state			m_DSPState;
	bool						m_BlockInitialized;
	vorbis_block				m_Block;
	vorbis_comment				m_Comment;
};

#endif
