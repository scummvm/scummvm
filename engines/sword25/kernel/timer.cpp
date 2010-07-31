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
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "sword25/kernel/timer.h"

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
