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
#include "bagel/mfc/winnt.h"
#include "bagel/mfc/atltime.h"

namespace Bagel {
namespace MFC {

#define DECLARE_DYNAMIC(class_name) \
public: \
	static const CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#define _DECLARE_DYNAMIC(class_name) \
public: \
	static CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

class CArchive;
class CDocument;
class CDumpContext;
struct CFileStatus;
class CMapPtrToPtr;
class CObject;
class CPtrArray;
struct CRuntimeClass;

typedef void CAtlTransactionManager;
typedef void CException;
typedef void CFileException;

// generate static object constructor for class registration
void AFXAPI AfxClassInit(CRuntimeClass *pNewClass);
struct AFX_CLASSINIT {
	AFX_CLASSINIT(CRuntimeClass *pNewClass) {
		AfxClassInit(pNewClass);
	}
};

struct CRuntimeClass {
	// Attributes
	LPCSTR m_lpszClassName;
	int m_nObjectSize;
	UINT m_wSchema; // schema number of the loaded class
	CObject *(PASCAL *m_pfnCreateObject)(); // NULL => abstract class
#ifdef _AFXDLL
	CRuntimeClass *(PASCAL *m_pfnGetBaseClass)();
#else
	CRuntimeClass *m_pBaseClass;
#endif

	// Operations
	CObject *CreateObject();
	BOOL IsDerivedFrom(const CRuntimeClass *pBaseClass) const;

	// dynamic name lookup and creation
	static CRuntimeClass *PASCAL FromName(LPCSTR lpszClassName);
	static CRuntimeClass *PASCAL FromName(LPCWSTR lpszClassName);
	static CObject *PASCAL CreateObject(LPCSTR lpszClassName);
	static CObject *PASCAL CreateObject(LPCWSTR lpszClassName);

	// Implementation
	void Store(CArchive &ar) const;
	static CRuntimeClass *PASCAL Load(CArchive &ar, UINT *pwSchemaNum);

	// CRuntimeClass objects linked together in simple list
	CRuntimeClass *m_pNextClass;       // linked list of registered classes
	const AFX_CLASSINIT *m_pClassInit;
};

class CObject {
public:
	// Object model (types, destruction, allocation)
	virtual CRuntimeClass *GetRuntimeClass() const;
	virtual ~CObject() = 0;  // virtual destructors are necessary

	// Diagnostic allocations
	void *PASCAL operator new(size_t nSize);
	void *PASCAL operator new(size_t, void *p);
	void PASCAL operator delete(void *p);
	void PASCAL operator delete(void *p, void *pPlace);

#if defined(_DEBUG)
	// for file name/line number tracking using DEBUG_NEW
	void *PASCAL operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
	void PASCAL operator delete(void *p, LPCSTR lpszFileName, int nLine);
#endif

	// Disable the copy constructor and assignment by default so you will get
	//   compiler errors instead of unexpected behavior if you pass objects
	//   by value or assign objects.
protected:
	CObject();
private:
	CObject(const CObject &objectSrc);              // no implementation
	void operator=(const CObject &objectSrc);       // no implementation

	// Attributes
public:
	BOOL IsSerializable() const;
	BOOL IsKindOf(const CRuntimeClass *pClass) const;

	// Overridables
	virtual void Serialize(CArchive &ar);

#if defined(_DEBUG) || defined(_AFXDLL)
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext &dc) const;
#endif

	// Implementation
public:
	static const CRuntimeClass classCObject;
#ifdef _AFXDLL
	static CRuntimeClass *PASCAL _GetBaseClass();
	static CRuntimeClass *PASCAL GetThisClass();
#endif
};

// needed for implementation
template<class TYPE, class ARG_TYPE>
class CArray;

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

	static AFX_DATA const HANDLE hFileNull;

	// Constructors
	CFile();

	CFile(HANDLE hFile);
	CFile(LPCTSTR lpszFileName, UINT nOpenFlags);

	/// <summary>
	/// CFile constructor</summary>
	/// <param name="lpszFileName">A string that is the path to the desired file. The path can be relative or absolute.</param>
	/// <param name="nOpenFlags">Sharing and access mode. Specifies the action to take when opening the file. You can combine options listed below by using the bitwise-OR (|) operator. One access permission and one share option are required; the modeCreate and modeNoInherit modes are optional.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	CFile(LPCTSTR lpszFileName, UINT nOpenFlags, CAtlTransactionManager *pTM);

	// Attributes
	Common::Stream *m_hFile = nullptr;
	operator HANDLE() const;

	virtual ULONGLONG GetPosition() const;
	BOOL GetStatus(CFileStatus &rStatus) const;
	virtual CString GetFileName() const;
	virtual CString GetFileTitle() const;
	virtual CString GetFilePath() const;
	virtual void SetFilePath(LPCTSTR lpszNewName);

	// Operations
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException *pError = NULL);

	/// <summary>
	/// Open is designed for use with the default CFile constructor</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	/// <param name="lpszFileName">A string that is the path to the desired file. The path can be relative or absolute.</param>
	/// <param name="nOpenFlags">Sharing and access mode. Specifies the action to take when opening the file. You can combine options listed below by using the bitwise-OR (|) operator. One access permission and one share option are required; the modeCreate and modeNoInherit modes are optional.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	/// <param name="pError">A pointer to an existing file-exception object that will receive the status of a failed operation</param>
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CAtlTransactionManager *pTM, CFileException *pError);

	/// <summary>
	/// This static function renames the specified file.</summary>
	/// <param name="lpszOldName">The old path.</param>
	/// <param name="lpszNewName">The new path.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	static void PASCAL Rename(LPCTSTR lpszOldName, LPCTSTR lpszNewName, CAtlTransactionManager *pTM = NULL);

	/// <summary>
	/// This static function deletes the file specified by the path.</summary>
	/// <param name="lpszFileName">A string that is the path to the desired file. The path can be relative or absolute.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	static void PASCAL Remove(LPCTSTR lpszFileName, CAtlTransactionManager *pTM = NULL);

	/// <summary>
	/// This method retrieves status information related to a given CFile object instance or a given file path.</summary>
	/// <returns>
	/// TRUE if succeeds; otherwise FALSE.</returns>
	/// <param name="lpszFileName">A string that is the path to the desired file. The path can be relative or absolute.</param>
	/// <param name="rStatus">A reference to a user-supplied CFileStatus structure that will receive the status information.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	static BOOL PASCAL GetStatus(LPCTSTR lpszFileName, CFileStatus &rStatus, CAtlTransactionManager *pTM = NULL);

	/// <summary>
	/// Sets the status of the file associated with this file location.</summary>
	/// <param name="lpszFileName">A string that is the path to the desired file. The path can be relative or absolute.</param>
	/// <param name="rStatus">The buffer containing the new status information.</param>
	/// <param name="pTM">Pointer to CAtlTransactionManager object</param>
	static void PASCAL SetStatus(LPCTSTR lpszFileName, const CFileStatus &status, CAtlTransactionManager *pTM = NULL);

	ULONGLONG SeekToEnd();
	void SeekToBegin();

	// Overridables
	virtual CFile *Duplicate() const;

	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual void SetLength(ULONGLONG dwNewLen);
	virtual ULONGLONG GetLength() const;

	virtual UINT Read(void *lpBuf, UINT nCount);
	virtual void Write(const void *lpBuf, UINT nCount);

	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount);
	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount);

	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	// Implementation
public:
	virtual ~CFile();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext &dc) const override;
#endif
	enum BufferCommand {
		bufferRead, bufferWrite, bufferCommit, bufferCheck
	};
	enum BufferFlags
	{
		bufferDirect = 0x01,
		bufferBlocking = 0x02
	};
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
		void **ppBufStart = NULL, void **ppBufMax = NULL);

protected:
	void CommonBaseInit(HANDLE hFile, CAtlTransactionManager *pTM);
	void CommonInit(LPCTSTR lpszFileName, UINT nOpenFlags, CAtlTransactionManager *pTM);

	BOOL m_bCloseOnDelete;
	CString m_strFileName;

	/// <summary>
	/// Pointer to CAtlTransactionManager object</summary>
	CAtlTransactionManager *m_pTM;
};

class CArchive {
protected:
	enum SchemaMapReservedRefs {
		objTypeArrayRef = 1
	};
	enum LoadArrayObjType {
		typeUndefined = 0, typeCRuntimeClass = 1, typeCObject = 2
	};
public:
	// Flag values
	enum Mode {
		store = 0, load = 1, bNoFlushOnDelete = 2, bNoByteSwap = 4
	};

	CArchive(CFile *pFile, UINT nMode, int nBufSize = 4096, void *lpBuf = NULL);
	~CArchive();

	// Attributes
	BOOL IsLoading() const;
	BOOL IsStoring() const;
	BOOL IsByteSwapping() const;
	BOOL IsBufferEmpty() const;

	CFile *GetFile() const;
	UINT GetObjectSchema(); // only valid when reading a CObject*
	void SetObjectSchema(UINT nSchema);

	// pointer to document being serialized -- must set to serialize
	//  COleClientItems in a document!
	CDocument *m_pDocument;

	// Operations
	UINT Read(void *lpBuf, UINT nMax);
	void EnsureRead(void *lpBuf, UINT nCount);
	void Write(const void *lpBuf, UINT nMax);
	void Flush();
	void Close();
	void Abort();   // close and shutdown without exceptions

	// reading and writing strings
	void WriteString(LPCTSTR lpsz);
	LPTSTR ReadString(_Out_writes_z_(((int)nMax < 0 ? -nMax : nMax) + 1) LPTSTR lpsz, _In_ UINT nMax);
	BOOL ReadString(CString &rString);

public:
	// Object I/O is pointer based to avoid added construction overhead.
	// Use the Serialize member function directly for embedded objects.
	friend CArchive &AFXAPI operator<<(CArchive &ar, const CObject *pOb);

	friend CArchive &AFXAPI operator>>(CArchive &ar, CObject *&pOb);
	friend CArchive &AFXAPI operator>>(CArchive &ar, const CObject *&pOb);

	// insertion operations
	CArchive &operator<<(BYTE by);
	CArchive &operator<<(WORD w);
	CArchive &operator<<(LONG l);
	CArchive &operator<<(DWORD dw);
	CArchive &operator<<(float f);
	CArchive &operator<<(double d);
	CArchive &operator<<(LONGLONG dwdw);
	CArchive &operator<<(ULONGLONG dwdw);

	CArchive &operator<<(int i);
	CArchive &operator<<(short w);
	CArchive &operator<<(char ch);
#if 0
	template < typename BaseType, bool t_bMFCDLL>
	CArchive &operator<<(const ATL::CSimpleStringT<BaseType, t_bMFCDLL> &str);

	template< typename BaseType, class StringTraits >
	CArchive &operator<<(const ATL::CStringT<BaseType, StringTraits> &str);

	template < typename BaseType, bool t_bMFCDLL>
	CArchive &operator>>(ATL::CSimpleStringT<BaseType, t_bMFCDLL> &str);

	template< typename BaseType, class StringTraits >
	CArchive &operator>>(ATL::CStringT<BaseType, StringTraits> &str);
#endif
	CArchive &operator<<(bool b);

	// extraction operations
	CArchive &operator>>(BYTE &by);
	CArchive &operator>>(WORD &w);
	CArchive &operator>>(DWORD &dw);
	CArchive &operator>>(LONG &l);
	CArchive &operator>>(float &f);
	CArchive &operator>>(double &d);
	CArchive &operator>>(LONGLONG &dwdw);
	CArchive &operator>>(ULONGLONG &dwdw);

	CArchive &operator>>(int &i);
	CArchive &operator>>(short &w);
	CArchive &operator>>(char &ch);
	CArchive &operator>>(bool &b);

	// object read/write
	CObject *ReadObject(const CRuntimeClass *pClass);
	void WriteObject(const CObject *pOb);
	// advanced object mapping (used for forced references)
	void MapObject(const CObject *pOb);

	// advanced versioning support
	void WriteClass(const CRuntimeClass *pClassRef);
	CRuntimeClass *ReadClass(const CRuntimeClass *pClassRefRequested = NULL,
		UINT *pSchema = NULL, DWORD *pObTag = NULL);
	void SerializeClass(const CRuntimeClass *pClassRef);

	// advanced operations (used when storing/loading many objects)
	void SetStoreParams(UINT nHashSize = 2053, UINT nBlockSize = 128);
	void SetLoadParams(UINT nGrowBy = 1024);

	void EnsureSchemaMapExists(CArray<LoadArrayObjType, const LoadArrayObjType &> **ppObjTypeArray = NULL);
	// Implementation
public:
	BOOL m_bForceFlat;  // for COleClientItem implementation (default TRUE)
	BOOL m_bDirectBuffer;   // TRUE if m_pFile supports direct buffering
	BOOL m_bBlocking;  // TRUE if m_pFile can block for unbounded periods of time
	void FillBuffer(UINT nAdditionalBytesNeeded);
	void CheckCount();  // throw exception if m_nMapCount is too large

	// special functions for reading and writing (16-bit compatible) counts
	DWORD_PTR ReadCount();
	void WriteCount(DWORD_PTR dwCount);

	// public for advanced use
	UINT m_nObjectSchema;
	CString m_strFileName;

protected:
	// archive objects cannot be copied or assigned
	CArchive(const CArchive &arSrc);
	void operator=(const CArchive &arSrc);

	BOOL m_nMode;
	BOOL m_bUserBuf;
	int m_nBufSize;
	CFile *m_pFile;
	BYTE *m_lpBufCur;
	BYTE *m_lpBufMax;
	BYTE *m_lpBufStart;

	// array/map for CObject* and CRuntimeClass* load/store
	UINT m_nMapCount;
	union
	{
		CPtrArray *m_pLoadArray;
		CMapPtrToPtr *m_pStoreMap;
	};
	// map to keep track of mismatched schemas
	CMapPtrToPtr *m_pSchemaMap;

	// advanced parameters (controls performance with large archives)
	UINT m_nGrowSize;
	UINT m_nHashSize;
};


struct CFileStatus {
	CTime m_ctime;          // creation date/time of file
	CTime m_mtime;          // last modification date/time of file
	CTime m_atime;          // last access date/time of file
	ULONGLONG m_size;       // logical size of file in bytes
	DWORD m_attribute;      // logical OR of CFile::Attribute enum values
	TCHAR m_szFullName[_MAX_PATH]; // absolute path name

#ifdef _DEBUG
	void Dump(CDumpContext &dc) const;
#endif
};

class CDumpContext {
public:
	CDumpContext(CFile *pFile = NULL);

	// Attributes
	int GetDepth() const;      // 0 => this object, 1 => children objects
	void SetDepth(int nNewDepth);

	// Operations
	CDumpContext &operator<<(LPCTSTR lpsz);
	CDumpContext &operator<<(LPCSTR lpsz);  // automatically widened

	CDumpContext &operator<<(const void *lp);
	CDumpContext &operator<<(const CObject *pOb);
	CDumpContext &operator<<(const CObject &ob);
	CDumpContext &operator<<(BYTE by);
	CDumpContext &operator<<(WORD w);
	CDumpContext &DumpAsHex(BYTE b);
	CDumpContext &DumpAsHex(WORD w);
#ifdef _WIN64
	CDumpContext &operator<<(LONG l);
	CDumpContext &operator<<(DWORD dw);
	CDumpContext &operator<<(int n);
	CDumpContext &operator<<(UINT u);
	CDumpContext &DumpAsHex(LONG l);
	CDumpContext &DumpAsHex(DWORD dw);
	CDumpContext &DumpAsHex(int n);
	CDumpContext &DumpAsHex(UINT u);
#else
	CDumpContext &operator<<(LONG_PTR l);
	CDumpContext &operator<<(DWORD_PTR dw);
	CDumpContext &operator<<(INT_PTR n);
	CDumpContext &operator<<(UINT_PTR u);
	CDumpContext &DumpAsHex(LONG_PTR l);
	CDumpContext &DumpAsHex(DWORD_PTR dw);
	CDumpContext &DumpAsHex(INT_PTR n);
	CDumpContext &DumpAsHex(UINT_PTR u);
#endif
	CDumpContext &operator<<(float f);
	CDumpContext &operator<<(double d);
	CDumpContext &operator<<(LONGLONG n);
	CDumpContext &operator<<(ULONGLONG n);
	CDumpContext &DumpAsHex(LONGLONG n);
	CDumpContext &DumpAsHex(ULONGLONG n);
	CDumpContext &operator<<(HWND h);
	CDumpContext &operator<<(HDC h);
	CDumpContext &operator<<(HMENU h);
	CDumpContext &operator<<(HACCEL h);
	CDumpContext &operator<<(HFONT h);
	void HexDump(LPCTSTR lpszLine, BYTE *pby, int nBytes, int nWidth);
	void Flush();

	// Implementation
protected:
	// dump context objects cannot be copied or assigned
	CDumpContext(const CDumpContext &dcSrc);
	void operator=(const CDumpContext &dcSrc);
	void OutputString(LPCTSTR lpsz);

	int m_nDepth;

public:
	CFile *m_pFile;
};

} // namespace MFC
} // namespace Bagel

#endif
