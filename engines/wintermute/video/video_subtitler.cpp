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


//  #include "dcgf.h"
#include "engines/wintermute/video/video_subtitler.h"
#define S_OK 0
#define BYTE byte
#define LONG long
#define MAX_PATH 127
#define _MAX_DRIVE 127
#define _MAX_DIR 127
#define _MAX_FNAME 127
#define DWORD byte

namespace Wintermute {
//////////////////////////////////////////////////////////////////////////
CVidSubtitler::CVidSubtitler(BaseGame *inGame): BaseClass(inGame) {
	m_LastSample = -1;
	m_CurrentSubtitle = 0;
	m_ShowSubtitle = false;
}

//////////////////////////////////////////////////////////////////////////
CVidSubtitler::~CVidSubtitler(void) {
	for (int i = 0; i < m_Subtitles.size(); i++) delete m_Subtitles[i];
	m_Subtitles.clear();
}


//////////////////////////////////////////////////////////////////////////
bool CVidSubtitler::LoadSubtitles(char *Filename, char *SubtitleFile) {
	if (!Filename) return S_OK;

	for (int i = 0; i < m_Subtitles.size(); i++) delete m_Subtitles[i];
	m_Subtitles.clear();

	m_LastSample = -1;
	m_CurrentSubtitle = 0;
	m_ShowSubtitle = false;


	char NewFile[MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];

	if (SubtitleFile) {
		strcpy(NewFile, SubtitleFile);
	}
#if 0
	else {
		_splitpath(Filename, drive, dir, fname, NULL);
		_makepath(NewFile, drive, dir, fname, ".SUB");
	}
#endif
	DWORD Size;

#if 0
	BYTE *Buffer = _gameRef->m_FileManager->ReadWholeFile(NewFile, &Size, false);

	if (Buffer == NULL) return S_OK; // no subtitles


	LONG Start, End;
	bool InToken;
	char *TokenStart;
	int TokenLength;
	int TokenPos;
	int TextLength;

	int Pos = 0;
	int LineLength = 0;
	while (Pos < Size) {
		Start = End = -1;
		InToken = false;
		TokenPos = -1;
		TextLength = 0;

		LineLength = 0;
		while (Pos + LineLength < Size && Buffer[Pos + LineLength] != '\n' && Buffer[Pos + LineLength] != '\0') LineLength++;

		int RealLength = LineLength - (Pos + LineLength >= Size ? 0 : 1);
		char *Text = new char[RealLength + 1];
		char *line = (char *)&Buffer[Pos];

		for (int i = 0; i < RealLength; i++) {
			if (line[i] == '{') {
				if (!InToken) {
					InToken = true;
					TokenStart = line + i + 1;
					TokenLength = 0;
					TokenPos++;
				} else TokenLength++;
			} else if (line[i] == '}') {
				if (InToken) {
					InToken = false;
					char *Token = new char[TokenLength + 1];
					strncpy(Token, TokenStart, TokenLength);
					Token[TokenLength] = '\0';
					if (TokenPos == 0) Start = atoi(Token);
					else if (TokenPos == 1) End = atoi(Token);

					delete [] Token;
				} else {
					Text[TextLength] = line[i];
					TextLength++;
				}
			} else {
				if (InToken) {
					TokenLength++;
				} else {
					Text[TextLength] = line[i];
					if (Text[TextLength] == '|') Text[TextLength] = '\n';
					TextLength++;
				}
			}
		}
		Text[TextLength] = '\0';

		if (Start != -1 && TextLength > 0 && (Start != 1 || End != 1)) m_Subtitles.Add(new CVidSubtitle(Game, Text, Start, End));

		delete [] Text;

		Pos += LineLength + 1;
	}

	delete [] Buffer;
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CVidSubtitler::Display() {
#if 0
	if (m_ShowSubtitle) {
		CBFont *font = Game->m_VideoFont ? Game->m_VideoFont : Game->m_SystemFont;
		int Height = font->GetTextHeight((BYTE *)m_Subtitles[m_CurrentSubtitle]->m_Text, Game->m_Renderer->m_Width);
		font->DrawText((BYTE *)m_Subtitles[m_CurrentSubtitle]->m_Text, 0, Game->m_Renderer->m_Height - Height - 5, Game->m_Renderer->m_Width, TAL_CENTER);
	}
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CVidSubtitler::Update(LONG Frame) {
	if (Frame != m_LastSample) {
		m_LastSample = Frame;

		// process subtitles
		m_ShowSubtitle = false;
		while (m_CurrentSubtitle < m_Subtitles.size()) {
			int End = m_Subtitles[m_CurrentSubtitle]->m_EndFrame;

			bool NextFrameOK = (m_CurrentSubtitle < m_Subtitles.size() - 1 && m_Subtitles[m_CurrentSubtitle + 1]->m_StartFrame <= Frame);

			if (Frame > End) {
				if (NextFrameOK) {
					m_CurrentSubtitle++;
				} else {
					m_ShowSubtitle = (End == 0);
					break;
				}
			} else {
				m_ShowSubtitle = true;
				break;
			}
		}
	}
	return S_OK;
}
}
