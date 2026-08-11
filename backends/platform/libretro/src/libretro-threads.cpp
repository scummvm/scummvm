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
#include "backends/platform/libretro/include/libretro-threads.h"

#define EMU_WAITING    (1 << 0)
#define MAIN_WAITING   (1 << 1)
#define EMU_STARTED    (1 << 2)
#define EMU_EXITED     (1 << 3)
static uint8 status = EMU_WAITING | MAIN_WAITING;
static int scummvm_res = -1;

#ifdef USE_LIBCO
#include <libco.h>
static cothread_t main_thread;
static cothread_t emu_thread;
#else
#include <rthreads/rthreads.h>
static uintptr_t main_thread_id;
static sthread_t *emu_thread;
static slock_t *emu_lock;
static slock_t *main_lock;
static scond_t *emu_cond;
static scond_t *main_cond;
#endif

extern char cmd_params[20][200];
extern char cmd_params_num;

static void retro_exit_to_main_thread() {
#ifdef USE_LIBCO
	co_switch(main_thread);
#else
	slock_lock(main_lock);
	status &= ~MAIN_WAITING;
	slock_unlock(main_lock);
	slock_lock(emu_lock);
	scond_signal(main_cond);

	status |= EMU_WAITING;
	while (status & EMU_WAITING) {
		scond_wait(emu_cond, emu_lock);
	}
	slock_unlock(emu_lock);
#endif
}

static int retro_run_emulator(void) {
	static const char *argv[20] = {0};
	for (int i = 0; i < cmd_params_num; i++)
		argv[i] = cmd_params[i];

	return scummvm_main(cmd_params_num, argv);
}

static void retro_wrap_emulator(void) {

	status &= ~EMU_EXITED;
	status |= EMU_STARTED;
	scummvm_res = retro_run_emulator();
	status |= EMU_EXITED;
	status &= ~EMU_STARTED;
	retro_exit_to_main_thread();
}

#ifndef USE_LIBCO
static void retro_wrap_emulator(void *args) {
	retro_wrap_emulator();
}
#endif

static void retro_free_emu_thread() {
#ifdef USE_LIBCO
	if (emu_thread)
		co_delete(emu_thread);
#else
	if (main_lock)
		slock_free(main_lock);
	if (emu_lock)
		slock_free(emu_lock);
	if (main_cond)
		scond_free(main_cond);
	if (emu_cond)
		scond_free(emu_cond);
#endif
	emu_thread = NULL;
}

void retro_switch_to_emu_thread() {
	if (retro_emu_thread_exited() || !retro_emu_thread_initialized())
		return;
#ifdef USE_LIBCO
	co_switch(emu_thread);
#else
	slock_lock(emu_lock);
	status &= ~EMU_WAITING;
	slock_unlock(emu_lock);
	slock_lock(main_lock);
	scond_signal(emu_cond);

	status |= MAIN_WAITING;
	while (status & MAIN_WAITING) {
		scond_wait(main_cond, main_lock);
	}
	slock_unlock(main_lock);
#endif
}

void retro_switch_to_main_thread() {
	retro_exit_to_main_thread();
}

bool retro_emu_thread_initialized() {
	return (bool)emu_thread;
}

bool retro_emu_thread_exited() {
	return (bool)(status & EMU_EXITED);
}

bool retro_init_emu_thread(void) {
	if (retro_emu_thread_initialized())
		return true;
	bool success = true;
#ifdef USE_LIBCO
	main_thread = co_active();
	emu_thread = co_create(65536 * sizeof(void *), retro_wrap_emulator);
	if (!emu_thread)
#else
	main_thread_id = sthread_get_current_thread_id();
	main_lock = slock_new();
	emu_lock = slock_new();
	main_cond = scond_new();
	emu_cond = scond_new();
	emu_thread = sthread_create(retro_wrap_emulator, NULL);

	if (!main_lock || !emu_lock || !main_cond || !emu_cond || !emu_thread)
#endif
		success = false;

	if (!success)
		retro_free_emu_thread();
	else
		status &= ~(EMU_EXITED | EMU_STARTED);

	return success;
}

void retro_deinit_emu_thread() {
	if (retro_emu_thread_initialized())
		retro_free_emu_thread();
}

int retro_get_scummvm_res() {
	return scummvm_res;
}

bool retro_emu_thread_started(void) {
	return (bool)(status & EMU_STARTED);
}
