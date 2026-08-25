/* Copyright (C) 2022 Giovanni Cascione <ing.cascione@gmail.com>
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
*/

#include <stdio.h>
#include <libretro.h>
#include "base/main.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-gl-context-handoff.h"
#include "backends/platform/libretro/include/libretro-threads.h"

extern char cmd_params[20][200];
extern char cmd_params_num;

static int scummvm_res = -1;

static int retro_run_emulator(void) {
	static const char *argv[20] = {0};
	for (int i = 0; i < cmd_params_num; i++)
		argv[i] = cmd_params[i];

	return scummvm_main(cmd_params_num, argv);
}

int retro_get_scummvm_res() {
	return scummvm_res;
}

#ifdef USE_LIBCO

#include <libco.h>

#ifndef EMU_THREAD_STACK_SIZE
#define EMU_THREAD_STACK_SIZE (1024 * 1024)
#endif

/* Since there is no guard page, the only warning available is a watermark. */
static const char *stack_anchor = NULL;
static bool stack_warned = false;

static void check_stack_headroom(void) {
	char probe;
	ptrdiff_t diff;
	size_t used;

	if (!stack_anchor || stack_warned)
		return;

	diff = stack_anchor - &probe;
	used = (size_t)(diff < 0 ? -diff : diff);

	if (used < (EMU_THREAD_STACK_SIZE / 4) * 3)
		return;

	stack_warned = true;
	if (retro_log_cb)
		retro_log_cb(RETRO_LOG_WARN, "[scummvm] Emulation stack at %u KB of %u KB; libco stacks have no guard page.\n",
		             (unsigned)(used / 1024), (unsigned)(EMU_THREAD_STACK_SIZE / 1024));
}

#define EMU_WAITING    (1 << 0)
#define MAIN_WAITING   (1 << 1)
#define EMU_STARTED    (1 << 2)
#define EMU_EXITED     (1 << 3)
static uint8 status = EMU_WAITING | MAIN_WAITING;

static cothread_t main_thread;
static cothread_t emu_thread;

static void retro_exit_to_main_thread(void) {
	check_stack_headroom();
	co_switch(main_thread);
}

static void retro_wrap_emulator(void) {
	char anchor;
	stack_anchor = &anchor;

	status &= ~EMU_EXITED;
	status |= EMU_STARTED;
	scummvm_res = retro_run_emulator();
	status |= EMU_EXITED;
	status &= ~EMU_STARTED;
	retro_exit_to_main_thread();
}

static void retro_free_emu_thread(void) {
	if (emu_thread)
		co_delete(emu_thread);
	emu_thread = NULL;
}

void retro_switch_to_emu_thread(void) {
	if (retro_emu_thread_exited() || !retro_emu_thread_initialized())
		return;
	co_switch(emu_thread);
}

void retro_switch_to_main_thread(void) {
	retro_exit_to_main_thread();
}

bool retro_emu_thread_initialized(void) {
	return (bool)emu_thread;
}

bool retro_emu_thread_exited(void) {
	return (bool)(status & EMU_EXITED);
}

bool retro_emu_thread_started(void) {
	return (bool)(status & EMU_STARTED);
}

bool retro_init_emu_thread(void) {
	if (retro_emu_thread_initialized())
		return true;

	main_thread = co_active();
	emu_thread = co_create(EMU_THREAD_STACK_SIZE, retro_wrap_emulator);
	if (!emu_thread) {
		retro_free_emu_thread();
		return false;
	}

	status &= ~(EMU_EXITED | EMU_STARTED);
	return true;
}

void retro_deinit_emu_thread(void) {
	if (retro_emu_thread_initialized())
		retro_free_emu_thread();
}

#else /* !USE_LIBCO */

#include <rthreads/rthreads.h>

#define TURN_MAIN 0
#define TURN_EMU  1

static sthread_t *emu_thread = NULL;
static slock_t *state_lock = NULL;
static scond_t *main_cond = NULL;
static scond_t *emu_cond = NULL;

/* Everything below is guarded by state_lock. The two threads never run
 * concurrently - the handshake hands control back and forth - but they are
 * distinct OS threads, so the flags still need a lock rather than the single
 * unsynchronised byte this used to share with the libco path. */
static uint8 turn = TURN_MAIN;
static bool emu_started = false;
static bool emu_exited = false;

static void retro_wrap_emulator(void *args) {
	slock_lock(state_lock);
	while (turn != TURN_EMU)
		scond_wait(emu_cond, state_lock);
	slock_unlock(state_lock);
	retro_gl_context_acquire();

	scummvm_res = retro_run_emulator();

	retro_gl_context_release();

	/* Hand control back and return, rather than parking on emu_cond: the
	 * thread has to actually exit so that retro_free_emu_thread() can join
	 * it before the lock and condition variables are destroyed. */
	slock_lock(state_lock);
	emu_exited = true;
	emu_started = false;
	turn = TURN_MAIN;
	scond_signal(main_cond);
	slock_unlock(state_lock);
}

static void retro_free_emu_thread(void) {
	if (emu_thread) {
		sthread_join(emu_thread);
		emu_thread = NULL;
	}
	if (main_cond) {
		scond_free(main_cond);
		main_cond = NULL;
	}
	if (emu_cond) {
		scond_free(emu_cond);
		emu_cond = NULL;
	}
	if (state_lock) {
		slock_free(state_lock);
		state_lock = NULL;
	}
	emu_started = false;
}

void retro_switch_to_emu_thread(void) {
	if (retro_emu_thread_exited() || !retro_emu_thread_initialized())
		return;

	retro_gl_context_release();
	slock_lock(state_lock);
	turn = TURN_EMU;
	scond_signal(emu_cond);
	while (turn != TURN_MAIN)
		scond_wait(main_cond, state_lock);
	slock_unlock(state_lock);
	retro_gl_context_acquire();
}

void retro_switch_to_main_thread(void) {
	retro_gl_context_release();
	slock_lock(state_lock);
	turn = TURN_MAIN;
	scond_signal(main_cond);
	while (turn != TURN_EMU)
		scond_wait(emu_cond, state_lock);
	slock_unlock(state_lock);
	retro_gl_context_acquire();
}

bool retro_emu_thread_initialized(void) {
	return emu_thread != NULL;
}

bool retro_emu_thread_exited(void) {
	bool ret;
	if (!state_lock)
		return false;
	slock_lock(state_lock);
	ret = emu_exited;
	slock_unlock(state_lock);
	return ret;
}

bool retro_emu_thread_started(void) {
	bool ret;
	if (!state_lock)
		return false;
	slock_lock(state_lock);
	ret = emu_started;
	slock_unlock(state_lock);
	return ret;
}

bool retro_init_emu_thread(void) {
	if (retro_emu_thread_initialized())
		return true;

	state_lock = slock_new();
	main_cond = scond_new();
	emu_cond = scond_new();

	if (!state_lock || !main_cond || !emu_cond) {
		retro_free_emu_thread();
		return false;
	}

	turn = TURN_MAIN;
	emu_exited = false;
	emu_started = true;

	emu_thread = sthread_create(retro_wrap_emulator, NULL);
	if (!emu_thread) {
		emu_started = false;
		retro_free_emu_thread();
		return false;
	}

	return true;
}

void retro_deinit_emu_thread(void) {
	if (retro_emu_thread_initialized())
		retro_free_emu_thread();
}

#endif /* USE_LIBCO */
