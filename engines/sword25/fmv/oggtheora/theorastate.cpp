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

#include "theorastate.h"

// -----------------------------------------------------------------------------

BS_TheoraState::BS_TheoraState() :
	m_StateInitialized(false)
{
	theora_comment_init(&m_Comment);
	theora_info_init(&m_Info);
}

// -----------------------------------------------------------------------------

BS_TheoraState::~BS_TheoraState()
{
	if (m_StateInitialized) theora_clear(&m_State);
	theora_info_clear(&m_Info);
	theora_comment_clear(&m_Comment);
}

// -----------------------------------------------------------------------------

int BS_TheoraState::DecodeHeader(ogg_packet * OggPacketPtr)
{
	return theora_decode_header(&m_Info, &m_Comment, OggPacketPtr);
}

// -----------------------------------------------------------------------------

int BS_TheoraState::DecodeInit()
{
	int Result = theora_decode_init(&m_State, &m_Info);
	m_StateInitialized = (Result == 0);
	return Result;
}

// -----------------------------------------------------------------------------

double BS_TheoraState::GranuleTime()
{
	return theora_granule_time(&m_State, m_State.granulepos);
}

// -----------------------------------------------------------------------------

int BS_TheoraState::DecodePacketIn(ogg_packet * OggPacketPtr)
{
	return theora_decode_packetin(&m_State, OggPacketPtr);
}

// -----------------------------------------------------------------------------

int BS_TheoraState::DecodeYUVOut(yuv_buffer * YUV)
{
	return theora_decode_YUVout(&m_State, YUV);
}
