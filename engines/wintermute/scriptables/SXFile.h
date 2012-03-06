/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __WmeSXFile_H__
#define __WmeSXFile_H__


#include "engines/wintermute/BScriptable.h"

namespace WinterMute {

class CBFile;

class CSXFile : public CBScriptable {
public:
	DECLARE_PERSISTENT(CSXFile, CBScriptable)
	CScValue *ScGetProperty(char *Name);
	HRESULT ScSetProperty(char *Name, CScValue *Value);
	HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	char *ScToString();
	CSXFile(CBGame *inGame, CScStack *Stack);
	virtual ~CSXFile();
private:
	CBFile *m_ReadFile;
	FILE *m_WriteFile;
	int m_Mode; // 0..none, 1..read, 2..write, 3..append
	bool m_TextMode;
	void Close();
	void Cleanup();
	uint32 GetPos();
	uint32 GetLength();
	bool SetPos(uint32 Pos, TSeek Origin = SEEK_TO_BEGIN);
	char *m_Filename;
};

} // end of namespace WinterMute

#endif
