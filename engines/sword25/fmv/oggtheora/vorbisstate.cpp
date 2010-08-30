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

#include "sword25/fmv/oggtheora/vorbisstate.h"

namespace Sword25 {

// -----------------------------------------------------------------------------

VorbisState::VorbisState() :
		m_DSPStateInitialized(false),
		m_BlockInitialized(false) {
	vorbis_info_init(&m_Info);
	vorbis_comment_init(&m_Comment);
}

// -----------------------------------------------------------------------------

VorbisState::~VorbisState() {
	if (m_BlockInitialized) vorbis_block_clear(&m_Block);
	if (m_DSPStateInitialized) vorbis_dsp_clear(&m_DSPState);
	vorbis_comment_clear(&m_Comment);
	vorbis_info_clear(&m_Info);
}

// -----------------------------------------------------------------------------

int VorbisState::SynthesisHeaderIn(ogg_packet *OggPacketPtr) {
	return vorbis_synthesis_headerin(&m_Info, &m_Comment, OggPacketPtr);
}

// -----------------------------------------------------------------------------

int VorbisState::SynthesisInit() {
	int Result = vorbis_synthesis_init(&m_DSPState, &m_Info);
	m_DSPStateInitialized = (Result == 0);
	return Result;
}

// -----------------------------------------------------------------------------

int VorbisState::BlockInit() {
	int Result = vorbis_block_init(&m_DSPState, &m_Block);
	m_BlockInitialized = (Result == 0);
	return Result;
}

// -----------------------------------------------------------------------------

int VorbisState::SynthesisPCMout(float ***PCM) {
	return vorbis_synthesis_pcmout(&m_DSPState, PCM);
}

// -----------------------------------------------------------------------------

int VorbisState::SynthesisRead(int Samples) {
	return vorbis_synthesis_read(&m_DSPState, Samples);
}

// -----------------------------------------------------------------------------

int VorbisState::Synthesis(ogg_packet *OggPacketPtr) {
	return vorbis_synthesis(&m_Block, OggPacketPtr);
}

// -----------------------------------------------------------------------------

int VorbisState::SynthesisBlockIn() {
	return vorbis_synthesis_blockin(&m_DSPState, &m_Block);
}

} // End of namespace Sword25
