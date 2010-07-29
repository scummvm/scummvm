// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "timer.h"

#define BS_LOG_PREFIX "BS_TIMER"

uint64_t BS_Timer::GetMicroTicks()
{
	HANDLE ThreadID = ::GetCurrentThread();

	DWORD_PTR OldAffinityMask = ::SetThreadAffinityMask(ThreadID, 1);

	uint64_t Frequency;
	::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&Frequency));

	uint64_t Tick;
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&Tick));

	::SetThreadAffinityMask(ThreadID, OldAffinityMask);
	
	return Tick * 1000000 / Frequency;
}

unsigned int BS_Timer::GetMilliTicks()
{
	return (unsigned int)(GetMicroTicks() / 1000);
}

bool BS_Timer::IsTimerAvaliable()
{
	LARGE_INTEGER Dummy;
	return ::QueryPerformanceFrequency(&Dummy) ? true : false;
}
