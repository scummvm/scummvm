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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_MAINMAP_H
#define SAGA2_MAINMAP_H 1

namespace Saga2 {

void main_saga2();

/* ===================================================================== *
   Multitasking control
 * ===================================================================== */

//  Forbid() disables all multitasking within this application,
//  and Permit() re-enables it. Use these when you need to access
//  critical data and don't want other tasks mucking with it.
//  (DOS tasking is cooperative, so these funcs are NULL).
//
//  REM: Use these with extreme caution! You can cause the
//  program to lock up if used improperly. Make sure every
//  Forbid() has a matching Permit(), even under error conditions.

#ifdef _WIN32
void Forbid(void);
void Permit(void);
#else
inline void Forbid(void) {}
inline void Permit(void) {}
#endif

#ifdef _WIN32
#define returnAfterThrow(n) return n
#else
#define returnAfterThrow(n)
#endif

#ifdef _WIN32
#define controlPause(n) ((void)0)
#else
#define controlPause(n) delay(n)
#endif

// command line parsing
//extern char *commandLineHelp;
void parseCommandLine(int argc, char *argv[]);

// memory access
uint32 pickHeapSize(uint32 minHeap);

// initialization & cleanup
void initCleanup(void);
bool initErrorHandlers(void);
void cleanupErrorHandlers(void);

bool initializeGame(void);
void shutdownGame(void);

bool initSystemTimer(void);
void cleanupSystemTimer(void);

void cleanupSystemTasks(void);
void cleanupPaletteData(void);

// message handling
void mousePoll(void);
bool handlingMessages(void);
void displayEventLoop(void);

// major parts of main that are actually in main.cpp
void cleanupGame(void);                  // auto-cleanup function
bool setupGame(void);
void cleanupPalettes(void);

void initBreakHandler(void);
void cleanupBreakHandler(void);

} // end of namespace Saga2

#endif
