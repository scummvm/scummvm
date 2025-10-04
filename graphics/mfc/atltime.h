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

#ifndef GRAPHICS_MFC_ATLTIME_H
#define GRAPHICS_MFC_ATLTIME_H

#include "graphics/mfc/minwindef.h"
#include "graphics/mfc/afxstr.h"

namespace Graphics {
namespace MFC {

typedef long clock_t;

class CTimeSpan {
public:
	CTimeSpan();
	CTimeSpan(int64 time);
	CTimeSpan(
	    long lDays,
	    int nHours,
	    int nMins,
	    int nSecs);

	int64 GetDays() const;
	int64 GetTotalHours() const;
	long GetHours() const;
	int64 GetTotalMinutes() const;
	long GetMinutes() const;
	int64 GetTotalSeconds() const;
	long GetSeconds() const;

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
	CString Format(const char *pszFormat) const;
	CString Format(unsigned int nID) const;

private:
	int64 m_timeSpan;
};

class CTime {
public:
	static CTime WINAPI GetCurrentTime();
	static bool WINAPI IsValidFILETIME(const FILETIME &ft);

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
	    uint16 wDosDate,
	    uint16 wDosTime,
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
	CString Format(const char *pszFormat) const;
	CString FormatGmt(const char *pszFormat) const;
	CString Format(unsigned int nFormatID) const;
	CString FormatGmt(unsigned int nFormatID) const;

private:
	int64 m_time;
};

class CFileTimeSpan {
public:
	CFileTimeSpan();
	CFileTimeSpan(const CFileTimeSpan &span);
	CFileTimeSpan(int64 nSpan);

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

	int64 GetTimeSpan() const;
	void SetTimeSpan(int64 nSpan);

protected:
	int64 m_nSpan;
};

class CFileTime : public FILETIME {
public:
	CFileTime();
	CFileTime(const FILETIME &ft);
	CFileTime(uint64 nTime);

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

	uint64 GetTime() const;
	void SetTime(uint64 nTime);

	CFileTime UTCToLocal() const;
	CFileTime LocalToUTC() const;

	static const uint64 Millisecond = 10000;
	static const uint64 Second = Millisecond * static_cast<uint64>(1000);
	static const uint64 Minute = Second * static_cast<uint64>(60);
	static const uint64 Hour = Minute * static_cast<uint64>(60);
	static const uint64 Day = Hour * static_cast<uint64>(24);
	static const uint64 Week = Day * static_cast<uint64>(7);
};

} // namespace MFC
} // namespace Graphics

#endif
