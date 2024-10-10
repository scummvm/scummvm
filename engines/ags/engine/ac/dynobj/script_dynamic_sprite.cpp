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

#include "ags/engine/ac/dynobj/script_dynamic_sprite.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/util/stream.h"
#include "ags/engine/ac/dynamic_sprite.h"

namespace AGS3 {

using namespace AGS::Shared;

int ScriptDynamicSprite::Dispose(void *address, bool force) {
	// always dispose
	if ((slot) && (!force))
		free_dynamic_sprite(slot);

	delete this;
	return 1;
}

const char *ScriptDynamicSprite::GetType() {
	return "DynamicSprite";
}

size_t ScriptDynamicSprite::CalcSerializeSize(const void * /*address*/) {
	return sizeof(int32_t);
}

void ScriptDynamicSprite::Serialize(const void *address, Stream *out) {
	out->WriteInt32(slot);
}


void ScriptDynamicSprite::Unserialize(int index, Stream *in, size_t data_sz) {
	slot = in->ReadInt32();
	ccRegisterUnserializedObject(index, this, this);
}

ScriptDynamicSprite::ScriptDynamicSprite(int theSlot) {
	slot = theSlot;
	ccRegisterManagedObject(this, this);
}

ScriptDynamicSprite::ScriptDynamicSprite() {
	slot = 0;
}

} // namespace AGS3
