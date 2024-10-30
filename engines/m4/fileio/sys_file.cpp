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

#include "m4/fileio/sys_file.h"
#include "m4/fileio/extensions.h"
#include "m4/fileio/fileio.h"
#include "m4/adv_r/db_env.h"
#include "m4/mem/memman.h"
#include "m4/vars.h"
#include "m4/m4.h"

namespace M4 {

// Hash entry format:
//    Filebame 33 bytes, Hagfile 1 byte, Disks 1 byte, Offset 4 bytes,
//    Size 4 bytes  Next_record 4 Bytes, Total is 47 bytes.
#define  HASH_RECORD_LENGTH 47

SysFile::SysFile(const Common::String &fname, FileMode myfmode) :
	filename(fname), fmode(myfmode) {
}

bool SysFile::exists() {
	if (fmode == BINARY || fmode == TEXT) {
		show_error_flag = false;
		open_read_low_level();
		show_error_flag = true;

	} else if (fmode == BINARYW || fmode == TEXTW) {
		show_error_flag = false;
		open_write();
		show_error_flag = true;
	}

	if (!_G(hag).hag_flag)
		return _fp != nullptr;
	else
		return (hag_success);
}

uint32 SysFile::size() {
	uint32 fsize;

	open_read();

	if (!_G(hag).hag_flag) {
		fsize = rs()->size();

	} else {
		if (hag_success)
			fsize = curr_hash_record.size;
		else
			fsize = 0;
	}

	return fsize;
}

uint32 SysFile::get_pos() {
	if (!_G(hag).hag_flag) {
		if (!_fp)
			return 0;

		return rs()->pos();

	} else {
		if (hag_success)
			return (uint32)(curr_hag_record->hag_pos - curr_hash_record.offset);
		else
			return  0;
	}
}

void SysFile::open_read_low_level() {
	Common::File temp_fp;
	uint32 hash_table_size;
	char hag_name[33];
	byte hagfile;
	Hag_Name_Record *temp_ptr;
	char *temp_name;
	Common::String last_string;
	Common::String resource_hag;
	Common::File hagfile_fp;

	if (filename.empty()) {
		_fp = nullptr;
		return;
	}

	if (!_G(hag).first_read_flag) {
		// First time to read
		if (_G(hag).hag_flag) {
			// Use hag file
			// Get hagfile table list here
			if (!temp_fp.open(_G(hag).hash_file))
				error("Hash file not found: %s", _G(hag).hash_file.toString().c_str());

			hash_table_size = temp_fp.readUint32LE();
			if (!temp_fp.seek(hash_table_size * HASH_RECORD_LENGTH, SEEK_CUR))
				error("fail to seek");

			_G(hag).hag_name_list = nullptr;
			while (!temp_fp.eos()) {
				if (temp_fp.read(hag_name, 33) != 33)
					break;
				hagfile = temp_fp.readByte();

				temp_ptr = (Hag_Name_Record *)mem_alloc(sizeof(Hag_Name_Record), "hag_name_list");
				assert(temp_ptr);

				// Check hag file exists or not
				Common::Path local_name(f_extension_new(hag_name, "HAG"));

				if (!Common::File::exists(local_name))
					error("couldn't find hag file: %s", local_name.toString().c_str());

				// put into hag_name_list //
				Common::strcpy_s(temp_ptr->filename, hag_name);
				temp_ptr->hagfile = hagfile;
				temp_ptr->next = _G(hag).hag_name_list;
				_G(hag).hag_name_list = temp_ptr;
			}

			temp_fp.close();
		} else {
			_G(hag).hag_flag = 0;
		}

		_G(hag).first_read_flag = 1;
	}

	switch (mode) {
	case UNOPENED:
		switch (fmode) {
		case TEXT:
			temp_name = env_find(filename);

			if (!_G(hag).hag_flag) {
				if (temp_name) {
					filename = temp_name;
					_fp = f_io_open(Common::Path(filename), "rb");

					if (!_fp && show_error_flag)
						error("Failed opening - %s", filename.c_str());
				} else {
					_fp = open_by_first_char();
				}
			} else {
				// Hag mode
				filename = get_last_string(filename);

				if (!open_hash_file() && show_error_flag)
					error("not in hag file: %s", filename.c_str());
			}
			break;

		case BINARY:
			// is it a font file? They're stored in a *special* place oo-la-la
			temp_name = env_find(filename);

			if (!_G(hag).hag_flag) {
				if (temp_name)
					filename = temp_name;
				open_by_first_char();

			} else {
				last_string = get_last_string(filename);

				if (!open_hash_file()) {
					if (show_error_flag)
						error("not in hag file: %s", filename.c_str());
				}
			}
			break;

		default:
			break;
		}
		break;

	case READ:
	case WRITE:
		break;

	default:
		break;
	}

	mode = READ;
}

void SysFile::open_read() {
	open_read_low_level();

	if (_G(hag).hag_flag && curr_hag_record) {
		if (!curr_hag_record->hag_fp)
			error("hag file not open for: %s", filename.c_str());
	}

	if (!_G(hag).hag_flag && !_fp) {
		error("Error opening - %s", filename.c_str());
	} else if (_G(hag).hag_flag && !hag_success) {
		error("Error opening - %s", filename.c_str());
	}
}

void SysFile::open_write() {
	error("open_write is not implemented in ScummVM");
}

Common::String SysFile::get_last_string(const Common::String &src) {
	int len, j, k;

	len = src.size();
	Common::String result;

	for (j = len - 1; j >= 0; j--) {
		if (src[j] == '\\' || src[j] == ':')
			break;
	}

	if (j >= 0) {
		for (k = j + 1; k < len; k++)
			result += src[k];
		return result;

	} else {
		return src;
	}
}

bool SysFile::open_hash_file() {
	Common::SeekableReadStream *hashfp;
	Common::Stream *temp_fp;
	uint32 hash_address;
	Common::String hag_name, temp_name;
	Common::String local_name;
	Hag_Record *temp_ptr;
	uint32  hash_table_size;
	bool found;
	Common::String ext_name;

	hashfp = dynamic_cast<Common::SeekableReadStream *>(f_io_open(_G(hag).hash_file, "rb"));
	if (!hashfp) {
		warning("open_hash_file: %s", _G(hag).hash_file.toString().c_str());
		hag_success = false;
		return false;
	}

	hash_table_size = hashfp->readUint32LE();
	hash_address = key_to_hash_address(filename, hash_table_size);

	if (!hash_search(filename, &curr_hash_record, curr_hag_record, hash_address, hashfp, hash_table_size, show_error_flag)) {
		hag_success = 0;
		return false;

	} else {
		// How to open hag file
		// Calculate Hagfile name - depends on hagfile field in curr_hash_record
		if (!get_local_name_from_hagfile(local_name, curr_hash_record.hagfile)) {
			hag_success = 0;
			return false;
		}

		// Check if this Hag file already open or not
		local_name = f_extension_new(local_name, "HAG");
		temp_name = local_name;
		hag_name = local_name;		// Original used in cd_resource + name

		found = false;
		temp_ptr = _G(hag).hag_file_list;

		// Search local open files for hag file...
		while (temp_ptr) {
			if (hag_name.equalsIgnoreCase(temp_ptr->hag_name)) {
				found = true;
				break;
			}
			temp_ptr = temp_ptr->next;
		}

		// Search resource directory open files for hag file
		if (!found) {
			temp_ptr = _G(hag).hag_file_list;
			found = false;
			while (temp_ptr) {
				if (temp_name.equalsIgnoreCase(temp_ptr->hag_name)) {
					found = true;
					break;
				}
				temp_ptr = temp_ptr->next;
			}
			if (!found) {
				// hag file is not open, try the current directory first, then RESOURCE_PATH
				temp_fp = f_io_open(Common::Path(hag_name), "rb");
				if (!temp_fp) {
					// hag_file is not in current directory, search for RESOURCE_PATH
					temp_fp = f_io_open(Common::Path(temp_name), "rb");
					if (!temp_fp) {
						error("hag file not found: %s", hag_name.c_str());
						hag_success = 0;
						return 0;
					}

					// Add this new open hag file in resource dir into open hag file list
					temp_ptr = (Hag_Record *)mem_alloc(sizeof(Hag_Record), "Hag_File_List");
					if (!temp_ptr) {
						f_io_close(temp_fp);
						error("creating Hag_record");
						hag_success = 0;
						return 0;
					}

					Common::strcpy_s(temp_ptr->hag_name, temp_name.c_str());
					temp_ptr->hag_fp = temp_fp;

					Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(temp_fp);
					assert(rs);

					if (!rs->seek(curr_hash_record.offset))
						term_message("fail to fseek");
					last_head_pos = rs->pos();

					temp_ptr->hag_pos = curr_hash_record.offset;
					temp_ptr->hagfile = curr_hash_record.hagfile;

					// insert the element into list
					temp_ptr->next = _G(hag).hag_file_list;
					_G(hag).hag_file_list = temp_ptr;
				}
				// we just opened a previously unopened hag file
				else {
					// add this new open hag file in exec dir into its list
					temp_ptr = (Hag_Record *)mem_alloc(sizeof(Hag_Record), "Hag_File_List");
					if (!temp_ptr) {
						f_io_close(temp_fp);
						error("creating hag_record");
						hag_success = 0;
						return 0;
					}

					Common::strcpy_s(temp_ptr->hag_name, hag_name.c_str());
					temp_ptr->hag_fp = temp_fp;

					Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(temp_fp);
					assert(rs);

					if (!rs->seek(curr_hash_record.offset))
						term_message("fail to fseek");
					last_head_pos = rs->pos();

					temp_ptr->hag_pos = curr_hash_record.offset;
					temp_ptr->hagfile = curr_hash_record.hagfile;

					// Insert the element into list
					temp_ptr->next = _G(hag).hag_file_list;
					_G(hag).hag_file_list = temp_ptr;
				}
			}
		}

		Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(temp_ptr->hag_fp);
		assert(rs);

		// set hag file pointer to current file position //
		if (!rs->seek(curr_hash_record.offset))
			term_message("fail to fseek");
		last_head_pos = rs->pos();

		temp_ptr->hag_pos = curr_hash_record.offset;
		curr_hag_record = temp_ptr;

		hag_success = true;
		return true;
	}
}

uint32 SysFile::key_to_hash_address(const Common::String &src, uint32 hash_table_size) {
	uint32 h;
	int i, len;

	Common::String key = src;
	key.toUppercase();

	if (key.empty())
		return 0;

	h = key[0];
	len = key.size();
	for (i = 1; i < len; i++)
		h = ((h * 256) + key[i]) % hash_table_size;   // * 256 is because one char is 8 bits

	return h;
}

int SysFile::hash_search(const Common::String &fname, Hash_Record *current_hash_record_ptr, Hag_Record *current_hag_record, uint32 hash_address,
	Common::SeekableReadStream *hashfp, uint32 hash_table_size, bool show_errors) {
	uint32 offset;
	bool finded = false;
	char myfilename[33];
	uint32 next_entry;
	uint32 best_dist, find_offset, next_record;
	Common::String local_name, local_hag_name;


	next_entry = hash_address;
	// 4 bytes is header of hash file, store hash_table_size
	offset = HASH_RECORD_LENGTH * next_entry + 4;
	best_dist = 0x7fffffff;
	find_offset = offset;
	myfilename[0] = '\0';

	while (!finded) {
		if (!hashfp->seek(offset))
			term_message("fail to fseek");
		hashfp->read(myfilename, 33);

		if (myfilename[0] == '\0') {
			// this hash table is empty, insert new record here
			f_io_close(hashfp);

			if (show_errors)
				error("not found in hag file: %s", fname.c_str());
			else
				term_message("fclass: file not found '%s', in hag file", fname.c_str());
			return 0;
		} else if (fname.equalsIgnoreCase(myfilename)) {
			// The new record already in hash table, do nothing
			auto &r = *current_hash_record_ptr;
			r.hagfile = hashfp->readByte();
			r.disks = hashfp->readByte();
			r.offset = hashfp->readUint32LE();
			r.size = hashfp->readUint32LE();
			next_record = hashfp->readUint32LE();
			r.filename = myfilename;

			// As long as we find a hag file, use it immedeiately
			get_local_name_from_hagfile(local_name, current_hash_record_ptr->hagfile);
			local_hag_name = f_extension_new(local_name, "HAG");
			local_name = local_hag_name;

			if (!Common::File::exists(Common::Path(local_name))) {
				finded = 1;
				find_offset = offset;
				break;
			}
			if (current_hag_record && current_hag_record->hagfile == current_hash_record_ptr->hagfile) {
				// in same hag file
				if (best_dist > (uint32)ABS((int32)current_hag_record->hag_pos - (int32)current_hash_record_ptr->offset)) {
					best_dist = ABS((int32)current_hag_record->hag_pos - (int32)current_hash_record_ptr->offset);
					find_offset = offset;
				}
			} else {
				find_offset = offset;
			}
			if (next_record == offset) {
				// only one record of fname in hash table
				finded = 1;
			} else {
				offset = next_record;
			}
		} else {   // collision here, search the next entry to see if it is empty until find a empty one
			next_entry = (next_entry + 1) % hash_table_size;  // search the hash table a round way
			offset = HASH_RECORD_LENGTH * next_entry + 4; // 4 bytes is header of hash file, store hash_table_size
		}
	}


	// get the best close one of hag file for multiple same fname
	if (find_offset != offset) {
		if (!hashfp->seek(find_offset))
			term_message("fail to fseek");

		auto &r = *current_hash_record_ptr;
		hashfp->read(myfilename, 33);
		r.filename = myfilename;
		r.hagfile = hashfp->readByte();
		r.disks = hashfp->readByte();
		r.offset = hashfp->readUint32LE();
		r.size = hashfp->readUint32LE();
	}

	f_io_close(hashfp);
	if (!finded) {
		error("not in hag file: %s", fname.c_str());
	}

	return true;
}

Common::Stream *SysFile::open_by_first_char() {
	if (filename.hasPrefix("*")) {
		// MADS folder file in original
		_fp = f_io_open(Common::Path(filename.c_str() + 1, '/'), "rb");
	} else {
		_fp = f_io_open(Common::Path(filename, '/'), "rb");
	}

	if (!_fp) {
		if (show_error_flag)
			error("fclass: file not found '%s'", filename.c_str());
		else
			term_message("fclass: file not found '%s'", filename.c_str());
	}

	return nullptr;
}

bool SysFile::get_local_name_from_hagfile(Common::String &local_name, byte hagfile) {
	bool found = false;
	Hag_Name_Record *temp_ptr = _G(hag).hag_name_list;

	while (temp_ptr && !found) {
		if (temp_ptr->hagfile == hagfile) {
			found = true;
			local_name = temp_ptr->filename;
		} else {
			temp_ptr = temp_ptr->next;
		}
	}

	return found;
}

bool SysFile::seek(uint32 pos) {
	if (!_G(hag).hag_flag) {
		return rs()->seek(pos);

	} else {
		if (hag_success) {
			Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(curr_hag_record->hag_fp);
			assert(rs);

			if (!rs->seek(curr_hash_record.offset + pos - curr_hag_record->hag_pos, SEEK_CUR))
				term_message("fail to fseek");
			last_head_pos = rs->pos();

			curr_hag_record->hag_pos = curr_hash_record.offset + pos;	// Change file position
			return true;

		} else {
			return false;
		}
	}
}

bool SysFile::seek_ahead(int32 amount) {
	if (!_G(hag).hag_flag) {
		return rs()->seek(amount, SEEK_CUR);

	} else {
		if (hag_success) {
			Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(curr_hag_record->hag_fp);
			assert(rs);

			if (!rs->seek(amount, SEEK_CUR))
				term_message("fail to fseek");

			last_head_pos = rs->pos();
			curr_hag_record->hag_pos += amount;		// Change file position
			return true;

		} else {
			return false;
		}
	}
}


uint32 SysFile::read(MemHandle bufferHandle) {
	int32 bytesToRead  = size() - get_pos();
	if (bytesToRead < 0)
		error("SysFile::read - %s", filename.c_str());

	return read(bufferHandle, (int32)bytesToRead);
}

int32 SysFile::read(byte *bufferHandle, int32 n) {
	void *h = bufferHandle;
	return read((MemHandle)&h, n);
}

int32 SysFile::read(MemHandle bufferHandle, int32 n) {
	uint32 temp_size;

	if (!bufferHandle)
		error("reading %s", filename.c_str());

	open_read();

	if (!*bufferHandle)
		mem_ReallocateHandle(bufferHandle, n, "SysFile");
	if (!*bufferHandle)
		error("Needed %d to read info", n);

	if (!_G(hag).hag_flag) {
		return (uint32)rs()->read(*bufferHandle, n);

	} else {
		// Hag mode
		if (hag_success) {
			Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(curr_hag_record->hag_fp);
			assert(rs);
			rs->seek(last_head_pos);
			uint32 temp_myfpos = rs->pos();

			temp_size = (uint32)rs->read(*bufferHandle, n);
			curr_hag_record->hag_pos = temp_myfpos + temp_size;   // Change file position

			last_head_pos = rs->pos();
			return temp_size;

		} else {
			return 0;
		}
	}
}

byte SysFile::readByte() {
	byte buf[1];
	void *ptr = (void *)buf;
	read(&ptr, 1);

	return buf[0];
}

uint16 SysFile::readUint16LE() {
	byte buf[2];
	void *ptr = (void *)buf;
	read(&ptr, 2);

	return READ_LE_UINT16(buf);
}

uint32 SysFile::readUint32LE() {
	byte buf[4];
	void *ptr = (void *)buf;
	read(&ptr, 4);

	return READ_LE_UINT32(buf);
}

Common::SeekableReadStream *SysFile::rs() const {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_fp);
	assert(rs);
	return rs;
}

void SysFile::close() {
	delete _fp;
	_fp = nullptr;
}


void sysfile_init(bool in_hag_mode) {
	_G(hag).hag_flag = in_hag_mode;

	if (in_hag_mode) {
		const char *name = "burger";
		if (g_engine->getGameType() == GType_Riddle)
			name = "ripley";
		else if (g_engine->isDemo() == GStyle_NonInteractiveDemo)
			name = "overview";

		_G(hag).hash_file = Common::Path(Common::String::format("%s.has", name));
		term_message("Initialized in hag mode");
	} else {
		term_message("Initialized in file mode");
	}
}

void sysfile_shutdown() {
	Hag_Record *temp_ptr;

	temp_ptr = _G(hag).hag_file_list;
	while (temp_ptr) {
		_G(hag).hag_file_list = _G(hag).hag_file_list->next;
		f_io_close(temp_ptr->hag_fp);
		mem_free(temp_ptr);
		temp_ptr = _G(hag).hag_file_list;
	}
}

} // namespace M4
