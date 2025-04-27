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

#ifndef BAGEL_MFC_ATLTIME_H
#define BAGEL_MFC_ATLTIME_H

#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/afxstr.h"

namespace Bagel {
namespace MFC {

class CTimeSpan {
public:
	CTimeSpan();
	CTimeSpan(_In_ __time64_t time);
	CTimeSpan(
		_In_ LONG lDays,
		_In_ int nHours,
		_In_ int nMins,
		_In_ int nSecs);

	LONGLONG GetDays() const;
	LONGLONG GetTotalHours() const;
	LONG GetHours() const;
	LONGLONG GetTotalMinutes() const;
	LONG GetMinutes() const;
	LONGLONG GetTotalSeconds() const;
	LONG GetSeconds() const;

	__time64_t GetTimeSpan() const;

	CTimeSpan operator+(_In_ CTimeSpan span) const;
	CTimeSpan operator-(_In_ CTimeSpan span) const;
	CTimeSpan &operator+=(_In_ CTimeSpan span);
	CTimeSpan &operator-=(_In_ CTimeSpan span);
	bool operator==(_In_ CTimeSpan span) const;
	bool operator!=(_In_ CTimeSpan span) const;
	bool operator<(_In_ CTimeSpan span) const;
	bool operator>(_In_ CTimeSpan span) const;
	bool operator<=(_In_ CTimeSpan span) const;
	bool operator>=(_In_ CTimeSpan span) const;

public:
	CString Format(_In_z_ LPCTSTR pszFormat) const;
	CString Format(_In_ UINT nID) const;
#if defined(_AFX) && defined(_UNICODE)
	// for compatibility with MFC 3.x
	CString Format(_In_z_ LPCSTR pFormat) const;
#endif

#ifdef _AFX
	CArchive &Serialize64(_In_ CArchive &ar);
#endif

private:
	int64 m_timeSpan;
};

class CTime {
public:
	static CTime WINAPI GetCurrentTime();
	static BOOL WINAPI IsValidFILETIME(_In_ const FILETIME &ft);

	CTime();
	CTime(_In_ __time64_t time);
	CTime(
		_In_ int nYear,
		_In_ int nMonth,
		_In_ int nDay,
		_In_ int nHour,
		_In_ int nMin,
		_In_ int nSec,
		_In_ int nDST = -1);
	CTime(
		_In_ WORD wDosDate,
		_In_ WORD wDosTime,
		_In_ int nDST = -1);
	CTime(
		_In_ const SYSTEMTIME &st,
		_In_ int nDST = -1);
	CTime(
		_In_ const FILETIME &ft,
		_In_ int nDST = -1);

	CTime &operator=(_In_ __time64_t time);

	CTime &operator+=(_In_ CTimeSpan span);
	CTime &operator-=(_In_ CTimeSpan span);

	CTimeSpan operator-(_In_ CTime time) const;
	CTime operator-(_In_ CTimeSpan span) const;
	CTime operator+(_In_ CTimeSpan span) const;

	bool operator==(_In_ CTime time) const;
	bool operator!=(_In_ CTime time) const;
	bool operator<(_In_ CTime time) const;
	bool operator>(_In_ CTime time) const;
	bool operator<=(_In_ CTime time) const;
	bool operator>=(_In_ CTime time) const;

	_Success_(return != NULL) struct tm *GetGmtTm(_Out_ struct tm *ptm) const;
	_Success_(return != NULL) struct tm *GetLocalTm(_Out_ struct tm *ptm) const;

	_Success_(return != false) bool GetAsSystemTime(_Out_ SYSTEMTIME &st) const;

	__time64_t GetTime() const;

	int GetYear() const;
	int GetMonth() const;
	int GetDay() const;
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetDayOfWeek() const;

	// formatting using "C" strftime
	CString Format(_In_z_ LPCTSTR pszFormat) const;
	CString FormatGmt(_In_z_ LPCTSTR pszFormat) const;
	CString Format(_In_ UINT nFormatID) const;
	CString FormatGmt(_In_ UINT nFormatID) const;

#if defined(_AFX) && defined(_UNICODE)
	// for compatibility with MFC 3.x
	CString Format(_In_z_ LPCSTR pFormat) const;
	CString FormatGmt(_In_z_ LPCSTR pFormat) const;
#endif

#ifdef _AFX
	CArchive &Serialize64(_In_ CArchive &ar);
#endif

private:
	__time64_t m_time;
};

class CFileTimeSpan {
public:
	CFileTimeSpan();
	CFileTimeSpan(_In_ const CFileTimeSpan &span);
	CFileTimeSpan(_In_ LONGLONG nSpan);

	CFileTimeSpan &operator=(_In_ const CFileTimeSpan &span);

	CFileTimeSpan &operator+=(_In_ CFileTimeSpan span);
	CFileTimeSpan &operator-=(_In_ CFileTimeSpan span);

	CFileTimeSpan operator+(_In_ CFileTimeSpan span) const;
	CFileTimeSpan operator-(_In_ CFileTimeSpan span) const;

	bool operator==(_In_ CFileTimeSpan span) const;
	bool operator!=(_In_ CFileTimeSpan span) const;
	bool operator<(_In_ CFileTimeSpan span) const;
	bool operator>(_In_ CFileTimeSpan span) const;
	bool operator<=(_In_ CFileTimeSpan span) const;
	bool operator>=(_In_ CFileTimeSpan span) const;

	LONGLONG GetTimeSpan() const;
	void SetTimeSpan(_In_ LONGLONG nSpan);

protected:
	LONGLONG m_nSpan;
};

class CFileTime : public FILETIME {
public:
	CFileTime();
	CFileTime(_In_ const FILETIME &ft);
	CFileTime(_In_ ULONGLONG nTime);

	static CFileTime WINAPI GetCurrentTime();

	CFileTime &operator=(_In_ const FILETIME &ft);

	CFileTime &operator+=(_In_ CFileTimeSpan span);
	CFileTime &operator-=(_In_ CFileTimeSpan span);

	CFileTime operator+(_In_ CFileTimeSpan span) const;
	CFileTime operator-(_In_ CFileTimeSpan span) const;
	CFileTimeSpan operator-(_In_ CFileTime ft) const;

	bool operator==(_In_ CFileTime ft) const;
	bool operator!=(_In_ CFileTime ft) const;
	bool operator<(_In_ CFileTime ft) const;
	bool operator>(_In_ CFileTime ft) const;
	bool operator<=(_In_ CFileTime ft) const;
	bool operator>=(_In_ CFileTime ft) const;

	ULONGLONG GetTime() const;
	void SetTime(_In_ ULONGLONG nTime);

	CFileTime UTCToLocal() const;
	CFileTime LocalToUTC() const;

	static const ULONGLONG Millisecond = 10000;
	static const ULONGLONG Second = Millisecond * static_cast<ULONGLONG>(1000);
	static const ULONGLONG Minute = Second * static_cast<ULONGLONG>(60);
	static const ULONGLONG Hour = Minute * static_cast<ULONGLONG>(60);
	static const ULONGLONG Day = Hour * static_cast<ULONGLONG>(24);
	static const ULONGLONG Week = Day * static_cast<ULONGLONG>(7);
};

} // namespace MFC
} // namespace Bagel

#endif
