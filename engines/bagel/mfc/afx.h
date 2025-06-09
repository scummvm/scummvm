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

struct CRuntimeClass;
class CObject;
class CException;
class CFileException;

#define DECLARE_DYNAMIC(class_name) \
	public: \
	static const CRuntimeClass class##class_name; \
	virtual const CRuntimeClass *GetRuntimeClass() const override; \


#define RUNTIME_CLASS(class_name) ((const CRuntimeClass *)(&class_name::class##class_name))

#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
	const CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), nullptr }; \
	const CRuntimeClass* class_name::GetRuntimeClass() const \
	{ return RUNTIME_CLASS(class_name); }

#define IMPLEMENT_DYNAMIC(class_name, base_class_name) \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, nullptr)

#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
	CObject *class_name::CreateObject() \
	{ return new class_name; } \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, \
	                       class_name::CreateObject)

struct CRuntimeClass {
	// Attributes
	const char *m_lpszClassName = nullptr;
	int m_nObjectSize = 0;
	UINT m_wSchema = 0;
	CObject *(*m_pfnCreateObject)() = nullptr;
	const CRuntimeClass *m_pBaseClass = nullptr;

	// CRuntimeClass objects linked together in simple list
	const CRuntimeClass *m_pNextClass = nullptr;       // linked list of registered classes

	CRuntimeClass(const char *m_lpszClassName_, int m_nObjectSize_, UINT m_wSchema_,
		CObject *(*m_pfnCreateObject_)(), const CRuntimeClass *m_pBaseClass_,
		const CRuntimeClass *m_pNextClass_);
	CObject *CreateObject() const;
	bool IsDerivedFrom(const CRuntimeClass *pBaseClass) const;
};

class CDumpContext {
};

class CObject {
public:
	static const CRuntimeClass classCObject;
public:
	CObject();
	CObject(const CObject &objectSrc);
	virtual ~CObject() = 0;
	virtual const CRuntimeClass *GetRuntimeClass() const;

	virtual void AssertValid() const {}
	virtual void Dump(CDumpContext &dc) const {}

	bool IsKindOf(const CRuntimeClass *pClass) const;
};

class CFile : public CObject {
	DECLARE_DYNAMIC(CFile)
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

	Common::SeekableReadStream *readStream() const;
	Common::WriteStream *writeStream() const;

public:
	CFile() {}
	CFile(const char *lpszFileName, UINT nOpenFlags);
	~CFile() {
		Close();
	}

	BOOL Open(const char *lpszFileName,
		UINT nOpenFlags = CFile::modeRead,
		CFileException *pError = nullptr);
	void Close();
	void Abort();
	ULONGLONG SeekToEnd();
	void SeekToBegin();
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual ULONGLONG GetLength() const;
	virtual ULONGLONG GetPosition() const;

	virtual UINT Read(void *lpBuf, UINT nCount);
	virtual void Write(const void *lpBuf, UINT nCount);
	UINT ReadHuge(void *lpBuf, UINT nCount) {
		return Read(lpBuf, nCount);
	}

	operator Common::SeekableReadStream *() {
		return readStream();
	}
	operator Common::SeekableReadStream &() {
		return *readStream();
	}

	/**
	 * Detaches the stream from the CFile
	 * without destroying it.
	 */
	Common::SeekableReadStream *detach();
};

/*============================================================================*/
/* Exceptions - Not supported in ScummVM */
class CException {};
class CFileException : public CException {};

} // namespace MFC
} // namespace Bagel

#endif
