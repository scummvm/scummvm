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

typedef long clock_t;

class CTimeSpan {
public:
	CTimeSpan();
	CTimeSpan(int64 time);
	CTimeSpan(
	    LONG lDays,
	    int nHours,
	    int nMins,
	    int nSecs);

	LONGLONG GetDays() const;
	LONGLONG GetTotalHours() const;
	LONG GetHours() const;
	LONGLONG GetTotalMinutes() const;
	LONG GetMinutes() const;
	LONGLONG GetTotalSeconds() const;
	LONG GetSeconds() const;

	int64 GetTimeSpan() const;

	CTimeSpan operator+(CTimeSpan span) const;
	CTimeSpan operator-(CTimeSpan span) const;
	CTimeSpan &operator+=(CTimeSpan span);
	CTimeSpan &operator-=(CTimeSpan span);
	bool operator==(CTimeSpan span) const;
	bool operator!=(CTimeSpan span) const;
	bool operator<(CTimeSpan span) const;
	bool operator>(CTimeSpan span) const;
	bool operator<=(CTimeSpan span) const;
	bool operator>=(CTimeSpan span) const;

public:
	CString Format(LPCTSTR pszFormat) const;
	CString Format(UINT nID) const;

private:
	int64 m_timeSpan;
};

class CTime {
public:
	static CTime WINAPI GetCurrentTime();
	static BOOL WINAPI IsValidFILETIME(const FILETIME &ft);

	CTime();
	CTime(int64 time);
	CTime(
	    int nYear,
	    int nMonth,
	    int nDay,
	    int nHour,
	    int nMin,
	    int nSec,
	    int nDST = -1);
	CTime(
	    WORD wDosDate,
	    WORD wDosTime,
	    int nDST = -1);
	CTime(
	    const SYSTEMTIME &st,
	    int nDST = -1);
	CTime(
	    const FILETIME &ft,
	    int nDST = -1);

	CTime &operator=(int64 time);

	CTime &operator+=(CTimeSpan span);
	CTime &operator-=(CTimeSpan span);

	CTimeSpan operator-(CTime time) const;
	CTime operator-(CTimeSpan span) const;
	CTime operator+(CTimeSpan span) const;

	bool operator==(CTime time) const;
	bool operator!=(CTime time) const;
	bool operator<(CTime time) const;
	bool operator>(CTime time) const;
	bool operator<=(CTime time) const;
	bool operator>=(CTime time) const;

	int64 GetTime() const;

	int GetYear() const;
	int GetMonth() const;
	int GetDay() const;
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetDayOfWeek() const;

	// formatting using "C" strftime
	CString Format(LPCTSTR pszFormat) const;
	CString FormatGmt(LPCTSTR pszFormat) const;
	CString Format(UINT nFormatID) const;
	CString FormatGmt(UINT nFormatID) const;

private:
	int64 m_time;
};

class CFileTimeSpan {
public:
	CFileTimeSpan();
	CFileTimeSpan(const CFileTimeSpan &span);
	CFileTimeSpan(LONGLONG nSpan);

	CFileTimeSpan &operator=(const CFileTimeSpan &span);

	CFileTimeSpan &operator+=(CFileTimeSpan span);
	CFileTimeSpan &operator-=(CFileTimeSpan span);

	CFileTimeSpan operator+(CFileTimeSpan span) const;
	CFileTimeSpan operator-(CFileTimeSpan span) const;

	bool operator==(CFileTimeSpan span) const;
	bool operator!=(CFileTimeSpan span) const;
	bool operator<(CFileTimeSpan span) const;
	bool operator>(CFileTimeSpan span) const;
	bool operator<=(CFileTimeSpan span) const;
	bool operator>=(CFileTimeSpan span) const;

	LONGLONG GetTimeSpan() const;
	void SetTimeSpan(LONGLONG nSpan);

protected:
	LONGLONG m_nSpan;
};

class CFileTime : public FILETIME {
public:
	CFileTime();
	CFileTime(const FILETIME &ft);
	CFileTime(ULONGLONG nTime);

	static CFileTime WINAPI GetCurrentTime();

	CFileTime &operator=(const FILETIME &ft);

	CFileTime &operator+=(CFileTimeSpan span);
	CFileTime &operator-=(CFileTimeSpan span);

	CFileTime operator+(CFileTimeSpan span) const;
	CFileTime operator-(CFileTimeSpan span) const;
	CFileTimeSpan operator-(CFileTime ft) const;

	bool operator==(CFileTime ft) const;
	bool operator!=(CFileTime ft) const;
	bool operator<(CFileTime ft) const;
	bool operator>(CFileTime ft) const;
	bool operator<=(CFileTime ft) const;
	bool operator>=(CFileTime ft) const;

	ULONGLONG GetTime() const;
	void SetTime(ULONGLONG nTime);

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
