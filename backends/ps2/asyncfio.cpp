/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "asyncfio.h"
#include <tamtypes.h>
#include <kernel.h>
#include <fileio.h>
#include <assert.h>
#include <string.h>

extern void sioprintf(const char *zFormat, ...);

AsyncFio::AsyncFio(void) {
	_runningOp = NULL;
	memset(_ioSlots, 0, MAX_HANDLES * sizeof(int));
	ee_sema_t newSema;
	newSema.init_count = 1;
	newSema.max_count = 1;
	_ioSema = CreateSema(&newSema);
}

AsyncFio::~AsyncFio(void) {
	DeleteSema(_ioSema);
}

int AsyncFio::open(const char *name, int ioMode) {
	WaitSema(_ioSema);
	checkSync();
	int res;
	fioOpen(name, ioMode);
	int fioRes = fioSync(FIO_WAIT, &res);
	if (fioRes != FIO_COMPLETE) {
		sioprintf("ERROR: fioOpen(%s, %X):\n", name, ioMode);
		sioprintf("  fioSync returned %d, open res = %d\n", fioRes, res);
        SleepThread();		
	}
	SignalSema(_ioSema);
	return res;
}

void AsyncFio::close(int handle) {
	WaitSema(_ioSema);
	checkSync();
	fioClose(handle);
	SignalSema(_ioSema);
}

void AsyncFio::checkSync(void) {
	if (_runningOp) {
		assert(fioSync(FIO_WAIT, _runningOp) == FIO_COMPLETE);
		_runningOp = NULL;
	}
}

void AsyncFio::read(int fd, void *dest, unsigned int len) {
	WaitSema(_ioSema);
	checkSync();
	assert(fd < MAX_HANDLES);
	_runningOp = _ioSlots + fd;
    fioRead(fd, dest, len);
	SignalSema(_ioSema);
}

void AsyncFio::write(int fd, const void *src, unsigned int len) {
	WaitSema(_ioSema);
	checkSync();
	assert(fd < MAX_HANDLES);
	_runningOp = _ioSlots + fd;
	fioWrite(fd, (unsigned char*)src, len);
	SignalSema(_ioSema);
}

int AsyncFio::seek(int fd, int offset, int whence) {
	WaitSema(_ioSema);
	checkSync();
	int res = fioLseek(fd, offset, whence);
	SignalSema(_ioSema);
	return res;
}

int AsyncFio::sync(int fd) {
	WaitSema(_ioSema);
	if (_runningOp == _ioSlots + fd)
		checkSync();
	SignalSema(_ioSema);
	return _ioSlots[fd];
}

bool AsyncFio::poll(int fd) {
	bool retVal = false;
	if (PollSema(_ioSema) > 0) {
		if (_runningOp == _ioSlots + fd) {
			if (fioSync(FIO_NOWAIT, _runningOp) == FIO_COMPLETE) {
				_runningOp = NULL;
				retVal = true;
			} else
				retVal = false;
		} else
			retVal = true;
		SignalSema(_ioSema);
	}
	return retVal;
}

bool AsyncFio::fioAvail(void) {
	bool retVal = false;
	if (PollSema(_ioSema) > 0) {
		if (_runningOp) {
			if (fioSync(FIO_NOWAIT, _runningOp) == FIO_COMPLETE) {
				_runningOp = NULL;
				retVal = true;
			} else
				retVal = false;
		} else
			retVal = true;
		SignalSema(_ioSema);
	}
	return retVal;
}

