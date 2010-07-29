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

#include "sword25/fmv/oggtheora/oggstate.h"

// -----------------------------------------------------------------------------

BS_OggState::BS_OggState()
{
	ogg_sync_init(&m_SyncState);
}

// -----------------------------------------------------------------------------

BS_OggState::~BS_OggState()
{
	ogg_sync_clear(&m_SyncState);
}

// -----------------------------------------------------------------------------

int BS_OggState::SyncPageout(ogg_page * OggPage)
{
	return ogg_sync_pageout(&m_SyncState, OggPage);
}

// -----------------------------------------------------------------------------

char * BS_OggState::SyncBuffer(long Size)
{
	return ogg_sync_buffer(&m_SyncState, Size);
}

// -----------------------------------------------------------------------------

int BS_OggState::SyncWrote(long Bytes)
{
	return ogg_sync_wrote(&m_SyncState, Bytes);
}
