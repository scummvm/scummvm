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

#ifndef LIBRETRO_GL_CONTEXT_HANDOFF_H
#define LIBRETRO_GL_CONTEXT_HANDOFF_H

/* The frontend makes its rendering context current on the thread that drives
 * retro_run(). ScummVM issues its GL calls from inside scummvm_main(), which
 * runs on the emulation thread, so the context has to travel with control at
 * every handoff.
 *
 * The two threads never run concurrently, so no reference counting is needed:
 * whichever thread is about to block releases the context, and whichever
 * thread is about to run acquires it. Visibility of the saved handles between
 * the two threads is provided by the lock in libretro-threads.cpp.
 *
 * These are no-ops when no context is current (software rendering) and when
 * the core is built with libco, where both halves share one OS thread.
 */

/* Unbind the context from the calling thread, remembering what was bound.
 * Call immediately before handing control to the other thread. */
void retro_gl_context_release(void);

/* Rebind whatever the last retro_gl_context_release() unbound. Call
 * immediately after regaining control. */
void retro_gl_context_acquire(void);

/* Whether a context handoff backend is available on this platform. Requesting
 * hardware rendering without one would leave the emulation thread issuing GL
 * calls against no current context, so the caller falls back to software. */
bool retro_gl_context_handoff_available(void);

#endif
