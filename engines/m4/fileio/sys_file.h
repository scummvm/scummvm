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

#ifndef M4_FILEIO_SYS_FILE_H
#define M4_FILEIO_SYS_FILE_H

#include "common/file.h"
#include "m4/fileio/fileio.h"
#include "m4/mem/reloc.h"

namespace M4 {

struct Hash_Record {
	Common::String filename;
	byte hagfile;
	byte disks;
	uint32 offset;
	uint32 size;
};

struct Hag_Statics {
	bool hag_flag = false;
	bool first_read_flag = false;
	Common::Path hash_file;
	Hag_Name_Record *hag_name_list = nullptr;
	Hag_Record *hag_file_list = nullptr;
};

class SysFile {
private:
	Common::Stream *_fp = nullptr;
	FileMode fmode;
	AccessMode mode = UNOPENED;
	int hag_success = 0;
	bool show_error_flag = true;
	Hag_Record *curr_hag_record = nullptr;
	Hash_Record curr_hash_record;
	uint32 last_head_pos = 0;

private:
	void open_read();
	void open_write();
	void open_read_low_level();
	bool open_hash_file();
	Common::String get_last_string(const Common::String &src);

	/**
	 * Calculate key (which is string) into hash table address ie. hash table entry
	 */
	uint32 key_to_hash_address(const Common::String &src, uint32 hash_table_size);

	/**
	 * Insert a hash record into HASH file depending hash_address. The algorithm is
	 * if hash_address entry is empty record, put the new record into this entry,
	 * if the hash_address entry is not empty, search next record of it, util find
	 * one empty entry and put the new record into the empty entry.
	*/
	int hash_search(const Common::String &fname, Hash_Record *current_hash_record_ptr, Hag_Record *curr_hag_record, uint32 hash_address,
		Common::SeekableReadStream *hashfp, uint32 hash_table_size, bool show_error_flag);

	/**
	 * Opens the file for access, paying attention to the first character..
	 * if it's a '*', it indicates a MADS folder file
	 */
	Common::Stream *open_by_first_char();

	bool get_local_name_from_hagfile(Common::String &local_name, byte hagfile);

	Common::SeekableReadStream *rs() const;

public:
	Common::String filename;

public:
	SysFile(const Common::String &fname, FileMode myfmode = BINARY);

	/**
	 * Return true if the file exists
	 */
	bool exists();

	/**
	 * Returns the file size
	 */
	uint32 size();

	/**
	 * Gets the current position
	 */
	uint32 get_pos();

	/**
	 * Seek to a given position
	 */
	bool seek(uint32 pos);

	/**
	 * Seek ahaead by a given amount
	 */
	bool seek_ahead(int32 amount);

	/**
	 * Read data
	 */
	uint32 read(MemHandle bufferHandle);
	int32 read(MemHandle bufferHandle, int32 n);
	int32 read(byte *bufferHandle, int32 n);

	/**
	 * Read in a 8-bit value
	 */
	byte readByte();
	int8 readSByte() {
		return (int8)readByte();
	}

	/**
	 * Read in a 16-bit value
	 */
	uint16 readUint16LE();
	int16 readSint16LE() {
		return (int16)readUint16LE();
	}

	/**
	 * Read in a 32-bit value
	 */
	uint32 readUint32LE();
	int32 readSint32LE() {
		return (int32)readUint32LE();
	}

	/**
	 * Close the file
	 */
	void close();
};

void sysfile_init(bool in_hag_mode);
void sysfile_shutdown();

} // namespace M4

#endif
