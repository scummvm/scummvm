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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/fmv/oggtheora/theorastate.h"

namespace Sword25 {

// -----------------------------------------------------------------------------

TheoraState::TheoraState() :
		m_StateInitialized(false) {
	theora_comment_init(&m_Comment);
	theora_info_init(&m_Info);
}

// -----------------------------------------------------------------------------

TheoraState::~TheoraState() {
	if (m_StateInitialized) theora_clear(&m_State);
	theora_info_clear(&m_Info);
	theora_comment_clear(&m_Comment);
}

// -----------------------------------------------------------------------------

int TheoraState::DecodeHeader(ogg_packet *OggPacketPtr) {
	return theora_decode_header(&m_Info, &m_Comment, OggPacketPtr);
}

// -----------------------------------------------------------------------------

int TheoraState::DecodeInit() {
	int Result = theora_decode_init(&m_State, &m_Info);
	m_StateInitialized = (Result == 0);
	return Result;
}

// -----------------------------------------------------------------------------

double TheoraState::GranuleTime() {
	return theora_granule_time(&m_State, m_State.granulepos);
}

// -----------------------------------------------------------------------------

int TheoraState::DecodePacketIn(ogg_packet *OggPacketPtr) {
	return theora_decode_packetin(&m_State, OggPacketPtr);
}

// -----------------------------------------------------------------------------

int TheoraState::DecodeYUVOut(yuv_buffer *YUV) {
	return theora_decode_YUVout(&m_State, YUV);
}

} // End of namespace Sword25
