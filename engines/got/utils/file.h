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

#ifndef GOT_UTILS_FILE_H
#define GOT_UTILS_FILE_H

#include "common/file.h"
#include "got/defines.h"

namespace Got {

extern long file_size(const char *path);
extern unsigned int read_file(const char *filename, const char *buff,
	long offset, unsigned int amount, int key);

extern bool load_sd_data();
extern bool load_actor(int file, int num);
extern bool load_picture(int index, char *buff);
extern void setup_filenames(int level);
extern bool  load_speech(int index);
extern long file_size(const char *path);
extern void *get_file(const char *filename, int key);
extern void save_game();
extern bool load_game(int flag);
extern void help();
extern long res_read(const Common::String &name, void *buff);

class File : public Common::File {
public:
	File() : Common::File() {}
	File(const Common::String &filename) : Common::File() {
		File::open(Common::Path(filename));
	}

	bool open(const Common::Path &filename) override;
};

} // namespace Got

#endif
