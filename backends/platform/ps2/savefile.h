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

#ifndef __PS2_SAVEFILE__
#define __PS2_SAVEFILE__

class McAccess;
class OSystem_PS2;
class Gs2dScreen;
class Ps2SaveFileManager;

#include "common/scummsys.h"
#include "common/savefile.h"
#include "backends/platform/ps2/rawsavefile.h"
#include "backends/platform/ps2/fileio.h"

class UclOutSaveFile : public Common::OutSaveFile, public RawWriteFile {
public:
	UclOutSaveFile(const char *filename, OSystem_PS2 *system, Gs2dScreen *screen, McAccess *mc);
	virtual ~UclOutSaveFile(void);
	virtual uint32 write(const void *ptr, uint32 size);
	virtual bool flush(void);
	virtual bool ioFailed(void) const;
	virtual void clearIOFailed(void);
private:
	OSystem_PS2 *_system;
	Gs2dScreen *_screen;

	bool _ioFailed, _wasFlushed;
	char _fileName[128];
};

class UclInSaveFile : public Common::InSaveFile, public RawReadFile {
public:
	UclInSaveFile(const char *filename, Gs2dScreen *screen, McAccess *mc);
	virtual ~UclInSaveFile(void);
	virtual bool eos(void) const;
	virtual uint32 read(void *ptr, uint32 size);
	virtual bool ioFailed(void) const;
	virtual void clearIOFailed(void);
	virtual bool skip(uint32 offset);

	virtual int32 pos(void) const;
	virtual int32 size(void) const;
	virtual bool seek(int pos, int whence = SEEK_SET);
private:
	Gs2dScreen *_screen;
	bool _ioFailed;
};

class AutoSaveFile : public Common::OutSaveFile {
public:
	AutoSaveFile(Ps2SaveFileManager *saveMan, const char *filename);
	~AutoSaveFile(void);
	virtual uint32 write(const void *ptr, uint32 size);
	virtual bool flush(void) {}
	virtual bool ioFailed(void) { return false; };
	virtual void clearIOFailed(void) {}
private:
	Ps2SaveFileManager *_saveMan;
	char _fileName[256];
	uint8 *_buf;
	uint32 _bufSize, _bufPos;
};

// Glue-classes, not only do we have to provide MC access through the savefilemanager, but also using
// posix file i/o, since that's what's the configmanager expects for storing the scummvm.ini

class Ps2McReadFile : public RawReadFile, public Ps2File {
public:
	Ps2McReadFile(Ps2SaveFileManager *saveMan);
	virtual ~Ps2McReadFile(void);
	virtual bool open(const char *name);
	virtual uint32 read(void *dest, uint32 len);
	virtual uint32 write(const void *src, uint32 len);
	virtual int32 tell(void);
	virtual int32 size(void);
	virtual int seek(int32 offset, int origin);
	virtual bool eof(void);
};

class Ps2McWriteFile : public RawWriteFile, public Ps2File {
public:
	Ps2McWriteFile(Ps2SaveFileManager *saveMan);
	virtual ~Ps2McWriteFile(void);
	virtual bool open(const char *name);
	virtual uint32 read(void *dest, uint32 len);
	virtual uint32 write(const void *src, uint32 len);
	virtual int32 tell(void);
	virtual int32 size(void);
	virtual int seek(int32 offset, int origin);
	virtual bool eof(void);
};

#endif // __PS2_SAVEFILE__
