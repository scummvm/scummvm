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
 */

/*
 * This file is based on Wintermute Engine
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/video/video_subtitle.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
CVidSubtitle::CVidSubtitle(BaseGame *inGame): BaseClass(inGame) {
	m_Text = NULL;
	m_StartFrame = m_EndFrame = 0;
}


//////////////////////////////////////////////////////////////////////////
CVidSubtitle::CVidSubtitle(BaseGame *inGame, char *Text, long StartFrame, long EndFrame): BaseClass(inGame) {
	m_Text = new char[strlen(Text) + 1];
	strcpy(m_Text, Text);
	// _gameRef->m_StringTable->Expand(&m_Text);
	m_StartFrame = StartFrame;
	m_EndFrame = EndFrame;
}


//////////////////////////////////////////////////////////////////////////
CVidSubtitle::~CVidSubtitle() {
	if (m_Text) {
		delete [] m_Text;
		m_Text = NULL;
	}
}
}
