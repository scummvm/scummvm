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

#ifndef MADS_CORE_LOADER_H
#define MADS_CORE_LOADER_H

#include "common/stream.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/pack.h"

namespace MADS {
namespace MADSV2 {

#define LOADER_DISK     0       /* File being read from disk */
#define LOADER_EMS      1       /* File being read from ems  */
#define LOADER_XMS      2       /* File being read from xms  */


typedef struct {
	int open;                   /* Open status (true = open)             */
	int reading;                /* Read/Write status (true = reading)    */
	byte mode;                  /* Access mode (LOADER_EMS/LOADER_DISK)  */
	Common::SeekableReadStream *handle; /* File handle (if LOADER_DISK)          */
	int ems_handle;             /* EMS handle  (if LOADER_EMS)           */
	int xms_handle;             /* XMS handle  (if LOADER_XMS)           */
	long xms_offset;            /* XMS offset  (if LOADER_XMS)           */
	int ems_page_marker;        /* Current EMS logical page #            */
	int ems_page_offset;        /* Current EMS logical page offset       */
	long decompress_size;       /* Decompressed size of file             */
	int pack_list_marker;       /* Packing list marker                   */
	PackList pack;              /* Packing list                          */
} Load;

typedef Load *LoadHandle;

/**
 * Simple encapsulation for using loaders in Conv::load
 */
class LoaderReadStream : public Common::SeekableReadStream {
private:
	LoadHandle _load;
public:
	LoaderReadStream(LoadHandle load) : _load(load) {}
	~LoaderReadStream() override {
	}

	int64 pos() const override { return 0; }
	int64 size() const override { return 0; }
	bool seek(int64 offset, int whence = SEEK_SET) override { return false; }
	bool eos() const override { return false; }
	uint32 read(void *dataPtr, uint32 dataSize) override;
};


/* Debug tracking variables */
#ifndef disable_statistics
extern long loader_found_in_ems;
extern long loader_found_in_xms;
extern long loader_found_on_disk;
extern long loader_timing_ems;
extern long loader_timing_xms;
extern long loader_timing_disk;
extern long loader_size_ems;
extern long loader_size_xms;
extern long loader_size_disk;
#endif

extern int  loader_ems_search_disabled;
extern char loader_last[14];

extern int loader_open(LoadHandle handle, const char *filename, const char *options, int flags);
extern int loader_close(LoadHandle handle);
extern void loader_set_priority(LoadHandle handle, int priority);
extern long loader_read(void *target, long record_size, long record_count, LoadHandle handle);
extern long loader_write(void *target, long record_size, long record_count, LoadHandle handle);
extern long loader_write_2(Common::WriteStream *source_handle, long total_size, LoadHandle handle);

} // namespace MADSV2
} // namespace MADS

#endif
