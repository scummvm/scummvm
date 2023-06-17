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

#include "m4/adv_r/adv_been.h"
#include "m4/core/errors.h"
#include "m4/mem/memman.h"
#include "m4/globals.h"

namespace M4 {

bool player_been_init(int16 num_scenes) {
	_G(scene_list).table = (int16 *)mem_alloc(sizeof(int16) * num_scenes, "been_scenes");

	if (!_G(scene_list).table)
		error_show(FL, 'OOM!', "player_been_init");

	_G(scene_list).total_scenes = num_scenes;
	player_reset_been();

	return true;
}

void player_been_shutdown() {
	mem_free(_G(scene_list).table);
}

void player_reset_been() {
	memset(_G(scene_list).table, 0, sizeof(int16) * _G(scene_list).total_scenes);
	_G(scene_list).tail = 0;
}

int32 player_been_restore(Common::SeekableReadStream *file_ptr) {
	// Read chunk type marker
	int32 size;

	if (file_ptr->readUint32BE() != 'BEEN')
		error_show(FL, 'PBR1');

	// Skip chunk size and read number of scenes
	file_ptr->skip(4);
	size = file_ptr->readUint32LE();

	// Do we need to reallocate the scene table?
	if (size != _G(scene_list).total_scenes) {
		_G(scene_list).table = (int16 *)mem_realloc(_G(scene_list).table, size * sizeof(int16), "been_scenes");
		if (!_G(scene_list).table)
			error_show(FL, 'OOM!');
	}
	_G(scene_list).total_scenes = size;

	// Read the tail
	_G(scene_list).tail = file_ptr->readUint32LE();

	// Read in the scene list
	for (int i = 0; i < _G(scene_list).total_scenes; ++i)
		_G(scene_list).table[i] = file_ptr->readUint16LE();

	return (4 * sizeof(int32) + sizeof(int16) * _G(scene_list).total_scenes);
}

int32 player_been_save(Common::WriteStream *handle) {
	int32 size;

	// write chunk identity
	handle->writeUint32BE('BEEN');

	// Write chunk size
	size = 4 * sizeof(int32) + sizeof(int16) * _G(scene_list).total_scenes;
	handle->writeUint32LE(size);

	// write number of scenes
	handle->writeUint32LE(_G(scene_list).total_scenes);

	// write current tail
	handle->writeUint32LE(_G(scene_list).tail);

	// Write scene list
	for (int i = 0; i < _G(scene_list).total_scenes; ++i)
		handle->writeUint16LE(_G(scene_list).table[i]);

	return (sizeof(int32) + (sizeof(int16) * _G(scene_list).total_scenes));
}

/**
 * Checks whether player has been in specified scene before
 */
static bool player_been_here(int16 scene_num) {
	for (int i = 0; i < _G(scene_list).tail; i++)
		if (_G(scene_list).table[i] == scene_num)
			return true;

	return false;
}

bool player_enters_scene(int16 scene_num) {
	if (player_been_here(scene_num))
		return true;

	_G(scene_list).table[_G(scene_list).tail] = scene_num;
	++_G(scene_list).tail;

	if (_G(scene_list).tail > _G(scene_list).total_scenes)
		error_show(FL, 'SLTS');

	return false;
}

void player_forgets_scene(int16 scene_num) {
	int i, j;

	// Check the list
	for (i = 0; i < _G(scene_list).tail; i++) {
		// Found a match
		if (_G(scene_list).table[i] == scene_num) {
			// Close the table
			for (j = i; j < _G(scene_list).tail; j++) {
				_G(scene_list).table[j] = _G(scene_list).table[j + 1];
			}

			// One less scene
			--_G(scene_list).tail;
			return;
		}
	}
}

} // End of namespace M4
