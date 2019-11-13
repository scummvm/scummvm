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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/archetype/game_stat.h"
#include "glk/archetype/archetype.h"
#include "glk/archetype/crypt.h"
#include "glk/archetype/saveload.h"
#include "glk/archetype/statement.h"
#include "glk/archetype/timestamp.h"

namespace Glk {
namespace Archetype {

void save_game_state(Common::WriteStream *bfile, XArrayType &objects) {
	int i;
	void *p;

	// Write out the timestamp associated with the original game
	bfile->writeUint32LE(GTimeStamp);

	// Get the encryption straight - reset the seed
	cryptinit(Encryption, GTimeStamp);

	for (i = 1; i < Dynamic; ++i) {
		if (index_xarray(objects, i, p)) {
			ObjectPtr op = (ObjectPtr)p;
			bfile->writeByte(vContSeq);

			dump_item_list(bfile, op->attributes, EXPR_LIST);
		}
	}

	for (i = Dynamic; i <= (int)objects.size(); ++i) {
		if (index_xarray(objects, i, p)) {
			bfile->writeByte(vContSeq);
			dump_object(bfile, (ObjectPtr)p);
		}
	}

	bfile->writeByte(vEndSeq);
}

bool load_game_state(Common::ReadStream *bfile, XArrayType &objects) {
	int i;
	void *p;
	ObjectPtr op;
	TimestampType tstamp;
	StatementKind sentinel;

	// Check the time stamp
	tstamp = bfile->readUint32LE();
	if (tstamp != GTimeStamp) {
		g_vm->writeln("State file does not match original .ACX file");
		return false;
	}

	// Get the encryption straight - reset the seed.Be careful upon loading since we have
	// to do UNPURPLE instead of PURPLE
	if (Encryption == PURPLE)
		Encryption = UNPURPLE;
	cryptinit(Encryption, GTimeStamp);

	// Need to flush out the previous attributes andload in the new ones. Dynamically allocated
	// objects are a little different since they might vary between game states
	for (i = 1; i < Dynamic; ++i) {
		if (index_xarray(objects, i, p)) {
			sentinel = (StatementKind)bfile->readByte();
			op = (ObjectPtr)p;
			dispose_item_list(op->attributes, EXPR_LIST);
			load_item_list(bfile, op->attributes, EXPR_LIST);
		}
	}

	// Flush dynamic objects.Dispose of each object andshrink back the xarray

	for (i = objects.size(); i >= Dynamic; --i) {
		if (index_xarray(objects, i, p)) {
			op = (ObjectPtr)p;
			dispose_object(op);
		}

		shrink_xarray(objects);
	}

	// sentinel has been set from before
	sentinel = (StatementKind)bfile->readByte();
	while (sentinel == CONT_SEQ) {
		load_object(bfile, op);
		p = op;
		append_to_xarray(objects, p);

		sentinel = (StatementKind)bfile->readByte();
	}

	if (Encryption == UNPURPLE)
		Encryption = PURPLE;

	return true;
}

} // End of namespace Archetype
} // End of namespace Glk
