/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_MFC_AFX_H
#define BAGEL_MFC_AFX_H

#include "common/stream.h"
#include "bagel/mfc/minwindef.h"

namespace Bagel {
namespace MFC {

class CObject;

struct CRuntimeClass {
	// Attributes
	LPCSTR m_lpszClassName = nullptr;
	int m_nObjectSize = 0;
	UINT m_wSchema = 0;
	CObject *(*m_pfnCreateObject)() = nullptr;
	CRuntimeClass *m_pBaseClass = nullptr;

	// CRuntimeClass objects linked together in simple list
	CRuntimeClass *m_pNextClass = nullptr;       // linked list of registered classes
	//const AFX_CLASSINIT *m_pClassInit;
};

class CDumpContext {
};

class CObject {
public:
	CObject();
	CObject(const CObject &objectSrc);
	virtual ~CObject() = 0;
	virtual CRuntimeClass *GetRuntimeClass() const;

	virtual void AssertValid() const {}
	virtual void Dump(CDumpContext &dc) const {}
};

class CFile : public CObject {
public:
public:
	// Flag values
	enum OpenFlags {
		modeRead = (int)0x00000,
		modeWrite = (int)0x00001,
		modeReadWrite = (int)0x00002,
		shareCompat = (int)0x00000,
		shareExclusive = (int)0x00010,
		shareDenyWrite = (int)0x00020,
		shareDenyRead = (int)0x00030,
		shareDenyNone = (int)0x00040,
		modeNoInherit = (int)0x00080,
#ifdef _UNICODE
		typeUnicode = (int)0x00400, // used in derived classes (e.g. CStdioFile) only
#endif
		modeCreate = (int)0x01000,
		modeNoTruncate = (int)0x02000,
		typeText = (int)0x04000, // used in derived classes (e.g. CStdioFile) only
		typeBinary = (int)0x08000, // used in derived classes (e.g. CStdioFile) only
		osNoBuffer = (int)0x10000,
		osWriteThrough = (int)0x20000,
		osRandomAccess = (int)0x40000,
		osSequentialScan = (int)0x80000,
	};

	enum Attribute {
		normal = 0x00,                // note: not same as FILE_ATTRIBUTE_NORMAL
		readOnly = FILE_ATTRIBUTE_READONLY,
		hidden = FILE_ATTRIBUTE_HIDDEN,
		system = FILE_ATTRIBUTE_SYSTEM,
		volume = 0x08,
		directory = FILE_ATTRIBUTE_DIRECTORY,
		archive = FILE_ATTRIBUTE_ARCHIVE,
		device = FILE_ATTRIBUTE_DEVICE,
		temporary = FILE_ATTRIBUTE_TEMPORARY,
		sparse = FILE_ATTRIBUTE_SPARSE_FILE,
		reparsePt = FILE_ATTRIBUTE_REPARSE_POINT,
		compressed = FILE_ATTRIBUTE_COMPRESSED,
		offline = FILE_ATTRIBUTE_OFFLINE,
		notIndexed = FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
		encrypted = FILE_ATTRIBUTE_ENCRYPTED
	};

	enum SeekPosition {
		begin = 0x0, current = 0x1, end = 0x2
	};

private:
	Common::Stream *_stream = nullptr;

public:
	CFile() {}
};

} // namespace MFC
} // namespace Bagel

#endif
