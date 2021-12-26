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

#ifndef __FRAMFS_SAVE_MANAGER__
#define __FRAMFS_SAVE_MANAGER__

#include <common/savefile.h>
#include <common/zlib.h>

#include <framfs.h> // N64 FramFS library

bool fram_deleteSaveGame(const char *filename);

class InFRAMSave : public Common::InSaveFile {
private:
	FRAMFILE *fd;

	uint32 read(void *buf, uint32 cnt) override;
	bool skip(uint32 offset) override;
	bool seek(int64 offs, int whence) override;

public:
	InFRAMSave() : fd(NULL) { }

	~InFRAMSave() {
		if (fd != NULL)
			framfs_close(fd);
	}

	bool eos() const override {
		return framfs_eof(fd);
	}
	void clearErr() override {
		framfs_clearerr(fd);
	}
	int64 pos() const override {
		return framfs_tell(fd);
	}
	int64 size() const override {
		return fd->size;
	}

	bool readSaveGame(const char *filename) {
		fd = framfs_open(filename, "r");
		return (fd != NULL);
	}
};

class OutFRAMSave : public Common::WriteStream {
private:
	FRAMFILE *fd;

public:
	uint32 write(const void *buf, uint32 cnt);
	virtual int64 pos() const {
		return framfs_tell(fd);
	}

	OutFRAMSave(const char *_filename) : fd(NULL) {
		fd = framfs_open(_filename, "w");
	}

	~OutFRAMSave() {
		if (fd != NULL) {
			finalize();
			framfs_close(fd);
		}
	}

	bool err() const {
		if (fd)
			return (framfs_error(fd) == 1);
		else
			return true;
	}
	void clearErr() {
		framfs_clearerr(fd);
	}
	void finalize() {
		framfs_flush(fd);
	}
};

class FRAMSaveManager : public Common::SaveFileManager {
public:
	void updateSavefilesList(Common::StringArray &lockedFiles) override {
		// this method is used to lock saves while cloud syncing
		// as there is no network on N64, this method wouldn't be used
		// thus it's not implemtented
	}

	Common::InSaveFile *openRawFile(const Common::String &filename) override {
		InFRAMSave *s = new InFRAMSave();
		if (s->readSaveGame(filename.c_str())) {
			return s;
		} else {
			delete s;
			return 0;
		}
	}

	Common::OutSaveFile *openForSaving(const Common::String &filename, bool compress = true) override {
		OutFRAMSave *s = new OutFRAMSave(filename.c_str());
		if (!s->err()) {
			return new Common::OutSaveFile(compress ? Common::wrapCompressedWriteStream(s) : s);
		} else {
			delete s;
			return 0;
		}
	}

	Common::InSaveFile *openForLoading(const Common::String &filename) override {
		InFRAMSave *s = new InFRAMSave();
		if (s->readSaveGame(filename.c_str())) {
			return Common::wrapCompressedReadStream(s);
		} else {
			delete s;
			return 0;
		}
	}

	bool removeSavefile(const Common::String &filename) override {
		return ::fram_deleteSaveGame(filename.c_str());
	}

	Common::StringArray listSavefiles(const Common::String &pattern) override;

	bool exists(const Common::String &filename) override {
		return InFRAMSave().readSaveGame(filename.c_str());
	}
};


#endif
