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

#include "SysClassRegistry.h"
#include "SysClass.h"
#include "ScStack.h"
#include "ScValue.h"
#include "ScScript.h"
#include "utils.h"
#include "BGame.h"
#include "BFile.h"
#include "BFileManager.h"
#include "PlatformSDL.h"
#include "scriptables/SXFile.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXFile, false)

//////////////////////////////////////////////////////////////////////////
CSXFile::CSXFile(CBGame *inGame, CScStack *Stack): CBScriptable(inGame) {
	Stack->CorrectParams(1);
	CScValue *Val = Stack->Pop();

	m_Filename = NULL;
	if (!Val->IsNULL()) CBUtils::SetString(&m_Filename, Val->GetString());

	m_ReadFile = NULL;
	m_WriteFile = NULL;

	m_Mode = 0;
	m_TextMode = false;
}


//////////////////////////////////////////////////////////////////////////
CSXFile::~CSXFile() {
	Cleanup();
}

//////////////////////////////////////////////////////////////////////////
void CSXFile::Cleanup() {
	delete[] m_Filename;
	m_Filename = NULL;
	Close();
}


//////////////////////////////////////////////////////////////////////////
void CSXFile::Close() {
	if (m_ReadFile) {
		Game->m_FileManager->CloseFile(m_ReadFile);
		m_ReadFile = NULL;
	}
	if (m_WriteFile) {
		fclose(m_WriteFile);
		m_WriteFile = NULL;
	}
	m_Mode = 0;
	m_TextMode = false;
}

//////////////////////////////////////////////////////////////////////////
char *CSXFile::ScToString() {
	if (m_Filename) return m_Filename;
	else return "[file object]";
}

#define FILE_BUFFER_SIZE 32768
//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetFilename
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetFilename") == 0) {
		Stack->CorrectParams(1);
		char *Filename = Stack->Pop()->GetString();
		Cleanup();
		CBUtils::SetString(&m_Filename, Filename);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OpenAsText / OpenAsBinary
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "OpenAsText") == 0 || strcmp(Name, "OpenAsBinary") == 0) {
		Stack->CorrectParams(1);
		Close();
		m_Mode = Stack->Pop()->GetInt(1);
		if (m_Mode < 1 || m_Mode > 3) {
			Script->RuntimeError("File.%s: invalid access mode. Setting read mode.", Name);
			m_Mode = 1;
		}
		if (m_Mode == 1) {
			m_ReadFile = Game->m_FileManager->OpenFile(m_Filename);
			if (!m_ReadFile) {
				//Script->RuntimeError("File.%s: Error opening file '%s' for reading.", Name, m_Filename);
				Close();
			} else m_TextMode = strcmp(Name, "OpenAsText") == 0;
		} else {
			if (strcmp(Name, "OpenAsText") == 0) {
				if (m_Mode == 2) m_WriteFile = fopen(m_Filename, "w+");
				else m_WriteFile = fopen(m_Filename, "a+");
			} else {
				if (m_Mode == 2) m_WriteFile = fopen(m_Filename, "wb+");
				else m_WriteFile = fopen(m_Filename, "ab+");
			}

			if (!m_WriteFile) {
				//Script->RuntimeError("File.%s: Error opening file '%s' for writing.", Name, m_Filename);
				Close();
			} else m_TextMode = strcmp(Name, "OpenAsText") == 0;
		}

		if (m_ReadFile || m_WriteFile) Stack->PushBool(true);
		else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Close
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Close") == 0) {
		Stack->CorrectParams(0);
		Close();
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetPosition") == 0) {
		Stack->CorrectParams(1);
		if (m_Mode == 0) {
			Script->RuntimeError("File.%s: File is not open", Name);
			Stack->PushBool(false);
		} else {
			int Pos = Stack->Pop()->GetInt();
			Stack->PushBool(SetPos(Pos));
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Delete
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Delete") == 0) {
		Stack->CorrectParams(0);
		Close();
		Stack->PushBool(CBPlatform::DeleteFile(m_Filename) != FALSE);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Copy
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Copy") == 0) {
		Stack->CorrectParams(2);
		char *Dest = Stack->Pop()->GetString();
		bool Overwrite = Stack->Pop()->GetBool(true);

		Close();
		Stack->PushBool(CBPlatform::CopyFile(m_Filename, Dest, !Overwrite) != FALSE);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadLine
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadLine") == 0) {
		Stack->CorrectParams(0);
		if (!m_TextMode || !m_ReadFile) {
			Script->RuntimeError("File.%s: File must be open in text mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		uint32 BufSize = FILE_BUFFER_SIZE;
		byte *Buf = (byte  *)malloc(BufSize);
		uint32 Counter = 0;
		byte b;
		bool FoundNewLine = false;
		HRESULT Ret = E_FAIL;
		do {
			Ret = m_ReadFile->Read(&b, 1);
			if (FAILED(Ret)) break;

			if (Counter > BufSize) {
				Buf = (byte  *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
				BufSize += FILE_BUFFER_SIZE;
			}
			if (b == '\n') {
				Buf[Counter] = '\0';
				FoundNewLine = true;
				break;
			} else if (b == 0x0D) continue;
			else {
				Buf[Counter] = b;
				Counter++;
			}
		} while (SUCCEEDED(Ret));

		if (Counter > BufSize) {
			Buf = (byte  *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
			BufSize += FILE_BUFFER_SIZE;
		}
		Buf[Counter] = '\0';

		if (!FoundNewLine && Counter == 0) Stack->PushNULL();
		else Stack->PushString((char *)Buf);

		free(Buf);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadText") == 0) {
		Stack->CorrectParams(1);
		int TextLen = Stack->Pop()->GetInt();

		if (!m_TextMode || !m_ReadFile) {
			Script->RuntimeError("File.%s: File must be open in text mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		uint32 BufSize = FILE_BUFFER_SIZE;
		byte *Buf = (byte  *)malloc(BufSize);
		uint32 Counter = 0;
		byte b;

		HRESULT Ret = E_FAIL;
		while (Counter < TextLen) {
			Ret = m_ReadFile->Read(&b, 1);
			if (FAILED(Ret)) break;

			if (Counter > BufSize) {
				Buf = (byte  *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
				BufSize += FILE_BUFFER_SIZE;
			}
			if (b == 0x0D) continue;
			else {
				Buf[Counter] = b;
				Counter++;
			}
		}

		if (Counter > BufSize) {
			Buf = (byte  *)realloc(Buf, BufSize + FILE_BUFFER_SIZE);
			BufSize += FILE_BUFFER_SIZE;
		}
		Buf[Counter] = '\0';

		if (TextLen > 0 && Counter == 0) Stack->PushNULL();
		else Stack->PushString((char *)Buf);

		free(Buf);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteLine / WriteText
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteLine") == 0 || strcmp(Name, "WriteText") == 0) {
		Stack->CorrectParams(1);
		char *Line = Stack->Pop()->GetString();
		if (!m_TextMode || !m_WriteFile) {
			Script->RuntimeError("File.%s: File must be open for writing in text mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		if (strcmp(Name, "WriteLine") == 0)
			fprintf(m_WriteFile, "%s\n", Line);
		else
			fprintf(m_WriteFile, "%s", Line);

		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// ReadBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadBool") == 0) {
		Stack->CorrectParams(0);
		if (m_TextMode || !m_ReadFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		bool Val;
		if (SUCCEEDED(m_ReadFile->Read(&Val, sizeof(bool)))) Stack->PushBool(Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadByte") == 0) {
		Stack->CorrectParams(0);
		if (m_TextMode || !m_ReadFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		byte Val;
		if (SUCCEEDED(m_ReadFile->Read(&Val, sizeof(byte )))) Stack->PushInt(Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadShort") == 0) {
		Stack->CorrectParams(0);
		if (m_TextMode || !m_ReadFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		short Val;
		if (SUCCEEDED(m_ReadFile->Read(&Val, sizeof(short)))) Stack->PushInt(65536 + Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadInt / ReadLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadInt") == 0 || strcmp(Name, "ReadLong") == 0) {
		Stack->CorrectParams(0);
		if (m_TextMode || !m_ReadFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		int Val;
		if (SUCCEEDED(m_ReadFile->Read(&Val, sizeof(int)))) Stack->PushInt(Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadFloat") == 0) {
		Stack->CorrectParams(0);
		if (m_TextMode || !m_ReadFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		float Val;
		if (SUCCEEDED(m_ReadFile->Read(&Val, sizeof(float)))) Stack->PushFloat(Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadDouble") == 0) {
		Stack->CorrectParams(0);
		if (m_TextMode || !m_ReadFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		double Val;
		if (SUCCEEDED(m_ReadFile->Read(&Val, sizeof(double)))) Stack->PushFloat(Val);
		else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ReadString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ReadString") == 0) {
		Stack->CorrectParams(0);
		if (m_TextMode || !m_ReadFile) {
			Script->RuntimeError("File.%s: File must be open for reading in binary mode.", Name);
			Stack->PushNULL();
			return S_OK;
		}
		uint32 Size;
		if (SUCCEEDED(m_ReadFile->Read(&Size, sizeof(uint32)))) {
			byte *Str = new byte[Size + 1];
			if (Str) {
				if (SUCCEEDED(m_ReadFile->Read(Str, Size))) {
					Str[Size] = '\0';
					Stack->PushString((char *)Str);
				}
				delete [] Str;
			} else Stack->PushNULL();
		} else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteBool
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteBool") == 0) {
		Stack->CorrectParams(1);
		bool Val = Stack->Pop()->GetBool();

		if (m_TextMode || !m_WriteFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, m_WriteFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteByte
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteByte") == 0) {
		Stack->CorrectParams(1);
		byte Val = Stack->Pop()->GetInt();

		if (m_TextMode || !m_WriteFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, m_WriteFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteShort
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteShort") == 0) {
		Stack->CorrectParams(1);
		short Val = Stack->Pop()->GetInt();

		if (m_TextMode || !m_WriteFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, m_WriteFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteInt / WriteLong
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteInt") == 0 || strcmp(Name, "WriteLong") == 0) {
		Stack->CorrectParams(1);
		int Val = Stack->Pop()->GetInt();

		if (m_TextMode || !m_WriteFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, m_WriteFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteFloat
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteFloat") == 0) {
		Stack->CorrectParams(1);
		float Val = Stack->Pop()->GetFloat();

		if (m_TextMode || !m_WriteFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, m_WriteFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteDouble
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteDouble") == 0) {
		Stack->CorrectParams(1);
		double Val = Stack->Pop()->GetFloat();

		if (m_TextMode || !m_WriteFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}
		fwrite(&Val, sizeof(Val), 1, m_WriteFile);
		Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WriteString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WriteString") == 0) {
		Stack->CorrectParams(1);
		char *Val = Stack->Pop()->GetString();

		if (m_TextMode || !m_WriteFile) {
			Script->RuntimeError("File.%s: File must be open for writing in binary mode.", Name);
			Stack->PushBool(false);
			return S_OK;
		}

		uint32 Size = strlen(Val);
		fwrite(&Size, sizeof(Size), 1, m_WriteFile);
		fwrite(Val, Size, 1, m_WriteFile);

		Stack->PushBool(true);

		return S_OK;
	}


	else return CBScriptable::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXFile::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("file");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Filename (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Filename") == 0) {
		m_ScValue->SetString(m_Filename);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Position (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Position") == 0) {
		m_ScValue->SetInt(GetPos());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Length (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Length") == 0) {
		m_ScValue->SetInt(GetLength());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TextMode") == 0) {
		m_ScValue->SetBool(m_TextMode);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccessMode (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccessMode") == 0) {
		m_ScValue->SetInt(m_Mode);
		return m_ScValue;
	}

	else return CBScriptable::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::ScSetProperty(char *Name, CScValue *Value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	if(strcmp(Name, "Length")==0){
	    int OrigLength = m_Length;
	    m_Length = max(Value->GetInt(0), 0);

	    char PropName[20];
	    if(m_Length < OrigLength){
	        for(int i=m_Length; i<OrigLength; i++){
	            sprintf(PropName, "%d", i);
	            m_Values->DeleteProp(PropName);
	        }
	    }
	    return S_OK;
	}
	else*/ return CBScriptable::ScSetProperty(Name, Value);
}

//////////////////////////////////////////////////////////////////////////
uint32 CSXFile::GetPos() {
	if (m_Mode == 1 && m_ReadFile) return m_ReadFile->GetPos();
	else if ((m_Mode == 2 || m_Mode == 3) && m_WriteFile) return ftell(m_WriteFile);
	else return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CSXFile::SetPos(uint32 Pos, TSeek Origin) {
	if (m_Mode == 1 && m_ReadFile) return SUCCEEDED(m_ReadFile->Seek(Pos, Origin));
	else if ((m_Mode == 2 || m_Mode == 3) && m_WriteFile) return fseek(m_WriteFile, Pos, (int)Origin) == 0;
	else return false;
}

//////////////////////////////////////////////////////////////////////////
uint32 CSXFile::GetLength() {
	if (m_Mode == 1 && m_ReadFile) return m_ReadFile->GetSize();
	else if ((m_Mode == 2 || m_Mode == 3) && m_WriteFile) {
		uint32 CurrentPos = ftell(m_WriteFile);
		fseek(m_WriteFile, 0, SEEK_END);
		int Ret = ftell(m_WriteFile);
		fseek(m_WriteFile, CurrentPos, SEEK_SET);
		return Ret;
	} else return 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CSXFile::Persist(CBPersistMgr *PersistMgr) {

	CBScriptable::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Filename));
	PersistMgr->Transfer(TMEMBER(m_Mode));
	PersistMgr->Transfer(TMEMBER(m_TextMode));

	uint32 Pos = 0;
	if (PersistMgr->m_Saving) {
		Pos = GetPos();
		PersistMgr->Transfer(TMEMBER(Pos));
	} else {
		PersistMgr->Transfer(TMEMBER(Pos));

		// try to re-open file if needed
		m_WriteFile = NULL;
		m_ReadFile = NULL;

		if (m_Mode != 0) {
			// open for reading
			if (m_Mode == 1) {
				m_ReadFile = Game->m_FileManager->OpenFile(m_Filename);
				if (!m_ReadFile) Close();
			}
			// open for writing / appending
			else {
				if (m_TextMode) {
					if (m_Mode == 2) m_WriteFile = fopen(m_Filename, "w+");
					else m_WriteFile = fopen(m_Filename, "a+");
				} else {
					if (m_Mode == 2) m_WriteFile = fopen(m_Filename, "wb+");
					else m_WriteFile = fopen(m_Filename, "ab+");
				}
				if (m_WriteFile) Close();
			}
			SetPos(Pos);
		}
	}

	return S_OK;
}

} // end of namespace WinterMute
