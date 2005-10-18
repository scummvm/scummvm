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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include <fileXio_rpc.h>

#define DEFAULT_MODE (FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IROTH | FIO_S_IWOTH)

extern void sioprintf(const char *zFormat, ...);

AsyncFio::AsyncFio(void) {
	_runningOp = NULL;
	memset((int *)_ioSlots, 0, MAX_HANDLES * sizeof(int));
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
	fileXioOpen(name, ioMode, DEFAULT_MODE);
	fileXioWaitAsync(FXIO_WAIT, &res);
	SignalSema(_ioSema);
	return res;
}

void AsyncFio::close(int handle) {
	WaitSema(_ioSema);
	checkSync();
	fileXioClose(handle);
	int res;
	fileXioWaitAsync(FXIO_WAIT, &res);
	if (res != 0) {
		sioprintf("ERROR: fileXioClose failed, EC %d", res);
		SleepThread();
	}
	_ioSlots[handle] = 0;
	SignalSema(_ioSema);
}

void AsyncFio::checkSync(void) {
	if (_runningOp) {
		fileXioWaitAsync(FXIO_WAIT, (int *)_runningOp);
		_runningOp = NULL;
	}
}

void AsyncFio::read(int fd, void *dest, unsigned int len) {
	WaitSema(_ioSema);
	checkSync();
	assert(fd < MAX_HANDLES);
	_runningOp = _ioSlots + fd;
	fileXioRead(fd, (unsigned char*)dest, len);
	SignalSema(_ioSema);
}

void AsyncFio::write(int fd, const void *src, unsigned int len) {
	WaitSema(_ioSema);
	checkSync();
	assert(fd < MAX_HANDLES);
	_runningOp = _ioSlots + fd;
	fileXioWrite(fd, (unsigned char*)src, len);
	SignalSema(_ioSema);
}

int AsyncFio::seek(int fd, int offset, int whence) {
	int res;
	WaitSema(_ioSema);
	checkSync();
	fileXioLseek(fd, offset, whence);
	fileXioWaitAsync(FXIO_WAIT, &res);
	SignalSema(_ioSema);
	return res;
}

int AsyncFio::mkdir(const char *name) {
	int res;
	WaitSema(_ioSema);
	checkSync();
	fileXioMkdir(name, DEFAULT_MODE);
	fileXioWaitAsync(FXIO_WAIT, &res);
	SignalSema(_ioSema);
	return res;
}

int AsyncFio::dopen(const char *name) {
	int res;
	WaitSema(_ioSema);
	checkSync();
	fileXioDopen(name);
	fileXioWaitAsync(FXIO_WAIT, &res);
	SignalSema(_ioSema);
	return res;
}

int AsyncFio::dread(int fd, iox_dirent_t *dest) {
	int res;
	WaitSema(_ioSema);
	checkSync();
	fileXioDread(fd, dest);
	fileXioWaitAsync(FXIO_WAIT, &res);
	SignalSema(_ioSema);
	return res;
}

void AsyncFio::dclose(int fd) {
	int res;
	WaitSema(_ioSema);
	checkSync();
	fileXioDclose(fd);
	fileXioWaitAsync(FXIO_WAIT, &res);
	assert(res == 0);
	SignalSema(_ioSema);
}

int AsyncFio::sync(int fd) {
	WaitSema(_ioSema);
	if (_runningOp == _ioSlots + fd)
		checkSync();
	int res = _ioSlots[fd];
	_ioSlots[fd] = 0;
	SignalSema(_ioSema);
	return res;
}

bool AsyncFio::poll(int fd) {
	bool retVal = false;
	if (PollSema(_ioSema) >= 0) {
		if (_runningOp == _ioSlots + fd) {
			if (fileXioWaitAsync(FXIO_NOWAIT, (int *)_runningOp) == FXIO_COMPLETE) {
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
			if (fileXioWaitAsync(FXIO_NOWAIT, (int *)_runningOp) == FXIO_COMPLETE) {
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

