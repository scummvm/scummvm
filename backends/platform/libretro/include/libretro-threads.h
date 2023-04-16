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

#ifndef LIBRETRO_THREADS_H
#define LIBRETRO_THREADS_H

/* ScummVM doesn't have a top-level main loop that we can use, so instead we run it in its own thread
 * and switch between it and the main thread. Calling these function will block the current thread
 * and unblock the other. Each function should be called from the other thread.
 */
void retro_switch_to_emu_thread(void);
void retro_switch_to_main_thread(void);

/* Initialize the emulation thread and any related resources.
 *
 * This function should be called from the main thread.
 */
bool retro_init_emu_thread(void);

/* Destroy the emulation thread and any related resources. Only call this after the emulation thread
 * has finished (or canceled) and joined.
 *
 * This function should be called from the main thread.
 */
void retro_deinit_emu_thread(void);

/* Returns true if the emulation thread was initialized successfully.
 *
 * This function should be called from the main thread.
 */
bool retro_emu_thread_initialized(void);

/* Returns true if the emulation thread has exited naturally.
 *
 * This function can be called from either the main or the emulation thread.
 */
bool retro_emu_thread_exited(void);

/* Returns scummvm_main return code or -1 if not available */
int retro_get_scummvm_res(void);

#endif

