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

#include "sword25/fmv/oggtheora/vorbisstate.h"

// -----------------------------------------------------------------------------

BS_VorbisState::BS_VorbisState() :
	m_DSPStateInitialized(false),
	m_BlockInitialized(false)
{
	vorbis_info_init(&m_Info);
	vorbis_comment_init(&m_Comment);
}

// -----------------------------------------------------------------------------

BS_VorbisState::~BS_VorbisState()
{
	if (m_BlockInitialized) vorbis_block_clear(&m_Block);
	if (m_DSPStateInitialized) vorbis_dsp_clear(&m_DSPState);
	vorbis_comment_clear(&m_Comment);
	vorbis_info_clear(&m_Info);
}

// -----------------------------------------------------------------------------

int BS_VorbisState::SynthesisHeaderIn(ogg_packet * OggPacketPtr)
{
	return vorbis_synthesis_headerin(&m_Info, &m_Comment, OggPacketPtr);
}

// -----------------------------------------------------------------------------

int BS_VorbisState::SynthesisInit()
{
	int Result = vorbis_synthesis_init(&m_DSPState, &m_Info);
	m_DSPStateInitialized = (Result == 0);
	return Result;
}

// -----------------------------------------------------------------------------

int BS_VorbisState::BlockInit()
{
	int Result = vorbis_block_init(&m_DSPState, &m_Block);
	m_BlockInitialized = (Result == 0);
	return Result;
}

// -----------------------------------------------------------------------------

int BS_VorbisState::SynthesisPCMout(float *** PCM)
{
	return vorbis_synthesis_pcmout(&m_DSPState, PCM);
}

// -----------------------------------------------------------------------------

int BS_VorbisState::SynthesisRead(int Samples)
{
	return vorbis_synthesis_read(&m_DSPState, Samples);
}

// -----------------------------------------------------------------------------

int BS_VorbisState::Synthesis(ogg_packet * OggPacketPtr)
{
	return vorbis_synthesis(&m_Block, OggPacketPtr);
}

// -----------------------------------------------------------------------------

int BS_VorbisState::SynthesisBlockIn()
{
	return vorbis_synthesis_blockin(&m_DSPState, &m_Block);
}
