/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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

#define MAX_HANDLES 32
#include <sys/stat.h>

class AsyncFio {
public:
	AsyncFio(void);
	~AsyncFio(void);
	int open(const char *name, int ioMode);
	void close(int handle);
	void read(int fd, void *dest, unsigned int len);
	void write(int fd, const void *src, unsigned int len);
	int seek(int fd, int offset, int whence);
	int mkdir(const char *name);
	int dopen(const char *name);
	int dread(int fd, iox_dirent_t *dest);
	void dclose(int fd);
	int mount(const char *mountpoint, const char *mountstring, int flag);
	int umount(const char *mountpoint);
	int sync(int fd);
	bool poll(int fd);
	bool fioAvail(void);
private:
	void checkSync(void);
	int _ioSema;
	volatile int * volatile _runningOp;
	volatile int _ioSlots[MAX_HANDLES];
};

